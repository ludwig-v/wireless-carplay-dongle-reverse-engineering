/*
 *
 *  Bluetooth HCI UART driver
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2004-2005  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/ioctl.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>

#include "hci_uart.h"

#if WOBT_NOTIFY
#include <linux/suspend.h>
#endif

#ifdef BTCOEX
#include "rtk_coex.h"
#endif

#define VERSION "2.2.74e8f89.20210819-170431"

#if HCI_VERSION_CODE > KERNEL_VERSION(3, 4, 0)
#define GET_DRV_DATA(x)		hci_get_drvdata(x)
#else
#define GET_DRV_DATA(x)		(struct hci_uart *)(x->driver_data)
#endif

#define SEMWAIT_TIMEOUT		50

/*
 * Linux 5.10 initializes a newly registered HCI controller asynchronously.
 * The U2W Carlinkit userspace assumes HCIUARTSETPROTO does not
 * return until hciconfig can inspect a fully initialized hci0; if it cannot,
 * attach_bluetooth.sh immediately resets the UART and retries.  Keep that
 * userspace contract local to the vendor Realtek line discipline.
 */
#define HCI_UART_SETUP_WAIT_MS	10000

#if WOBT_NOTIFY
struct hci_rsp_read_local {
	__u8     status;
	__u8     hci_ver;
	__le16   hci_rev;
	__u8     lmp_ver;
	__le16   manufacturer;
	__le16   lmp_subver;
} __packed;
#endif

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
static int reset = 0;
#endif

static struct hci_uart_proto *hup[HCI_UART_MAX_PROTO];
static int hci_uart_flush(struct hci_dev *hdev);

int hci_uart_register_proto(struct hci_uart_proto *p)
{
	if (p->id >= HCI_UART_MAX_PROTO)
		return -EINVAL;

	if (hup[p->id])
		return -EEXIST;

	hup[p->id] = p;

	return 0;
}

int hci_uart_unregister_proto(struct hci_uart_proto *p)
{
	if (p->id >= HCI_UART_MAX_PROTO)
		return -EINVAL;

	if (!hup[p->id])
		return -EINVAL;

	hup[p->id] = NULL;

	return 0;
}

static struct hci_uart_proto *hci_uart_get_proto(unsigned int id)
{
	if (id >= HCI_UART_MAX_PROTO)
		return NULL;

	return hup[id];
}

static inline void hci_uart_tx_complete(struct hci_uart *hu, int pkt_type)
{
	struct hci_dev *hdev = hu->hdev;

	/* Update HCI stat counters */
	switch (pkt_type) {
	case HCI_COMMAND_PKT:
		hdev->stat.cmd_tx++;
		break;

	case HCI_ACLDATA_PKT:
		hdev->stat.acl_tx++;
		break;

	case HCI_SCODATA_PKT:
		hdev->stat.sco_tx++;
		break;
	}
}

static inline void hci_proto_read_lock(struct hci_uart *hu)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	percpu_down_read(&hu->proto_lock);
#else
	down_read(&hu->proto_lock);
#endif
}

static inline int hci_proto_read_trylock(struct hci_uart *hu)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	return percpu_down_read_trylock(&hu->proto_lock);
#else
	return down_read_trylock(&hu->proto_lock);
#endif
}

static inline void hci_proto_read_unlock(struct hci_uart *hu)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	percpu_up_read(&hu->proto_lock);
#else
	up_read(&hu->proto_lock);
#endif
}

static inline void hci_proto_write_lock(struct hci_uart *hu)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	percpu_down_write(&hu->proto_lock);
#else
	down_write(&hu->proto_lock);
#endif
}

static inline void hci_proto_write_unlock(struct hci_uart *hu)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	percpu_up_write(&hu->proto_lock);
#else
	up_write(&hu->proto_lock);
#endif
}

static inline int hci_proto_init_rwlock(struct hci_uart *hu)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	return percpu_init_rwsem(&hu->proto_lock);
#else
	init_rwsem(&hu->proto_lock);
	return 0;
#endif
}

static inline void hci_proto_free_rwlock(struct hci_uart *hu)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	percpu_free_rwsem(&hu->proto_lock);
#endif
}

static inline struct sk_buff *hci_uart_dequeue(struct hci_uart *hu)
{
	struct sk_buff *skb = hu->tx_skb;

	if (!skb) {
		hci_proto_read_lock(hu);

		if (test_bit(HCI_UART_PROTO_READY, &hu->flags))
			skb = hu->proto->dequeue(hu);

		hci_proto_read_unlock(hu);
	} else {
		hu->tx_skb = NULL;
	}

	return skb;
}

/* This may be called in an IRQ context */
int hci_uart_tx_wakeup(struct hci_uart *hu)
{
	/* If acquiring lock fails we assume the tty is being closed because
	 * that is the only time the write lock is acquired. If, however,
	 * at some point in the future the write lock is also acquired in
	 * other situations, then this must be revisited.
	 */
	if (!hci_proto_read_trylock(hu))
		return 0;

	/* proto_lock is locked */
	if (!test_bit(HCI_UART_PROTO_READY, &hu->flags))
		goto no_schedule;

	if (in_interrupt() || in_atomic()) {
		if (test_and_set_bit(HCI_UART_SENDING, &hu->tx_state)) {
			set_bit(HCI_UART_TX_WAKEUP, &hu->tx_state);
			goto no_schedule;
		}
	} else {
		/* NOTE: proto_lock can't be spin lock, because it may
		 * schedule here. Schedule is not allowed while atomic
		 */
		if (down_timeout(&hu->tx_sem,
				 msecs_to_jiffies(SEMWAIT_TIMEOUT)) == -ETIME) {
			pr_warn("%s: Something went wrong with wait\n",
				__func__);
			goto no_schedule;
		}
		/* semaphore is locked */
		if (test_and_set_bit(HCI_UART_SENDING, &hu->tx_state)) {
			set_bit(HCI_UART_TX_WAKEUP, &hu->tx_state);
			up(&hu->tx_sem);
			goto no_schedule;
		}
		up(&hu->tx_sem);
	}

	BT_DBG("");

	schedule_work(&hu->write_work);

no_schedule:
	hci_proto_read_unlock(hu);

	return 0;
}

static void hci_uart_write_work(struct work_struct *work)
{
	struct hci_uart *hu = container_of(work, struct hci_uart, write_work);
	struct tty_struct *tty = hu->tty;
	struct hci_dev *hdev = hu->hdev;
	struct sk_buff *skb;

	/* REVISIT: should we cope with bad skbs or ->write() returning
	 * and error value ?
	 */

 restart:
	clear_bit(HCI_UART_TX_WAKEUP, &hu->tx_state);

	while ((skb = hci_uart_dequeue(hu))) {
		int len;

		set_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
		len = tty->ops->write(tty, skb->data, skb->len);
		hdev->stat.byte_tx += len;

		skb_pull(skb, len);
		if (skb->len) {
			hu->tx_skb = skb;
			break;
		}

		hci_uart_tx_complete(hu, bt_cb(skb)->pkt_type);
		kfree_skb(skb);
	}

	if (down_timeout(&hu->tx_sem, msecs_to_jiffies(SEMWAIT_TIMEOUT))) {
		pr_warn("%s: Something went wrong with wait\n", __func__);
		goto restart;
	}
	/* semaphore is locked */
	if (test_bit(HCI_UART_TX_WAKEUP, &hu->tx_state)) {
		up(&hu->tx_sem);
		goto restart;
	}

	clear_bit(HCI_UART_SENDING, &hu->tx_state);
	up(&hu->tx_sem);
	return;
}

/* ------- Interface to HCI layer ------ */
/* Initialize device */
static int hci_uart_open(struct hci_dev *hdev)
{
	BT_DBG("%s %p", hdev->name, hdev);

	/* Undo clearing this from hci_uart_close() */
	hdev->flush = hci_uart_flush;

#if HCI_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	set_bit(HCI_RUNNING, &hdev->flags);
#endif

#ifdef BTCOEX
	rtk_btcoex_open(hdev);
#endif

	return 0;
}

/* static void hci_flush_sync(struct hci_dev *hdev)
 * {
 * #if HCI_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
 * 	u8 buf[2] = { 0, 0 };
 * 	struct sk_buff *skb;
 * 
 * 	BT_INFO("hci flush sync");
 * 
 * 	set_bit(HCI_INIT, &hdev->flags);
 * 	skb = __hci_cmd_sync(hdev, 0xfc19, 2, buf, msecs_to_jiffies(2000));
 * 	clear_bit(HCI_INIT, &hdev->flags);
 * 
 * 	if (IS_ERR(skb)) {
 * 		BT_ERR("command 0xfc19 tx failed (%ld)\n", PTR_ERR(skb));
 * 		return;
 * 	}
 * 
 * 	if (skb->len == 1)
 * 		BT_INFO("hci flush sync status %u", skb->data[0]);
 * 
 * 	kfree_skb(skb);
 * #endif
 * }
 */

static int __hci_uart_flush(struct hci_dev *hdev, u8 sync)
{
	struct hci_uart *hu = GET_DRV_DATA(hdev);	//(struct hci_uart *) hdev->driver_data;
	struct tty_struct *tty = hu->tty;

	BT_INFO("%s: hdev %p tty %p", __func__, hdev, tty);

	/* Make sure all HCI packets has been transmitted */
	/* if (sync && test_bit(HCI_RUNNING, &hdev->flags))
	 * 	hci_flush_sync(hdev);
	 */

	if (hu->tx_skb) {
		kfree_skb(hu->tx_skb);
		hu->tx_skb = NULL;
	}

	/* Flush any pending characters in the driver and discipline. */
	/* tty_ldisc_flush(tty);
	 * tty_driver_flush_buffer(tty);
	 */
	/* Don't flush the tty. Sometime, the hdev is closed abnormally.
	 * There may be cmd complete event in rx buf or the sent ack in tx buf.
	 * tty flush will result in hciX: command 0xXXXX tx timeout
	 */
	/*
	 * The Carlinkit vendor's 2019 rtk_hci_uart module proceeds directly to
	 * the protocol flush here.  Its attach script deliberately transfers
	 * ownership from the early setup process to BlueZ, so waiting as much as
	 * 500 ms for the UART to drain delays the HCI down/up notification that
	 * creates /org/bluez/hci0.  H5 already provides reliable delivery and
	 * acknowledgement for packets queued before this transition.
	 */

	hci_proto_read_lock(hu);

	if (test_bit(HCI_UART_PROTO_READY, &hu->flags))
		hu->proto->flush(hu);

	hci_proto_read_unlock(hu);

	return 0;
}

/* Reset device */
static int hci_uart_flush(struct hci_dev *hdev)
{
	return __hci_uart_flush(hdev, 1);
}

/* Close device */
static int hci_uart_close(struct hci_dev *hdev)
{
	BT_INFO("%s: hdev %p", __func__, hdev);

	/* When in kernel 4.4.0 and greater, the HCI_RUNNING bit is
	 * cleared in hci_dev_do_close(). */
#if HCI_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	if (!test_and_clear_bit(HCI_RUNNING, &hdev->flags))
		return 0;
#else
	if (test_bit(HCI_RUNNING, &hdev->flags))
		BT_ERR("HCI_RUNNING is not cleared before.");
#endif

	if (test_bit(HCI_RUNNING, &hdev->flags))
		__hci_uart_flush(hdev, 0);
	else
		__hci_uart_flush(hdev, 1);

	hdev->flush = NULL;

#ifdef BTCOEX
	rtk_btcoex_close();
#endif

	return 0;
}

/* Send frames from HCI layer */
#if HCI_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
int hci_uart_send_frame(struct sk_buff *skb)
#else
int hci_uart_send_frame(struct hci_dev *hdev, struct sk_buff *skb)
#endif
{
#if HCI_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
	struct hci_dev *hdev = (struct hci_dev *)skb->dev;
#endif
	struct hci_uart *hu;

	if (!hdev) {
		BT_ERR("Frame for unknown device (hdev=NULL)");
		return -ENODEV;
	}

#if HCI_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	if (!test_bit(HCI_RUNNING, &hdev->flags))
		return -EBUSY;
#endif

	hu = GET_DRV_DATA(hdev);	//(struct hci_uart *) hdev->driver_data;

	BT_DBG("%s: type %d len %d", hdev->name, bt_cb(skb)->pkt_type,
	       skb->len);

#ifdef BTCOEX
	if (bt_cb(skb)->pkt_type == HCI_COMMAND_PKT)
		rtk_btcoex_parse_cmd(skb->data, skb->len);
	if (bt_cb(skb)->pkt_type == HCI_ACLDATA_PKT)
		rtk_btcoex_parse_l2cap_data_tx(skb->data, skb->len);
#endif

	hci_proto_read_lock(hu);

	if (!test_bit(HCI_UART_PROTO_READY, &hu->flags)) {
		hci_proto_read_unlock(hu);
		return -EUNATCH;
	}

	hu->proto->enqueue(hu, skb);
	hci_proto_read_unlock(hu);

	hci_uart_tx_wakeup(hu);

	return 0;
}

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
static void hci_uart_destruct(struct hci_dev *hdev)
{
	if (!hdev)
		return;

	BT_DBG("%s", hdev->name);
	kfree(hdev->driver_data);
}
#endif

#if WOBT_NOTIFY
static int hci_uart_async_send(struct hci_uart *hu, u16 opcode,
			       u32 plen, const void *param)
{
	int len = HCI_COMMAND_HDR_SIZE + plen;
	struct hci_command_hdr *hdr;
	struct sk_buff *skb;

	skb = bt_skb_alloc(len, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	hdr = (struct hci_command_hdr *)skb_put(skb, HCI_COMMAND_HDR_SIZE);
	hdr->opcode = cpu_to_le16(opcode);
	hdr->plen   = plen;

	if (plen)
		memcpy(skb_put(skb, plen), param, plen);

	BT_INFO("rtl: skb len %d", skb->len);

	bt_cb(skb)->pkt_type = HCI_COMMAND_PKT;

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
#if HCI_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	bt_cb(skb)->opcode = opcode;
#else
	bt_cb(skb)->hci.opcode = opcode;
#endif
#endif

	/* Stand-alone HCI commands must be flagged as
	 * single-command requests.
	 */
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
#if HCI_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	bt_cb(skb)->req.start = true;
#else

#if HCI_VERSION_CODE < KERNEL_VERSION(4, 5, 0)
	bt_cb(skb)->hci.req_start = true;
#else

	bt_cb(skb)->hci.req_flags |= HCI_REQ_START;
#endif
#endif /* 4.4.0 */
#endif /* 3.10.0 */

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
	hci_uart_send_frame(skb);
#else
	hci_uart_send_frame(hu->hdev, skb);
#endif

	/* hci_proto_read_lock(hu);

	 * if (!test_bit(HCI_UART_PROTO_READY, &hu->flags)) {
	 * 	hci_proto_read_unlock(hu);
	 * 	BT_ERR("rtl send: proto not ready");
	 * 	return -EUNATCH;
	 * }

	 * hu->proto->enqueue(hu, skb);
	 * hci_proto_read_unlock(hu);

	 * hci_uart_tx_wakeup(hu);
	 */

	return 0;
}

static int rtl_read_local_version(struct hci_dev *hdev, u8 *hci_ver,
				  u16 *hci_rev, u16 *lmp_subver)
{
	struct hci_rsp_read_local *ver;
	struct sk_buff *skb;

	skb = __hci_cmd_sync(hdev, 0x1001, 0, NULL, HCI_INIT_TIMEOUT);
	if (IS_ERR(skb)) {
		BT_ERR("rtl: Could not read lmp subversion");
		return PTR_ERR(skb);
	}

	if (skb->len != sizeof(struct hci_rsp_read_local)) {
		BT_ERR("%s: rtl: Local version length mismatch", hdev->name);
		kfree_skb(skb);
		return -EIO;
	}

	ver = (struct hci_rsp_read_local *)skb->data;
	*hci_ver = ver->hci_ver;
	*hci_rev = le16_to_cpu(ver->hci_rev);
	*lmp_subver = le16_to_cpu(ver->lmp_subver);

	kfree_skb(skb);

	return 0;
}

#if RTKBT_TV_POWERON_WHITELIST
static int rtkbt_lookup_le_device_poweron_whitelist(struct hci_dev *hdev)
{
	struct hci_conn_params *p;
	u8 *params;
	int result = 0;
	struct sk_buff *skb;

	hci_dev_lock(hdev);
	list_for_each_entry(p, &hdev->le_conn_params, list) {
#if 0 // for debug message
		BT_INFO("%s(): auto_connect = %d", __FUNCTION__, p->auto_connect);
		BT_INFO("%s(): addr_type = 0x%02x", __FUNCTION__, p->addr_type);
		BT_INFO("%s(): addr=%02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__,
                                p->addr.b[5], p->addr.b[4], p->addr.b[3],
                                p->addr.b[2], p->addr.b[1], p->addr.b[0]);
#endif
		if ( p->auto_connect == HCI_AUTO_CONN_ALWAYS &&
			p->addr_type == ADDR_LE_DEV_PUBLIC ) {

			BT_INFO("%s(): Set RTKBT LE Power-on Whitelist for "
				"%02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__,
                                p->addr.b[5], p->addr.b[4], p->addr.b[3],
                                p->addr.b[2], p->addr.b[1], p->addr.b[0]);

			params = kzalloc(8, GFP_ATOMIC);
			if (!params) {
				BT_ERR("Can't allocate memory for params");
				return -ENOMEM;
			}

			params[0] = 0x00;
			params[1] = p->addr.b[0];
			params[2] = p->addr.b[1];
			params[3] = p->addr.b[2];
			params[4] = p->addr.b[3];
			params[5] = p->addr.b[4];
			params[6] = p->addr.b[5];

			skb = __hci_cmd_sync(hdev, 0xfc7b, 7, params, HCI_INIT_TIMEOUT);
			if (IS_ERR(skb)) {
				BT_ERR("rtl: Command failed for power-on whitelist");
				return PTR_ERR(skb);
			}

			kfree(params);
			kfree_skb(skb);
		}
	}
	hci_dev_unlock(hdev);

	return result;
}
#endif

static int hci_uart_pm_notifier(struct notifier_block *b, unsigned long v, void *d)
{
	int result;
	struct hci_uart *hu = container_of(b, struct hci_uart, pm_notify_block);
	u8 hci_ver = 0;
	u16 hci_rev = 0;
	u16 lmp_subver = 0;
#if WOBT_NOTIFY_BG_SCAN_LE_WHITELIST_ONLY
	u8 params_bg_scan[5] = { 0x60, 0x01, 0x10, 0x00, 0x01 };
#endif
	u8 params_suspend_notify[1] = { 0x01 };

	BT_INFO("%s: %lu", __func__, v);
	switch (v) {
	case PM_SUSPEND_PREPARE:
		BT_INFO("rtl: bt suspending");
#if WOBT_NOTIFY_BG_SCAN_LE_WHITELIST_ONLY
		/* Send set back ground scan parameters to Controller for power-on mode */
		result = hci_uart_async_send(hu, 0xfc7a, 5, params_bg_scan);
		if (result)
			BT_ERR("Realtek bg-scan h5-bt failed");
		/* FIXME: Ensure the above vendor command is sent to Controller
		 * and we received the h5 ack from Controller
		 * */
		 msleep(500);

#endif

#if RTKBT_TV_POWERON_WHITELIST
		result = rtkbt_lookup_le_device_poweron_whitelist(hu->hdev);
		if (result < 0) {
			BT_ERR("rtkbt_lookup_le_device_poweron_whitelist error: %d", result);
		}
#endif

		/* Send host sleep notification to Controller */
		/* skb = __hci_cmd_sync(hu->hdev, 0xfc28, 1, &param,
		 * 		     HCI_INIT_TIMEOUT);
		 * if (IS_ERR(skb)) {
		 * 	BT_ERR("Realtek Suspend h5-bt failed");
		 * 	goto done;
		 * }
		 * kfree_skb(skb);
		 */
		result = hci_uart_async_send(hu, 0xfc28, 1, params_suspend_notify);
		if (result)
			BT_ERR("Realtek suspend h5-bt failed");

		/* FIXME: Ensure the above vendor command is sent to Controller
		 * and we received the h5 ack from Controller
		 * */
		msleep(500);

		break;
	case PM_POST_SUSPEND:
		result = rtl_read_local_version(hu->hdev, &hci_ver, &hci_rev,
						&lmp_subver);
		if (result)
			break;
		BT_INFO("rtl resume: hci ver %u, hci rev %04x, lmp subver %04x",
			hci_ver, hci_rev, lmp_subver);
		break;
	default:
		BT_INFO("Caught msg %lu other than SUSPEND_PREPARE", v);
		break;
	}

	return 0;
}
#endif

/* ------ LDISC part ------ */
/* hci_uart_tty_open
 *
 * Called when line discipline changed to HCI_UART.
 *
 * Arguments:
 *     tty    pointer to tty info structure
 * Return Value:
 *     0 if success, otherwise error code
 */
static int hci_uart_tty_open(struct tty_struct *tty)
{
	struct hci_uart *hu = (void *)tty->disc_data;

	BT_DBG("tty %p", tty);

	/* But nothing ensures disc_data to be NULL. And since ld->ops->open
	 * shall be called only once, we do not need the check at all.
	 * So remove it.
	 *
	 * Note that this is not an issue now, but n_tty will start using the
	 * disc_data pointer and this invalid 'if' would trigger then rendering
	 * TTYs over BT unusable.
	 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
	/* FIXME: This btw is bogus, nothing requires the old ldisc to clear
	 * the pointer
	 */
	if (hu)
		return -EEXIST;
#endif

	/* Error if the tty has no write op instead of leaving an exploitable
	 * hole
	 */
	if (tty->ops->write == NULL)
		return -EOPNOTSUPP;

	if (!(hu = kzalloc(sizeof(struct hci_uart), GFP_KERNEL))) {
		BT_ERR("Can't allocate control structure");
		return -ENFILE;
	}

	tty->disc_data = hu;
	hu->tty = tty;
	tty->receive_room = 65536;

	INIT_WORK(&hu->write_work, hci_uart_write_work);

	hci_proto_init_rwlock(hu);
	sema_init(&hu->tx_sem, 1);

	/* Flush any pending characters in the driver and line discipline. */

	/* FIXME: why is this needed. Note don't use ldisc_ref here as the
	   open path is before the ldisc is referencable */

	if (tty->ldisc->ops->flush_buffer)
		tty->ldisc->ops->flush_buffer(tty);
	tty_driver_flush_buffer(tty);

#if WOBT_NOTIFY
	hu->pm_notify_block.notifier_call = hci_uart_pm_notifier;
	register_pm_notifier(&hu->pm_notify_block);
#endif

	return 0;
}

/* hci_uart_tty_close()
 *
 * Called when the line discipline is changed to something
 * else, the tty is closed, or the tty detects a hangup.
 */
static void hci_uart_tty_close(struct tty_struct *tty)
{
	struct hci_uart *hu = (void *)tty->disc_data;
	struct hci_dev *hdev;

	BT_INFO("%s: tty %p", __func__, tty);

	/* Detach from the tty */
	tty->disc_data = NULL;

	if (!hu)
		return;

	hdev = hu->hdev;
	if (hdev)
		hci_uart_close(hdev);

	if (test_bit(HCI_UART_PROTO_READY, &hu->flags)) {
		hci_proto_write_lock(hu);
		clear_bit(HCI_UART_PROTO_READY, &hu->flags);
		hci_proto_write_unlock(hu);

		cancel_work_sync(&hu->write_work);

		if (hdev) {
			if (test_bit(HCI_UART_REGISTERED, &hu->flags))
				hci_unregister_dev(hdev);
			hci_free_dev(hdev);
		}
		hu->proto->close(hu);
	}
	clear_bit(HCI_UART_PROTO_SET, &hu->flags);

	hci_proto_free_rwlock(hu);
#if WOBT_NOTIFY
	unregister_pm_notifier(&hu->pm_notify_block);
#endif

	kfree(hu);
}

/* hci_uart_tty_wakeup()
 *
 * Callback for transmit wakeup. Called when low level
 * device driver can accept more send data.
 *
 * Arguments:        tty    pointer to associated tty instance data
 * Return Value:    None
 */
static void hci_uart_tty_wakeup(struct tty_struct *tty)
{
	struct hci_uart *hu = (void *)tty->disc_data;

	BT_DBG("");

	if (!hu)
		return;

	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	if (tty != hu->tty)
		return;

	if (test_bit(HCI_UART_PROTO_READY, &hu->flags))
		hci_uart_tx_wakeup(hu);
}

/* hci_uart_tty_receive()
 *
 * Called by tty low level driver when receive data is
 * available.
 *
 * Arguments:  tty          pointer to tty isntance data
 *             data         pointer to received data
 *             flags        pointer to flags for data
 *             count        count of received data in bytes
 *
 * Return Value:    None
 */
static void hci_uart_tty_receive(struct tty_struct *tty, const u8 *data,
				 const u8 *flags, size_t count)
{
	struct hci_uart *hu = (void *)tty->disc_data;
	int (*proto_receive)(struct hci_uart *hu, void *data, int len);

	if (!hu || tty != hu->tty)
		return;

	hci_proto_read_lock(hu);

	if (!test_bit(HCI_UART_PROTO_READY, &hu->flags)) {
		hci_proto_read_unlock(hu);
		return;
	}

	proto_receive = hu->proto->recv;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
	proto_receive(hu, (void *)data, count);
	hci_proto_read_unlock(hu);
#else
	hci_proto_read_unlock(hu);
	/* It does not need a lock here as it is already protected by a mutex in
	 * tty caller
	 */
	proto_receive(hu, (void *)data, count);
#endif

	if (hu->hdev)
		hu->hdev->stat.byte_rx += count;

	tty_unthrottle(tty);
}

static int hci_uart_register_dev(struct hci_uart *hu)
{
	struct hci_dev *hdev;

	BT_INFO("hci_uart_register_dev");

	/* Initialize and register HCI device */
	hdev = hci_alloc_dev();
	if (!hdev) {
		BT_ERR("Can't allocate HCI device");
		return -ENOMEM;
	}

	hu->hdev = hdev;

#if HCI_VERSION_CODE > KERNEL_VERSION(2, 6, 33)
	hdev->bus = HCI_UART;
#else
	hdev->type = HCI_UART;
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
	hci_set_drvdata(hdev, hu);
#else
	hdev->driver_data = hu;
#endif

	hdev->open = hci_uart_open;
	hdev->close = hci_uart_close;
	hdev->flush = hci_uart_flush;
	hdev->send = hci_uart_send_frame;

	/*
	 * rtk_hciattach has already downloaded and configured the controller.
	 * Register hci0 DOWN, as the vendor 3.14 stack did, and let hcid issue
	 * the first real HCIDEVUP.  The automatic 5.10 registration-time open
	 * races the attach helper and makes it close the line discipline.
	 */

	/* NOTE: No hdev->setup setting for Realtek BTUART because
	 * the download procedure is done with rtk_hciattach in userspace
	 * before this function called in hci_uart_set_proto()
	 */

	SET_HCIDEV_DEV(hdev, hu->tty->dev);

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
	hdev->destruct = hci_uart_destruct;
	hdev->owner = THIS_MODULE;
#endif

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
	if (!reset)
		hci_set_quirk(hdev, HCI_QUIRK_NO_RESET);
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	if (test_bit(HCI_UART_RAW_DEVICE, &hu->hdev_flags))
		hci_set_quirk(hdev, HCI_QUIRK_RAW_DEVICE);
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
	if (test_bit(HCI_UART_EXT_CONFIG, &hu->hdev_flags))
		hci_set_quirk(hdev, HCI_QUIRK_EXTERNAL_CONFIG);
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
	if (!test_bit(HCI_UART_RESET_ON_INIT, &hu->hdev_flags))
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
		hci_set_quirk(hdev, HCI_QUIRK_RESET_ON_CLOSE);
#else
		hci_set_quirk(hdev, HCI_QUIRK_NO_RESET);
#endif
#endif

	hci_set_quirk(hdev, HCI_QUIRK_IGNORE_INIT_UNKNOWN_COMMAND);
	hci_set_quirk(hdev, HCI_QUIRK_KEEP_POWERED_AFTER_SETUP);

	/*
	 * Vendor userspace treats successful legacy enumeration as proof that
	 * initialization is complete.  Do not expose the transient HCI_SETUP
	 * state through HCIGETDEVLIST/HCIGETDEVINFO.
	 */
	hci_set_quirk(hdev, HCI_QUIRK_HIDE_DURING_SETUP);

#if HCI_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
	hci_set_quirk(hdev, HCI_QUIRK_SIMULTANEOUS_DISCOVERY);
#endif

	if (hci_register_dev(hdev) < 0) {
		BT_ERR("Can't register HCI device");
		hci_free_dev(hdev);
		return -ENODEV;
	}

	set_bit(HCI_UART_REGISTERED, &hu->flags);

#ifdef BTCOEX
	rtk_btcoex_probe(hdev);
#endif

	return 0;
}

static int hci_uart_set_proto(struct hci_uart *hu, int id)
{
	struct hci_uart_proto *p;
	unsigned int waited = 0;
	int err;

	p = hci_uart_get_proto(id);
	if (!p)
		return -EPROTONOSUPPORT;

	err = p->open(hu);
	if (err)
		return err;

	hu->proto = p;
	set_bit(HCI_UART_PROTO_READY, &hu->flags);

	/* Initialize and register HCI dev */
	err = hci_uart_register_dev(hu);
	if (err) {
		clear_bit(HCI_UART_PROTO_READY, &hu->flags);
		p->close(hu);
		return err;
	}

	/*
	 * hci_register_dev() queues hci_power_on() and returns while HCI_SETUP
	 * is still set.  Do not let the legacy rtk_hciattach command finish in
	 * that transient state.  HCI_QUIRK_HIDE_DURING_SETUP simultaneously
	 * keeps HCIGETDEVLIST/HCIGETDEVINFO from exposing the half-ready hci0.
	 */
	while (hci_dev_test_flag(hu->hdev, HCI_SETUP) &&
	       waited < HCI_UART_SETUP_WAIT_MS) {
		msleep(20);
		waited += 20;
	}

	if (hci_dev_test_flag(hu->hdev, HCI_SETUP)) {
		BT_ERR("%s: controller setup timed out", hu->hdev->name);
		return -ETIMEDOUT;
	}

	return 0;
}

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
static int hci_uart_set_flags(struct hci_uart *hu, unsigned long flags)
{
	/* TODO: Add HCI_UART_INIT_PENDING, HCI_UART_VND_DETECT check  */
	unsigned long valid_flags = BIT(HCI_UART_RAW_DEVICE) |
				    BIT(HCI_UART_RESET_ON_INIT) |
				    BIT(HCI_UART_CREATE_AMP) |
				    BIT(HCI_UART_EXT_CONFIG);

	if (flags & ~valid_flags)
		return -EINVAL;

	hu->hdev_flags = flags;

	return 0;
}
#endif

/* hci_uart_tty_ioctl()
 *
 *    Process IOCTL system call for the tty device.
 *
 * Arguments:
 *
 *    tty        pointer to tty instance data
 *    file       pointer to open file object for device
 *    cmd        IOCTL command code
 *    arg        argument for IOCTL call (cmd dependent)
 *
 * Return Value:    Command dependent
 */
static int hci_uart_tty_ioctl(struct tty_struct *tty, unsigned int cmd,
			      unsigned long arg)
{
	struct hci_uart *hu = (void *)tty->disc_data;
	int err = 0;

	BT_DBG("");

	/* Verify the status of the device */
	if (!hu)
		return -EBADF;

	switch (cmd) {
	case HCIUARTSETPROTO:
		if (!test_and_set_bit(HCI_UART_PROTO_SET, &hu->flags)) {
			err = hci_uart_set_proto(hu, arg);
			if (err) {
				clear_bit(HCI_UART_PROTO_SET, &hu->flags);
				return err;
			}
		} else
			return -EBUSY;
		break;

	case HCIUARTGETPROTO:
		if (test_bit(HCI_UART_PROTO_SET, &hu->flags))
			return hu->proto->id;
		return -EUNATCH;

	case HCIUARTGETDEVICE:
		if (test_bit(HCI_UART_REGISTERED, &hu->flags))
			return hu->hdev->id;
		return -EUNATCH;

	case HCIUARTSETFLAGS:
		if (test_bit(HCI_UART_PROTO_SET, &hu->flags))
			return -EBUSY;
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
		err = hci_uart_set_flags(hu, arg);
		if (err)
			return err;
#else
		hu->hdev_flags = arg;
#endif
		break;

	case HCIUARTGETFLAGS:
		return hu->hdev_flags;

	default:
		err = n_tty_ioctl_helper(tty, cmd, arg);
		break;
	};

	return err;
}

/*
 * We don't provide read/write/poll interface for user space.
 */
// static ssize_t hci_uart_tty_write(struct tty_struct *tty, struct file *file,
//				  const unsigned char *data, size_t count)

//static ssize_t hci_uart_tty_write(struct tty_struct *tty, struct file *file,
//				  const unsigned char *data, size_t count)
				  

static ssize_t hci_uart_tty_read(struct tty_struct *tty, struct file *file,
				 u8 *buf, size_t nr, void **cookie,
				 unsigned long offset)
{
		return 0;
}
//static ssize_t hci_uart_tty_read(struct tty_struct *tty, struct file *file,
//				 const unsigned char  *data, size_t count)
//{
//	return 0;
//}

static ssize_t hci_uart_tty_write(struct tty_struct *tty, struct file *file,
				  const unsigned char *data, size_t count)
{
	return 0;
}

static unsigned int hci_uart_tty_poll(struct tty_struct *tty,
				      struct file *filp, poll_table * wait)
{
	return 0;
}

static struct tty_ldisc_ops hci_uart_ldisc;

static int __init hci_uart_init(void)
{
	int err;

	BT_INFO("HCI UART driver ver %s", VERSION);

	/* Register the tty discipline */

	memset(&hci_uart_ldisc, 0, sizeof(hci_uart_ldisc));
	hci_uart_ldisc.num = N_HCI;
	hci_uart_ldisc.name = "n_hci";
	hci_uart_ldisc.open = hci_uart_tty_open;
	hci_uart_ldisc.close = hci_uart_tty_close;
	hci_uart_ldisc.read = hci_uart_tty_read;
	hci_uart_ldisc.write = hci_uart_tty_write;
	hci_uart_ldisc.ioctl = hci_uart_tty_ioctl;
	hci_uart_ldisc.compat_ioctl = hci_uart_tty_ioctl;
	hci_uart_ldisc.poll = hci_uart_tty_poll;
	hci_uart_ldisc.receive_buf = hci_uart_tty_receive;
	hci_uart_ldisc.write_wakeup = hci_uart_tty_wakeup;
	hci_uart_ldisc.owner = THIS_MODULE;

	if ((err = tty_register_ldisc(&hci_uart_ldisc))) {
		BT_ERR("HCI line discipline registration failed. (%d)", err);
		return err;
	}
#ifdef CONFIG_BT_HCIUART_RTKH4
	h4_init();
#endif
	/* Add realtek h5 support */
	h5_init();

#ifdef BTCOEX
	rtk_btcoex_init();
#endif

	return 0;
}

static void __exit hci_uart_exit(void)
{
#ifdef CONFIG_BT_HCIUART_RTKH4
	h4_deinit();
#endif
	h5_deinit();

	/* Release tty registration of line discipline */
	tty_unregister_ldisc(&hci_uart_ldisc);

#ifdef BTCOEX
	rtk_btcoex_exit();
#endif
}

module_init(hci_uart_init);
module_exit(hci_uart_exit);

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
module_param(reset, bool, 0644);
MODULE_PARM_DESC(reset, "Send HCI reset command on initialization");
#endif

MODULE_AUTHOR("Marcel Holtmann <marcel@holtmann.org>");
MODULE_DESCRIPTION("Bluetooth HCI UART driver ver " VERSION);
MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL");
MODULE_ALIAS_LDISC(N_HCI);

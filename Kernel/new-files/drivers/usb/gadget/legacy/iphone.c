/* Reconstructed Carlinkit iPhone/iAP2 USB gadget (vendor iphone.c). */
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/hid.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/usb/audio.h>
#include <linux/usb/composite.h>
#include <linux/workqueue.h>
#include <linux/unaligned.h>

#include "g_zero.h"

#define DRIVER_NAME "iPhone"
#define IAP2_RECV_SIZE 4096
#define IAP2_IOCTL_CLOSE 26881
#define IAP2_HID_BUFFER_SIZE 768

USB_GADGET_COMPOSITE_OPTIONS();

static char *serial = "41de1cfe2e099bec5932187411378387653dbc63";
static int req_buf_size = 200;
static int req_count = 256;
static int audio_buf_size = 48000;
static bool loopdefault;
static unsigned int autoresume = 5;
static unsigned int max_autoresume;
static unsigned int autoresume_interval_ms;

module_param(serial, charp, S_IRUGO);
module_param(req_buf_size, int, S_IRUGO);
module_param(req_count, int, S_IRUGO);
module_param(audio_buf_size, int, S_IRUGO);
module_param(loopdefault, bool, S_IRUGO | S_IWUSR);
module_param(autoresume, uint, S_IRUGO);
module_param(max_autoresume, uint, S_IRUGO);
module_param(autoresume_interval_ms, uint, S_IRUGO);

static struct usb_zero_options gzero_options = {
	.pattern = 0,
	.isoc_interval = GZERO_ISOC_INTERVAL,
	.isoc_maxpacket = GZERO_ISOC_MAXPACKET,
	.bulk_buflen = GZERO_BULK_BUFLEN,
	.qlen = GZERO_QLEN,
};
module_param_named(buflen, gzero_options.bulk_buflen, uint, 0);
module_param_named(pattern, gzero_options.pattern, uint, S_IRUGO | S_IWUSR);
module_param_named(isoc_interval, gzero_options.isoc_interval, uint, S_IRUGO | S_IWUSR);
module_param_named(isoc_maxpacket, gzero_options.isoc_maxpacket, uint, S_IRUGO | S_IWUSR);
module_param_named(isoc_mult, gzero_options.isoc_mult, uint, S_IRUGO | S_IWUSR);
module_param_named(isoc_maxburst, gzero_options.isoc_maxburst, uint, S_IRUGO | S_IWUSR);
module_param_named(qlen, gzero_options.qlen, uint, S_IRUGO | S_IWUSR);

static char serial_string[64];
enum {
	STR_MANUFACTURER, STR_PRODUCT, STR_SERIAL, STR_RESERVED,
	STR_PTP, STR_IPOD, STR_PTP_AMD, STR_PTP_AMD_AUE,
};
static struct usb_string strings_dev[] = {
	[STR_MANUFACTURER].s = "Apple Inc.",
	[STR_PRODUCT].s = DRIVER_NAME,
	[STR_SERIAL].s = serial_string,
	[STR_RESERVED].s = "Reserved",
	[STR_PTP].s = "PTP",
	[STR_IPOD].s = "iPod USB Interface",
	[STR_PTP_AMD].s = "PTP + Apple Mobile Device",
	[STR_PTP_AMD_AUE].s = "PTP + Apple Mobile Device + Apple USB Ethernet",
	{ },
};
static struct usb_gadget_strings stringtab_dev = {
	.language = 0x0409, .strings = strings_dev,
};
static struct usb_gadget_strings *dev_strings[] = { &stringtab_dev, NULL };
static struct usb_device_descriptor device_desc = {
	.bLength = sizeof(device_desc), .bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = cpu_to_le16(0x0200), .idVendor = cpu_to_le16(0x05ac),
	.idProduct = cpu_to_le16(0x12a8), .bcdDevice = cpu_to_le16(0x0903),
	.bNumConfigurations = 4,
};

struct f_audio_buf {
	u8 *buf;
	unsigned int actual;
	struct list_head list;
};
struct f_iphone {
	struct usb_function function;       /* original offset 0 */
	struct usb_composite_dev *cdev;     /* 88 */
	struct usb_ep *audio_out_ep;        /* 92 */
	struct usb_ep *hid_in_ep;           /* 96 */
	struct usb_request *hid_req;        /* 100 */
	spinlock_t lock;                    /* 104 */
	struct f_audio_buf *copy_buf;       /* 108 */
	struct work_struct playback_work;   /* 112 */
	struct list_head play_queue;        /* 128 */
	struct list_head controls;          /* 136 */
	u8 set_cmd;                         /* 144 */
	struct usb_audio_control *set_con;  /* 148 */
};
static inline struct f_iphone *func_to_iphone(struct usb_function *f)
{
	return container_of(f, struct f_iphone, function);
}

static struct usb_interface_descriptor ac_interface_desc = {
	.bLength = USB_DT_INTERFACE_SIZE, .bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceClass = USB_CLASS_AUDIO,
	.bInterfaceSubClass = USB_SUBCLASS_AUDIOCONTROL,
};
DECLARE_UAC_AC_HEADER_DESCRIPTOR(1);
static struct uac1_ac_header_descriptor_1 ac_header_desc = {
	.bLength = sizeof(ac_header_desc), .bDescriptorType = USB_DT_CS_INTERFACE,
	.bDescriptorSubtype = UAC_HEADER, .bcdADC = cpu_to_le16(0x0100),
	.wTotalLength = cpu_to_le16(0x001e), .bInCollection = 1,
	.baInterfaceNr = { 1 },
};
static struct uac_input_terminal_descriptor input_terminal_desc = {
	.bLength = UAC_DT_INPUT_TERMINAL_SIZE,
	.bDescriptorType = USB_DT_CS_INTERFACE,
	.bDescriptorSubtype = UAC_INPUT_TERMINAL, .bTerminalID = 1,
	.wTerminalType = cpu_to_le16(UAC_TERMINAL_STREAMING),
	.bAssocTerminal = 2, .bNrChannels = 2,
	.wChannelConfig = cpu_to_le16(3),
};
static struct uac1_output_terminal_descriptor output_terminal_desc = {
	.bLength = UAC_DT_OUTPUT_TERMINAL_SIZE,
	.bDescriptorType = USB_DT_CS_INTERFACE,
	.bDescriptorSubtype = UAC_OUTPUT_TERMINAL, .bTerminalID = 2,
	.wTerminalType = cpu_to_le16(UAC_TERMINAL_STREAMING),
	.bAssocTerminal = 1, .bSourceID = 1,
};
static struct usb_interface_descriptor as_interface_alt_0_desc = {
	.bLength = USB_DT_INTERFACE_SIZE, .bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceClass = USB_CLASS_AUDIO,
	.bInterfaceSubClass = USB_SUBCLASS_AUDIOSTREAMING,
};
static struct usb_interface_descriptor as_interface_alt_1_desc = {
	.bLength = USB_DT_INTERFACE_SIZE, .bDescriptorType = USB_DT_INTERFACE,
	.bAlternateSetting = 1, .bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_AUDIO,
	.bInterfaceSubClass = USB_SUBCLASS_AUDIOSTREAMING,
};
static struct uac1_as_header_descriptor as_header_desc = {
	.bLength = UAC_DT_AS_HEADER_SIZE, .bDescriptorType = USB_DT_CS_INTERFACE,
	.bDescriptorSubtype = UAC_AS_GENERAL, .bTerminalLink = 2, .bDelay = 1,
	.wFormatTag = cpu_to_le16(UAC_FORMAT_TYPE_I_PCM),
};
DECLARE_UAC_FORMAT_TYPE_I_DISCRETE_DESC(9);
static struct uac_format_type_i_discrete_descriptor_9 as_type_i_desc = {
	.bLength = UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(9),
	.bDescriptorType = USB_DT_CS_INTERFACE,
	.bDescriptorSubtype = UAC_FORMAT_TYPE, .bFormatType = UAC_FORMAT_TYPE_I,
	.bNrChannels = 2, .bSubframeSize = 2, .bBitResolution = 16,
	.bSamFreqType = 9,
};
static struct usb_endpoint_descriptor as_out_ep_desc = {
	.bLength = USB_DT_ENDPOINT_AUDIO_SIZE, .bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_OUT, .bmAttributes = USB_ENDPOINT_XFER_ISOC,
	.wMaxPacketSize = cpu_to_le16(192), .bInterval = 4,
};
static struct uac_iso_endpoint_descriptor as_iso_out_desc = {
	.bLength = UAC_ISO_ENDPOINT_DESC_SIZE,
	.bDescriptorType = USB_DT_CS_ENDPOINT,
	.bDescriptorSubtype = UAC_EP_GENERAL, .bmAttributes = 1,
};
static struct usb_interface_descriptor hidg_interface_desc = {
	.bLength = USB_DT_INTERFACE_SIZE, .bDescriptorType = USB_DT_INTERFACE,
	.bNumEndpoints = 1, .bInterfaceClass = USB_CLASS_HID,
};
static struct hid_descriptor hidg_desc = {
	.bLength = sizeof(hidg_desc), .bDescriptorType = HID_DT_HID,
	.bcdHID = cpu_to_le16(0x0111), .bNumDescriptors = 1,
	.rpt_desc = { .bDescriptorType = HID_DT_REPORT,
		.wDescriptorLength = cpu_to_le16(208) },
};
static struct usb_endpoint_descriptor hidg_in_ep_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE, .bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN, .bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = cpu_to_le16(64), .bInterval = 1,
};

static const u8 hid_report_descriptor[208] = {
	0x09,0x01,0xa1,0x01,0x75,0x08,0x26,0x80,0x00,0x15,0x00,
	0x09,0x01,0x85,0x01,0x95,0x05,0x82,0x02,0x01,
	0x09,0x01,0x85,0x02,0x95,0x09,0x82,0x02,0x01,
	0x09,0x01,0x85,0x03,0x95,0x0d,0x82,0x02,0x01,
	0x09,0x01,0x85,0x04,0x95,0x11,0x82,0x02,0x01,
	0x09,0x01,0x85,0x05,0x95,0x19,0x82,0x02,0x01,
	0x09,0x01,0x85,0x06,0x95,0x31,0x82,0x02,0x01,
	0x09,0x01,0x85,0x07,0x95,0x5f,0x82,0x02,0x01,
	0x09,0x01,0x85,0x08,0x95,0xc1,0x82,0x02,0x01,
	0x09,0x01,0x85,0x09,0x96,0x01,0x01,0x82,0x02,0x01,
	0x09,0x01,0x85,0x0a,0x96,0x81,0x01,0x82,0x02,0x01,
	0x09,0x01,0x85,0x0b,0x96,0x01,0x02,0x82,0x02,0x01,
	0x09,0x01,0x85,0x0c,0x96,0xff,0x02,0x82,0x02,0x01,
	0x09,0x01,0x85,0x0d,0x95,0x05,0x92,0x02,0x01,
	0x09,0x01,0x85,0x0e,0x95,0x09,0x92,0x02,0x01,
	0x09,0x01,0x85,0x0f,0x95,0x0d,0x92,0x02,0x01,
	0x09,0x01,0x85,0x10,0x95,0x11,0x92,0x02,0x01,
	0x09,0x01,0x85,0x11,0x95,0x19,0x92,0x02,0x01,
	0x09,0x01,0x85,0x12,0x95,0x31,0x92,0x02,0x01,
	0x09,0x01,0x85,0x13,0x95,0x5f,0x92,0x02,0x01,
	0x09,0x01,0x85,0x14,0x95,0xc1,0x92,0x02,0x01,
	0x09,0x01,0x85,0x15,0x95,0xff,0x92,0x02,0x01,0xc0,
};
static struct usb_descriptor_header *f_audio_desc[] = {
	(struct usb_descriptor_header *)&ac_interface_desc,
	(struct usb_descriptor_header *)&ac_header_desc,
	(struct usb_descriptor_header *)&input_terminal_desc,
	(struct usb_descriptor_header *)&output_terminal_desc,
	(struct usb_descriptor_header *)&as_interface_alt_0_desc,
	(struct usb_descriptor_header *)&as_interface_alt_1_desc,
	(struct usb_descriptor_header *)&as_header_desc,
	(struct usb_descriptor_header *)&as_type_i_desc,
	(struct usb_descriptor_header *)&as_out_ep_desc,
	(struct usb_descriptor_header *)&as_iso_out_desc,
	(struct usb_descriptor_header *)&hidg_interface_desc,
	(struct usb_descriptor_header *)&hidg_desc,
	(struct usb_descriptor_header *)&hidg_in_ep_desc, NULL,
};
static const unsigned int sample_rates[] = {
	8000,11025,12000,16000,22050,24000,32000,44100,48000,
};
static const unsigned int hid_report_size_table[] = {
	5,9,13,17,25,49,95,193,257,385,513,767,
};
static const u8 iap2_detect_packet[] = { 0xff,0x55,0x02,0x00,0xee,0x10 };

static DEFINE_MUTEX(iap2_lock);
static u8 iap2_recv_buf[IAP2_RECV_SIZE];
static unsigned int iap2_recv_buf_size;
static struct f_iphone *iap2_audio;
static bool iap2_closed, iap2_send_finish;
static struct miscdevice iap2_device;

static struct f_audio_buf *f_audio_buffer_alloc(unsigned int size)
{
	struct f_audio_buf *b = kzalloc(sizeof(*b), GFP_ATOMIC);
	if (!b)
		return NULL;
	b->buf = kzalloc(size, GFP_ATOMIC);
	if (!b->buf) { kfree(b); return NULL; }
	INIT_LIST_HEAD(&b->list);
	return b;
}
static void f_audio_buffer_free(struct f_audio_buf *b)
{
	if (b) { kfree(b->buf); kfree(b); }
}
static void f_audio_playback_work(struct work_struct *work)
{
	struct f_iphone *a = container_of(work, struct f_iphone, playback_work);
	struct f_audio_buf *b = NULL;
	unsigned long flags;
	spin_lock_irqsave(&a->lock, flags);
	if (!list_empty(&a->play_queue)) {
		b = list_first_entry(&a->play_queue, struct f_audio_buf, list);
		list_del_init(&b->list);
	}
	spin_unlock_irqrestore(&a->lock, flags);
	f_audio_buffer_free(b);
}
static void f_hid_complete(struct usb_ep *ep, struct usb_request *req)
{
	iap2_send_finish = true;
	if (iap2_audio)
		dev_err(&iap2_audio->cdev->gadget->dev,
			"ep %s hid req actual len: %u\n", ep->name, req->actual);
}
static void f_hid_set_report_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_iphone *a = req->context;
	u8 *data = req->buf;
	unsigned int len = req->actual;
	if (!a || !len)
		return;
	dev_err(&a->cdev->gadget->dev,
		"ep %s hid set report req actual len: %u\n", ep->name, len);
	if (!data[1] && len >= 6 && len - 2 < ((data[4] << 8) | data[5]) &&
	    memcmp(data + 2, iap2_detect_packet, sizeof(iap2_detect_packet))) {
		dev_err(&a->cdev->gadget->dev,
			"iap2_data length error: %02x-%02x, %02x-%02x, drop it!!!\n",
			data[0],data[1],data[4],data[5]);
		return;
	}
	put_unaligned_le16(len, data);
	mutex_lock(&iap2_lock);
	if (iap2_recv_buf_size + len > sizeof(iap2_recv_buf))
		dev_err(&a->cdev->gadget->dev,
			"f_hid_recv_iap2_data buffer not enough:%u--%u\n",
			iap2_recv_buf_size,len);
	else {
		memcpy(iap2_recv_buf + iap2_recv_buf_size, data, len);
		iap2_recv_buf_size += len;
	}
	mutex_unlock(&iap2_lock);
}
static void f_audio_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_iphone *a = req->context;
	struct f_audio_buf *b;
	unsigned long flags;
	u32 value = 0;
	if (!a || req->status)
		return;
	if (ep != a->audio_out_ep) {
		if (a->set_con) {
			memcpy(&value, req->buf, min_t(unsigned int,req->length,4));
			if (a->set_con->set)
				a->set_con->set(a->set_con,a->set_cmd,(u16)value);
			a->set_con = NULL;
		}
		return;
	}
	b = a->copy_buf;
	if (!b)
		return;
	if (audio_buf_size - b->actual < req->actual) {
		spin_lock_irqsave(&a->lock, flags);
		list_add_tail(&b->list, &a->play_queue);
		spin_unlock_irqrestore(&a->lock, flags);
		queue_work(system_wq, &a->playback_work);
		b = f_audio_buffer_alloc(audio_buf_size);
		if (!b)
			return;
	}
	memcpy(b->buf + b->actual, req->buf, req->actual);
	b->actual += req->actual;
	a->copy_buf = b;
	usb_ep_queue(ep, req, GFP_ATOMIC);
}

static int f_audio_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct f_iphone *a = func_to_iphone(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request *req = cdev->req;
	u16 val = le16_to_cpu(ctrl->wValue), idx = le16_to_cpu(ctrl->wIndex);
	unsigned int len = le16_to_cpu(ctrl->wLength), dtype = val >> 8;
	int response = -EOPNOTSUPP;

	switch (ctrl->bRequestType) {
	case USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE:
		if (ctrl->bRequest != USB_REQ_GET_DESCRIPTOR)
			return -EOPNOTSUPP;
		if (dtype == HID_DT_HID) {
			len = min_t(unsigned int,len,sizeof(hidg_desc));
			memcpy(req->buf,&hidg_desc,len);
		} else if (dtype == HID_DT_REPORT) {
			len = min_t(unsigned int,len,sizeof(hid_report_descriptor));
			memcpy(req->buf,hid_report_descriptor,len);
		} else return -EOPNOTSUPP;
		break;
	case USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE:
		if (idx > 1) return -EOPNOTSUPP;
		len = min_t(unsigned int,len,sizeof(response));
		req->context = a; req->complete = f_audio_complete;
		memcpy(req->buf,&response,len);
		break;
	case USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE:
		if (idx > 1) {
			if (idx != 2 || ctrl->bRequest != 9) return -EOPNOTSUPP;
			if (len) { req->context = a; req->complete = f_hid_set_report_complete; }
		} else {
			a->set_cmd = ctrl->bRequest & 0xf;
			req->context = a; req->complete = f_audio_complete;
		}
		break;
	case USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_ENDPOINT:
		if (ctrl->bRequest != 1) return -EOPNOTSUPP;
		break;
	case USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_ENDPOINT:
		return -EOPNOTSUPP;
	default:
		dev_err(&cdev->gadget->dev,
			"invalid control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType,ctrl->bRequest,val,idx,len);
		return -EOPNOTSUPP;
	}
	req->zero = 0; req->length = len;
	response = usb_ep_queue(cdev->gadget->ep0,req,GFP_ATOMIC);
	if (response < 0)
		dev_err(&cdev->gadget->dev,"audio response on err %d\n",response);
	return response;
}
static int f_audio_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_iphone *a = func_to_iphone(f);
	int ret = 0;
	if (intf == as_interface_alt_0_desc.bInterfaceNumber) {
		if (alt == 1) {
			ret = config_ep_by_speed(a->cdev->gadget, f,
						 a->audio_out_ep);
			if (!ret)
				ret = usb_ep_enable(a->audio_out_ep);
			if (!ret)
				a->audio_out_ep->driver_data = a;
		}
		return ret;
	}
	if (intf != hidg_interface_desc.bInterfaceNumber)
		return 0;
	ret = config_ep_by_speed(a->cdev->gadget,f,a->hid_in_ep);
	if (ret) dev_err(&a->cdev->gadget->dev,"config_ep_by_speed FAILED!\n");
	else { ret = usb_ep_enable(a->hid_in_ep); if (!ret) a->hid_in_ep->driver_data = a; }
	iap2_audio = a;
	dev_err(&a->cdev->gadget->dev,"Set iPod Hid Interface Alt\n");
	return ret;
}
static int f_audio_get_alt(struct usb_function *f, unsigned intf)
{
	dev_err(&f->config->cdev->gadget->dev,"Get iPod Hid Interface %u\n",intf);
	return -EOPNOTSUPP;
}
static void f_audio_disable(struct usb_function *f)
{
	dev_err(&f->config->cdev->gadget->dev,"iPod Function Disabled!!!\n");
}

static int f_audio_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_iphone *a = func_to_iphone(f);
	struct usb_composite_dev *cdev = c->cdev;
	struct usb_endpoint_descriptor reserved_hid_desc = hidg_in_ep_desc;
	struct usb_ep *reserved_hid_ep;
	unsigned int i,rate;
	int id,ret = -ENODEV;
	for (i=0;i<ARRAY_SIZE(sample_rates);i++) {
		rate=sample_rates[i]; as_type_i_desc.tSamFreq[i][0]=rate;
		as_type_i_desc.tSamFreq[i][1]=rate>>8; as_type_i_desc.tSamFreq[i][2]=rate>>16;
	}
	id=usb_interface_id(c,f); if (id<0) return id; ac_interface_desc.bInterfaceNumber=id;
	id=usb_interface_id(c,f); if (id<0) return id;
	as_interface_alt_0_desc.bInterfaceNumber=id; as_interface_alt_1_desc.bInterfaceNumber=id;
	a->audio_out_ep=usb_ep_autoconfig(cdev->gadget,&as_out_ep_desc);
	if (!a->audio_out_ep) return -ENODEV;
	a->audio_out_ep->driver_data=cdev;
	id=usb_interface_id(c,f); if (id<0) { ret=id; goto fail; }
	hidg_interface_desc.bInterfaceNumber=id;
	/*
	 * The vendor layout deliberately reserves the first matching interrupt
	 * IN endpoint.  Use a descriptor copy: 5.10 autoconfiguration mutates
	 * the supplied descriptor and claims the endpoint immediately.
	 */
	reserved_hid_ep=usb_ep_autoconfig(cdev->gadget,&reserved_hid_desc);
	if (!reserved_hid_ep) goto fail;
	reserved_hid_ep->driver_data=cdev;
	a->hid_in_ep=usb_ep_autoconfig(cdev->gadget,&hidg_in_ep_desc);
	if (!a->hid_in_ep) goto fail;
	a->hid_in_ep->driver_data=cdev;
	a->hid_req=usb_ep_alloc_request(a->hid_in_ep,GFP_ATOMIC);
	if (!a->hid_req) { ret=-ENOMEM; goto fail; }
	a->hid_req->buf=kzalloc(IAP2_HID_BUFFER_SIZE,GFP_ATOMIC);
	if (!a->hid_req->buf) { ret=-ENOMEM; goto fail; }
	ret=usb_assign_descriptors(f,f_audio_desc,f_audio_desc,NULL,NULL);
	if (ret) goto fail;
	ret=misc_register(&iap2_device);
	if (!ret) return 0;
	usb_free_all_descriptors(f);
fail:
	if (a->hid_req) { kfree(a->hid_req->buf); usb_ep_free_request(a->hid_in_ep,a->hid_req); a->hid_req=NULL; }
	if (a->hid_in_ep) a->hid_in_ep->driver_data=NULL;
	if (a->audio_out_ep) a->audio_out_ep->driver_data=NULL;
	return ret;
}
static void f_audio_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_iphone *a=func_to_iphone(f);
	iap2_audio=NULL; iap2_closed=true; misc_deregister(&iap2_device);
	cancel_work_sync(&a->playback_work);
	if (a->audio_out_ep) usb_ep_disable(a->audio_out_ep);
	if (a->hid_in_ep) {
		usb_ep_disable(a->hid_in_ep);
		if (a->hid_req) { usb_ep_dequeue(a->hid_in_ep,a->hid_req); kfree(a->hid_req->buf); usb_ep_free_request(a->hid_in_ep,a->hid_req); }
	}
	f_audio_buffer_free(a->copy_buf); usb_free_all_descriptors(f); kfree(a);
}

static int iap2_open(struct inode *inode, struct file *file)
{
	if (!iap2_audio) return -ENODEV;
	iap2_closed=false; pr_info("ipod_iap2_open\n"); return 0;
}
static ssize_t iap2_read(struct file *file,char __user *buf,size_t count,loff_t *ppos)
{
	unsigned int plen,n;
	for (;;) { if (!iap2_audio || iap2_closed) return -ENODEV; if (READ_ONCE(iap2_recv_buf_size)) break; msleep(5); }
	mutex_lock(&iap2_lock);
	if (iap2_recv_buf_size<2) { mutex_unlock(&iap2_lock); return -EIO; }
	plen=get_unaligned_le16(iap2_recv_buf);
	if (plen<2 || plen>iap2_recv_buf_size) { mutex_unlock(&iap2_lock); return -EIO; }
	n=min_t(unsigned int,count,plen-2);
	if (copy_to_user(buf,iap2_recv_buf+2,n)) { mutex_unlock(&iap2_lock); return -EFAULT; }
	iap2_recv_buf_size-=plen;
	if (iap2_recv_buf_size) memmove(iap2_recv_buf,iap2_recv_buf+plen,iap2_recv_buf_size);
	mutex_unlock(&iap2_lock); return n;
}
static ssize_t iap2_write(struct file *file,const char __user *buf,size_t count,loff_t *ppos)
{
	struct f_iphone *a=iap2_audio; struct usb_request *req; unsigned int report; int ret;
	if (!a || !(req=a->hid_req)) return -ENODEV;
	for (report=0;report<ARRAY_SIZE(hid_report_size_table);report++) if (count<hid_report_size_table[report]) break;
	if (report==ARRAY_SIZE(hid_report_size_table)) { dev_err(&a->cdev->gadget->dev,"f_hid_send_iap2_data length too long: %u\n",(unsigned int)count); return -ENOMEM; }
	req->status=0; req->zero=0; req->length=hid_report_size_table[report]+1;
	req->context=a; req->complete=f_hid_complete; memset(req->buf,0,req->length);
	if (copy_from_user((u8 *)req->buf+2,buf,count)) return -EFAULT;
	((u8 *)req->buf)[0]=report+1; iap2_send_finish=false;
	ret=usb_ep_queue(a->hid_in_ep,req,GFP_ATOMIC);
	if (ret) { dev_err(&a->cdev->gadget->dev,"%s queue req: %d\n",a->hid_in_ep->name,ret); return ret; }
	while (iap2_audio && !iap2_closed && !iap2_send_finish) msleep(5);
	return count;
}
static long iap2_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	pr_info("ipod_iap2_ioctl %u\n",cmd); if (cmd!=IAP2_IOCTL_CLOSE) return -EINVAL; iap2_closed=true; return 0;
}
static int iap2_flush(struct file *file,fl_owner_t id) { pr_info("ipod_iap2_flush\n"); return 0; }
static int iap2_release(struct inode *inode,struct file *file)
{
	pr_info("ipod_iap2_release\n"); mutex_lock(&iap2_lock); iap2_recv_buf_size=0; mutex_unlock(&iap2_lock); return 0;
}
static const struct file_operations iap2_fops = {
	.owner=THIS_MODULE,.open=iap2_open,.read=iap2_read,.write=iap2_write,
	.unlocked_ioctl=iap2_ioctl,.flush=iap2_flush,.release=iap2_release,.llseek=noop_llseek,
};
static struct miscdevice iap2_device = {
	.minor=MISC_DYNAMIC_MINOR,.name="ipod_iap2",.fops=&iap2_fops,
};

static int audio_do_config(struct usb_configuration *c)
{
	struct f_iphone *a=kzalloc(sizeof(*a),GFP_KERNEL); int ret;
	if (!a) return 0;
	a->function.name="g_audio"; a->function.strings=dev_strings;
	a->function.bind=f_audio_bind; a->function.unbind=f_audio_unbind;
	a->function.set_alt=f_audio_set_alt; a->function.get_alt=f_audio_get_alt;
	a->function.setup=f_audio_setup; a->function.disable=f_audio_disable; a->cdev=c->cdev;
	spin_lock_init(&a->lock); INIT_WORK(&a->playback_work,f_audio_playback_work);
	INIT_LIST_HEAD(&a->play_queue); INIT_LIST_HEAD(&a->controls);
	ret=usb_add_function(c,&a->function);
	if (ret) kfree(a); else dev_info(&c->cdev->gadget->dev,"audio_buf_size %d, req_buf_size %d, req_count %d\n",audio_buf_size,req_buf_size,req_count);
	return 0;
}

static struct delayed_work start_work;
static void iph_start_work(struct work_struct *work)
{
	struct file *fp; loff_t pos=0; char one='1';
	pr_info("f_iphone: do_hnp\n");
	fp=filp_open("/sys/bus/platform/devices/ci_hdrc.1/inputs/b_bus_req",O_WRONLY,0644);
	if (IS_ERR(fp)) { pr_err("android_usb: open hnp file failed\n"); return; }
	kernel_write(fp, &one, 1, &pos); filp_close(fp,NULL);
}
static int ss_config_setup(struct usb_configuration *c,const struct usb_ctrlrequest *ctrl)
{
	struct usb_composite_dev *d=c->cdev; struct usb_request *req=d->req;
	u16 val=le16_to_cpu(ctrl->wValue),idx=le16_to_cpu(ctrl->wIndex),lenv=le16_to_cpu(ctrl->wLength); int len;
	dev_err(&d->gadget->dev,"iph_ctrlrequest %02x.%02x v%04x i%04x l%u\n",ctrl->bRequestType,ctrl->bRequest,val,idx,lenv);
	if (ctrl->bRequestType==0x40 && ctrl->bRequest==0x51) {
		if (val<=1) queue_delayed_work(system_wq,&start_work,msecs_to_jiffies(50)); len=0;
	} else if (ctrl->bRequestType==0xc0 && ctrl->bRequest==0x53) {
		((u8 *)req->buf)[0]=1; ((u8 *)req->buf)[1]=0; len=4;
	} else {
		dev_err(&d->gadget->dev,"unknown class-specific control req %02x.%02x v%04x i%04x l%u\n",ctrl->bRequestType,ctrl->bRequest,val,idx,lenv); return -EOPNOTSUPP;
	}
	req->zero=0; req->length=len; len=usb_ep_queue(d->gadget->ep0,req,GFP_ATOMIC);
	if (len<0) dev_err(&d->gadget->dev,"ss_config_setup setup response queue error\n");
	dev_err(&d->gadget->dev,"iph_ctrlrequest value: %d\n",len); return len;
}
#define IPHONE_CONFIG(name,value) { .label=name,.setup=ss_config_setup, \
	.bConfigurationValue=value,.bmAttributes=USB_CONFIG_ATT_SELFPOWER,.MaxPower=500 }
static struct usb_configuration ptp_driver=IPHONE_CONFIG("ptp",1);
static struct usb_configuration ipod_driver=IPHONE_CONFIG("ipod",2);
static struct usb_configuration ptp_amd_driver=IPHONE_CONFIG("ptp+apple md",3);
static struct usb_configuration ptp_amd_aue_driver=IPHONE_CONFIG("ptp+apple md+apple ue",4);

static struct timer_list autoresume_timer;
static struct usb_composite_dev *autoresume_cdev;
static unsigned int autoresume_step_ms;
static struct usb_function_instance *func_inst_ss,*func_inst_lb;
static struct usb_function *func_ss,*func_ss2,*func_lb,*func_lb2;
static void zero_autoresume(struct timer_list *timer)
{
	struct usb_composite_dev *d=autoresume_cdev;
	if (d->config && d->gadget->speed!=USB_SPEED_UNKNOWN) usb_gadget_wakeup(d->gadget);
}
static void zero_suspend(struct usb_composite_dev *d)
{
	if (d->gadget->speed==USB_SPEED_UNKNOWN || !autoresume) return;
	if (max_autoresume && autoresume_step_ms>max_autoresume*1000) autoresume_step_ms=autoresume*1000;
	mod_timer(&autoresume_timer,jiffies+msecs_to_jiffies(autoresume_step_ms));
	autoresume_step_ms+=autoresume_interval_ms;
}
static void zero_resume(struct usb_composite_dev *d) { timer_delete(&autoresume_timer); }

static int zero_bind(struct usb_composite_dev *d)
{
	struct f_ss_opts *opts; int ret;
	INIT_DELAYED_WORK(&start_work,iph_start_work);
	strncpy(serial_string,serial,sizeof(serial_string)-1); serial_string[sizeof(serial_string)-1]=0;
	ret=usb_string_ids_tab(d,strings_dev); if (ret<0) return ret;
	device_desc.iManufacturer=strings_dev[STR_MANUFACTURER].id;
	device_desc.iProduct=strings_dev[STR_PRODUCT].id; device_desc.iSerialNumber=strings_dev[STR_SERIAL].id;
	autoresume_cdev=d; timer_setup(&autoresume_timer,zero_autoresume,0);
	func_inst_ss=usb_get_function_instance("PTP"); if (IS_ERR(func_inst_ss)) return PTR_ERR(func_inst_ss);
	func_inst_lb=usb_get_function_instance("PTP");
	if (IS_ERR(func_inst_lb)) { ret=PTR_ERR(func_inst_lb); func_inst_lb=NULL; goto err_inst_ss; }
	opts=container_of(func_inst_ss,struct f_ss_opts,func_inst);
	opts->pattern=gzero_options.pattern; opts->isoc_interval=gzero_options.isoc_interval;
	opts->isoc_maxpacket=gzero_options.isoc_maxpacket; opts->isoc_mult=gzero_options.isoc_mult;
	opts->isoc_maxburst=gzero_options.isoc_maxburst; opts->bulk_buflen=gzero_options.bulk_buflen;
	func_ss=usb_get_function(func_inst_ss);
	if (IS_ERR(func_ss)) { ret=PTR_ERR(func_ss); func_ss=NULL; goto err_inst_lb; }
	func_ss2=usb_get_function(func_inst_lb);
	if (IS_ERR(func_ss2)) { ret=PTR_ERR(func_ss2); func_ss2=NULL; goto err_func_ss; }
	/*
	 * iphone.c inherited the next two writes from Gadget Zero's Loopback
	 * instance, but changed the requested function to PTP.  In the vendor
	 * f_ptp layout those offsets are pattern/isoc_interval, so the literal
	 * writes produce the invalid values 4096/32.  Configure this as the PTP
	 * instance it actually is; otherwise every PTP configuration stalls.
	 */
	opts=container_of(func_inst_lb,struct f_ss_opts,func_inst);
	opts->pattern=gzero_options.pattern; opts->isoc_interval=gzero_options.isoc_interval;
	opts->isoc_maxpacket=gzero_options.isoc_maxpacket; opts->isoc_mult=gzero_options.isoc_mult;
	opts->isoc_maxburst=gzero_options.isoc_maxburst; opts->bulk_buflen=gzero_options.bulk_buflen;
	func_lb=NULL; func_lb2=usb_get_function(func_inst_lb);
	if (IS_ERR(func_lb2)) { ret=PTR_ERR(func_lb2); func_lb2=NULL; goto err_func_ss2; }
	ptp_driver.iConfiguration=strings_dev[STR_PTP].id;
	ipod_driver.iConfiguration=strings_dev[STR_IPOD].id;
	ptp_amd_driver.iConfiguration=strings_dev[STR_PTP_AMD].id;
	ptp_amd_aue_driver.iConfiguration=strings_dev[STR_PTP_AMD_AUE].id;
	usb_add_config_only(d,&ptp_driver); ret=usb_add_function(&ptp_driver,func_ss); if (ret) goto err_func_lb2;
	usb_ep_autoconfig_reset(d->gadget); ret=usb_add_config(d,&ipod_driver,audio_do_config); if (ret) goto err_func_lb2;
	usb_add_config_only(d,&ptp_amd_driver); usb_ep_autoconfig_reset(d->gadget);
	ret=usb_add_function(&ptp_amd_driver,func_ss2); if (ret) goto err_func_lb2;
	usb_add_config_only(d,&ptp_amd_aue_driver); usb_ep_autoconfig_reset(d->gadget);
	ret=usb_add_function(&ptp_amd_aue_driver,func_lb2); if (ret) goto err_func_lb2;
	usb_ep_autoconfig_reset(d->gadget); usb_composite_overwrite_options(d,&coverwrite);
	dev_info(&d->gadget->dev,"%s, version: Kevin Shi 2020\n",DRIVER_NAME);
	dev_info(&d->gadget->dev,"%s, serial: %s\n",DRIVER_NAME,serial); return 0;
err_func_lb2: if (func_lb2) { usb_put_function(func_lb2); func_lb2=NULL; }
err_func_ss2: if (func_ss2) { usb_put_function(func_ss2); func_ss2=NULL; }
err_func_ss: if (func_ss) { usb_put_function(func_ss); func_ss=NULL; }
err_inst_lb: usb_put_function_instance(func_inst_lb); func_inst_lb=NULL;
err_inst_ss: usb_put_function_instance(func_inst_ss); func_inst_ss=NULL; return ret;
}
static int zero_unbind(struct usb_composite_dev *d)
{
	cancel_delayed_work_sync(&start_work); timer_delete_sync(&autoresume_timer);
	if (!IS_ERR_OR_NULL(func_ss)) usb_put_function(func_ss);
	if (!IS_ERR_OR_NULL(func_ss2)) usb_put_function(func_ss2);
	usb_put_function_instance(func_inst_ss);
	if (!IS_ERR_OR_NULL(func_lb)) usb_put_function(func_lb);
	if (!IS_ERR_OR_NULL(func_lb2)) usb_put_function(func_lb2);
	usb_put_function_instance(func_inst_lb);
	func_ss=func_ss2=func_lb=func_lb2=NULL; func_inst_ss=func_inst_lb=NULL; return 0;
}
static struct usb_composite_driver zero_driver = {
	.name="zero",.dev=&device_desc,.strings=dev_strings,.max_speed=USB_SPEED_HIGH,
	.bind=zero_bind,.unbind=zero_unbind,.suspend=zero_suspend,.resume=zero_resume,
};
static int __init iphone_init(void) { return usb_composite_probe(&zero_driver); }
module_init(iphone_init);
static void __exit iphone_cleanup(void) { usb_composite_unregister(&zero_driver); }
module_exit(iphone_cleanup);
MODULE_AUTHOR("Reconstructed from Carlinkit vendor module");
MODULE_DESCRIPTION("Reconstructed Carlinkit iPhone/iAP2 USB gadget");
MODULE_LICENSE("GPL");

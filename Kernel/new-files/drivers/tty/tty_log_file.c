// SPDX-License-Identifier: GPL-2.0
/* RAM-backed console compatible with the original Carlinkit kernel. */

#include <linux/console.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>

#define TTY_LOG_FILE_NAME "ttyLogFile"
#define TTY_LOG_FILE_PATH "/tmp/ttyLog"
#define TTY_LOG_BUFFER_SIZE (64 * 1024)
#define TTY_LOG_WRITE_SIZE 4096

static char tty_log_buffer[TTY_LOG_BUFFER_SIZE];
static unsigned int tty_log_head, tty_log_tail, tty_log_count;
static DEFINE_SPINLOCK(tty_log_lock);
static struct tty_driver *tty_log_driver;
static struct tty_port tty_log_port;

static const struct tty_port_operations tty_log_port_ops = {
};

static void tty_log_store(const char *buf, unsigned int len)
{
	unsigned long flags;

	spin_lock_irqsave(&tty_log_lock, flags);
	while (len--) {
		tty_log_buffer[tty_log_head++] = *buf++;
		if (tty_log_head == TTY_LOG_BUFFER_SIZE)
			tty_log_head = 0;
		if (tty_log_count == TTY_LOG_BUFFER_SIZE) {
			if (++tty_log_tail == TTY_LOG_BUFFER_SIZE)
				tty_log_tail = 0;
		} else {
			tty_log_count++;
		}
	}
	spin_unlock_irqrestore(&tty_log_lock, flags);
}

static unsigned int tty_log_take(char *buf, unsigned int size)
{
	unsigned long flags;
	unsigned int len = 0;

	spin_lock_irqsave(&tty_log_lock, flags);
	while (len < size && tty_log_count) {
		buf[len++] = tty_log_buffer[tty_log_tail++];
		if (tty_log_tail == TTY_LOG_BUFFER_SIZE)
			tty_log_tail = 0;
		tty_log_count--;
	}
	spin_unlock_irqrestore(&tty_log_lock, flags);
	return len;
}

static void tty_log_console_write(struct console *console, const char *buf,
				  unsigned int len)
{
	tty_log_store(buf, len);
}

static struct tty_driver *tty_log_console_device(struct console *console,
						 int *index)
{
	*index = 0;
	return tty_log_driver;
}

static int tty_log_console_setup(struct console *console, char *options)
{
	console->index = 0;
	return 0;
}

static struct console tty_log_console = {
	.name = TTY_LOG_FILE_NAME,
	.write = tty_log_console_write,
	.device = tty_log_console_device,
	.setup = tty_log_console_setup,
	.flags = CON_PRINTBUFFER,
	.index = -1,
};

static int __init tty_log_console_init(void)
{
	register_console(&tty_log_console);
	return 0;
}
console_initcall(tty_log_console_init);

static int tty_log_open(struct tty_struct *tty, struct file *file)
{
	return tty_port_open(&tty_log_port, tty, file);
}

static void tty_log_close(struct tty_struct *tty, struct file *file)
{
	tty_port_close(&tty_log_port, tty, file);
}

static ssize_t tty_log_write(struct tty_struct *tty, const u8 *buf,
			     size_t count)
{
	tty_log_store(buf, count);
	return count;
}

static unsigned int tty_log_write_room(struct tty_struct *tty)
{
	return TTY_LOG_BUFFER_SIZE;
}

static const struct tty_operations tty_log_ops = {
	.open = tty_log_open,
	.close = tty_log_close,
	.write = tty_log_write,
	.write_room = tty_log_write_room,
};

static int tty_log_file_thread(void *unused)
{
	struct file *file = NULL;
	char *buf = kmalloc(TTY_LOG_WRITE_SIZE, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;

	while (!kthread_should_stop()) {
		unsigned int len;
		ssize_t written;

		if (!file) {
			/*
			 * Do not create the file here.  The vendor init script mounts
			 * tmpfs on /tmp and then creates /tmp/ttyLog.  Creating it as
			 * soon as the JFFS2 root is mounted can race that tmpfs mount,
			 * leaving this thread writing forever to a hidden JFFS2 inode.
			 * Keep the early output in the ring until userspace has created
			 * the file on the final /tmp mount.
			 */
			file = filp_open(TTY_LOG_FILE_PATH,
					 O_WRONLY | O_APPEND, 0);
			if (IS_ERR(file)) {
				file = NULL;
				msleep_interruptible(200);
				continue;
			}
		}

		len = tty_log_take(buf, TTY_LOG_WRITE_SIZE);
		if (!len) {
			msleep_interruptible(100);
			continue;
		}

		written = kernel_write(file, buf, len, &file->f_pos);
		if (written < 0) {
			filp_close(file, NULL);
			file = NULL;
		}
	}

	if (file)
		filp_close(file, NULL);
	kfree(buf);
	return 0;
}

static int __init tty_log_driver_init(void)
{
	struct device *dev;
	int ret;

	tty_log_driver = tty_alloc_driver(1, TTY_DRIVER_REAL_RAW |
					       TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(tty_log_driver))
		return PTR_ERR(tty_log_driver);

	tty_log_driver->driver_name = "tty-log-file";
	tty_log_driver->name = TTY_LOG_FILE_NAME;
	tty_log_driver->major = 0;
	tty_log_driver->minor_start = 0;
	tty_log_driver->type = TTY_DRIVER_TYPE_SYSTEM;
	tty_log_driver->subtype = SYSTEM_TYPE_CONSOLE;
	tty_log_driver->init_termios = tty_std_termios;
	tty_set_operations(tty_log_driver, &tty_log_ops);

	tty_port_init(&tty_log_port);
	tty_log_port.ops = &tty_log_port_ops;
	ret = tty_register_driver(tty_log_driver);
	if (ret) {
		tty_driver_kref_put(tty_log_driver);
		tty_log_driver = NULL;
		return ret;
	}

	dev = tty_port_register_device(&tty_log_port, tty_log_driver, 0, NULL);
	if (IS_ERR(dev))
		pr_warn("tty-log-file: could not register tty device: %ld\n",
			PTR_ERR(dev));

	if (IS_ERR(kthread_run(tty_log_file_thread, NULL, "ttyLogFileThread")))
		pr_warn("tty-log-file: could not start log writer thread\n");
	return 0;
}
device_initcall(tty_log_driver_init);

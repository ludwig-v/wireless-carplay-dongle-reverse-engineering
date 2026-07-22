/*
 * Carlinkit Android composite gadget reconstruction.
 *
 * The vendor gadget exposes the traditional android_usb sysfs interface and
 * provides two selectable functions: a vendor iAP2 bulk transport and CDC
 * NCM.  The iAP2 descriptor and userspace behaviour were reconstructed from
 * the vendor 3.14.52 zImage.
 */

#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/usb/composite.h>

#define DRIVER_NAME             "android_usb"
#define IAP2_NAME               "android_iap2"
#define IAP2_BUFSIZE            0x1000
#define IAP2_TX_REQS            2
#define IAP2_RX_REQS            2

struct iap2_request {
	struct usb_request *req;
	struct list_head list;
	size_t offset;
};

struct iap2_dev {
	struct usb_composite_dev *cdev;
	spinlock_t lock;
	struct mutex io_lock;
	wait_queue_head_t rx_wait;
	wait_queue_head_t tx_wait;
	struct list_head tx_idle;
	struct list_head rx_idle;
	struct list_head rx_done;
	struct usb_ep *in_ep;
	struct usb_ep *out_ep;
	bool online;
	bool error;
	struct usb_function function;
};

static struct iap2_dev iap2_device;

static struct usb_interface_assoc_descriptor iap2_iad = {
	.bLength = sizeof(iap2_iad),
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 0,
	.bInterfaceCount = 1,
	.bFunctionClass = USB_CLASS_VENDOR_SPEC,
	.bFunctionSubClass = 0xf0,
};

static struct usb_interface_descriptor iap2_intf = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass = 0xf0,
};

static struct usb_endpoint_descriptor iap2_fs_in = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor iap2_fs_out = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_OUT,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor iap2_hs_in = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = cpu_to_le16(512),
};

static struct usb_endpoint_descriptor iap2_hs_out = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_OUT,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = cpu_to_le16(512),
};

static struct usb_descriptor_header *iap2_fs_descs[] = {
	(struct usb_descriptor_header *)&iap2_iad,
	(struct usb_descriptor_header *)&iap2_intf,
	(struct usb_descriptor_header *)&iap2_fs_in,
	(struct usb_descriptor_header *)&iap2_fs_out,
	NULL,
};

static struct usb_descriptor_header *iap2_hs_descs[] = {
	(struct usb_descriptor_header *)&iap2_iad,
	(struct usb_descriptor_header *)&iap2_intf,
	(struct usb_descriptor_header *)&iap2_hs_in,
	(struct usb_descriptor_header *)&iap2_hs_out,
	NULL,
};

static struct usb_string iap2_strings[] = {
	{ .s = "android_iap2" },
	{ },
};

static struct usb_gadget_strings iap2_string_table = {
	.language = 0x0409,
	.strings = iap2_strings,
};

static struct usb_gadget_strings *iap2_string_tables[] = {
	&iap2_string_table,
	NULL,
};

static struct iap2_request *iap2_req_alloc(struct usb_ep *ep, gfp_t flags)
{
	struct iap2_request *r;

	r = kzalloc(sizeof(*r), flags);
	if (!r)
		return NULL;
	r->req = usb_ep_alloc_request(ep, flags);
	if (!r->req)
		goto fail;
	r->req->buf = kmalloc(IAP2_BUFSIZE, flags);
	if (!r->req->buf)
		goto fail_req;
	r->req->context = r;
	INIT_LIST_HEAD(&r->list);
	return r;

fail_req:
	usb_ep_free_request(ep, r->req);
fail:
	kfree(r);
	return NULL;
}

static void iap2_req_free(struct usb_ep *ep, struct iap2_request *r)
{
	if (!r)
		return;
	kfree(r->req->buf);
	usb_ep_free_request(ep, r->req);
	kfree(r);
}

static void iap2_free_list(struct usb_ep *ep, struct list_head *head);

static void iap2_tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct iap2_request *r = req->context;
	struct iap2_dev *dev = ep->driver_data;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);
	if (req->status && req->status != -ESHUTDOWN)
		dev->error = true;
	list_add_tail(&r->list, &dev->tx_idle);
	spin_unlock_irqrestore(&dev->lock, flags);
	wake_up_interruptible(&dev->tx_wait);
}

static void iap2_rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct iap2_request *r = req->context;
	struct iap2_dev *dev = ep->driver_data;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);
	r->offset = 0;
	if (!req->status && req->actual)
		list_add_tail(&r->list, &dev->rx_done);
	else {
		if (req->status && req->status != -ESHUTDOWN)
			dev->error = true;
		list_add_tail(&r->list, &dev->rx_idle);
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	wake_up_interruptible(&dev->rx_wait);
}

static void iap2_queue_rx(struct iap2_dev *dev)
{
	struct iap2_request *r;
	unsigned long flags;
	int ret;

	for (;;) {
		spin_lock_irqsave(&dev->lock, flags);
		if (!dev->online || list_empty(&dev->rx_idle)) {
			spin_unlock_irqrestore(&dev->lock, flags);
			break;
		}
		r = list_first_entry(&dev->rx_idle, struct iap2_request, list);
		list_del_init(&r->list);
		spin_unlock_irqrestore(&dev->lock, flags);

		r->req->length = IAP2_BUFSIZE;
		ret = usb_ep_queue(dev->out_ep, r->req, GFP_ATOMIC);
		if (ret) {
			spin_lock_irqsave(&dev->lock, flags);
			list_add_tail(&r->list, &dev->rx_idle);
			dev->error = true;
			spin_unlock_irqrestore(&dev->lock, flags);
			break;
		}
	}
}

static int iap2_open(struct inode *inode, struct file *file)
{
	file->private_data = &iap2_device;
	return 0;
}

static int iap2_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static unsigned int iap2_poll(struct file *file, poll_table *wait)
{
	struct iap2_dev *dev = file->private_data;
	unsigned long flags;
	unsigned int mask = 0;

	poll_wait(file, &dev->rx_wait, wait);
	poll_wait(file, &dev->tx_wait, wait);
	spin_lock_irqsave(&dev->lock, flags);
	if (!list_empty(&dev->tx_idle))
		mask |= POLLOUT | POLLWRNORM;
	if (!list_empty(&dev->rx_done) || dev->error)
		mask |= POLLIN | POLLRDNORM;
	if (!dev->online)
		mask |= POLLHUP;
	spin_unlock_irqrestore(&dev->lock, flags);
	return mask;
}

static ssize_t iap2_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	struct iap2_dev *dev = file->private_data;
	struct iap2_request *r;
	unsigned long flags;
	ssize_t written = 0;
	size_t n;
	int ret;

	if (!count)
		return -EINVAL;

	if (mutex_lock_interruptible(&dev->io_lock))
		return -ERESTARTSYS;

	while (count) {
		spin_lock_irqsave(&dev->lock, flags);
		if (!dev->online) {
			spin_unlock_irqrestore(&dev->lock, flags);
			ret = written ? written : -ENODEV;
			goto out;
		}
		if (dev->error) {
			dev->error = false;
			spin_unlock_irqrestore(&dev->lock, flags);
			ret = written ? written : -EIO;
			goto out;
		}
		if (list_empty(&dev->tx_idle)) {
			spin_unlock_irqrestore(&dev->lock, flags);
			if (file->f_flags & O_NONBLOCK) {
				ret = written ? written : -EAGAIN;
				goto out;
			}
			ret = wait_event_interruptible(dev->tx_wait,
				!dev->online || dev->error ||
				!list_empty(&dev->tx_idle));
			if (ret) {
				ret = written ? written : ret;
				goto out;
			}
			continue;
		}

		r = list_first_entry(&dev->tx_idle, struct iap2_request, list);
		list_del_init(&r->list);
		spin_unlock_irqrestore(&dev->lock, flags);

		n = min_t(size_t, count, IAP2_BUFSIZE);
		if (copy_from_user(r->req->buf, buf, n)) {
			spin_lock_irqsave(&dev->lock, flags);
			list_add_tail(&r->list, &dev->tx_idle);
			spin_unlock_irqrestore(&dev->lock, flags);
			ret = written ? written : -EFAULT;
			goto out;
		}
		r->req->length = n;
		r->req->complete = iap2_tx_complete;
		ret = usb_ep_queue(dev->in_ep, r->req, GFP_KERNEL);
		if (ret) {
			spin_lock_irqsave(&dev->lock, flags);
			list_add_tail(&r->list, &dev->tx_idle);
			spin_unlock_irqrestore(&dev->lock, flags);
			ret = written ? written : ret;
			goto out;
		}
		buf += n;
		count -= n;
		written += n;
	}
	ret = written;
out:
	mutex_unlock(&dev->io_lock);
	return ret;
}

static ssize_t iap2_read(struct file *file, char __user *buf, size_t count,
			 loff_t *ppos)
{
	struct iap2_dev *dev = file->private_data;
	struct iap2_request *r;
	unsigned long flags;
	ssize_t copied = 0;
	size_t n;
	int ret;

	if (!count)
		return -EINVAL;
	if (mutex_lock_interruptible(&dev->io_lock))
		return -ERESTARTSYS;

	for (;;) {
		spin_lock_irqsave(&dev->lock, flags);
		if (!dev->online) {
			spin_unlock_irqrestore(&dev->lock, flags);
			ret = -ENODEV;
			goto out;
		}
		if (dev->error) {
			dev->error = false;
			spin_unlock_irqrestore(&dev->lock, flags);
			ret = -EIO;
			goto out;
		}
		if (!list_empty(&dev->rx_done))
			break;
		spin_unlock_irqrestore(&dev->lock, flags);
		if (file->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto out;
		}
		ret = wait_event_interruptible(dev->rx_wait,
			!dev->online || dev->error || !list_empty(&dev->rx_done));
		if (ret)
			goto out;
	}

	r = list_first_entry(&dev->rx_done, struct iap2_request, list);
	list_del_init(&r->list);
	spin_unlock_irqrestore(&dev->lock, flags);

	while (count && r->offset < r->req->actual) {
		n = min_t(size_t, count, r->req->actual - r->offset);
		if (copy_to_user(buf, r->req->buf + r->offset, n)) {
			ret = copied ? copied : -EFAULT;
			goto put_back;
		}
		r->offset += n;
		buf += n;
		count -= n;
		copied += n;
	}

	if (r->offset < r->req->actual) {
		spin_lock_irqsave(&dev->lock, flags);
		list_add(&r->list, &dev->rx_done);
		spin_unlock_irqrestore(&dev->lock, flags);
	} else {
		spin_lock_irqsave(&dev->lock, flags);
		list_add_tail(&r->list, &dev->rx_idle);
		spin_unlock_irqrestore(&dev->lock, flags);
		iap2_queue_rx(dev);
	}
	ret = copied;
	goto out;

put_back:
	spin_lock_irqsave(&dev->lock, flags);
	list_add(&r->list, &dev->rx_done);
	spin_unlock_irqrestore(&dev->lock, flags);
out:
	mutex_unlock(&dev->io_lock);
	return ret;
}

static const struct file_operations iap2_fops = {
	.owner = THIS_MODULE,
	.open = iap2_open,
	.release = iap2_release,
	.read = iap2_read,
	.write = iap2_write,
	.poll = iap2_poll,
	.llseek = noop_llseek,
};

static struct miscdevice iap2_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = IAP2_NAME,
	.fops = &iap2_fops,
};

static void iap2_disable_function(struct usb_function *f)
{
	struct iap2_dev *dev = container_of(f, struct iap2_dev, function);
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);
	dev->online = false;
	spin_unlock_irqrestore(&dev->lock, flags);
	if (dev->in_ep && dev->in_ep->driver_data)
		usb_ep_disable(dev->in_ep);
	if (dev->out_ep && dev->out_ep->driver_data)
		usb_ep_disable(dev->out_ep);
	wake_up_interruptible(&dev->rx_wait);
	wake_up_interruptible(&dev->tx_wait);
}

static int iap2_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct iap2_dev *dev = container_of(f, struct iap2_dev, function);
	int ret;

	if (alt)
		return -ENOTSUPP;
	iap2_disable_function(f);

	ret = config_ep_by_speed(dev->cdev->gadget, f, dev->in_ep);
	if (ret)
		return ret;
	ret = usb_ep_enable(dev->in_ep);
	if (ret)
		return ret;
	dev->in_ep->driver_data = dev;

	ret = config_ep_by_speed(dev->cdev->gadget, f, dev->out_ep);
	if (ret)
		goto fail_in;
	ret = usb_ep_enable(dev->out_ep);
	if (ret)
		goto fail_in;
	dev->out_ep->driver_data = dev;

	dev->online = true;
	dev->error = false;
	iap2_queue_rx(dev);
	wake_up_interruptible(&dev->rx_wait);
	wake_up_interruptible(&dev->tx_wait);
	pr_info("iAP2 Using interface %x\n", intf);
	return 0;

fail_in:
	usb_ep_disable(dev->in_ep);
	return ret;
}

static int iap2_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct iap2_dev *dev = container_of(f, struct iap2_dev, function);
	struct usb_composite_dev *cdev = c->cdev;
	struct iap2_request *r;
	int id, i, ret;

	dev->cdev = cdev;
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	iap2_iad.bFirstInterface = id;
	iap2_intf.bInterfaceNumber = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	iap2_strings[0].id = id;
	iap2_intf.iInterface = id;

	dev->in_ep = usb_ep_autoconfig(cdev->gadget, &iap2_fs_in);
	if (!dev->in_ep)
		return -ENODEV;
	dev->out_ep = usb_ep_autoconfig(cdev->gadget, &iap2_fs_out);
	if (!dev->out_ep)
		return -ENODEV;
	iap2_hs_in.bEndpointAddress = iap2_fs_in.bEndpointAddress;
	iap2_hs_out.bEndpointAddress = iap2_fs_out.bEndpointAddress;

	ret = usb_assign_descriptors(f, iap2_fs_descs, iap2_hs_descs, NULL, NULL);
	if (ret)
		return ret;

	for (i = 0; i < IAP2_TX_REQS; i++) {
		r = iap2_req_alloc(dev->in_ep, GFP_KERNEL);
		if (!r) {
			ret = -ENOMEM;
			goto fail_requests;
		}
		r->req->complete = iap2_tx_complete;
		list_add_tail(&r->list, &dev->tx_idle);
	}
	for (i = 0; i < IAP2_RX_REQS; i++) {
		r = iap2_req_alloc(dev->out_ep, GFP_KERNEL);
		if (!r) {
			ret = -ENOMEM;
			goto fail_requests;
		}
		r->req->complete = iap2_rx_complete;
		list_add_tail(&r->list, &dev->rx_idle);
	}
	return 0;

fail_requests:
	iap2_free_list(dev->in_ep, &dev->tx_idle);
	iap2_free_list(dev->out_ep, &dev->rx_idle);
	usb_free_all_descriptors(f);
	return ret;
}

static void iap2_free_list(struct usb_ep *ep, struct list_head *head)
{
	struct iap2_request *r, *tmp;

	list_for_each_entry_safe(r, tmp, head, list) {
		list_del(&r->list);
		iap2_req_free(ep, r);
	}
}

static void iap2_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct iap2_dev *dev = container_of(f, struct iap2_dev, function);

	iap2_disable_function(f);
	iap2_free_list(dev->in_ep, &dev->tx_idle);
	iap2_free_list(dev->out_ep, &dev->rx_idle);
	iap2_free_list(dev->out_ep, &dev->rx_done);
	usb_free_all_descriptors(f);
	dev->in_ep = NULL;
	dev->out_ep = NULL;
}

/* Traditional Android sysfs composite wrapper. */
struct carlinkit_android {
	struct usb_composite_dev *cdev;
	struct class *class;
	struct device *dev;
	struct mutex lock;
	bool enabled;
	bool use_iap2;
	bool use_ncm;
	struct usb_function_instance *ncm_inst;
	struct usb_function *ncm_func;
};

static struct carlinkit_android android_dev;

static char manufacturer[256] = "Android";
static char product[256] = "Android";
static char serial[256] = "0123456789ABCDEF";

static struct usb_string android_strings[] = {
	{ .s = manufacturer },
	{ .s = product },
	{ .s = serial },
	{ },
};

static struct usb_gadget_strings android_string_table = {
	.language = 0x0409,
	.strings = android_strings,
};

static struct usb_gadget_strings *android_string_tables[] = {
	&android_string_table,
	NULL,
};

static struct usb_device_descriptor android_desc = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = cpu_to_le16(0x0200),
	.bDeviceClass = USB_CLASS_PER_INTERFACE,
	.idVendor = cpu_to_le16(0x18d1),
	.idProduct = cpu_to_le16(0x0001),
	.bcdDevice = cpu_to_le16(0xffff),
	.bNumConfigurations = 1,
};

static int android_config_bind(struct usb_configuration *c)
{
	int ret;

	if (android_dev.use_iap2) {
		ret = usb_add_function(c, &iap2_device.function);
		if (ret)
			return ret;
	}
	if (android_dev.use_ncm) {
		ret = usb_add_function(c, android_dev.ncm_func);
		if (ret)
			return ret;
	}
	return 0;
}

static struct usb_configuration android_config = {
	.label = "android",
	.bConfigurationValue = 1,
	.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.MaxPower = 500,
};

static ssize_t functions_show(struct device *d,
			      struct device_attribute *attr, char *buf)
{
	if (android_dev.use_iap2 && android_dev.use_ncm)
		return sprintf(buf, "iap2,ncm\n");
	if (android_dev.use_iap2)
		return sprintf(buf, "iap2\n");
	if (android_dev.use_ncm)
		return sprintf(buf, "ncm\n");
	return sprintf(buf, "\n");
}

static ssize_t functions_store(struct device *d,
			       struct device_attribute *attr,
			       const char *buf, size_t size)
{
	char functions[32];
	size_t len;

	if (android_dev.enabled)
		return -EBUSY;
	len = min_t(size_t, size, sizeof(functions) - 1);
	memcpy(functions, buf, len);
	functions[len] = '\0';
	android_dev.use_iap2 = strstr(functions, "iap2") != NULL;
	android_dev.use_ncm = strstr(functions, "ncm") != NULL;
	if (!android_dev.use_iap2 && !android_dev.use_ncm)
		return -EINVAL;
	return size;
}
static DEVICE_ATTR(functions, S_IRUGO | S_IWUSR, functions_show,
		   functions_store);

static ssize_t enable_show(struct device *d, struct device_attribute *attr,
			   char *buf)
{
	return sprintf(buf, "%u\n", android_dev.enabled);
}

static ssize_t enable_store(struct device *d, struct device_attribute *attr,
			    const char *buf, size_t size)
{
	unsigned int enable;
	int ret = 0;

	if (kstrtouint(buf, 0, &enable))
		return -EINVAL;
	mutex_lock(&android_dev.lock);
	if (!!enable == android_dev.enabled)
		goto out;
	if (enable) {
		android_dev.cdev->desc = android_desc;
		ret = usb_add_config(android_dev.cdev, &android_config,
				     android_config_bind);
		if (!ret) {
			usb_gadget_connect(android_dev.cdev->gadget);
			android_dev.enabled = true;
		}
	} else {
		usb_gadget_disconnect(android_dev.cdev->gadget);
		usb_remove_config(android_dev.cdev, &android_config);
		android_dev.enabled = false;
	}
out:
	mutex_unlock(&android_dev.lock);
	return ret ? ret : size;
}
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);

#define DESC_ATTR8(_name, _field)                                           \
static ssize_t _name##_show(struct device *d, struct device_attribute *a,   \
			    char *buf)                                    \
{ return sprintf(buf, "%u\n", android_desc._field); }                    \
static ssize_t _name##_store(struct device *d, struct device_attribute *a,  \
			     const char *buf, size_t size)                  \
{ unsigned int v; if (kstrtouint(buf, 0, &v)) return -EINVAL;               \
  if (v > 0xff) return -ERANGE; android_desc._field = v; return size; }      \
static DEVICE_ATTR(_name, S_IRUGO | S_IWUSR, _name##_show, _name##_store)

#define DESC_ATTR16(_name, _field)                                          \
static ssize_t _name##_show(struct device *d, struct device_attribute *a,   \
			    char *buf)                                    \
{ return sprintf(buf, "%04x\n", le16_to_cpu(android_desc._field)); }     \
static ssize_t _name##_store(struct device *d, struct device_attribute *a,  \
			     const char *buf, size_t size)                  \
{ unsigned int v; if (sscanf(buf, "%x", &v) != 1) return -EINVAL;          \
  if (v > 0xffff) return -ERANGE; android_desc._field = cpu_to_le16(v);      \
  return size; }                                                             \
static DEVICE_ATTR(_name, S_IRUGO | S_IWUSR, _name##_show, _name##_store)

DESC_ATTR16(idVendor, idVendor);
DESC_ATTR16(idProduct, idProduct);
DESC_ATTR16(bcdDevice, bcdDevice);
DESC_ATTR8(bDeviceClass, bDeviceClass);
DESC_ATTR8(bDeviceSubClass, bDeviceSubClass);
DESC_ATTR8(bDeviceProtocol, bDeviceProtocol);

#define STRING_ATTR(_name, _buffer)                                         \
static ssize_t _name##_show(struct device *d, struct device_attribute *a,   \
			    char *buf)                                    \
{ return sprintf(buf, "%s\n", _buffer); }                                 \
static ssize_t _name##_store(struct device *d, struct device_attribute *a,  \
			     const char *buf, size_t size)                  \
{ size_t n = min_t(size_t, size, sizeof(_buffer) - 1);                      \
  memcpy(_buffer, buf, n); if (n && _buffer[n - 1] == '\n') n--;            \
  _buffer[n] = 0; return size; }                                             \
static DEVICE_ATTR(_name, S_IRUGO | S_IWUSR, _name##_show, _name##_store)

STRING_ATTR(iManufacturer, manufacturer);
STRING_ATTR(iProduct, product);
STRING_ATTR(iSerial, serial);

static ssize_t state_show(struct device *d, struct device_attribute *attr,
			  char *buf)
{
	if (!android_dev.cdev || !android_dev.enabled)
		return sprintf(buf, "DISCONNECTED\n");
	if (android_dev.cdev->config)
		return sprintf(buf, "CONFIGURED\n");
	return sprintf(buf, "CONNECTED\n");
}
static DEVICE_ATTR(state, S_IRUGO, state_show, NULL);

static struct device_attribute *android_attrs[] = {
	&dev_attr_idVendor, &dev_attr_idProduct, &dev_attr_bcdDevice,
	&dev_attr_bDeviceClass, &dev_attr_bDeviceSubClass,
	&dev_attr_bDeviceProtocol, &dev_attr_iManufacturer,
	&dev_attr_iProduct, &dev_attr_iSerial, &dev_attr_functions,
	&dev_attr_enable, &dev_attr_state, NULL,
};

static int android_bind(struct usb_composite_dev *cdev)
{
	int id;

	usb_gadget_disconnect(cdev->gadget);
	android_dev.cdev = cdev;
	id = usb_string_ids_tab(cdev, android_strings);
	if (id < 0)
		return id;
	android_desc.iManufacturer = android_strings[0].id;
	android_desc.iProduct = android_strings[1].id;
	android_desc.iSerialNumber = android_strings[2].id;
	usb_gadget_set_selfpowered(cdev->gadget);
	return 0;
}

static int android_unbind(struct usb_composite_dev *cdev)
{
	android_dev.cdev = NULL;
	return 0;
}

static struct usb_composite_driver android_driver = {
	.name = DRIVER_NAME,
	.dev = &android_desc,
	.strings = android_string_tables,
	.max_speed = USB_SPEED_HIGH,
	.bind = android_bind,
	.unbind = android_unbind,
};

static int __init carlinkit_android_init(void)
{
	struct device_attribute **attr;
	int ret;

	spin_lock_init(&iap2_device.lock);
	mutex_init(&iap2_device.io_lock);
	init_waitqueue_head(&iap2_device.rx_wait);
	init_waitqueue_head(&iap2_device.tx_wait);
	INIT_LIST_HEAD(&iap2_device.tx_idle);
	INIT_LIST_HEAD(&iap2_device.rx_idle);
	INIT_LIST_HEAD(&iap2_device.rx_done);
	iap2_device.function.name = IAP2_NAME;
	iap2_device.function.strings = iap2_string_tables;
	iap2_device.function.bind = iap2_bind;
	iap2_device.function.unbind = iap2_unbind;
	iap2_device.function.set_alt = iap2_set_alt;
	iap2_device.function.disable = iap2_disable_function;

	ret = misc_register(&iap2_misc);
	if (ret)
		return ret;

	android_dev.ncm_inst = usb_get_function_instance("ncm");
	if (IS_ERR(android_dev.ncm_inst)) {
		ret = PTR_ERR(android_dev.ncm_inst);
		goto fail_misc;
	}
	android_dev.ncm_func = usb_get_function(android_dev.ncm_inst);
	if (IS_ERR(android_dev.ncm_func)) {
		ret = PTR_ERR(android_dev.ncm_func);
		goto fail_ncm_inst;
	}

	mutex_init(&android_dev.lock);
	android_dev.class = class_create("android_usb");
	if (IS_ERR(android_dev.class)) {
		ret = PTR_ERR(android_dev.class);
		goto fail_ncm;
	}
	android_dev.dev = device_create(android_dev.class, NULL, MKDEV(0, 0),
					NULL, "android0");
	if (IS_ERR(android_dev.dev)) {
		ret = PTR_ERR(android_dev.dev);
		goto fail_class;
	}
	for (attr = android_attrs; *attr; attr++) {
		ret = device_create_file(android_dev.dev, *attr);
		if (ret)
			goto fail_device;
	}

	ret = usb_composite_probe(&android_driver);
	if (ret)
		goto fail_device;
	pr_info("Carlinkit Android iAP2/NCM gadget registered\n");
	return 0;

fail_device:
	device_destroy(android_dev.class, MKDEV(0, 0));
fail_class:
	class_destroy(android_dev.class);
fail_ncm:
	usb_put_function(android_dev.ncm_func);
fail_ncm_inst:
	usb_put_function_instance(android_dev.ncm_inst);
fail_misc:
	misc_deregister(&iap2_misc);
	return ret;
}
late_initcall(carlinkit_android_init);

MODULE_AUTHOR("Carlinkit reconstruction");
MODULE_DESCRIPTION("Carlinkit Android iAP2 and NCM composite gadget");
MODULE_LICENSE("GPL");

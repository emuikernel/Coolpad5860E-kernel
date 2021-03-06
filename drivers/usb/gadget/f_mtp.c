#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/utsname.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <asm/siginfo.h>	//siginfo
#include <linux/rcupdate.h>	//rcu_read_lock
#include <linux/sched.h>	//find_task_by_pid_type
#include <linux/delay.h>
#include "ptp.h"

#include <linux/usb/ch9.h>
#include <linux/usb/android_composite.h>
#include <linux/usb/gadget.h>
#include <linux/platform_device.h>

#include "gadget_chips.h"

#include <linux/io.h>
#include <linux/wait.h>
#include <linux/err.h>

#include "u_char.c"
#include "kfifo.c"
//#include "f_mtp.h"

struct mtp_platform_data {
  char *vendor;
  char *product;
  int release;
};

static pid_t pid_mtp;
#define SIG_MTP 44


struct mtp_ep_desc {
	struct usb_endpoint_descriptor *mtp_in;
	struct usb_endpoint_descriptor *mtp_out;
	struct usb_endpoint_descriptor *mtp_int;
};

#define MAX_STATUS_DATA_SIZE	(PTP_MAX_STATUS_SIZE - 4)
/* device status cache */
struct device_status {
	u16 length;
	u16 code;
	u8 data[MAX_STATUS_DATA_SIZE];
};

struct f_mtp {
	struct gchar	        gc;
	struct usb_composite_dev *cdev;
	u8                      ctrl_id;
	u8                      mtp_id;
	u8                      minor;
	u8                      connected;
	struct device_status	dev_status;
	spinlock_t              lock;

	struct mtp_ep_desc      fs;
	struct mtp_ep_desc      hs;

	struct usb_ep                   *notify;
	struct usb_endpoint_descriptor  *notify_desc;
	 u16                     device_status;
	 const char		*vendor;
	const char		*product;
	int				release;
	struct platform_device *pdev;

	int usb_speed;
};

static struct f_mtp		*the_mtp;
/*-------------------------------------------------------------------------*/

static inline struct f_mtp *func_to_mtp(struct usb_function *f)
{
	return container_of(f, struct f_mtp, gc.func);
}

static inline struct f_mtp *gchar_to_mtp(struct gchar *p)
{
	return container_of(p, struct f_mtp, gc);
}

/*
 * USB String Descriptors
 */
/* String IDs */
#define INTERFACE_STRING_INDEX	0

#define MS_OS_STRING_DESCRIPTOR_ID     0xee

static struct usb_string mtp_string_defs[] = {
#if 0
       { 0, "MTP" ,},
       { MS_OS_STRING_DESCRIPTOR_ID , "MSFT100" ,},
       {  },
#else
	/* Naming interface "MTP" so libmtp will recognize us */
	[INTERFACE_STRING_INDEX].s	= "MTP",
	{  },	/* end of list */
#endif
};

static struct usb_gadget_strings mtp_string_table = {
	.language       = 0x0409,       /* en-US */
	.strings        = mtp_string_defs,
};

static struct usb_gadget_strings *mtp_strings[] = {
	&mtp_string_table,
	NULL,
};

/*
 * USB Interface Descriptors
 */

static struct usb_interface_descriptor mtp_intf   = {
	.bLength                = sizeof(mtp_intf),
	.bDescriptorType        = USB_DT_INTERFACE,
	.bAlternateSetting      = 0,
	.bNumEndpoints          = 3,
	.bInterfaceClass        = 6,
	.bInterfaceSubClass     = USB_SUBCLASS_PTP,
	.bInterfaceProtocol     = USB_PROTOCOL_PTP,
};

/*
 * USB Endpoint Descriptors
 */

/* High speed support */
static struct usb_endpoint_descriptor mtp_ep_hs_in_desc  = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(PTP_HS_DATA_PKT_SIZE),
	.bInterval              = 0,
};


static struct usb_endpoint_descriptor mtp_ep_hs_out_desc  = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_OUT,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(PTP_HS_DATA_PKT_SIZE),
	.bInterval              = 0,
};


static struct usb_endpoint_descriptor mtp_ep_hs_int_desc  = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN,
	.bmAttributes           = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize         = __constant_cpu_to_le16(PTP_HS_EVENT_PKT_SIZE),
	.bInterval              = 12,
};

static struct usb_descriptor_header *mtp_hs_function[]  = {
	(struct usb_descriptor_header *) &mtp_intf,
	(struct usb_descriptor_header *) &mtp_ep_hs_in_desc,
	(struct usb_descriptor_header *) &mtp_ep_hs_out_desc,
	(struct usb_descriptor_header *) &mtp_ep_hs_int_desc,
	NULL,
};

/* Full speed support */
static struct usb_endpoint_descriptor mtp_ep_fs_in_desc  = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(PTP_FS_DATA_PKT_SIZE),
	.bInterval              = 0,
};

static struct usb_endpoint_descriptor mtp_ep_fs_out_desc  = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_OUT,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(PTP_FS_DATA_PKT_SIZE),
	.bInterval              = 0,
};

static struct usb_endpoint_descriptor mtp_ep_fs_int_desc  = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN,
	.bmAttributes           = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize         = __constant_cpu_to_le16(PTP_FS_EVENT_PKT_SIZE),
	.bInterval              = 255,
};

static struct usb_descriptor_header *mtp_fs_function[]  = {
	(struct usb_descriptor_header *) &mtp_intf,
	(struct usb_descriptor_header *) &mtp_ep_fs_in_desc,
	(struct usb_descriptor_header *) &mtp_ep_fs_out_desc,
	(struct usb_descriptor_header *) &mtp_ep_fs_int_desc,
	NULL,
};

static struct usb_descriptor_header *mtp_null_function[]  = {
	NULL
};

/**
 * This function will be called when the request on the interrupt
 * end point being used for class specific events is completed.
 * Notes -
 * The protocol does not give any specifications about what needs
 * should be done in such case.
 * Revisit if there is more information.
 */
static void
mtp_notify_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_mtp *mtp               = req->context;
	struct usb_composite_dev *cdev	= mtp->cdev;

	VDBG(cdev, "%s:\n", __func__);

	switch (req->status) {
	case 0:
		/* normal completionn */
		break;

	case -ESHUTDOWN:
		/* disconnect */
		WARNING(cdev, "%s: %s shutdown\n", __func__, ep->name);
		break;

	default:
		WARNING(cdev, "%s: unexpected %s status %d\n",
				__func__, ep->name, req->status);
		break;
	}

	kfree(req->buf);
	usb_ep_free_request(ep, req);
	return;
}

#if 0
/**
 * build_device_status() - prepares the device status response
 *
 * @mtp: the f_mtp struct
 * @buf: buffer to build the response data into
 * @buf_len: length of buffer in bytes
 *
 * uses spinlock mtp->lock
 *
 * returns number of bytes copied.
 */
static int build_device_status(struct f_mtp *mtp, void *buf, size_t buf_len)
{
	int copied, len;
	__le16 *ptr = buf;
	struct device_status *status = &mtp->dev_status;

	spin_lock_irq(&mtp->lock);
	len = status->length;
	if (len > buf_len) {
		WARNING(mtp->cdev, "%s Insufficient buffer for dev_status\n",
					__func__);
		/* limit status data to available buffer */
		len = buf_len;
	}

	*ptr++ = cpu_to_le16(len);
	*ptr++ = cpu_to_le16(status->code);
	copied = 4;

	if (len > 4) {
		len -= 4;
		if (len > MAX_STATUS_DATA_SIZE) {
			len = MAX_STATUS_DATA_SIZE;
			WARNING(mtp->cdev, "%s limited status to %d bytes\n",
					__func__, len);
		}
		memcpy(ptr, status->data, len);
		copied += len;
	}
	spin_unlock_irq(&mtp->lock);
	return copied;
}
#endif

/**
 * cache_device_status() - saves the device status to struct f_mtp
 *
 * @mtp: the f_mtp struct
 * @length: length of PTP device status
 * @code: code of PTP device status
 * @buf: user space buffer pointing to PTP device status container
 *
 * uses spinlock mtp->lock
 *
 * returns 0 on success. negative on error
 */
static int cache_device_status(struct f_mtp *mtp,
				u16 length, u16 code, const void __user *buf)
{
	u8 *uninitialized_var(tmp_data);

	if (length > 4) {
		if (!buf) {
			WARNING(mtp->cdev, "%s No data buffer provided\n",
					__func__);
			return -EINVAL;
		}

		length -= 4; /* get length of data section */
		if (length > MAX_STATUS_DATA_SIZE) {
			length = MAX_STATUS_DATA_SIZE;
			WARNING(mtp->cdev, "%s limited status data to %d "
				"bytes\n", __func__, length);
		}

		tmp_data = kmalloc(length, GFP_KERNEL);
		if (!tmp_data)
			return -ENOMEM;

		/* 4 bytes are for header, leave them out */
		if (copy_from_user(tmp_data, buf + 4, length)) {
			ERROR(mtp->cdev, "%s copy_from_user fault\n", __func__);
			kfree(tmp_data);
			return -EFAULT;
		}
		length += 4;	/* undo the previous minus */
	}

	spin_lock_irq(&mtp->lock);
	if (length > 4) {
		memcpy(mtp->dev_status.data, tmp_data, length - 4);
		kfree(tmp_data);
	}
	mtp->dev_status.length = length;
	mtp->dev_status.code = code;
	spin_unlock_irq(&mtp->lock);
	return 0;
}

static int send_status_to_user(void)
{
	struct siginfo info;	
	struct task_struct *t;
	int ret;
	memset(&info, 0, sizeof(struct siginfo));	
	info.si_signo = SIG_MTP;	
	info.si_code = SI_QUEUE;
	info.si_int = 0; 	
	rcu_read_lock();	
	t = find_task_by_vpid(pid_mtp);  //find the task_struct associated with this pid	
	if(t == NULL)
		{		
		printk(KERN_ERR "USB:MTP:send_status_to_user:no such pid\n");
		rcu_read_unlock();
		return -ENODEV;	
		}	
	rcu_read_unlock();	
	ret = send_sig_info(SIG_MTP, &info, t);    //send the signal	
	if (ret < 0) 
		{		
		printk("error sending signal\n");		
		return ret;	
		}
	return ret;
}

/**
 * Handle the MTP specific setup requests
 */
static int
mtp_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct	f_mtp *mtp		= func_to_mtp(f);
	struct	usb_composite_dev *cdev	= f->config->cdev;
	struct	usb_request *req	= cdev->req;

	int	value			= -EOPNOTSUPP;
	u16 wIndex = le16_to_cpu(ctrl->wIndex);
	u16 wValue = le16_to_cpu(ctrl->wValue);
	u16 wLength = le16_to_cpu(ctrl->wLength);
        struct ptp_device_status_data status_data;
	printk(KERN_INFO "USB:MTP:mtp_setup(%d)\n", ctrl->bRequest);


	switch (ctrl->bRequest) {
	case PTP_REQ_GET_EXTENDED_EVENT_DATA:
		/* FIXME need to implement
		 * Maybe we could have an IOCTL to save the extended event
		 * data with the driver and then send it to host whenever
		 * we get this request
		 */
		printk(KERN_INFO "%s: FIXME: PTP request GET_EXTENDED_EVENT_DATA, "
					"not implemented\n", __func__);
		//goto stall;
		break;

	case PTP_REQ_CANCEL:
		DBG(cdev, "%s: PTP: CANCEL\n", __func__);
		if (ctrl->bRequestType != (USB_DIR_OUT |
					USB_TYPE_CLASS | USB_RECIP_INTERFACE))
			goto stall;

		if (wValue != 0 || wIndex != 0 || wLength != 6)
			goto stall;
		
               usb_ep_fifo_flush(mtp->gc.ep_in);
               usb_ep_fifo_flush(mtp->gc.ep_out);
               usb_ep_fifo_flush(mtp->notify);

		mtp->gc.mtp_read_status=ECANCELED;
		mtp->gc.mtp_write_status=ECANCELED;
		
               mtp->device_status = PTP_RC_OK;
               value=0;
               break;
       /* After this we should get the DEVICE_RESET_REQUEST from the host*/

	case PTP_REQ_DEVICE_RESET:
		DBG(cdev, "%s: PTP: DEVICE_RESET\n", __func__);
		
		if (ctrl->bRequestType != (USB_DIR_OUT |
					USB_TYPE_CLASS | USB_RECIP_INTERFACE))
					{						
						goto stall;
					}

		if (wValue != 0 || wIndex != 0 || wLength != 0)
		{			
			goto stall;
		}
		send_status_to_user();	
		
               /* Reset the endpoints */
               usb_ep_fifo_flush(mtp->gc.ep_in);
               usb_ep_fifo_flush(mtp->gc.ep_out);
               usb_ep_fifo_flush(mtp->notify);

               mtp->device_status = PTP_RC_OK;
               value=0;
               break;

	case PTP_REQ_GET_DEVICE_STATUS:
		/* return the cached device status */
		DBG(cdev, "%s: PTP: GET_DEVICE_STATUS\n", __func__);

		if (ctrl->bRequestType != (USB_DIR_IN |
					USB_TYPE_CLASS | USB_RECIP_INTERFACE)) {
			goto stall;
		}

		if (wValue != 0 || wIndex != 0)
			goto stall;


		switch (mtp->device_status) {
			printk(KERN_INFO "USB:MTP:MTP_SETUP: The device status is %x \n",mtp->device_status);
               case PTP_RC_DEVICE_BUSY:
                       DBG(cdev, "%s: PTP_RC_DEVICE_BUSY\n", __func__);
                       status_data.wLength = __constant_cpu_to_le16(4);
                       status_data.code = __constant_cpu_to_le16(
                                       PTP_RC_DEVICE_BUSY);
                       value = min(wLength, status_data.wLength);
                       memcpy(req->buf, &status_data, value);
                       break;

               case PTP_RC_OK:
                       DBG(cdev, "%s: PTP_RC_OK\n", __func__);
                       status_data.wLength = __constant_cpu_to_le16(4);
                       status_data.code = __constant_cpu_to_le16(PTP_RC_OK);
                       value = min(wLength, status_data.wLength);
                       memcpy(req->buf, &status_data, value);
                       break;

               /* TBD: other response codes */

               default:
                       break;
               }
		value = min(wLength, (u16)value);
		break;

	/* TBD: other response codes */
	default:
		WARNING(cdev,
			"%s: FIXME, got PTP request 0x%x, not implemented\n",
				__func__, ctrl->bRequest);
		break;
	}

	/* data phase of control transfer */
	if (value >= 0) {
		req->length = value;
		req->zero = value < wLength;
		value = usb_ep_queue(cdev->gadget->ep0,
				req, GFP_ATOMIC);
		if (value < 0) {
			DBG(cdev, "%s: ep_queue --> %d\n", __func__, value);
			req->status = 0;
			return value;
		}
		return value;
	}
return value;
stall:
       usb_ep_fifo_flush(mtp->gc.ep_in);
       usb_ep_fifo_flush(mtp->gc.ep_out);
       usb_ep_fifo_flush(mtp->notify);

       usb_ep_set_halt(mtp->gc.ep_in);
       usb_ep_set_halt(mtp->gc.ep_out);
       usb_ep_set_halt(mtp->notify);
       return value;

}

static long
mtp_ioctl(struct gchar *gc, unsigned int cmd, unsigned long arg)
{
	int status;
	struct f_mtp			*mtp	= gchar_to_mtp(gc);
	struct usb_composite_dev	*cdev	= mtp->cdev;
	int packet_size;
	struct usb_request *notify_req;
	void *event_packet;
	u32 event_packet_len;
	struct ptp_device_status_data ptp_status;

     
        printk(KERN_INFO "USB:MTP:mtp_ioctl(%d)\n", cmd);
        switch (cmd) {
	case MTP_IOCTL_WRITE_ON_INTERRUPT_EP:

		/* get size of packet */
		if (copy_from_user(&event_packet_len,
				(void __user *)arg, 4))
			return -EFAULT;

		event_packet_len = le32_to_cpu(event_packet_len);
		if (event_packet_len > mtp->notify->maxpacket) {
			ERROR(cdev, "%s Max event packet limit exceeded\n",
							__func__);
			return -EFAULT;
		}

		event_packet = kmalloc(event_packet_len, GFP_KERNEL);
		if (!event_packet) {
			ERROR(cdev, "%s cannot allocate memory for event\n",
							__func__);
			return -ENOMEM;
		}

		/* read full packet */
		if (copy_from_user(event_packet,
				(void __user *)arg, event_packet_len)) {
			kfree(event_packet);
			return -EFAULT;
		}

		/* Allocate request object to be used with this endpoint. */
		notify_req = usb_ep_alloc_request(mtp->notify, GFP_KERNEL);
		if (!notify_req) {
			ERROR(cdev,
				"%s: could not allocate notify EP request\n",
								__func__);
			kfree(event_packet);
			return -ENOMEM;
		}

		notify_req->buf = event_packet;
		notify_req->context = mtp;
		notify_req->complete = mtp_notify_complete;
		notify_req->length = event_packet_len;
		if (unlikely(event_packet_len == mtp->notify->maxpacket))
			notify_req->zero = 1;
		else
			notify_req->zero = 0;


		status = usb_ep_queue(mtp->notify, notify_req, GFP_ATOMIC);
		if (status) {
			ERROR(cdev,
				"%s: EVENT packet could not be queued %d\n",
					__func__, status);
			usb_ep_free_request(mtp->notify, notify_req);
			kfree(event_packet);
			return status;
		}
		return 0;

	case MTP_IOCTL_GET_MAX_DATAPKT_SIZE:
		switch (mtp->usb_speed) {
		case USB_SPEED_LOW:
		case USB_SPEED_FULL:
			packet_size = PTP_FS_DATA_PKT_SIZE;
			break;

		case USB_SPEED_HIGH:
		case USB_SPEED_WIRELESS:
			packet_size = PTP_HS_DATA_PKT_SIZE;
			break;

		default:
			return -EINVAL;
		}
        printk(KERN_INFO "USB:MTP:mtp_ioctl:Reached here becoz of user prog \n");

		status = put_user(packet_size, (int *)arg);
		if (status) {
			ERROR(cdev,
				"%s: could not send max data packet size\n",
								__func__);
			return -EFAULT;
		}
		return 0;

	case MTP_IOCTL_GET_MAX_EVENTPKT_SIZE:
		switch (mtp->usb_speed) {
		case USB_SPEED_LOW:
		case USB_SPEED_FULL:
			packet_size = PTP_FS_EVENT_PKT_SIZE;
			break;

		case USB_SPEED_HIGH:
		case USB_SPEED_WIRELESS:
			packet_size = PTP_HS_EVENT_PKT_SIZE;
			break;

		default:
			return -EINVAL;
		}

		status = put_user(packet_size, (int *)arg);
		if (status) {
			ERROR(cdev,
				"%s: couldn't send max event packet size\n",
								__func__);
			return -EFAULT;
		}
		return 0;

	case MTP_IOCTL_SET_DEVICE_STATUS:
		if (copy_from_user(&ptp_status, (const void __user *)arg,
							sizeof(ptp_status)))
			return -EFAULT;

		status = cache_device_status(mtp,
					__le16_to_cpu(ptp_status.wLength),
					__le16_to_cpu(ptp_status.code),
					(const void __user *)(arg));
		return status;
	
	case MTP_IOCTL_DEVICE_RESET:
			//printk("Reset Value is %d\n",resetValue);
			if (copy_from_user(&pid_mtp,(void __user *)arg,sizeof(pid_t)))
				return -EFAULT;
			status =0;
			return status;	

	default:
		//WARNING(cdev, "%s: unhandled IOCTL %d\n", __func__, cmd);
		return -EINVAL;
	}
}

static void
mtp_disconnect(struct gchar *gc)
{
	struct f_mtp *mtp = gchar_to_mtp(gc);
	struct usb_composite_dev *cdev = mtp->cdev;
	int status = 0;

	if (!mtp->connected)
		return;

	//modified by heyong
	/*status  = usb_function_deactivate(&gc->func);*/
	if (status) {
		WARNING(cdev, "%s: could not deactivate mtp function %d, "
			"status: %d\n", __func__, mtp->minor, status);
	} else {
		mtp->connected  = false;
		INFO(cdev, "mtp function %d disconnected\n", mtp->minor);
	}
}

static void mtp_connect(struct gchar *gc)
{
	struct f_mtp *mtp               = gchar_to_mtp(gc);
	struct usb_composite_dev *cdev	= mtp->cdev;
	int status                      = 0;

	if (mtp->connected)
		return;

	/*status  = usb_function_activate(&gc->func);*/
	if (status) {
		WARNING(cdev, "%s: could not activate mtp function %d, "
				"status: %d\n", __func__, mtp->minor, status);
	} else {
		mtp->connected  = true;
		INFO(cdev, "mtp function %d connected\n", mtp->minor);
	}
}

/*add by yanghaishan,20110525*/
static int configurated = 0;

/**
 * Set alt-settings of the interface.
 * When connected with host, this function will get called for the
 * number of interfaces defined for this gadget.
 * This function enables all the required end points.
 */
static int
mtp_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct usb_composite_dev *cdev  = f->config->cdev;
	struct f_mtp *mtp               = func_to_mtp(f);
	int status                      = -1;
	printk(KERN_INFO "USB!MTP:%s\n", __func__);

	if (intf != mtp->mtp_id)
		return -EINVAL;

	if (alt != 0) {
		WARNING(cdev, "%s: invalid setting\n", __func__);
		return -EINVAL;
	}

	/* Handle interrupt endpoint */
	if (mtp->notify->driver_data) {
		DBG(cdev, "%s: notify reset mtp control %d\n", __func__, intf);
		usb_ep_disable(mtp->notify);
	} else {
		DBG(cdev, "%s: notify init mtp ctrl %d\n", __func__, intf);
		mtp->notify_desc = ep_choose(cdev->gadget,
					mtp->hs.mtp_int,
					mtp->fs.mtp_int);
	}

	status = usb_ep_enable(mtp->notify, mtp->notify_desc);
	if (status != 0)
		ERROR(cdev, "%s: Error enabling endpoint\n", __func__);
	mtp->notify->driver_data = mtp;



	if (mtp->gc.ep_in->driver_data) {
		/* Altsetting 0 for an interface that already has 0 altset,
		 * ignore this
		 */
		 printk(KERN_ERR "USB!MTP:mtp_set_alt return\n");
		//return 0;
	}

	if (!mtp->gc.ep_in_desc) {
		DBG(cdev, "%s: init mtp %d\n", __func__, mtp->minor);
		mtp->gc.ep_in_desc = ep_choose(cdev->gadget,
					mtp->hs.mtp_in,
					mtp->fs.mtp_in);
		mtp->gc.ep_out_desc = ep_choose(cdev->gadget,
					mtp->hs.mtp_out,
					mtp->fs.mtp_out);
	}

	DBG(cdev, "%s: mtp %d enable\n", __func__, mtp->minor);
	printk(KERN_INFO "USB!%s: mtp %d enable\n", __func__, mtp->minor);
	status = gchar_connect(&mtp->gc, mtp->minor, f->name);
	if (status) {
		ERROR(cdev,
			"%s: gchar_connect() failed %d\n", __func__, status);
		return status;
	}
	configurated = 1;

	/* Get the USB speed */
	mtp->usb_speed = cdev->gadget->speed;
	return 0;
}

static void
mtp_disable(struct usb_function *f)
{
	struct f_mtp *mtp               = func_to_mtp(f);
	struct usb_composite_dev *cdev  = f->config->cdev;

	printk(KERN_INFO "%s: mtp %d disable\n", __func__, mtp->minor);
	/* disable OUT/IN endpoints */
	if(configurated)
		gchar_disconnect(&mtp->gc);
	/* disable INT endpoint */
	if (mtp->notify->driver_data) {
		usb_ep_disable(mtp->notify);
		mtp->notify->driver_data = NULL;
		mtp->notify_desc = NULL;
	}
	mtp->usb_speed = -1;
}

static int
__init mtp_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_mtp                    *mtp    = func_to_mtp(f);
	struct usb_composite_dev        *cdev   = c->cdev;
	int                             status;
	struct usb_ep                   *ep     = NULL;
	
	printk(KERN_INFO "USB:MTP:mtp_bind\n");
	/* allocate instance-specific interface IDs and patch up descriptors */
	/* We have only ONE MTP interface. So get the unused interface ID for
	 * this interface.*/
	status = usb_interface_id(c, f);
	if (status < 0)
		return status;
	mtp->mtp_id = status;
	mtp_intf.bInterfaceNumber = status;

	status = -ENOMEM;
	/* Allocate the endpoints */
	/* mtp_ep_fs_in_desc */
	ep = usb_ep_autoconfig(cdev->gadget, &mtp_ep_fs_in_desc);
	if (!ep)
		goto fail;
	mtp->gc.ep_in = ep;
	ep->driver_data = cdev;

	/* mtp_ep_fs_out_desc */
	ep = usb_ep_autoconfig(cdev->gadget, &mtp_ep_fs_out_desc);
	if (!ep)
		goto fail;
	mtp->gc.ep_out = ep;
	ep->driver_data = cdev;

	/* mtp_ep_fs_int_desc */
	ep = usb_ep_autoconfig(cdev->gadget, &mtp_ep_fs_int_desc);
	if (!ep)
		goto fail;
	mtp->notify = ep;
	ep->driver_data = cdev;


	/* copy descriptors, and track endpoint copies */
	f->descriptors = usb_copy_descriptors(mtp_fs_function);
	mtp->fs.mtp_in = usb_find_endpoint(mtp_fs_function,
			f->descriptors, &mtp_ep_fs_in_desc);
	mtp->fs.mtp_out = usb_find_endpoint(mtp_fs_function,
			f->descriptors, &mtp_ep_fs_out_desc);
	mtp->fs.mtp_int = usb_find_endpoint(mtp_fs_function,
			f->descriptors, &mtp_ep_fs_int_desc);

	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		/* Copy endpoint address */
		mtp_ep_hs_in_desc.bEndpointAddress =
			mtp_ep_fs_in_desc.bEndpointAddress;
		mtp_ep_hs_out_desc.bEndpointAddress =
			mtp_ep_fs_out_desc.bEndpointAddress;
		mtp_ep_hs_int_desc.bEndpointAddress =
			mtp_ep_fs_int_desc.bEndpointAddress;

		/* Copy descriptors, and track endpoint copies */
		f->hs_descriptors = usb_copy_descriptors(mtp_hs_function);
		mtp->hs.mtp_in = usb_find_endpoint(mtp_hs_function,
				f->hs_descriptors, &mtp_ep_hs_in_desc);
		mtp->hs.mtp_out = usb_find_endpoint(mtp_hs_function,
				f->hs_descriptors, &mtp_ep_hs_out_desc);
		mtp->hs.mtp_int = usb_find_endpoint(mtp_hs_function,
				f->hs_descriptors, &mtp_ep_hs_int_desc);
	}

	/* Prevent enumeration until someone opens
	 * the port from the user space */
	/*status = usb_function_deactivate(f);*/
	status = 0;
	if (status < 0) {
		WARNING(cdev, "%s: mtp %d: can't prevent enumeration, %d\n",
				__func__, mtp->minor, status);
		mtp->connected = true;
	}


	INFO(cdev, "mtp %d: %s speed IN/%s OUT/%s INT/%s\n",
			mtp->minor,
			gadget_is_dualspeed(cdev->gadget) ? "dual" : "full",
			mtp->gc.ep_in->name, mtp->gc.ep_out->name,
			mtp->notify->name);
	//mtp->gc.ep_in->driver_data = NULL;
	//mtp->gc.ep_out->driver_data = NULL;
	/*printk(KERN_ERR "USB!MTP:mtp_bind:mtp->gc=%p,mtp->gc.ep_in->driver_data=%p,mtp->gc.ep_out->driver_data=%p\n",
		&mtp->gc, mtp->gc.ep_in->driver_data, mtp->gc.ep_out->driver_data);*/

	return 0;
fail:
	if (mtp->gc.ep_out)
		mtp->gc.ep_out->driver_data = NULL;

	if (mtp->gc.ep_in)
		mtp->gc.ep_in->driver_data = NULL;

	if (mtp->notify)
		mtp->notify->driver_data = NULL;

	ERROR(cdev, "%s/%p: cant bind, err %d\n", f->name, f, status);

	return status;
}

static void
mtp_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_mtp *mtp = func_to_mtp(f);
	printk(KERN_INFO "USB:MTP:mtp_unbind!!!\n");
	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);

	usb_free_descriptors(f->descriptors);
	kfree(mtp);
}

//extern struct android_dev *_android_dev;

int mtp_probe(struct platform_device *pdev)
{
	struct mtp_platform_data *pdata = pdev->dev.platform_data;
	struct f_mtp *mtp = the_mtp;
	mtp->pdev = pdev;
	printk(KERN_INFO "USB:MTP:mtp_probe pdata: %p\n", pdata);

	if (pdata) {
	if (pdata->vendor)
	  mtp->vendor = pdata->vendor;
	if (pdata->product)
	  mtp->product = pdata->product;
	if (pdata->release)
	  mtp->release = pdata->release;
	}
	return 0;
}


static struct platform_driver mtp_platform_driver = {
    .driver = { .name = "mtp_gadget", },
      .probe = mtp_probe,
};



/**
 * mtp_bind_config - add a MTP function to a configuration
 * @c: the configuration to support MTP
 * @port_num: /dev/ttyGS* port this interface will use
 * Context: single threaded during gadget setup
 *
 * Returns zero on success, else negative errno.
 *
 * Caller must have called @gchar_setup() with enough devices to
 * handle all the ones it binds. Caller is also responsible
 * for calling @gchar_cleanup() before module unload.
 */
int
__init mtp_bind_config(struct usb_configuration *c)
{
	struct f_mtp    *mtp    = NULL;
	int             status  = 0;
        int minor=0;
	printk(KERN_INFO "USB!MTP: mtp_bind_config\n");		
	status = gchar_setup(c->cdev->gadget, 1);
	if (status < 0)
		return status;
	//mdelay(50);
	//printk(KERN_ERR "USB!MTP: gchar setuped\n");
	
	/* allocate device global string IDs and patch descriptors*/
	if (mtp_string_defs[0].id == 0) {
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		mtp_string_defs[0].id = status;
		mtp_intf.iInterface = status;
	}
	
	

	/* allocate and initialize one new instance */
	mtp = kzalloc(sizeof(*mtp), GFP_KERNEL);
	if (!mtp)
		return -ENOMEM;

	
        
       status=platform_driver_register(&mtp_platform_driver);
       if(status<0)
          return -1;
        
        
        spin_lock_init(&mtp->lock);


	mtp->minor		= minor;

	mtp->gc.func.name	= "mtp";
	mtp->gc.func.strings	= mtp_strings;
	mtp->gc.func.descriptors = mtp_fs_function;
	mtp->gc.func.hs_descriptors = mtp_hs_function;
	mtp->gc.open		= mtp_connect;
	mtp->gc.close		= mtp_disconnect;
	mtp->gc.ioctl		= mtp_ioctl;

	mtp->gc.func.bind	= mtp_bind;
	mtp->gc.func.unbind	= mtp_unbind;
	mtp->gc.func.set_alt	= mtp_set_alt;
	mtp->gc.func.setup	= mtp_setup;
	mtp->gc.func.disable	= mtp_disable;

	/* start disabled */
	mtp->gc.func.disabled = 1;
	
	mtp->usb_speed		= -1;  /* invalid speed */

	mtp->cdev		= c->cdev;
	the_mtp=mtp;

	/* default device status is BUSY */
	cache_device_status(mtp, 4, PTP_RC_DEVICE_BUSY, 0);

	status = usb_add_function(c, &mtp->gc.func);
	if (status) {
		kfree(mtp);
	}

	return status;

}


static struct android_usb_function mtp_function = {
	.name = "mtp",
	.bind_config = mtp_bind_config,
};

static int __init init(void)
{
	printk(KERN_INFO "USB!f_mtp init\n");
	android_register_function(&mtp_function);
	return 0;
}
module_init(init);

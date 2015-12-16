/*
 * Rawbulk Gadget Function Driver from VIA Telecom
 *
 * Copyright (C) 2011 VIA Telecom, Inc.
 * Author: Karfield Chen (kfchen@via-telecom.com)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/* #define DEBUG */
/* #define VERBOSE_DEBUG */

#define DRIVER_AUTHOR   "peter <peter.he@yulong.com>"
#define DRIVER_DESC     "Rawbulk Gadget - transport data from CP to Gadget"
#define DRIVER_VERSION  "1.0.1"
#define DRIVER_NAME     "usb_rawbulk"

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/err.h>
#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>

#include <linux/usb.h>
#include <linux/usb/composite.h>
#include <linux/usb/rawbulk.h>

#define function_to_rawbulk(f) container_of(f, struct rawbulk_function, function)

static void simple_setup_complete(struct usb_ep *ep,
                struct usb_request *req) {
    ;//DO NOTHING
}

static int rawbulk_function_setup(struct usb_function *f, const struct
        usb_ctrlrequest *ctrl) {
    struct rawbulk_function *fn = function_to_rawbulk(f);
    struct usb_composite_dev *cdev = f->config->cdev;
    struct usb_request *req = cdev->req;

	printk(KERN_ERR "forwarding ctl request: RqTp %02x Rq %02x Val %02x Idx %02x Len %02x\n",
		ctrl->bRequestType, ctrl->bRequest, ctrl->wValue, ctrl->wIndex, ctrl->wLength);

    if (ctrl->bRequestType == (USB_DIR_IN | USB_TYPE_VENDOR)) {
	    /* handle request when the function is disabled */
	    if (ctrl->bRequest == 0x02 || ctrl->bRequest == 0x05) {
		    /* return CD status or connection status */
		    *(unsigned char *)req->buf = fn->dsr_state & 0xff;
		    req->length = 1;
		    req->complete = simple_setup_complete;
	    } else if (ctrl->bRequest == 0x04) /* return CBP ID */
		    req->length = sprintf(req->buf, "CBP_KUNLUN");
	    else req->length = 0;
	    req->zero = 0;
	    return usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
    }

    if (!rawbulk_check_enable(fn) || rawbulk_transfer_state(fn->transfer_id) < 0) {
	    /* Complete the status stage */
		printk(KERN_ERR "%s Complete the status stage!\n",__func__);
	    return usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
    }

    if (rawbulk_check_enable(fn)) {
		printk(KERN_ERR "%s rawbulk_check_enable is enabled!\n",__func__);
        return rawbulk_forward_ctrlrequest(ctrl);
    }

	printk(KERN_ERR "%s EOPNOTSUPP!\n",__func__);
    return -EOPNOTSUPP;
}

static struct work_struct modem_control_work;
static void do_modem_control(struct work_struct *data) {
    struct rawbulk_function *fn = rawbulk_get_by_index(RAWBULK_TID_MODEM);
    if (rawbulk_check_enable(fn)) {
        struct usb_device *cp = rawbulk_get_cp();
        if (cp) {
            int ret = usb_control_msg(cp, usb_rcvctrlpipe(cp, 0), 0x01,
                    USB_DIR_OUT | USB_TYPE_VENDOR, fn->dtr_state & 0x01, 0,
                    NULL, 0, USB_CTRL_SET_TIMEOUT);
            if (ret < 0)
                printk(KERN_ERR "%s - %d!\n", __func__, ret);
            rawbulk_put_cp(cp);
        }
    }
}

static void simple_ctrl_complete(struct urb *urb) {
	if (urb->status < 0)
		printk(KERN_ERR "Smobc %d\n", urb->status);
}
int modem_function_setup(struct usb_function *f, 
        const struct usb_ctrlrequest *ctrl)
{
    struct rawbulk_function *fn = function_to_rawbulk(f);
    struct usb_composite_dev *cdev = f->config->cdev;
    struct usb_request *req = cdev->req;
    int         value = -EOPNOTSUPP;
    u16         w_index = le16_to_cpu(ctrl->wIndex);
    u16         w_value = le16_to_cpu(ctrl->wValue);
	u16         w_length = le16_to_cpu(ctrl->wLength);

	printk("%s - fn id %d name %s\n", __func__, fn->transfer_id, fn->longname);
	switch(ctrl->bRequest) {
        case 0x01:
            if(ctrl->bRequestType == (USB_DIR_OUT | USB_TYPE_VENDOR)) {//0x40
                /* set/clear DTR */
                //dev->setdtr = w_value & 0x01;
                //schedule_work(&dev->setflow_work);
#if 0
                if (rawbulk_check_enable(fn)) {
                    struct usb_ctrlrequest ctrl0 = {
                        .bRequestType = USB_DIR_OUT | USB_TYPE_VENDOR, //0x40
                        .bRequest = 0x01,
                        .wLength = 0,
                        .wIndex = 0,
                    };
                    printk(KERN_ERR "Fuck here!\n");
                    ctrl0.wValue = w_value & 0x01;
                    rawbulk_async_control_msg(&ctrl0, simple_ctrl_complete);
                }
#endif
		fn->dtr_state = w_value & 0x01;
                schedule_work(&modem_control_work);
                value = 0;
            }
            break;
        case 0x02:
            if(ctrl->bRequestType == (USB_DIR_IN | USB_TYPE_VENDOR)) {//0xC0
                /* DSR | CD109 */
                *((unsigned char *)req->buf) = fn->dsr_state & 0xff;
                value = 1;
            }
            break;
        case 0x03:
            if(ctrl->bRequestType == (USB_DIR_OUT | USB_TYPE_VENDOR)) {//0x40
                /* xcvr */
                printk("%s - CTRL SET XCVR 0x%02x\n", __func__, w_value);
                value = 0;
            }
            break;
        case 0x04:
            if((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_VENDOR) {
                if(ctrl->bRequestType & USB_DIR_IN) {//0xC0
                    /* return ID */
                    *((unsigned char *)req->buf) = 0;
                    value = 1;
                } else {//0x40
                    /* store ID */
                    value = 0;
                }
            }
            break;
        case 0x05:
            if(ctrl->bRequestType == (USB_DIR_IN | USB_TYPE_VENDOR)) {//0xC0
                /* connect status */
                printk("%s - CTRL CONNECT STATUS\n", __func__);
                *((unsigned char *)req->buf) = 0;
                value = 1;
            }
            break;
        default:
            printk("invalid control req%02x.%02x v%04x i%04x l%d\n",
                    ctrl->bRequestType, ctrl->bRequest,
                    w_value, w_index, w_length);
    }

    /* respond with data transfer or status phase? */
    if (value >= 0) {
        /*dbg("respond: req%02x.%02x v%04x i%04x l%d\n",
          ctrl->bRequestType, ctrl->bRequest,
          w_value, w_index, w_length);*/
        req->zero = 0;
        req->length = value;
        req->complete = simple_setup_complete;
        value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
        if (value < 0)
            printk("response err %d\n", value);
    }

    /* device either stalls (value < 0) or reports success */
    return value;
}

static int rawbulk_function_bind(struct usb_configuration *c, struct
        usb_function *f) {
    int rc, ifnum;
    struct device *dev;
    struct rawbulk_function *fn = function_to_rawbulk(f);
    struct usb_gadget *gadget = c->cdev->gadget;
    struct usb_ep *ep_out, *ep_in;

    rc = usb_interface_id(c, f);
    if (rc < 0)
        return rc;
    ifnum = rc;

    fn->interface.bInterfaceNumber = cpu_to_le16(ifnum);

    ep_out = usb_ep_autoconfig(gadget, (struct usb_endpoint_descriptor *)
            fn->fs_descs[BULKOUT_DESC]);
    if (!ep_out)
        return -ENOMEM;

    ep_in = usb_ep_autoconfig(gadget, (struct usb_endpoint_descriptor *)
            fn->fs_descs[BULKIN_DESC]);
    if (!ep_in) {
        usb_ep_disable(ep_out);
        return -ENOMEM;
    }

    ep_out->driver_data = fn;
    ep_in->driver_data = fn;
    fn->bulk_out = ep_out;
    fn->bulk_in = ep_in;

    if (gadget_is_dualspeed(gadget)) {
        fn->hs_bulkin_endpoint.bEndpointAddress =
            fn->fs_bulkin_endpoint.bEndpointAddress;
        fn->hs_bulkout_endpoint.bEndpointAddress =
            fn->fs_bulkout_endpoint.bEndpointAddress;
    }

    fn->cdev = c->cdev;
    fn->activated = 0;

    rc = rawbulk_register_tty(fn);
    if (rc < 0)
        printk(KERN_ERR "failed to register tty for %s\n", fn->longname);

    return rawbulk_bind_function(fn->transfer_id, f, ep_out, ep_in);
}

static void rawbulk_function_unbind(struct usb_configuration *c, struct
        usb_function *f) {
    struct rawbulk_function *fn = function_to_rawbulk(f);

    rawbulk_unregister_tty(fn);
    rawbulk_unbind_function(fn->transfer_id);
}

static void do_activate(struct work_struct *data) {
    struct rawbulk_function *fn = container_of(data, struct rawbulk_function,
            activator);
    int rc;
    printk(KERN_ERR "Rawbulk async work: tid %d to be %s has enabled? %d\n",
            fn->transfer_id, fn->activated? "activted": "detached",
            rawbulk_check_enable(fn));
    if (fn->activated) { /* enumerated */
        /* enabled endpoints */
        rc = usb_ep_enable(fn->bulk_out, ep_choose(fn->cdev->gadget,
                    &fn->hs_bulkout_endpoint, &fn->fs_bulkout_endpoint));
        if (rc < 0)
            return;
        rc = usb_ep_enable(fn->bulk_in, ep_choose(fn->cdev->gadget,
                    &fn->hs_bulkin_endpoint, &fn->fs_bulkin_endpoint));
        if (rc < 0) {
            usb_ep_disable(fn->bulk_out);
            return;
        }

        fn->bulk_out->driver_data = fn;
        fn->bulk_in->driver_data = fn;

        /* start rawbulk if enabled */
        if (rawbulk_check_enable(fn)) {
            wake_lock(&fn->keep_awake);
            rc = rawbulk_start_transactions(fn->transfer_id, fn->nups,
                    fn->ndowns, fn->upsz, fn->downsz, fn->splitsz);
            if (rc < 0)
                rawbulk_disable_function(fn);
			else if (fn->transfer_id == RAWBULK_TID_MODEM)
				schedule_delayed_work(&fn->query_work, HZ);
        }

        /* start tty io */
        rc = rawbulk_tty_alloc_request(fn);
        if (rc < 0)
            return;
        if (!rawbulk_check_enable(fn))
            rawbulk_tty_start_io(fn);
    } else { /* disconnect */
        printk(KERN_ERR "%s===>>>>disconnect\n",__func__);
        if (rawbulk_check_enable(fn)) {
            printk(KERN_ERR "%s====>>>>> this fn is enable\n",__func__);
			if (fn->transfer_id == RAWBULK_TID_MODEM)
		    {
                printk(KERN_ERR "%s====>>>usb cable is disconnected\n",__func__);
        		schedule_delayed_work(&fn->query_work, HZ);
                control_dtr(0);
            }                
            rawbulk_stop_transactions(fn->transfer_id);
            /* keep the enable state, so we can enable again in next time */
            //set_enable_state(fn, 0); 
            wake_unlock(&fn->keep_awake);
        } else
            rawbulk_tty_stop_io(fn);

        rawbulk_tty_free_request(fn);

        usb_ep_disable(fn->bulk_out);
        usb_ep_disable(fn->bulk_in);

        fn->bulk_out->driver_data = NULL;
        fn->bulk_in->driver_data = NULL;
    }
}

static int rawbulk_function_setalt(struct usb_function *f, unsigned intf,
        unsigned alt) {
    struct rawbulk_function *fn = function_to_rawbulk(f);
    fn->activated = 1;
    schedule_work(&fn->activator);
    return 0;
}

static void rawbulk_function_disable(struct usb_function *f) {
    struct rawbulk_function *fn = function_to_rawbulk(f);
    fn->activated = 0;
    schedule_work(&fn->activator);
}

int rawbulk_function_add(struct usb_configuration *c, int transfer_id) {
    int rc;
    struct rawbulk_function *fn = rawbulk_get_by_index(transfer_id);

    printk("add %s to config.\n", fn->longname);

    if (!fn)
        return -ENOMEM;

    rc = usb_string_id(c->cdev);
    if (rc < 0)
        return rc;

    fn->string_defs[0].id = rc;
    fn->function.name = fn->longname;

    if(get_bypass_status() == 0)
	{
        if(transfer_id != RAWBULK_TID_ETS)
		    fn->function.disabled = 1;
    }
    else
        fn->function.disabled = 0;
 	printk("%s - transfer_id %d disabled(%d)\n", __func__, fn->transfer_id,fn->function.disabled);
    if (fn->transfer_id == 0)
        fn->function.setup = modem_function_setup;//rawbulk_function_setup;
    else
	fn->function.setup = NULL;
    fn->function.bind = rawbulk_function_bind;
    fn->function.unbind = rawbulk_function_unbind;
    fn->function.set_alt = rawbulk_function_setalt;
    fn->function.disable = rawbulk_function_disable;

    INIT_WORK(&fn->activator, do_activate);
    INIT_WORK(&modem_control_work, do_modem_control);

    return usb_add_function(c, &fn->function);
}

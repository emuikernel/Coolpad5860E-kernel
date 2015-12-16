/*
   USB Driver for CBP

   Copyright (C) 2010 Karfield Chen <kfchen@via-telecom.com>

   This driver is free software; you can redistribute it and/or modify
   it under the terms of Version 2 of the GNU General Public License as
   published by the Free Software Foundation.

   Portions copied from the Option driver by Matthias Urlichs <smurf@smurf.noris.de>

History: ask VIA-Telecom HZ for more information

Work sponsored by: VIA-Telecom Kunlun Project

This driver exists because the "normal" serial driver doesn't work too well
with GSM modems. Issues:
- data loss -- one single Receive URB is not nearly enough
- nonstandard flow (Option devices) control
- controlling the baud rate doesn't make sense

Some of the "one port" devices actually exhibit multiple USB instances
on the USB bus. This is not a bug, these ports are used for different
device features.
*/

#define DRIVER_VERSION "v0.0.1"
#define DRIVER_AUTHOR "Karfield Chen <kfchen@via-telecom.com>"
#define DRIVER_DESC "USB Driver for CBP(Data, ETS, AT, PCV, LBS...), product of VIA-Telecom Kunlun project"

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#ifdef CONFIG_USB_BYPASS
#include <linux/usb/cdc.h>
#include "../misc/bypass.h"
#endif

#include <linux/usb/rawbulk.h>

/* Function prototypes */
static int cbp_probe(struct usb_serial *serial, const struct usb_device_id *id);
static int  cbp_open(struct tty_struct *tty, struct usb_serial_port *port);
static void cbp_close(struct usb_serial_port *port);
static int  cbp_startup(struct usb_serial *serial);
static void cbp_disconnect(struct usb_serial *serial);
static void cbp_release(struct usb_serial *serial);
static int  cbp_write_room(struct tty_struct *tty);

static void cbp_instat_callback(struct urb *urb);

#ifdef CONFIG_USB_ANDROID_RAWBULK
static int cbp_incepted_write(struct tty_struct *tty, struct usb_serial_port
        *port, const unsigned char *buf, int count);
#endif
static int cbp_write(struct tty_struct *tty, struct usb_serial_port *port,
        const unsigned char *buf, int count);
static int  cbp_chars_in_buffer(struct tty_struct *tty);
static void cbp_set_termios(struct tty_struct *tty,
        struct usb_serial_port *port, struct ktermios *old);
static void cbp_dtr_rts(struct usb_serial_port *port, int on);
static int  cbp_tiocmget(struct tty_struct *tty, struct file *file);
static int  cbp_tiocmset(struct tty_struct *tty, struct file *file,
        unsigned int set, unsigned int clear);
static int  cbp_send_setup(struct usb_serial_port *port);
#ifdef CONFIG_PM
static int  cbp_suspend(struct usb_serial *serial, pm_message_t message);
static int  cbp_resume(struct usb_serial *serial);
#endif
static int  usb_serial_cbp_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void usb_serial_cbp_disconnect(struct usb_interface *interface);
static void play_delayed(struct usb_serial_port *port);
int usb_serial_port_open(struct usb_serial_port *port);
int usb_serial_port_close(struct usb_serial_port *port);

#ifdef CONFIG_AP2MODEM_VIATELECOM
extern void sleep_modem(void);
extern void wake_modem(void);
#endif
/* Vendor and product IDs */
/* VIA-Telecom */
#define VIATELECOM_VENDOR_ID			0x15eb
#define VIATELECOM_PRODUCT_ID			0x0001

/*The number of ttyUSB[X] port used to data call*/
#define VIA_DATA_CALL_NUM (0)

static struct usb_device_id cbp_ids[] = {
    { USB_DEVICE(VIATELECOM_VENDOR_ID, VIATELECOM_PRODUCT_ID) }, /* VIA-Telecom CBP */
    { } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, cbp_ids);

static struct usb_driver cbp_driver = {
    .name       = "via-cbp",
    .probe      = usb_serial_cbp_probe,
    .disconnect = usb_serial_cbp_disconnect,
#ifdef CONFIG_PM
    .suspend    = usb_serial_suspend,
    .resume     = usb_serial_resume,
    .supports_autosuspend =	1,
#endif
    .id_table   = cbp_ids,
    .no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_driver cbp_1port_device = {
    .driver = {
        .owner =	THIS_MODULE,
        .name =		"cbp",
    },
    .description       = "VIA-Telecom AP/CP USB Connection",
    .usb_driver        = &cbp_driver,
    .id_table          = cbp_ids,
    .num_ports         = 1,
    .probe             = cbp_probe,
    .open              = cbp_open,
    .close             = cbp_close,
#ifndef CONFIG_USB_ANDROID_RAWBULK
    .write             = cbp_write,
#else
	.write 			   = cbp_incepted_write,
#endif
    .write_room        = cbp_write_room,
    .chars_in_buffer   = cbp_chars_in_buffer,
    .set_termios       = cbp_set_termios,
    .tiocmget          = cbp_tiocmget,
    .tiocmset          = cbp_tiocmset,
    .attach            = cbp_startup,
    .disconnect        = cbp_disconnect,
    .dtr_rts           = cbp_dtr_rts,
    .release           = cbp_release,
    .read_int_callback = cbp_instat_callback,
#ifdef CONFIG_PM
    .suspend           = cbp_suspend,
    .resume            = cbp_resume,
#endif
};

static int debug;

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 64
#define IN_BUFLEN 4096
#define OUT_BUFLEN (64 * 16)

struct cbp_port_private {
    /* Input endpoints and buffer for this port */
    struct urb *in_urbs[N_IN_URB];
    u8 *in_buffer[N_IN_URB];
    /* Output endpoints and buffer for this port */
    struct urb *out_urbs[N_OUT_URB];
    u8 *out_buffer[N_OUT_URB];
    unsigned long out_busy;		/* Bit vector of URBs in use */
    unsigned long out_busy2;

    struct usb_anchor delayed_anchor;
    int opened;

    /* Settings for the port */
    int rts_state;	/* Handshaking pins (outputs) */
    int dtr_state;
    int cts_state;	/* Handshaking pins (inputs) */
    int dsr_state;
    int dcd_state;
    int ri_state;

    unsigned long tx_start_time[N_OUT_URB];

#ifdef CONFIG_USB_ANDROID_RAWBULK
	spinlock_t incept_lock;
    unsigned int inception:1;
#endif
};

struct cbp_intf_private {
    spinlock_t suspend_lock;
    unsigned int suspended;
    int outurb_counts;
};

static int check_out_busy(int i, struct cbp_port_private *portdata) {
    if (i < 32)
        return test_bit(i, &portdata->out_busy);
    else
        return test_bit(i - 32, &portdata->out_busy2);
}

static int check_and_set_out_busy(int i, struct cbp_port_private *portdata) {
    if (i < 32)
        return test_and_set_bit(i, &portdata->out_busy);
    else
        return test_and_set_bit(i - 32, &portdata->out_busy2);
}

static int get_port_number(struct usb_serial_port *port) {
    if (!port || !port->serial)
        return -1;
    return (int)port->serial->interface->cur_altsetting->desc.bInterfaceNumber;
}

/* Functions used by new usb-serial code. */
#ifdef CONFIG_USB_BYPASS
static struct bypass_ops h_write_ops;
#endif

static int __init cbp_init(void)
{
    int retval;
    retval = usb_serial_register(&cbp_1port_device);
    if (retval)
        goto failed_1port_device_register;
    retval = usb_register(&cbp_driver);
    if (retval)
        goto failed_driver_register;

#ifdef CONFIG_USB_BYPASS
    bypass_register(&h_write_ops);
#endif

    printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
            DRIVER_DESC "\n");

    return 0;

failed_driver_register:
    usb_serial_deregister(&cbp_1port_device);
failed_1port_device_register:
    return retval;
}

static void __exit cbp_exit(void)
{
#ifdef CONFIG_USB_BYPASS
    bypass_unregister(1);
#endif
    usb_deregister(&cbp_driver);
    usb_serial_deregister(&cbp_1port_device);
}

module_init(cbp_init);
module_exit(cbp_exit);

static void cbp_set_termios(struct tty_struct *tty,
        struct usb_serial_port *port, struct ktermios *old_termios)
{
    dbg("%s", __func__);
    /* Doesn't support option setting */
    tty_termios_copy_hw(tty->termios, old_termios);
    cbp_send_setup(port);
}

static int cbp_tiocmget(struct tty_struct *tty, struct file *file)
{
    struct usb_serial_port *port = tty->driver_data;
    unsigned int value;
    struct cbp_port_private *portdata;

    portdata = usb_get_serial_port_data(port);

    value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
        ((portdata->dtr_state) ? TIOCM_DTR : 0) |
        ((portdata->cts_state) ? TIOCM_CTS : 0) |
        ((portdata->dsr_state) ? TIOCM_DSR : 0) |
        ((portdata->dcd_state) ? TIOCM_CAR : 0) |
        ((portdata->ri_state) ? TIOCM_RNG : 0);

    return value;
}

static int cbp_tiocmset(struct tty_struct *tty, struct file *file,
        unsigned int set, unsigned int clear)
{
    struct usb_serial_port *port = tty->driver_data;
    struct cbp_port_private *portdata;

    portdata = usb_get_serial_port_data(port);

    /* FIXME: what locks portdata fields ? */
    if (set & TIOCM_RTS)
        portdata->rts_state = 1;
    if (set & TIOCM_DTR)
        portdata->dtr_state = 1;

    if (clear & TIOCM_RTS)
        portdata->rts_state = 0;
    if (clear & TIOCM_DTR)
        portdata->dtr_state = 0;
    return cbp_send_setup(port);
}

#ifdef CONFIG_USB_BYPASS
static int bypass_write(int port_num, const unsigned char *buf, int count)
{
    struct bypass *bypass = bypass_get();
    struct usb_serial_port *port;

    if(bypass == NULL)
    {
        printk("%s - bypass_get error\n",__func__);
        return -1;
    }

    switch(port_num) {
        case 0: port = bypass->h_modem_port;break;
        case 1: port = bypass->h_ets_port;break;
        case 2: port = bypass->h_atc_port;break;
        case 3: port = bypass->h_pcv_port;break;
        case 4: port = bypass->h_gps_port;break;
        default: port = NULL;
    }

    if(port == NULL) {
        dbg("can NOT find port (num %d)\n", port_num);
        return -EIO;
    }

    return cbp_write(NULL, port, buf, count);
}

static int bypass_setflow(int port_num, unsigned int set, unsigned int clear)
{
    struct bypass *bypass = bypass_get();
    struct usb_serial_port *port;
    struct cbp_port_private *portdata;

    if(bypass == NULL)
    {
        printk("%s - bypass_get error\n",__func__);
        return -1;
    }

    switch(port_num) {
        case 0: port = bypass->h_modem_port;break;
        case 1: port = bypass->h_ets_port;break;
        case 2: port = bypass->h_atc_port;break;
        case 3: port = bypass->h_pcv_port;break;
        case 4: port = bypass->h_gps_port;break;
        default: port = NULL;
    }

    if(port == NULL) {
        dbg("can NOT find port (num %d)\n", port_num);
        return -EIO;
    }

    portdata = usb_get_serial_port_data(port);
    if(!portdata)
        return -ENOMEM;

    /* FIXME: what locks portdata fields ? */
    if (set & TIOCM_RTS)
        portdata->rts_state = 1;
    if (set & TIOCM_DTR)
        portdata->dtr_state = 1;

    if (clear & TIOCM_RTS)
        portdata->rts_state = 0;
    if (clear & TIOCM_DTR)
        portdata->dtr_state = 0;
    return cbp_send_setup(port);
}

static struct bypass_ops h_write_ops = {
    .h_write = bypass_write,
    .h_setflow = bypass_setflow,
};
#endif

/* Write */
static void cbp_outdat_callback(struct urb *urb);
static int cbp_write(struct tty_struct *tty, struct usb_serial_port *port,
        const unsigned char *buf, int count)
{
    struct cbp_port_private *portdata;
    int i;
    int left, todo;
    struct urb *this_urb = NULL; /* spurious */
    int err;
    //struct usb_serial *serial = port->serial;
    unsigned long flags = 0;
    struct cbp_intf_private *intfdata = NULL;

    if(!port && !port->serial && port->serial->disconnected)
        return -EIO;

    portdata = usb_get_serial_port_data(port);
    intfdata = port->serial->private;

    dbg("%s: write port %d (%d chars)\n", __func__, port->number, count);

    i = 0;
    left = count;
    for (i = 0; left > 0 && i < N_OUT_URB; i++) {
        todo = left;
        if (todo > OUT_BUFLEN)
            todo = OUT_BUFLEN;

        this_urb = portdata->out_urbs[i];
        if (check_and_set_out_busy(i, portdata)) {
            if(i == N_OUT_URB - 1)
                printk("%s - all out urbs(%d) has been used!\n", __func__, N_OUT_URB);
            if (time_before(jiffies,
                        portdata->tx_start_time[i] + 10 * HZ))
                continue;
            printk("%s - this urb(%p) has been submited last 10 seconds, unlink now!\n", __func__, this_urb);
            usb_unlink_urb(this_urb);
            clear_bit(i, &portdata->out_busy);
            usb_autopm_put_interface_async(port->serial->interface);				
            continue;
        }
        dbg("%s: endpoint %d buf %d", __func__,
                usb_pipeendpoint(this_urb->pipe), i);
#ifdef CONFIG_PM
        err = usb_autopm_get_interface_async(port->serial->interface);
        if(err < 0)
        {
            printk(KERN_ERR "%s - autopm get interface async error, %d\n", __func__, err);
            break;
        }
#endif
        /* send the data */
        memcpy(this_urb->transfer_buffer, buf, todo);
        this_urb->transfer_buffer_length = todo;

        spin_lock_irqsave(&intfdata->suspend_lock, flags);
        if((intfdata->suspended) && (port->serial->suspend_count) )
        {
            usb_anchor_urb(this_urb, &portdata->delayed_anchor);
            spin_unlock_irqrestore(&intfdata->suspend_lock, flags);
        }
        else
        {
            intfdata->outurb_counts++;
            spin_unlock_irqrestore(&intfdata->suspend_lock, flags);

            this_urb->dev = port->serial->dev;
            err = usb_submit_urb(this_urb, GFP_ATOMIC);
            if (err) {
                printk(KERN_ERR "usb_submit_urb %p (write bulk) failed "
                        "(%d)\n", this_urb, err);
                clear_bit(i, &portdata->out_busy);

                spin_lock_irqsave(&intfdata->suspend_lock, flags);
                intfdata->outurb_counts--;
                spin_unlock_irqrestore(&intfdata->suspend_lock, flags);
                continue;
            }
        }
        portdata->tx_start_time[i] = jiffies;
        buf += todo;
        left -= todo;
    }

    count -= left;
    dbg("%s: wrote (did %d)\n", __func__, count);
    return count;
}

static void cbp_indat_callback(struct urb *urb)
{
    int err;
    int endpoint;
    struct usb_serial_port *port;
    struct tty_struct *tty;
    unsigned char *data = urb->transfer_buffer;
    int status = urb->status;
	int jj;

    endpoint = usb_pipeendpoint(urb->pipe);
    port =  urb->context;

    if (status) {
		if(status != -2 )
        printk(KERN_ERR "%s: nonzero status: %d on endpoint %02x.\n",
                __func__, status, endpoint);
    } else {
        /* here we steal data to bypass but never push to tty */
#ifdef CONFIG_USB_BYPASS
        struct bypass *bypass = bypass_get();
        if((bypass != NULL) && (urb->actual_length)) {
            if((get_port_number(port) == 0) && (bypass->modem_status == 1)) {
                if(bypass->ops->g_modem_write) {
                    int i = 32;
                    void *buf = urb->transfer_buffer;
                    unsigned int left = urb->actual_length;
                    unsigned int wrote = 0;
                    while(i --) {
                        wrote = bypass->ops->g_modem_write(buf, left);
                        buf += wrote;
                        left -= wrote;
                        if(left == 0)
                            break;
                    }
                    if(left)
                        printk("%s - modem_write haven't over %d\n", __func__, left);
                }
                goto RESUBMIT_URB;
            } else if(((get_port_number(port) == 1) && (bypass->ets_status == 1)) ||
                    ((get_port_number(port) == 2) && (bypass->at_status == 1)) ||
                    ((get_port_number(port) == 3) && (bypass->pcv_status == 1)) ||
                    ((get_port_number(port) == 4) && (bypass->gps_status == 1)))
            {
                if(bypass->ops->g_write) {
                    bypass->ops->g_write(get_port_number(port),
                            urb->transfer_buffer, urb->actual_length);
                }
                //goto RESUBMIT_URB;
            }
        }
#endif
#if 1
        if(get_port_number(port) == 2){
            printk(KERN_ERR "##%s port 2 push %d bytes\n",__func__,urb->actual_length);
            for(jj = 0;jj< urb->actual_length;jj++)
                printk("%x ",*(data+jj));
            printk("\n");
        }

        if(get_port_number(port) == 4){
            printk(KERN_ERR "##%s port 4 push %d bytes\n",__func__,urb->actual_length);
            for(jj = 0;jj< urb->actual_length;jj++)
                printk("%x ",*(data+jj));
            printk("\n");
        }

#endif
        tty = tty_port_tty_get(&port->port);
        if (urb->actual_length && tty) {
            tty_buffer_request_room(tty, urb->actual_length);
            tty_insert_flip_string(tty, data, urb->actual_length);
            tty_flip_buffer_push(tty);
            if((get_port_number(port) == 2)|| (get_port_number(port) == 4))
                printk(KERN_ERR "%s port(%d)++++++++++++++++++++====len=%d\n",__func__,get_port_number(port),urb->actual_length);
        } else
            printk(KERN_INFO "%s: empty read urb received", __func__);
        tty_kref_put(tty);

#ifdef CONFIG_PM
        if(port->serial->suspend_count)
        {
			dbg("=====tang enter %s  suspend_count =0x%x \n", __func__,port->serial->suspend_count);
            return;
        }	
#endif

RESUBMIT_URB:
        /* Resubmit urb so we continue receiving */
        if (port->port.count && status != -ESHUTDOWN)
        {
            /* keep read alive in default timeout(3*HZ) */
            usb_mark_last_busy(interface_to_usbdev(port->serial->interface));

            err = usb_submit_urb(urb, GFP_ATOMIC);
            if (err)
                printk(KERN_ERR "%s: resubmit read urb failed. "
                        "(%d)\n", __func__, err);
        }
    }
    return;
}

static void cbp_outdat_callback(struct urb *urb)
{
#ifdef CONFIG_USB_BYPASS
    struct bypass *bypass = bypass_get();
#endif
    struct usb_serial_port *port;
    struct cbp_port_private *portdata;
    int i;
    struct cbp_intf_private *intfdata = NULL;

    dbg("%s status %d\n", __func__, urb->status);

    port =  urb->context;

#ifdef CONFIG_USB_BYPASS
    if(!((port == bypass->h_modem_port) && bypass->modem_status))
        usb_serial_port_softint(port);
#endif

#ifdef CONFIG_PM
    usb_autopm_put_interface_async(port->serial->interface);
#endif

    portdata = usb_get_serial_port_data(port);
    intfdata = port->serial->private;

    spin_lock(&intfdata->suspend_lock);
    intfdata->outurb_counts--;
    spin_unlock(&intfdata->suspend_lock);

    for (i = 0; i < N_OUT_URB; ++i) {
        if (portdata->out_urbs[i] == urb) {
            smp_mb__before_clear_bit();
            clear_bit(i, &portdata->out_busy);
            break;
        }
    }
}

static void cbp_instat_callback(struct urb *urb)
{
    int err;
    int status = urb->status;
    struct usb_serial_port *port =  urb->context;
    struct cbp_port_private *portdata = usb_get_serial_port_data(port);
    struct usb_serial *serial = port->serial;

    dbg("%s", __func__);
    dbg("%s: urb %p port %p has data %p", __func__, urb, port, portdata);

    if (status == 0) {
        struct usb_ctrlrequest *req_pkt =
            (struct usb_ctrlrequest *)urb->transfer_buffer;

        if (!req_pkt) {
            dbg("%s: NULL req_pkt\n", __func__);
            return;
        }
        if ((req_pkt->bRequestType == 0xA1) &&
                (req_pkt->bRequest == 0x20)) {
            int old_dcd_state;
            unsigned char signals = *((unsigned char *)
                    urb->transfer_buffer +
                    sizeof(struct usb_ctrlrequest));

            dbg("%s: signal x%x", __func__, signals);

            old_dcd_state = portdata->dcd_state;
            portdata->cts_state = 1;
            portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
            portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
            portdata->ri_state = ((signals & 0x08) ? 1 : 0);

            if (old_dcd_state && !portdata->dcd_state) {
                struct tty_struct *tty =
                    tty_port_tty_get(&port->port);
                if (tty && !C_CLOCAL(tty))
                    tty_hangup(tty);
                tty_kref_put(tty);
            }
        } else {
            dbg("%s: type %x req %x", __func__,
                    req_pkt->bRequestType, req_pkt->bRequest);
        }
    } else
        err("%s: error %d", __func__, status);

    /* Resubmit urb so we continue receiving IRQ data */
    if (status != -ESHUTDOWN && status != -ENOENT) {
        urb->dev = serial->dev;
        err = usb_submit_urb(urb, GFP_ATOMIC);
        if (err)
            dbg("%s: resubmit intr urb failed. (%d)",
                    __func__, err);
    }
}

static int cbp_write_room(struct tty_struct *tty)
{
    struct usb_serial_port *port = tty->driver_data;
    struct cbp_port_private *portdata;
    int i;
    int data_len = 0;
    struct urb *this_urb;

    portdata = usb_get_serial_port_data(port);

    for (i = 0; i < N_OUT_URB; i++) {
        this_urb = portdata->out_urbs[i];
        if (this_urb && !check_out_busy(i, portdata))
            data_len += OUT_BUFLEN;
    }

    dbg("%s: %d", __func__, data_len);
    return data_len;
}

static int cbp_chars_in_buffer(struct tty_struct *tty)
{
    struct usb_serial_port *port = tty->driver_data;
    struct cbp_port_private *portdata;
    int i;
    int data_len = 0;
    struct urb *this_urb;

    portdata = usb_get_serial_port_data(port);

    for (i = 0; i < N_OUT_URB; i++) {
        this_urb = portdata->out_urbs[i];
        /* FIXME: This locking is insufficient as this_urb may
           go unused during the test */
        if (this_urb && check_out_busy(i, portdata))
            data_len += this_urb->transfer_buffer_length;
    }
    dbg("%s: %d", __func__, data_len);
    return data_len;
}

static int cbp_probe(struct usb_serial *serial, const struct usb_device_id *id)
{
    struct cbp_intf_private *data = NULL;

    data = serial->private = kzalloc(sizeof(struct cbp_intf_private), GFP_KERNEL);
    if(!data)
    {
        printk(KERN_ERR "%s, alloc cbp_intf_private error\n", __func__);
        return -ENOMEM;
    }

    spin_lock_init(&data->suspend_lock);

	return 0;
}

static int cbp_open(struct tty_struct *tty, struct usb_serial_port *port)
{
    struct cbp_port_private *portdata;
    struct usb_serial *serial = NULL;
    int i, err;
    struct urb *urb;
    struct cbp_intf_private *intfdata = NULL;

    if(port)
        serial = port->serial;
    else
    {
        printk("cbp_open:port is null!\n");
        return -EIO;
    }

    if(!serial)
    {
        printk("cbp_open:serial is null!\n");
        return -EIO;
    }

    if(serial->disconnected)
    {
        printk("cbp_open:serial->disconnected is 1!\n");
        return -EIO;
    }

    printk(KERN_INFO "%s\n", __func__);

    portdata = usb_get_serial_port_data(port);
    intfdata = port->serial->private;

    /* Set some sane defaults */
    portdata->rts_state = 1;
    portdata->dtr_state = 1;

    /* Reset low level data toggle and start reading from endpoints */
    for (i = 0; i < N_IN_URB; i++) {
        urb = portdata->in_urbs[i];
        if (!urb)
            continue;
        if (urb->dev != serial->dev) {
            dbg("%s: dev %p != %p", __func__,
                    urb->dev, serial->dev);
            continue;
        }

        /*
         * make sure endpoint data toggle is synchronized with the
         * device
         */
        usb_clear_halt(urb->dev, urb->pipe);

        err = usb_submit_urb(urb, GFP_KERNEL);
        if (err) {
            dbg("%s: submit urb %d failed (%d) %d",
                    __func__, i, err,
                    urb->transfer_buffer_length);
        }
    }

    /* Reset low level data toggle on out endpoints */
    for (i = 0; i < N_OUT_URB; i++) {
        urb = portdata->out_urbs[i];
        if (!urb)
            continue;
        urb->dev = serial->dev;
        /* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
           usb_pipeout(urb->pipe), 0); */
    }
    //if(tty && (serial->interface->cur_altsetting->desc.bInterfaceNumber == 0))//hey add 11-1-7 for open web fail
    //    tty->low_latency = 1;
    cbp_send_setup(port);

#if 0
    if(serial->interface)
    {
        if(serial->interface->cur_altsetting)
        {
            if( VIA_DATA_CALL_NUM == (serial->interface->cur_altsetting->desc).bInterfaceNumber ){
                usb_autopm_get_interface(serial->interface);
            }
        }
    }
#endif
    spin_lock_irq(&intfdata->suspend_lock);
    portdata->opened = 1;
    spin_unlock_irq(&intfdata->suspend_lock);

    return 0;
}

static void cbp_dtr_rts(struct usb_serial_port *port, int on)
{
    struct usb_serial *serial = port->serial;
    struct cbp_port_private *portdata;

    dbg("%s", __func__);
    portdata = usb_get_serial_port_data(port);
    mutex_lock(&serial->disc_mutex);
    portdata->rts_state = on;
    portdata->dtr_state = on;
    if (serial->dev)
    {
        printk(KERN_ERR "%s===>on=%d\n",__func__,on);
        cbp_send_setup(port);
    }
    mutex_unlock(&serial->disc_mutex);
}


static void cbp_close(struct usb_serial_port *port)
{
    int i;
    struct usb_serial *serial;// = port->serial;
    struct cbp_port_private *portdata;
    struct cbp_intf_private *intfdata = NULL;

    if(!port)
        return;

    serial = port->serial;
    if(!serial)
        return;

    if(serial->disconnected)
        return;

    portdata = usb_get_serial_port_data(port);
    intfdata = serial->private;

    portdata->rts_state = 0;
    portdata->dtr_state = 0;

#if 0
    if(serial->interface)
    {
        if(serial->interface->cur_altsetting)
        {
            if(VIA_DATA_CALL_NUM == (serial->interface->cur_altsetting->desc).bInterfaceNumber){
                usb_autopm_put_interface(serial->interface);
            }
        }
    }
#endif

    if (serial->dev) {
        spin_lock_irq(&intfdata->suspend_lock);
        portdata->opened = 0;
        spin_unlock_irq(&intfdata->suspend_lock);

        mutex_lock(&serial->disc_mutex);
        if (!serial->disconnected) {
            cbp_send_setup(port);
            /* Stop reading/writing urbs */
            for (i = 0; i < N_IN_URB; i++)
                usb_kill_urb(portdata->in_urbs[i]);
            for (i = 0; i < N_OUT_URB; i++)
                usb_kill_urb(portdata->out_urbs[i]);
        }
        mutex_unlock(&serial->disc_mutex);
    }
    tty_port_tty_set(&port->port, NULL);
}

/* Helper functions used by cbp_setup_urbs */
static struct urb *cbp_setup_urb(struct usb_serial *serial, int endpoint,
        int dir, void *ctx, char *buf, int len,
        void (*callback)(struct urb *))
{
    struct urb *urb;

    if (endpoint == -1)
        return NULL;		/* endpoint not needed */

    urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
    if (urb == NULL) {
        dbg("%s: alloc for endpoint %d failed.", __func__, endpoint);
        return NULL;
    }

    /* Fill URB using supplied data. */
    usb_fill_bulk_urb(urb, serial->dev,
            usb_sndbulkpipe(serial->dev, endpoint) | dir,
            buf, len, callback, ctx);

    return urb;
}

/* Setup urbs */
static void cbp_setup_urbs(struct usb_serial *serial)
{
    int i, j;
    struct usb_serial_port *port;
    struct cbp_port_private *portdata;

    dbg("%s", __func__);

    for (i = 0; i < serial->num_ports; i++) {
        port = serial->port[i];
        portdata = usb_get_serial_port_data(port);

        /* Do indat endpoints first */
        for (j = 0; j < N_IN_URB; ++j) {
            portdata->in_urbs[j] = cbp_setup_urb(serial,
                    port->bulk_in_endpointAddress,
                    USB_DIR_IN, port,
                    portdata->in_buffer[j],
                    IN_BUFLEN, cbp_indat_callback);
        }

        /* outdat endpoints */
        for (j = 0; j < N_OUT_URB; ++j) {
            portdata->out_urbs[j] = cbp_setup_urb(serial,
                    port->bulk_out_endpointAddress,
                    USB_DIR_OUT, port,
                    portdata->out_buffer[j],
                    OUT_BUFLEN, cbp_outdat_callback);
        }
    }
}


/** send RTS/DTR state to the port.
 *
 * This is exactly the same as SET_CONTROL_LINE_STATE from the PSTN
 * CDC.
 */
static int cbp_send_setup(struct usb_serial_port *port)
{
    struct usb_serial *serial = port->serial;
    struct cbp_port_private *portdata;
    int ifNum;// = serial->interface->cur_altsetting->desc.bInterfaceNumber;
    dbg("%s", __func__);

    if(serial->interface)
    {
        if(serial->interface->cur_altsetting)
        {
            ifNum = serial->interface->cur_altsetting->desc.bInterfaceNumber;
        }
        else
            return-EIO;
    }
    else
        return -EIO;
    portdata = usb_get_serial_port_data(port);

    /* VIA-Telecom CBP DTR format */
    return usb_control_msg(serial->dev,
            usb_sndctrlpipe(serial->dev, 0),
            0x01, 0x40, portdata->dtr_state? 1: 0, ifNum,
            NULL, 0, USB_CTRL_SET_TIMEOUT);
}

static int cbp_startup(struct usb_serial *serial)
{
    int i, j, err;
    struct usb_serial_port *port;
    struct cbp_port_private *portdata;
    u8 *buffer;
#ifdef CONFIG_USB_BYPASS
    struct bypass *bypass = bypass_get();
#endif

    dbg("%s", __func__);

    /* Now setup per port private data */
    for (i = 0; i < serial->num_ports; i++) {
        port = serial->port[i];
        portdata = kzalloc(sizeof(*portdata), GFP_KERNEL);
        if (!portdata) {
            dbg("%s: kmalloc for cbp_port_private (%d) failed!.",
                    __func__, i);
            return 1;
        }

        init_usb_anchor(&portdata->delayed_anchor);
        for (j = 0; j < N_IN_URB; j++) {
            buffer = (u8 *)__get_free_page(GFP_KERNEL);
            if (!buffer)
                goto bail_out_error;
            portdata->in_buffer[j] = buffer;
        }

        for (j = 0; j < N_OUT_URB; j++) {
            buffer = kmalloc(OUT_BUFLEN, GFP_KERNEL);
            if (!buffer)
                goto bail_out_error2;
            portdata->out_buffer[j] = buffer;
        }

        usb_set_serial_port_data(port, portdata);

        if (!port->interrupt_in_urb)
            continue;
        err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
        if (err)
            dbg("%s: submit irq_in urb failed %d",
                    __func__, err);
    }

    cbp_setup_urbs(serial);

#ifdef CONFIG_USB_BYPASS
    port = serial->port[0];
    switch(get_port_number(port)) {
        case 0:
            bypass->h_modem_port = port;
            if(bypass->modem_status)
                usb_serial_port_open(bypass->h_modem_port);
            break;
        case 1:
            bypass->h_ets_port = port;
            /* after jump to loader, there only exist ets intf */
            if(bypass->ets_jump_flag == 1)
                usb_serial_port_open(bypass->h_ets_port);
            else if(bypass->ets_status)
                usb_serial_port_open(bypass->h_ets_port);
            break;
        case 2:
            bypass->h_atc_port = port;
            if(bypass->at_status)
                usb_serial_port_open(bypass->h_atc_port);
            break;
        case 3:
            bypass->h_pcv_port = port;
            if(bypass->pcv_status)
                usb_serial_port_open(bypass->h_pcv_port);
            break;
        case 4:
            bypass->h_gps_port = port;
            if(bypass->gps_status)
                usb_serial_port_open(bypass->h_gps_port);
            break;
        default:
            printk(KERN_ERR "fail to found bypass port by port %d\n", port->number);
    }
#endif

	return 0;

bail_out_error2:
    for (j = 0; j < N_OUT_URB; j++)
        kfree(portdata->out_buffer[j]);
bail_out_error:
    for (j = 0; j < N_IN_URB; j++)
        if (portdata->in_buffer[j])
            free_page((unsigned long)portdata->in_buffer[j]);
    kfree(portdata);
    return 1;
}

static void stop_read_write_urbs(struct usb_serial *serial)
{
    int i, j;
    struct usb_serial_port *port = NULL;
    struct cbp_port_private *portdata = NULL;

    /* Stop reading/writing urbs */
    for (i = 0; i < serial->num_ports; ++i) {
        port = serial->port[i];
        if(!port)
            continue;
        portdata = usb_get_serial_port_data(port);
        if(!portdata)
            continue;
        for (j = 0; j < N_IN_URB; j++)
        {
            if(!portdata->in_urbs[j])
                continue;
            printk(KERN_INFO "%s==>in_intfnum=%d j=%d\n",__func__,serial->interface->cur_altsetting->desc.bInterfaceNumber,j);
            printk(KERN_INFO "%s==>in_urb(%p) hcpriv(%p)\n",__func__,portdata->in_urbs[j],portdata->in_urbs[j]->hcpriv);
            usb_kill_urb(portdata->in_urbs[j]);
        }
        for (j = 0; j < N_OUT_URB; j++)
        {
            if(!portdata->out_urbs[j])
                continue;
            //printk(KERN_ERR "%s==>out_intfnum=%d j=%d\n",__func__,serial->interface->cur_altsetting->desc.bInterfaceNumber,j);
            //printk(KERN_ERR "%s==>out_urb(%p) hcpriv(%p)\n",__func__,portdata->out_urbs[j],portdata->out_urbs[j]->hcpriv);
            usb_kill_urb(portdata->out_urbs[j]);
        }
    }
}

static void cbp_disconnect(struct usb_serial *serial)
{
    struct usb_serial_port *port;
#ifdef CONFIG_USB_BYPASS
    struct bypass *bypass = bypass_get();
#endif
    dbg("%s", __func__);

    if(!serial)
        return ;

    port = serial->port[0];
    if (!port)
        return;

#ifdef CONFIG_USB_BYPASS
    switch(get_port_number(port)) {
        case 0:
            if(bypass->modem_status)
                usb_serial_port_close(port);
            bypass->h_modem_port = NULL;
            break;
        case 1:
            if(bypass->ets_status)
                usb_serial_port_close(port);
            bypass->h_ets_port = NULL;
            break;
        case 2:
            if(bypass->at_status)
                usb_serial_port_close(port);
            bypass->h_atc_port = NULL;
            break;
        case 3:
            if(bypass->pcv_status)
                usb_serial_port_close(port);
            bypass->h_pcv_port = NULL;
            break;
        case 4:
            if(bypass->gps_status)
                usb_serial_port_close(port);
            bypass->h_gps_port = NULL;
            break;
        default:
            break;
    }
#endif
    stop_read_write_urbs(serial);
}

static void cbp_release(struct usb_serial *serial)
{
    int i, j;
    struct usb_serial_port *port;
    struct cbp_port_private *portdata;

    dbg("%s", __func__);

    if(!serial)
        return ;

    /* Now free them */
    for (i = 0; i < serial->num_ports; ++i) {
        port = serial->port[i];
        portdata = usb_get_serial_port_data(port);

        for (j = 0; j < N_IN_URB; j++) {
            if (portdata->in_urbs[j]) {
                usb_free_urb(portdata->in_urbs[j]);
                free_page((unsigned long)
                        portdata->in_buffer[j]);
                portdata->in_urbs[j] = NULL;
            }
        }
        for (j = 0; j < N_OUT_URB; j++) {
            if (portdata->out_urbs[j]) {
                usb_free_urb(portdata->out_urbs[j]);
                kfree(portdata->out_buffer[j]);
                portdata->out_urbs[j] = NULL;
            }
        }
    }

    /* Now free per port private data */
    for (i = 0; i < serial->num_ports; i++) {
        port = serial->port[i];
        kfree(usb_get_serial_port_data(port));
    }
}

struct modem_work
{
    struct usb_serial * modem_serial;
    struct delayed_work line_state_query_work;
};

static struct modem_work mdm_work;

//static struct delayed_work line_state_query_work;
static struct usb_device *modem_dev;
static void cbp_query_line_state(struct work_struct *data) {
    int retval;
#ifdef CONFIG_USB_BYPASS
    struct bypass *bypass = bypass_get();
#endif
    int line_state = 0;
    struct usb_serial		*serial = NULL;
    struct modem_work       *work = NULL;

    work = container_of(data, struct modem_work, line_state_query_work.work);

    if(!work)
        return;
    else
        serial = work->modem_serial;


    if(!serial)
        return;


    if(serial->disconnected)
        return;

#ifdef CONFIG_USB_BYPASS
    if(!bypass || !modem_dev)
        return;
#endif
    retval = usb_control_msg(modem_dev, usb_rcvctrlpipe(modem_dev, 0),
            0x02, 0xC0, 0x00, 0,
            &line_state, 1, USB_CTRL_SET_TIMEOUT);

#ifdef CONFIG_USB_BYPASS
    if(retval >= 0) {
        bypass->modem_line_state = line_state;
    }
#endif 
    //schedule_delayed_work(&line_state_query_work, HZ);
    schedule_delayed_work(&mdm_work.line_state_query_work, 3*HZ);
}

#ifdef CONFIG_PM
static int cbp_suspend(struct usb_serial *serial, pm_message_t message)
{
    struct cbp_intf_private *intfdata = NULL;
    int i = 0;

    if(!serial)
        return -EINVAL;

    intfdata = serial->private;

    if(!intfdata)	
        return -EINVAL;

    dbg("%s entered", __func__);
    if(message.event & PM_EVENT_AUTO)
    {
        spin_lock_irq(&intfdata->suspend_lock);
        i = intfdata->outurb_counts;
        spin_unlock_irq(&intfdata->suspend_lock);

        if(i)
        {
            return -EBUSY;
        }
    }
#ifdef CONFIG_USB_BYPASS
    if(serial->interface->cur_altsetting->desc.bInterfaceNumber == VIA_DATA_CALL_NUM)
    {
        //cancel_delayed_work(&line_state_query_work);
        cancel_delayed_work(&mdm_work.line_state_query_work);
    }
#endif
    spin_lock_irq(&intfdata->suspend_lock);
    intfdata->suspended = 1;
    spin_unlock_irq(&intfdata->suspend_lock);

    stop_read_write_urbs(serial);
    return 0;
}

static int cbp_resume(struct usb_serial *serial)
{
    int err, i, j;
    struct usb_serial_port *port;
    struct urb *urb;
    struct cbp_port_private *portdata;
    struct cbp_intf_private *intfdata = NULL;

    if(!serial)
        return -EINVAL;

    intfdata = serial->private;

    if(!intfdata)
        return -EINVAL;

#if 1
    serial->suspend_count --;
    if(serial->suspend_count)
    {
        printk(KERN_INFO "%s: suspend not zero, return directly\n", __func__);
        return 0;
    }
#endif

#ifdef CONFIG_AP2MODEM_VIATELECOM
    wake_modem();
#endif

    /* get the interrupt URBs resubmitted unconditionally */
    for (i = 0; i < serial->num_ports; i++) {
        port = serial->port[i];
        if (!port->interrupt_in_urb) {
            dbg("%s: No interrupt URB for port %d\n", __func__, i);
            continue;
        }
        port->interrupt_in_urb->dev = serial->dev;
        err = usb_submit_urb(port->interrupt_in_urb, GFP_NOIO);
        dbg("Submitted interrupt URB for port %d (result %d)", i, err);
        if (err < 0) {
            err("%s: Error %d for interrupt URB of port%d",
                    __func__, err, i);
            return err;
        }
    }

    for (i = 0; i < serial->num_ports; i++) {
        /* walk all ports */
        port = serial->port[i];
        portdata = usb_get_serial_port_data(port);

        spin_lock_irq(&intfdata->suspend_lock);
        /* skip closed ports */
        if (!port->port.count || !portdata->opened) {
            spin_unlock_irq(&intfdata->suspend_lock);
            continue;
        }

        for (j = 0; j < N_IN_URB; j++) {
            urb = portdata->in_urbs[j];
            err = usb_submit_urb(urb, GFP_NOIO);
            if (err < 0) {
                spin_unlock_irq(&intfdata->suspend_lock);
                printk(KERN_ERR "%s: Error %d for bulk URB %d",
                        __func__, err, i);
                return err;
            }
        }
        play_delayed(port);
        spin_unlock_irq(&intfdata->suspend_lock);
    }

    spin_lock_irq(&intfdata->suspend_lock);
    intfdata->suspended = 0;
    spin_unlock_irq(&intfdata->suspend_lock);

    if(serial->interface)
    {
        if(serial->interface->cur_altsetting)
        {
#ifdef CONFIG_USB_BYPASS
            if(serial->interface->cur_altsetting->desc.bInterfaceNumber == VIA_DATA_CALL_NUM)
                //schedule_delayed_work(&line_state_query_work, HZ);
                schedule_delayed_work(&mdm_work.line_state_query_work, HZ);
#endif
        }
    }

    return 0;
}

static void play_delayed(struct usb_serial_port *port)
{
    struct cbp_port_private *portdata = NULL;
    struct urb *urb = NULL;
    int err = 0;
    struct cbp_intf_private *intfdata = NULL;

    if(!port)
        return ;
    portdata = usb_get_serial_port_data(port);
    if(!portdata)
        return ;
    intfdata = port->serial->private;
    if(!intfdata)
        return ;
    while ((urb = usb_get_from_anchor(&portdata->delayed_anchor))) {
        err = usb_submit_urb(urb, GFP_ATOMIC);
        if (!err)
            intfdata->outurb_counts++;
    }
}

#endif

#ifdef CONFIG_USB_ANDROID_RAWBULK
static int cbp_incepted_write(struct tty_struct *tty, struct usb_serial_port *port,
		   const unsigned char *buf, int count)
{
    int inception;
    struct cbp_port_private *portdata = usb_get_serial_port_data(port);
    spin_lock(&portdata->incept_lock);
    inception = portdata->inception;
    spin_unlock(&portdata->incept_lock);

    if (inception)
        return 0;

    return cbp_write(tty, port, buf, count);
}

#if defined(CONFIG_USB_SUSPEND) && defined(CONFIG_AP2MODEM_VIATELECOM)
extern void wake_modem(void);
extern void sleep_modem(void);
#endif

int cbp_rawbulk_intercept(void *private_data, int enable) {
    int rc = 0;
    int n;
    struct usb_serial *serial = private_data;
    struct usb_serial_port *port = serial->port[0];
    struct cbp_port_private *portdata = usb_get_serial_port_data(port);
    struct cbp_intf_private *intfdata = serial->private;
    struct usb_interface *interface = serial->interface;

    spin_lock(&portdata->incept_lock);
    if (portdata->inception == !!enable) {
        spin_unlock(&portdata->incept_lock);
        return -ENOENT;
    }
    spin_unlock(&portdata->incept_lock);

    if (enable) {
#if defined(CONFIG_USB_SUSPEND) && defined(CONFIG_AP2MODEM_VIATELECOM)
        wake_modem();
#endif
#ifdef CONFIG_PM
        rc = usb_autopm_get_interface(interface);
#endif
        /* Stop reading/writing urbs */
        for (n = 0; n < N_IN_URB; n++)
            usb_kill_urb(portdata->in_urbs[n]);
        for (n = 0; n < N_OUT_URB; n++)
            usb_kill_urb(portdata->out_urbs[n]);
    } else {
        int opened;
#ifdef CONFIG_PM
        usb_autopm_put_interface(interface);
#endif
        spin_lock_irq(&intfdata->suspend_lock);
        opened = portdata->opened;
        spin_unlock_irq(&intfdata->suspend_lock);

        printk("%s - intf suspended %d opened %d\n", __func__, intfdata->suspended, opened);
        if (!intfdata->suspended && opened) {
            for (n = 0; n < N_IN_URB; n ++) {
                struct urb *urb = portdata->in_urbs[n];
                if (!urb || !urb->dev)
                    continue;
                usb_clear_halt(urb->dev, urb->pipe);
                rc = usb_submit_urb(urb, GFP_KERNEL);
                if (rc < 0)
                    break;
            }
        }
    }

    if (!rc) {
        spin_lock(&portdata->incept_lock);
        portdata->inception = !!enable;
        spin_unlock(&portdata->incept_lock);
    }


    return rc;
}

#endif

static int usb_serial_cbp_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    int ret;
    struct usb_device *udev = interface_to_usbdev(interface);

    if(!udev)
        return -ENODEV;

    /* you can filter interface here */

#ifdef CONFIG_PM
    /* enable autopm */
    //usb_autopm_enable(interface);
    
    interface->needs_remote_wakeup = 0;
    //udev->autosuspend_disabled = 0;
    udev->autosuspend_delay = 3 * HZ;
    usb_enable_autosuspend(udev);
#endif /* CONFIG_PM */
    ret = usb_serial_probe(interface, id);
    if(interface) {
        if(interface->cur_altsetting)
        {
            if(!ret && (interface->cur_altsetting->desc.bInterfaceNumber == VIA_DATA_CALL_NUM)) {
#ifdef CONFIG_USB_BYPASS
                struct bypass *bypass = bypass_get();
#endif
                modem_dev = udev;


                mdm_work.modem_serial = usb_get_intfdata(interface);
#ifdef CONFIG_USB_BYPASS
                INIT_DELAYED_WORK(&mdm_work.line_state_query_work, cbp_query_line_state);


                bypass->modem_line_state = 0;//modify to 0 by guanshibo 12-24
                //INIT_DELAYED_WORK(&line_state_query_work, cbp_query_line_state);
                //bypass->modem_line_state = -1;
                //schedule_delayed_work(&line_state_query_work, HZ);
                schedule_delayed_work(&mdm_work.line_state_query_work, HZ);
#endif
            }
        }
    }
    return ret;
}

static void usb_serial_cbp_disconnect(struct usb_interface *interface) {
#ifdef CONFIG_USB_ANDROID_RAWBULK
    rawbulk_unbind_host_interface(interface);
#endif
    if(interface)
    {
        if(interface->cur_altsetting)
        {
#ifdef CONFIG_USB_BYPASS
            if(interface->cur_altsetting->desc.bInterfaceNumber == VIA_DATA_CALL_NUM)
                //cancel_delayed_work(&line_state_query_work);
                cancel_delayed_work(&mdm_work.line_state_query_work);
#endif
        }
    }
    usb_serial_disconnect(interface);
}

#ifdef CONFIG_USB_BYPASS
int usb_serial_port_close(struct usb_serial_port *port)
{
    struct usb_serial *serial = NULL;

    if(!port)
        return -EINVAL;

    serial = port->serial;

    if(!serial)
        return -EINVAL;

    if(!serial->interface)
        return -EINVAL;

    //if(serial->disconnected)
    //	return -EINVAL;

    if(port->port.count == 1) {
        stop_read_write_urbs(serial);
#ifdef CONFIG_PM
        usb_autopm_put_interface(serial->interface);
#endif
    }
    --port->port.count;
    return port->port.count;
}
EXPORT_SYMBOL_GPL(usb_serial_port_close);

int usb_serial_port_open(struct usb_serial_port *port)
{
    struct usb_serial *serial = NULL;
    int retval;
    unsigned long flags;

    if(!port) {
        dbg("invaild port\n");
        return -EINVAL;
    }
    serial = port->serial;
    if(!serial)
        return -EINVAL;
    printk("%s port_num %d count %d\n", __func__, port->number, port->port.count);
    /* force low_latency on so that our tty_push actually forces the data
       through, otherwise it is scheduled, and with high data rates (like
       with OHCI) data can get lost. */

    /* clear the throttle flags */
    spin_lock_irqsave(&port->lock, flags);
    //port->throttled = 0;
    //port->throttle_req = 0;
    ++port->port.count;
    spin_unlock_irqrestore(&port->lock, flags);

    if(port->port.count != 1) {
        dbg("port already open, do nothing\n");
        return -ENODEV;
    }

    /*
     * WARNING:
     *  do NOT lock port->mutex before usb_autopm_get_interface(), 'cause this
     *  will make a dead lock between autopm lock and port mutex
     * */
#ifdef CONFIG_PM
    retval = usb_autopm_get_interface(serial->interface);
    if(retval)
        goto exit;
#endif

    if (mutex_lock_interruptible(&port->mutex)) {
#ifdef CONFIG_PM
        usb_autopm_put_interface(serial->interface);
#endif
        retval = -ERESTARTSYS;
        goto exit;
    }

    retval = cbp_open(NULL, port);
    mutex_unlock(&port->mutex);

#ifdef CONFIG_PM
    //    usb_autopm_put_interface(serial->interface);
#endif

exit:
    if(retval)
        port->port.count --;

    return retval;
}
EXPORT_SYMBOL_GPL(usb_serial_port_open);
#endif /* CONFIG_USB_BYPASS */


MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");

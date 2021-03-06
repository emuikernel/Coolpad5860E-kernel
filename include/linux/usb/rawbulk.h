/*
 * Rawbulk Driver from VIA Telecom
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

#ifndef __RAWBULK_H__
#define __RAWBULK_H__

#include <linux/usb.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/wakelock.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/platform_device.h>

#define DEFAULT_UPSTREAM_COUNT          16
#define DEFAULT_DOWNSTREAM_COUNT        4
#define DEFAULT_UPSTREAM_BUFFER_SIZE    64
#define DEFAULT_DOWNSTREAM_BUFFER_SIZE  1024
#define DEFAULT_DOWNSTREAM_SPLIT_SIZE   64

#define INTF_DESC       0
#define BULKIN_DESC     1
#define BULKOUT_DESC    2
#define MAX_DESC_ARRAY  4

#define NAME_BUFFSIZE   64
#define MAX_ATTRIBUTES    10

#define MAX_TTY_RX          4
#define MAX_TTY_RX_PACKAGE  512
#define MAX_TTY_TX          8
#define MAX_TTY_TX_PACKAGE  64

struct rawbulk_function {
    int transfer_id;
    char *longname;
    char *shortname;
    struct device dev;

    /* Controls */
    spinlock_t lock;
    int enable: 1;
    int activated: 1; /* set when usb enabled */
    int tty_opened: 1;

    struct work_struct activator; /* asynic transaction starter */

    struct wake_lock keep_awake;

    /* USB Gadget related */
    struct usb_function function;
    struct usb_composite_dev *cdev;
    struct usb_ep *bulk_out, *bulk_in;

    int rts_state;		/* Handshaking pins (outputs) */
    int dtr_state;
    int cts_state;		/* Handshaking pins (inputs) */
    int dsr_state;
    int dcd_state;
    int ri_state;
    struct delayed_work query_work;

    /* TTY related */
    struct tty_struct *tty;
    struct tty_port port;
    spinlock_t tx_lock;
    struct list_head tx_free;
    struct list_head tx_inproc;
    spinlock_t rx_lock;
    struct list_head rx_free;
    struct list_head rx_inproc;
    struct list_head rx_throttled;
    unsigned int last_pushed;

    /* Transfer Controls */
    int nups;
    int ndowns;
    int upsz;
    int downsz;
    int splitsz;

    /* Descriptors and Strings */
    struct usb_descriptor_header *fs_descs[MAX_DESC_ARRAY];
    struct usb_descriptor_header *hs_descs[MAX_DESC_ARRAY];
    struct usb_string string_defs[2];
    struct usb_gadget_strings string_table;
    struct usb_gadget_strings *strings[2];
    struct usb_interface_descriptor interface;
    struct usb_endpoint_descriptor fs_bulkin_endpoint;
    struct usb_endpoint_descriptor hs_bulkin_endpoint;
    struct usb_endpoint_descriptor fs_bulkout_endpoint;
    struct usb_endpoint_descriptor hs_bulkout_endpoint;

    /* Sysfs Accesses */
    int max_attrs;
    struct device_attribute attr[MAX_ATTRIBUTES];
};

enum transfer_id {
    RAWBULK_TID_MODEM,
    RAWBULK_TID_ETS,
    RAWBULK_TID_AT,
    RAWBULK_TID_PCV,
    RAWBULK_TID_GPS,
    _MAX_TID
};

/* platform data struct for rawbulk */

struct usb_rawbulk_platform_data {
    struct rawbulk_fn_data {
        int upstream_transactions;
        int downstream_transactions;
        int upstream_buffersize;
        int downstream_buffersize;
        int downstream_splitcount;
    } fn_params[_MAX_TID];
};

typedef int (*rawbulk_intercept_t)(void *private_data, int enable);

struct rawbulk_function *rawbulk_get_by_index(int index);

/* rawbulk tty io */
int rawbulk_register_tty(struct rawbulk_function *fn);
void rawbulk_unregister_tty(struct rawbulk_function *fn);

int rawbulk_tty_stop_io(struct rawbulk_function *fn);
int rawbulk_tty_start_io(struct rawbulk_function *fn);
int rawbulk_tty_alloc_request(struct rawbulk_function *fn);
void rawbulk_tty_free_request(struct rawbulk_function *fn);

/* bind/unbind for gadget */
int rawbulk_bind_function(int transfer_id, struct usb_function *function,
        struct usb_ep *bulk_out, struct usb_ep *bulk_in);
void rawbulk_unbind_function(int trasfer_id);
int rawbulk_check_enable(struct rawbulk_function *fn);
void rawbulk_disable_function(struct rawbulk_function *fn);

int get_bypass_status(void);

/* bind/unbind host interfaces */
int rawbulk_bind_host_interface(struct usb_interface *interface,
        rawbulk_intercept_t inceptor, void *private_data);
void rawbulk_unbind_host_interface(struct usb_interface *interface);

/* operations for transactions */
int rawbulk_start_transactions(int transfer_id, int nups, int ndowns,
        int upsz, int downsz, int splitsz);
void rawbulk_stop_transactions(int transfer_id);
int rawbulk_halt_transactions(int transfer_id);
int rawbulk_resume_transactions(int transfer_id);

int rawbulk_suspend_host_interface(int transfer_id, pm_message_t message);
int rawbulk_resume_host_interface(int transfer_id);

int rawbulk_forward_ctrlrequest(const struct usb_ctrlrequest *ctrl);
int rawbulk_async_control_msg(const struct usb_ctrlrequest *ctrl, usb_complete_t completer);

/* statics and state */
int rawbulk_transfer_statistics(int transfer_id, char *buf);
int rawbulk_transfer_state(int transfer_id);

struct usb_device *rawbulk_get_cp(void);
void rawbulk_put_cp(struct usb_device *cp);

int control_dtr(int set); 
#endif /* __RAWBULK_HEADER_FILE__ */

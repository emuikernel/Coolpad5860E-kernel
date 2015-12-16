/*
 * Copyright (c) 2009-~ Hu bin
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * Author:       Shaoneng Wang
 * Created Time: Mon 08 Mar 2010 03:05:25 PM CST
 * File Name:    include/linux/bypass.h
 *
 * Description:  
 */
#ifndef __BYPASS_H__
#define __BYPASS_H__

/* circular buffer */
struct gs_buf {
    unsigned                buf_size;
    char                    *buf_buf;
    char                    *buf_get;
    char                    *buf_put;
};

/*
 * The port structure holds info for each port, one for each minor number
 * (and thus for each /dev/ node).
 */
struct gs_port {
    spinlock_t              port_lock;      /* guard port_* access */

    struct gserial          *port_usb;
    struct tty_struct       *port_tty;

    unsigned                open_count;
    bool                    openclose;      /* open/close in progress */
    u8                      port_num;

    wait_queue_head_t       close_wait;     /* wait for last close */

    struct list_head        read_pool;
    struct list_head        read_queue;
    unsigned                n_read;
    struct tasklet_struct   push;

    struct list_head        write_pool;
    struct gs_buf           port_write_buf;
    wait_queue_head_t       drain_wait;     /* wait while writes drain */

    /* REVISIT this state ... */
    struct usb_cdc_line_coding port_line_coding;    /* 8-N-1 etc */
};

struct bypass_ops {
    int (*h_write)(int port_num, const unsigned char *buf, int count);
    int (*h_setflow)(int port_num, unsigned int setbits, unsigned int clearbits);
    int (*g_write)(int port_num, const unsigned char *buf, int count);
#ifdef CONFIG_VIAUSBMODEM
    int (*h_modem_write)(const unsigned char *buf, int count);
#endif
    int (*g_modem_write)(const unsigned char *buf, int count);
    void (*acm_connect)(void);
    void (*acm_disconnect)(void);
    void (*bp_connect)(int port_num);
};

struct bypass {

    int ets_status;
    int at_status;
    int gps_status;
    int pcv_status;
    int modem_status;
    int modem_line_state;

    int ets_jump_flag;

    spinlock_t	lock;      /* guard bypass multi access */

#ifdef CONFIG_USB_SERIAL_VIATELECOM_CBP
    struct usb_serial_port *h_modem_port;
#endif
    struct usb_serial_port *h_ets_port;
    struct usb_serial_port *h_atc_port;
    struct usb_serial_port *h_gps_port;
    struct usb_serial_port *h_pcv_port;

    struct bypass_ops *ops;
};

extern int bypass_register(struct bypass_ops *ops); 
extern void bypass_unregister(int type);
extern struct bypass * bypass_get(void);
int usb_serial_port_close(struct usb_serial_port *port);
int usb_serial_port_open(struct usb_serial_port *port);
#endif /* __BYPASS_H__ */

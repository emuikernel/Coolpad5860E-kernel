/* 
 * usb bypass
 * this code for bypass from usb host to usb gadget 
 * author: shaoneng wang from via-telecom.
 */
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/string.h>

#include <linux/device.h>
#include <linux/usb.h>
#include <linux/usb/cdc.h>
//#include <linux/generic_serial.h>
#include <linux/tty.h>
#include <linux/usb/serial.h>
      

#include "bypass.h"

struct bypass *bypass;//this variable will across the bypass progress
//EXPORT_SYMBOL(bypass);

#ifdef CONFIG_VIAUSBMODEM
extern int cpbypass_modem_open(void);
extern void cpbypass_modem_close(void);
#endif /* CONFIG_VIAUSBMODEM */
//extern int bypass_port_assign(struct usb_serial_port *port, int index);
//extern int usb_serial_port_open(struct usb_serial_port *port);

static ssize_t ets_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int ret_val = 0;

	if (bypass->ets_status == 0)
		ret_val += sprintf(buf+ret_val, "tty\n");
	if (bypass->ets_status == 1)
		ret_val += sprintf(buf+ret_val, "gadget\n");
	return ret_val;

}

static ssize_t modem_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int ret_val = 0;

	if (bypass->modem_status == 0)
		ret_val += sprintf(buf+ret_val, "tty\n");
	if (bypass->modem_status == 1)
		ret_val += sprintf(buf+ret_val, "gadget\n");
	return ret_val;

}
static ssize_t at_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int ret_val = 0;

	if(bypass->at_status == 0)
		ret_val += sprintf(buf+ret_val, "tty\n");
	if(bypass->at_status == 1)
		ret_val += sprintf(buf+ret_val, "gadget\n");
	return ret_val;
}
static ssize_t gps_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int ret_val = 0;

	if(bypass->gps_status == 0)
		ret_val += sprintf(buf+ret_val, "tty\n");
	if(bypass->gps_status == 1)
		ret_val += sprintf(buf+ret_val, "gadget\n");
	return ret_val;
}
static ssize_t pcv_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int ret_val = 0;

	if(bypass->pcv_status == 0)
		ret_val += sprintf(buf+ret_val, "tty\n");
	if(bypass->pcv_status == 1)
		ret_val += sprintf(buf+ret_val, "gadget\n");
	return ret_val;
}
static ssize_t ets_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t n)
{
	if (!strncmp(buf, "tty", strlen("tty")))
	{
		if(!bypass->ets_status)//for avoid reclose again
			return n;
		bypass->ets_status = 0;
		usb_serial_port_close(bypass->h_ets_port);
	}
	if (!strncmp(buf, "gadget", strlen("gadget")))
	{
		if(bypass->h_ets_port)
		{
			if(bypass->ets_status)//for avoid reopen again
				return n;
			bypass->ets_status = 1;
			 if(!bypass->h_ets_port->port.count)
				usb_serial_port_open(bypass->h_ets_port);
			if(bypass->ops->bp_connect)
				bypass->ops->bp_connect(1);
		}
		else printk("bypass->h_ets_port is NULL, do nothing\n");
	}
	return n;


}

static ssize_t modem_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t n)
{
	if (!strncmp(buf, "tty", strlen("tty")))
	{
		if(!bypass->modem_status)
			return n;
		bypass->modem_status = 0;
#ifdef CONFIG_VIAUSBMODEM
		cpbypass_modem_close();
#else
        usb_serial_port_close(bypass->h_modem_port);
#endif /* CONFIG_VIAUSBMODEM */
		if(bypass->ops->acm_disconnect)
			bypass->ops->acm_disconnect();
		else
			printk("do nothing....acm_connect not register\n");
	}
	if (!strncmp(buf, "gadget", strlen("gadget")))
	{
		if(bypass->modem_status)
			return n;
		bypass->modem_status = 1;
#ifdef CONFIG_VIAUSBMODEM
		cpbypass_modem_open();
#else
        usb_serial_port_open(bypass->h_modem_port);
#endif /* CONFIG_VIAUSBMODEM */
		if(bypass->ops->acm_connect)
			bypass->ops->acm_connect();
		else
			printk("do nothing.....acm_disconnect not register \n");
	}
	return n;


}

static ssize_t at_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	if(!strncmp(buf, "tty", strlen("tty")))
	{
		if(!bypass->at_status)
			return n;
		usb_serial_port_close(bypass->h_atc_port);
		bypass->at_status = 0;
	}
	if(!strncmp(buf, "gadget", strlen("gadget")))
	{
		if(bypass->h_atc_port)
		{
			if(bypass->at_status)
				return n;
			bypass->at_status = 1;
			usb_serial_port_open(bypass->h_atc_port);
			if(bypass->ops->bp_connect)
				bypass->ops->bp_connect(2);
		}
		else printk("bypass->h_atc_port is NULL, do nothing\n");
	}
	return n;
}

static ssize_t gps_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	if(!strncmp(buf, "tty", strlen("tty")))
	{
		if(!bypass->gps_status)
			return n;
		usb_serial_port_close(bypass->h_gps_port);
		bypass->gps_status = 0;
	}
	if(!strncmp(buf, "gadget", strlen("gadget")))
	{
		if(bypass->h_gps_port)
		{
			if(bypass->gps_status)
				return n;
			bypass->gps_status = 1;
			usb_serial_port_open(bypass->h_gps_port);
			if(bypass->ops->bp_connect)
				bypass->ops->bp_connect(4);
		}
		else printk("bypass->h_gps_port is NULL, do nothing\n");
	}
	return n;
}

static ssize_t pcv_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	if(!strncmp(buf, "tty", strlen("tty")))
	{
		if(!bypass->pcv_status)
			return n;
		usb_serial_port_close(bypass->h_pcv_port);
		bypass->pcv_status = 0;
	}
	if(!strncmp(buf, "gadget", strlen("gadget")))
	{
		if(bypass->h_pcv_port)
		{
			if(bypass->pcv_status)
				return n;
			bypass->pcv_status = 1;
			usb_serial_port_open(bypass->h_pcv_port);
			if(bypass->ops->bp_connect)
				bypass->ops->bp_connect(3);
		}
		else printk("bypass->h_pcv_port is NULL, do nothing\n");

	}
	return n;
}
static struct kobj_attribute ets_attr = {  
        .attr   = {                             
                .name = __stringify(ets),     
                .mode = 0644,                   
        },                                      
        .show   = ets_show,                 
        .store  = ets_store,                
};

static struct kobj_attribute modem_attr = {  
        .attr   = {                             
                .name = __stringify(modem),     
                .mode = 0644,                   
        },                                      
        .show   = modem_show,                 
        .store  = modem_store,                
};

static struct kobj_attribute at_attr = {
	.attr	= {
		.name = __stringify(at),
		.mode = 0644,
	},
	.show	= at_show,
	.store	= at_store,
};

static struct kobj_attribute gps_attr = {  
        .attr   = {                             
                .name = __stringify(gps),     
                .mode = 0644,                   
        },                                      
        .show   = gps_show,                 
        .store  = gps_store,                
};
static struct kobj_attribute pcv_attr = {  
        .attr   = {                             
                .name = __stringify(pcv),     
                .mode = 0644,                   
        },                                      
        .show   = pcv_show,                 
        .store  = pcv_store,                
};
static struct attribute * g[] = {
	&ets_attr.attr,
	&at_attr.attr,
	&gps_attr.attr,
	&pcv_attr.attr,
	&modem_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = g,
};

struct kobject *bypass_obj;

static int bypass_sysfs_init(void)
{
	bypass_obj = kobject_create_and_add("usb_bypass",NULL);
	if (!bypass_obj)
		return -ENOMEM;
	return sysfs_create_group(bypass_obj, &attr_group);
}


int bypass_register(struct bypass_ops *ops) {

	printk("enter bypass_register\n");
	
	if(!(bypass && bypass->ops))
	{
		printk("bypass or bypass->ops is NULL\n");
		return -1;
	}

	if (ops->h_write)
		bypass->ops->h_write = ops->h_write;
    if (ops->h_setflow)
        bypass->ops->h_setflow = ops->h_setflow;
	if (ops->g_write)
		bypass->ops->g_write = ops->g_write;
#ifdef CONFIG_VIAUSBMODEM
	if (ops->h_modem_write)
		bypass->ops->h_modem_write = ops->h_modem_write;
#endif /* CONFIG_VIAUSBMODEM */
	if (ops->g_modem_write)
		bypass->ops->g_modem_write = ops->g_modem_write;
	if (ops->acm_connect)
		bypass->ops->acm_connect = ops->acm_connect;
	if (ops->acm_disconnect)
		bypass->ops->acm_disconnect = ops->acm_disconnect;
	if(ops->bp_connect)
		bypass->ops->bp_connect = ops->bp_connect;
	return 0;
}
EXPORT_SYMBOL(bypass_register);

void bypass_unregister(int type) {
	
	printk("enter bypass_unregister\n");
	switch(type)
    {
        case 1: bypass->ops->h_write = NULL;
                bypass->ops->h_setflow = NULL;
                break;
        case 2: bypass->ops->g_write = NULL;
                bypass->ops->bp_connect = NULL;
                break;
#ifdef CONFIG_VIAUSBMODEM
        case 3: bypass->ops->h_modem_write = NULL;break;
#endif /* CONFIG_VIAUSBMODEM */
        case 4: bypass->ops->g_modem_write = NULL;
                bypass->ops->acm_connect = NULL;
                bypass->ops->acm_disconnect = NULL;
                break;
        default:printk("bypass_unregister type error\n");
                break;
    };
}
EXPORT_SYMBOL(bypass_unregister);
		
struct bypass * bypass_get(void) {

//	printk("enter bypass_get\n");
	
	if(!(bypass && bypass->ops))
	{
		printk("--error, bypass or bypass->ops is NULL\n");
		return NULL;
	}
	else
		return bypass;
}
EXPORT_SYMBOL(bypass_get);


static int __init bypass_init(void)
{
	printk("enter bypass_init\n");
	bypass = kzalloc(sizeof(struct bypass), GFP_KERNEL);
	if( bypass == NULL)
		return -ENOMEM;
	bypass->ops = kzalloc(sizeof(struct bypass_ops), GFP_KERNEL);
	if( bypass->ops == NULL)
		return -ENOMEM;
	spin_lock_init(&bypass->lock);
	bypass_sysfs_init();
	return 0;
}

//module_init(bypass_init);
subsys_initcall(bypass_init);

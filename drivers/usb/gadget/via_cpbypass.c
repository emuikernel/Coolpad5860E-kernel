/*
 * drivers/usb/gadget/via_cpbypass.c
 * Copyright (C) 2010 YULONG Company.
 * Author: YULONG Company
 *
 * This program is free dispware; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free dispware Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#ifdef CONFIG_USB_ANDROID_RAWBULK
#include <linux/usb/rawbulk.h>
#endif

#include "gadget_chips.h"

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */
#include "usbstring.c"
#include "config.c"
#include "epautoconf.c"


MODULE_AUTHOR("Shaoneng Wang");
MODULE_DESCRIPTION("VIA TELECOM CP BYPASS");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static const char longname[] = "Gadget VIA TELECOM";

/* Default vendor and product IDs, overridden by platform data */
#define VENDOR_ID		0x1EBF
#define PRODUCT_ID		0x60FF

struct usb_composite_dev *cpbypass_cdev;

//static atomic_t adb_enable_excl;

/* string IDs are assigned dynamically */

#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2
#define STRING_CONF_IDX			3
#define STRING_INTF0_IDX		4
#define STRING_INTF1_IDX		5
#define STRING_INTF2_IDX		6
#define STRING_INTF3_IDX		7
#define STRING_INTF4_IDX		8

static char sn[33] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00};

/* String Table */
static struct usb_string strings_dev[] = {
	/* These dummy values should be overridden by platform data */
	[STRING_MANUFACTURER_IDX].s = "VIA Telecom",
	[STRING_PRODUCT_IDX].s = "CDS7",
    [STRING_SERIAL_IDX].s = sn,
/*	[STRING_CONF_IDX].s = "CONF1",
	[STRING_INTF0_IDX].s = "INTF0",
	[STRING_INTF1_IDX].s = "INTF1",
	[STRING_INTF2_IDX].s = "INTF2",
	[STRING_INTF3_IDX].s = "INTF3",
	[STRING_INTF4_IDX].s = "INTF4",
*/	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength              = sizeof(device_desc),
	.bDescriptorType      = USB_DT_DEVICE,
	.bcdUSB               = __constant_cpu_to_le16(0x0200),
	.bDeviceClass		=0xef,
	.bDeviceSubClass	=0x2,
	.bDeviceProtocol	=0x1,
//	.bDeviceClass         = 0,
//	.bDeviceSubClass      = 0,
//	.bDeviceProtocol      = 0,  
	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(PRODUCT_ID),
	.bcdDevice            = __constant_cpu_to_le16(0xffff),
	.bNumConfigurations   = 1,
};

extern int rawbulk_function_add(struct usb_configuration *c, int transfer_id);
static int __init cpbypass_bind_config(struct usb_configuration *c)
{
	int ret;
	printk("cpbypass_bind_config\n");

	ret = rawbulk_function_add(c, RAWBULK_TID_ETS);
	if (ret < 0)
		return ret;
	ret = rawbulk_function_add(c, RAWBULK_TID_MODEM);
	if (ret < 0)
		return ret;
	ret = rawbulk_function_add(c, RAWBULK_TID_AT);
	if (ret < 0)
		return ret;
	ret = rawbulk_function_add(c, RAWBULK_TID_PCV);
	if (ret < 0)
		return ret;
	ret = rawbulk_function_add(c, RAWBULK_TID_GPS);
	return ret;
}

static struct usb_configuration cpbypass_config_driver = {
	.label		= "CONF1",
	.bind		= cpbypass_bind_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower	= 0xFA,//CONFIG_USB_GADGET_VBUS_DRAW / 2,
};

void android_enable_function(struct usb_function *f, int enable)
{
	int disable = !enable;
	int product_id;
	struct usb_function		*func;


	if (!!f->disabled != disable) {
			list_for_each_entry(func, &cpbypass_config_driver.functions, list) {
                if(!get_bypass_status()){
                    printk(KERN_ERR "bypass all is selected !disable=%d\n",!disable);
				    if (!strcmp(func->name, "rawbulk-ets")|| 
                        !strcmp(func->name, "rawbulk-at")|| 
                        !strcmp(func->name, "rawbulk-modem")||
                        !strcmp(func->name, "rawbulk-pcv")|| 
                        !strcmp(func->name, "rawbulk-gps")) {
					        usb_function_set_enabled(func, !disable);
                    }
                }
                else{
                    printk(KERN_ERR "bypass only is selected !disable=%d\n",!disable);
                    if (!strcmp(func->name, "rawbulk-at")|| 
                        !strcmp(func->name, "rawbulk-modem")|| 
                        !strcmp(func->name, "rawbulk-pcv")|| 
                        !strcmp(func->name, "rawbulk-gps")) {
					        usb_function_set_enabled(func, !disable);
                    }
                }
            }
		if (cpbypass_cdev)
			cpbypass_cdev->desc.idProduct = device_desc.idProduct;
		usb_composite_force_reset(cpbypass_cdev);
	}
}

static int __init cpbypass_bind(struct usb_composite_dev *cdev)
{
	struct usb_gadget	*gadget = cdev->gadget;
	int			gcnum;
	int			id;
	int			ret;

	printk("cpbypass_bind\n");

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;

	/* register our configuration */
	ret = usb_add_config(cdev, &cpbypass_config_driver);
	if (ret) {
		printk(KERN_ERR "usb_add_config failed\n");
		return ret;
	}

	gcnum = usb_gadget_controller_number(gadget);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0200 + gcnum);
	else {
		/* gadget zero is so simple (for now, no altsettings) that
		 * it SHOULD NOT have problems with bulk-capable hardware.
		 * so just warn about unrcognized controllers -- don't panic.
		 *
		 * things like configuration and altsetting numbering
		 * can need hardware-specific attention though.
		 */
		pr_warning("%s: controller '%s' not recognized\n",
			longname, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

//	usb_gadget_set_selfpowered(gadget);
	cpbypass_cdev = cdev;
//	cpbypass_usb_set_connected(1);

	return 0;
}

static struct usb_composite_driver cpbypass_usb_driver = {
	.name		= "cpbypass_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= cpbypass_bind,
    .enable_function = android_enable_function,
};


static int __init init(void)
{

	printk("cpbypass init\n");

	return  usb_composite_register(&cpbypass_usb_driver);
}
module_init(init);

static void __exit cleanup(void)
{
	usb_composite_unregister(&cpbypass_usb_driver);
}
module_exit(cleanup);

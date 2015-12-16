/*
 * Gadget Driver for Android
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
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


#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/switch.h>
#include <linux/usb/android_composite.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/wakelock.h>


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
#include "composite.c"

MODULE_AUTHOR("YuLong");
MODULE_DESCRIPTION("Coolpad Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static const char longname[] = "Gadget Android";

/* Default vendor and product IDs, overridden by platform data */
#define VENDOR_ID		            0x1EBF

#define SINGLE_UMS_PRODUCT_ID		0x6003
#define UMS_ADB_PRODUCT_ID		    0x603A
#define SINGLE_MTP_PRODUCT_ID		0x6004
#define MTP_ADB_PRODUCT_ID		    0x603B
#define SINGLE_RNDIS_PRODUCT_ID		0x601A
#define RNDIS_ADB_PRODUCT_ID		0x601A

/* usb???￡???????¨???￥
 * add by yanghaishan. */
enum usb_mode{
    USB_MODE_SUIT = 0, 
    USB_MODE_MTP,           
    USB_MODE_UDISK,
	USB_MODE_NET,
};

struct android_dev {
	struct usb_composite_dev *cdev;
	struct usb_configuration *config;
	int num_products;
	struct android_usb_product *products;
	int num_functions;
	char **functions;

	int product_id;
	int version;
    	struct wake_lock wake_lock;//add by yanghaishan,move usb_mass_storage wake lock to here,100914
};

static struct android_dev *_android_dev;

bool iNandBoot = false;//是否为inand下载模式

enum usb_mode g_usb_mode;//the mode of usb

bool is_usb_mode_change = false;//是否usb模式切换

#define MAX_USB_SERIAL_LEN 33
static char sn[MAX_USB_SERIAL_LEN];
static char device_name[MAX_USB_SERIAL_LEN];

//add by yanghaishan,100810
typedef struct device_params_head
{
    char c8SyncByte[10];     //tag="DEVICE"
    char c8DeviceName[32];   //device name
    char c8BSP[32];          //BL version:CP9130_P0_001_20100710
    char c8ESN[32];          //device ESN
    char c8SN[32];           //device SN
}*pDevice_params_head;


/* string IDs are assigned dynamically */

#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

/* String Table */
static struct usb_string strings_dev[] = {
	/* These dummy values should be overridden by platform data */
	[STRING_MANUFACTURER_IDX].s = "YuLong",
	[STRING_PRODUCT_IDX].s = device_name,
	[STRING_SERIAL_IDX].s = sn,
	{  }			/* end of list */
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
	.bDeviceClass         = USB_CLASS_PER_INTERFACE,
	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(SINGLE_UMS_PRODUCT_ID),//default is suit mode
	.bcdDevice            = __constant_cpu_to_le16(0xffff),
	.bNumConfigurations   = 1,
};

static struct list_head _functions = LIST_HEAD_INIT(_functions);
static int _registered_function_count = 0;

static struct android_usb_function *get_function(const char *name)
{
	struct android_usb_function	*f;
	list_for_each_entry(f, &_functions, list) {
		if (!strcmp(name, f->name))
			return f;
	}
	return 0;
}

static void bind_functions(struct android_dev *dev)
{
	struct android_usb_function	*f;
	char **functions = dev->functions;
	int i;

	for (i = 0; i < dev->num_functions; i++) {
		char *name = *functions++;
		f = get_function(name);
		if (f)
			f->bind_config(dev->config);
		else
			printk(KERN_ERR "function %s not found in bind_functions\n", name);
	}
}

static int android_bind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;

	printk(KERN_DEBUG "android_bind_config\n");
	dev->config = c;
	
	g_usb_mode = USB_MODE_SUIT;//coolpad suit mode

	/* bind our functions if they have all registered */
	if (_registered_function_count == dev->num_functions)
		bind_functions(dev);

	return 0;
}

static int android_setup_config(struct usb_configuration *c,
		const struct usb_ctrlrequest *ctrl);

static struct usb_configuration android_config_driver = {
	.label		= "coolpad_usb",
	.bind		= android_bind_config,
	.setup		= android_setup_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower	= 0xFA, /* 500ma */
};

static int android_setup_config(struct usb_configuration *c,
		const struct usb_ctrlrequest *ctrl)
{
	int i;
	int ret = -EOPNOTSUPP;

	for (i = 0; i < android_config_driver.next_interface_id; i++) {
		if (android_config_driver.interface[i]->setup) {
			ret = android_config_driver.interface[i]->setup(
				android_config_driver.interface[i], ctrl);
			if (ret >= 0)
				return ret;
		}
	}
	return ret;
}

static int product_has_function(struct android_usb_product *p,
		struct usb_function *f)
{
	char **functions = p->functions;
	int count = p->num_functions;
	const char *name = f->name;
	int i;

	for (i = 0; i < count; i++) {
		if (!strcmp(name, *functions++))
			return 1;
	}
	return 0;
}

static int product_matches_functions(struct android_usb_product *p)
{
	struct usb_function		*f;
	list_for_each_entry(f, &android_config_driver.functions, list) {
		if (product_has_function(p, f) == !!f->disabled)
			return 0;
	}
	return 1;
}

static int get_product_id(struct android_dev *dev)
{
	struct android_usb_product *p = dev->products;
	int count = dev->num_products;
	int i;

	if (p) {
		for (i = 0; i < count; i++, p++) {
			if (product_matches_functions(p))
				return p->product_id;
		}
	}
	/* use default product ID */
	return dev->product_id;
}

static int android_bind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct usb_gadget	*gadget = cdev->gadget;
	int			gcnum = 0;
	int id = 0;
	int product_id = 0;
	int ret = 0;

	printk(KERN_INFO "android_bind\n");

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
	ret = usb_add_config(cdev, &android_config_driver);
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

	usb_gadget_set_selfpowered(gadget);
	dev->cdev = cdev;
	product_id = get_product_id(dev);
	device_desc.idProduct = __constant_cpu_to_le16(product_id);
	cdev->desc.idProduct = device_desc.idProduct;

	return 0;
}

static struct usb_composite_driver android_usb_driver = {
	.name		= "android_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= android_bind,
	.enable_function = android_enable_function,
};

void android_register_function(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;

	printk(KERN_INFO "android_register_function %s\n", f->name);
	list_add_tail(&f->list, &_functions);
	_registered_function_count++;

	/* bind our functions if they have all registered
	 * and the main driver has bound.
	 */
	if (dev && dev->config && _registered_function_count == dev->num_functions)
		bind_functions(dev);
}

void android_enable_function(struct usb_function *f, int enable)
{
	struct android_dev *dev = _android_dev;
	int disable = !enable;
	int product_id;

	if (!!f->disabled != disable) {
		usb_function_set_enabled(f, !disable);

#ifdef CONFIG_USB_ANDROID_RNDIS
		if (!strcmp(f->name, "rndis")) {
			struct usb_function		*func;

			/* We need to specify the COMM class in the device descriptor
			 * if we are using RNDIS.
			 */
			if (enable)
#ifdef CONFIG_USB_ANDROID_RNDIS_WCEIS
				dev->cdev->desc.bDeviceClass = USB_CLASS_WIRELESS_CONTROLLER;
#else
				dev->cdev->desc.bDeviceClass = USB_CLASS_COMM;
#endif
			else
				dev->cdev->desc.bDeviceClass = USB_CLASS_PER_INTERFACE;

			/* Windows does not support other interfaces when RNDIS is enabled,
			 * so we disable UMS and MTP when RNDIS is on.
			 */
			list_for_each_entry(func, &android_config_driver.functions, list) {
				if (!strcmp(func->name, "usb_mass_storage")
					|| !strcmp(func->name, "mtp")) {
					usb_function_set_enabled(func, !enable);
				}
			}

		}
#endif

		if(enable)
		{//adb enable
			switch(g_usb_mode)
			{
				case USB_MODE_SUIT:
				case USB_MODE_UDISK:
				{
					device_desc.idProduct =__constant_cpu_to_le16(UMS_ADB_PRODUCT_ID);	
					break;	
				}
				case USB_MODE_MTP:
				{
					device_desc.idProduct =__constant_cpu_to_le16(MTP_ADB_PRODUCT_ID);
					break;
				}
				case USB_MODE_NET:
				{
					device_desc.idProduct =__constant_cpu_to_le16(RNDIS_ADB_PRODUCT_ID);
					break;
				}

				default:
					device_desc.idProduct =__constant_cpu_to_le16(UMS_ADB_PRODUCT_ID);
					break;
			}
		}
		else
		{//adb disable
			switch(g_usb_mode)
			{
				case USB_MODE_SUIT:
				case USB_MODE_UDISK:
				{
					device_desc.idProduct =__constant_cpu_to_le16(SINGLE_UMS_PRODUCT_ID);	
					break;	
				}
				case USB_MODE_MTP:
				{
					device_desc.idProduct =__constant_cpu_to_le16(SINGLE_MTP_PRODUCT_ID);
					break;
				}
				case USB_MODE_NET:
				{
					device_desc.idProduct =__constant_cpu_to_le16(SINGLE_RNDIS_PRODUCT_ID);
					break;
				}
				
				default:
					device_desc.idProduct =__constant_cpu_to_le16(SINGLE_UMS_PRODUCT_ID);
					break;
			}
		}


		if (dev->cdev)
			dev->cdev->desc.idProduct = device_desc.idProduct;
		is_usb_mode_change = true;
		usb_composite_force_reset(dev->cdev);
		is_usb_mode_change = false;
	}
}

/*usb?￡ê??D??éè±?′ò?a*/
static int device_mode_change_open(struct inode *inode, struct file *file)
{
	return 0;
}

/*usb?￡ê??D??éè±?êí·?*/
static int device_mode_change_release(struct inode *inode, struct file *file)
{
	return 0;
}

#define ENABLE 1
#define DISABLE 0
#define MAX_DEVICE_NAME_SIZE  30

/**********************
* Function : usb?￡ê??D??
* 
* Input : 
           file : device handle
           buffer : command
           count : command size
           ppos : offset
 * Output :
             none
 *
 * Return : 1 sueccess,other fail
*/
static ssize_t device_mode_change_write(struct file *file, const char __user *buffer,
			 size_t count, loff_t *ppos)
{
	unsigned char cmd[MAX_DEVICE_NAME_SIZE + 1] = {0};
	unsigned int cnt = MAX_DEVICE_NAME_SIZE;
	struct usb_function		*func = NULL;
	int adb_enabled = 0;

	if (count <= 0)
		return 0;

	if (cnt > count)
		cnt = count;

	if (copy_from_user(cmd, buffer, cnt))
		return -EFAULT;
	cmd[cnt] = 0;
	printk(KERN_INFO "USB!%s:%s\n",__FUNCTION__,cmd);


	if(strcmp(cmd,"mtp") == 0)
	{
		
		if(g_usb_mode == USB_MODE_MTP)
			return 0;

		//usb_composite_force_reset(_android_dev->cdev);/*call this function will cause the mtp_disable null point,modify by yanghaishan,20110521*/
		unsigned long			flags = 0;

		g_usb_mode = USB_MODE_MTP;//coolpad mtp mode		
		list_for_each_entry(func, &android_config_driver.functions, list)
		{
			//if(func->name)
			{
				//printk(KERN_ERR "USB!device_mode_change_write:func->name=%s\n", func->name);
				if (!strcmp(func->name, "usb_mass_storage"))
				{
					usb_function_set_enabled(func, DISABLE);
				}
				if (!strcmp(func->name, "mtp"))
				{
					usb_function_set_enabled(func, ENABLE);
				}
				if (!strcmp(func->name, "rndis"))
				{
					usb_function_set_enabled(func, DISABLE);
				}
		
				if (!strcmp(func->name, "adb"))
				{
					if(!func->disabled)
						adb_enabled = 1;
				}
			}
		}
		if (adb_enabled)
			device_desc.idProduct =
				__constant_cpu_to_le16(MTP_ADB_PRODUCT_ID);
		else
			device_desc.idProduct =
				__constant_cpu_to_le16(SINGLE_MTP_PRODUCT_ID);
		if (_android_dev->cdev)
			_android_dev->cdev->desc.idProduct = device_desc.idProduct;
		is_usb_mode_change = true;
		usb_composite_force_reset(_android_dev->cdev);
	}
	if(strcmp(cmd,"suit") == 0)
	{
		if(g_usb_mode == USB_MODE_SUIT)
			return 0;
			
		g_usb_mode = USB_MODE_SUIT;//coolpad suit mode

		list_for_each_entry(func, &android_config_driver.functions, list)
		{
			if (!strcmp(func->name, "usb_mass_storage"))
			{
				usb_function_set_enabled(func, ENABLE);
			}
			if (!strcmp(func->name, "mtp"))
			{
				usb_function_set_enabled(func, DISABLE);
			}
			if (!strcmp(func->name, "rndis"))
			{
				usb_function_set_enabled(func, DISABLE);
			}
			
			if (!strcmp(func->name, "adb"))
			{
				if(!func->disabled)
					adb_enabled = 1;
			}
		}


		if (adb_enabled)
			device_desc.idProduct =
				__constant_cpu_to_le16(UMS_ADB_PRODUCT_ID);
		else
			device_desc.idProduct =
				__constant_cpu_to_le16(SINGLE_UMS_PRODUCT_ID);
		if (_android_dev->cdev)
			_android_dev->cdev->desc.idProduct = device_desc.idProduct;
		is_usb_mode_change = true;
		usb_composite_force_reset(_android_dev->cdev);
	}

	if(strcmp(cmd,"udisk") == 0)
	{
		if(g_usb_mode == USB_MODE_UDISK)
			return 0;
			
		g_usb_mode = USB_MODE_UDISK;//coolpad udisk mode
		
		list_for_each_entry(func, &android_config_driver.functions, list)
		{
			if (!strcmp(func->name, "usb_mass_storage"))
			{
				usb_function_set_enabled(func, ENABLE);
			}
			if (!strcmp(func->name, "mtp"))
			{
				usb_function_set_enabled(func, DISABLE);
			}
			if (!strcmp(func->name, "rndis"))
			{
				usb_function_set_enabled(func, DISABLE);
			}
			if (!strcmp(func->name, "adb"))
			{
				if(!func->disabled)
					adb_enabled = 1;
			}
		}


		if (adb_enabled)
			device_desc.idProduct =
				__constant_cpu_to_le16(UMS_ADB_PRODUCT_ID);
		else
			device_desc.idProduct =
				__constant_cpu_to_le16(SINGLE_UMS_PRODUCT_ID);
		if (_android_dev->cdev)
			_android_dev->cdev->desc.idProduct = device_desc.idProduct;
		is_usb_mode_change = true;
		usb_composite_force_reset(_android_dev->cdev);
	}
	
	if(strcmp(cmd,"net") == 0)
	{
		if(g_usb_mode == USB_MODE_NET)
			return 0;
			
		g_usb_mode = USB_MODE_NET;//coolpad net mode
		
		list_for_each_entry(func, &android_config_driver.functions, list)
		{
			if (!strcmp(func->name, "usb_mass_storage"))
			{
				usb_function_set_enabled(func, DISABLE);
			}
			if (!strcmp(func->name, "mtp"))
			{
				usb_function_set_enabled(func, DISABLE);
			}
			if (!strcmp(func->name, "rndis"))
			{
				usb_function_set_enabled(func, ENABLE);
			}
			if (!strcmp(func->name, "adb"))
			{
				if(!func->disabled)
					adb_enabled = 1;
			}
		}

				
		if (adb_enabled)
			device_desc.idProduct =
				__constant_cpu_to_le16(RNDIS_ADB_PRODUCT_ID);
		else
			device_desc.idProduct =
				__constant_cpu_to_le16(SINGLE_RNDIS_PRODUCT_ID);
		if (_android_dev->cdev)
			_android_dev->cdev->desc.idProduct = device_desc.idProduct;
		is_usb_mode_change = true;
		usb_composite_force_reset(_android_dev->cdev);
	}

		/* set product ID to the appropriate value */

	return 1;
}

static const struct file_operations device_mode_change_fops = {
	.owner = THIS_MODULE,
	.open = device_mode_change_open,
	.write = device_mode_change_write,
	.release = device_mode_change_release,
};

static struct miscdevice mode_change_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "usb_change_mode",
	.fops = &device_mode_change_fops,
};

/*system suspend lock adjust*/
void adjust_suspend_wake_lock(int on)
{
	if(on)
	{//online
		printk(KERN_INFO "USB!%s:wake_lock\n", __func__);
		wake_lock(&_android_dev->wake_lock);
	}
	else
	{//offline
        	printk(KERN_INFO "USB!%s:wake_unlock\n", __func__);
		wake_unlock(&_android_dev->wake_lock);  	    
	}

}

static int __init android_probe(struct platform_device *pdev)
{
	struct android_usb_platform_data *pdata = pdev->dev.platform_data;
	struct android_dev *dev = _android_dev;

	printk(KERN_INFO "USB!%s:pdata:%p\n", __func__, pdata);

	if (pdata) {
		dev->products = pdata->products;
		dev->num_products = pdata->num_products;
		dev->functions = pdata->functions;
		dev->num_functions = pdata->num_functions;
		
#if 0
//use native defined,delete by yanghaishan

		if (pdata->vendor_id)
			device_desc.idVendor =
				__constant_cpu_to_le16(pdata->vendor_id);
		if (pdata->product_id) {
			dev->product_id = pdata->product_id;
			device_desc.idProduct =
				__constant_cpu_to_le16(pdata->product_id);
		}
		if (pdata->version)
			dev->version = pdata->version;

		if (pdata->product_name)
			strings_dev[STRING_PRODUCT_IDX].s = pdata->product_name;
		if (pdata->manufacturer_name)
			strings_dev[STRING_MANUFACTURER_IDX].s =
					pdata->manufacturer_name;
		if (pdata->serial_number)
			strings_dev[STRING_SERIAL_IDX].s = pdata->serial_number;
#endif
	}

	return usb_composite_register(&android_usb_driver);
}

static struct platform_driver android_platform_driver = {
	.driver = { .name = "android_usb", },
	.probe = android_probe,
};

//extern int yl_params_kernel_read(char *buf,size_t count);
//extern int yl_get_bootreason(char *out,int len);

/*usb module driver initiate*/
static int __init init(void)
{
	struct android_dev *dev;
	int ret = 0;
	struct device_params_head device ;
	char bootreason[20] = {0};


	printk(KERN_INFO "USB!%s:coolpad_usb init\n", __func__);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	/* set default values, which should be overridden by platform data */
	//dev->product_id = PRODUCT_ID;
	_android_dev = dev;

	wake_lock_init(&dev->wake_lock, WAKE_LOCK_SUSPEND, "usb_gadget");

	//add by yanghaishan,100810
	iNandBoot = false;
	//yl_get_bootreason(bootreason,20);


	if(strncmp(bootreason,"iNAND",5) == 0)
	{
		printk(KERN_INFO "USB!%s:boot reason is iNAND\n", __func__);	
		iNandBoot = true;
	}

	//get device params
	memset(&device, 0, sizeof(device));
	strcpy(device.c8SyncByte, "DEVICE");
	//yl_params_kernel_read((char*)(&device), sizeof(device));

	if(!iNandBoot)
	{//normal mode
		memset(device_name, 0, sizeof(device_name));
		memset(sn, 0, sizeof(sn));
		//memcpy(device_name, device.c8DeviceName, sizeof(device.c8DeviceName));
		//memcpy(sn, device.c8DeviceName, sizeof(device.c8DeviceName));
		memcpy(device_name, "CoolpadN930", strlen("CoolpadN930"));
		memcpy(sn, "CoolpadN930", strlen("CoolpadN930"));
		ret = platform_driver_register(&android_platform_driver);
		if (ret)
		{
				wake_lock_destroy(&dev->wake_lock);
				kfree(dev);
				return ret;
		}
	
		ret = misc_register(&mode_change_device);
		if (ret) {
			platform_driver_unregister(&android_platform_driver);
			wake_lock_destroy(&dev->wake_lock);
			kfree(dev);
			return ret;
		}
	}
	else
	{//inand download mode
		memset(device_name, 0, sizeof(device_name));
		memset(sn, 0, sizeof(sn));
		//memcpy(device_name, device.c8DeviceName, sizeof(device.c8DeviceName));
		//memcpy(sn, device.c8SN, sizeof(device.c8SN));
		memcpy(device_name, "CoolpadN930", strlen("CoolpadN930"));
		memcpy(sn, "CoolpadN930", strlen("CoolpadN930"));
		ret = platform_driver_register(&android_platform_driver);
		if (ret)
		{
				wake_lock_destroy(&dev->wake_lock);
				kfree(dev);
				return ret;
		}
	}
	
	return ret;
}
module_init(init);

static void __exit cleanup(void)
{
	usb_composite_unregister(&android_usb_driver);
	misc_deregister(&mode_change_device);
	platform_driver_unregister(&android_platform_driver);
    wake_lock_destroy(&_android_dev->wake_lock);
	kfree(_android_dev);
	_android_dev = NULL;
}
module_exit(cleanup);

/*
 * drivers/usb/gadget/via_usermode.c
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
 
/* #define DEBUG */
/* #define VERBOSE_DEBUG */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <linux/usb/android.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#include <linux/uaccess.h>
#include "f_mass_storage.h"
#include "f_adb.h"
#include "gadget_chips.h"
#include "u_serial.h"
#include "u_ether22.h"

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

extern int acm_function_bind_config(struct usb_configuration *c);
extern void ser0_function_enable(int enable);
extern void ser1_function_enable(int enable);
extern void ser2_function_enable(int enable);
extern int yl_get_bootreason(char * out,int len);


MODULE_AUTHOR("Mike Lockwood");
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static const char longname[] = "Gadget Android";

/* Default vendor and product IDs, overridden by platform data */
#define VENDOR_ID		0x1EBF
#define COMPOSITE_PRODUCT_ID		0x602D//0x6045
#define SINGLE_UMS_PRODUCT_ID		0x6046
#define SINGLE_MTP_PRODUCT_ID		0x6047
#define COOLPAD_DRIVER_VERSION		0x01
#define MASS_STORAGE_NLUNS	        0x0
#define ADB_PRODUCT_ID	        0x602D//0x6045

struct android_dev {
	struct usb_composite_dev *cdev;

	int composite_pid;
	int single_ums_pid;
	int single_mtp_pid;
	int version;

	int adb_enabled;
	int nluns;
};

bool iNandBoot = false;
enum usb_mode g_usb_mode;
bool is_usb_mode_change = false;//avoid to appear after change mode
int g_serial_enabled = 0;
int g_ltecal_mode = 0;

static atomic_t adb_enable_excl;
static struct android_dev *_android_dev;

/* string IDs are assigned dynamically */

#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

/* String Table */
static struct usb_string strings_dev[] = {
	/* These dummy values should be overridden by platform data */
	[STRING_MANUFACTURER_IDX].s = "Coolpad",
	[STRING_PRODUCT_IDX].s = "Coolpad 5860E",
	[STRING_SERIAL_IDX].s = "Coolpad_5860E",
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
//	.bDeviceClass         = USB_CLASS_PER_INTERFACE,
//	.bDeviceClass         = USB_CLASS_COMM,

	.bDeviceClass		=0xef,
	.bDeviceSubClass	=0x2,
	.bDeviceProtocol	=0x1,

	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(COMPOSITE_PRODUCT_ID),
	.bcdDevice            = __constant_cpu_to_le16(0x0100),
	.bNumConfigurations   = 1,
};

void android_usb_set_connected(int connected)
{
    if (_android_dev && _android_dev->cdev && _android_dev->cdev->gadget) {
        if (connected)
            usb_gadget_connect(_android_dev->cdev->gadget);
        else
            usb_gadget_disconnect(_android_dev->cdev->gadget);
    }
}
extern int rndis_function_bind_config(struct usb_configuration *c);
static int __init android_bind_config(struct usb_configuration *c)
{
    struct android_dev *dev = _android_dev;
    int ret;
    printk(KERN_DEBUG "android_bind_config\n");

	//ret = mtp_bind_config(dev->cdev,c);
	//if (ret)
	//	return ret;

	rndis_function_bind_config(c);
    ret = mass_storage_function_add(dev->cdev, c);
    if (ret)
        return ret;

    //ret = modem_function_add(dev->cdev, c);
    //if (ret < 0)
    //    return ret;

    ret = gser_bind_config(c, 0); //ttyGS0 - ChinaTelecom PCUI
    if (ret < 0)
        return ret;

    ret = gser_bind_config(c, 1); //ttyGS1 - ChinaTelecom NMEA
    if (ret < 0)
        return ret;
#if 0
    ret = gser_bind_config(c, 2); //ttyGS2 - ChinaTelecom Diagnostics Interface
    if (ret < 0)
        return ret;
#endif
#if 0
    ret = acm_function_bind_config(c);
    if (ret < 0)
        return ret;
#endif
#if 0
    ret = cdrom_storage_function_add(dev->cdev, c, 1);
    if (ret)
        return ret;
#endif
	//g_usb_mode = USB_MODE_SUIT;
	g_usb_mode = USB_MODE_UDISK;
    return adb_function_add(dev->cdev, c);
}

static struct usb_configuration android_config_driver = {
	.label		= "android",
	.bind		= android_bind_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
//	.bMaxPower	= CONFIG_USB_GADGET_VBUS_DRAW / 2,
	.bMaxPower	= 0xfa,
};


static int __init android_bind(struct usb_composite_dev *cdev)
{
    struct android_dev *dev = _android_dev;
    struct usb_gadget	*gadget = cdev->gadget;
    int			gcnum;
    int			id;
    int			ret = 0;

    printk(KERN_INFO "android_bind\n");

	//ret = gchar_setup(cdev->gadget, 1);
	//if (ret < 0)
    //    	return ret;

    ret = gserial_setup(cdev->gadget, 2);//externs to 3 interfaces
    if (ret < 0)
        return ret;

    /* Allocate string descriptor numbers ... note that string
     * contents can be overridden by the composite_dev glue.
     */
    id = usb_string_id(cdev);
    if (id < 0) {
        ret = id;
        goto fail;
    }

    strings_dev[STRING_MANUFACTURER_IDX].id = id;
    device_desc.iManufacturer = id;

    id = usb_string_id(cdev);
    if (id < 0) {
        ret = id;
        goto fail;
    }
    strings_dev[STRING_PRODUCT_IDX].id = id;
    device_desc.iProduct = id;

    id = usb_string_id(cdev);
    if (id < 0) {
        ret = id;
        goto fail;
    }
    strings_dev[STRING_SERIAL_IDX].id = id;
    device_desc.iSerialNumber = id;

    /* register our configuration */
    ret = usb_add_config(cdev, &android_config_driver);
    if (ret) {
        printk(KERN_ERR "usb_add_android_config failed\n");
        goto fail;
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

    return 0;
fail:
    gserial_cleanup();
    return ret;
}

static struct usb_composite_driver android_usb_driver = {
	.name		= "android_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= android_bind,
};


static void enable_adb(struct android_dev *dev, int enable)
{
	if (enable != dev->adb_enabled) {
		dev->adb_enabled = enable;
		adb_function_enable(enable);
#if 1
		/* set product ID to the appropriate value */
		if (enable)
		{//enable
			device_desc.idProduct =
				__constant_cpu_to_le16(dev->composite_pid);
		}
		else
		{//disable
			if(g_usb_mode == USB_MODE_UDISK)
			    device_desc.idProduct =
				    __constant_cpu_to_le16(dev->composite_pid);
			else if(g_usb_mode == USB_MODE_MTP)
				device_desc.idProduct =
				    __constant_cpu_to_le16(dev->single_mtp_pid);
			else/*charge or udisk mode*/
			    device_desc.idProduct =
				    __constant_cpu_to_le16(dev->single_ums_pid);
		}

		if (dev->cdev)
			dev->cdev->desc.idProduct = device_desc.idProduct;

        if(g_serial_enabled == 1)
        {
		    if (dev->cdev)
			    dev->cdev->desc.idProduct = __constant_cpu_to_le16(0x6045);
			device_desc.idProduct = dev->cdev->desc.idProduct;
        }
        else
        {
		    if (dev->cdev)
			    dev->cdev->desc.idProduct = __constant_cpu_to_le16(0x602D);
			device_desc.idProduct = dev->cdev->desc.idProduct;
        }
        if(g_ltecal_mode == 1)
        {
		    if (dev->cdev)
			    dev->cdev->desc.idProduct = __constant_cpu_to_le16(0x6045);
			device_desc.idProduct = dev->cdev->desc.idProduct;
        }

#endif
		/* force reenumeration */
		if (dev->cdev && dev->cdev->gadget &&
				dev->cdev->gadget->speed != USB_SPEED_UNKNOWN) {
			usb_gadget_disconnect(dev->cdev->gadget);
			msleep(10);
			is_usb_mode_change = true;//sign online message is mode change,yanghaishan,101020
			usb_gadget_connect(dev->cdev->gadget);
		}
	}
}

/*adb使能设备打开函数*/
static int adb_enable_open(struct inode *ip, struct file *fp)
{
	if (atomic_inc_return(&adb_enable_excl) != 1) {
		atomic_dec(&adb_enable_excl);
		return -EBUSY;
	}

	printk(KERN_INFO "USB!%s:enabling adb\n", __func__);
	enable_adb(_android_dev, 1);

	return 0;
}

/*adb使能设备释放函数*/
static int adb_enable_release(struct inode *ip, struct file *fp)
{
	printk(KERN_INFO "USB!%s:disabling adb\n", __func__);
	enable_adb(_android_dev, 0);
	atomic_dec(&adb_enable_excl);
	return 0;
}

static const struct file_operations adb_enable_fops = {
	.owner =   THIS_MODULE,
	.open =    adb_enable_open,
	.release = adb_enable_release,
};

static struct miscdevice adb_enable_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "android_adb_enable",
	.fops = &adb_enable_fops,
};
//added by coolpad tanzhongjun 2010-10-28
#define MAX_DEVICE_NAME_SIZE  30

/*usb模式切换设备打开*/
static int device_mode_change_open(struct inode *inode, struct file *file)
{
	return 0;
}

/*usb模式切换设备释放*/
static int device_mode_change_release(struct inode *inode, struct file *file)
{
	return 0;
}

/**********************
* Function : usb模式切换
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
	unsigned char cmd[MAX_DEVICE_NAME_SIZE + 1];
	unsigned int cnt = MAX_DEVICE_NAME_SIZE;

	if (count <= 0)
		return 0;

	if (cnt > count)
		cnt = count;

	if (copy_from_user(cmd, buffer, cnt))
		return -EFAULT;

	cmd[cnt] = 0;
	printk(KERN_ERR "USB!%s:%s\n",__FUNCTION__,cmd);

	if(strcmp(cmd,"enable_serial") == 0)
    { 
		g_usb_mode = USB_MODE_UDISK;//coolpad udisk mode
		ser0_function_enable(1);
		ser1_function_enable(1);
        
        g_serial_enabled = 1;
		device_desc.idProduct = __constant_cpu_to_le16(0x6045);

		if (_android_dev->cdev)
            _android_dev->cdev->desc.idProduct = __constant_cpu_to_le16(0x6045);

		usb_gadget_disconnect(_android_dev->cdev->gadget);
		msleep(10);
		is_usb_mode_change = true;
        
		usb_gadget_connect(_android_dev->cdev->gadget);
    } 
	else if(strcmp(cmd,"disable_serial") == 0)
    { 
		g_usb_mode = USB_MODE_UDISK;//coolpad udisk mode
		ser0_function_enable(0);
		ser1_function_enable(0);
        
        g_serial_enabled = 0;
		device_desc.idProduct = __constant_cpu_to_le16(0x602D);
				//__constant_cpu_to_le16(_android_dev->composite_pid);
		if (_android_dev->cdev)
            _android_dev->cdev->desc.idProduct = __constant_cpu_to_le16(0x602D);
			//_android_dev->cdev->desc.idProduct = device_desc.idProduct;

		usb_gadget_disconnect(_android_dev->cdev->gadget);
		msleep(10);
		is_usb_mode_change = true;
		usb_gadget_connect(_android_dev->cdev->gadget);
    } 

    //added by coolpad heyong 2010-11-19
	if(strcmp(cmd,"mtp") == 0)
	{
		if(g_usb_mode == USB_MODE_MTP)
			return 0;

#ifdef CONFIG_USB_MODEM_SUPPORT
		if(g_usb_mode == USB_MODE_SUIT)
		{
			//notify_control_line_state(0xFF);
			//g_acm_connected = 0;
		}
#endif
		g_usb_mode = USB_MODE_MTP;//coolpad mtp mode
		msc_function_enable(1);
		ser0_function_enable(1);
		ser1_function_enable(1);
		ser2_function_enable(1);

		if (_android_dev->adb_enabled)
			device_desc.idProduct =
				__constant_cpu_to_le16(_android_dev->composite_pid);
		else
			device_desc.idProduct =
				__constant_cpu_to_le16(_android_dev->single_mtp_pid);
		if (_android_dev->cdev)
			_android_dev->cdev->desc.idProduct = device_desc.idProduct;

		usb_gadget_disconnect(_android_dev->cdev->gadget);
		msleep(10);
		is_usb_mode_change = true;
		usb_gadget_connect(_android_dev->cdev->gadget);
	}
	if(strcmp(cmd,"suit") == 0)
	{
		if(g_usb_mode == USB_MODE_SUIT)
			return 0;
		g_usb_mode = USB_MODE_SUIT;//coolpad suit mode

		msc_function_enable(1);

		ser0_function_enable(1);
		ser1_function_enable(1);
		ser2_function_enable(1);

		device_desc.idProduct =
				__constant_cpu_to_le16(_android_dev->composite_pid);

		if (_android_dev->cdev)
			_android_dev->cdev->desc.idProduct = device_desc.idProduct;

		usb_gadget_disconnect(_android_dev->cdev->gadget);
		msleep(10);
		is_usb_mode_change = true;
		usb_gadget_connect(_android_dev->cdev->gadget);
	}

	if(strcmp(cmd,"charge") == 0)
	{
		if(g_usb_mode == USB_MODE_CHARGE)
			return 0;

#ifdef CONFIG_USB_MODEM_SUPPORT
		if(g_usb_mode == USB_MODE_SUIT)
		{
			//notify_control_line_state(0xFF);
			//g_acm_connected = 0;
		}
#endif
		g_usb_mode = USB_MODE_CHARGE;//charge
		msc_function_enable(1);

		if (_android_dev->adb_enabled)
			device_desc.idProduct =
				__constant_cpu_to_le16(_android_dev->composite_pid);
		else
			device_desc.idProduct =
				__constant_cpu_to_le16(_android_dev->single_ums_pid);

		if (_android_dev->cdev)
			_android_dev->cdev->desc.idProduct = device_desc.idProduct;

		usb_gadget_disconnect(_android_dev->cdev->gadget);
		msleep(10);
		is_usb_mode_change = true;
		usb_gadget_connect(_android_dev->cdev->gadget);
	}

	if(strcmp(cmd,"udisk") == 0)
	{
		if(g_usb_mode == USB_MODE_UDISK)
			return 0;

#ifdef CONFIG_USB_MODEM_SUPPORT
		if(g_usb_mode == USB_MODE_SUIT)
		{
			//notify_control_line_state(0xFF);
			//g_acm_connected = 0;
		}
#endif
		g_usb_mode = USB_MODE_UDISK;//pc u disk
		ser0_function_enable(0);
		ser1_function_enable(0);
		ser2_function_enable(0);
		msc_function_enable(1);


		if (_android_dev->adb_enabled)
			device_desc.idProduct =
				__constant_cpu_to_le16(_android_dev->composite_pid);
		else
			device_desc.idProduct =
				__constant_cpu_to_le16(_android_dev->single_ums_pid);

		if (_android_dev->cdev)
			_android_dev->cdev->desc.idProduct = device_desc.idProduct;

		usb_gadget_disconnect(_android_dev->cdev->gadget);
		msleep(10);
		is_usb_mode_change = true;
		usb_gadget_connect(_android_dev->cdev->gadget);
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
//end
static int __init android_probe(struct platform_device *pdev)
{
#if 0
	struct android_usb_platform_data *pdata = pdev->dev.platform_data;
	struct android_dev *dev = _android_dev;

	printk(KERN_INFO "USB!%s:pdata:%p\n", __func__, pdata);


	if (pdata) {
		if (pdata->vendor_id)
			device_desc.idVendor =
				__constant_cpu_to_le16(pdata->vendor_id);
		//delete by yanghaishan,2010.09.27
		/*if (pdata->product_id) {
			dev->product_id = pdata->product_id;
			device_desc.idProduct =
				__constant_cpu_to_le16(pdata->product_id);
		}
		if (pdata->adb_product_id)
			dev->adb_product_id = pdata->adb_product_id;*/
		if (pdata->version)
			dev->version = pdata->version;

		if (pdata->product_name)
			strings_dev[STRING_PRODUCT_IDX].s = pdata->product_name;
		if (pdata->manufacturer_name)
			strings_dev[STRING_MANUFACTURER_IDX].s = pdata->manufacturer_name;
		if (pdata->serial_number)
			strings_dev[STRING_SERIAL_IDX].s = pdata->serial_number;
		dev->nluns = pdata->nluns;
	}
#endif
	return 0;
}

static struct platform_driver android_platform_driver = {
	.driver = { .name = "android_usb", },
	.probe = android_probe,
};

static int __init init(void)
{
	struct android_dev *dev;
	int ret;
    char boot_reason[20]={0};

    yl_get_bootreason(boot_reason,20);

	printk(KERN_INFO "android init\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	/* set default values, which should be overridden by platform data */
	if(strncmp("lte_rf_test",boot_reason,11)!=0)
	{
        dev->composite_pid = COMPOSITE_PRODUCT_ID;
        g_ltecal_mode = 0;
    }
    else
    {
        dev->composite_pid = 0x6045; 
        g_ltecal_mode = 1;
    }
	device_desc.idProduct =
			__constant_cpu_to_le16(dev->composite_pid);
	dev->single_ums_pid = SINGLE_UMS_PRODUCT_ID;
	dev->single_mtp_pid = SINGLE_MTP_PRODUCT_ID;
	dev->version = COOLPAD_DRIVER_VERSION;
	dev->nluns = MASS_STORAGE_NLUNS;
	_android_dev = dev;

	ret = platform_driver_register(&android_platform_driver);
	if (ret)
		return ret;

    printk(KERN_INFO "register adb_enable_device %p\n", &adb_enable_device);
	ret = misc_register(&adb_enable_device);
	if (ret) {
		platform_driver_unregister(&android_platform_driver);
                printk(KERN_INFO "error %d\n", ret);
		return ret;
	}
#if 1
    printk(KERN_INFO "register mode_change_device %p\n", &mode_change_device);
    ret = misc_register(&mode_change_device);
    if (ret) {
        misc_deregister(&adb_enable_device);
        platform_driver_unregister(&android_platform_driver);
        printk(KERN_INFO "error %d\n", ret);
        return ret;
    }
#endif
    printk(KERN_INFO "regiser composite\n");
	ret = usb_composite_register(&android_usb_driver);
	if (ret) {
		misc_deregister(&adb_enable_device);
		platform_driver_unregister(&android_platform_driver);
	}

	return ret;
}
module_init(init);

static void __exit cleanup(void)
{
    mass_storage_function_remove();
    usb_composite_unregister(&android_usb_driver);
    gserial_cleanup();

	gether_cleanup();
    printk(KERN_INFO "deregister adb_enable_device %p\n", &adb_enable_device);
	#if 1
    misc_deregister(&mode_change_device);
    #endif
	misc_deregister(&adb_enable_device);
    platform_driver_unregister(&android_platform_driver);
    kfree(_android_dev);
    _android_dev = NULL;
}
module_exit(cleanup);

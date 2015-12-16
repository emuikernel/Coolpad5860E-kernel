/*
 * drivers/media/video/camera_version.c
 *
 * camera version driver
 *
 *
 * Copyright (C) 2012 Yulong
 *
 * Author:zhanglin
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/clk.h>

#include <linux/i2c.h> 
#include <linux/delay.h> 
#include <linux/earlysuspend.h> 
#include <linux/platform_device.h> 
#include <linux/leds.h> 
#include <linux/slab.h> 
#include <linux/gpio.h> 

#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>

#include <asm/io.h>    
#include "../arch/arm/mach-omap2/mux.h"  
#include <plat/omap-pm.h>

#define DEBUG 1 

#ifdef DEBUG
#define FLT_ERR_LOG(fmt, ...)  \
                printk(KERN_ERR "camera_version:" fmt, ##__VA_ARGS__) 
#define FLT_NOTE_LOG(fmt, ...) 
#else
#define FLT_ERR_LOG(fmt, ...) 
#endif

#define FACTORY_MODE 1

#if FACTORY_MODE//for factory_mode
#define	CAMERA_VERSION_PROC_FILE	"driver/camera_version"
static struct proc_dir_entry *camera_version_proc_file;
#endif

extern int sensor_version;
extern int sensor_byd_kerr;

static int camera_version_t = 8;  //=8 for test

#if FACTORY_MODE //for factory_mode

static int camera_version_print(int version)
{
      if(version == 0)
      {     
         FLT_ERR_LOG("Sensor is MT9T113, and the vendor is SUNNY!");
      }
      else if(version == 1)
      {
         FLT_ERR_LOG("Sensor is S5K5CA, and the vendor is BYD!");
      }
      else if(version == 2)
      {
         FLT_ERR_LOG("Sensor is S5K5CA, and the vendor is KERR!");
      }
      else
      {
         FLT_ERR_LOG("Can not know the sensor and vendor!!!");
      }

      return 0;
}


static ssize_t camera_version_proc_write(struct file *filp,
				     const char *buff, size_t len,
				     loff_t * off)
{
	FLT_NOTE_LOG("==camera_version_write ==");

        return 0;
}

static ssize_t camera_version_proc_read(struct file *filp,
				     int *buff, size_t len,
				     loff_t * off)
{     
        int ret = 0;

        camera_version_t = sensor_version;
        if(camera_version_t == 1){
                camera_version_t = sensor_byd_kerr;
        }

	FLT_ERR_LOG("==camera_version_read ==");
        if(len > sizeof(camera_version_t)) 
        {
           ret = len ? -ENXIO : 0;  
           goto out; 
        }

        if(put_user(camera_version_t,buff)) {
	   ret = -EFAULT;
           goto out;
	}
    
        ret = len;
        FLT_ERR_LOG("== read %d bytes ok==", len);
        camera_version_print(camera_version_t);
        	
out:
        return ret;
}

static ssize_t camera_version_proc_open(struct inode *iNode,struct file *filp)
{
	FLT_ERR_LOG("==camera_version_open ===");

        return 0;
}

static ssize_t camera_version_proc_release(struct inode *iNode,struct file *filp)
{
	FLT_ERR_LOG("==camera_version_release ==");

        return 0;
}

static int camera_version_proc_ioctl(struct inode* inode,struct file* file,unsigned int cmd,unsigned long arg)
{

  FLT_NOTE_LOG("=============camera_version_proc_ioctl cmd: %d  ===!!!",cmd);
  return 0;

}

static struct file_operations camera_version_proc_ops = {
        .owner = THIS_MODULE,
        .open = camera_version_proc_open,
	.ioctl = camera_version_proc_ioctl,
	.write = camera_version_proc_write,
        .read = camera_version_proc_read,
        .release = camera_version_proc_release,
};

static void create_camera_version_proc_file(void)
{
	camera_version_proc_file=
	    create_proc_entry(CAMERA_VERSION_PROC_FILE, 0666, NULL);
	if (camera_version_proc_file) {
		camera_version_proc_file->proc_fops = &camera_version_proc_ops;
	} else
		FLT_ERR_LOG("proc file create failed!");
}
static void remove_camera_version_proc_file(void)
{
	extern struct proc_dir_entry proc_root;
	remove_proc_entry(CAMERA_VERSION_PROC_FILE, &proc_root);
}
#endif


static  int __init camera_version_init(void)
{
   FLT_ERR_LOG(" ====entring camera_version_init");
   #if FACTORY_MODE//for factory_mode
    create_camera_version_proc_file();
   #endif
   return 0; //platform_driver_register(&camera_version_driver);
}
static int __exit camera_version_exit(void)
{
   FLT_ERR_LOG("====exiting camera_version_init");
    #if FACTORY_MODE//for factory_mode                                                                
      remove_camera_version_proc_file();                                                    
    #endif
    return 0;
}

late_initcall(camera_version_init);
module_exit(camera_version_exit);
MODULE_AUTHOR("zhanglin");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("camera version driver");




/* drivers/misc/vib-omap-pwm.c
 *
 * Copyright (C) 2009 Motorola, Inc.
 * Copyright (C) 2008 HTC Corporation.
 * Copyright (C) 2007 Google, Inc.
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/i2c/twl.h>
#include <plat/common.h>
#include <plat/board.h>
#include <plat/clock.h>
#include <plat/control.h>
#include <plat/gpio.h>
#include <linux/ioctl.h>
#include <linux/err.h>
#include <linux/vib-omap-pwm.h>
#include <linux/slab.h>

#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/regulator/consumer.h>
#include <linux/workqueue.h>
#include <linux/clk.h>
#include <plat/dmtimer.h>
#include "../../arch/arm/mach-omap2/mux.h"

static struct workqueue_struct *vibrator_wq;


struct vibrator_dev
{
	struct cdev vibrator_cdev;		/*cdev结构体*/
	unsigned int vibrator_major;	
};

static struct vibrator_dev *vibrator_dev;

#define PWM_VIBRATOR_GP_TIMER_NUM       (10)
#define MAPPHONE_GLOHAP_EN_OMAP_GPIO    (180)
#define GPIO_15		15			//for vibration software

/* TODO: replace with correct header */
#include "../staging/android/timed_output.h"

#define TWL4030_BASEADD_INTBR	        (0x0085)
#define TWL5030_GPBR1_OFFSET	        (0x91-TWL4030_BASEADD_INTBR)
#define TWL5030_PMBR1_OFFSET	        (0x92-TWL4030_BASEADD_INTBR)
#define TWL5030_VIBRA_CTL_OFFSET        (0x45)
#define TWL5030_VIBRA_EN_SHIFT          (0x0)

#define TWL5030_GPIO7_VIBRASYNC_PWMASK  (0x1)
#define TWL5030_GPIO7_VIBRASYNC_PWSHIFT (4)

#define TWL5030_VIBRATOR_CFG_REG		(0x05)
#define VIB_CFG_EN						(0x1 << 3)
#define VIB_PWM_EN						(0x1 << 2)
#define VIB_PERCENT_25					(0x3 << 0)
#define VIB_PERCENT_50					(0x2 << 0)					
#define VIB_PERCENT_75					(0x1 << 0)	
#define VIB_PERCENT_100					(0x0 << 0)

#define TWL4030_PROTECT_KEY             (0x0E)

#define PWM_ON_OFF_PERIOD               (5714)
#define PWM_ON_PERIOD                   (0) //(PWM_ON_OFF_PERIOD >> 1)
#define PWM_OFF_PERIOD                  (0) //(PWM_ON_OFF_PERIOD >> 1)


static int vibrator_open(struct inode *inode, struct file *filp);
static int vibrator_close(struct inode *inode, struct file *file);
static ssize_t vibrator_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos);
static ssize_t vibrator_write(struct file *filp, const char __user *buf,  size_t count, loff_t *ppos);
static void vibrator_enable(struct timed_output_dev *dev, int value);

static struct work_struct vibrator_work;
static struct hrtimer vibe_timer;
static struct omap_dm_timer *pwm_timer;
static spinlock_t vibe_lock;
static int vibe_state;
static int vibe_timer_state;

/* periods in microseconds, old vals: on = 700, off = 467 */
/* For titanium, pwm frequency should be 175Hz. */
static unsigned long on_period = PWM_ON_PERIOD;
static unsigned long off_period = PWM_OFF_PERIOD;
static unsigned long load_reg;
static unsigned long cmp_reg;

static unsigned int pwm_level = 50; // 50% duty cycle

static void pwm_timer_init(void)
{
	unsigned char  rd_data;
	unsigned char vibrator_cfg = 0;	

	/* timer pwm setup */
	omap_dm_timer_set_source(pwm_timer,
		OMAP_TIMER_SRC_32_KHZ);

/*	omap_dm_timer_stop(pwm_timer);
	omap_dm_timer_set_load(pwm_timer, 1, -load_reg);
	omap_dm_timer_set_match(pwm_timer, 1, -cmp_reg);

	omap_dm_timer_set_pwm(pwm_timer, 1, 1,
		OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);

	omap_dm_timer_write_counter(pwm_timer, -2);*/

	
    /* set the PWM1 output to GPIO.7 */
    if (0 != twl_i2c_read_u8(TWL4030_MODULE_INTBR, &rd_data, TWL5030_PMBR1_OFFSET))
    {   /*error*/
        //printk("read i2c error\n");
        return;
    }
    rd_data |= (TWL5030_GPIO7_VIBRASYNC_PWMASK<<TWL5030_GPIO7_VIBRASYNC_PWSHIFT);
    twl_i2c_write_u8(TWL4030_MODULE_INTBR, rd_data, TWL5030_PMBR1_OFFSET);

    twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &vibrator_cfg,
                TWL5030_VIBRATOR_CFG_REG);
                
    vibrator_cfg |=  VIB_CFG_EN;
    vibrator_cfg &=  ~VIB_PWM_EN;
                
    twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, vibrator_cfg,
            TWL5030_VIBRATOR_CFG_REG);

}

/**************************************************************************
* Function    : vibrator_open()
* Description : 文件打开函数
*
* Calls       :
* Called By   : 
*
* Input :  
* 		none.
*
* Output : 
*		none.
*
* Return : 
*		Returns zero if successful , else return -ev.
*
* others : 
**************************************************************************/
static int vibrator_open(struct inode *inode, struct file *filp)
{

	if(inode == NULL || filp == NULL)
		return -EINVAL;
		
	/*将设备结构体指针赋值给文件私有数据指针*/  
    filp->private_data = vibrator_dev;  
	
    //yl_debug("vibrator! open++!\r\n");

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, TWL4030_VAUX1_DEDICATED);		//set default AVDD	2.8V
	
	//yl_debug("vibrator! open--!\r\n");
    return 0;
}


/**************************************************************************
* Function    : vibrator_close()
* Description : 流接口函数
*
* Calls       :
* Called By   : 
*
* Input :  
* 		inode: file node pointer
*		file:  file operation pointer
*
* Output : 
*		none.
*
* Return : 
*		Returns 0.
*
* others : 
**************************************************************************/
static int vibrator_close(struct inode *inode, struct file *file)
{

	if(inode == NULL || file == NULL)
		return -EINVAL;
		
    //yl_debug("vibrator! release!\r\n");

    return 0;
}


/**************************************************************************
* Function    : vibrator_read()
* Description : 流接口函数，应用通过调用readFile调用到该函数，
*                         从对应设备句柄的驱动读取数据
*
* Calls       :
* Called By   : 
*
* Input :  
* 		filp:  设备句柄
*		buf:   数据Buffer
*		count: 需要读取的数据长度.
*
* Output : 
*		none.
*
* Return : 
*		返回实际读取到的数据长度
*
* others : 
**************************************************************************/
static ssize_t vibrator_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    char key_state = 0;
    size_t count_temp=0;
    
    if(filp == NULL || buf == NULL || ppos == NULL)
		return -EINVAL;
		
    key_state=gpio_get_value(GPIO_15);
    count_temp=copy_to_user(buf,&key_state,1);
    //yl_debug("vibrator_read:keystate=%d\r\n",key_state);    
    
    return 1;
}


/**************************************************************************
* Function    : vibrator_write()
* Description : 流接口函数，应用通过调用writeFile调用到该函数，
*                         向对应设备句柄的驱动下发数据
* Calls       :
* Called By   : 
*
* Input :  
* 		filp:  设备句柄
*		buf:   数据Buffer
*		count: 需要下发的数据长度
*
* Output : 
*		none.
*
* Return : 
*		返回实际下发的数据长度
*
* others : 
**************************************************************************/
static ssize_t vibrator_write(struct file *filp, const char __user *buf,  size_t count, loff_t *ppos)
{
    char level;
    size_t count_temp=0;

    if(filp == NULL || buf == NULL || ppos == NULL)
		return -EINVAL;		
    
    count_temp=copy_from_user((&level), buf, count);
    
    //yl_debug("vibrator_write:level=%d,count=%d\n",level,count);	
    if(level < VIBRATOR_PWM_LEVEL_MIN || level > VIBRATOR_PWM_LEVEL_MAX) 
    {
        //printk(KERN_ERR "arg is invalid, need between 0 < arg < 100, while set vibrator pwm level\n");
        return 0;
    }

    pwm_level = level;
    return 1;
}


// turn on vibrator power
static void turn_on_vibrator_power(void)
{
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, TWL4030_PROTECT_KEY);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, TWL4030_PROTECT_KEY);

    twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, TWL4030_VAUX1_DEDICATED);        //VAUX1 3.0V
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x07 << 5), TWL4030_VAUX1_DEV_GRP);   //VAUX1 DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, TWL4030_PROTECT_KEY);
}

// turn off vibrator power
static void turn_off_vibrator_power(void)
{
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, TWL4030_PROTECT_KEY);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, TWL4030_PROTECT_KEY);

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0, TWL4030_VAUX1_DEV_GRP);//VAUX1 3.0V DEV_GRP belong to NULL
	
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, TWL4030_PROTECT_KEY);
}

static void set_gptimer_pwm_vibrator(int on)
{
	if (pwm_timer == NULL) {
		pr_err(KERN_ERR "vibrator pwm timer is NULL\n");
		return;
	}

	if (on) {
		if (!vibe_timer_state) {
			omap_dm_timer_enable(pwm_timer);
			pwm_timer_init();
            omap_dm_timer_start(pwm_timer);

			omap_dm_timer_set_load(pwm_timer, 1, -load_reg);
			omap_dm_timer_set_match(pwm_timer, 1, -cmp_reg);

			omap_dm_timer_set_pwm(pwm_timer, 1, 1,
				OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);

			omap_dm_timer_write_counter(pwm_timer, -2);

			vibe_timer_state = 1;
		}
        turn_on_vibrator_power();     
        omap_mux_init_signal("mcspi2_somi.gpt10_pwm_evt", OMAP_PIN_OUTPUT);
		//gpio_direction_output(MAPPHONE_GLOHAP_EN_OMAP_GPIO, 1);
	} else {
		if (vibe_timer_state) {
			omap_dm_timer_stop(pwm_timer);
			omap_dm_timer_disable(pwm_timer);
            vibe_timer_state = 0;
		}
        turn_off_vibrator_power();
        omap_mux_init_signal("gpio_180", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
		gpio_direction_output(MAPPHONE_GLOHAP_EN_OMAP_GPIO, 0);
	}
}

static void update_vibrator(struct work_struct *work)
{
	set_gptimer_pwm_vibrator(vibe_state);
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	unsigned long	flags = 0;

	spin_lock_irqsave(&vibe_lock, flags);
	hrtimer_cancel(&vibe_timer);

	if (value == 0)
		vibe_state = 0;
	else {
		value = (value > 15000 ? 15000 : value);
		vibe_state = 1;

        // change the on/off period based on the value passed by application
        on_period = (PWM_ON_OFF_PERIOD * (100 - pwm_level)) / 100;
        off_period = (PWM_ON_OFF_PERIOD * pwm_level) / 100;

        // update the load register and match count
        load_reg = 32768 * (on_period + off_period) / 1000000;
        cmp_reg = 32768 * off_period / 1000000;

        hrtimer_start(&vibe_timer,
			ktime_set(value / 1000, (value % 1000) * 1000000),
			HRTIMER_MODE_REL);
	}

	spin_unlock_irqrestore(&vibe_lock, flags);

	queue_work(vibrator_wq, &vibrator_work);
//	schedule_work(&vibrator_work);
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	if (hrtimer_active(&vibe_timer)) {
		ktime_t r = hrtimer_get_remaining(&vibe_timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} else
		return 0;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	vibe_state = 0;
//	schedule_work(&vibrator_work);
	queue_work(vibrator_wq, &vibrator_work);
	return HRTIMER_NORESTART;
}

static struct timed_output_dev gptimer_pwm_vibrator = {
	.name = "vibrator",
	.get_time = vibrator_get_time,
	.enable = vibrator_enable,
};

void __init vibrator_omap_pwm_init(int initial_vibrate)
{
//	INIT_WORK(&vibrator_work, update_vibrator);

	spin_lock_init(&vibe_lock);
	vibe_state = 0;
	vibe_timer_state = 0;
	hrtimer_init(&vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vibe_timer.function = vibrator_timer_func;

    if (gpio_request(MAPPHONE_GLOHAP_EN_OMAP_GPIO, "linear-vib") < 0)
		printk(KERN_ERR "haptics: Fail to get GPIO for linear vib\n");
	else
		gpio_direction_output(MAPPHONE_GLOHAP_EN_OMAP_GPIO, 0);

	pwm_timer = omap_dm_timer_request_specific(PWM_VIBRATOR_GP_TIMER_NUM);
	if (pwm_timer == NULL) {
		pr_err(KERN_ERR "failed to request vibrator pwm timer\n");
		return;
	}

	/* omap_dm_timer_request_specific enables the timer */
	omap_dm_timer_disable(pwm_timer);    

	timed_output_dev_register(&gptimer_pwm_vibrator);

	load_reg = 32768 * (on_period + off_period) / 1000000;
	cmp_reg = 32768 * off_period / 1000000;

	vibrator_enable(NULL, 0);
	if (initial_vibrate)
		vibrator_enable(NULL, initial_vibrate);

	pr_info("vib-omap-pwm initialized\n");
}

static struct file_operations vibrator_fops = 
{
	.owner       = THIS_MODULE,
	.open        = vibrator_open,
	.release     = vibrator_close,
	.write       = vibrator_write,
	.read        = vibrator_read,
};

/**************************************************************************
* Function    : vibrator_setup_cdev()
* Description : 字符设备函数，用来注册vibrator设备并，即添加cdev结构体
*
* Calls       :
* Called By   : 
*
* Input :  
* 		dev --- vibrator device structure to register.
*		index --- the minor device number
*
* Output : 
*		none.
*
* Return : 
*		null.
*
* others : 
**************************************************************************/
static void vibrator_setup_cdev(struct vibrator_dev *dev, int index)
{
	unsigned int err;
	int	dev_number = 0;
	//yl_debug("vibrator_setup_cdev()++\r\n");
	
	dev_number = MKDEV(vibrator_dev->vibrator_major, index);
	
	if( (dev == NULL) || (&dev->vibrator_cdev == NULL) )
	{
		//printk("vibrator_fops is null!\r\n");
	}
	
	cdev_init(&dev->vibrator_cdev, &vibrator_fops);

	dev->vibrator_cdev.owner = THIS_MODULE;
	dev->vibrator_cdev.ops   = &vibrator_fops;
	
	//1 denotes the device count number
	err = cdev_add(&dev->vibrator_cdev, dev_number, 1);
	
	if(err)
	{
		//printk("vibrator!vibrator_setup_cdev: Error %d adding vibratordev\r\n",err);	
	}
	//yl_debug("vibrator_setup_cdev()--\r\n");
	
	return;
}

static int __init vib_omap_pwm_init(void)
{
    static struct class *pvibrator_class_device;
    unsigned int result;
    dev_t dev_number;
     
    //yl_debug("vibrator_init: ++vibrator! init!\r\n");

    vibrator_dev = kmalloc(sizeof(struct vibrator_dev), GFP_KERNEL);
    if(!vibrator_dev)/*alloc failure*/
    {
		//printk("vibrator_init: kmalloc vibrator_dev fail!\r\n");
		goto error;
    }
    memset(vibrator_dev, 0, sizeof(struct vibrator_dev));

    //0: first of the requested range of minor numbers
	//1: the number of minor numbers required
    result = alloc_chrdev_region(&dev_number, 0, 1, PWM_VIBRATOR_NAME);  	
	
    if(!result)
    {
		vibrator_dev->vibrator_major = MAJOR(dev_number);
		//printk("vibrator_init: vibrator major number is %d!\r\n", vibrator_dev->vibrator_major);
    }
    else
    {
		//printk(KERN_ERR"vibrator_init: request major number fail!\r\n");
		return result;
    } 
    
    //0: the minor device number
    vibrator_setup_cdev(vibrator_dev, 0);


	vibrator_wq = create_singlethread_workqueue("vibrator_wq");
	INIT_WORK(&vibrator_work, update_vibrator);

	//创建设备节点
	pvibrator_class_device = class_create(THIS_MODULE, "pwm-vibrator-class");
	if(IS_ERR(pvibrator_class_device))
	{
		//printk(KERN_ERR"vibrator_init: vibrator register node fail!\r\n");
	}
	
	//0: the minor device number
	device_create(pvibrator_class_device, NULL, MKDEV(vibrator_dev->vibrator_major, 0), NULL, PWM_VIBRATOR_NAME);

  	vibrator_omap_pwm_init(0);

    return 0;      

        
error:
    if(NULL != vibrator_dev)
    {
    	kfree(vibrator_dev);
    	vibrator_dev = NULL;
    }
    //yl_debug("--vibrator! init!\r\n");
    
    return -EINVAL;
}

static void __exit vib_omap_pwm_exit(void)
{        
    timed_output_dev_unregister(&gptimer_pwm_vibrator);
	return;
}



module_init(vib_omap_pwm_init);
module_exit(vib_omap_pwm_exit);

MODULE_DESCRIPTION("timed output gptimer pwm vibrator device");
MODULE_LICENSE("GPL");


/*
 * drivers/input/keyboard/headset_key.c
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <mach/gpio.h>
#include <mach/mux.h>
#include <linux/delay.h>
#include <plat/misc.h>  // for smartphone_calling_enable
#include <plat/yl_debug.h>//define yl_debug();

//#define    HEADSET_KEY_GPIO     14

#define    HEADSET_KEY_GPIO     (192 + 1)


#define	   DETECT_DELAY_TIME	10
#define    MAX_DETECT_COUNT		5

extern int get_twl4030_hsmic_status(void);
extern int get_headset_plug_status2(void);

static int headset_key_irq = 0;


struct headset_key_struct
{
	struct semaphore sem;
	struct work_struct  work;
	struct hrtimer timer;
	int timer_running;
	int pre_level;
	int crnt_level;
	int pass_key;
	int detect_count;
};

static struct headset_key_struct headset_key_stru;
static struct headset_key_struct *p_headset_key = &headset_key_stru;
static struct workqueue_struct *headset_key_wq;
static struct input_dev *zoom2_headset_key2;


static int headset_key_detect_on(void);

int get_headset_key_irq_number(void)
{
	return headset_key_irq;
}

static irqreturn_t zoom2_headset_key_irq(int irq, void *_zoom2_headset_key)
{
	struct input_dev *zoom2_headset_key = _zoom2_headset_key;
	
	if(NULL == zoom2_headset_key)
	{
		printk(KERN_ERR"zoom2_headset_key_irq: pointer parameter is null");
		return IRQ_NONE;
	}

#ifdef CONFIG_LOCKDEP
	/* WORKAROUND for lockdep forcing IRQF_DISABLED on us, which
	 * we don't want and can't tolerate since this is a threaded
	 * IRQ and can sleep due to the i2c reads it has to issue.
	 * Although it might be friendlier not to borrow this thread
	 * context...
	 */
	local_irq_enable();
#endif
	if((get_twl4030_hsmic_status() == 1) && (get_headset_plug_status2() == 0))
	{
		printk("before execute headset_key_detect_on() in %s func\n",__FUNCTION__);
		headset_key_detect_on();
	}	
	
	return IRQ_HANDLED;
}


/* constant on, up to maximum allowed time */
static int headset_key_detect_on(void)
{
//	down(&p_headset_key->sem);
	if(p_headset_key->timer_running == 0)
	{       
		hrtimer_start(&p_headset_key->timer, ktime_set(0, DETECT_DELAY_TIME*1000000), HRTIMER_MODE_REL);
    	p_headset_key->timer_running = 1;    
	}
	
    return 0;
}

static enum hrtimer_restart headset_key_timer_func(struct hrtimer *timer)
{
//	printk("p_headset_key->detect_count = %d, in %s func\n",p_headset_key->detect_count,__FUNCTION__);
//	yl_debug("p_headset_key->detect_count = %d, in %s func\n",p_headset_key->detect_count,__FUNCTION__);

	if(p_headset_key->timer_running == 1)
	{
    	queue_work(headset_key_wq, &p_headset_key->work);			
		//hrtimer_start(&p_headset_key->timer, ktime_set(0, DETECT_DELAY_TIME*1000000), HRTIMER_MODE_REL);
    }
    
    return HRTIMER_NORESTART;
}


static void headset_key_work_func(struct work_struct *work)
{
	p_headset_key->crnt_level = gpio_get_value(HEADSET_KEY_GPIO);
	if(get_twl4030_hsmic_status() != 1)//ÖÐÍ¾¶Ïmicbias£¬Í£Ö¹timer
	{
	    if(p_headset_key->timer_running)
        {    
            p_headset_key->timer_running=0;
            hrtimer_cancel(&p_headset_key->timer);
        }
        
        return;
	}
	if(p_headset_key->detect_count>1)
	{
		if(p_headset_key->crnt_level!=p_headset_key->pre_level)
		{	
			 p_headset_key->detect_count=0;   		
			 p_headset_key->pre_level=p_headset_key->crnt_level;
		}
	}
    p_headset_key->detect_count++;

//	printk("p_headset_key->detect_count = %d, in %s func\n",p_headset_key->detect_count,__FUNCTION__);
	yl_debug("p_headset_key->detect_count = %d, in %s func\n",p_headset_key->detect_count,__FUNCTION__);

    if(p_headset_key->detect_count >= MAX_DETECT_COUNT)
	{
        p_headset_key->timer_running=0;
        hrtimer_cancel(&p_headset_key->timer);
        p_headset_key->detect_count = 0; 
        if(p_headset_key->crnt_level!=p_headset_key->pass_key)
        {
        	if(zoom2_headset_key2)
        	{
        		yl_debug("input_report_key p_headset_key->crnt_level = %d, in %s func\n",p_headset_key->crnt_level,__FUNCTION__);
				input_report_key(zoom2_headset_key2, KEY_OK, p_headset_key->crnt_level);
				input_sync(zoom2_headset_key2);
				p_headset_key->pass_key = p_headset_key->crnt_level;
			}
        }
//        hrtimer_cancel(&p_headset_key->timer);
//        up(&p_headset_key->sem);

	}
	else
	{
	    p_headset_key->timer_running=1;
	    
	   	if(p_headset_key->detect_count >= (MAX_DETECT_COUNT - 14) &&
			p_headset_key->crnt_level == p_headset_key->pass_key)
		
		{
			yl_debug("the pass_key = %d, crnt_level = %d, skip to input_report_key in %s func\n",
							p_headset_key->pass_key,p_headset_key->crnt_level,__FUNCTION__);

		    p_headset_key->timer_running=0;
		    hrtimer_cancel(&p_headset_key->timer);
		    p_headset_key->detect_count = 0; 
	 	}
		else
		{
        	yl_debug("restart:detect_count=%d,hsmic=%d,HEADSET_KEY=%d\n",p_headset_key->detect_count,get_twl4030_hsmic_status(),gpio_get_value(HEADSET_KEY_GPIO));
			hrtimer_start(&p_headset_key->timer, ktime_set(0, DETECT_DELAY_TIME*1000000), HRTIMER_MODE_REL);
		}
	}
}
#ifdef CONFIG_PM
// Tushar - now suspend and resume is called from twl4030.c (sound/soc/codecs) suspend and resume
int zoom2_headset_key_suspend(void)//Modified by Tushar
//static int zoom2_headset_key_suspend(struct platform_device *pdev, pm_message_t state)
{
    yl_debug("enter %s func",__FUNCTION__);
    
	if(0 == smartphone_calling_enable)
    {
	    if(headset_key_irq)
	    {
	        disable_irq(headset_key_irq);
	    }
    
	    if(p_headset_key->timer_running)
	    {    
	        p_headset_key->timer_running=0;
	        hrtimer_cancel(&p_headset_key->timer);
	        cancel_work_sync(&p_headset_key->work);	
	    }
    
//      omap_cfg_reg(AF11_34XX_GPIO_14);
//		omap_mux_init_signal("gpio_14", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
    }
	return 0;
}

int zoom2_headset_key_resume(void)//Modified by Tushar
//static int zoom2_headset_key_resume(struct platform_device *pdev)
{	
    yl_debug("enter %s func",__FUNCTION__);
    if(0 == smartphone_calling_enable)
    {
//      omap_cfg_reg(W13_3430_USB1HS_PHY_DATA0);
//		omap_mux_init_signal("gpio_14", OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP);
        gpio_direction_input(HEADSET_KEY_GPIO);

	    if(headset_key_irq)
	    {
	        enable_irq(headset_key_irq);
	    }
	}

	return 0;
}

#else

#define zoom2_headset_key_suspend	NULL
#define zoom2_headset_key_resume	NULL

#endif

static int __devinit zoom2_headset_key_probe(struct platform_device *pdev)
{
	struct input_dev *zoom2_headset_key;
	int err=0;
	
	//int headset_key_irq = platform_get_irq(pdev, 0);

	//power key handle
	zoom2_headset_key = input_allocate_device();
	if (!zoom2_headset_key) 
	{
		printk(KERN_ERR"Can't allocate zoom2_headset_key\n");
		err = -ENOMEM;
		goto out;
	}

	gpio_request(HEADSET_KEY_GPIO,"headset key twl5030 gpio6");
	gpio_direction_input(HEADSET_KEY_GPIO);
	headset_key_irq = gpio_to_irq(HEADSET_KEY_GPIO);
	
	//IRQF_TRIGGER_FALLING
	err = request_irq(headset_key_irq, zoom2_headset_key_irq,
			 IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			"headset key irq", zoom2_headset_key);
			
	if (err < 0) 
	{
		printk(KERN_ERR"Can't get IRQ for headset_key: %d\n", err);
		goto free_input_dev;
	}

	zoom2_headset_key->evbit[0] = BIT_MASK(EV_KEY);
//	zoom2_headset_key->keybit[BIT_WORD(KEY_OK)] = BIT_MASK(KEY_OK);
	zoom2_headset_key->name = "headset_key";
	zoom2_headset_key->phys = "headset_key/input0";
	zoom2_headset_key->dev.parent = &pdev->dev;

	input_set_capability(zoom2_headset_key, EV_KEY, KEY_OK);
	input_set_capability(zoom2_headset_key, EV_KEY, KEY_PHONE);

	platform_set_drvdata(pdev, zoom2_headset_key);

	err = input_register_device(zoom2_headset_key);
	if (err) 
	{
		printk(KERN_ERR"Can't register headset_key: %d\n", err);
		goto free_irq_and_out;
	}

	zoom2_headset_key2 = zoom2_headset_key;
	return 0;

free_irq_and_out:
	free_irq(headset_key_irq, NULL);
free_input_dev:
	input_free_device(zoom2_headset_key);
out:
	return err;
}

static int __devexit zoom2_headset_key_remove(struct platform_device *pdev)
{
	struct input_dev *zoom2_headset_key = platform_get_drvdata(pdev);
	int irq = gpio_to_irq(HEADSET_KEY_GPIO);

	free_irq(irq, zoom2_headset_key);
	input_unregister_device(zoom2_headset_key);

	return 0;
}

struct platform_driver zoom2_headset_key_driver = {
	.probe		= zoom2_headset_key_probe,
	.remove		= __devexit_p(zoom2_headset_key_remove),
// Tushar - now suspend and resume is called from twl4030.c (sound/soc/codecs) suspend and resum
//    .suspend    = zoom2_headset_key_suspend,
//    .resume     = zoom2_headset_key_resume,
	.driver		= {
		.name	= "headset_key",
		.owner	= THIS_MODULE,
	},
};

static int __init zoom2_headset_key_init(void)
{
	memset(p_headset_key,0,sizeof(struct headset_key_struct));

	headset_key_wq = create_singlethread_workqueue("headset_key_wq");
	
//    printk("zoom2_headset_key_init:headset_key_wq=%x\n",headset_key_wq);
	if (!headset_key_wq)
		return -ENOMEM;
	
    sema_init(&p_headset_key->sem, 1);
	INIT_WORK(&p_headset_key->work, headset_key_work_func);
    p_headset_key->timer_running = 0;

    hrtimer_init(&p_headset_key->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    p_headset_key->timer.function = headset_key_timer_func;

    p_headset_key->pass_key=-1;
    p_headset_key->pre_level=-1;
	return platform_driver_register(&zoom2_headset_key_driver);
}
module_init(zoom2_headset_key_init);

static void __exit zoom2_headset_key_exit(void)
{
	cancel_work_sync(&p_headset_key->work);
    if (headset_key_wq)
    {
		destroy_workqueue(headset_key_wq);
	}
    
	platform_driver_unregister(&zoom2_headset_key_driver);
}
module_exit(zoom2_headset_key_exit);

MODULE_ALIAS("yulong: cp9130");
MODULE_DESCRIPTION("headset key driver");
MODULE_AUTHOR("guotao@yulong.com");


/*
 *  drivers/switch/switch_gpio.c
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <plat/misc.h>  // for smartphone_calling_enable
#include <plat/yl_debug.h>
#include <linux/slab.h>
#include "../../arch/arm/mach-omap2/mux.h"

#define ONE_KEY_MUTE 15
#define HALL_KEY     39
#define HEADSET_KEY  (OMAP_MAX_GPIO_LINES + 2)  /* TWL4030 GPIO_2 */


extern int get_twl4030_hsmic_status(void);
extern void set_twl4030_hsmic_status(int);

static 	struct gpio_switch_data *g_switch_data;
static  int headset_plug_index = -1;
static struct workqueue_struct *switch_gpio_wq;
static struct delayed_work delayed_work;
static struct delayed_work *p_delayed_work = &delayed_work;

#define    HEADSET_KEY_GPIO     14

#define	   DETECT_DELAY_TIME	20
#define    MAX_DETECT_COUNT		10

static int headset_plug_status = 1;
static int headset_plug_status2 = 1;

//键盘相关部分
typedef enum
{
  SWITCH_NOCHANGE, //Key is in up position. 
  SWITCH_CHANGING  //Report key press and wait for key to be up
} switch_state_type;

struct switch_params_struct {
    unsigned int switch_hold_time;    
    unsigned char switch_crnt_value;
    unsigned char switch_changed;
    switch_state_type switch_state;
    unsigned char switch_passed;    
};

struct gpio_switch_data {
	struct switch_dev sdev;
	
	struct switch_params_struct switch_params;
	unsigned char num_resources;
	unsigned char gpio;	
	unsigned char gpio_irq_state;
	unsigned short on_time;	
	unsigned short off_time;	
	unsigned short report_time;
	const char *name_on;
	const char *name_off;
	const char *state_on;
	const char *state_off;
	int irq;
	struct work_struct work;
	
	struct hrtimer timer;
	int timer_running;
};
struct gpio_switch_data *switch_data=NULL;
//struct input_dev *gpio_switch_input_dev=NULL;

int get_headset_irq_state(void)
{
    if(headset_plug_index<0)
      return headset_plug_index;
	return switch_data[headset_plug_index].gpio_irq_state;
}

int get_headset_plug_irq_number(void)
{
    if(headset_plug_index<0)
      return headset_plug_index;
      
	return switch_data[headset_plug_index].irq;
}

int get_headset_plug_status(void)
{
	return headset_plug_status;
}

int get_headset_plug_status2(void)
{
	return headset_plug_status2;
}

int  IsEarPhoneInsert(void)
{
	int state = 0;
	
//	state = gpio_get_value(GPIO_HEAD_DET);
	state = headset_plug_status; 
	if(state == 1)   
    {
        return 1;
    }
    else		//Insert state == 0
    {
    	//printk("EarPhone Is Insert in %s func\n",__FUNCTION__); 
		return 0;
    }
}

extern int zoom_is_lcd_on(void);

static void gpio_swtich_powerbutton_handle(unsigned char switch_value)
{
    char report_power_key=0;
#if 0
    if(zoom_is_lcd_on())//亮屏情况下，插入皮套，发POWER灭屏
    {
        if(switch_value)
        {
            report_power_key=1;
        }

    }
    else if(switch_value==0)//灭屏情况下，拔出皮套，发POWER亮屏
    {
        report_power_key=1;
    }
   
    if(report_power_key)
    {
    	input_report_key(gpio_switch_input_dev, KEY_POWER, 1);
    	input_sync(gpio_switch_input_dev);
    	input_report_key(gpio_switch_input_dev, KEY_POWER, 0);
    	input_sync(gpio_switch_input_dev);
	}
#endif
}

void delayed_work_func(struct work_struct *work)
{
	headset_plug_status2 = headset_plug_status;
}

static void gpio_switch_work(struct work_struct *work)
{    
    unsigned int i;
    char disable_time=1;
	
    //printk("enter %s func,num_resources=%d\n",__FUNCTION__,switch_data[0].num_resources);

    #if 1
    //unsigned int i;
    for (i = 0 ; i < switch_data[0].num_resources; i++) 
    {
    
        if(switch_data[i].gpio_irq_state)
        {
            disable_irq(gpio_to_irq(switch_data[i].gpio));
            
            switch_data[i].gpio_irq_state=0;
        }
    }   
    #endif

    //扫描触摸按键状态
    for(i=0;i<switch_data[0].num_resources;i++)
	{	    
	    if(switch_data[i].switch_params.switch_passed || switch_data[i].switch_params.switch_crnt_value==gpio_get_value(switch_data[i].gpio))
	    {
	        switch_data[i].switch_params.switch_changed=0;
	    }
	    else 
	    {
        	switch_data[i].switch_params.switch_changed=1;
        }
	}

    //根据按键状态，处理上报事件和开关定时器状态 
    for(i=0;i<switch_data[0].num_resources;i++)
    {
        switch (switch_data[i].switch_params.switch_state)
        {
            /* If key was up, de-bounce it if key is now down.
            */
            case SWITCH_NOCHANGE:
              if ( switch_data[i].switch_params.switch_changed )
              {
                switch_data[i].switch_params.switch_state = SWITCH_CHANGING;
                switch_data[i].switch_params.switch_hold_time = 0;
                
                disable_time = 0;
              }
              else
              {
                switch_data[i].switch_params.switch_state = SWITCH_NOCHANGE;
              }
              break;

            /* If waiting for key to be up
            */
            case SWITCH_CHANGING:
              if ( switch_data[i].switch_params.switch_changed )
              {
                 disable_time = 0;     
				 unsigned int delay = 2000;

                 if(!switch_data[i].switch_params.switch_passed)
                    {
                        switch_data[i].switch_params.switch_hold_time += DETECT_DELAY_TIME;
                        if(switch_data[i].switch_params.switch_hold_time>= switch_data[i].report_time)
                          {                                  
                              switch_data[i].switch_params.switch_passed=1;          
							  if(HEADSET_KEY==switch_data[i].gpio)
                              {                            
                                  //modified by guotao, 2010-09-08
                                  headset_plug_status=switch_data[i].switch_params.switch_crnt_value;
								  printk("headset_plug_status=%d, in %s func\n", !headset_plug_status, __func__);
                                  switch_set_state(&switch_data[i].sdev, !headset_plug_status);  
								  cancel_delayed_work_sync(p_delayed_work);
                                  if (headset_plug_status == 0) {
                                  	set_twl4030_hsmic_status(1);
									schedule_delayed_work(p_delayed_work, msecs_to_jiffies(delay));
                                  } else {
                                  	set_twl4030_hsmic_status(0);
									headset_plug_status2 = headset_plug_status;
                                  }
                              }
                              else
                              {
          						switch_set_state(&switch_data[i].sdev, switch_data[i].switch_params.switch_crnt_value);   
                              }

                              switch_data[i].switch_params.switch_crnt_value=!switch_data[i].switch_params.switch_crnt_value;

                              //if(HALL_KEY==switch_data[i].gpio)
                              //{
                                  //gpio_swtich_powerbutton_handle(!switch_data[i].switch_params.switch_crnt_value);
                              //}

                              yl_debug( "gpio=%d,state=%d in %s func\n",switch_data[i].gpio,switch_data[i].switch_params.switch_crnt_value,__FUNCTION__);
                              if(switch_data[i].switch_params.switch_crnt_value==1)
                              {
                                  switch_data[i].report_time=switch_data[i].on_time;
                              }
                              else
                              {
                                  switch_data[i].report_time=switch_data[i].off_time;
                              }
                          }                           
                    }

              }
              else
              {
                disable_time = 0;     
                switch_data[i].switch_params.switch_state = SWITCH_NOCHANGE;
                switch_data[i].switch_params.switch_passed=0;                
                //printk( "release: gpio=%d,state=%d\n",switch_data[i].gpio,switch_data[i].switch_params.switch_crnt_value);
              }
              break;

            default:
              /* change here to recover */
              printk( "Illegal key state %d", switch_data[i].switch_params.switch_state);
              /* does not return */
        }
        
        //printk("switch_hold_time[%d]=%d,switch_crnt_value=%d,switch_changed=%d\n",i,switch_data[i].switch_params.switch_hold_time,switch_data[i].switch_params.switch_crnt_value,switch_data[i].switch_params.switch_changed);
        
        //printk("switch_state[%d]=%d,switch_passed=%d,report_time=%d\n",i,switch_data[i].switch_params.switch_state,switch_data[i].switch_params.switch_passed,switch_data[i].report_time);

    }


    if(disable_time)
    {    
        switch_data[0].timer_running=0;
        hrtimer_cancel(&switch_data[0].timer);
        #if 1
    	for (i = 0 ; i < switch_data[0].num_resources; i++) 
    	{
    	    
            if(switch_data[i].gpio_irq_state==0)
            {
    		    enable_irq(gpio_to_irq(switch_data[i].gpio));    		    
                switch_data[i].gpio_irq_state=1;
            }
    	}	
    	#endif
    }
    else
    {
       switch_data[0].timer_running=1;
       hrtimer_start(&switch_data[0].timer, ktime_set(0, DETECT_DELAY_TIME*1000000), HRTIMER_MODE_REL);
    }
}

static enum hrtimer_restart gpio_switch_timer(struct hrtimer *timer)
{   
   schedule_work(&switch_data[0].work);
   
   return HRTIMER_NORESTART;
}

static irqreturn_t gpio_switch_irq_handler(int irq, void *dev_id)
{
#if 0
    unsigned int i;
	for (i = 0 ; i < switch_data[0].num_resources; i++) 
	{
	
        if(switch_data[i].gpio_irq_state)
        {
		    //disable_irq_nosync(gpio_to_irq(switch_data[i].gpio));
		    
            switch_data[i].gpio_irq_state=0;
        }
	}	
#endif
    //printk("irq=%d in gpio_switch_irq_handler\n",irq);

	if(switch_data[0].timer_running == 0)
    {
       switch_data[0].timer_running=1;
       hrtimer_start(&switch_data[0].timer, ktime_set(0, 1000), HRTIMER_MODE_REL);
    }	
	return IRQ_HANDLED;
}

static ssize_t switch_gpio_print_state(struct switch_dev *sdev, char *buf)
{
	struct gpio_switch_data	*switch_data_pointer =
		container_of(sdev, struct gpio_switch_data, sdev);
	const char *state;
	if (switch_get_state(sdev))
		state = switch_data_pointer->state_on;
	else
		state = switch_data_pointer->state_off;

	if (state)
		return sprintf(buf, "%s\n", state);
	return -1;
}

#ifdef CONFIG_PM

static int gpio_switch_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct gpio_switch_data *switch_data= platform_get_drvdata(pdev);
	int i;

    yl_debug("enter %s func",__FUNCTION__);
    
    for(i = 0; i < pdev->num_resources; i++)
    {
        if(HALL_KEY==switch_data[i].gpio)
        {
            continue;
        }
		else if(switch_data[i].gpio == HEADSET_KEY && smartphone_calling_enable) //HEADSET_KEY denote headset detect IRQ pin- tushar
	    {
            continue;
        }

        if(switch_data[i].gpio_irq_state)
        {
            disable_irq(switch_data[i].irq);
            switch_data[i].gpio_irq_state=0;
        }
    }
    
    if(switch_data[0].timer_running&& smartphone_calling_enable== 0) //HEADSET_KEY denote headset detect IRQ pin)- tushar
    {    
        switch_data[0].timer_running=0;
        hrtimer_cancel(&switch_data[0].timer);
    }
	return 0;
}

static int gpio_switch_resume(struct platform_device *pdev)
{
	struct gpio_switch_data *switch_data= platform_get_drvdata(pdev);
	int i;
	
    yl_debug("enter %s func",__FUNCTION__);

    for(i = 0; i < pdev->num_resources; i++)
    {
        if(switch_data[i].gpio_irq_state==0)
        {
            enable_irq(switch_data[i].irq);
            switch_data[i].gpio_irq_state=1;
        }
    }

	if(switch_data[0].timer_running == 0) //HEADSET_KEY denote headset detect IRQ pin)- tushar
	{
		switch_data[0].timer_running=1;
		hrtimer_start(&switch_data[0].timer, ktime_set(0, 1000), HRTIMER_MODE_REL);
	}

	return 0;
}

#else

#define gpio_switch_suspend	NULL
#define gpio_switch_resume	NULL

#endif

static int gpio_switch_probe(struct platform_device *pdev)
{
	int ret = 0;
	int i;
		
    switch_data = kcalloc(pdev->num_resources,sizeof(struct gpio_switch_data), GFP_KERNEL);
	if (!switch_data)
		return -ENOMEM;
		
	platform_set_drvdata(pdev, switch_data);
#if 0
	gpio_switch_input_dev = input_allocate_device();
	if (gpio_switch_input_dev == NULL) {
		input_free_device(gpio_switch_input_dev);
		goto err_switch_dev_register;
	}

	gpio_switch_input_dev->name = "hall key";
	set_bit(EV_SYN, gpio_switch_input_dev->evbit);
	set_bit(EV_KEY, gpio_switch_input_dev->evbit);

    input_set_capability(gpio_switch_input_dev, EV_KEY, KEY_POWER);	
	ret = input_register_device(gpio_switch_input_dev);
	if (ret) 
	{
		printk(KERN_ERR"Can't register gpio_switch_input_dev: %d\n", ret);
		goto err_input_register_device_failed;
	}
#endif

    memset(switch_data,0,pdev->num_resources*sizeof(*switch_data));      

    for(i = 0; i < pdev->num_resources; i++)
    {
        switch_data[i].num_resources= pdev->num_resources;
    	switch_data[i].sdev.name    = pdev->resource[i].name;
    	switch_data[i].gpio         = pdev->resource[i].start;
    	switch_data[i].on_time      = (pdev->resource[i].end>>16)&0xffff;    	
    	switch_data[i].off_time     = pdev->resource[i].end&0xffff;    	
        switch_data[i].report_time  = 50;
    	switch_data[i].sdev.print_state = switch_gpio_print_state;
    	ret = switch_dev_register(&switch_data[i].sdev);
    	if(ret<0)
    	{
    	    //printk("failed to register switch device at %d\n",i);
            break;
    	}
    	//printk("gpio[%d] ontime[%d] offtime[%d]\n",switch_data[i].gpio,switch_data[i].on_time,switch_data[i].off_time);
    }
	if (ret < 0)
	{
	    for(--i; i >= 0; i--)
	    {            
            switch_dev_unregister(&switch_data[i].sdev);
	    }
		goto err_switch_dev_register;
	}
	
    switch_data[0].timer_running=0;
	hrtimer_init(&switch_data[0].timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	switch_data[0].timer.function = gpio_switch_timer;

    INIT_WORK(&switch_data[0].work, gpio_switch_work);
	INIT_DELAYED_WORK(p_delayed_work, delayed_work_func);

    //配置GPIO
    //omap_mux_init_signal("gpio_15", OMAP_PIN_INPUT_PULLUP);    
    //omap_mux_init_signal("gpio_39", OMAP_PIN_INPUT | OMAP_WAKEUP_EN);
    
    for(i = 0; i < pdev->num_resources; i++)
    {
        //printk("gpio[%d]request at %d\n",switch_data[i].gpio,i);
    	ret = gpio_request(switch_data[i].gpio, switch_data[i].sdev.name);
    	if (ret < 0)
    	{
    	    //printk("failed to gpio_request at %d\n",i);
    		goto err_request_gpio;
    	}
    	
    	gpio_direction_input(switch_data[i].gpio);
        switch_data[i].switch_params.switch_crnt_value=!gpio_get_value(switch_data[i].gpio);

    	switch_data[i].irq = gpio_to_irq(switch_data[i].gpio);
    	if (switch_data[i].irq < 0) {
    		ret = switch_data[i].irq;
    		//printk("failed to exe gpio to irq at %d\n",i);
    		goto err_detect_irq_num_failed;
    	}
        //printk("gpio[%d]=irq[%d]\n",switch_data[i].gpio,switch_data[i].irq);
        //modified by guotao, 2010-01-04
    	ret = request_irq(switch_data[i].irq, gpio_switch_irq_handler,
    			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
    			switch_data[i].sdev.name,
    			&switch_data[i]);
    	if (ret < 0)
    	{
    	    //printk("failed to request irq %d\n",i);
    	    switch_data[i].gpio_irq_state=0;
    		goto err_request_irq;
    	}
    	switch_data[i].gpio_irq_state=1;
    	if(switch_data[i].gpio==HEADSET_KEY)
    	{
            headset_plug_index=i;
    	}
    }  

    /* Perform initial detection */
    //gpio_switch_work(&switch_data[0].work);

    if(get_twl4030_hsmic_status() == 0)
    {        
        set_twl4030_hsmic_status(1);
        switch_data[headset_plug_index].switch_params.switch_crnt_value=!gpio_get_value(switch_data[headset_plug_index].gpio);
	}
	
    gpio_switch_irq_handler(switch_data[headset_plug_index].irq,NULL);

	return 0;

err_request_irq:
err_detect_irq_num_failed:
    for(--i; i >= 0; i--)
    {
	    gpio_free(switch_data[i].gpio);
	}

err_request_gpio:

    for(i = 0; i < pdev->num_resources; i++)
    {
        switch_dev_unregister(&switch_data[i].sdev);
    }

err_input_register_device_failed:
	//input_free_device(gpio_switch_input_dev);

err_switch_dev_register:

	kfree(switch_data);

	return ret;
}

static int __devexit gpio_switch_remove(struct platform_device *pdev)
{
	int i;
	
    for(i=0;i<pdev->num_resources;i++)
    {    
	    cancel_work_sync(&switch_data[i].work);
    	free_irq(switch_data[i].irq,&switch_data[i]);
        switch_dev_unregister(&switch_data[i].sdev);
    }
    if(switch_data[0].timer_running)
    {    
        switch_data[0].timer_running=0;
        hrtimer_cancel(&switch_data[0].timer);
    }

    //input_unregister_device(gpio_switch_input_dev);

	kfree(switch_data);

	return 0;
}
 
static struct platform_driver gpio_switch_driver = {
	.probe		= gpio_switch_probe,
	.remove		= __devexit_p(gpio_switch_remove),
	.suspend	= gpio_switch_suspend,
	.resume		= gpio_switch_resume,
	.driver		= {
		.name	= "switch-gpio",
		.owner	= THIS_MODULE,
	},
};

static int __init gpio_switch_init(void)
{
	return platform_driver_register(&gpio_switch_driver);
}

static void __exit gpio_switch_exit(void)
{
	cancel_work_sync(&g_switch_data->work);
    if (switch_gpio_wq)
    {
		destroy_workqueue(switch_gpio_wq);
	}
	
	platform_driver_unregister(&gpio_switch_driver);
}

#ifndef MODULE
late_initcall(gpio_switch_init);
#else
module_init(gpio_switch_init);
#endif
module_exit(gpio_switch_exit);

MODULE_AUTHOR("Mike Lockwood <lockwood@android.com>");
MODULE_DESCRIPTION("GPIO Switch driver");
MODULE_LICENSE("GPL");

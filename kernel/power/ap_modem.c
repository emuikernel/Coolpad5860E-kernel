/*
 * linux/drivers/power/ap_modem.c
 *
 * VIA CBP driver for Linux
 *
 * Copyright (C) 2009 VIA TELECOM Corporation, Inc.
 * Author: VIA TELECOM Corporation, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/wakelock.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/gpio.h>

#include <linux/regulator/consumer.h>


#include <plat/control.h>
//#include <plat/mux.h>
//#include <mach/mux.h>
#include "../../arch/arm/mach-omap2/mux.h"
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/usb.h>

#include <mach/gpio_define_for_cp5860e.h>

#include "../drivers/usb/core/usb.h"

#define POWER_HOLD_DELAY    (500) //ms
#define RESET_HOLD_DELAY	(100) //ms
#define POWER_ON_DELAY      3000 //ms
#define WAKE_HOLD_DELAY	     2 //ms	

#define MDM_WAKEUP_LOCK_TIME (3) // seconds that lock the console to susupend
#define MDM_RESET_LOCK_TIME  (3)
#define WAKE_LEVEL	(1)
#define SLEEP_LEVEL	(!(WAKE_LEVEL))

#define POWER_CBP_AT_BOOT
//#define SUPPORT_AP_READY
//#define VIA_AP_MODEM_DEBUG
#ifdef VIA_AP_MODEM_DEBUG
#undef dbg
#define dbg(format, arg...) printk("[AP_MODEM]: " format "\n" , ## arg)
#else
#undef dbg
#define dbg(format, arg...) do {} while (0)
#endif

#define VENDOR_ID			(0x15eb)
#define PRODUCT_ID      	(0x0001)

#define RH_VENDOR_ID		(0x1d6b)
#define RH_PRODUCT_ID		(0x0001)

struct ap_modem{
    struct wake_lock modem_lock;
    struct wake_lock reset_lock;
    struct regulator *vsim;
    int wake_count;
    spinlock_t lock;
};
static struct ap_modem ap_mdm;
static struct ap_modem *pmdm = &ap_mdm;

static struct usb_interface *if_at = NULL;
static struct delayed_work mdm_wake_usb_work;

/*Emulate modem detached by pull P0M1 when CBP jump*/
#define MDM_JUMP_DELAY		1100 //time for emulating detached by pull P0M1

/*Emulate modem detached by pull P0M1 when CBP reset*/
#define MDM_RESET_DELAY		5500 //time for emulating detached by pull P0M1

#define MDM_WAKEUP_LOCK_TIME (3) // seconds that lock the console to susupend
#define MDM_OP_LOCK_TIME	20 // seconds that lock the console to susupend

static DECLARE_WAIT_QUEUE_HEAD(mdm_pull_delay);

static void via_freeze_pins(void);
static void via_unfreeze_pins(void);
static int via_gpio_init(void);

static int at_if_count = 0;
static bool via_gpio_init_finish = 0;

static void release_wake_lock(struct work_struct *data)
{
	//printk("release_wake_lock+++++++++++++++++++++++++++++++\n");
	wake_unlock(&pmdm->modem_lock);//hey add 12-24
    if (if_at && at_if_count > 0) {
        usb_autopm_put_interface(if_at);
	    if(--at_if_count==0)
            if_at = NULL;
    }
}

static int usb_be_sleep(void)
{
    dbg(" ====usb_be_sleep=[%d] \n",gpio_get_value(GPIO_AP_WAKE_MDM));
    printk(KERN_ERR " ====usb_be_sleep=[%d] \n",gpio_get_value(GPIO_AP_WAKE_MDM));
    return gpio_get_value(GPIO_AP_WAKE_MDM) == SLEEP_LEVEL;
}

struct cp_usb_device {
    struct usb_device * cp_dev;
    struct mutex cp_mutex;
};

static struct cp_usb_device _cp_usb_dev = {
    .cp_dev = NULL,
};
void ap_bind_usb_device(struct usb_device *dev) {
    dbg("%s: enter\n", __func__);
    dbg("dev=0x%p, name=%s, devp=0x%p, name=%s\n",
        dev, kobject_name(&dev->dev.kobj), dev->parent, kobject_name(&dev->parent->dev.kobj));
    
    if(_cp_usb_dev.cp_dev)
    {
        printk(KERN_ERR "%s: already initialized\n", __func__);
        return;
    }
    mutex_init(&(_cp_usb_dev.cp_mutex));
    mutex_lock(&(_cp_usb_dev.cp_mutex));
    if (_cp_usb_dev.cp_dev && _cp_usb_dev.cp_dev!= dev)
        printk(KERN_ERR "cp usb dev(%p) has binded, overwrite with(%p).\n",
                _cp_usb_dev.cp_dev, dev);
    _cp_usb_dev.cp_dev = dev;
    mutex_unlock(&(_cp_usb_dev.cp_mutex));

    return;
}
EXPORT_SYMBOL_GPL(ap_bind_usb_device);

void ap_unbind_usb_device(void) {
    dbg("%s: enter\n", __func__);

    if(_cp_usb_dev.cp_dev)
    {
        mutex_lock(&(_cp_usb_dev.cp_mutex));
        _cp_usb_dev.cp_dev = NULL;
        mutex_unlock(&(_cp_usb_dev.cp_mutex));
        mutex_destroy(&(_cp_usb_dev.cp_mutex));
    }
    else
    {
        printk(KERN_ERR "%s: already destroy, do noting\n", __func__);
    }
    return;
}
EXPORT_SYMBOL_GPL(ap_unbind_usb_device);


extern int in_dpm_suspend;
static int has_error_flag = 0;

/* added by sguan, for usb device wakeup */
static void usb_wakeup(struct work_struct *p)
{
    struct usb_device *udev = NULL;
    int usbsleep;
   
    usbsleep = usb_be_sleep();
	printk(KERN_ERR "usb_wakeup enter===>usbsleep=%d\n",usbsleep);
	if(usbsleep){
		//int count = 0;
		for(;;)
		{
			if(in_dpm_suspend)
			{
				has_error_flag = 1;
                set_current_state(TASK_UNINTERRUPTIBLE);
				schedule_timeout(msecs_to_jiffies(500));
				//if (count++ > 20)
				//	break;
			}
			else
			{
				printk(KERN_ERR "%s: in_dpm_suspend change, can wake RH\n", __func__);
				break;
			}
		}
#if 0	
		printk("usb_be_sleep()\n");
        udev = usb_find_device(VENDOR_ID, PRODUCT_ID);
        if(!udev)
        {
            if_at = NULL;
            printk(KERN_ERR "%s-usb find device error\n", __func__);
            udev = usb_find_device(RH_VENDOR_ID, RH_PRODUCT_ID);
            if(!udev){
                printk(KERN_ERR "%s-usb find root hut device error\n", __func__);
                return ;
            }
        } else {
            if_at = usb_ifnum_to_if(udev, 0);
            if (if_at){			
		        at_if_count++;	
                usb_autopm_get_interface(if_at);
            }
		    else{
		        printk(KERN_ERR "%s-if_at==NULL\n", __func__);
		        return;
		    } 
        }
#else
	    if(has_error_flag)
	    {   
		    printk(KERN_ERR "%s: sguan--WARNING, usb_wakeup occur before dpm_resume!!!!!\n", __func__);
	    }
	    has_error_flag = 0;

	    if(_cp_usb_dev.cp_dev == NULL)
	    {
		    printk(KERN_ERR "%s: Can't find cp usb devices, error!!\n", __func__);
		    return;
	    }
	    mutex_lock(&(_cp_usb_dev.cp_mutex));
	    udev = _cp_usb_dev.cp_dev;
	    usb_get_dev(udev);
	    if (!udev) {
		    printk(KERN_ERR "%s - usb find device error\n", __func__);
		    mutex_unlock(&(_cp_usb_dev.cp_mutex));
		    return;
	    }
#endif
        dbg("atuo resume the udev by MDM_WAKE\n");
        usb_lock_device(udev);
        usb_mark_last_busy(udev);
        if(usb_autoresume_device(udev) == 0)
        {
			printk("usb_autoresume_device()==0\n");
            usb_autosuspend_device(udev);
        }
        usb_unlock_device(udev);

        usb_put_dev(udev);
#if 0	
		if (if_at && at_if_count > 0)
		schedule_delayed_work(&mdm_wake_usb_work,HZ);
#else
        mutex_unlock(&(_cp_usb_dev.cp_mutex));
#endif		
    }
   
	printk(KERN_ERR "usb_wakeup exit\n");
    return;
}

/* End of added by sguan */

static DECLARE_WORK(usb_wakeup_work, usb_wakeup);

//add by Aliang for modem sleep
void ap_pullup_modem(void)
{
    //printk(KERN_ERR" ########ap_pullup_gpio129_to_modem###### \n");
    //gpio_direction_output(GPIO_MDM_RDY, WAKE_LEVEL);
    //mdelay(WAKE_HOLD_DELAY);
}

void ap_pulldown_modem(void)
{
    //printk(KERN_ERR" #######ap_pulldown_gpio129_to_modem##### \n");
    //gpio_direction_output(GPIO_MDM_RDY, SLEEP_LEVEL);
    //mdelay(WAKE_HOLD_DELAY);
}


void wake_modem(void)
{
    dbg(" ====wake_modem \n");
    printk(KERN_ERR " ====wake_modem \n");
    gpio_direction_output(GPIO_AP_WAKE_MDM, WAKE_LEVEL);
    mdelay(WAKE_HOLD_DELAY);
}
EXPORT_SYMBOL_GPL(wake_modem);

void sleep_modem(void)
{
    dbg(" ====sleep_modem \n");
    printk(KERN_ERR " ====sleep_modem \n");

	if(via_gpio_init_finish)
	{
    	gpio_direction_output(GPIO_AP_WAKE_MDM, SLEEP_LEVEL);
    	mdelay(WAKE_HOLD_DELAY);
	}
	else
	{
		via_gpio_init();
		via_gpio_init_finish = 1;

		gpio_direction_output(GPIO_AP_WAKE_MDM, SLEEP_LEVEL);
    	mdelay(WAKE_HOLD_DELAY);
	}
}
EXPORT_SYMBOL_GPL(sleep_modem);

#ifdef SUPPORT_AP_READY
int ap_set_modem_sleep(int force)
{
    unsigned long flags;

    dbg("%s set(%d) modem sleep.\n", force?"Forcely":"Normally", pmdm->wake_count);

    spin_lock_irqsave(&pmdm->lock, flags);
    pmdm->wake_count--;
    if(pmdm->wake_count <= 0 || force){
        pmdm->wake_count = 0;
    }
    spin_unlock_irqrestore(&pmdm->lock, flags);

    /*nobody need CBP be waken*/
    if(pmdm->wake_count == 0){
        sleep_modem();
    }

    return pmdm->wake_count;
}


int ap_set_modem_wake(int force)
{
    unsigned long flags;

    dbg("%s set(%d) modem wake.\n", force?"Forcely":"Normally", pmdm->wake_count);

    spin_lock_irqsave(&pmdm->lock, flags);
    pmdm->wake_count++;
    if(pmdm->wake_count <= 0 || force){
        pmdm->wake_count = 1;
    }
    spin_unlock_irqrestore(&pmdm->lock, flags);

    if(pmdm->wake_count == 1){
        wake_modem();
    }

    return pmdm->wake_count;
}
#else
int ap_set_modem_sleep(int force)
{
    return 0;
}
int ap_set_modem_wake(int force)
{
    return 0;
}
#endif

EXPORT_SYMBOL(ap_set_modem_wake);
EXPORT_SYMBOL(ap_set_modem_sleep);

void ap_poweroff_modem(void)
{
	 printk("ap_poweroff_modem!\n");
     gpio_direction_output(GPIO_POWER_MDM, 0);
     gpio_direction_output(GPIO_RESET_MDM, 1);
     mdelay(RESET_HOLD_DELAY);
     gpio_direction_output(GPIO_RESET_MDM, 0);
     mdelay(1);
	 via_freeze_pins();
}
EXPORT_SYMBOL(ap_poweroff_modem);

ssize_t modem_power_show(
	struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int power = 0;
    int ret = 0;
	int num = 0;

	while((ret == 0) && (num<10)){
		power = gpio_get_value(C_VIA_STATE);//(GPIO_POWER_MDM);
		printk(KERN_ERR "power show gpio41=[%x] \n", power);
		if(power){
		    ret += sprintf(buf + ret, "on\n");
		}else{
		    ret += sprintf(buf + ret, "off\n");
		}
		num++;
	}
	printk("modem_power_show-->buf:%s\n",buf);
    return ret;
}

ssize_t modem_power_store(
	struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t n)
{
    int power;

    /* power the modem */
    if ( !strncmp(buf, "on", strlen("on"))) {
        power = 1;
    }else if(!strncmp(buf, "off", strlen("off"))){
        power = 0;
    }else{
        pr_info("%s: input %s is invalid.\n", __FUNCTION__, buf);
        goto end;
    }

    /* FIXME: AP must power on the GPS and switch the GPS control to CBP,
      * otherwise the CBP will reboot because the error commucation with GPS.
      */
    dbg("power %s modem.\n", power?"on":"off");

    wake_modem();

    printk(KERN_ERR "POWER_STORE=[%x] \n", power);

    if(power){
		/*simulate Power Key pressed to boot CP*/
		omap_mux_init_signal("sim_io.gpio_126",OMAP_PIN_OUTPUT);
		gpio_request(GPIO_POWER_MDM, "modem_power_on");

		via_unfreeze_pins();
        gpio_direction_output(GPIO_RESET_MDM, 0);
        gpio_direction_output(GPIO_POWER_MDM, 1);
        mdelay(POWER_ON_DELAY);
        gpio_direction_output(GPIO_POWER_MDM, 0);

        dbg("Power on the CBP, done");

    }
    else
    {
        ap_poweroff_modem();
		//via_freeze_pins();
        dbg("Power off the CBP, done");
    }

end:
    return n;
}

ssize_t modem_reset_show(
	struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int reset = 0;
    int ret = 0;

    reset = gpio_get_value(GPIO_RESET_MDM);
    if(reset){
        ret += sprintf(buf + ret, "resetting\n");
    }else{
        ret += sprintf(buf + ret, "working\n");
    }

    return ret;
}

ssize_t modem_reset_store(
	struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t n)
{
    /* reset the modem */
    if ( !strncmp(buf, "reset", strlen("reset"))) {
	    printk(KERN_ERR "modem_reset_store!\n");
        //wake_lock_timeout(&pmdm->modem_lock, MDM_OP_LOCK_TIME*HZ);
        /*make sure that OHCI is waken*/
        schedule_work(&usb_wakeup_work);
        wake_lock_timeout(&pmdm->reset_lock, MDM_RESET_LOCK_TIME * HZ);
        schedule_timeout(HZ);

		/*Power off the CP*/
        gpio_direction_output(GPIO_POWER_MDM, 0);
        gpio_direction_output(GPIO_RESET_MDM, 1);
        mdelay(RESET_HOLD_DELAY);
        gpio_direction_output(GPIO_RESET_MDM, 0);
        mdelay(RESET_HOLD_DELAY);
		 
		/*simulate Power Key pressed to boot CP*/
		omap_mux_init_signal("sim_io.gpio_126",OMAP_PIN_OUTPUT);
		gpio_request(GPIO_POWER_MDM, "modem_power_on");
//		printk(KERN_ERR "The via module power on register value is = %4x\n", omap_readl(0x48002a54));

		via_unfreeze_pins();
        gpio_direction_output(GPIO_POWER_MDM, 1);
        mdelay(POWER_HOLD_DELAY);
        gpio_direction_output(GPIO_POWER_MDM, 0);
    }
    return n;
}

ssize_t modem_status_show(
	struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
#if 1
    int sleep = 0;
    int ret = 0;

    sleep = !!gpio_get_value(GPIO_MDM_RDY);
    if(sleep == SLEEP_LEVEL){
        ret += sprintf(buf + ret, "sleep\n");
    }else{
        ret += sprintf(buf + ret, "wake\n");
    }

    return ret;
#else
    int power = 0;
    int ret = 0;

    power = gpio_get_value(C_VIA_STATE);//(GPIO_POWER_MDM);

	printk("power show gpio41=[%x] \n", power);
    if(power){
        ret += sprintf(buf + ret, "on\n");
    }else{
        ret += sprintf(buf + ret, "off\n");
    }
	return ret;

#endif
}

ssize_t modem_status_store(
	struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t n)
{
    return n;
}

ssize_t modem_set_show(
	struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int sleep = 0;
    int ret = 0;

    sleep = !!gpio_get_value(GPIO_AP_WAKE_MDM);
    if(sleep == SLEEP_LEVEL){
        ret += sprintf(buf + ret, "sleep\n");
    }else{
        ret += sprintf(buf + ret, "wake\n");
    }

    return ret;
}

ssize_t modem_set_store(
	struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t n)
{
    if ( !strncmp(buf, "wake", strlen("wake"))) {
        /*Modem must be waken,*/
        //ap_set_modem_wake(1);
        wake_modem();
    }else if( !strncmp(buf, "sleep", strlen("sleep"))){
        /*Modem can be sleep*/
        //ap_set_modem_sleep(1);
        sleep_modem();
    }else{
        dbg("Unknow command.\n");
    }

    return n;
}

/*FIXME: the GPIO_AP_WAKE_MDM maybe should be kept*/
/*static int ap_suspend(struct platform_device *pdev, pm_message_t state)
{
    sleep_modem();
    return 0;
}

static int ap_resume(struct platform_device *pdev)
{
    int sleep;

    sleep = !!gpio_get_value(GPIO_MDM_RDY);
    dbg("%s: Modem request AP to be %s.\n", __FUNCTION__, sleep?"WAKEN":"SLEEP");

    wake_modem();
    return 0;
}*/

/*the action to sleep or wake the modem would be done in OHCI driver*/
static struct platform_driver ap_modem_driver = {
	.driver.name = "ap_modem",
	.suspend = NULL,
	.resume = NULL,
};

static struct platform_device ap_modem_device = {
	.name = "ap_modem",
};

/*
 * modem status interrupt handler
 */
extern void ohci_resume_rh_autostop(void);
static irqreturn_t modem_wake_ap_irq(int irq, void *data)
{
    int sleep;

    sleep = !!gpio_get_value(GPIO_MDM_RDY);
    dbg("Modem request AP to be %s.\n", sleep?"WAKEN":"SLEEP");
    printk(KERN_ERR "Modem request AP to be %s.\n", sleep?"WAKEN":"SLEEP");
#if defined(CONFIG_USB_SUSPEND)
    ohci_resume_rh_autostop();
    if(usb_be_sleep()){
        schedule_work(&usb_wakeup_work);
    }
#endif
    /*skip pm suspend for a while*/
    wake_lock_timeout(&pmdm->modem_lock, MDM_WAKEUP_LOCK_TIME * HZ);
    return IRQ_HANDLED;
}

static int via_gpio_init(void)
{
	int ret = 0;
    int power = 0;
	omap_mux_init_signal("sim_io.gpio_126",OMAP_PIN_OUTPUT);
    ret = gpio_request(GPIO_POWER_MDM, "modem_power_on");
    if(ret < 0)
    {
        pr_err("request GPIO_POWER_MDM(%d) failed\n", GPIO_POWER_MDM);
    }
	gpio_direction_output(GPIO_POWER_MDM, 0);

	omap_mux_init_signal("sim_pwrctrl.gpio_128",OMAP_PIN_INPUT);//此管脚不使用，配置为输入，防漏电
    ret = gpio_request(GPIO_MDM_WAKE_AP, "modem_wakeup_ap");
    if(ret < 0)
    {
        pr_err("request GPIO_MDM_WAKE_AP(%d) failed\n", GPIO_MDM_WAKE_AP);
    }
	gpio_direction_input(GPIO_MDM_WAKE_AP);

	omap_mux_init_signal("sim_clk.gpio_127",OMAP_PIN_INPUT);
    ret = gpio_request(GPIO_AP_WAKE_MDM, "ap_wakeup_mdm");
    if(ret < 0)
    {
        pr_err("request GPIO_AP_WAKE_MDM(%d) failed\n", GPIO_AP_WAKE_MDM);
    }

	omap_mux_init_signal("uart1_rts.gpio_149",OMAP_PIN_INPUT);
    ret = gpio_request(GPIO_RESET_MDM, "reset_mdm");
    if(ret < 0)
    {
        pr_err("request GPIO_RESET_MDM(%d) failed\n", GPIO_RESET_MDM);
    }
	gpio_direction_output(GPIO_RESET_MDM, 0);

	omap_mux_init_signal("sim_rst.gpio_129",OMAP_PIN_INPUT | OMAP_WAKEUP_EN);//模块唤醒AP
    ret = gpio_request(GPIO_MDM_RDY, "mdm_ready");
    if(ret < 0)
    {
        pr_err("request GPIO_MDM_RDY(%d) failed\n", GPIO_MDM_RDY);
    }
	gpio_direction_input(GPIO_MDM_RDY);
	
	omap_mux_init_signal("gpmc_a8.gpio_41",OMAP_PIN_INPUT);//PSHOLD 模块上电后会将该管脚置高电平
	ret = gpio_request(C_VIA_STATE, "get_power");
	if(ret < 0)
    {
        pr_err("request get_power(%d) failed\n", C_VIA_STATE);
    }
	gpio_direction_input(C_VIA_STATE);
	power = gpio_get_value(C_VIA_STATE);//(GPIO_POWER_MDM);

    printk(KERN_ERR "%s+++++++++++++power=%d\n",__func__,power);

	omap_mux_init_signal("mcspi2_simo.gpio_179",OMAP_PIN_INPUT);//未使用，配置为输入，防漏电
	ret = gpio_request(GPIO_MDM_RSV2, "MDM_RSV2");
	if(ret < 0)
    {
        pr_err("request MDM_RSV2(%d) failed\n", GPIO_MDM_RSV2);
    }
	gpio_direction_input(GPIO_MDM_RSV2);

	return ret;
}

static void via_freeze_pins(void)
{
	gpio_direction_output(GPIO_MDM_WAKE_AP, 0);//not used

	gpio_direction_output(GPIO_AP_WAKE_MDM, 0);//ap wake up or sleep modem

	gpio_direction_output(GPIO_MDM_RDY, 0);//modem wake up ap

	gpio_direction_output(C_VIA_STATE, 0);//power state

	gpio_direction_output(GPIO_MDM_RSV2, 0);//not used

	omap_mux_init_signal(USB2_TXSE0_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_request(USB2_TXSE0_PIN,"USB2_TXSE0_PIN");
	gpio_direction_output(USB2_TXSE0_PIN,0);

	omap_mux_init_signal(USB2_TXDAT_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_request(USB2_TXDAT_PIN,"USB2_TXDAT_PIN");
	gpio_direction_output(USB2_TXDAT_PIN,0);
}

static void via_unfreeze_pins(void)
{
	//gpio_29 usb2_txse0 FOR VIA
	omap_mux_init_signal("etk_d15.mm2_txse0",OMAP_PIN_INPUT);

	//gpio_177 USB2_TXDAT for VIA
	omap_mux_init_signal("mcspi1_cs3.mm2_txdat",OMAP_PIN_INPUT);

	gpio_direction_input(GPIO_MDM_WAKE_AP);

	gpio_direction_output(GPIO_AP_WAKE_MDM, 1);

	gpio_direction_input(GPIO_MDM_RDY);

	gpio_direction_input(C_VIA_STATE);

	gpio_direction_input(GPIO_MDM_RSV2);
}

static int __init ap_modem_init(void)
{
    int ret = 0;
	
	INIT_DELAYED_WORK(&mdm_wake_usb_work,release_wake_lock);

    pmdm->wake_count = 0;
    spin_lock_init(&pmdm->lock);
    wake_lock_init(&pmdm->modem_lock, WAKE_LOCK_SUSPEND, "modem");
    wake_lock_init(&pmdm->reset_lock, WAKE_LOCK_SUSPEND, "modem_rst");

    ret = platform_device_register(&ap_modem_device);
    if (ret < 0) {
        pr_err("ap_modem_init: platform_device_register failed\n");
        goto err_reg_device;
    }
    ret = platform_driver_register(&ap_modem_driver);
    if (ret < 0) {
        pr_err("ap_modem_init: platform_driver_register failed\n");
        goto err_reg_driver;
    }

	if(!via_gpio_init_finish)
	{
		via_gpio_init();
		via_gpio_init_finish = 1;
	}
 
    modem_wake_ap_irq(gpio_to_irq(GPIO_MDM_RDY), NULL);

    /*CP will wake AP by plull up the pin */
    set_irq_type(gpio_to_irq(GPIO_MDM_RDY), IRQ_TYPE_EDGE_RISING);
    ret = request_irq(gpio_to_irq(GPIO_MDM_RDY), modem_wake_ap_irq, IRQF_TRIGGER_RISING, "Modem ready", NULL);
    if (ret < 0) {
        pr_err("%s: fail to request irq for GPIO_MDM_RDY\n", __FUNCTION__);
        goto err_req_irq;
    }

    gpio_direction_output(GPIO_AP_WAKE_MDM, WAKE_LEVEL);
    gpio_direction_output(GPIO_RESET_MDM, 0);

#ifdef POWER_CBP_AT_BOOT
    wake_modem();
    printk(KERN_ERR "sguan--gpio_pwer_mdm set low\n");
    gpio_direction_output(GPIO_POWER_MDM, 0); //If GPIOS high, CBP will got key down
#endif

    dbg("AP MODEM INIT is ok.\n");
    return 0;

err_req_irq:
    gpio_free(GPIO_MDM_RDY);
err_reg_driver:
	platform_driver_unregister(&ap_modem_driver);
err_reg_device:
	platform_device_unregister(&ap_modem_device);
	return -1;
}

static void  __exit ap_modem_exit(void)
{
	gpio_free(GPIO_MDM_RDY);	
	gpio_free(GPIO_RESET_MDM);
	gpio_free(GPIO_AP_WAKE_MDM);
	gpio_free(GPIO_MDM_WAKE_AP);
	gpio_free(GPIO_POWER_MDM);
	free_irq(gpio_to_irq(GPIO_MDM_RDY), NULL);
	wake_lock_destroy(&pmdm->modem_lock);
	platform_driver_unregister(&ap_modem_driver);
	platform_device_unregister(&ap_modem_device);
}

late_initcall(ap_modem_init);
module_exit(ap_modem_exit);

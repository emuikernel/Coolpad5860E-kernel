/*
 * Copyright (C) 2009 Motorola, Inc.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free dispware
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/i2c/twl.h>
#include <linux/slab.h>
#include <plat/yl_debug.h>//define yl_debug();


#define DISP_BUTTON_DEV 	"omap-led"	/*the device name*/
#define LED_DEBUG 			0

/*twl5030 registers for led control */
#define TWL5030_LED_BASE_ADDR   	0xee
#define TWL5030_PWMA_BASE_ADDR  	0xef
#define	TWL5030_PWMB_BASE_ADDR  	0xf1
#define TWL5030_GPIO_BASE_ADDR   	0x98 

#define TWL5030_LEDEN_OFFSET     	0x0
#define TWL5030_PWMAON_OFSSET    	0x0
#define TWL5030_PWMAOFF_OFFSET   	0x01
#define TWL5030_PWMBON_OFFSET    	0x0
#define TWL5030_PWMBOFF_OFFSET   	0x01

#define TWL5030_VIBRA_CTL_OFFSET 	0x45
#define TWL5030_VIBRA_EN_SHIFT   	0x0

/* bit shift for control*/
#define TWL5030_LEDAON_SHIFT 		0
#define TWL5030_LEDBON_SHIFT 		1
#define TWL5030_LEDAPWM_SHIFT 		4
#define TWL5030_LEDBPWM_SHIFT 		5


#if LED_DEBUG
#define LED_LOG(fmt, arg...) printk("<led-omap-disp.c>" fmt, ##arg)
#else
#define LED_LOG(fmt, arg...)
#endif
#define LED_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)
#define assert(i)  BUG_ON(!(i))

/*******************************************************************/
static void disp_button_set(struct led_classdev *led_cdev,
			    enum led_brightness value);

static void omap_set_ledb_5030_work(struct work_struct *work);

struct disp_button_led_data {
	struct led_classdev disp_button_class_dev;/*led class dev*/
	struct zoom2_led_device *pdata;
	struct delayed_work	work; 
	int brightness;							  /*led value: 0:off, 255:on*/
};

struct disp_button_led_data  g_pwm_led;
struct disp_button_led_data  *g_apwm_led = &g_pwm_led;


/*************************************************************************
  Function:       omap_set_ledb_5030_work
  Description:    set the 5030 ledb
  Calls:          twl_i2c_read_u8()
  				  twl_i2c_write_u8()
  Called By:      schedule_work
  Input:          @work : work struct of setting ledb.
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static void omap_set_ledb_5030_work(struct work_struct *work)
{
	u8 rd_data,tmp;
	u8 pwmon,pwmoff;
	
	assert(work);

	LED_LOG("omap_set_ledb_5030:brightness=%d \n ",g_apwm_led->brightness);
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_LED, &rd_data, TWL5030_LEDEN_OFFSET))
	{
		LED_ERR("read i2c error for led\n");
		return;
	}

	LED_LOG("omap_set_ledb_5030 LEDEN i2c read:%02x \n ", rd_data); 
	
	if (g_apwm_led->brightness)
	{
		#if 0
		/* vibrator disable */
		if (0 != twl_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &rd_data2, TWL5030_VIBRA_CTL_OFFSET))
		{
			LED_ERR("read vibrator i2c error for led\n");
			return;
		}
		LED_LOG("5030 vibra ctl i2c read:%02x \n ", rd_data2); 
		if (rd_data2 & (1<<TWL5030_VIBRA_EN_SHIFT))
		{
			rd_data2 &= ~(1<<TWL5030_VIBRA_EN_SHIFT);
			twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, rd_data2, TWL5030_VIBRA_CTL_OFFSET);
			LED_LOG("5030 vibra ctl i2c write:%02x \n ", rd_data2); 
		}
		#endif

		/* enable LEDEN */
		tmp = rd_data;
		tmp |= ((1<<TWL5030_LEDBON_SHIFT)|(1<<TWL5030_LEDBPWM_SHIFT));
		if(tmp != rd_data){
			twl_i2c_write_u8(TWL4030_MODULE_LED, tmp, TWL5030_LEDEN_OFFSET);
        }
		LED_LOG("ledb on! i2c write:%02x \n ", rd_data); 

		/* for pwd use*/
		pwmon = 1; //set the pwm ontime the 1st clk cycle.
		pwmoff = (u8)(g_apwm_led->brightness)>>1;
		if (pwmon == pwmoff)
		{
			pwmoff++;
		}
		LED_LOG("\nset ledb ontime:%d, offtime:%d .\n", pwmon, pwmoff);
		twl_i2c_write_u8(TWL4030_MODULE_PWMB, pwmon, TWL5030_PWMBON_OFFSET);
		twl_i2c_write_u8(TWL4030_MODULE_PWMB, pwmoff, TWL5030_PWMBOFF_OFFSET);

	}
	else
	{
		/* disable LEDB in LEDEN reg.*/
		rd_data &= ~((1<<TWL5030_LEDBON_SHIFT)|(1<<TWL5030_LEDBPWM_SHIFT));
		twl_i2c_write_u8(TWL4030_MODULE_LED, rd_data, TWL5030_LEDEN_OFFSET);
		LED_LOG("ledb off!omap_set_ledb_5030 i2c write:%02x \n ", rd_data); 
	}

	return;
}


/*************************************************************************
  Function:       disp_button_set
  Description:    set the td led light
  Calls:          schedule_delayed_work
  Called By:      
  Input:          @led_cdev : led class device
                  @value	: led value, 0 is off, 255 is on 
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static void disp_button_set(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	assert(led_cdev);

	LED_LOG("omap_set_ledb value:%d\n", value);

	#if 0
	if(g_apwm_led->brightness == value)
	{
		LED_LOG("ledb is already in [%d],return!\n", value);
		return;
	}
	#endif
	g_apwm_led->brightness = value;
	//disp_button_led_data->disp_button_class_dev = led_cdev;
	
	schedule_delayed_work(&g_apwm_led->work, 0);
	
	return;
}


//EXPORT_SYMBOL(disp_button_set);
/*************************************************************************
  Function:       disp_button_probe
  Description:    Probes the driver for attachment.
  Calls:          
  Called By:      
  Input:          platform_device *pdev : platform device.
  Output:         none
  Return:         Returns 0 if successful, or -EBUSY if unable to get client attached data,
				  or -ENODEV platform_data is NULL.
  Others:         
**************************************************************************/
static int disp_button_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct disp_button_led_data *info = NULL;

	pr_info("---%s---\r\n",__FUNCTION__);

	if (pdev == NULL) {
		LED_ERR("%s: platform data required\n", __func__);
		return -ENODEV;

	}
	else
	{

		info = kzalloc(sizeof(struct disp_button_led_data), GFP_KERNEL);
		if (info == NULL) {
			ret = -ENOMEM;
			return ret;
		}
		else
		{
	
			info->pdata = pdev->dev.platform_data;
			//platform_set_drvdata(pdev, info);
			info->disp_button_class_dev.name = "button-backlight";
			info->disp_button_class_dev.brightness_set = disp_button_set;
			ret = led_classdev_register(&pdev->dev, &info->disp_button_class_dev);
			if (ret < 0) {
				LED_ERR("%s:Register button backlight class failed\n", __func__);
				goto err_reg_button_class_failed;
			}
			INIT_DELAYED_WORK(&g_apwm_led->work, (void *)omap_set_ledb_5030_work);

			pr_info("+++%s+++\r\n",__FUNCTION__);

			return ret;

		}	// if (info == NULL) ... else ...

	}	// if (pdev == NULL) ... else ...

err_reg_button_class_failed:
	if (info != NULL)
		kfree(info);

	return ret;
}


/**************************************************************************
* Function    : disp_button_remove
* Description : led driver remove handler, Complement of disp_button_probe().
*               Unregister led device. 
* 
* Calls       :
* Called By   : 
*
* Input :  
* 		platform_device *pdev: platform device structure
* 		
* Output : 
*  	 	none.
*
* Return : 
*		Returns zero if successful, or non-zero otherwise.
*
* others : 
**************************************************************************/
static int disp_button_remove(struct platform_device *pdev)
{
	struct disp_button_led_data *info = platform_get_drvdata(pdev);
	led_classdev_unregister(&info->disp_button_class_dev);
	return 0;
}


/**************************************************************************
* Function    : disp_button_suspend
* Description : Power manager function is called when system suspend.
* Calls       :
* Called By   :
*          When system suspend,system function will call this function.
* Input :  struct i2c_client *client,pm_message_t state
* Output :     
* Return : 0 
* others : 
**************************************************************************/
static int disp_button_suspend(struct platform_device *dev,pm_message_t state)
{
	unsigned char rd_data = 0x00;
	unsigned char pwmon,pwmoff;

	yl_debug("%s--\n",__FUNCTION__);
	cancel_delayed_work_sync(&g_apwm_led->work);
	
	/* for pwd use*/
	pwmon = 1; //set the pwm ontime the 1st clk cycle.
	pwmoff = 1;
	twl_i2c_write_u8(TWL4030_MODULE_PWMB, pwmon, TWL5030_PWMBON_OFFSET);
	twl_i2c_write_u8(TWL4030_MODULE_PWMB, pwmoff, TWL5030_PWMBOFF_OFFSET);

	/* disable LEDB in LEDEN reg.*/
	rd_data &= ~((1<<TWL5030_LEDBON_SHIFT)|(1<<TWL5030_LEDBPWM_SHIFT));
	twl_i2c_write_u8(TWL4030_MODULE_LED, rd_data, TWL5030_LEDEN_OFFSET);
	LED_LOG("ledb off!omap_set_ledb_5030 i2c write:%02x \n ", rd_data); 

	return 0;
}


/**************************************************************************
* Function    : disp_button_resume
* Description : Power manager function is called when system wake up.
* Calls       :
* Called By   :
*          When system wake up,system function will call this function.
* Input :  struct i2c_client *client
* Output :     
* Return : 0 
* others : 
**************************************************************************/
static int disp_button_resume(struct platform_device *dev)
{
	yl_debug("%s--\n",__FUNCTION__);
	schedule_delayed_work(&g_apwm_led->work, msecs_to_jiffies(400));
	return 0;
}


static struct platform_driver ld_disp_button_driver = {
	.probe = disp_button_probe,
	.remove = disp_button_remove,
	.suspend = disp_button_suspend,
	.resume = disp_button_resume,
	.driver = {
		   	.name = DISP_BUTTON_DEV,
			.owner		= THIS_MODULE,
		   },
};


/**************************************************************************
* Function    : led_disp_button_init
* Description : Registers platform device.
* 
* Calls       :
* Called By   : 
*
* Input :  
* 		none.
* 		
* Output : 
*  	 	none.
*
* Return : 
* 		Returns 0 on success,
* 		error code otherwise.
*
* others : 
**************************************************************************/
static int __init led_disp_button_init(void)
{
	return platform_driver_register(&ld_disp_button_driver);
}

static void __exit led_disp_button_exit(void)
{
	platform_driver_unregister(&ld_disp_button_driver);
}

module_init(led_disp_button_init);
module_exit(led_disp_button_exit);

MODULE_DESCRIPTION("Display Button Lighting driver");
MODULE_AUTHOR("COOLPAD,xiaoweima");
MODULE_LICENSE("GPL");

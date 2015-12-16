/*
 * drivers/leds/leds-omap.c
 * Copyright (C) 2009 Texas Instruments, Inc.
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
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>

#include <mach/hardware.h>
#include <mach/led.h>
#include <mach/mux.h>
#include <linux/delay.h>

#include <linux/i2c/twl4030.h>
#include <linux/workqueue.h>


/* our context */
#define COMPLEX_LED
//#define LED_PWM 1
#undef LED_PWM
#define LED_COMPLEX 1
#define LED_SIMPLE  0
#define LED_DEBUG 1

/* omap3430 gpio for leds */
#define SMS_LED_GPIO  178
#define IM_LED_GPIO  179
#define BT_LED_GPIO  180 /*180*/
#define JOG_LED_GPIO  181

/*twl5030 registers for led control */
#define TWL5030_LED_BASE_ADDR   0xee
#define TWL5030_PWMA_BASE_ADDR  0xef
#define	TWL5030_PWMB_BASE_ADDR  0xf1
#define TWL5030_GPIO_BASE_ADDR   0x98 

#define TWL5030_LEDEN_OFFSET     0x0
#define TWL5030_PWMAON_OFSSET    0x0
#define TWL5030_PWMAOFF_OFFSET   0x01
#define TWL5030_PWMBON_OFFSET    0x0
#define TWL5030_PWMBOFF_OFFSET   0x01

#define TWL5030_PWM0ON_OFFSET    0x0
#define TWL5030_PWM0OFF_OFFSET   0x01
#define TWL5030_PWM1ON_OFFSET    0x0
#define TWL5030_PWM1OFF_OFFSET   0x01

#define TWL4030_BASEADD_INTBR	0x0085
#define TWL5030_GPBR1_OFFSET	(0x91-TWL4030_BASEADD_INTBR)
#define TWL5030_PMBR1_OFFSET	(0x92-TWL4030_BASEADD_INTBR)
#define TWL5030_VIBRA_CTL_OFFSET 0x45
#define TWL5030_VIBRA_EN_SHIFT   0x0

#define TWL5030_GPIODATADIR1_OFFSET  0x03
#define TWL5030_GPIODATAOUT1_OFFSET  0x06
#define TWL5030_GPIO_CTRL_OFFSET  0x12

/* bit shift for control*/
#define TWL5030_LEDAON_SHIFT 0
#define TWL5030_LEDBON_SHIFT 1
#define TWL5030_LEDAPWM_SHIFT 4
#define TWL5030_LEDBPWM_SHIFT 5
#define TWL5030_GPIO_ON_SHIFT 2
#define TWL5030_GPIO7_SHIFT  7

/* used for PWM[0,1] control.*/
#define GPBR1_PWENABLE_SHIFT 3
#define GPBR1_PWM0_ENABLE_SHIFT 2
#define GPBR1_PWCLK_ENABLE_SHIFT 1
#define GPBR1_PWM0_CLK_ENABLE_SHIFT 0
#define TWL5030_GPIO7_VIBRASYNC_PWMASK 0x3
#define TWL5030_GPIO7_VIBRASYNC_PWSHIFT 4


/* PWM time for LED*/
#define TWL5030_PWMLEDON_TIME 64
#define TWL5030_PWMLEDOFF_TIME 64


/* Omap 3430 platform GPT-PWM control setting */
#define OMAP3430_GPT9_BASE 0x49040000
#define OMAP3430_GPT10_BASE 0x48086000
#define OMAP3430_GPT11_BASE 0x48088000

#define OMAP3430_GPT9_TCLR (OMAP3430_GPT9_BASE+0x24)
#define OMAP3430_GPT10_TCLR (OMAP3430_GPT10_BASE+0x24)
#define OMAP3430_GPT11_TCLR (OMAP3430_GPT11_BASE+0x24)

#define OMAP3430_GPT9_TLDR (OMAP3430_GPT9_BASE+0x2C)
#define OMAP3430_GPT10_TLDR (OMAP3430_GPT10_BASE+0x2C)
#define OMAP3430_GPT11_TLDR (OMAP3430_GPT11_BASE+0x2C)

#define OMAP3430_GPT9_TMAR (OMAP3430_GPT9_BASE+0x38)
#define OMAP3430_GPT10_TMAR (OMAP3430_GPT10_BASE+0x38)
#define OMAP3430_GPT11_TMAR (OMAP3430_GPT11_BASE+0x38)

#define OMAP3430_GPT9_TTGR (OMAP3430_GPT9_BASE+0x30)
#define OMAP3430_GPT10_TTGR (OMAP3430_GPT10_BASE+0x30)
#define OMAP3430_GPT11_TTGR (OMAP3430_GPT11_BASE+0x30)

#define OMAP3430_GPT9_TCCR (OMAP3430_GPT9_BASE+0x28)
#define OMAP3430_GPT10_TCCR (OMAP3430_GPT10_BASE+0x28)
#define OMAP3430_GPT11_TCCR (OMAP3430_GPT11_BASE+0x28)

#define OMAP3430_GPTi_TCLR_PT_MASK 0x1
#define OMAP3430_GPTi_TCLR_PT_SHIFT 12
#define OMAP3430_GPTi_TCLR_TRG_MASK 0x3
#define OMAP3430_GPTi_TCLR_TRG_SHIFT 10
#define OMAP3430_GPTi_TCLR_CE_MASK 0x1
#define OMAP3430_GPTi_TCLR_CE_SHIFT 6
#define OMAP3430_GPTi_TCLR_AR_MASK 0x1
#define OMAP3430_GPTi_TCLR_AR_SHIFT 1
#define OMAP3430_GPTi_TCLR_ST_MASK 0x1
#define OMAP3430_GPTi_TCLR_ST_SHIFT 0

#define OMAP3430_GPTi_TCLR_PWM_MASK \
		((OMAP3430_GPTi_TCLR_PT_MASK<<OMAP3430_GPTi_TCLR_PT_SHIFT)|\
		(OMAP3430_GPTi_TCLR_TRG_MASK<<OMAP3430_GPTi_TCLR_TRG_SHIFT)|\
		(OMAP3430_GPTi_TCLR_CE_MASK<<OMAP3430_GPTi_TCLR_CE_SHIFT)|\
		(OMAP3430_GPTi_TCLR_AR_MASK<<OMAP3430_GPTi_TCLR_AR_SHIFT)|\
		(OMAP3430_GPTi_TCLR_ST_MASK<<OMAP3430_GPTi_TCLR_ST_SHIFT))

#define OMAP3430_GPTi_TCLR_PWM_STRART \
		((0x1<<OMAP3430_GPTi_TCLR_PT_SHIFT)|\
		(0x2<<OMAP3430_GPTi_TCLR_TRG_SHIFT)|\
		(0x1<<OMAP3430_GPTi_TCLR_CE_SHIFT)|\
		(0x1<<OMAP3430_GPTi_TCLR_AR_SHIFT))
//		(0x1<<OMAP3430_GPTi_TCLR_ST_SHIFT))





#if LED_DEBUG
#define LED_LOG(fmt, arg...) printk("<led-omap.c>" fmt, ##arg)
#define LED_DOUBLE_CHECK 1
#else
#define LED_LOG(fmt, arg...)
#define LED_DOUBLE_CHECK 0
#endif
#define LED_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)
#define assert(i)  BUG_ON(!(i))

/*******************************************************************/
static void omap_set_leda(struct led_classdev *led_cdev,
			    enum led_brightness value);

static void omap_set_ledb(struct led_classdev *led_cdev,
			    enum led_brightness value);

static void omap_set_led_pwr(struct led_classdev *led_cdev,
			    enum led_brightness value);

static void omap_set_led_gpio(struct led_classdev *led_cdev,
			    enum led_brightness value);

static void omap_set_leda_5030_work(struct work_struct *work);
static void omap_set_ledb_5030_work(struct work_struct *work);
static void omap_set_led_gpio_5030_work(struct work_struct *work);
static void omap_gpio_pwm_gpt11(struct led_classdev *led_cdev,
			    enum led_brightness value);
static void omap_gpio_pwm_gpt9(struct led_classdev *led_cdev,
					enum led_brightness value);
static void omap_gpio_pwm_gpt10(struct led_classdev *led_cdev,
					enum led_brightness value);


enum led_index {
    TWL5030_LEDA=0,
    TWL5030_LEDB=0,		
    SMS_LED,
    IM_LED,
    BT_LED,
    JOG_LED,
    TD_LED,
    WIFI_LED,
    PWR_LED,

    MAX_LEDs,
};

/* if led control is much complex than cpu's gpio, do it as work */
#ifdef COMPLEX_LED

/*led_work:	control data params for work func use. **/
struct led_control_work {
	struct work_struct	work;     	/*work struct*/
	struct led_classdev *led_cdev; 	/*led class dev*/
	int brightness;					/*led value: 0:off, 255:on*/
};

#endif

/*
yulong led driver function table.
complex:	if led control complex, not just GPIO setting.
work_func:	asynchronically work func for complex led control. 
led_work:	control data params for work func use. **/
struct omap_led_handle {
	void (*led_set)(struct led_classdev *led_cdev,
                    enum led_brightness brightness);	
	//const char		*name;
	int complex;
	work_func_t work_func;
	struct led_control_work * led_work;
};

#define COMPLEX_LED_SIZE 3
/*static data declared allocation for yulong led control parameter */
static struct led_control_work led_work_pool[COMPLEX_LED_SIZE];

/*static define the yulong led driver handle function table */
static struct omap_led_handle led_handle[] = {
    [TWL5030_LEDB  ]   = {
    	.led_set = omap_set_ledb,
		.complex = LED_COMPLEX,
		.work_func = omap_set_ledb_5030_work,
		.led_work = &led_work_pool[0],
	},
#if 0	
    [TWL5030_LEDA]   = {
    	.led_set = omap_set_leda,
		.complex = LED_COMPLEX,
		.work_func = omap_set_leda_5030_work,
		.led_work = &led_work_pool[1],
	},

    [SMS_LED ]   = {
		.led_set = omap_set_led_gpio,
		.complex = LED_SIMPLE,/*to be replaced by macor*/
		.work_func = NULL,
		.led_work = NULL,
	},
		
    [IM_LED  ]   = {
    	.led_set = omap_gpio_pwm_gpt9,
		.complex = LED_COMPLEX,
		.work_func = NULL,
		.led_work = NULL,
	},
	
    [BT_LED  ]   = {
    	.led_set = omap_gpio_pwm_gpt10,
		.complex = LED_COMPLEX,
		.work_func = NULL,
		.led_work = NULL,
	},

	[JOG_LED] = {
		.led_set = omap_gpio_pwm_gpt11,
		.complex = LED_COMPLEX,
		.work_func = NULL,
		.led_work = NULL,
	},
	
    [PWR_LED ]   = {
    	.led_set = omap_set_led_pwr,
		.complex = LED_COMPLEX,
		.work_func = omap_set_led_gpio_5030_work,
		.led_work = &led_work_pool[2],
	},
#endif	
};


/*yulong.liucunqing. add leds */
static struct omap_led_config leds_config[] = {
	[TWL5030_LEDB] = {
		.cdev	= {
			.name	= "led-keypad",
		},
		.gpio	= -1,
	},
#if 0	
	[SMS_LED] = {
		.cdev	= {
			.name	= "led-sms",
		},
		.gpio	= -1,
	},
	[IM_LED] = {
		.cdev	= {
			.name	= "led-im",
		},
		.gpio	= -1,
	},
	[BT_LED] = {
		.cdev	= {
			.name	= "led-bt",
		},
		.gpio = -1,
	},
	[JOG_LED] = {
		.cdev	= {
			.name	= "led-JOG",
		},
		.gpio = -1,
	},
	[TD_LED] = {
		.cdev	= {
			.name	= "led-td",
		},
		.gpio = -1,
	},
	[WIFI_LED] = {
		.cdev	= {
			.name	= "led-wifi",
		},
		.gpio = -1,
	},
	[PWR_LED] = {
		.cdev	= {
			.name	= "led-pwr",
		},
		.gpio = -1,
	},
#endif
};

static struct omap_led_platform_data leds_data = {
	.nr_leds	= ARRAY_SIZE(leds_config),
	.leds		= leds_config,
};


/*************************************************************************
  Function:       omap_set_leda
  Description:    set the wifi led light
  Calls:          schedule_work
  Called By:      
  Input:          @led_cdev : led class device
                  @value	: led value, 0 is off, 255 is on 
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static void omap_set_leda(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	struct led_control_work *led_work = led_handle[TWL5030_LEDA].led_work;
	assert(led_cdev);

	LED_LOG("omap_set_leda value:%d\n", value);
	
	if(led_work->brightness == value)
	{
		LED_LOG("leda is already in [%d],return!\n", value);
		return;
	}
	led_work->brightness = value;
	led_work->led_cdev = led_cdev;
	schedule_work(&led_work->work);

	return;
}


/*************************************************************************
  Function:       omap_set_ledb
  Description:    set the td led light
  Calls:          schedule_work
  Called By:      
  Input:          @led_cdev : led class device
                  @value	: led value, 0 is off, 255 is on 
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static void omap_set_ledb(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	struct led_control_work *led_work = led_handle[TWL5030_LEDB].led_work;
	assert(led_cdev);

	LED_LOG("omap_set_ledb value:%d\n", value);

	if(led_work->brightness == value)
	{
		LED_LOG("ledb is already in [%d],return!\n", value);
		return;
	}
	led_work->brightness = value;
	led_work->led_cdev = led_cdev;
	
	schedule_work(&led_work->work);

	
	return;
}


/*************************************************************************
  Function:       omap_set_led_pwr
  Description:    set the power led light
  Calls:          schedule_work
  Called By:      
  Input:          @led_cdev : led class device
                  @value	: led value, 0 is off, 255 is on 
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static void omap_set_led_pwr(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	struct led_control_work *led_work = led_handle[PWR_LED].led_work;
	assert(led_cdev);

	LED_LOG("omap_set_led_pwr value:%d\n", value);
	if(led_work->brightness == value)
	{
		LED_LOG("ledpwr is already in [%d],return!\n", value);
		return;
	}
	led_work->brightness = value;
	led_work->led_cdev = led_cdev;
	schedule_work(&led_work->work);
	return;
}


/*************************************************************************
  Function:       omap_set_leda_5030_work
  Description:    set the 5030 leda
  Calls:          twl4030_i2c_read_u8()
  				  twl4030_i2c_write_u8()
  Called By:      schedule_work
  Input:          @work : work struct of setting leda.
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static void omap_set_leda_5030_work(struct work_struct *work)
{
	struct led_control_work *ledwork;
	u8 rd_data, rd_data2, tmp;
	u8 pwmon,pwmoff;
	assert(work);

	ledwork = container_of(work, struct led_control_work, work);
	LED_LOG("omap_set_leda_5030:ledwork->brightness=%d \n ",ledwork->brightness);
	if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_LED, &rd_data, TWL5030_LEDEN_OFFSET))
	{
		LED_ERR("omap_set_leda_5030 error!\n");
		return;
	}
	LED_LOG("omap_set_ledb_5030 LEDEN i2c read:%02x \n ", rd_data); 
	
    if (ledwork->brightness)
    {

		/* vibrator disable */
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &rd_data2, TWL5030_VIBRA_CTL_OFFSET))
		{
			LED_ERR("read vibrator i2c error for led\n");
			return;
		}
		LED_LOG("5030 vibra ctl i2c read:%02x \n ", rd_data2); 
		if (rd_data2 & (1<<TWL5030_VIBRA_EN_SHIFT))
		{
			rd_data2 &= ~(1<<TWL5030_VIBRA_EN_SHIFT);
			twl4030_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, rd_data2, TWL5030_VIBRA_CTL_OFFSET);
			LED_LOG("5030 vibra ctl i2c write:%02x \n ", rd_data2); 
		}
		
		/* double check*/ 
		/*
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &rd_data2, TWL5030_VIBRA_CTL_OFFSET))
		{
			LED_LOG("read vibrator i2c error for led\n");
			return;
		}
		LED_LOG("5030 vibra ctl i2c read again:%02x \n ", rd_data2); 
		*/
	
    		tmp = rd_data;
		tmp |= ((1<<TWL5030_LEDAON_SHIFT)|(1<<TWL5030_LEDAPWM_SHIFT));
		if(tmp != rd_data)
       		twl4030_i2c_write_u8(TWL4030_MODULE_LED, tmp, TWL5030_LEDEN_OFFSET);

#ifdef LED_PWM /* for pwd use*/
		pwmon = 1; //set the pwm ontime the 1st clk cycle.
		pwmoff = (u8)(ledwork->brightness)>>1;
		if (pwmon == pwmoff)
		{
			pwmoff++;
		}
		LED_LOG("\nset leda ontime:%d, offtime:%d .\n", pwmon, pwmoff);
		twl4030_i2c_write_u8(TWL4030_MODULE_PWMA, pwmon, TWL5030_PWMAON_OFSSET);
		twl4030_i2c_write_u8(TWL4030_MODULE_PWMA, pwmoff, TWL5030_PWMAOFF_OFFSET);
#else
		/* set the led always on */
		twl4030_i2c_write_u8(TWL4030_MODULE_PWMA, TWL5030_PWMLEDON_TIME, TWL5030_PWMAON_OFSSET);
		twl4030_i2c_write_u8(TWL4030_MODULE_PWMA, TWL5030_PWMLEDOFF_TIME, TWL5030_PWMAOFF_OFFSET);
#endif
	}
	else
	{
	    /* disable LEDA in LEDEN reg.*/
    	rd_data &= ~((1<<TWL5030_LEDAON_SHIFT)|(1<<TWL5030_LEDAPWM_SHIFT));
        twl4030_i2c_write_u8(TWL4030_MODULE_LED, rd_data, TWL5030_LEDEN_OFFSET);
	}



	return;
}



/*************************************************************************
  Function:       omap_set_ledb_5030_work
  Description:    set the 5030 ledb
  Calls:          twl4030_i2c_read_u8()
  				  twl4030_i2c_write_u8()
  Called By:      schedule_work
  Input:          @work : work struct of setting ledb.
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static void omap_set_ledb_5030_work(struct work_struct *work)
{
	struct led_control_work *ledwork;
	u8 rd_data, rd_data2,tmp;
	u8 pwmon,pwmoff;

	assert(work);

	ledwork = container_of(work, struct led_control_work, work);

	LED_LOG("omap_set_ledb_5030:brightness=%d \n ",ledwork->brightness);
	if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_LED, &rd_data, TWL5030_LEDEN_OFFSET))
	{
		LED_ERR("read i2c error for led\n");
		return;
	}

	LED_LOG("omap_set_ledb_5030 LEDEN i2c read:%02x \n ", rd_data); 
	
	if (ledwork->brightness)
	{
		/* vibrator disable */
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &rd_data2, TWL5030_VIBRA_CTL_OFFSET))
		{
			LED_ERR("read vibrator i2c error for led\n");
			return;
		}
		LED_LOG("5030 vibra ctl i2c read:%02x \n ", rd_data2); 
		if (rd_data2 & (1<<TWL5030_VIBRA_EN_SHIFT))
		{
			rd_data2 &= ~(1<<TWL5030_VIBRA_EN_SHIFT);
			twl4030_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, rd_data2, TWL5030_VIBRA_CTL_OFFSET);
			LED_LOG("5030 vibra ctl i2c write:%02x \n ", rd_data2); 
		}


		/* double check*/ 
		/*
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &rd_data2, TWL5030_VIBRA_CTL_OFFSET))
		{
			LED_LOG("read vibrator i2c error for led\n");
			return;
		}
		LED_LOG("5030 vibra ctl i2c read again:%02x \n ", rd_data2); 
		*/


		/* enable LEDEN */
		tmp = rd_data;
		tmp |= ((1<<TWL5030_LEDBON_SHIFT)|(1<<TWL5030_LEDBPWM_SHIFT));
		if(tmp != rd_data)
			twl4030_i2c_write_u8(TWL4030_MODULE_LED, tmp, TWL5030_LEDEN_OFFSET);
		LED_LOG("ledb on! i2c write:%02x \n ", rd_data); 

#ifdef LED_PWM /* for pwd use*/
		pwmon = 1; //set the pwm ontime the 1st clk cycle.
		pwmoff = (u8)(ledwork->brightness)>>1;
		if (pwmon == pwmoff)
		{
			pwmoff++;
		}
		LED_LOG("\nset ledb ontime:%d, offtime:%d .\n", pwmon, pwmoff);
		twl4030_i2c_write_u8(TWL4030_MODULE_PWMB, pwmon, TWL5030_PWMBON_OFFSET);
		twl4030_i2c_write_u8(TWL4030_MODULE_PWMB, pwmoff, TWL5030_PWMBOFF_OFFSET);
#else
		/* set the led always on */
		twl4030_i2c_write_u8(TWL4030_MODULE_PWMB, TWL5030_PWMLEDON_TIME, TWL5030_PWMBON_OFFSET);
		twl4030_i2c_write_u8(TWL4030_MODULE_PWMB, TWL5030_PWMLEDOFF_TIME, TWL5030_PWMBOFF_OFFSET);
#endif
	}
	else
	{
		/* disable LEDA in LEDEN reg.*/
		rd_data &= ~((1<<TWL5030_LEDBON_SHIFT)|(1<<TWL5030_LEDBPWM_SHIFT));
		twl4030_i2c_write_u8(TWL4030_MODULE_LED, rd_data, TWL5030_LEDEN_OFFSET);
		LED_LOG("ledb off!omap_set_ledb_5030 i2c write:%02x \n ", rd_data); 
	}



	return;

}


/*************************************************************************
  Function:       omap_set_led_gpio_5030_work
  Description:    set the 5030 gpio
  Calls:          twl4030_i2c_read_u8()
  				  twl4030_i2c_write_u8()
  Called By:      schedule_work
  Input:          @work : work struct of setting 5030 gpio.
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
#ifndef LED_PWM
/* control GPIO.7, gpio control is power-saving. */
static void omap_set_led_gpio_5030_work(struct work_struct *work)
{
	struct led_control_work *ledwork;
	u8 rd_data = 0;

	assert(work);

	ledwork = container_of(work, struct led_control_work, work);

	/*enable the GPIO module */
	if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, TWL5030_GPIO_CTRL_OFFSET))
	{   /*error*/
		LED_ERR("read i2c error for led\n");
		return;
	}
	LED_LOG("omap_set_led_gpio_5030 GPIO_CTRL:%02x \n ", rd_data); 
	rd_data |= 1<<TWL5030_GPIO_ON_SHIFT;
	twl4030_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, TWL5030_GPIO_CTRL_OFFSET);

#if LED_DOUBLE_CHECK
	if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, TWL5030_GPIO_CTRL_OFFSET))
	{   /*error*/
		LED_ERR("read i2c error for led\n");
		return;
	}
	LED_LOG("omap_set_led_gpio_5030 GPIO_CTRL:%02x \n ", rd_data); 
#endif

	
    /* set the gpio direction --output */
	if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, TWL5030_GPIODATADIR1_OFFSET))
	{   /*error*/
		LED_ERR("read i2c error for led\n");
		return;
	}
	LED_LOG("omap_set_led_gpio_5030 GPIODATADIR1:%02x \n ", rd_data); 
	rd_data |= 1<<TWL5030_GPIO7_SHIFT;
	twl4030_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, TWL5030_GPIODATADIR1_OFFSET);

#if LED_DOUBLE_CHECK
	if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, TWL5030_GPIODATADIR1_OFFSET))
	{   /*error*/
		LED_ERR("read i2c error for led\n");
		return;
	}
	LED_LOG("omap_set_led_gpio_5030 GPIODATADIR1:%02x \n ", rd_data); 

#endif


	/* set the output value */
	if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, TWL5030_GPIODATAOUT1_OFFSET))
	{   /*error*/
		LED_ERR("read i2c error for led\n");
		return;
	}

	LED_LOG("omap_set_led_gpio_5030 outvalue i2c read:%02x \n ", rd_data); 

    if (ledwork->brightness)
    {
    	rd_data |= 1<<TWL5030_GPIO7_SHIFT;
		LED_LOG("omap_set_led_gpio_5030 i2c write:%02x \n ", rd_data); 
        twl4030_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, TWL5030_GPIODATAOUT1_OFFSET);
		LED_LOG("5030 gpio.7: on\n ");
	}
	else
	{
    	rd_data &= ~(1<<TWL5030_GPIO7_SHIFT);
		LED_LOG("omap_set_led_gpio_5030 i2c write:%02x \n ", rd_data); 
        twl4030_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, TWL5030_GPIODATAOUT1_OFFSET);
		LED_LOG("5030 gpio.7: off\n ");
	}

#if LED_DOUBLE_CHECK
	if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, TWL5030_GPIODATAOUT1_OFFSET))
	{	/*error*/
		LED_ERR("read i2c error for led\n");
		return;
	}

	LED_LOG("omap_set_led_gpio_5030 outvalue i2c read:%02x \n ", rd_data); 
#endif

	return;
}

#else

static void omap_set_led_gpio_5030_work(struct work_struct *work)
{
	struct led_control_work *ledwork;
	u8 rd_data, rd_data2;
	u8 pwmon,pwmoff;
	assert(work);

	ledwork = container_of(work, struct led_control_work, work);

	if (ledwork->brightness)
	{

		/* enable the INTBR setting for PWM1 control*/
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_INTBR, &rd_data, TWL5030_GPBR1_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		LED_LOG("\n%s GPBR1:%02x\n", __func__, rd_data);

		rd_data2 = rd_data;
		rd_data2 |= ((1<<GPBR1_PWENABLE_SHIFT)|(1<<GPBR1_PWCLK_ENABLE_SHIFT));
		if (rd_data2 != rd_data)
			twl4030_i2c_write_u8(TWL4030_MODULE_INTBR, rd_data2, TWL5030_GPBR1_OFFSET);

#if LED_DOUBLE_CHECK
		//double check. temp debug.
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_INTBR, &rd_data, TWL5030_GPBR1_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		LED_LOG("\n%s GPBR1:%02x double checked.\n", __func__, rd_data);
#endif



		/* set the PWM1 output to GPIO.7 */
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_INTBR, &rd_data, TWL5030_PMBR1_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		
		LED_LOG("\n%s PMBR1:%02x  \n", __func__, rd_data);
		rd_data2 = rd_data;
		rd_data2 |= (TWL5030_GPIO7_VIBRASYNC_PWMASK<<TWL5030_GPIO7_VIBRASYNC_PWSHIFT);
		if (rd_data2 != rd_data)
			twl4030_i2c_write_u8(TWL4030_MODULE_INTBR, rd_data2, TWL5030_PMBR1_OFFSET);

#if LED_DOUBLE_CHECK
		//double checked. temp debug.
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_INTBR, &rd_data, TWL5030_PMBR1_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		LED_LOG("\n%s PMBR1:%02x double checked. \n", __func__, rd_data);


		//double checked. temp debug.
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_PWM1, &rd_data, TWL5030_PWM1ON_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		LED_LOG("\n%s PWM1ON:%02x. \n", __func__, rd_data);
		
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_PWM1, &rd_data, TWL5030_PWM1OFF_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		LED_LOG("\n%s PWM1OFF:%02x. \n", __func__, rd_data);

#endif


		/* set the PWM1 ontime/offtime cycle */
		pwmon = 1; //the pwm ontime set to 1 in 128 clk cycle totally.
		pwmoff = (u8)(ledwork->brightness)>>1;
		if (pwmon >= pwmoff) //pwmon is not allowed to greater than pwmoff
		{
			pwmoff = pwmon+1;
		}
		LED_LOG("\nset ledgpio ontime:%d, offtime:%d .\n", pwmon, pwmoff);
		twl4030_i2c_write_u8(TWL4030_MODULE_PWM1, pwmon, TWL5030_PWM1ON_OFFSET);
		twl4030_i2c_write_u8(TWL4030_MODULE_PWM1, pwmoff, TWL5030_PWM1OFF_OFFSET);


#if LED_DOUBLE_CHECK
	//double checked. temp debug.
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_PWM1, &rd_data, TWL5030_PWM1ON_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		LED_LOG("\n%s PWM1ON:%02x double checked. \n", __func__, rd_data);
		
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_PWM1, &rd_data, TWL5030_PWM1OFF_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		LED_LOG("\n%s PWM1OFF:%02x double checked. \n", __func__, rd_data);
#endif

		
	}
	else /*off the led */
	{
		
		/* disable the INTBR setting for PWM1 control*/
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_INTBR, &rd_data, TWL5030_GPBR1_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		rd_data &= ~((1<<GPBR1_PWENABLE_SHIFT)|(1<<GPBR1_PWCLK_ENABLE_SHIFT));
		twl4030_i2c_write_u8(TWL4030_MODULE_INTBR, rd_data, TWL5030_GPBR1_OFFSET);

		
#if LED_DOUBLE_CHECK
		//double check. temp debug.
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_INTBR, &rd_data, TWL5030_GPBR1_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		LED_LOG("\n%s GPBR1:%02x double checked.\n", __func__, rd_data);
#endif

		/* set the PWM1 output to GPIO.7 */
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_INTBR, &rd_data, TWL5030_PMBR1_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		
		LED_LOG("\n%s PMBR1:%02x  \n", __func__, rd_data);
		rd_data &= ~(TWL5030_GPIO7_VIBRASYNC_PWMASK<<TWL5030_GPIO7_VIBRASYNC_PWSHIFT);
		twl4030_i2c_write_u8(TWL4030_MODULE_INTBR, rd_data, TWL5030_PMBR1_OFFSET);

#if LED_DOUBLE_CHECK
		//double checked. temp debug.
		if (0 != twl4030_i2c_read_u8(TWL4030_MODULE_INTBR, &rd_data, TWL5030_PMBR1_OFFSET))
		{	/*error*/
			LED_ERR("read i2c error for led\n");
			return;
		}
		LED_LOG("\n%s PMBR1:%02x double checked. \n", __func__, rd_data);
#endif

	}


	return;
}

#endif

/*************************************************************************
  Function:       omap_set_led_gpio
  Description:    set the MCU 3430 gpio
  Calls:          gpio_set_value()
  Called By:      schedule_work
  Input:          @led_cdev : led class device
                  @value	: led value, 0 is off, 255 is on 
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static void omap_set_led_gpio(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	struct omap_led_config *led_dev;
	int val = 0;
	
	assert(led_cdev);
	
	led_dev = container_of(led_cdev, struct omap_led_config, cdev);

	val = (value)?1:0; 
	
	//gpio_set_value(led_dev->gpio, val);
	gpio_direction_output(led_dev->gpio, val);
	LED_LOG("omap_set_led_gpio:%d to:%d\n", led_dev->gpio, val);

	return;
}

#define GPT9_FUNC_CLK_REG 0x48005000
#define GPT9_INTE_CLK_REG 0x48005010
#define GPT10_FUNC_CLK_REG 0x48004A00
#define GPT10_INTE_CLK_REG 0x48004A10
#define GPT11_FUNC_CLK_REG GPT10_FUNC_CLK_REG
#define GPT11_INTE_CLK_REG GPT10_INTE_CLK_REG
#define GPT9_FUNC_CLK_BIT 10
#define GPT10_FUNC_CLK_BIT 11
#define GPT11_FUNC_CLK_BIT 12



int omap_pwm_gpt_clk_enable(int enable, int gpti)
{
	unsigned int en;
	
	en = enable?1:0;

	/* GPtimer 9 */
	if (9 == gpti)
	{
		
		//func clk
		//printk(KERN_NOTICE "\n:gpt9 func clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48005000));

		*(volatile unsigned int *)IO_ADDRESS(GPT9_FUNC_CLK_REG) =
			(*(volatile unsigned int *)IO_ADDRESS(GPT9_FUNC_CLK_REG) &
			(~(1<<GPT9_FUNC_CLK_BIT)))|(en<<GPT9_FUNC_CLK_BIT);
		
		//printk(KERN_NOTICE "\n:gpt9 func clk check:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48005000));
		
		//interface clk
		//printk(KERN_NOTICE "\n:gpt9 inter clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48005010));

		*(volatile unsigned int *)IO_ADDRESS(GPT9_INTE_CLK_REG) =
			(*(volatile unsigned int *)IO_ADDRESS(GPT9_INTE_CLK_REG) & 
			(~(1<<GPT9_FUNC_CLK_BIT)))|(en<<GPT9_FUNC_CLK_BIT);
		
		//printk(KERN_NOTICE "\n:gpt9 inter clk check:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48005010));
		printk(KERN_NOTICE " ");
	}
	/* GPtimer 10 */
	else if (10 == gpti)
	{
		/*func clk*/
		//printk(KERN_NOTICE "\n:gpt10 func clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A00));
		*(volatile unsigned int *)IO_ADDRESS(GPT10_FUNC_CLK_REG) = 
			((*(volatile unsigned int *)IO_ADDRESS(GPT10_FUNC_CLK_REG))& 
			(~(1<<GPT10_FUNC_CLK_BIT)))|(en<<GPT10_FUNC_CLK_BIT);

		//printk(KERN_NOTICE "\n:gpt10 func clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A00));
		/*interface clk*/
		//printk(KERN_NOTICE "\n:gpt10 inter clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A10));

		*(volatile unsigned int *)IO_ADDRESS(GPT10_INTE_CLK_REG) = 
			((*(volatile unsigned int *)IO_ADDRESS(GPT10_INTE_CLK_REG))& 
			(~(1<<GPT10_FUNC_CLK_BIT)))|(en<<GPT10_FUNC_CLK_BIT);
		
		//printk(KERN_NOTICE "\n:gpt10 inter clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(GPT10_INTE_CLK_REG));
		printk(KERN_NOTICE " ");

	}
	/* GPtimer 11 */
	else if (11 == gpti)
	{
		/*func clk*/
		*(volatile unsigned int *)IO_ADDRESS(GPT11_FUNC_CLK_REG) = 
			((*(volatile unsigned int *)IO_ADDRESS(GPT11_FUNC_CLK_REG))&
			(~(1<<GPT11_FUNC_CLK_BIT)))|(en<<GPT11_FUNC_CLK_BIT);
		/*interface clk*/
		*(volatile unsigned int *)IO_ADDRESS(GPT11_INTE_CLK_REG) = 
			((*(volatile unsigned int *)IO_ADDRESS(GPT11_INTE_CLK_REG))& 
			(~(1<<GPT11_FUNC_CLK_BIT)))|(en<<GPT11_FUNC_CLK_BIT);
		//printk(KERN_NOTICE "\n:gpt11 inter clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(GPT11_INTE_CLK_REG));
		printk(KERN_NOTICE " ");

	}


	
	
	return 0;
	
}



static void omap_gpio_pwm_gpt11(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
//	struct omap_led_config *led_dev;
	u8 pwmon;
	int ret;
//	assert(led_cdev);
	
//	led_dev = container_of(led_cdev, struct omap_led_config, cdev);
	LED_LOG("\nled brightness:%d\n", value);


	if (value)
	{
	
		pwmon = (u8)value;

		if (pwmon < 3)
			pwmon = 3;
		if (pwmon > 253)
			pwmon = 253;

#if 0
		/*func clk*/
		printk(KERN_NOTICE "\n:gpt11 func clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A00));
		*(volatile unsigned int *)IO_ADDRESS(0x48004A00) = (*(volatile unsigned int *)IO_ADDRESS(0x48004A00)) | (0x1<<12);
		printk(KERN_NOTICE "\n:gpt11 func clk check:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A00));

		/*interface clk*/
		printk(KERN_NOTICE "\n:gpt11 inter clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A10));
		*(volatile unsigned int *)IO_ADDRESS(0x48004A10) = (*(volatile unsigned int *)IO_ADDRESS(0x48004A10)) | (0x1<<12);
		printk(KERN_NOTICE "\n:gpt11 inter clk check:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A10));

#endif
		ret = omap_pwm_gpt_clk_enable(1, 11);
		if (ret < 0)
		{
			LED_ERR("omap_gpio_pwm_gpt11, omap_pwm_gpt_clk_enable failed!\n ");
			return;
		}
		//set the counters 
		
		//printk(KERN_NOTICE "\n:OMAP3430_GPT11_TLDR :%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TLDR));
		*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TLDR) = (unsigned int)(0xffffffff - 256);
		//printk(KERN_NOTICE "\n:OMAP3430_GPT11_TLDR check:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TLDR));

		//printk(KERN_NOTICE "\n:OMAP3430_GPT11_TMAR :%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TMAR));
		*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TMAR) = (unsigned int)(0xffffffff - 256 + pwmon);
	//	printk(KERN_NOTICE "\n:OMAP3430_GPT11_TMAR check:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TMAR));

		//printk(KERN_NOTICE "\n:OMAP3430_GPT11_TCCR :%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TCCR));
		*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TCCR) = (unsigned int)(0xffffffff - 256);
		//printk(KERN_NOTICE "\n:OMAP3430_GPT11_TCCR check:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TCCR));
		//set control reg.

		//printk(KERN_NOTICE "\n:OMAP3430_GPT11_TCLR :%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TCLR));
		//*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TCLR) = (OMAP3430_GPTi_TCLR_PWM_STRART);
		*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TCLR) = ((OMAP3430_GPTi_TCLR_PWM_STRART) |(0x1<<OMAP3430_GPTi_TCLR_ST_SHIFT));

		//printk(KERN_NOTICE "\n:OMAP3430_GPT11_TCLR check:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TCLR));

		
	}
	else
	{

		//disable clk

		
		*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TCLR) = 0;
		//*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT11_TCLR) = 0;
		
		//omap_pwm_gpt_clk_enable(0, 11);

	}
	
	return;
}
static void omap_gpio_pwm_gpt9(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	//	struct omap_led_config *led_dev;
	u8 pwmon;
	int ret;
	//	assert(led_cdev);
		
	//	led_dev = container_of(led_cdev, struct omap_led_config, cdev);
	
		LED_LOG("\nled brightness:%d\n", value);
		if (value)
		{
		
			pwmon = (u8)value;

			if (pwmon < 2)
				pwmon = 2;
			if (pwmon > 253)
				pwmon = 253;

#if 0			
			//func clk
			printk(KERN_NOTICE "\n:gpt9 func clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48005000));
			*(volatile unsigned int *)IO_ADDRESS(0x48005000) = *(volatile unsigned int *)IO_ADDRESS(0x48005000) | 0x0400;
			printk(KERN_NOTICE "\n:gpt9 func clk check:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48005000));
			
			//interface clk
			printk(KERN_NOTICE "\n:gpt9 inter clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48005010));
			*(volatile unsigned int *)IO_ADDRESS(0x48005010) = *(volatile unsigned int *)IO_ADDRESS(0x48005010) | 0x0400;
			printk(KERN_NOTICE "\n:gpt9 inter clk check:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48005010));
#endif
			ret = omap_pwm_gpt_clk_enable(1, 9);
			if (ret < 0)
			{
				LED_ERR("omap_gpio_pwm_gpt9, omap_pwm_gpt_clk_enable failed!\n ");
				return;
			}

			

			//set the counters 
			//printk(KERN_NOTICE "\n:OMAP3430_GPT9_TLDR :%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TLDR));
			*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TLDR) = (unsigned int)(0xffffffff - 256);
			//printk(KERN_NOTICE "\n:OMAP3430_GPT9_TLDR check:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TLDR));


			//printk(KERN_NOTICE "\n:OMAP3430_GPT9_TMAR :%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TMAR));
			*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TMAR) = (unsigned int)(0xffffffff - 256 + pwmon);
			//printk(KERN_NOTICE "\n:OMAP3430_GPT9_TMAR check:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TMAR));

				
			*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TCCR) = (unsigned int)(0xffffffff - 256);

				
				//set control reg.
				//printk(KERN_NOTICE "\n:OMAP3430_GPT9_TCLR:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TCLR));
				*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TCLR) = ((OMAP3430_GPTi_TCLR_PWM_STRART) |(0x1<<OMAP3430_GPTi_TCLR_ST_SHIFT));
				//printk(KERN_NOTICE "\n:OMAP3430_GPT9_TCLR check:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TCLR));


	


		}
		else
		{
		
		//printk(KERN_NOTICE "\n:OMAP3430_GPT9_TCLR:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TCLR));
		//	*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TCLR) &= ~(0x1<<OMAP3430_GPTi_TCLR_ST_SHIFT);
		//printk(KERN_NOTICE "\n:OMAP3430_GPT9_TCLR check:%x\n", *(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TCLR));
		*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT9_TCLR) = 0;

		//omap_pwm_gpt_clk_enable(0, 9);

		}
	

	return;
}
static void omap_gpio_pwm_gpt10(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	//	struct omap_led_config *led_dev;
	u8 pwmon;
	int ret;
	//	assert(led_cdev);
		
	//	led_dev = container_of(led_cdev, struct omap_led_config, cdev);
		LED_LOG("\nled brightness:%d\n", value);

	
		if (value)
		{
		
			pwmon = (u8)value;
			if (pwmon < 3)
				pwmon = 3;
			if (pwmon > 253)
				pwmon = 253;

#if 0
			
			/*func clk*/
			//printk(KERN_NOTICE "\n:gpt10 func clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A00));
			*(volatile unsigned int *)IO_ADDRESS(0x48004A00) = (*(volatile unsigned int *)IO_ADDRESS(0x48004A00)) | (0x1<<11);
			//printk(KERN_NOTICE "\n:gpt10 func clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A00));
			/*interface clk*/
			//printk(KERN_NOTICE "\n:gpt10 inter clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A10));
			*(volatile unsigned int *)IO_ADDRESS(0x48004A10) = (*(volatile unsigned int *)IO_ADDRESS(0x48004A10)) | (0x1<<11);
			printk(KERN_NOTICE "\n:gpt10 inter clk:%x\n", *(volatile unsigned int *)IO_ADDRESS(0x48004A10));

#endif
			ret = omap_pwm_gpt_clk_enable(1, 10);
			if (ret < 0)
			{
				LED_ERR("omap_gpio_pwm_gpt10, omap_pwm_gpt_clk_enable failed!\n ");
				return;
			}

			//set counters.
			*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT10_TLDR) = (unsigned int)(0xffffffff - 256);
			*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT10_TMAR) = (unsigned int)(0xffffffff - 256 + pwmon);
			*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT10_TCCR) = (unsigned int)(0xffffffff - 256);

			//set control reg.
			*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT10_TCLR) = (unsigned int)((OMAP3430_GPTi_TCLR_PWM_STRART) |(0x1<<OMAP3430_GPTi_TCLR_ST_SHIFT));
			//*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT10_TCLR) |= (unsigned int)(0x1<<OMAP3430_GPTi_TCLR_ST_SHIFT);


		}
		else
		{
			//*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT10_TCLR) &= ~(0x1<<OMAP3430_GPTi_TCLR_ST_SHIFT);
			*(volatile unsigned int *)IO_ADDRESS(OMAP3430_GPT10_TCLR) = 0;
			
			//omap_pwm_gpt_clk_enable(0, 10);
		}

	return;
}

/*************************************************************************
  Function:       omap_led_probe
  Description:    initialization function of all led devices.
  Calls:          gpio_request()
  				  gpio_direction_output()
  				  INIT_WORK()
  				  led_classdev_register()
  				  led_classdev_unregister()
  				  gpio_free()
  				  
  Called By:      schedule_work
  Input:          @led_cdev : led class device
                  @value	: led value, 0 is off, 255 is on 
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static int omap_led_probe(struct platform_device *dev)
{
	struct omap_led_platform_data *pdata;
	struct omap_led_config *leds;
	struct omap_led_handle *led_hdl = led_handle;
	int i, ret = 0;

	assert(dev);

	dev->dev.platform_data=&leds_data;
	pdata = dev->dev.platform_data;
	leds = pdata->leds;
	LED_LOG("enter %s at line %d\n",__FUNCTION__,__LINE__);
	for (i = 0; ret >= 0 && i < pdata->nr_leds; i++) 
	{
		/* the gpio is available*/
		if ((leds[i].gpio > 0) && (led_hdl[i].complex == LED_SIMPLE))  // panzhihua0522 ¸ÄÕý¸³Öµ´íÎó
		{
			LED_LOG("request gpio(%d) for led(%s)\n", leds[i].gpio, leds[i].cdev.name);
			ret = gpio_request(leds[i].gpio, leds[i].cdev.name);
			if (ret < 0)
			{//fix bug. if one led errors, others continue.
				//break;
				ret = 0;//0 is ok.
				continue;
			}

			/*set the gpio direction and output vaule:0 */
			gpio_direction_output(leds[i].gpio, 0);
			LED_LOG("request ok\n");
		}
		if (led_hdl[i].led_set)
		{
			leds[i].cdev.brightness_set = led_hdl[i].led_set;
			LED_LOG("led dump: set_led\n"); 
		}
		
#ifdef COMPLEX_LED
		/* init complex led work */
		LED_LOG("led dump: \nwork_func-%x\ncomplex-%d\nwork-%x\n", 
		led_hdl[i].work_func, led_hdl[i].complex,led_hdl[i].led_work
		);
		if ((led_hdl[i].work_func) 
			&& (led_hdl[i].complex == LED_COMPLEX)
			&& (led_hdl[i].led_work))
		{
			INIT_WORK(&led_hdl[i].led_work->work, led_hdl[i].work_func);
			led_work_pool[i].brightness=127;
		}
#endif

		ret = led_classdev_register(&dev->dev, &leds[i].cdev);
		LED_LOG("led_init led %d:(%s) ret:%d\n", leds[i].gpio, leds[i].cdev.name, ret);

		if (ret < 0)
		{
			led_classdev_unregister(&leds[i].cdev);
			gpio_free(leds[i].gpio);
			ret = 0;//reset the retval and continue the rest. 
		}
	}
//liucunqing fix driver bug.
#if 0
	if (ret < 0 && i > 1) {
		for (i = i - 2; i >= 0; i--) {
			led_classdev_unregister(&leds[i].cdev);
			gpio_free(leds[i].gpio);
		}
	}
#endif
	return ret;
}


/*************************************************************************
  Function:       omap_led_remove
  Description:    remove all led devices.
  Calls:          led_classdev_unregister()
  				  gpio_free()
  Called By:      
  Input:          @dev : led device configuration of omap3430 zoom2 board
  Output:         none
  Return:         0
  Others:         
**************************************************************************/
static int omap_led_remove(struct platform_device *dev)
{
	struct omap_led_platform_data *pdata;
	struct omap_led_config *leds;
	int i;

	assert(dev);

	pdata = dev->dev.platform_data;
	leds = pdata->leds;

	for (i = 0; i < pdata->nr_leds; i++) 
	{
		led_classdev_unregister(&leds[i].cdev);
		gpio_free(leds[i].gpio);
	}

	return 0;
}


#ifdef CONFIG_PM
/*************************************************************************
  Function:       omap_led_suspend
  Description:    suspend all led devices.
  Calls:          led_classdev_suspend()
  Called By:      
  Input:          @dev 		: led device configuration of omap3430 zoom2 board
  				  @state 	: power management message. 
  Output:         none
  Return:         0
  Others:         
**************************************************************************/
static int omap_led_suspend(struct platform_device *dev, pm_message_t state)
{
	struct omap_led_platform_data *pdata;
	struct omap_led_config *leds;
	struct led_control_work *led_work = led_handle[TWL5030_LEDB].led_work;

	int i;

	assert(dev);

	cancel_work_sync(&led_work->work);

	pdata = dev->dev.platform_data;
	leds = pdata->leds;
	
	for (i = 0; i < pdata->nr_leds; i++)
	{
		led_classdev_suspend(&leds[i].cdev);
	}
	
	return 0;
}


/*************************************************************************
  Function:       omap_led_resume
  Description:    resume all led devices.
  Calls:          led_classdev_resume()
  Called By:      
  Input:          @dev 		: led device configuration of omap3430 zoom2 board
  Output:         none
  Return:         0
  Others:         
**************************************************************************/
static int omap_led_resume(struct platform_device *dev)
{
	struct omap_led_platform_data *pdata = dev->dev.platform_data;
	struct omap_led_config *leds = pdata->leds;
	int i;

	assert(dev);
	
	for (i = 0; i < pdata->nr_leds; i++)
	{
		led_classdev_resume(&leds[i].cdev);
	}
	
	return 0;
}
#else
#define omap_led_suspend	NULL
#define omap_led_resume		NULL
#endif

static struct platform_driver omap_led_driver = {
	.probe		= omap_led_probe,
	.remove		= omap_led_remove,
	.suspend	= omap_led_suspend,
	.resume		= omap_led_resume,
	.driver		= {
		.name		= "omap-led",
		.owner		= THIS_MODULE,
	},
};
 

/*************************************************************************
  Function:       omap_led_init
  Description:    led driver initialization
  Calls:          platform_device_register()
  				  platform_driver_register()
  Called By:      
  Input:          void
  Output:         none
  Return:         success:0, other:fail
  Others:         
**************************************************************************/
static int __init omap_led_init(void)
{
	int ret = 0;
	LED_LOG("enter %s at line %d\n",__FUNCTION__,__LINE__);

    #if 0
	//set the GPIO mode to be output.
	omap_cfg_reg(T3_34XX_GPIO_178);
	omap_cfg_reg(T3_34XX_GPIO_179);
	omap_cfg_reg(Y3_34XX_GPIO180);
	omap_cfg_reg(Y4_34XX_GPIO181);
	#endif
	

	return platform_driver_register(&omap_led_driver);
}

	
/*************************************************************************
  Function:       omap_led_exit
  Description:    led driver de-initialization
  Calls:          platform_device_unregister()
  				  platform_driver_unregister()
  Called By:      
  Input:          void
  Output:         none
  Return:         void
  Others:         
**************************************************************************/
static void __exit omap_led_exit(void)
{
 	platform_driver_unregister(&omap_led_driver);
	return;
}

module_init(omap_led_init);
module_exit(omap_led_exit);

MODULE_AUTHOR("liucunqing@yulong.com>");
MODULE_DESCRIPTION("OMAP LED driver");
MODULE_LICENSE("GPL");

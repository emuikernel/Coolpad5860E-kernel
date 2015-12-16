/*
 * Copyright (C) 2009 Texas Instruments Inc.
 *
 * Modified from mach-omap2/board-zoom-peripherals.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c/twl.h>
#include <linux/spi/spi.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <plat/common.h>
#include <plat/control.h>
#include <plat/mcspi.h>
#include <plat/display.h>
#include <plat/omap-pm.h>
#include <plat/misc.h>
#include <plat/yl_debug.h>//added by huangjiefeng
#include "mux.h"

#ifdef CONFIG_PANEL_SIL9022
#include <mach/sil9022.h>
#endif

#define LCD_PANEL_ENABLE_GPIO		(7 + OMAP_MAX_GPIO_LINES)
#define LCD_PANEL_RESET_GPIO_PROD	96
#define LCD_PANEL_RESET_GPIO_PILOT	55
#define LCD_PANEL_QVGA_GPIO		    56
#define TV_PANEL_ENABLE_GPIO		95
#define SIL9022_RESET_GPIO          97

#define ENABLE_VAUX2_DEDICATED          0x09
#define ENABLE_VAUX2_DEV_GRP            0x20
#define ENABLE_VAUX3_DEDICATED          0x03
#define ENABLE_VAUX3_DEV_GRP            0x20

#define ENABLE_VPLL2_DEDICATED          0x05
#define ENABLE_VPLL2_DEV_GRP            0xE0
#define TWL4030_VPLL2_DEV_GRP           0x33
#define TWL4030_VPLL2_DEDICATED         0x36

int omap_mux_init_signal(const char *muxname, int val);

struct zoom_dss_board_info {
	int gpio_flag;
};

#define LCD_PANEL_BACKLIGHT_GPIO        182
// LCD backlight intensity
#define LCD_BACKLIGHT_LEVEL_STARTUP_DURATION    (5)             // as recommended by h/w team
#define LCD_BACKLIGHT_LEVEL_STARTUP             (50)            // as recommended by h/w team
#define LCD_BACKLIGHT_LEVEL                     (110)

unsigned int g_backlight_level = LCD_BACKLIGHT_LEVEL;
int is_lcd_suspend = false;

 
#define GPT8_FUNC_CLK    		(0x48005000)
#define GPT8_INTERFACE_CLK    	(0x48005010)

extern void msdelay();
void turn_on_lcd_power(void)
{
    printk(KERN_ERR "enter %s func!\n",__FUNCTION__);
    
	//power on some power domain
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, TWL4030_PROTECT_KEY);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, TWL4030_PROTECT_KEY);

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x04, TWL4030_VSIM_DEDICATED);//vsim 2.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x07<<5), TWL4030_VSIM_DEV_GRP);//vsim DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, TWL4030_VAUX2_DEDICATED);        //VAUX2 1.8V
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x07 << 5), TWL4030_VAUX2_DEV_GRP);   //VAUX2 DEV_GRP belong to P1 P2 P3
        msdelay(50);
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x09, TWL4030_VMMC2_DEDICATED);        //VMMC2 2.8V
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x07 << 5), TWL4030_VMMC2_DEV_GRP);   //VMMC2 DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, TWL4030_PROTECT_KEY);
}

void turn_off_lcd_power(void)
{
    printk(KERN_ERR "enter %s func!\n",__FUNCTION__);

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, TWL4030_PROTECT_KEY);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, TWL4030_PROTECT_KEY);

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0, TWL4030_VAUX2_DEV_GRP);   //VAUX2 DEV_GRP belong to P1 P2 P3
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0, TWL4030_VMMC2_DEV_GRP);   //VMMC2 DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, TWL4030_PROTECT_KEY);
}

void turn_on_pll2(void)
{
	//power on some power domain
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x36);//vpll2 belong to no dev group
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (7<<5), 0x33);//vpll2 belong to no dev group

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);
}

int zoom_pwm_output_clock_enable(void) 
{ 
	unsigned int reg;
	
    reg = omap_readl(GPT8_FUNC_CLK);
	reg |= 0x0200;
	omap_writel(reg, GPT8_FUNC_CLK);

	reg = omap_readl(GPT8_INTERFACE_CLK);
	reg |= 0x0200;
	omap_writel(reg, GPT8_INTERFACE_CLK);

	return 0;
} 
  
void zoom_pwm_output_clock_disable(void) 
{ 
	unsigned int reg;
	
    reg = omap_readl(GPT8_FUNC_CLK);
	reg &= 0xFFFFFDFF;
	omap_writel(reg, GPT8_FUNC_CLK);

	reg = omap_readl(GPT8_INTERFACE_CLK);
	reg &= 0xFFFFFDFF;
	omap_writel(reg, GPT8_INTERFACE_CLK);
} 

#define GPT8_TCLR       (0x4903e024)
#define GPT8_TCRR       (0x4903e028)
#define GPT8_TLDR       (0x4903e02c)
#define GPT8_TTGR       (0x4903e030)
#define GPT8_TMAR       (0x4903e038)

static int zoom_pwm_config_pwmon(unsigned int level)
{
#define TIMER_REF_TICKS       (32768)
#define MAX_BRIGHTNESS_LEVEL  (255)
#define MAX_DUTY_CYCLE        (100)
#define DIVIDER_1M            (1000000)
#define PWM_ON_OFF_PERIOD     (31500)          // PWM freq = ~25K

  int ret = 0;
  unsigned int pwm_level = 0;
  unsigned int on_period = 0;
  unsigned int off_period = 0;
  unsigned int load_reg = 0;
  unsigned int cmp_reg = 0;
  
  
  // enable pwm gpt8 fclk and iclk
  ret = zoom_pwm_output_clock_enable();
  if(0 != ret)
  {
    return -1;
  }
  
  // calculate the duty cycle based on brightness value ==> 0 - 255 <==> 0 - 100 %
  pwm_level = (MAX_DUTY_CYCLE * level) / MAX_BRIGHTNESS_LEVEL;
  
  // calculate the on-off duration basedon duty cycle
  on_period = (PWM_ON_OFF_PERIOD * pwm_level) / MAX_DUTY_CYCLE;
  off_period = (PWM_ON_OFF_PERIOD * (MAX_DUTY_CYCLE - pwm_level)) / MAX_DUTY_CYCLE;
  
  // calculate load and compare register value based on calculated on-off period
  load_reg = TIMER_REF_TICKS * (on_period + off_period) / DIVIDER_1M;
  cmp_reg = TIMER_REF_TICKS * off_period / DIVIDER_1M;

  // configure the timer load value
  //*(volatile unsigned int *)IO_ADDRESS(GPT8_TLDR) = -load_reg;
  omap_writel(-load_reg, GPT8_TLDR);

  // configure the value to be compared with counter
  //*(volatile unsigned int *)IO_ADDRESS(GPT8_TMAR) = -cmp_reg;
  omap_writel(-cmp_reg, GPT8_TMAR);

  // trigger the counter reload of timer
  //*(volatile unsigned int *)IO_ADDRESS(GPT8_TTGR) = 0xfffffffa; 
  omap_writel(0xfffffffa, GPT8_TTGR);

  // optional features related to timer functionality - pwm and capture mode features - Stop the timer
  //*(volatile unsigned int *)IO_ADDRESS(GPT8_TCLR) = 0x00001842;   // Toggle modulation[12] | Overflow and match trigger [11:10] | compare enabled[6] | auto reload[1] | start/stop timer[0]
  omap_writel(0x00001842, GPT8_TCLR);

  // optional features related to timer functionality - pwm and capture mode features - Start the timer
  //*(volatile unsigned int *)IO_ADDRESS(GPT8_TCLR) = 0x00001843;   // toggle modulation[12] | overflow and match trigger [11:10] | compare enabled[6] | auto reload[1] | start/stop timer[0]
  omap_writel(0x00001843, GPT8_TCLR);


  return ret;
}

void zoom_pwm_config_pwmoff(void) 
{
	zoom_pwm_output_clock_disable();
	omap_mux_init_signal("gpio_182",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_request(182, "pwmoff");
	gpio_direction_output(LCD_PANEL_BACKLIGHT_GPIO, 0);    
} 

extern int yl_get_debug_enalbe(int modem_name);
static int zoom_set_backlight_level(struct omap_dss_device *dssdev, int level) 
{  
    int ret = 0;

    if (level < 0) 
	{ 
		g_backlight_level = 0;
                 
    } 
	else 
	{
        // set max level = 254 to avoid 100% duty cycle as it can damage backlight driver ic
        if (level > 254)
		{ 
            g_backlight_level = 254;
		}
		else
		{
			g_backlight_level = level;
		}
		
	} 

    if(false == is_lcd_suspend)
    {
	    ret = zoom_pwm_config_pwmon(g_backlight_level);
        yl_lcd_debug(LOG_INFO,"\n############ %s: %d ############\n", __func__, g_backlight_level);
    }


    return 0; 
}

int zoom_is_lcd_on(void)
{
    return g_backlight_level; 
}

static int zoom_get_backlight_level(struct omap_dss_device *dssdev) 
{ 
    return g_backlight_level; 
}

static void zoom_lcd_tv_panel_init(void)
{
#if 0
	int ret;
	unsigned char lcd_panel_reset_gpio;

	if (omap_rev() > OMAP3430_REV_ES3_0) {
		/* Production Zoom2 board:
		 * GPIO-96 is the LCD_RESET_GPIO
		 */
		lcd_panel_reset_gpio = LCD_PANEL_RESET_GPIO_PROD;
	} else {
		/* Pilot Zoom2 board:
		 * GPIO-55 is the LCD_RESET_GPIO
		 */
		lcd_panel_reset_gpio = LCD_PANEL_RESET_GPIO_PILOT;
	}

	ret = gpio_request(lcd_panel_reset_gpio, "lcd reset");
	if (ret) {
		pr_err("Failed to get LCD reset GPIO.\n");
		return;
	}
	gpio_direction_output(lcd_panel_reset_gpio, 1);

	ret = gpio_request(LCD_PANEL_QVGA_GPIO, "lcd qvga");
	if (ret) {
		pr_err("Failed to get LCD_PANEL_QVGA_GPIO.\n");
		goto err0;
	}
	gpio_direction_output(LCD_PANEL_QVGA_GPIO, 1);

	ret = gpio_request(TV_PANEL_ENABLE_GPIO, "tv panel");
	if (ret) {
		pr_err("Failed to get TV_PANEL_ENABLE_GPIO.\n");
		goto err1;
	}
	gpio_direction_output(TV_PANEL_ENABLE_GPIO, 0);

	return;

err1:
	gpio_free(LCD_PANEL_QVGA_GPIO);
err0:
	gpio_free(lcd_panel_reset_gpio);
#endif
}

int zoom_panel_power_enable(int enable)
{
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,
				(enable) ? ENABLE_VPLL2_DEDICATED : 0,
				TWL4030_VPLL2_DEDICATED);
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,
				(enable) ? ENABLE_VPLL2_DEV_GRP : 0,
				TWL4030_VPLL2_DEV_GRP);

	return 0;
}

static int zoom_panel_enable_lcd(struct omap_dss_device *dssdev)
{
	int ret;
	//struct zoom_dss_board_info *pdata;

	yl_lcd_debug(LOG_INFO, "<board-zoom-panel.c> zoom_panel_enable_lcd IN\n");
	
	//ret = zoom_panel_power_enable(1);
	//if (ret < 0)
	//	return ret;

	omap_mux_init_signal("mcspi2_cs1.gpt8_pwm_evt",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
  
	zoom_pwm_config_pwmon(g_backlight_level);
  
	printk(KERN_ERR"<board-zoom-panel.c> zoom_panel_enable_lcd out\n");
	
	
	return 0;
}

static void zoom_panel_disable_lcd(struct omap_dss_device *dssdev)
{

	zoom_panel_power_enable(0);
	
	zoom_pwm_config_pwmoff();
}
//extern void Module_usb_switch_to_tv_out(void);
static int zoom_panel_enable_tv(struct omap_dss_device *dssdev)
{
#if 0
#define ENABLE_VDAC_DEDICATED           0x03
#define ENABLE_VDAC_DEV_GRP             0x20
	printk("<board-zoom-panel.c> zoom_panel_enable_tv IN\n");
	
	//Module_usb_switch_to_tv_out();
	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,
			ENABLE_VDAC_DEDICATED,
			TWL4030_VDAC_DEDICATED);
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,
			ENABLE_VDAC_DEV_GRP, TWL4030_VDAC_DEV_GRP);
			
	printk("<board-zoom-panel.c> zoom_panel_enable_tv OUT\n");
#endif
	return 0;
}

static void zoom_panel_disable_tv(struct omap_dss_device *dssdev)
{
#if 0
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00,
			TWL4030_VDAC_DEDICATED);
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00,
			TWL4030_VDAC_DEV_GRP);
#endif
}

static struct zoom_dss_board_info zoom_dss_lcd_data = {
	.gpio_flag = 0,
};

static struct omap_dss_device zoom_lcd_device = {
	.name = "lcd",
	.driver_name = "cp9930_sharp_lcd_panel",
	.type = OMAP_DISPLAY_TYPE_DPI,
	.phy.dpi.data_lines = 24,
	.platform_enable = zoom_panel_enable_lcd,
	.platform_disable = zoom_panel_disable_lcd,
	.set_backlight = zoom_set_backlight_level, 
    .get_backlight = zoom_get_backlight_level, 
	.dev = {
		.platform_data = &zoom_dss_lcd_data,
	},
};

static struct omap_dss_device zoom_tv_device = {
	.name                   = "tv",
	.driver_name            = "venc",
	.type                   = OMAP_DISPLAY_TYPE_VENC,
	.phy.venc.type          = -1,
	.platform_enable        = zoom_panel_enable_tv,
	.platform_disable       = zoom_panel_disable_tv,
};

#ifdef CONFIG_PANEL_SIL9022
void config_hdmi_gpio(void)
{
	/* HDMI_RESET uses CAM_PCLK mode 4*/
	omap_mux_init_signal("gpio_97", OMAP_PIN_INPUT_PULLUP);
}

void zoom_hdmi_reset_enable(int level)
{
	/* Set GPIO_97 to high to pull SiI9022 HDMI transmitter out of reset
	* and low to disable it.
	*/
	gpio_request(SIL9022_RESET_GPIO, "hdmi reset");
	gpio_direction_output(SIL9022_RESET_GPIO, level);
}

static int zoom_panel_enable_hdmi(struct omap_dss_device *dssdev)
{
	zoom_hdmi_reset_enable(1);
	return 0;
}

static void zoom_panel_disable_hdmi(struct omap_dss_device *dssdev)
{
	zoom_hdmi_reset_enable(0);
}

struct hdmi_platform_data zoom_hdmi_data = {
#ifdef CONFIG_PM
	.set_min_bus_tput = omap_pm_set_min_bus_tput,
	.set_max_mpu_wakeup_lat =  omap_pm_set_max_mpu_wakeup_lat,
#endif
};

static struct omap_dss_device zoom_hdmi_device = {
	.name = "hdmi",
	.driver_name = "hdmi_panel",
	.type = OMAP_DISPLAY_TYPE_DPI,
	.phy.dpi.data_lines = 24,
	.platform_enable = zoom_panel_enable_hdmi,
	.platform_disable = zoom_panel_disable_hdmi,
	.dev = {
		.platform_data = &zoom_hdmi_data,
	},
};
#endif


static struct omap_dss_device *zoom_dss_devices[] = {
	&zoom_lcd_device,
    &zoom_tv_device,
#ifdef CONFIG_PANEL_SIL9022
	&zoom_hdmi_device,
#endif

};

static struct omap_dss_board_info zoom_dss_data = {
	.num_devices = ARRAY_SIZE(zoom_dss_devices),
	.devices = zoom_dss_devices,
	.default_device = &zoom_lcd_device,//&zoom_tv_device,//
};

static struct omap2_mcspi_device_config dss_lcd_mcspi_config = {
	.turbo_mode             = 0,
	.single_channel         = 1,  /* 0: slave, 1: master */
};

static struct spi_board_info cp9930_lr388_spi_board_info[] __initdata = {
	[0] = {
		.modalias               = "cp9930_lr388_spi",
		.bus_num                = 1,
		.chip_select            = 2,
		.max_speed_hz           = 375000,
        .mode                   = OMAP2_MCSPI_MASTER,
		.controller_data        = &dss_lcd_mcspi_config,
	},
};

void __init zoom_display_init(enum omap_dss_venc_type venc_type)
{
	yl_lcd_debug(LOG_INFO, "<board-zoom-panel.c> zoom_display_init IN\n");
	zoom_tv_device.phy.venc.type = venc_type;
	omap_display_init(&zoom_dss_data);
	spi_register_board_info(cp9930_lr388_spi_board_info,
				ARRAY_SIZE(cp9930_lr388_spi_board_info));
	zoom_lcd_tv_panel_init();
	yl_lcd_debug(LOG_INFO, "<board-zoom-panel.c> zoom_display_init OUT\n");
}


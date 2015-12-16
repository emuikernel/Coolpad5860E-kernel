/*
 * Copyright (C) 2009 Texas Instruments Inc.
 *
 * Modified from mach-omap2/board-zoom2.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/input/matrix_keypad.h>
#include <linux/gpio.h>
#include <linux/i2c/twl.h>
#include <linux/regulator/machine.h>
#include <linux/synaptics_i2c_rmi.h>
#include <linux/mmc/host.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/common.h>
#include <plat/usb.h>
#include <plat/control.h>
#ifdef CONFIG_SERIAL_OMAP
#include <plat/omap-serial.h>
#include <plat/serial.h>
#endif
#include <linux/interrupt.h>
#include <linux/switch.h>

#ifdef CONFIG_LEDS_OMAP_DISPLAY
#include <linux/leds.h>
#include <linux/leds-omap-display.h>
#endif
#include <linux/delay.h>
#include <linux/i2c-gpio.h>
#include <plat/keypad.h>

#ifdef CONFIG_PANEL_SIL9022
#include <mach/sil9022.h>
#endif

#include <mach/board-zoom.h>

#include "mux.h"
#include "hsmmc.h"
#include "twl4030.h"
#include "prm-regbits-34xx.h"

//added by zhuhui in 2001.09.01
#ifdef CONFIG_TOUCHSCREEN_FT5X06_TS
#include "board-cp5860e-touchscreen.h"
#endif

#include "board-cp5860e-sensors.h"    ///linronghui add for sensors.20110930
#include "board-cp5860e-audio.h" //yuanyufang 

#include "board-cp5860e-camera.h" 

#include <linux/wakelock.h>
#include <plat/misc.h>

#include <media/v4l2-int-device.h>

#if (defined(CONFIG_VIDEO_IMX046) || defined(CONFIG_VIDEO_IMX046_MODULE)) && \
	defined(CONFIG_VIDEO_OMAP3)
#include <media/imx046.h>
extern struct imx046_platform_data zoom2_imx046_platform_data;
#endif

#if (defined(CONFIG_VIDEO_LV8093) || defined(CONFIG_VIDEO_LV8093_MODULE)) && \
	defined(CONFIG_VIDEO_OMAP3)
#include <media/lv8093.h>
extern struct imx046_platform_data zoom2_lv8093_platform_data;
#define LV8093_PS_GPIO		7
/* GPIO7 is connected to lens PS pin through inverter */
#define LV8093_PWR_OFF		1
#define LV8093_PWR_ON		(!LV8093_PWR_OFF)
#endif

#ifdef CONFIG_LEDS_OMAP_DISPLAY
/* PWM output/clock enable for LCD backlight*/
#define REG_INTBR_GPBR1				0xc
#define REG_INTBR_GPBR1_PWM1_OUT_EN		(0x1 << 3)
#define REG_INTBR_GPBR1_PWM1_OUT_EN_MASK	(0x1 << 3)
#define REG_INTBR_GPBR1_PWM1_CLK_EN		(0x1 << 1)
#define REG_INTBR_GPBR1_PWM1_CLK_EN_MASK	(0x1 << 1)

/* pin mux for LCD backlight*/
#define REG_INTBR_PMBR1				0xd
#define REG_INTBR_PMBR1_PWM1_PIN_EN		(0x3 << 4)
#define REG_INTBR_PMBR1_PWM1_PIN_MASK		(0x3 << 4)

#define MAX_CYCLES				0x7f
#define MIN_CYCLES				75
#define LCD_PANEL_BACKLIGHT_GPIO		(7 + OMAP_MAX_GPIO_LINES)
#endif

#define BLUETOOTH_UART	UART2

static struct wake_lock uart_lock;


extern u8 is_p0;

/* Zoom2 has Qwerty keyboard*/
static int board_keymap[] = {
#if 0
	KEY(0, 0, KEY_E),
	KEY(1, 0, KEY_R),
	KEY(2, 0, KEY_T),
	KEY(3, 0, KEY_HOME),
	KEY(6, 0, KEY_I),
	KEY(7, 0, KEY_LEFTSHIFT),
	KEY(0, 1, KEY_D),
	KEY(1, 1, KEY_F),
	KEY(2, 1, KEY_G),
	KEY(3, 1, KEY_SEND),
	KEY(6, 1, KEY_K),
	KEY(7, 1, KEY_ENTER),
	KEY(0, 2, KEY_X),
	KEY(1, 2, KEY_C),
	KEY(2, 2, KEY_V),
	KEY(3, 2, KEY_END),
	KEY(6, 2, KEY_DOT),
	KEY(7, 2, KEY_CAPSLOCK),
	KEY(0, 3, KEY_Z),
	KEY(1, 3, KEY_KPPLUS),
	KEY(2, 3, KEY_B),
	KEY(3, 3, KEY_F1),
	KEY(6, 3, KEY_O),
	KEY(7, 3, KEY_SPACE),
	KEY(0, 4, KEY_W),
	KEY(1, 4, KEY_Y),
	KEY(2, 4, KEY_U),
	KEY(3, 4, KEY_F2),
	KEY(4, 4, KEY_VOLUMEUP),
	KEY(6, 4, KEY_L),
	KEY(7, 4, KEY_LEFT),
	KEY(0, 5, KEY_S),
	KEY(1, 5, KEY_H),
	KEY(2, 5, KEY_J),
	KEY(3, 5, KEY_F3),
	KEY(5, 5, KEY_VOLUMEDOWN),
	KEY(6, 5, KEY_M),
	KEY(4, 5, KEY_ENTER),
	KEY(7, 5, KEY_RIGHT),
	KEY(0, 6, KEY_Q),
	KEY(1, 6, KEY_A),
	KEY(2, 6, KEY_N),
	KEY(3, 6, KEY_BACKSPACE),
	KEY(6, 6, KEY_P),
	KEY(7, 6, KEY_UP),
	KEY(6, 7, KEY_SELECT),
	KEY(7, 7, KEY_DOWN),
	KEY(0, 7, KEY_PROG1),	/*MACRO 1 <User defined> */
	KEY(1, 7, KEY_PROG2),	/*MACRO 2 <User defined> */
	KEY(2, 7, KEY_PROG3),	/*MACRO 3 <User defined> */
	KEY(3, 7, KEY_PROG4),	/*MACRO 4 <User defined> */
#endif
KEY(0, 0, KEY_VOLUMEUP),
KEY(1, 0, KEY_VOLUMEDOWN),
//KEY(0, 0, KEY_BACK),
//KEY(1, 0, KEY_HOME), 
};

static struct matrix_keymap_data board_map_data = {
	.keymap			= board_keymap,
	.keymap_size		= ARRAY_SIZE(board_keymap),
};

static struct twl4030_keypad_data zoom_kp_twl4030_data = {
	.keymap_data	= &board_map_data,
	.rows		= 2,
	.cols		= 1,
	.rep		= 1,
};

//added by huangjiefeng in 20110726
static int omap_keymap[] = {
	KEY(0, 0, KEY_CAMERA),
};

static unsigned int omap_key_row_gpios[] = {
    7,
};

static unsigned int omap_key_col_gpios[] = {
	OMAP_MAX_GPIO_LINES+1,//gpio invalid
};

static struct omap_kp_platform_data omap_kp_data = {
	.rows		= 1,
	.cols		= 1,
	.keymap		= omap_keymap,
	.keymapsize	= ARRAY_SIZE(omap_keymap),
	.delay		= 4,
	.rep		= 0,
	.row_gpios  = omap_key_row_gpios,	
	.col_gpios  = omap_key_col_gpios,
};

static struct platform_device omap_kp_device = {
	.name		= "omap-keypad",
	.id		= -1,
	.dev		= {
		.platform_data = &omap_kp_data,
	},
};
//End huangjiefeng

static struct __initdata twl4030_power_data zoom_t2scripts_data;

static struct regulator_consumer_supply zoom_vmmc1_supply = {
	.supply		= "vmmc",
};

/////////////////////penglanhua
static struct regulator_consumer_supply zoom_vio_supply = {
	.supply		= "vio",
};

static struct regulator_consumer_supply zoom_vmmc2_supply = {
	.supply		= "vmmc",
};

///////////////////////


static struct regulator_consumer_supply zoom_vsim_supply = {
	.supply		= "vmmc_aux",
};

/* VMMC1 for OMAP VDD_MMC1 (i/o) and MMC1 card */
static struct regulator_init_data zoom_vmmc1 = {
	.constraints = {
		.min_uV			= 1850000,
		.max_uV			= 3150000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &zoom_vmmc1_supply,
};

/* VMMC2 for MMC2 card */
static struct regulator_init_data zoom_vmmc2 = {
	.constraints = {
		.min_uV			= 2850000,
		.max_uV			= 2850000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &zoom_vmmc2_supply,
};

////////////////penglanhua
/*  for MMC2 card /inand */
static struct regulator_init_data zoom_vio = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &zoom_vio_supply,
};

/* VSIM for OMAP VDD_MMC1A (i/o for DAT4..DAT7) */
static struct regulator_init_data zoom_vsim = {
	.constraints = {
		.min_uV			= 3000000,
		.max_uV			= 3000000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &zoom_vsim_supply,
};

#define HEADSET_KEY  (OMAP_MAX_GPIO_LINES + 2)  /* TWL4030 GPIO_2 */

static struct resource switch_platform_resource[] = 
{
#if 0
	[0] = 
	{
		.start	    = ONE_KEY_MUTE,
		.end        = 0x00140014,//hi=20ms,lo=20ms
		.name       = "okm",
		.flags	    = IORESOURCE_MEM,
	},
	[1] =
	{
		.start	    = HALL_KEY,
		.end        = 0x00140014,//hi=20ms,lo=20ms
		.name       = "hall",
		.flags	    = IORESOURCE_IRQ,
	},
#endif
	[0] =
	{
		.start	    = HEADSET_KEY,
		.end        = 0x00320032,//hi=0x258=600ms,lo=0032=50ms	//ÐÞžÄŒì²âÊ±Œä
		.name       = "h2w",
		.flags	    = IORESOURCE_IRQ,
	},
};

#if 0
static struct gpio_switch_platform_data headset_switch_data = {
	.name		= "h2w",
	.gpio		= OMAP_MAX_GPIO_LINES + 2, /* TWL4030 GPIO_2 */
};
#endif

static struct platform_device headset_key_device = {
	.name			    = "headset_key",
	.id			        = -1,
	.num_resources  	= 0,
	.resource	      	= NULL,
};


static struct platform_device twl4030_test_device = {
	.name				= "twl4030_test",
	.id			        = -1,
	.num_resources  	= 0,
	.resource	      	= NULL,
};

static struct platform_device headset_switch_device = {
	.name		= "switch-gpio",
	.id		= -1,
	.num_resources   = ARRAY_SIZE(switch_platform_resource),//èµæºæ°é
    .resource	     = switch_platform_resource,
	.dev		= {
		.platform_data = NULL,
	}
};

#ifdef CONFIG_LEDS_OMAP_DISPLAY
/* omap3 led display */
static void zoom_pwm_config(u8 brightness)
{

	u8 pwm_off = 0;

	pwm_off = (MIN_CYCLES * (LED_FULL - brightness) +
		   MAX_CYCLES * (brightness - LED_OFF)) /
		(LED_FULL - LED_OFF);

	pwm_off = clamp(pwm_off, (u8)MIN_CYCLES, (u8)MAX_CYCLES);

	printk(KERN_DEBUG "PWM Duty cycles = %d\n", pwm_off);

	/* start at 0 */
	twl_i2c_write_u8(TWL4030_MODULE_PWM1, 0, 0);
	twl_i2c_write_u8(TWL4030_MODULE_PWM1, pwm_off, 1);
}

static void zoom_pwm_enable(int enable)
{
	u8 gpbr1;

	twl_i2c_read_u8(TWL4030_MODULE_INTBR, &gpbr1, REG_INTBR_GPBR1);
	gpbr1 &= ~REG_INTBR_GPBR1_PWM1_OUT_EN_MASK;
	gpbr1 |= (enable ? REG_INTBR_GPBR1_PWM1_OUT_EN : 0);
	twl_i2c_write_u8(TWL4030_MODULE_INTBR, gpbr1, REG_INTBR_GPBR1);

	twl_i2c_read_u8(TWL4030_MODULE_INTBR, &gpbr1, REG_INTBR_GPBR1);
	gpbr1 &= ~REG_INTBR_GPBR1_PWM1_CLK_EN_MASK;
	gpbr1 |= (enable ? REG_INTBR_GPBR1_PWM1_CLK_EN : 0);
	twl_i2c_write_u8(TWL4030_MODULE_INTBR, gpbr1, REG_INTBR_GPBR1);
}

void omap_set_primary_brightness(u8 brightness)
{
	u8 pmbr1;
	static int zoom_pwm1_config;
	static int zoom_pwm1_output_enabled;

	if (zoom_pwm1_config == 0) {
		twl_i2c_read_u8(TWL4030_MODULE_INTBR, &pmbr1, REG_INTBR_PMBR1);

		pmbr1 &= ~REG_INTBR_PMBR1_PWM1_PIN_MASK;
		pmbr1 |=  REG_INTBR_PMBR1_PWM1_PIN_EN;
		twl_i2c_write_u8(TWL4030_MODULE_INTBR, pmbr1, REG_INTBR_PMBR1);

		zoom_pwm1_config = 1;
	}

	if (!brightness) {
		zoom_pwm_enable(0);
		zoom_pwm1_output_enabled = 0;
		return;
	}

	zoom_pwm_config(brightness);
	if (zoom_pwm1_output_enabled == 0) {
		zoom_pwm_enable(1);
		zoom_pwm1_output_enabled = 1;
	}

	printk(KERN_DEBUG "Zoom LCD Backlight brightness = %d\n", brightness);
}

static struct omap_disp_led_platform_data omap_disp_led_data = {
	.flags = LEDS_CTRL_AS_ONE_DISPLAY,
	.primary_display_set = omap_set_primary_brightness,
	.secondary_display_set = NULL,
};

static struct platform_device omap_disp_led = {
	.name   =       "display_led",
	.id     =       -1,
	.dev    = {
		.platform_data = &omap_disp_led_data,
	},
};
/* end led Display */
#endif

static struct platform_device zoom_led_device = {
	.name	= "omap-led",
	.id	= -1,
	.dev		= {
		.platform_data = NULL,
	}
};

static struct platform_device *zoom_board_devices[] __initdata = {
	&omap_kp_device,
	&headset_key_device,
	&twl4030_test_device,
	&headset_switch_device,
#ifdef CONFIG_LEDS_OMAP_DISPLAY
	&omap_disp_led,
#endif
	//add by maxiaowei for LED	
	&zoom_led_device,
};

static struct omap2_hsmmc_info mmc[] __initdata = {
	{
		.name		= "external",
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_wp	= -EINVAL,
		.power_saving	= false,
	},
	{
		.name		= "internal",
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.nonremovable	= true,
		.power_saving	= false,
	},
	{
		.mmc		= 3,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
	},
	{}      /* Terminator */
};

static struct regulator_consumer_supply zoom_vpll2_supply = {
	.supply         = "vdds_dsi",
};

static struct regulator_consumer_supply zoom_vdda_dac_supply = {
	.supply         = "vdda_dac",
};

static struct regulator_init_data zoom_vpll2 = {
	.constraints = {
		.min_uV                 = 1800000,
		.max_uV                 = 1800000,
		.valid_modes_mask       = REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask         = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &zoom_vpll2_supply,
};

static struct regulator_init_data zoom_vdac = {
	.constraints = {
		.min_uV                 = 1800000,
		.max_uV                 = 1800000,
		.valid_modes_mask       = REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask         = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &zoom_vdda_dac_supply,
};

static int zoom_twl_gpio_setup(struct device *dev,
		unsigned gpio, unsigned ngpio)
{
	/* gpio + 0 is "mmc0_cd" (input/IRQ) */
	mmc[0].gpio_cd = gpio + 0;

#ifdef CONFIG_MMC_EMBEDDED_SDIO
	/* The controller that is connected to the 128x device
	 * should have the card detect gpio disabled. This is
	 * achieved by initializing it with a negative value
	 */
	mmc[CONFIG_TIWLAN_MMC_CONTROLLER - 1].gpio_cd = -EINVAL;
#endif

	omap2_hsmmc_init(mmc);

	/* link regulators to MMC adapters ... we "know" the
	 * regulators will be set up only *after* we return.
	*/
	zoom_vmmc1_supply.dev = mmc[0].dev;
	zoom_vsim_supply.dev = mmc[0].dev;
	//zoom_vmmc2_supply.dev = mmc[1].dev;
	//////////////////////penglanhua
	zoom_vio_supply.dev = mmc[1].dev;
	///////////////////

	return 0;
}

/* EXTMUTE callback function */
static void zoom2_set_hs_extmute(int mute)
{
	//gpio_set_value(ZOOM2_HEADSET_EXTMUTE_GPIO, mute);
}

static int zoom_batt_table[] = {
/* 0 C*/
30800, 29500, 28300, 27100,
26000, 24900, 23900, 22900, 22000, 21100, 20300, 19400, 18700, 17900,
17200, 16500, 15900, 15300, 14700, 14100, 13600, 13100, 12600, 12100,
11600, 11200, 10800, 10400, 10000, 9630,  9280,  8950,  8620,  8310,
8020,  7730,  7460,  7200,  6950,  6710,  6470,  6250,  6040,  5830,
5640,  5450,  5260,  5090,  4920,  4760,  4600,  4450,  4310,  4170,
4040,  3910,  3790,  3670,  3550
};

static struct twl4030_bci_platform_data zoom_bci_data = {
	.battery_tmp_tbl	= zoom_batt_table,
	.tblsize		= ARRAY_SIZE(zoom_batt_table),
};

static struct twl4030_usb_data zoom_usb_data = {
	.usb_mode	= T2_USB_MODE_ULPI,
};

static struct twl4030_gpio_platform_data zoom_gpio_data = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
	.irq_base	= TWL4030_GPIO_IRQ_BASE,
	.irq_end	= TWL4030_GPIO_IRQ_END,
	.setup		= zoom_twl_gpio_setup,
	.debounce	= 0x04,
};

static struct twl4030_madc_platform_data zoom_madc_data = {
	.irq_line	= 1,
};

static struct twl4030_codec_audio_data zoom_audio_data = {
	.audio_mclk	= 26000000,
	.ramp_delay_value = 3, /* 161 ms */
	.hs_extmute	= 1,
	.set_hs_extmute	= zoom2_set_hs_extmute,
};

static struct twl4030_codec_data zoom_codec_data = {
	.audio_mclk = 26000000,
	.audio = &zoom_audio_data,
};

static struct twl4030_platform_data zoom_twldata = {
	.irq_base	= TWL4030_IRQ_BASE,
	.irq_end	= TWL4030_IRQ_END,

	/* platform_data for children goes here */
	.bci		= &zoom_bci_data,
	.madc		= &zoom_madc_data,
	.usb		= &zoom_usb_data,
	.gpio		= &zoom_gpio_data,
	.keypad		= &zoom_kp_twl4030_data,
	.power		= &zoom_t2scripts_data,
	.codec		= &zoom_codec_data,
	.vmmc1          = &zoom_vmmc1,
	//.vmmc2		= &zoom_vmmc2,
	/////////////////////////penglanhua
	.vio            = &zoom_vio,
	.vsim           = &zoom_vsim,
	.vpll2		= &zoom_vpll2,
	.vdac		= &zoom_vdac,

};


static struct i2c_board_info __initdata zoom_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("twl5030", 0x48),
		.flags		= I2C_CLIENT_WAKE,
		.irq		= INT_34XX_SYS_NIRQ,
		.platform_data	= &zoom_twldata,
	},
};


static struct i2c_board_info __initdata zoom2_i2c_bus2_info_p1[] = {
#ifdef CONFIG_TOUCHSCREEN_ATMEL224E_TS
	{
         I2C_BOARD_INFO("qt602240_ts",  0x4A),                    //added by zhuhui
		.platform_data = &tw_qt602240_platform_data,
		.irq = OMAP_GPIO_IRQ(OMAP_FT5X06_IRQ_GPIO_P1),
		.flags		= I2C_CLIENT_WAKE,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_FT5X06_TS
	{
		//I2C_BOARD_INFO("ft5x0x_ts",  0xB8), synaptics-rmi-ts                             //added by zhuhui
         I2C_BOARD_INFO("ft5x0x_ts",  0x2c),
		.platform_data = &ft5x06_platform_data,
		.irq = OMAP_GPIO_IRQ(OMAP_FT5X06_IRQ_GPIO_P1),
		.flags		= I2C_CLIENT_WAKE,
	},
#endif	
#if 0
	{
		I2C_BOARD_INFO(SYNAPTICS_I2C_RMI_NAME,  0x20),
		.platform_data = &synaptics_platform_data,
		.irq = OMAP_GPIO_IRQ(OMAP_SYNAPTICS_GPIO),
	},
#endif
#if (defined(CONFIG_VIDEO_IMX046) || defined(CONFIG_VIDEO_IMX046_MODULE)) && \
	defined(CONFIG_VIDEO_OMAP3)
	{
		I2C_BOARD_INFO(IMX046_NAME, IMX046_I2C_ADDR),
		.platform_data = &zoom2_imx046_platform_data,
	},
#endif
#if (defined(CONFIG_VIDEO_LV8093) || defined(CONFIG_VIDEO_LV8093_MODULE)) && \
	defined(CONFIG_VIDEO_OMAP3)
	{
		I2C_BOARD_INFO(LV8093_NAME,  LV8093_AF_I2C_ADDR),
		.platform_data = &zoom2_lv8093_platform_data,
	},
#endif
};

static struct i2c_board_info __initdata zoom2_i2c_bus2_info_p0[] = {
#ifdef CONFIG_TOUCHSCREEN_ATMEL224E_TS
	{       
         I2C_BOARD_INFO("qt602240_ts",  0x4A),                 //added by zhuhui
		.platform_data = &tw_qt602240_platform_data,
		.irq = OMAP_GPIO_IRQ(OMAP_FT5X06_IRQ_GPIO_P0),
		.flags		= I2C_CLIENT_WAKE,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_FT5X06_TS
	{
		//I2C_BOARD_INFO("ft5x0x_ts",  0xB8), synaptics-rmi-ts                             //added by zhuhui
         I2C_BOARD_INFO("ft5x0x_ts",  0x2c),
		.platform_data = &ft5x06_platform_data,
		.irq = OMAP_GPIO_IRQ(OMAP_FT5X06_IRQ_GPIO_P0),
		.flags		= I2C_CLIENT_WAKE,
	},
#endif	
#if 0
	{
		I2C_BOARD_INFO(SYNAPTICS_I2C_RMI_NAME,  0x20),
		.platform_data = &synaptics_platform_data,
		.irq = OMAP_GPIO_IRQ(OMAP_SYNAPTICS_GPIO),
	},
#endif
#if (defined(CONFIG_VIDEO_IMX046) || defined(CONFIG_VIDEO_IMX046_MODULE)) && \
	defined(CONFIG_VIDEO_OMAP3)
	{
		I2C_BOARD_INFO(IMX046_NAME, IMX046_I2C_ADDR),
		.platform_data = &zoom2_imx046_platform_data,
	},
#endif
#if (defined(CONFIG_VIDEO_LV8093) || defined(CONFIG_VIDEO_LV8093_MODULE)) && \
	defined(CONFIG_VIDEO_OMAP3)
	{
		I2C_BOARD_INFO(LV8093_NAME,  LV8093_AF_I2C_ADDR),
		.platform_data = &zoom2_lv8093_platform_data,
	},
#endif
};


static int __init omap_i2c_init(void)
{
	/* Disable OMAP 3630 internal pull-ups for I2Ci */
	if (cpu_is_omap3630()) {

		u32 prog_io;

		prog_io = omap_ctrl_readl(OMAP343X_CONTROL_PROG_IO1);
		/* Program (bit 19)=1 to disable internal pull-up on I2C1 */
		prog_io |= OMAP3630_PRG_I2C1_PULLUPRESX;
		/* Program (bit 0)=1 to disable internal pull-up on I2C2 */
		prog_io |= OMAP3630_PRG_I2C2_PULLUPRESX;
		omap_ctrl_writel(prog_io, OMAP343X_CONTROL_PROG_IO1);

		prog_io = omap_ctrl_readl(OMAP36XX_CONTROL_PROG_IO2);
		/* Program (bit 7)=1 to disable internal pull-up on I2C3 */
		prog_io |= OMAP3630_PRG_I2C3_PULLUPRESX;
		omap_ctrl_writel(prog_io, OMAP36XX_CONTROL_PROG_IO2);

		prog_io = omap_ctrl_readl(OMAP36XX_CONTROL_PROG_IO_WKUP1);
		/* Program (bit 5)=1 to disable internal pull-up on I2C4(SR) */
		prog_io |= OMAP3630_PRG_SR_PULLUPRESX;
		omap_ctrl_writel(prog_io, OMAP36XX_CONTROL_PROG_IO_WKUP1);
	}

	omap_register_i2c_bus(1, 100, NULL, zoom_i2c_boardinfo,
			ARRAY_SIZE(zoom_i2c_boardinfo));
	if( !is_p0 )
		omap_register_i2c_bus(2, 400, NULL, zoom2_i2c_bus2_info_p1,
				ARRAY_SIZE(zoom2_i2c_bus2_info_p1));
	else
		omap_register_i2c_bus(2, 400, NULL, zoom2_i2c_bus2_info_p0,
				ARRAY_SIZE(zoom2_i2c_bus2_info_p0));
	omap_register_i2c_bus(3, 400, NULL, zoom2_i2c_bus3_info,
			ARRAY_SIZE(zoom2_i2c_bus3_info));

#if defined(CONFIG_VIDEO_OMAP3) && defined(CONFIG_I2C_GPIO)
    omap3430_add_camera_i2c();
#endif

#if defined(CONFIG_SOUND_ALC108)
	omap3630_add_alc108_i2c_gpio();
#endif

#if defined(CONFIG_SOUND_FM2018)
	omap3630_add_fm2018_i2c_gpio();
//	fm2018_i2c_init();		//modified by guotao, 2010-06-18
#endif

	return 0;
}

#ifdef CONFIG_USB_MUSB_OTG
static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
	.mode			= MUSB_OTG,
	.power			= 100,
};
#else
#ifdef CONFIG_USB_MUSB_HDRC_HCD
static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
	.mode			= MUSB_HOST,
	.power			= 100,
};
#endif

#ifdef CONFIG_USB_GADGET_MUSB_HDRC
static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
	.mode			= MUSB_PERIPHERAL,
	.power			= 100,
};
#endif
#endif

static void plat_hold_wakelock(void *up, int flag)
{
	struct uart_omap_port *up2 = (struct uart_omap_port *)up;

	/* Specific wakelock for bluetooth usecases */
	if ((up2->pdev->id == BLUETOOTH_UART)
			&& ((flag == WAKELK_TX) || (flag == WAKELK_RX)))
		wake_lock_timeout(&uart_lock, 2*HZ);
}
#define WAKE_LOCK_ACTIVE                 (1U << 9)
static bool is_uart2_active(void)
{
	int ret=0;
	if (uart_lock.flags & WAKE_LOCK_ACTIVE)
		ret=1;
	else
		ret =0;

	return ret;
}

static struct omap_uart_port_info omap_serial_platform_data[] = {
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.plat_hold_wakelock = NULL,
		.rts_padconf   = 0,
		.cts_padconf   = 0,
		.padconf       = 0,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.plat_hold_wakelock = plat_hold_wakelock,
		.plat_omap_bt_active = is_uart2_active,
		.rts_padconf   = 0x176,
		.cts_padconf   = 0x174,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.plat_hold_wakelock = NULL,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.plat_hold_wakelock = NULL,
	},
	{
		.flags		= 0
	}
};


void zoom_turnon_vsim(void)
{
	unsigned int reg;
	
    u8 vsim_val=0;
	//power on vsim before init camera
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x3A);//vsim 2.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x07<<5), 0x37);//vsim DEV_GRP belong to P1 P2 P3
	
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &vsim_val, 0x3A);
	
    printk(KERN_ERR"file: %s,\t function: %s VSIM REG(0x95)=0x%x \n", __FILE__, __func__,vsim_val);

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);

	//set gpio126-129 function
	reg = omap_readl(0x48002520);
	reg |= (3<<8);
	omap_writel(reg, 0x48002520);

	reg = omap_readl(0x48002a5c);
	reg |= (1<<6);
	omap_writel(reg, 0x48002a5c);
}

//added by huangjiefeng in 100330
unsigned int hardware_version = 0;
unsigned int get_device_hardware_version(void)
{	
//	hardware_version = system_rev;
	return hardware_version;
}

void set_device_hardware_version(unsigned int version)
{
	hardware_version = version;
}

static void enable_board_wakeup_source(void)
{
	/* T2 interrupt line (keypad) */
	omap_mux_init_signal("sys_nirq",
		OMAP_WAKEUP_EN | OMAP_PIN_INPUT_PULLUP);
}

void __init zoom_peripherals_init(void)
{
	is_p0 = is_p0_board_version();

	wake_lock_init(&uart_lock, WAKE_LOCK_SUSPEND, "uart_wake_lock");

	twl4030_get_scripts(&zoom_t2scripts_data);
	omap_i2c_init();
	platform_add_devices(zoom_board_devices,
		ARRAY_SIZE(zoom_board_devices));

	//added by zhuhui in 2011.09.01
	#ifdef CONFIG_TOUCHSCREEN_FT5X06_TS
//	ft5x06_dev_init();
	#endif

	omap_serial_init(omap_serial_platform_data);
	usb_musb_init(&musb_board_data);
	enable_board_wakeup_source();
	zoom2_cam_init();
	zoom2_audio_init();
#ifdef CONFIG_PANEL_SIL9022
	config_hdmi_gpio();
	zoom_hdmi_reset_enable(1);
#endif
}

#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/bootmem.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sensor/yl_sensor_power_config.h>

#include <plat/sram.h>
#include <plat/clockdomain.h>
#include <plat/powerdomain.h>
#include <plat/control.h>
#include <plat/serial.h>
#include <plat/sdrc.h>
#include <plat/prcm.h>
#include <plat/gpmc.h>
#include <plat/dma.h>
#include <plat/usb.h>
#include <plat/misc.h>
#include <plat/smartreflex.h>
#include <plat/voltage.h>
#include <plat/yl_debug.h>//added by huangjiefeng in 2012.4.17

#include <mach/gpio_define_for_cp5860e.h>

#include <asm/tlbflush.h>

#include "cm.h"
#include "cm-regbits-34xx.h"
#include "prm-regbits-34xx.h"
#include "mux34xx.h"
#include "mux.h"

#include "prm.h"
#include "pm.h"
#include "sdrc.h"
#include "yl_pm_debug.h"


#define HIGH_LEVEL              1
#define LOW_LEVEL               0

//add by guotao
#define 	GPIO_22				(22)
#define 	GPIO_97				(97)
#define 	GPIO_6				(6)
#define 	GPIO_194			(194)

extern int ac_charger_flag;
extern int usb_charger_flag;
//int lte_power_status = 0;
static int via_power_status = 0;

#ifdef CONFIG_SWITCH_GPIO
extern int get_headset_plug_status(void);
#endif
extern int get_lte_power_status(void);

void lte_gpio_suspend_cfg(void)
{
	u32 reg = 0;
	//LTE reset
	omap_mux_init_signal(LTE_nRST_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_21
	gpio_direction_output(LTE_nRST_PIN,LOW_LEVEL);

	//lte ps_hold
	omap_mux_init_signal(LTE_PSHOLD_PIN_NAME,OMAP_PIN_INPUT);//gpio_23
	gpio_direction_input(LTE_PSHOLD_PIN);

	//lte state(rsv)
	omap_mux_init_signal(LTE_STATE_PIN_NAME,OMAP_PIN_INPUT_PULLDOWN);//gpio_114
	gpio_request(LTE_STATE_PIN,"LTE_STATE_PIN");
	gpio_direction_input(LTE_STATE_PIN);

	//LTE go to sleep(LTE-->AP, unuse)
	omap_mux_init_signal(L2M_SLEEP_PIN_NAME,OMAP_PIN_INPUT_PULLDOWN);//gpio_137
	gpio_direction_input(L2M_SLEEP_PIN);

	//AP control lte to go to sleep(unuse)
	omap_mux_init_signal(M2L_SLEEP_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_138
	gpio_direction_output(M2L_SLEEP_PIN,LOW_LEVEL);

	//LTE USB SEL(P2 board unused)
	omap_mux_init_signal(LTE_USB_SEL_PIN_NAME, OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_150
	gpio_request(LTE_USB_SEL_PIN,"LTE_USB_SEL_PIN");
	gpio_direction_output(LTE_USB_SEL_PIN,LOW_LEVEL);

	//LTE RVD1
	omap_mux_init_signal(LTE_RVD1_PIN_NAME,OMAP_PIN_INPUT_PULLDOWN);//gpio_159
	gpio_direction_input(LTE_RVD1_PIN);

	//LTE BOOT MODE(P2 Board is Cam strobe)
	omap_mux_init_signal(LTE_MODE0_SEL_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_170
	gpio_direction_output(LTE_MODE0_SEL_PIN,LOW_LEVEL);

	if(get_lte_power_status())
	{
		//lte pwr_en
		omap_mux_init_signal(LTE_PWR_EN_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_HIGH);//gpio_24
		gpio_direction_output(LTE_PWR_EN_PIN,HIGH_LEVEL);

		//M2L_WAKE gpio_136
		reg = omap_readl(0x48002164);
		reg &= 0xFFFF0000;
		reg |= 0x11F;
		omap_writel(reg, 0x48002164);//GPIO_136 have a down pulse when wakeup, so config to mode7 before suspend for workaround

		//gpio_139 LTE WAKEUP AP
		//set to wakeupsource

		//gpio_158 LTE RST (LTE--> AP)
		//set to wakeupsource
	}
	else
	{
		//lte pwr_en
		omap_mux_init_signal(LTE_PWR_EN_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_24
		gpio_direction_output(LTE_PWR_EN_PIN, LOW_LEVEL);

		//M2L_WAKE
		omap_mux_init_signal(M2L_WAKE_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_136
		gpio_direction_output(M2L_WAKE_PIN,LOW_LEVEL);

		//gpio_139 LTE WAKEUP AP
		//set in set_wakeup_source

		//gpio_158 LTE RST (LTE--> AP)
		//set in set_wakeup_source
	}
}

void lte_gpio_resume_cfg(void)
{
	//LTE reset
	omap_mux_init_signal(LTE_nRST_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_21
	gpio_direction_output(LTE_nRST_PIN,LOW_LEVEL);

	//lte ps_hold
	//omap_mux_init_signal(LTE_PSHOLD_PIN_NAME,OMAP_PIN_INPUT_PULLDOWN);//gpio_23
	//gpio_direction_input(LTE_PSHOLD_PIN);

	//LTE go to sleep(LTE-->AP, unuse)
	omap_mux_init_signal(L2M_SLEEP_PIN_NAME,OMAP_PIN_INPUT_PULLDOWN);//gpio_137
	gpio_direction_input(L2M_SLEEP_PIN);

	//AP control lte to go to sleep(unuse)
	omap_mux_init_signal(M2L_SLEEP_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_138
	gpio_direction_output(M2L_SLEEP_PIN,LOW_LEVEL);

	//LTE WAKEUP AP
	omap_mux_init_signal(L2M_WAKE_PIN_NAME,OMAP_PIN_INPUT_PULLDOWN);//gpio_139
	gpio_direction_input(L2M_WAKE_PIN);

	//LTE USB SEL
	omap_mux_init_signal(LTE_USB_SEL_PIN_NAME, OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_150
	gpio_request(LTE_USB_SEL_PIN,"LTE_USB_SEL_PIN");
	gpio_direction_output(LTE_USB_SEL_PIN,LOW_LEVEL);

	//LTE RST (LTE--> AP)
	//omap_mux_init_signal(SOR_RST_N_PIN_NAME, OMAP_PIN_INPUT_PULLDOWN);//gpio_158
	//gpio_direction_input(SOR_RST_N_PIN);

	//LTE RVD1
	omap_mux_init_signal(LTE_RVD1_PIN_NAME,OMAP_PIN_INPUT_PULLDOWN);//gpio_159
	gpio_direction_input(LTE_RVD1_PIN);

	//LTE BOOT MODE
	omap_mux_init_signal(LTE_MODE0_SEL_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_170
	gpio_direction_output(LTE_MODE0_SEL_PIN,LOW_LEVEL);

	if(get_lte_power_status())
	{

		//lte pwr_en
		omap_mux_init_signal(LTE_PWR_EN_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_HIGH);//gpio_24
		gpio_direction_output(LTE_PWR_EN_PIN,HIGH_LEVEL);

		//lte state(rsv)
		omap_mux_init_signal(LTE_STATE_PIN_NAME,OMAP_PIN_INPUT);//gpio_114
		gpio_request(LTE_STATE_PIN,"LTE_STATE_PIN");
		gpio_direction_input(LTE_STATE_PIN);

		//M2L_WAKE
		omap_mux_init_signal(M2L_WAKE_PIN_NAME,OMAP_PIN_INPUT);//gpio_136
		gpio_direction_output(M2L_WAKE_PIN,HIGH_LEVEL);

	}
	else
	{
		//lte pwr_en
		omap_mux_init_signal(LTE_PWR_EN_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_24
		gpio_direction_output(LTE_PWR_EN_PIN, LOW_LEVEL);

		//lte state(rsv)
		omap_mux_init_signal(LTE_STATE_PIN_NAME,OMAP_PIN_INPUT_PULLDOWN);//gpio_114
		gpio_request(LTE_STATE_PIN,"LTE_STATE_PIN");
		gpio_direction_input(LTE_STATE_PIN);

		//M2L_WAKE
		omap_mux_init_signal(M2L_WAKE_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_136
		gpio_direction_output(M2L_WAKE_PIN,LOW_LEVEL);
	}
}

void via_gpio_suspend_cfg(void)
{
	int reg;

	omap_mux_init_signal("gpio_41",OMAP_PIN_INPUT);//C_VIA_STATE
	printk("suspend: via power state is %s\n", gpio_get_value(41)?"on":"off");

	//gpio_126_1 VIA_PWR_EN
	reg = omap_readl(0x48002a54);
	reg &= 0x0000ffff;
	reg |= (0x110c << 15);//input pulldown
	omap_writel(reg, 0x48002a54);

	if(smartphone_calling_enable == 0)
	{
		//gpio_127 AP_WAKEUP_VIA(L: sleep, H: wakeup)
		omap_mux_init_signal("gpio_127",OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN);
		gpio_direction_output(127, 0);
	}
	//gpio_128 connect to VIA, but unuse, config to input float
	omap_mux_init_signal("gpio_128",OMAP_PIN_INPUT);
	//gpio_129 VIA WAKEUP AP(H: wakeup)
	//omap_mux_init_signal("gpio_129",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_129 input pulldown

	//gpio_149 VIA RST
	omap_mux_init_signal("gpio_149",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(149, 0);

	//gpio_179 connect to VIA, but unuse, config to input float
	omap_mux_init_signal("gpio_179",OMAP_PIN_INPUT);
}

void via_gpio_resume_cfg(void)
{
	int reg;

//	omap_mux_init_signal("gpio_41",OMAP_PIN_INPUT);//C_VIA_STATE
//	printk("resume: via power state is %s\n", gpio_get_value(41)?"on":"off");

	//gpio_126_1 VIA_PWR_EN
	reg = omap_readl(0x48002a54);
	reg &= 0x0000ffff;
	reg |= (0x110c << 15);//input pulldown
	omap_writel(reg, 0x48002a54);

	//gpio_128 connect to VIA, but unuse, config to input float
	omap_mux_init_signal("gpio_128",OMAP_PIN_INPUT);
	//gpio_129 VIA WAKEUP AP(H: wakeup)
	//omap_mux_init_signal("gpio_129",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_129 input pulldown

	//gpio_149 VIA RST
	omap_mux_init_signal("gpio_149",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(149, 0);

	//gpio_179 connect to VIA, but unuse, config to input float
	omap_mux_init_signal("gpio_179",OMAP_PIN_INPUT);
}

static void lcd_gpio_suspend_cfg(void)
{
	omap_mux_init_signal("gpio_40", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//LCD_RST
	gpio_direction_output(40, 0);
	omap_mux_init_signal("gpio_66", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_67", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_68", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_69", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	
	omap_mux_init_signal("gpio_70", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_71", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_72", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_73", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_74", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_75", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_76", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_77", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_78", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_79", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_80", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_81", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_82", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_83", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_84", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_85", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_86", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_87", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_88", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_89", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_90", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_91", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_92", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("gpio_93", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);

	//SPI
	omap_mux_init_signal("gpio_171",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_171 mode4 input down
	omap_mux_init_signal("gpio_172",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_172 mode4 input down
	omap_mux_init_signal("gpio_173",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_173 mode4 input down
	omap_mux_init_signal("gpio_174", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_174 mode4 input down
}

//static void lcd_gpio_resume_cfg(void)
//{

//}

#if 0
static void sensor_gpio_suspend_cfg(void)
{
	if(smartphone_calling_enable == 1)
	{
		//gpio_156 ACC_EN
		omap_mux_init_signal("gpio_156",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_HIGH);
		gpio_direction_output(156, 1);
	}
	else
	{
		control_sensor_device_power(0);	
	}
}


static void sensor_gpio_resume_cfg(void)
{
	control_sensor_device_power(1);
}
#endif

static void audio_gpio_suspend_cfg(void)
{
	//gpio_22 C_PCM_EN, don't config

	/*connection speaker, codec, camera flashlight*/
	/*because three peripherals'power alway is on, i2c config to pullup*/
	omap_mux_init_signal("gpio_34",OMAP_PIN_INPUT);//gpio_34 AMP_IIC_SDA 
    omap_mux_init_signal("gpio_35",OMAP_PIN_INPUT);//gpio_35 AMP_IIC_SCL

	if(smartphone_calling_enable == 1)//suspend in calling, can't close audio device power
	{
		printk(KERN_DEBUG"PM!%s: suspend in calling\n", __func__);

		omap_writel(0x40, PRM_CLKSRC_CTRL);//CLKREQ is active
		omap_writel(0x80 , PRM_CLKOUT_CTRL);//Enable 26M clock work

		//gpio_10
		omap_mux_init_signal("sys_clkout1",OMAP_PIN_OUTPUT);

		//gpio_154 control FM2018 bypass(L:normal, H:bypass)
	
		//AUDIO_SCL
		omap_mux_init_signal("gpio_161",OMAP_PIN_INPUT | OMAP_PIN_INPUT_PULLUP);//gpio_161
		//AUDIO_SDA
		omap_mux_init_signal("gpio_162",OMAP_PIN_INPUT | OMAP_PIN_INPUT_PULLUP);//gpio_162

		//gpio_181 RST_FM2010
		omap_mux_init_signal("gpio_181",OMAP_PIN_INPUT);//gpio_181
	}
	else//normal suspend
	{
		//printk("prm_clksrc_ctrl=0x%8x, prm_clkout_ctrl=0x%8x\n", omap_readl(PRM_CLKSRC_CTRL), omap_readl(PRM_CLKOUT_CTRL));		
		omap_writel(0x48, PRM_CLKSRC_CTRL);//Disable 26M clock
		omap_writel(0x0 , PRM_CLKOUT_CTRL);

		//26M clock work
		omap_mux_init_signal("gpio_10",OMAP_PIN_OUTPUT  | OMAP_PIN_OFF_OUTPUT_LOW);//output low
		gpio_direction_output(10, 0);

		//gpio_154 control FM2018 bypass(L:normal, H:bypass)
		omap_mux_init_signal("gpio_154",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
		gpio_direction_output(154, 0);

		//AUDIO_SCL
		omap_mux_init_signal("gpio_161",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_161 output low
		gpio_direction_output(161, 0);
		//AUDIO_SDA
		omap_mux_init_signal("gpio_162",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_162 output low
		gpio_direction_output(162, 0);
		//Codec_CLK_nEN(L: enable, H: bypass)
		omap_mux_init_signal("gpio_176",OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP);
		gpio_direction_output(176, 1);
		//gpio_181 RST_FM2010
		omap_mux_init_signal("gpio_181",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
		gpio_direction_output(181, 0);	
	}	
}

static void audio_gpio_resume_cfg(void)
{
	//gpio_22 C_PCM_EN, don't config

	/*connection speaker, codec, camera flashlight*/
	/*because three peripherals'power alway is on, i2c config to pullup*/
	omap_mux_init_signal("gpio_34",OMAP_PIN_INPUT);//gpio_34 AMP_IIC_SDA 
    omap_mux_init_signal("gpio_35",OMAP_PIN_INPUT);//gpio_35 AMP_IIC_SCL

	//Codec_CLK_nEN(L: enable, H: bypass)
	omap_mux_init_signal("gpio_176",OMAP_PIN_OUTPUT);
	gpio_direction_output(176, 0);
}

void gpio_suspend_cfg(void)//add by huangjiefeng for test suspend
{
	u32 reg = 0;
	//gpio_0 twl5030 irq, don't need config
	//gpio_1 omap3630 26M CLK input
	
	//gpio_2-gpio_8 SYS_BOOT0-SYS_BOOT6, config to input float or don't config
	omap_mux_init_signal("gpio_2",OMAP_PIN_INPUT);//Input float
	omap_mux_init_signal("gpio_7",OMAP_PIN_INPUT);//KEY_CAM
	omap_mux_init_signal("gpio_8",OMAP_PIN_INPUT);//input float

	//gpio_9 sys_off_mode
	//gpio_10 for 26Mclk

	//gpio11 JTAG_EMU0
	omap_mux_init_signal("gpio_11",OMAP_PIN_INPUT);
	//gpio_12 WL_CLK 
	//gpio_13 WL_CMD

	omap_mux_init_signal("gpio_14", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_14 cam_scl
	omap_mux_init_signal("gpio_15", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_15 cam_sda

	omap_mux_init_signal("gpio_16", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_16 TW_nRST

	//gpio_17 WL_DAT3
	//gpio_18 WL_DAT0
	//gpio_19 WL_DAT1
	//gpio_20 WL_DAT2

	//gpio_21 LTE RESET
	//gpio_22 C_PCM_EN
	//gpio_23 PSHOLD(LTE POWER STATE)
	//gpio_24 LTE POWER EN
	//gpio_25 WL_WAKE
	//gpio_26 WL_EN
	//gpio_27 BT_EN

	omap_mux_init_signal("gpio_28",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_28 output USB1_SEL0

	//gpio_29 usb2_txse0 FOR VIA
	//gpio_30 nRESWARM

	//JTAG_EMU1
	omap_mux_init_signal("gpio_31",OMAP_PIN_INPUT);
	//no gpio_32 gpio_33

	//gpio_34 AMP_IIC_SDA /*connection speaker, codec, camera flashlight*/
    //gpio_35 AMP_IIC_SCL

	if(ac_charger_flag)
	{	
		omap_mux_init_signal("gpio_36",OMAP_PIN_INPUT_PULLUP);//CHG_EN1
		omap_mux_init_signal("gpio_37",OMAP_PIN_OUTPUT);//CHG_EN2
		gpio_direction_output(36, 1);
		gpio_direction_output(37, 1);
	}
	else if(usb_charger_flag)
	{
		omap_mux_init_signal("gpio_36",OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("gpio_37",OMAP_PIN_INPUT_PULLDOWN);
		gpio_direction_output(36, 0);
		gpio_direction_output(37, 0);
	}
	else//Stop charge
	{
		omap_mux_init_signal("gpio_36",OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("gpio_37",OMAP_PIN_INPUT_PULLDOWN);
		gpio_direction_output(36, 0);
		gpio_direction_output(37, 0);
	}
	
	//gpio_38 ACC_IRQ
	//gpio_39 COMPASS_IRQ	
	//gpio_40 LCD_RST
	//gpio_41 C_VIA_STATE

	//CHG_POK
	omap_mux_init_signal("gpio_42",OMAP_PIN_INPUT_PULLUP);
	//CHG_STATE, set charge full for wakeup source
	//omap_mux_init_signal("gpio_43",OMAP_PIN_INPUT_PULLUP | OMAP_WAKEUP_EN);
	
	//gpio_44-gpio_54 unused, they are addr signal, so don't need config. 
	//gpio_55-gpio_58 is iis signal, so don't need config.

	omap_mux_init_signal("gpio_59",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//unused
	gpio_direction_output(59, 0);

	//gpio_60,61, don't need config
	//gpio_62,63 mcp control signal, don't need config

	//unuse
	omap_mux_init_signal("gpio_64",OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP);//gpio_64 input up

	omap_mux_init_signal("gpio_65", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);// unused

	//gpio_66-gpio_93 is lcd sinal

	//gpio_94-gpio_97 camera control sinal
	omap_mux_init_signal("gpio_94",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//output low
	omap_mux_init_signal("gpio_95",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//output low
	omap_mux_init_signal("gpio_96",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//output low
	omap_mux_init_signal("gpio_97",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//output low

	//gpio_98 camera_rst
	omap_mux_init_signal("gpio_98",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(98, 0);

	omap_mux_init_signal("gpio_99", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//unused
	omap_mux_init_signal("gpio_100",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//unused
	
	//gpio_101-gpio_108 camera data line, and gpio_105-gpio_108 input pulldown, only input used
	omap_mux_init_signal("gpio_101",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(101, 0);
	omap_mux_init_signal("gpio_102",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(102, 0);
	omap_mux_init_signal("gpio_103",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(103, 0);
	omap_mux_init_signal("gpio_104",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(104, 0);
	omap_mux_init_signal("gpio_105", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
	omap_mux_init_signal("gpio_106",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
	omap_mux_init_signal("gpio_107",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
	omap_mux_init_signal("gpio_108",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);

	//Camera_EN
	omap_mux_init_signal("gpio_109",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(109, 0);
	//CAM2_PWDN
	omap_mux_init_signal("gpio_110",OMAP_PIN_OUTPUT  | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(110, 0);
	
	//gpio_111-gpio_113 unused
	omap_mux_init_signal("gpio_111",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
	omap_mux_init_signal("gpio_112",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
	omap_mux_init_signal("gpio_113",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);

	//gpio_114 ,lte_state

	//TW_nINT
	omap_mux_init_signal("gpio_115",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//input low

	//gpio_116-gpio_119 iis sinal, don't need config
	//gpio_120-gpio_125 usb and mmc1 sinal, don't need config

	//gpio_126_1 VIA_PWR_EN

	//gpio_126_2: cam_strobe
	reg = omap_readl(0x48002130);
	reg &= 0x0000ffff;
	reg |= (0x1108 << 15);//pulldown, can not config to gpio
	omap_writel(reg, 0x48002130);

	//gpio_127 AP_WAKEUP_VIA(L: sleep, H: wakeup)
	//gpio_128 connect to VIA, but unuse, config to input
	//gpio_129 VIA WAKEUP AP(H: wakeup)
	//gpio_130-gpio_135 LTE SDIO sinal
	//gpio_136 AP wakeup LTE
	//gpio_137 connect to LTE, but unuse
	//gpio_138 connect to LTE, but unuse
	//gpio_139 LTE wakeup ap
	//gpio_140-gpio_143 BT_PCM, don't need config
	//gpio_144-gpio_147 BT_UART

	//USB1_SEL1
	//omap_mux_init_signal("gpio_148",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);

	//gpio_149 VIA RST
	//gpio_150 LTE_USB_SEL
	//gpio_151 USB_SEL0, don't need config

	//flash light ctrl2
	omap_mux_init_signal("gpio_152",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(152, 0);
	//flash light ctrl1
	omap_mux_init_signal("gpio_153",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(153, 0);

	//gpio_154 control FM2018 bypass(L:normal, H:bypass)

	//flashlight en
	omap_mux_init_signal("gpio_155",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(155, 0);
	
	//gpio_156 ACC_EN

	//USB_SEL1
	if(get_console_suspend_status())
	{
		omap_mux_init_signal("gpio_151",OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP);
		gpio_direction_output(151, 1);

		omap_mux_init_signal("gpio_157",OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP);
		gpio_direction_output(157, 1);
	}
	else
	{
		omap_mux_init_signal("gpio_151",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
		gpio_direction_output(151, 0);

		omap_mux_init_signal("gpio_157",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
		gpio_direction_output(157, 0);
	}

	//gpio_158 LTE reset status
	//gpio_159 LTE RSV	
	//gpio_160 MCBSP CLK, don't need config
	//gpio_161 AUDIO_SCL
	//gpio_162 AUDIO_SDA

	omap_mux_init_signal("gpio_163",OMAP_PIN_INPUT);//gpio163 input float BAT_DECT

	//gpio164 ALS_PS_INT
	//gpio_165,166 debug uart

	//300M camera pwr_en
	omap_mux_init_signal("gpio_167",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(167, 0);

	//TW_I2C_SCL(I2C.2)
	omap_mux_init_signal("gpio_168", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
	
	//gpio_169 usb_dat3, don't need configs
	//gpio_170 lte boot mode
	//gpio_171-gpio_174 LCD SPI
	//gpio_175 Codec_nRST
	//gpio_176 Codec_CLK_nEN
	//gpio_177 USB2_TXDAT for VIA
	
	//gpio_178 TW_EN
	omap_mux_init_signal("gpio_178",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(178, 0);
	//gpio_179 connect to VIA, but unuse, config to input float

	//VIBRA_SYNC for Motor
	omap_mux_init_signal("gpio_180",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(180, 0);

	//gpio_181 RST_FM2010
	//gpio_182 LCD_PWM(gptmr8)

	//TW_I2C_SDA(I2C.2)
	omap_mux_init_signal("gpio_183", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);

	//gpio_184, ACC_SCL(I2C.3)
	//gpio_185, ACC_SDA(I2C.3)

	omap_mux_init_signal("gpio_186",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//unused
	gpio_direction_output(186, 0);

	//no gpio_187
	//gpio_188-gpio_191 USB DATA LINE, don't need config

	//((*(volatile unsigned int *)IO_ADDRESS(0x48002a00))) =  0x01070107;//config i2c4 to safe mode

	lte_gpio_suspend_cfg();
	via_gpio_suspend_cfg();
	lcd_gpio_suspend_cfg();
	//sensor_gpio_suspend_cfg();//poweroff sensor in lte_modem.c, because it often wakeup system if do it here
	audio_gpio_suspend_cfg();
	wifi_gpio_suspend();

	#ifdef CONFIG_YL_MODEM_N930	
	if(USB_SWITCH_CDMA == get_usb_switch_state())
	{		
		ModuleUsbChannelSelectCdma();//Modify by qgf for9930
	}
	else if(USB_SWITCH_LOCOST == get_usb_switch_state())
	{		
		ModuleUsbChannelSelectLocosto();//Modify by qgf for9930
	}	
	#endif /* CONFIG_YL_MODEM_N930 */
}

void gpio_resume_cfg(void)
{
	u32 reg;
	//gpio_0 twl5030 irq, don't need config
	//gpio_1 omap3630 26M CLK input
	//gpio_2-gpio_8 SYS_BOOT0-SYS_BOOT6, config to input float or don't config
	//gpio_9 sys_off_mode
	//gpio_10 for 26Mclk

	//gpio11 JTAG_EMU0
	omap_mux_init_signal("jtag_emu0",OMAP_PIN_INPUT);
	//gpio_12 WL_CLK 
	//gpio_13 WL_CMD

	//omap_mux_init_signal("gpio_14", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_14 cam_scl
	//omap_mux_init_signal("gpio_15", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_15 cam_sda
	//omap_mux_init_signal("gpio_16", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_16 TW_nRST

	//gpio_17 WL_DAT3
	//gpio_18 WL_DAT0
	//gpio_19 WL_DAT1
	//gpio_20 WL_DAT2

	//gpio_21 LTE RESET
	//gpio_22 C_PCM_EN
	//gpio_23 PSHOLD(LTE POWER STATE)
	//gpio_24 LTE POWER EN
	//gpio_25 WL_WAKE
	//gpio_26 WL_EN
	//gpio_27 BT_EN

	//omap_mux_init_signal("gpio_28",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_28 output USB1_SEL0

	omap_mux_init_signal("gpio_41",OMAP_PIN_INPUT);//C_VIA_STATE
	via_power_status = gpio_get_value(41);
	printk("resume: via power state is %s\n", via_power_status?"on":"off");
	 
	if(via_power_status)
	{
		//gpio_29 usb2_txse0 FOR VIA
		omap_mux_init_signal("etk_d15.mm2_txse0",OMAP_PIN_INPUT);
		//gpio_177 USB2_TXDAT for VIA
		omap_mux_init_signal("mcspi1_cs3.mm2_txdat",OMAP_PIN_INPUT);
	}

	//gpio_30 nRESWARM

	//JTAG_EMU1
	omap_mux_init_signal("jtag_emu1",OMAP_PIN_INPUT);//gpio_31
	//no gpio_32 gpio_33

	//gpio_34 AMP_IIC_SDA /*connection speaker, codec, camera flashlight*/
    //gpio_35 AMP_IIC_SCL

	omap_mux_init_signal("gpio_36",OMAP_PIN_OUTPUT);
	omap_mux_init_signal("gpio_37",OMAP_PIN_OUTPUT);
	
	//gpio_38 ACC_IRQ
	//gpio_39 COMPASS_IRQ	
	//gpio_40 LCD_RST
	//gpio_41 C_VIA_STATE

	//CHG_POK
	omap_mux_init_signal("gpio_42",OMAP_PIN_INPUT_PULLUP);	
	//CHG_STATE, set charge full for wakeup source
	omap_mux_init_signal("gpio_43",OMAP_PIN_INPUT_PULLUP | OMAP_WAKEUP_EN);
	
	//gpio_44-gpio_54 unused, they are addr signal, so don't need config. 
	//gpio_55-gpio_58 is iis signal, so don't need config.
	//gpio_60,61, don't need config
	//gpio_62,63 mcp control signal, don't need config

	//gpio_64 unused
	omap_mux_init_signal("gpio_64",OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP);//gpio_64 
	//gpio_66-gpio_93 is lcd sinal	
	//gpio_101-gpio_108 camera data line
	//gpio_105-gpio_108 input pulldown, only input used	
	//gpio_109 Camera_EN
	//gpio_110 CAM2_PWDN	
	//gpio_111-gpio_113 unused
	//gpio_114 ,lte_state
	//gpio_115 TW_nINT
	//gpio_116-gpio_119 iis sinal, don't need config
	//gpio_120-gpio_125 usb and mmc1 sinal, don't need config
	//gpio_126_1 VIA_PWR_EN

	//gpio_126_2: cam_strobe
	reg = omap_readl(0x48002130);
	reg &= 0x0000ffff;
	reg |= (0x1108 << 15);//pulldown, can not config to gpio
	omap_writel(reg, 0x48002130);

	//gpio_127 AP_WAKEUP_VIA(L: sleep, H: wakeup)
	//gpio_128 connect to VIA, but unuse, config to input
	//gpio_129 VIA WAKEUP AP(H: wakeup)
	//gpio_130-gpio_135 LTE SDIO sinal
	//gpio_136 AP wakeup LTE
	//gpio_137 connect to LTE, but unuse
	//gpio_138 connect to LTE, but unuse
	//gpio_139 LTE wakeup ap
	//gpio_140-gpio_143 BT_PCM, don't need config
	//gpio_144-gpio_147 BT_UART

	//USB1_SEL1
	//omap_mux_init_signal("gpio_148",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);

	//gpio_149 VIA RST
	//gpio_150 LTE_USB_SEL
	//gpio_151 USB_SEL0, 
	omap_mux_init_signal("gpio_151",OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP);
	gpio_direction_output(151, 1);

	//gpio_152 flash light ctrl2
	//gpio_153 flash light ctrl1	
	//gpio_154 control FM2018 bypass(L:normal, H:bypass)
	//gpio_155 flashlight en
	//gpio_156 ACC_EN

	//USB_SEL1
	omap_mux_init_signal("gpio_157",OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP);
	gpio_direction_output(157, 1);

	//gpio_158 LTE reset status
	//gpio_159 LTE RSV	
	//gpio_160 MCBSP CLK, don't need config
	//gpio_161 AUDIO_SCL
	//gpio_162 AUDIO_SDA

	omap_mux_init_signal("gpio_163",OMAP_PIN_INPUT);//gpio163 input float BAT_DECT

	//gpio164 ALS_PS_INT
	//gpio_165,166 debug uart

	//gpio_167 300M camera pwr_en

	//TW_I2C_SCL(I2C.2)
	//omap_mux_init_signal("i2c2_scl",OMAP_PIN_INPUT);//gpio_168
	
	//gpio_169 usb_dat3, don't need configs
	//gpio_170 lte boot mode
	//gpio_171-gpio_174 LCD SPI
	//gpio_175 Codec_nRST
	//gpio_176 Codec_CLK_nEN
	//gpio_177 USB2_TXDAT for VIA
	//gpio_178 TW_EN
	//gpio_179 connect to VIA, but unuse, config to input float
	//gpio_180 VIBRA_SYNC for Motor
	//gpio_181 RST_FM2010
	//gpio_182 LCD_PWM(gptmr8)

	//TW_I2C_SDA(I2C.2)
	//omap_mux_init_signal("i2c2_sda",OMAP_PIN_INPUT);//gpio_183

	//gpio_184, ACC_SCL(I2C.3)
	//gpio_185, ACC_SDA(I2C.3)
	//gpio_186, unused
	//no gpio_187
	//gpio_188-gpio_191 USB DATA LINE, don't need config
	
	if(smartphone_calling_enable == 1)
	{	
		#ifdef CONFIG_SWITCH_GPIO
#if 0
        if(get_headset_plug_status()==0)//sunruichen modify 2011.05.31
        {
            printk("PM!%s: enable headset amp \n", __func__);
            gpio_direction_output(GPIO_97, 1);  // enable headset amp
            //gpio_direction_output(GPIO_6, 0); // enable headset mic
        }
        else
        {
        	printk("PM!%s: enable earpiece \n", __func__);
        	omap_mux_init_signal("gpio_6",OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLUP);
        	gpio_direction_output(GPIO_6, 1); 
        }
#endif
        #endif
	}

	lte_gpio_resume_cfg();
	via_gpio_resume_cfg();
	//sensor_gpio_resume_cfg();//poweron sensor in lte_modem.c
	audio_gpio_resume_cfg();
	wifi_gpio_resume();
	
	//((*(volatile unsigned int *)IO_ADDRESS(0x48002a00))) =  0x01180118;//restore i2c4 mode 
}


void set_smartphone_wakeup_source(void)
{
	u32 reg = 0;

	//charge full wakeup source
	omap_mux_init_signal("gpio_43",OMAP_PIN_INPUT_PULLUP);
	reg = omap_readl(0x4800208c);
	reg |= (1 << 14);
	omap_writel(reg, 0x4800208c);//gpio_43

	if(smartphone_calling_enable == 1)
	{
		//set KEY_ACK for wakeup source
		
		//set ALS_PS for wakeup source
		omap_mux_init_signal("gpio_164",OMAP_PIN_INPUT | OMAP_WAKEUP_EN);//gpio164 ALS_PS_INT
	}
	else
	{
		//disable KEY_ACK for wakeup source

		//disable ALS_PS for wakeup source
		omap_mux_init_signal("gpio_164",OMAP_PIN_INPUT_PULLDOWN);//gpio164 ALS_PS_INT
	}

	/*added by xiao for multicomplex cts to gpio waken 2010-9-2*/
	if(bt_is_on())
	{
		omap_mux_init_signal("gpio_144",OMAP_WAKEUP_EN | OMAP_PIN_INPUT_PULLUP);
	}
    //add by lizilai
    if(wifi_is_on())
	{
		    printk(KERN_DEBUG"wifi is on!\n");
			omap_mux_init_signal("gpio_25", OMAP_PIN_INPUT_PULLUP | OMAP_WAKEUP_EN );//gpio_25	
	}  
	else
	{
	        omap_mux_init_signal("gpio_25", OMAP_PIN_INPUT);
	}	 
    //end
	if(get_lte_power_status())
	{
		//M2L_WAKE gpio_136
		reg = omap_readl(0x48002164);
		reg &= 0xFFFF0000;
		reg |= 0x11F;
		omap_writel(reg, 0x48002164);//GPIO_136 have a down pulse when wakeup, so config to mode7 before suspend for workaround

		//LTE WAKEUP AP(L2M_WAKE)
		omap_mux_init_signal(L2M_WAKE_PIN_NAME,OMAP_PIN_INPUT | OMAP_WAKEUP_EN);//gpio_139
		gpio_direction_input(L2M_WAKE_PIN);

		//LTE RST (LTE--> AP)
		omap_mux_init_signal(SOR_RST_N_PIN_NAME, OMAP_PIN_INPUT | OMAP_WAKEUP_EN);//gpio_158
		gpio_direction_input(SOR_RST_N_PIN);
	}
	else
	{
		omap_mux_init_signal(M2L_WAKE_PIN_NAME,OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_136
		gpio_direction_output(M2L_WAKE_PIN,LOW_LEVEL);

		//LTE WAKEUP AP(L2M_WAKE)
		omap_mux_init_signal(L2M_WAKE_PIN_NAME,OMAP_PIN_INPUT_PULLDOWN);//gpio_139
		gpio_direction_input(L2M_WAKE_PIN);

		//LTE RST (LTE--> AP)
		omap_mux_init_signal(SOR_RST_N_PIN_NAME, OMAP_PIN_INPUT_PULLDOWN);//gpio_158
		gpio_direction_input(SOR_RST_N_PIN);
	}
}


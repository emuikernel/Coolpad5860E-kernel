/*
 * arch/arm/mach-omap2/misc.c
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free dispware
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */  

#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <mach/gpio.h>
#include <plat/misc.h>
#include "mux.h"
#include <linux/i2c/twl.h>/*for i2c add by xiao*/
#include <linux/delay.h>

#define BT_EN_GPIO 27

/*added by xiao for wl1271 power mamager 2010-5-6*/
struct wl1271_state_t wl1271_state;
EXPORT_SYMBOL(wl1271_state);

void wifi_gpio_suspend(void)
{
	if(wl1271_is_power_off())
	{
		//wl1271 gpio config
		printk("enter wifi_gpio_suspend by xiao\n");

		/*wlan sdio pin*/
		omap_mux_init_signal("gpio_12", OMAP_PIN_INPUT);	
		omap_mux_init_signal("gpio_13", OMAP_PIN_INPUT);
		omap_mux_init_signal("gpio_17", OMAP_PIN_INPUT);
		omap_mux_init_signal("gpio_18", OMAP_PIN_INPUT);
		omap_mux_init_signal("gpio_19", OMAP_PIN_INPUT);
		omap_mux_init_signal("gpio_20", OMAP_PIN_INPUT);

		/*wifi wake pin*/
		omap_mux_init_signal("gpio_25", OMAP_PIN_INPUT);

		/*wifi,bt,fm enable gpio pin*/
		omap_mux_init_signal("gpio_26", OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("gpio_27", OMAP_PIN_INPUT_PULLDOWN);
		gpio_direction_output(27, 0);
	}
	else
	{
		/*added by xiao by 2010-8-4*/
		if(wifi_is_on())
		{
			omap_mux_init_signal("gpio_26",OMAP_PIN_OUTPUT);//gpio_26
			printk(KERN_DEBUG"PM!%s: wifi on\n", __func__);
		}
		if(bt_is_on())
		{
			omap_mux_init_signal("gpio_27",OMAP_PIN_OUTPUT);//gpio_26
			printk(KERN_DEBUG"PM!%s: bt on\n", __func__);
			/*added by xiao for multicomplex cts to gpio waken 2010-9-2*/
			omap_mux_init_signal("gpio_144",OMAP_WAKEUP_EN | OMAP_PIN_INPUT_PULLUP);
		}
	}
}
EXPORT_SYMBOL(wifi_gpio_suspend);
void wifi_gpio_resume(void)
{
	if(wl1271_is_power_off())
	{
		wifi_gpio_suspend();
	}
	else
	{
		//1271 wifi bt gpio cfg xiaoxiao
		printk("enter wifi_gpio_resume by xiao\n");
		/*wl1271 sdio interface*/
		omap_mux_init_signal("etk_clk.sdmmc3_clk", OMAP_PIN_INPUT_PULLUP);
		omap_mux_init_signal("etk_ctl.sdmmc3_cmd", OMAP_PIN_INPUT_PULLUP);//added by xiao
		omap_mux_init_signal("etk_d4.sdmmc3_dat0", OMAP_PIN_INPUT_PULLUP);
		omap_mux_init_signal("etk_d5.sdmmc3_dat1", OMAP_PIN_INPUT_PULLUP);
		omap_mux_init_signal("etk_d6.sdmmc3_dat2", OMAP_PIN_INPUT_PULLUP);
		omap_mux_init_signal("etk_d3.sdmmc3_dat3", OMAP_PIN_INPUT_PULLUP);

		/* WLAN PW_EN and IRQ */
		omap_mux_init_signal("gpio_25", OMAP_PIN_INPUT);
		omap_mux_init_signal("gpio_26", OMAP_PIN_OUTPUT);
		omap_mux_init_signal("gpio_27", OMAP_PIN_OUTPUT);

		/*bt uart interface*/
		omap_mux_init_signal("uart2_cts.uart2_cts", OMAP_PIN_INPUT_PULLUP);
		omap_mux_init_signal("uart2_rts.uart2_rts", OMAP_PIN_OUTPUT);
		omap_mux_init_signal("uart2_tx.uart2_tx", OMAP_PIN_OUTPUT);
		omap_mux_init_signal("uart2_rx.uart2_rx", OMAP_PIN_INPUT_PULLUP);
		printk("exit wifi_gpio_resume by xiao\n");
	}
}
EXPORT_SYMBOL(wifi_gpio_resume);
/**********************added by xiao 2010-4-10***************************/
/*
*	disable/enable ldo power pin output
*     by xiaoxiangyun, 2010-4-1
*/
void power_wl1271(int on)
{
	int ret=0;
	printk("WL12XX:enter %s(%d)\n", __func__, on);
#ifdef CONFIG_WL12XX_LDO
#define WIFI_LDO_PM_MASTER 0x0e
#define WIFI_LDO_PM_RECEIVER 0x1f
	ret=twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, WIFI_LDO_PM_MASTER);
	if(ret)
	{
	 	printk("power_wl1271 error by xiao1\n");
		return;
	}
	ret=twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, WIFI_LDO_PM_MASTER);
	if(ret)
	{
	 	printk("power_wl1271 error by xiao2\n");
		return;
	}
#endif
	if(on)  // power on wl1271
	{
#ifdef CONFIG_W112XX_LDO
		ret=twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,0xe0,WIFI_LDO_PM_RECEIVER);//added by xiao
		if(ret)
		{
		 	printk("power_wl1271 error by xiao3\n");
			return;
		}
#endif
		wl1271_state.WIFI_1V8=1;
		mdelay(20);
	}
	else   //power off wl1271
	{
#ifdef CONFIG_WL12XX_LDO
		ret=twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,0x00,WIFI_LDO_PM_RECEIVER);//added by xiao
		if(ret)
		{
		 	printk("power_wl1271 error by xiao3\n");
			return;
		}
#endif
		wl1271_state.WIFI_1V8=0;
	}
#ifdef CONFIG_WL12XX_LDO

	ret=twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, WIFI_LDO_PM_MASTER);
	if(ret)
	{
	 	printk("power_wl1271 error by xiao4\n");
		return;
	}
#endif
	return;
}
EXPORT_SYMBOL(power_wl1271);
/**********************added by xiao 2010-4-10***************************/

/*   2010-5-11
*  return value: 0---->wl1271 power on; 1----->wl1271 power off.
*/
int wl1271_is_power_off(void)
{
    if(wl1271_state.wifi_state || wl1271_state.bt_state)
    {
		return 0;
    }
	else
    {
		return 1;
    }
}
EXPORT_SYMBOL(wl1271_is_power_off);   
/******added by xiao 2010-8-4******/
int wifi_is_on(void)
{
    if(wl1271_state.wifi_state)
    {
		return 1;
    }
	else
    {
		return 0;
    }
}
int bt_is_on(void)
{
	if(wl1271_state.bt_state)
    {
		return 1;
    }
	else
    {
		return 0;
    }
}
/******added by xiao 2010-8-4 end******/                                                                    

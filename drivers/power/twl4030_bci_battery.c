/*
 * linux/drivers/power/twl4030_bci_battery.c
 *
 * OMAP2430/3430 BCI battery driver for Linux
 *
 * Copyright (C) 2008 Texas Instruments, Inc.
 * Author: Texas Instruments, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/rtc.h>//打印时间函数
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <linux/power_supply.h>
#include <linux/i2c/twl4030-madc.h>
#include <linux/usb/otg.h>
#include <linux/irq.h>
#include <plat/mux.h>
#include <plat/mux_omap.h>
#include <plat/cpu_ddr_version.h>
#include <plat/misc.h>
#include <plat/yl_debug.h>
#include <mach/gpio.h>
#include "twl4030_bci_reg.h"

/* Ptr to thermistor table */
int *therm_tbl;

int usb_charger_flag = 0;
static int LVL_1, LVL_2, LVL_3, LVL_4;
int charger_plug_flag = 0;
int ac_charger_flag = 0;
int bat_volt_low_flag = 0;
extern void wifi_bt_power_on(int on);
extern charger_status g_charger_type;
static int bci_charging_current;
static int check_modem_network = 0;
static int charge_state_irq;
static int charge_over_volt_irq;
int vbat_det_irq;
extern bool battery_charge_full;
extern unsigned long before_suspend_times;
extern unsigned long after_wakeup_times;
struct twl4030_bci_device_info *g_pbattery_info=NULL;
void twl4030_power_suspend_group(u8 base_addr, u8 sleep_state, u8 type);
int rm_chg_from_suspend = 0;
extern int pre_battery_volt;

//test suspend off
#define PRINT_PMIC_POWER_STATE 1

#define DEVGROUP_OFFSET		0
#define TYPE_OFFSET		    1
#define REMAP_OFFSET		2
#define GROUP_P3            (4 << 5)
static u8 twl4030_power_addrs[] = {
	[RES_VAUX1]	= 0x17,
	[RES_VAUX2]	= 0x1b,
	[RES_VAUX3]	= 0x1f,
	[RES_VAUX4]	= 0x23,
	[RES_VMMC1]	= 0x27,
	[RES_VMMC2]	= 0x2b,
	[RES_VPLL1]	= 0x2f,
	[RES_VPLL2]	= 0x33,
	[RES_VSIM]	= 0x37,
	[RES_VDAC]	= 0x3b,
	[RES_VINTANA1]	= 0x3f,
	[RES_VINTANA2]	= 0x43,
	[RES_VINTDIG]	= 0x47,
	[RES_VIO]	= 0x4b,
	[RES_VDD1]	= 0x55,
	[RES_VDD2]	= 0x63,
	[RES_VUSB_1V5]	= 0x71,
	[RES_VUSB_1V8]	= 0x74,
	[RES_VUSB_3V1]	= 0x77,
	[RES_VUSBCP]	= 0x7a,
	[RES_REGEN]	= 0x7f,
	[RES_NRES_PWRON] = 0x82,
	[RES_CLKEN]	= 0x85,
	[RES_SYSEN]	= 0x88,
	[RES_HFCLKOUT]	= 0x8b,
	[RES_32KCLKOUT]	= 0x8e,
	[RES_RESET]	= 0x91,
	[RES_Main_Ref]	= 0x94,
};


#ifdef PRINT_PMIC_POWER_STATE
void print_pmic_power_state(void)
{
	u8 group = 0;
	u8 type = 0;
	u8 remap = 0;
	u8 dedicate = 0;
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

	//vpll1
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x2f);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x30);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x31);//vsim DEV_GRP belong to P1 P2 P3
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &dedicate, 0x32);//vdac DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"vpll1 group=0x%x, type=0x%x, remap=0x%x, dedicate=0x%x", group, type, remap, dedicate);

	//vinana1
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x3f);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x40);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x41);//vsim DEV_GRP belong to P1 P2 P3
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &dedicate, 0x42);//vdac DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"vinana1 group=0x%x, type=0x%x, remap=0x%x, dedicate=0x%x", group, type, remap, dedicate);

	//vinana1
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x47);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x48);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x49);//vsim DEV_GRP belong to P1 P2 P3
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &dedicate, 0x4a);//vdac DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"vintdig group=0x%x, type=0x%x, remap=0x%x, dedicate=0x%x", group, type, remap, dedicate);

	//vio
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x4b);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x4c);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x4d);//vsim DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"vio group=0x%x, type=0x%x, remap=0x%x", group, type, remap);

	//regen
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x7f);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x80);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x81);//vsim DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"regen group=0x%x, type=0x%x, remap=0x%x", group, type, remap);

	//nres
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x82);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x83);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x84);//vsim DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"nres group=0x%x, type=0x%x, remap=0x%x", group, type, remap);

	//clken
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x85);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x86);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x87);//vsim DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"clken group=0x%x, type=0x%x, remap=0x%x", group, type, remap);

	//sysen
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x88);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x89);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x8a);//vsim DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"sysen group=0x%x, type=0x%x, remap=0x%x", group, type, remap);

	//HFCLKOUT
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x8B);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x8C);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x8D);//vsim DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"HFCLKOUT group=0x%x, type=0x%x, remap=0x%x", group, type, remap);

	//32KHFCLKOUT
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x8D);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x8F);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x90);//vsim DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"32KHFCLKOUT group=0x%x, type=0x%x, remap=0x%x", group, type, remap);

	//TRITON
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x91);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x92);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x93);//vsim DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"TRITON group=0x%x, type=0x%x, remap=0x%x", group, type, remap);

	//MAINREF
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &group, 0x94);//vdac 1.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &type, 0x95);//vsim 2.8v
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, 0x96);//vsim DEV_GRP belong to P1 P2 P3
	printk(KERN_ERR"MAINREF group=0x%x, type=0x%x, remap=0x%x", group, type, remap);


	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);
}
#endif


static unsigned long get_local_rtc_time(void)
{
	struct rtc_time tm;
	struct timespec ts;
	unsigned long temp;
	
	getnstimeofday(&ts);
	temp = ts.tv_sec;
	ts.tv_sec += 3600*8;
	rtc_time_to_tm(ts.tv_sec, &tm);
	/*printk(KERN_ERR"(Date: %d-%02d-%02d  Time: %02d:%02d:%02d.%09lu)\r\n",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);*/

	//modified for prevent output log during taking photo or video recording----by liqinghua
	yl_bat_debug(LOG_DEBUG,"(Date: %d-%02d-%02d  Time: %02d:%02d:%02d.%09lu)\r\n\n",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);

	return temp;
}

void enable_all_ldo(void)
{
	yl_bat_debug(LOG_DEBUG,"huangjiefeng: test enable ldo======================================\r\n");
	
	//10??a?¤?é?¨LDO
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x04, 0x3A);//vsim 2.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x37);//vsim DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x03, 0x3e);//vdac 1.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x3b);//vdac DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x03, 0x32);//vpll1 1.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x2f);//vpll1 DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x36);//vpll2 1.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x33);//vpll2 DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x03, 0x1A);//vaux1 2.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x17);//vaux1 DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x1e);//vaux2 1.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x1B);//vaux2 DEV_GRP belong to P1

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x01, 0x22);//vaux3 1.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x1f);//vaux3 DEV_GRP belong to P1

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x26);//vaux4 1.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x23);//vaux4 DEV_GRP belong to P1 P2 P3
	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x02, 0x2A);//VMMC1 3v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x27);//VMMC1 DEV_GRP belong to P1 P2 P3
	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x2E);//VMMC2 2.8v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x2B);//VMMC1 DEV_GRP belong to P1 P2 P3
	
	//7??a???é?¨LDO
	//twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x0B, 0x42);//VINTANA1 ?????ˉ????¨??????aèˉ?	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x3F);//VINTANA1 DEV_GRP belong to P1 P2 P3
	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x01, 0x46);//VINTANA2 2.75v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x43);//VINTANA2 DEV_GRP belong to P1 P2 P3
	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x04, 0x4A);//VINTDIG 1.5v
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x47);//VINTDIG DEV_GRP belong to P1 P2 P3
	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x71);//VINTUSB1V5 DEV_GRP belong to P1 P2 P3	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x74);//VINTUSB1V8 DEV_GRP belong to P1 P2 P3
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x77);//VINTUSB1P8 DEV_GRP belong to P1 P2 P3

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);

}


static irqreturn_t charge_full_interrupt(int irq, void *_di)
{
	struct twl4030_bci_device_info *di = _di;
	
	yl_bat_debug(LOG_DEBUG,"BAT!%s: charge_full_interrupt\r\n", __func__);

	schedule_delayed_work(&di->charge_full_handle, msecs_to_jiffies(1000));//1000ms
	return IRQ_HANDLED;
}

static irqreturn_t charger_over_volt_interrupt(int irq, void *data)
{
	yl_bat_debug(LOG_DEBUG,"BAT!%s: charger_over_volt_interrupt\r\n", __func__);
	//system_stop_charge();
	return IRQ_HANDLED;
}

static irqreturn_t det_bat_status_interrupt(int irq, void *_di)
{
	printk(KERN_ERR"BAT!%s: battery insert or remove\r\n", __func__);

	detect_battery_status();

	return IRQ_HANDLED;
}

/*
 * Return battery capacity
 * Or < 0 on failure.
 */
#define  INTERVAL_NUMBER     11
int BattVolThresholds[] = {4157, 4029, 3947,3874, 3819, 3765, 3739, 3713, 3683, 3650, 3620, 3570};
int battery_percent[] =   {100,  90,   80,  70,   60,   50,   40,   30,   20,  10,   2,   0 };
//int BattVolThresholds[] = {4157, 4029, 3947,3874, 3819, 3765, 3739, 3713, 3683, 3636, 3570};
//int BattVolThresholds[] = {4160, 4059, 3967,3892, 3825, 3770, 3735, 3711, 3680, 3630, 3500};
//int battery_percent[] =   {100,  90,   80,  70,   60,   50,   40,   30,   20,   10,   0 };
//int BattVolThresholds[] = {4150,4126,4091,4050,4008,3973,3944,3909,3874,3844,3827,3809,3797,3786,3774,3762,3745,3727,3703,3650,3500};
//int battery_percent[] =   {100, 95,  90,  85,  80,  75,  70,  65,  60,  55,  50,  45,  40,  35,  30,  25,  20,  15,  10,  5,   0,  };
//#define  INTERVAL_NUMBER     ((sizeof(BattVolThresholds)/sizeof(int))-1)

static int twl4030battery_capacity(struct twl4030_bci_device_info *di)
{
	int i = 0;
	int percent = 0;
	static int pre_percent = 0;
	static int j = 0;
	int ret = 0;
	u8 val = 0;
	static bool is_boot = true;
	static int count = 0;
	int dif_per = 0;
	int rtc_store_val = 0;

#if 0
/***********************************************************************
close_gsm_modem_volt < 8% //volt range v < 3602
close_cdma_modem_volt < 5% // volt range v < 3552
reboot_modem_volt > 14% // volt range v > 3650

v<3605, <15% //remind user charge
************************************************************************/
	ret = ((di->voltage_uV - SMARTPHONE_MIN_VOLT) * 100)/(SMARTPHONE_MAX_VOLT - SMARTPHONE_MIN_VOLT);
	if(ret < 0)
		ret = 0;

	if(ret > 100)
		ret = 100;

	if(ret == 100)
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
	else if((ret < 100) && (charger_plug_flag))//battery full and later battery volt decrease
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
#endif

    if((di->voltage_uV >= BattVolThresholds[0]) || (battery_charge_full && (di->voltage_uV >= 4100)))
    {
        percent = 100;
		if(twl4030_identify_insert_device() == charger_unplug)//sometimes lost interrupt
		{
			charger_plug_flag = 0;
			ac_charger_flag = 0;
			usb_charger_flag = 0;
			battery_charge_full = false;
			printk(KERN_ERR"BAT!%s: lost usb remove interrupt\n", __func__);
		}
        printk(KERN_WARNING"The voltage of battery is full\r\n");
    }
    else if(di->voltage_uV <= BattVolThresholds[INTERVAL_NUMBER])
    {
        percent = 0;
		battery_charge_full = false;
        printk(KERN_WARNING"The voltage of battery come to shutoff smartphone\r\n");
    }
    else
    {
        for(i=0; i<INTERVAL_NUMBER; i++)
        {
            if((di->voltage_uV < BattVolThresholds[i]) && (di->voltage_uV >= BattVolThresholds[i+1]))
            {
				percent = (di->voltage_uV - BattVolThresholds[i+1])* (battery_percent[i]-battery_percent[i+1])/(BattVolThresholds[i] - BattVolThresholds[i+1]);
				percent += battery_percent[i+1];
                break;
            }
        }
		battery_charge_full = false;

		if(percent < 2)
		{
			if((di->voltage_uV < 3620) && (di->voltage_uV >= 3595))
			{
				percent = 2;
			}
			else if((di->voltage_uV < 3595) && (di->voltage_uV > 3570))
			{
				percent = 1;
			}
		}
    }

	if(is_boot)
	{
		count++;
		if(count == 3)
		{
			is_boot = false;
			twl_i2c_read_u8(TWL4030_MODULE_BACKUP,&val,BACKUP_REG_D);//read the saved value of battery when power on.
			rtc_store_val = val;
			if(rtc_store_val == 110)//battery has been charged before power on
			{
				yl_bat_debug(LOG_DEBUG,"BAT! The battery has been charged before power on!\r\n");
				rtc_store_val = 0;
			}
			else
			{
				dif_per = rtc_store_val - percent;
				yl_bat_debug(LOG_DEBUG,"BAT!read voltage from rtc backup. rtc_store_val = %d  dif_per = %d\r\n",rtc_store_val,dif_per);
			}

			if((dif_per > -10) && (dif_per < 10) && (rtc_store_val != 0))
			{
				percent = rtc_store_val;
				if(percent == 1)
				{
					pre_battery_volt = 3590;
					di->voltage_uV = 3590;
				}
				else if(percent == 2)
				{
					pre_battery_volt = 3620;
					di->voltage_uV = 3620;
				}
				else if(percent == 100)
				{
					pre_battery_volt = 4157;
					di->voltage_uV = 4157;
				}
				else
				{
					for(i=0;i<10;i++)
					{
						if((percent < battery_percent[i]) && (percent >= battery_percent[i+1]))
						{
							pre_battery_volt = BattVolThresholds[i+1] + (((percent - battery_percent[i+1])* (BattVolThresholds[i] - BattVolThresholds[i+1]))/(battery_percent[i] - battery_percent[i+1])) + 2;//convert the percent to valtage.
							di->voltage_uV = pre_battery_volt;
							break;
						}
					}
				}
				yl_bat_debug(LOG_DEBUG,"BAT! restore from rtc backup.  pre_battery_volt = %d\r\n",pre_battery_volt);
			}
		}
	}

	if(percent != pre_percent)//If the battery changed, save the value
	{
		val = percent;
		ret = twl_i2c_write_u8(TWL4030_MODULE_BACKUP,val,BACKUP_REG_D);
		yl_bat_debug(LOG_DEBUG,"percent != pre_percent. write volt to rtc backup. val = %d  ret = %d \r\n",val,ret);
	}

	if(charger_plug_flag)
	{
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;	
		if((percent < pre_percent) && (j < 10))
		{
			percent = pre_percent;
			j++;
		}
		else
		{
			j = 0;
		}
	}
	else
	{
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
		j = 0;
		if((percent > pre_percent) && (pre_percent < 15) && (pre_percent > 0))//14%, picture is red
		{
			percent = pre_percent;
		}
	}
	
	pre_percent = percent;
	/*printk("BAT!%s: Percent = %d, battery_volt=%d, %s\r\n", __func__, percent, di->voltage_uV, (charger_plug_flag?"charger_in":"charger_out"));*/
	yl_bat_debug(LOG_DEBUG,"BAT!%s: Percent = %d, battery_volt=%d, %s\r\n", __func__, percent, di->voltage_uV, (charger_plug_flag?"charger_in":"charger_out"));
	get_local_rtc_time();
	return percent;
}

/*
 * Report and clear the charger presence event.
 */
static inline int twl4030charger_presence_evt(struct twl4030_bci_device_info *_di)
{
	int insert_device = 0;
	u8 set = 0, clear = 0;
	struct twl4030_bci_device_info *di = _di;

	insert_device = twl4030_identify_insert_device();//判断插入或拔出的是U盘还是charger 
	if (insert_device == usb_client_device)//u盘或鼠标
	{
		yl_bat_debug(LOG_DEBUG,"BAT!%s: USBHOST!! \n", __func__);
		set = CHG_PRES_FALLING;
		clear = CHG_PRES_RISING;
		
		usb_device_insert_done();		
	}
	else if (insert_device == charger_plug) //charger
	{ /* If the AC charger have been connected */
		/* configuring falling edge detection for CHG_PRES */
		set = CHG_PRES_FALLING;
		clear = CHG_PRES_RISING;
		
		insert_device = g_charger_type;//twl4030_identify_charge_device();//hey delete 10-8-26
		if(insert_device == ac_charge)
		{
			charger_plug_flag = 1;
			ac_charger_flag = 1;
			usb_charger_flag = 0;
			setup_charge_current(AC_CHARGE_DEVICE, AC_CHARGE_CURRENT);
		}
		else if(insert_device == usb_charge)
		{
			charger_plug_flag = 1;
			usb_charger_flag = 1;
			ac_charger_flag = 0;
			setup_charge_current(USB_CHARGE_DEVICE, USB_CHARGE_CURRENT);
		}
		else
		{
			charger_plug_flag = 0;
			ac_charger_flag = 0;
			usb_charger_flag = 0;
			printk(KERN_WARNING"BAT!%s: Charger plug/unplug error warning\r\n", __func__);
		}

		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
	} 
	else //拔出设备
	{ /* If the AC charger have been disconnected */
		/* configuring rising edge detection for CHG_PRES */
		set = CHG_PRES_RISING;
		clear = CHG_PRES_FALLING;
		charger_plug_flag = 0;
		ac_charger_flag = 0;
		set_charge_mode_to_usb();
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
	}

	/* Update the interrupt edge detection register */
	device_insert_interrupt_done(clear, set);
	
	return 0;
}

/*
 * Interrupt service routine
 *
 * Attends to TWL 4030 power module interruptions events, specifically
 * USB_PRES (USB charger presence) CHG_PRES (AC charger presence) events
 *
 */
static irqreturn_t twl4030charger_interrupt(int irq, void *_di)
{
	struct twl4030_bci_device_info *di = _di;

#ifdef CONFIG_LOCKDEP
	/* WORKAROUND for lockdep forcing IRQF_DISABLED on us, which
	 * we don't want and can't tolerate.  Although it might be
	 * friendlier not to borrow this thread context...
	 */
	local_irq_enable();
#endif
	yl_bat_debug(LOG_DEBUG,"BAT!%s: charger plug/unplug interrupt\r\n", __func__);
	twl4030charger_presence_evt(di);
	power_supply_changed(&di->bat);//notify baterry volt

	return IRQ_HANDLED;
}

#ifdef CONFIG_YL_MODEM_N930
extern void ModuleUsbChannelDelete(void);
#endif
extern bool from_sleep_to_wakeup;
void charge_interrupt_done(int insert_device)
{
	struct twl4030_bci_device_info *di;

#ifdef CONFIG_LOCKDEP
	/* WORKAROUND for lockdep forcing IRQF_DISABLED on us, which
	 * we don't want and can't tolerate.  Although it might be
	 * friendlier not to borrow this thread context...
	 */
	local_irq_enable();
#endif
    if(g_pbattery_info == NULL)
      return;

    di= g_pbattery_info;
	yl_bat_debug(LOG_DEBUG,"BAT!%s: Enter.\r\n", __func__);

	//twl4030charger_presence_evt(di);
	if(insert_device == ac_charge)
	{
		charger_plug_flag = 1;
		ac_charger_flag = 1;
		usb_charger_flag = 0;
		setup_charge_current(AC_CHARGE_DEVICE, AC_CHARGE_CURRENT);	
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
	}
	else if(insert_device == usb_charge)
	{
		charger_plug_flag = 1;
		usb_charger_flag = 1;
		ac_charger_flag = 0;
		msleep(150);//add delay for avoiding usb inrush current test fail, huangjiefeng in 20120103
		setup_charge_current(USB_CHARGE_DEVICE, USB_CHARGE_CURRENT);
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
	}
	else //拔出设备
	{ 
		charger_plug_flag = 0;
		ac_charger_flag = 0;
		usb_charger_flag = 0;
		battery_charge_full = false;
		set_charge_mode_to_usb();
		//system_stop_charge();//stop charge for avoiding usb inrush current test fail, huangjiefeng in 20120103
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
#ifdef CONFIG_YL_MODEM_N930
		if(USB_SWITCH_AP != get_usb_switch_state())
		{
			ModuleUsbChannelDelete();
		}
#endif
	}
	
	if (from_sleep_to_wakeup)
	{
		rm_chg_from_suspend = 1;
		msleep(500);
	}
	cancel_delayed_work(&di->twl4030_bci_monitor_work);
	schedule_delayed_work(&di->twl4030_bci_monitor_work, 2*HZ);

	power_supply_changed(&di->bat);//notify baterry volt
}

/*
 * This function handles the twl4030 battery presence interrupt
 */
static int twl4030battery_presence_evt(void)
{
	int ret;
	u8 uninitialized_var(batstsmchg), uninitialized_var(batstspchg);

	/* check for the battery presence in main charge*/
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
			&batstsmchg, REG_BCIMFSTS3);
	if (ret)
		return ret;

	/* check for the battery presence in precharge */
	ret = twl_i2c_read_u8(TWL4030_MODULE_PRECHARGE,
			&batstspchg, REG_BCIMFSTS1);
	if (ret)
		return ret;

	/*
	 * REVISIT: Physically inserting/removing the batt
	 * does not seem to generate an int on 3430ES2 SDP.
	 */
	if ((batstspchg & BATSTSPCHG) || (batstsmchg & BATSTSMCHG)) {
		/* In case of the battery insertion event */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, BATSTS_EDRRISIN,
			BATSTS_EDRFALLING, REG_BCIEDR2);
		if (ret)
			return ret;
	} else {
		/* In case of the battery removal event */
		ret = clear_n_set(TWL4030_MODULE_INTERRUPTS, BATSTS_EDRFALLING,
			BATSTS_EDRRISIN, REG_BCIEDR2);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * This function handles the twl4030 battery voltage level interrupt.
 */
static int twl4030battery_level_evt(void)
{
	int ret;
	u8 uninitialized_var(mfst);

	/* checking for threshold event */
	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
			&mfst, REG_BCIMFSTS2);
	if (ret)
		return ret;

	/* REVISIT could use a bitmap */
	if (mfst & VBATOV4) {
		LVL_4 = 1;
		LVL_3 = 0;
		LVL_2 = 0;
		LVL_1 = 0;
	} else if (mfst & VBATOV3) {
		LVL_4 = 0;
		LVL_3 = 1;
		LVL_2 = 0;
		LVL_1 = 0;
	} else if (mfst & VBATOV2) {
		LVL_4 = 0;
		LVL_3 = 0;
		LVL_2 = 1;
		LVL_1 = 0;
	} else {
		LVL_4 = 0;
		LVL_3 = 0;
		LVL_2 = 0;
		LVL_1 = 1;
	}

	return 0;
}

/*
 * Interrupt service routine
 *
 * Attends to BCI interruptions events,
 * specifically BATSTS (battery connection and removal)
 * VBATOV (main battery voltage threshold) events
 *
 */
static irqreturn_t twl4030battery_interrupt(int irq, void *_di)
{
	u8 uninitialized_var(isr1a_val), uninitialized_var(isr2a_val);
	u8 clear_2a, clear_1a;
	int ret;

#ifdef CONFIG_LOCKDEP
	/* WORKAROUND for lockdep forcing IRQF_DISABLED on us, which
	 * we don't want and can't tolerate.  Although it might be
	 * friendlier not to borrow this thread context...
	 */
	local_irq_enable();
#endif
	yl_debug("battery evt interrupt\r\n");
	ret = twl_i2c_read_u8(TWL4030_MODULE_INTERRUPTS, &isr1a_val,
				REG_BCIISR1A);
	if (ret)
		return IRQ_NONE;

	ret = twl_i2c_read_u8(TWL4030_MODULE_INTERRUPTS, &isr2a_val,
				REG_BCIISR2A);
	if (ret)
		return IRQ_NONE;

	clear_2a = (isr2a_val & VBATLVL_ISR1) ? (VBATLVL_ISR1) : 0;
	clear_1a = (isr1a_val & BATSTS_ISR1) ? (BATSTS_ISR1) : 0;
	clear_1a |= ((isr1a_val & ICHGEOC_ISR1) ? (ICHGEOC_ISR1) : 0);

	/* cleaning BCI interrupt status flags */
	ret = twl_i2c_write_u8(TWL4030_MODULE_INTERRUPTS,
			clear_1a , REG_BCIISR1A);
	if (ret)
		return IRQ_NONE;

	ret = twl_i2c_write_u8(TWL4030_MODULE_INTERRUPTS,
			clear_2a , REG_BCIISR2A);
	if (ret)
		return IRQ_NONE;

	/* battery connetion or removal event */
	if (isr1a_val & BATSTS_ISR1)
		twl4030battery_presence_evt();
	/* battery voltage threshold event*/
	else if (isr2a_val & VBATLVL_ISR1)
		twl4030battery_level_evt();
	/*电池充电满*/
	else if (isr1a_val & ICHGEOC_ISR1)
	{
		yl_debug("Battery EOC interrupt\r\n");
	}
	else
	{
		printk(KERN_WARNING"BAT!%s: Battery not support other interrupt\r\n", __func__);
		return IRQ_NONE;
	}

	return IRQ_HANDLED;
}

#if 0
/*
 * Returns the main charge FSM status
 * Or < 0 on failure.
 */
static int twl4030bci_status(void)
{
	int ret;
	u8 uninitialized_var(status);

	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
		&status, REG_BCIMSTATEC);
	if (ret) {
		pr_err("BAT!%s: twl4030_bci: error reading BCIMSTATEC\n", __func__);
		return ret;
	}

	return (int) (status & BCIMSTAT_MASK);
}
#endif
static int twl4030battery_charger_event(struct notifier_block *nb,
		unsigned long event, void *_data)
{
	twl4030charger_usb_en(event == USB_EVENT_VBUS);
	return 0;
}

static enum power_supply_property twl4030_bci_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW,
};

static enum power_supply_property twl4030_bk_bci_battery_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static enum power_supply_property twl4030_usb_battery_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property twl4030_ac_battery_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static void
twl4030_bk_bci_battery_read_status(struct twl4030_bci_device_info *di)
{
	di->bk_voltage_uV = twl4030backupbatt_voltage();
}

static void twl4030_bk_bci_battery_work(struct work_struct *work)
{
	struct twl4030_bci_device_info *di = container_of(work,
		struct twl4030_bci_device_info,
		twl4030_bk_bci_monitor_work.work);

	twl4030_bk_bci_battery_read_status(di);
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, 60*60*HZ);//1 hour
}

static void twl4030_bci_battery_read_status(struct twl4030_bci_device_info *di)
{
	di->temp_C = twl4030battery_temperature();
	di->voltage_uV = twl4030_get_battery_voltage();//twl4030battery_voltage();

	if(di->voltage_uV < 3580)
		bat_volt_low_flag = 1;
	else
		bat_volt_low_flag = 0;

	di->current_uA = twl4030battery_current();
	di->capacity = twl4030battery_capacity(di);

	//get_local_rtc_time();
}

static void twl4030_bci_battery_update_status(struct twl4030_bci_device_info *di)
{
	int old_capacity = di->capacity;
	int old_charge_status = di->charge_status;

	twl4030_bci_battery_read_status(di);

	if((old_capacity != di->capacity) || (old_charge_status != di->charge_status) || (get_battery_status()==false) || get_timer_wakeup_status())
	{
		yl_bat_debug(LOG_DEBUG, "BAT!%s: notify battery status\n", __func__);
		power_supply_changed(&di->bat);//notify baterry volt and charger status
	}
#if 0

	int old_charge_source = di->charge_rsoc;
	int old_charge_status = di->charge_status;
	int old_capacity = di->capacity;
	static int stable_count;

	twl4030_bci_battery_read_status(di);
	di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;
	if (power_supply_am_i_supplied(&di->bat))
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
	else
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;

	/*
	 * Read the current usb_charger_flag
	 * compare this with the value from the last
	 * update cycle to determine if there was a
	 * change.
	 */
	di->charge_rsoc = usb_charger_flag;

	/*
	 * Battery voltage fluctuates, when we are on a threshold
	 * level, we do not want to keep informing user of the
	 * capacity fluctuations, or he the battery progress will
	 * keep moving.
	 */
	if (old_capacity != di->capacity) {
		stable_count = 0;
	} else {
		stable_count++;
	}
	/*
	 * Send uevent to user space to notify
	 * of some battery specific events.
	 * Ac plugged in, USB plugged in and Capacity
	 * level changed.
	 * called every 100 jiffies = 0.7 seconds
	 * 20 stable cycles means capacity did not change
	 * in the last 15 seconds.
	 */
	if ((old_charge_status != di->charge_status)
			|| (stable_count == 20)
			|| (old_charge_source !=  di->charge_rsoc)) {
		power_supply_changed(&di->bat);
	}
#endif
}

extern void print_usb_reg(void);
static void twl4030_bci_battery_work(struct work_struct *work)
{
	struct twl4030_bci_device_info *di = container_of(work,
		struct twl4030_bci_device_info, twl4030_bci_monitor_work.work);

	if(get_timer_wakeup_status())
	{
		check_modem_network = 2;
	}

#ifdef PRINT_PMIC_POWER_STATE
	if(omap_clock_test == 134)
	{	
		print_pmic_power_state();
	}
#endif

	twl4030_bci_battery_update_status(di);

//	if ((omap_clock_test != CHAGNGE_NOTIFY_BATTERY_TIME) && !bat_volt_low_flag)
	if (omap_clock_test != CHAGNGE_NOTIFY_BATTERY_TIME)
	{
		if(charger_plug_flag)
		{
			schedule_delayed_work(&di->twl4030_bci_monitor_work, 60*HZ);
		}
		else
		{
			schedule_delayed_work(&di->twl4030_bci_monitor_work, 5*HZ);
		}
	}
	/*else if ((omap_clock_test != CHAGNGE_NOTIFY_BATTERY_TIME) && (bat_volt_low_flag == 1))
	{
		if(charger_plug_flag)
		{
			schedule_delayed_work(&di->twl4030_bci_monitor_work, 10*HZ);
		}
		else
		{
			schedule_delayed_work(&di->twl4030_bci_monitor_work, 5*HZ);
		}
	}*/
	else
		schedule_delayed_work(&di->twl4030_bci_monitor_work, 5*HZ);//test	
}


#define to_twl4030_bci_device_info(x) container_of((x), \
			struct twl4030_bci_device_info, bat);

static void twl4030_bci_battery_external_power_changed(struct power_supply *psy)
{
	struct twl4030_bci_device_info *di = to_twl4030_bci_device_info(psy);

	cancel_delayed_work(&di->twl4030_bci_monitor_work);
	schedule_delayed_work(&di->twl4030_bci_monitor_work, 0);
}

#define to_twl4030_bk_bci_device_info(x) container_of((x), \
		struct twl4030_bci_device_info, bk_bat);

static int twl4030_bk_bci_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct twl4030_bci_device_info *di = to_twl4030_bk_bci_device_info(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = di->bk_voltage_uV;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int twl4030_usb_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = usb_charger_flag;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int twl4030_ac_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = ac_charger_flag;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int twl4030_bci_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct twl4030_bci_device_info *di;

	di = to_twl4030_bci_device_info(psy);

	//printk("psp=%d\r\n",psp);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = di->charge_status;
		return 0;
	default:
		break;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = di->voltage_uV;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = di->current_uA;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = di->temp_C;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
#if 0
		status = twl4030bci_status();
		if ((status & AC_STATEC) == AC_STATEC)
			val->intval = POWER_SUPPLY_TYPE_MAINS;
		else if (usb_charger_flag)
			val->intval = POWER_SUPPLY_TYPE_USB;
		else
			val->intval = 0;
#endif
		if (ac_charger_flag)
			val->intval = POWER_SUPPLY_TYPE_MAINS;
		else if (usb_charger_flag)
			val->intval = POWER_SUPPLY_TYPE_USB;
		else
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		/*
		 * need to get the correct percentage value per the
		 * battery characteristics. Approx values for now.
		 */
		yl_bat_debug(LOG_DEBUG,"BAT!%s: POWER_SUPPLY_PROP_CAPACITY\r\n", __func__);
		val->intval = twl4030battery_capacity(di);
		break;

	case POWER_SUPPLY_PROP_PRESENT:
		yl_bat_debug(LOG_DEBUG,"BAT!%s: check battery status\n", __func__);
		if(get_battery_status() == false)
			val->intval = 0;
		else
			val->intval = 1;
		break;

	case POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW://added for check modem sign after suspend for 1h
		yl_bat_debug(LOG_DEBUG,"BAT!%s: check modem sign 1h %d\n", __func__, get_timer_wakeup_status());
		if(get_timer_wakeup_status() == 0)
		{
			val->intval = 0;
			check_modem_network = 0;
		}
		else
		{
			val->intval = 1;
			check_modem_network--;
			if(check_modem_network < 0)
				check_modem_network = 0;
		}	
		
		if(check_modem_network == 0)
			set_timer_wakeup_flag(0);//clear flag
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static char *twl4030_bci_supplied_to[] = {
	"twl4030_bci_battery",
};

extern int battery_init_finish;
extern spinlock_t bat_lock;
extern void usb_reinit(void);
static int __init twl4030_bci_battery_probe(struct platform_device *pdev)
{
	struct twl4030_bci_platform_data *pdata = pdev->dev.platform_data;
	struct twl4030_bci_device_info *di;
	int ret;
	int charge_device;
	int irq;

	therm_tbl = pdata->battery_tmp_tbl;
	//bci_charging_current = pdata->twl4030_bci_charging_current;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	di->bat.name = "battery";//"twl4030_bci_battery";
	di->bat.supplied_to = twl4030_bci_supplied_to;
	di->bat.num_supplicants = ARRAY_SIZE(twl4030_bci_supplied_to);
	di->bat.type = POWER_SUPPLY_TYPE_BATTERY;
	di->bat.properties = twl4030_bci_battery_props;
	di->bat.num_properties = ARRAY_SIZE(twl4030_bci_battery_props);
	di->bat.get_property = twl4030_bci_battery_get_property;
	di->bat.external_power_changed =
			twl4030_bci_battery_external_power_changed;

	di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;

	di->bk_bat.name = "twl4030_bci_bk_battery";
	di->bk_bat.type = POWER_SUPPLY_TYPE_BATTERY;
	di->bk_bat.properties = twl4030_bk_bci_battery_props;
	di->bk_bat.num_properties = ARRAY_SIZE(twl4030_bk_bci_battery_props);
	di->bk_bat.get_property = twl4030_bk_bci_battery_get_property;
	di->bk_bat.external_power_changed = NULL;

	/*
	 * Android expects a battery type POWER_SUPPLY_TYPE_USB
	 * as a usb charger battery. This battery
	 * and its "online" property are used to determine if the
	 * usb cable is plugged in or not.
	 */
	di->usb_bat.name = "usb";//yeruiquan.modify name according to system read file
	di->usb_bat.supplied_to = twl4030_bci_supplied_to;
	di->usb_bat.type = POWER_SUPPLY_TYPE_USB;
	di->usb_bat.properties = twl4030_usb_battery_props;
	di->usb_bat.num_properties = ARRAY_SIZE(twl4030_usb_battery_props);
	di->usb_bat.get_property = twl4030_usb_battery_get_property;
	di->usb_bat.external_power_changed = NULL;

	di->ac_bat.name = "mains";
	di->ac_bat.supplied_to = twl4030_bci_supplied_to;
	di->ac_bat.type = POWER_SUPPLY_TYPE_MAINS;
	di->ac_bat.properties = twl4030_ac_battery_props;
	di->ac_bat.num_properties = ARRAY_SIZE(twl4030_ac_battery_props);
	di->ac_bat.get_property = twl4030_ac_battery_get_property;
	di->ac_bat.external_power_changed = NULL;


	ret = twl4030_hardware_init();
	if (ret)
	{
		goto hardware_init_fail;
	}

	if(twl4030_identify_insert_device() == charger_plug)
	{
		charge_device = g_charger_type;//twl4030_identify_charge_device();//hey delete 10-8-26
		if(twl4030_battery_presence_identify())//系统有电池
		{
			if (charge_device == ac_charge)
			{
				setup_charge_current(AC_CHARGE_DEVICE, AC_CHARGE_CURRENT);				
				usb_charger_flag = 0;
				ac_charger_flag = 1;
			}
			else if (charge_device == usb_charge)
			{
				setup_charge_current(USB_CHARGE_DEVICE, USB_CHARGE_CURRENT);
				usb_charger_flag = 1;
				ac_charger_flag = 0;
			}
		}
		else//系统没有电池，使用充电器供电
		{
			enter_cv_mode(charge_device);
		}
		charger_plug_flag = 1;
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
	}
	else
	{
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
	}

	//twl4030charger_usb_en(ENABLE);
	//twl4030battery_hw_level_en(ENABLE);
	//twl4030battery_hw_presence_en(ENABLE);

	platform_set_drvdata(pdev, di);

	/* REVISIT do we need to request both IRQs ?? */
	if (g_mid_hardware_version != P0_BOARD)
	{
		gpio_request(CHG_STATE_PIN, "charge_sts_irq");
		gpio_direction_input(CHG_STATE_PIN);
		charge_state_irq = gpio_to_irq(CHG_STATE_PIN);//把GPIO转换成物理中断号
		ret = request_irq(charge_state_irq, charge_full_interrupt, /*IRQ_TYPE_EDGE_RISING*/IRQ_TYPE_EDGE_BOTH, "charge_state", di);//Register irq, 上升沿中断
		if(ret)
		{
			printk(KERN_ERR"BAT!%s: twl4030_bci_battery_prob: fail to install charge full irq\r\n", __func__);
			return ret;
		}

		gpio_request(CHG_OVER_PIN, "charge_over_irq");
		gpio_direction_input(CHG_OVER_PIN);
		charge_over_volt_irq = gpio_to_irq(CHG_OVER_PIN);//把GPIO转换成物理中断号
		ret = request_irq(charge_over_volt_irq, charger_over_volt_interrupt, IRQ_TYPE_EDGE_RISING, "charge_OVER_VOLT", NULL);//Register irq, 上升沿中断
		if(ret)
		{
			printk(KERN_ERR"BAT!%s: twl4030_bci_battery_prob: fail to install charge full irq\r\n", __func__);
			return ret;
		}

		ret = gpio_request(VBAT_DET_PIN, "det_bat_irq");
		if(ret)
		{
			printk(KERN_ERR"BAT!%s: twl4030_bci_battery_prob: request vbat_det_pin fail\r\n", __func__);
		}
		gpio_direction_input(VBAT_DET_PIN);
		vbat_det_irq = gpio_to_irq(VBAT_DET_PIN);//把GPIO转换成物理中断号
		ret = request_irq(vbat_det_irq, det_bat_status_interrupt, IRQ_TYPE_EDGE_FALLING, "det_bat_status", NULL);//detect battery insert/remove
		if(ret)
		{
			printk(KERN_ERR"BAT!%s: twl4030_bci_battery_prob: fail to install det_bat_status_interrupt\r\n", __func__);
			return ret;
		}
	}
	
	/* request BCI interruption */
	irq = platform_get_irq(pdev, 1);
	ret = request_irq(irq, twl4030battery_interrupt,
		0, pdev->name, NULL);
	if (ret) {
		dev_dbg(&pdev->dev, "could not request irq %d, status %d\n",
			irq, ret);
		goto batt_irq_fail;
	}


	/* request Power interruption */
	irq = platform_get_irq(pdev, 0);
	ret = request_irq(irq, twl4030charger_interrupt,
		0, pdev->name, di);

	if (ret) {
		dev_dbg(&pdev->dev, "could not request irq %d, status %d\n",
			irq, ret);
		goto chg_irq_fail;
	}

	ret = power_supply_register(&pdev->dev, &di->bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register main battery\n");
		goto batt_failed;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bci_monitor_work,
				twl4030_bci_battery_work);
	schedule_delayed_work(&di->twl4030_bci_monitor_work, 0);

	ret = power_supply_register(&pdev->dev, &di->bk_bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register backup battery\n");
		goto bk_batt_failed;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&di->twl4030_bk_bci_monitor_work,
				twl4030_bk_bci_battery_work);
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, 500);

	ret = power_supply_register(&pdev->dev, &di->usb_bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register usb battery\n");
		goto usb_batt_failed;
	}

	ret = power_supply_register(&pdev->dev, &di->ac_bat);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register ac battery\n");
		goto usb_batt_failed;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&di->charge_full_handle, charge_full_done);

	di->nb.notifier_call = twl4030battery_charger_event;
	ret = otg_register_notifier(otg_get_transceiver(), &di->nb);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to register usb battery\n");
		goto otg_notify_failed;
	}

	g_pbattery_info = di;
	battery_init_finish = 1;

	return 0;

otg_notify_failed:
	power_supply_unregister(&di->usb_bat);
usb_batt_failed:
	power_supply_unregister(&di->bk_bat);
bk_batt_failed:
	power_supply_unregister(&di->bat);
batt_failed:
	free_irq(irq, di);
chg_irq_fail:
	irq = platform_get_irq(pdev, 1);
	free_irq(irq, NULL);
batt_irq_fail:
hardware_init_fail:
	twl4030charger_ac_en(DISABLE,charger_unplug);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);
	kfree(di);

	return ret;
}

static int __exit twl4030_bci_battery_remove(struct platform_device *pdev)
{
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);
	int irq;

	otg_unregister_notifier(otg_get_transceiver(), &di->nb);
	twl4030charger_ac_en(DISABLE, charger_unplug);
	twl4030charger_usb_en(DISABLE);
	twl4030battery_hw_level_en(DISABLE);
	twl4030battery_hw_presence_en(DISABLE);

	irq = platform_get_irq(pdev, 0);
	free_irq(irq, di);

	irq = platform_get_irq(pdev, 1);
	free_irq(irq, NULL);

	free_irq(charge_state_irq, NULL);
	free_irq(charge_over_volt_irq, NULL);

	flush_scheduled_work();
	power_supply_unregister(&di->bat);
	power_supply_unregister(&di->bk_bat);
	platform_set_drvdata(pdev, NULL);
	kfree(di);

	return 0;
}

void set_core_volt_to_retention(void)
{
	u32 reg;

	//set vdd1 to retention
	reg = omap_readl(PRM_VC_CMD_VAL_0);
	reg &= ~(0xff << 8);
	reg |= (0x13 << 8);
	omap_writel(reg, PRM_VC_CMD_VAL_0);

	//set vdd2 to retention	
	reg = omap_readl(PRM_VC_CMD_VAL_1);
	reg &= ~(0xff << 8);
	reg |= (0x13 << 8);
	omap_writel(reg, PRM_VC_CMD_VAL_1);
/*
	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &tmp, VDD1_VMODE_CFG);
	tmp |= (1 << 2);
	tmp &= ~1;
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, tmp, VDD1_VMODE_CFG);

	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &tmp, VDD2_VMODE_CFG);
	tmp |= (1 << 2);
	tmp &= ~1;
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, tmp, VDD2_VMODE_CFG);
*/
}


void twl5030_gpio_suspend(void)
{
	u8 data;

	twl_i2c_read_u8(TWL4030_MODULE_GPIO, &data, 0x1c);
	//data |= (1<<2);
	data |= ((1<<1)|(1<<2)|(1<<6));
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, data, 0x1c);

	twl_i2c_read_u8(TWL4030_MODULE_GPIO, &data, 0x22);
	//data |= (1<<2);
	data |= ((1<<1)|(1<<2)|(1<<6));
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, data, 0x22);
}


void twl5030_gpio_resume(void)
{
	u8 data;
	twl_i2c_read_u8(TWL4030_MODULE_GPIO, &data, 0x1c);
	//data &= (~(1<<2));
	data &= (~((1<<1)|(1<<2)|(1<<6)));
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, data, 0x1c);

	twl_i2c_read_u8(TWL4030_MODULE_GPIO, &data, 0x22);
	//data &= (~(1<<2));
	data &= (~((1<<1)|(1<<2)|(1<<6)));
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, data, 0x22);
}


void twl4030_power_suspend_group(u8 base_addr, u8 sleep_state, u8 type)
{
	u8 remap;
	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, GROUP_P3, (base_addr+DEVGROUP_OFFSET)); //DEV_GRP belong to P3

	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &remap, (base_addr+REMAP_OFFSET));
	remap &= ~(0x0F);
	remap |= sleep_state;
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, remap, (base_addr+REMAP_OFFSET));
	if(0 != type)
	{
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, type, (base_addr+TYPE_OFFSET));
	}
}

#ifdef CONFIG_PM
static int twl4030_bci_battery_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);

	disable_irq(charge_over_volt_irq);

#ifdef PRINT_PMIC_POWER_STATE
	if(omap_clock_test == 135)
	{	
		print_pmic_power_state();
	}
#endif
	//disable_irq(charge_state_irq);

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x00), 0x3d);//vdac
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x08), 0x3c);//vdac type
	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x08), 0x39);//vsim remap
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x10), 0x38);//vsim type

	//if use gpio105-gpio108 for gpio, and gpio need work, vpll2 powerdomain can not go to off
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x10, 0x34);//vpll2 type
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x35);//vpll2 remap

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);

	//di->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;
	cancel_delayed_work(&di->twl4030_bci_monitor_work);
	cancel_delayed_work(&di->twl4030_bk_bci_monitor_work);

	if(smartphone_calling_enable == 1)
	{
		twl5030_gpio_resume();
		//power on vaux4(audio) power domain
		twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
		twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x26);//vaux4 1.8v for audio
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, P1_P2_P3_ENABLE, 0x23);//vaux4 DEV_GRP belong to P1 P2 P3

		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x08, 0x25);//vaux4 1.8v in suspend for audio

		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x7<<5), 0x43);//vintana2 type
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x08), 0x45);//vintana2 remap
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x10), 0x44);//vintana2 type

		twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);
	}
	else
	{
		twl5030_gpio_suspend();

		//power off vaux4(audio) power domain
		twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
		twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x26);//vaux4 1.8v for audio
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0, 0x23);//vaux4 DEV_GRP belong to P1 P2 P3

		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x25);//vaux4 0v in suspedn for audio

		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x00), 0x45);//vintana2 remap
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x10), 0x44);//vintana2 type
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x00), 0x43);//vintana2 GROUP off

		twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);						
	}

	before_suspend_times = get_local_rtc_time();

	return 0;
}


static int twl4030_bci_battery_resume(struct platform_device *pdev)
{
	struct twl4030_bci_device_info *di = platform_get_drvdata(pdev);

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, (0x7<<5), 0x43);//vintana2 type
	enable_irq(charge_over_volt_irq);
#ifdef PRINT_PMIC_POWER_STATE
	if(omap_clock_test == 136)
	{	
		print_pmic_power_state();
	}
#endif
	
	//enable_irq(charge_state_irq);
    if(smartphone_calling_enable == 0)
	{
    	twl5030_gpio_resume();
    }
	after_wakeup_times = get_local_rtc_time();
	//twl4030_charge_madc_setup();//open MADC clock in resume
	schedule_delayed_work(&di->twl4030_bci_monitor_work, 0);
	schedule_delayed_work(&di->twl4030_bk_bci_monitor_work, 50);

	return 0;
}
#else
#define twl4030_bci_battery_suspend	NULL
#define twl4030_bci_battery_resume	NULL
#endif /* CONFIG_PM */


static struct platform_driver twl4030_bci_battery_driver = {
	.probe		= twl4030_bci_battery_probe,
	.remove		= __exit_p(twl4030_bci_battery_remove),
	.suspend	= twl4030_bci_battery_suspend,
	.resume		= twl4030_bci_battery_resume,
	.driver		= {
		.name	= "twl4030_bci",
		.owner  = THIS_MODULE,
	},
};

MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:twl4030_bci");
MODULE_AUTHOR("Texas Instruments Inc");

static int __init twl4030_battery_init(void)
{
	return platform_driver_register(&twl4030_bci_battery_driver);
}
module_init(twl4030_battery_init);

static void __exit twl4030_battery_exit(void)
{
	free_irq(charge_state_irq, NULL);
	free_irq(charge_over_volt_irq, NULL);
	platform_driver_unregister(&twl4030_bci_battery_driver);
}
module_exit(twl4030_battery_exit);


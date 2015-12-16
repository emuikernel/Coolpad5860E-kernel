/*
 * linux/drivers/i2c/chips/twl4030_poweroff.c
 *
 * Power off device
 *
 * Copyright (C) 2008 Nokia Corporation
 *
 * Written by Peter De Schrijver <peter.de-schrijver@nokia.com>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/i2c/twl.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <plat/gpio.h>
#include <mach/mux.h>
#include <plat/misc.h>
#include <plat/yl_debug.h>

#define PWR_P1_SW_EVENTS	0x10
#define PWR_P2_SW_EVENTS    0x11
#define PWR_P3_SW_EVENTS    0x12
#define PWR_DEVOFF	        (1<<0)

#define STARTON_VBUS       		(1 << 5)
#define STARTON_VBAT       		(1 << 4)
#define STARTON_RTC             (1 << 3)
#define STARTON_USB        		(1 << 2)
#define STARTON_CHG        		(1 << 1)
#define PWR_STOPON_POWERON (1<<6)

#define CFG_P123_TRANSITION	0x03
#define SEQ_OFFSYNC	(1<<0)
#define STARTON_PWON            (1 << 0)
#define TWL_4030_INT_PWR_EDR1    0x05
#define TWL_4030_INT_PWR_EDR2    0x06
#define TWL_4030_INT_PWR_IMR1    0x01
#define TWL_4030_INT_PWR_IMR2    0x03

#define PHY_TO_OFF_PM_MASTER(p)              (p - 0x36)
#define R_P1_SW_EVENTS                   PHY_TO_OFF_PM_MASTER(0x46)
#define R_P2_SW_EVENTS                   PHY_TO_OFF_PM_MASTER(0x47)
#define R_P3_SW_EVENTS                   PHY_TO_OFF_PM_MASTER(0x48)

#define CHARGER_INSERT             1

extern void set_alarm_reboot(void);
extern int twl4030_identify_insert_device(void);
extern bool user_set_poweroff_alarm(struct rtc_wkalrm *user_alarm);
extern int yl_params_kernel_write(const char *buf,size_t count);
extern void wm9093_PowerDown( void );

static void setnoreboot(void)
{
	u8 val;
	int err;

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

	//Set CFG_P1_TRANSITION
	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val, 0x00);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while reading TWL4030 CFG_P1_TRANSITION\n", err);
		return;
	}

	val |= (STARTON_PWON | STARTON_VBUS | STARTON_USB | STARTON_CHG | STARTON_RTC);
	val &= (~STARTON_VBAT);	

	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val, 0x00);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while writing TWL4030 CFG_P1_TRANSITION\n", err);
		return;
	}

	//Set CFG_P2_TRANSITION
	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val, 0x01);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while reading TWL4030 CFG_P2_TRANSITION\n", err);
		return;
	}

	val |= (STARTON_PWON | STARTON_VBUS | STARTON_USB | STARTON_CHG | STARTON_RTC);
	val &= (~STARTON_VBAT);

	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val, 0x01);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while writing TWL4030 CFG_P2_TRANSITION\n", err);
		return;
	}

	//Set CFG_P3_TRANSITION
	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val, 0x02);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while reading TWL4030 CFG_P3_TRANSITION\n", err);
		return;
	}

	val |= (STARTON_PWON | STARTON_VBUS | STARTON_USB | STARTON_CHG | STARTON_RTC);
	val &= (~STARTON_VBAT);

	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val, 0x02);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while writing TWL4030 CFG_P3_TRANSITIONn", err);
		return;
	}

	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);

	return;
}

void save_user_alarm_to_nandflash(struct rtc_wkalrm *user_alarm)
{
	int ret;
	char rtc_buf[60] = "ALM_TIME";

	if(NULL == user_alarm)
	{
		printk(KERN_ERR"%s user_alarm pointer is null\n", __func__);
		return;
	}

	memcpy(&rtc_buf[10], (char *)user_alarm, sizeof(struct rtc_wkalrm));
	ret = yl_params_kernel_write(rtc_buf, sizeof(rtc_buf));
	if(ret <= 0)
	{
		printk(KERN_WARNING"%s: write user alarm to nandflash fail, once again\n", __func__);
		ret = yl_params_kernel_write(rtc_buf,sizeof(rtc_buf));
		if(ret <= 0)
		{
			printk(KERN_ERR"%s: write user alarm to nandflash fail!\n", __func__);
		}
	}

	return;
}

void poweroff_device(void)
{
	u8 reg = 0;
	omap_mux_init_signal("gpio_2",OMAP_PIN_INPUT);//gpio_2 Input float

	omap_mux_init_signal("gpio_7",OMAP_PIN_INPUT);//gpio_7 top panel key, input float and not pulldown in suspend
	
	omap_mux_init_signal("gpio_8",OMAP_PIN_INPUT);//input float

	omap_mux_init_signal("gpio_11",OMAP_PIN_INPUT);//gpio11 input float

	omap_mux_init_signal("gpio_15",OMAP_PIN_INPUT);//gpio_15 input float
	omap_mux_init_signal("gpio_16",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_16 input down

	omap_mux_init_signal("gpio_21",OMAP_PIN_OUTPUT);//gpio_21 usb_sw_sel1

	omap_mux_init_signal("gpio_27",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_27
	gpio_direction_output(27, 0);

	omap_mux_init_signal("gpio_28",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_28 output
	gpio_direction_output(28, 0) ;

	omap_mux_init_signal("gpio_29",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_29 output
	gpio_direction_output(29, 0);

	omap_mux_init_signal("gpio_31",OMAP_PIN_INPUT);//input float

	omap_mux_init_signal("gpio_34",OMAP_PIN_OUTPUT);
	omap_mux_init_signal("gpio_35",OMAP_PIN_OUTPUT);	

	omap_mux_init_signal("gpio_36",OMAP_PIN_INPUT_PULLDOWN);
	omap_mux_init_signal("gpio_37",OMAP_PIN_INPUT_PULLDOWN);
	gpio_direction_output(36, 0);
	gpio_direction_output(37, 0);
	
	//gpio_38 config in behind
	//omap_cfg_reg(R3_34XX_GPIO39);
	//gpio_40 config in behind
	omap_mux_init_signal("gpio_42",OMAP_PIN_INPUT_PULLUP);
	//omap_cfg_reg(K3_34XX_GPIO43_INPUT);

	gpio_direction_output(55, 0);

	omap_mux_init_signal("gpio_58",OMAP_PIN_INPUT);//gpio_58 input

	//omap_mux_init_signal("gpio_59",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	//gpio_direction_output(59, 0);

	omap_mux_init_signal("gpio_63",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//input PULLDOWN

	omap_mux_init_signal("gpio_64",OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP);//gpio_64 input up

	//omap_cfg_reg(XX_OMAP_3430_GPIO_65);//output low
	omap_mux_init_signal("gpio_65",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	
	omap_mux_init_signal("gpio_94",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//output low
	omap_mux_init_signal("gpio_95",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//output low
	omap_mux_init_signal("gpio_96",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//input down
	gpio_direction_output(96, 0);//output 0

	omap_mux_init_signal("gpio_98",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(98, 0);

	omap_mux_init_signal("gpio_99",OMAP_PIN_INPUT_PULLDOWN |  OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_99 input pulldown
	omap_mux_init_signal("gpio_100",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_100 input pulldown
	omap_mux_init_signal("gpio_101",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio101 input float FM_SCL
	gpio_direction_output(101, 0);
	omap_mux_init_signal("gpio_102",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio102 input float FM_SDA
	gpio_direction_output(102, 0);
	omap_mux_init_signal("gpio_103",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_103 output low
	omap_mux_init_signal("gpio_104",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_104 output low

	//gpio_105-gpio_108 input pulldown
	omap_mux_init_signal("gpio_105", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
	omap_mux_init_signal("gpio_106",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
	omap_mux_init_signal("gpio_107",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
	omap_mux_init_signal("gpio_108",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);

	omap_mux_init_signal("gpio_109",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//output low
	gpio_direction_output(109, 0);
	omap_mux_init_signal("gpio_110",OMAP_PIN_OUTPUT  | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_110 output low
	gpio_direction_output(110, 0);
	
	omap_mux_init_signal("gpio_111",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_111 output
	gpio_direction_output(111 , 0);
	
	omap_mux_init_signal("gpio_112",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//input low
	omap_mux_init_signal("gpio_113",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//input low
	omap_mux_init_signal("gpio_114",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//input low
	omap_mux_init_signal("gpio_115",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//input low

	//omap_cfg_reg(P27_3430_MMC1_DAT4);//gpio_126_1 input pulllow
	omap_mux_init_signal("gpio_126",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//mode4 input down
	//omap_cfg_reg(P26_3430_GPIO_127);
	//gpio_direction_output(127, 1);
	//omap_cfg_reg(R27_3430_GPIO_128);
	//gpio_direction_output(128, 0);

	omap_mux_init_signal("gpio_129",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_129 input pulldown

	omap_mux_init_signal("gpio_153",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_153 output low
	gpio_direction_output(153, 0);

	omap_mux_init_signal("gpio_158",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(158, 0);

	omap_mux_init_signal("gpio_164",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio164 input float FM_IRQ

	omap_mux_init_signal("gpio_167",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(167, 0);

	omap_mux_init_signal("gpio_168", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_168 input down
	omap_mux_init_signal("gpio_170",OMAP_PIN_INPUT);//gpio_170 sel_mic1 , Don't care

	omap_mux_init_signal("gpio_171",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_171 mode4 input down
	omap_mux_init_signal("gpio_172",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_172 mode4 input down
	omap_mux_init_signal("gpio_173",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_173 mode4 input down
	omap_mux_init_signal("gpio_174", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_174 mode4 input down

	omap_mux_init_signal("gpio_175",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_175 output low
	gpio_direction_output(175, 0);
	omap_mux_init_signal("gpio_176",OMAP_PIN_INPUT);//gpio_176 input float
	omap_mux_init_signal("gpio_177",OMAP_PIN_INPUT);//gpio_177 input float

	omap_mux_init_signal("gpio_178",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	
	gpio_direction_output(178, 0);

	omap_mux_init_signal("gpio_179",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(179, 0);

	omap_mux_init_signal("gpio_180",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(180, 0);

	omap_mux_init_signal("gpio_182", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_182 output low
	gpio_direction_output(182, 0);

	omap_mux_init_signal("gpio_183",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_183 input down

	omap_mux_init_signal("gpio_10",OMAP_PIN_OUTPUT  | OMAP_PIN_OFF_OUTPUT_LOW);//output low
	gpio_direction_output(10, 0);

	omap_mux_init_signal("gpio_14",OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);//gpio_14 key_ack, input pull down
    
	omap_mux_init_signal("gpio_22",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(22, 0);

	omap_mux_init_signal("gpio_97", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_97 EAR_EN
    gpio_direction_output(97, 0);

	omap_mux_init_signal("gpio_154",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_154 output low
	gpio_direction_output(154, 0);

	omap_mux_init_signal("gpio_161",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_161 output low
	gpio_direction_output(161, 0);
	omap_mux_init_signal("gpio_162",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_162 output low
	gpio_direction_output(162, 0);

	omap_mux_init_signal("gpio_181",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	gpio_direction_output(181, 0);
			
	wifi_gpio_suspend();
	
}

extern int yl_get_bootreason(char *out, int len);
static int reboot_flag = 0;
void yl_reboot_flag_set(int flag)
{
	reboot_flag = flag;
	return;
}
EXPORT_SYMBOL(yl_reboot_flag_set);

static int get_reboot_flag(void)
{
	return reboot_flag;
}

void twl4030_poweroff(void)
{
	u8 uninitialized_var(val);
	int err;
	struct rtc_wkalrm user_alarm;
	u8 reg = 0;
	
    #define RECOVER_SHUTDOWN
    #ifdef RECOVER_SHUTDOWN
	char boot_reason[20];
	yl_get_bootreason(boot_reason, 20);
    #endif /* RECOVER_SHUTDOWN */
    
	
	//enable power domain access
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

	twl_i2c_read_u8(TWL4030_MODULE_PM_RECEIVER, &reg, 0x05);
	reg &= ~(1 << 3);
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, reg, 0x05);

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, REG_BB_CFG);//disable backup battery charge

	//disable power domain access
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);


	wm9093_PowerDown();

	/*add by aliang for VIA modem power off--2010.10.10*/
	#if defined(CONFIG_AP2MODEM_VIATELECOM)
    extern void ap_poweroff_modem(void);
    ap_poweroff_modem( );
	#endif

	#ifdef CONFIG_YL_MODEM_5860E
	extern int LTE_MODULE_GetPowerStatus(void);
	extern void LTE_MODULE_HARDWARE_POWEROFF(void);
	if(LTE_MODULE_GetPowerStatus())
	{
		LTE_MODULE_HARDWARE_POWEROFF();
	}
	#endif

	poweroff_device();//fix leak in poweroff

 	#ifdef RECOVER_SHUTDOWN
	if((strcmp(boot_reason, "recovery")==0) || get_reboot_flag()) 
	{
	    printk(KERN_ERR "recovery mode,reboot after 5s!\n");
		if((strcmp(boot_reason, "recovery")==0))
		{
			err = twl_i2c_write_u8(TWL4030_MODULE_BACKUP, 0x00, BACKUP_REG_B);
		}
		else//reboot
		{	err = twl_i2c_read_u8(TWL4030_MODULE_BACKUP, &val, BACKUP_REG_B);
			val |= 0x04;//set reboot flag, get detail to see kernel/sys.c
			err = twl_i2c_write_u8(TWL4030_MODULE_BACKUP, val, BACKUP_REG_B);
		}
		if(err)
		{
			printk(KERN_ERR"it is fail to i2c write in set alarm to reboot...\n");
		}
	    set_alarm_reboot();		    
	}else
    #endif /* RECOVER_SHUTDOWN */
	if(twl4030_identify_insert_device() == CHARGER_INSERT)
	{
		printk("set alarm to reboot\n");
		
		err = twl_i2c_read_u8(TWL4030_MODULE_BACKUP, &val, BACKUP_REG_B);
		if(err)
		{
			printk(KERN_ERR"it is fail to i2c read in set alarm to reboot \n");
		}

		val &= ~0x0f;
		
		if(user_set_poweroff_alarm(&user_alarm))
		{
			printk("%s: have user alarm\n", __func__);
			user_alarm.time.tm_year -= 100;
			user_alarm.time.tm_mon += 1;
			save_user_alarm_to_nandflash(&user_alarm);
			val |= 0x02;//have user alarm
		}
		else
		{
			yl_debug("%s: no user alarm\n", __func__);
			val |= 0x01;//no user alarm
		}

		err = twl_i2c_write_u8(TWL4030_MODULE_BACKUP, val, BACKUP_REG_B);
		if(err)
		{
			printk(KERN_ERR"it is fail to i2c write in set alarm to reboot \n");
		}
	
		set_alarm_reboot();
	}
	else
	{
		err = twl_i2c_read_u8(TWL4030_MODULE_BACKUP, &val, BACKUP_REG_B);
		if(err)
		{
			printk(KERN_ERR"it is fail to i2c read in set alarm to reboot...\n");
		}

		val &= ~0x0f;
		err = twl_i2c_write_u8(TWL4030_MODULE_BACKUP, val, BACKUP_REG_B);
		if(err)
		{
			printk(KERN_ERR"it is fail to i2c write in set alarm to reboot...\n");
		}
	}

    twl_i2c_write_u8(TWL4030_MODULE_INT, 0xff, TWL_4030_INT_PWR_EDR1);
    twl_i2c_write_u8(TWL4030_MODULE_INT, 0xff, TWL_4030_INT_PWR_EDR2);
    twl_i2c_write_u8(TWL4030_MODULE_INT, 0x00, TWL_4030_INT_PWR_IMR1);
    twl_i2c_write_u8(TWL4030_MODULE_INT, 0x00, TWL_4030_INT_PWR_IMR2);
   
	setnoreboot();

	//turn off vaux1: motor
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);	
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x17);
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x19);
	//disable power domain access
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);

	//enable power domain access
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);

	twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &reg, REG_CFG_BOOT);
	reg |= (1<<7);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, reg, REG_CFG_BOOT);

	/* Make sure SEQ_OFFSYNC is set so that all the res goes to wait-on */
	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val,
				   CFG_P123_TRANSITION);
	if (err) {
		pr_warning("I2C error %d while reading TWL4030 PM_MASTER CFG_P123_TRANSITION\n",
			err);
		return;
	}

	val |= SEQ_OFFSYNC;
	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val,
				    CFG_P123_TRANSITION);
	if (err) {
		pr_warning("I2C error %d while writing TWL4030 PM_MASTER CFG_P123_TRANSITION\n",
			err);
		return;
	}
	//diable powerdomain access
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);

	printk(KERN_EMERG"\r\nPower off smartphone...\r\n");

	//cut off p1 power
	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val,
				  PWR_P1_SW_EVENTS);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while reading TWL4030"
					"PM_MASTER P1_SW_EVENTS\n", err);
		return ;
	}

	val |= PWR_DEVOFF;

	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val,
				   PWR_P1_SW_EVENTS);

	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while writing TWL4030"
					"PM_MASTER P1_SW_EVENTS\n", err);
		return ;
	}
	//cut off p2 power
	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val, PWR_P2_SW_EVENTS);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while reading TWL4030 PM_MASTER P2_SW_EVENTS\n", err);
		return;
	}

	val |= PWR_DEVOFF;
	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val, PWR_P2_SW_EVENTS);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while writing TWL4030 PM_MASTER P2_SW_EVENTS\n", err);
		return;
	}

	//cut off p3 power
	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val, PWR_P3_SW_EVENTS);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while reading TWL4030 PM_MASTER P3_SW_EVENTS\n", err);
		return;
	}

	val |= PWR_DEVOFF;
	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val, PWR_P3_SW_EVENTS);
	if (err) 
	{
		printk(KERN_WARNING "I2C error %d while writing TWL4030 PM_MASTER P3_SW_EVENTS\n", err);
		return;
	}

	return;
}

static int __init twl4030_poweroff_init(void)
{
	pm_power_off = twl4030_poweroff;

	return 0;
}

static void __exit twl4030_poweroff_exit(void)
{
	pm_power_off = NULL;
}

module_init(twl4030_poweroff_init);
module_exit(twl4030_poweroff_exit);

MODULE_DESCRIPTION("Triton2 device power off");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter De Schrijver");

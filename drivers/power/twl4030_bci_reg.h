/*
 * drivers/power/twl4030_bci_reg.h
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

#ifndef __TWL4030_BCI_REG_H__
#define __TWL4030_BCI_REG_H__

#include <linux/power_supply.h>


#ifdef __cplusplus
extern "C" {
#endif

#define T2_BATTERY_VOLT			0x04
#define T2_BATTERY_TEMP			0x06
#define T2_BATTERY_CUR			0x08

#define P1_P2_P3_ENABLE     	(0x07 << 5)

#define REG_BCIMDKEY    		0x001

/* charger constants */
#define NO_PW_CONN				0
#define AC_PW_CONN				0x01
#define USB_PW_CONN				0x02

/* TWL4030_MODULE_USB */
#define REG_POWER_CTRL			0x0AC
#define OTG_EN					0x020
#define REG_PHY_CLK_CTRL		0x0FE
#define REG_PHY_CLK_CTRL_STS	0x0FF
#define PHY_DPLL_CLK			0x01

#define REG_BCICTL1				0x023
#define REG_BCICTL2				0x024
#define CGAIN					0x020
#define ITHEN					0x010
#define ITHSENS					0x007

/* Boot BCI flag bits */
#define BCIAUTOWEN				0x020
#define CONFIG_DONE				0x010
#define BCIAUTOUSB				0x002
#define BCIAUTOAC				0x001
#define BCIMSTAT_MASK			0x03F
#define CVENAC          		(1 << 2)

/* Boot BCI register */
#define REG_BOOT_BCI			0x007
#define REG_CTRL1				0x00
#define REG_SW1SELECT_MSB		0x07
#define SW1_CH9_SEL				0x02
#define REG_CTRL_SW1			0x012
#define SW1_TRIGGER				0x020
#define EOC_SW1					0x002
#define REG_GPCH9				0x049
#define REG_STS_HW_CONDITIONS	0x0F
#define STS_VBUS				0x080
#define STS_CHG					0x02
#define STS_USB         		0x04
#define REG_BCIMSTATEC			0x02
#define REG_BCIMFSTS4			0x010
#define REG_BCIMFKEY			0x011
#define REG_BCIIREF1			0x027
#define REG_BCIIREF2			0x028
#define REG_BCIMFSTS2			0x00E
#define REG_BCIMFSTS3			0x00F
#define REG_BCIMFSTS1			0x001
#define USBFASTMCHG				0x004
#define BATSTSPCHG				0x004
#define BATSTSMCHG				0x040
#define VBATOV4					0x020
#define VBATOV3					0x010
#define VBATOV2					0x008
#define VBATOV1					0x004
#define BBCHEN					0x010

/* Power supply charge interrupt */
#define REG_PWR_ISR1			0x00
#define REG_PWR_IMR1			0x01
#define REG_PWR_EDR1			0x05
#define REG_PWR_SIH_CTRL		0x007

#define USB_PRES				0x004
#define CHG_PRES				0x002

#define USB_PRES_RISING			0x020
#define USB_PRES_FALLING		0x010
#define CHG_PRES_RISING			0x008
#define CHG_PRES_FALLING		0x004
#define AC_STATEC				0x20
#define COR						0x004

/* interrupt status registers */
#define REG_BCIISR1A			0x0
#define REG_BCIISR2A			0x01

/* Interrupt flags bits BCIISR1 */
#define BATSTS_ISR1		   	 	0x080
#define VBATLVL_ISR1			0x001
#define ICHGEOC_ISR1    		(1 << 4)//充电完成中断

/* Interrupt mask registers for int1*/
#define REG_BCIIMR1A			0x002
#define REG_BCIIMR2A			0x003

 /* Interrupt masks for BCIIMR1 */
#define BATSTS_IMR1		    	0x080
#define VBATLVL_IMR1			0x001
#define ICHGEOC_IMR1    		(1 << 4)

/* Interrupt edge detection register */
#define REG_BCIEDR1				0x00A
#define REG_BCIEDR2				0x00B
#define REG_BCIEDR3				0x00C

/* BCIEDR2 */
#define	BATSTS_EDRRISIN		    0x080
#define BATSTS_EDRFALLING	    0x040
#define ICHGEOC_EDRRISIN    	0x020
#define ICHGEOC_EDRFALLING  	0x001

/* BCIEDR3 */
#define	VBATLVL_EDRRISIN		0x02

/* Step size and prescaler ratio */
#define TEMP_STEP_SIZE			147
#define TEMP_PSR_R				100

#define VOLT_STEP_SIZE			588
#define VOLT_PSR_R				100

#define CURR_STEP_SIZE			147
#define CURR_PSR_R1				44
#define CURR_PSR_R2				80

#define BK_VOLT_STEP_SIZE		441
#define BK_VOLT_PSR_R			100

#define AC_CHARGE_DEVICE        1
#define USB_CHARGE_DEVICE       2
#define AC_CHARGE_CURRENT		809
#define USB_CHARGE_CURRENT		500

#define ENABLE		    		1
#define DISABLE		    		1

#define HIGH_LEVEL      		1
#define LOW_LEVEL       		0

#define ENABLE_CURRENT_VOLT_SET          0xE7
#define DISABLE_MANUAL_CHARGE            0x2a

#define PM_MASTER_PROTECT_KEY            0x00e
/* TWL4030_MODULE_USB */
#define REG_FUNC_CTRL   				 0x04
#define OTG_EN							 0x020
#define MADC_ON					         0x01
#define MADC_HFCLK_EN			         0x80
#define REG_GPBR1           	         0x0c
#define DEFAULT_MADC_CLK_EN		         0x10
//#define REG_PHY_CLK_CTRL				 0xFE
//#define REG_PHY_CLK_CTRL_STS			 0xFF

#define REG_IFC_CTRL                     0x07
#define REG_IFC_CTRL_SET                 0x08
#define REG_IFC_CTRL_CLR                 0x09

#define REG_MCPC_IO_CTRL                 0x33
#define REG_MCPC_IO_CTRL_SET             0x34
#define REG_MCPC_IO_CTRL_CLR             0x35

#define REG_CARKIT_SM_CTRL               0xA1
#define REG_CARKIT_SM_CTRL_SET           0xA2
#define REG_CARKIT_SM_CTRL_CLR           0xA3

//#define REG_POWER_CTRL                   0xAC
#define REG_POWER_CTRL_SET               0xAD
#define REG_POWER_CTRL_CLR               0xAE

#define REG_PHY_PWR_CTRL                 0xFD

//usb register, added by huangjiefeng
#define REG_OTHER_INT_EN_RISE            0x86
#define REG_OTHER_INT_EN_RISE_SET        0x87
#define REG_OTHER_INT_EN_RISE_CLR        0x88
#define REG_OTHER_INT_EN_FALL            0x89     
#define REG_OTHER_INT_EN_FALL_SET        0x8A
#define REG_OTHER_INT_EN_FALL_CLR        0x8B
#define REG_OTHER_INT_STS                0x8C
#define REG_OTHER_FUNC_CTRL              0x80
#define REG_OTG_CTRL                     0x0A
#define REG_OTG_CTRL_SET                 0x0B
#define REG_OTG_CTRL_CLEAR               0x0C

//define USB bits 
#define DP_HI_RISE_EN                    (1 << 5)
#define DP_HI_FALL_EN                    (1 << 5)
#define DM_HI_RISE_EN                    (1 << 6)
#define DM_HI_FALL_EN                    (1 << 6)
#define DM_PULLUP                        (1 << 7)
#define DP_PULLUP                        (1 << 6)
#define DPPULLDOWN                       (1 << 1)
#define DMPULLDOWN                       (1 << 2)
#define DP_STATUS                        (1 << 5)
#define DM_STATUS                        (1 << 6)

#define REG_BCIMFTH1        	0x16
#define REG_BCIMFTH2        	0x17
#define REG_BCIMFTH3        	0x18
#define REG_BCIMFTH4        	0x19
#define REG_BCIMFTH5        	0x1A
#define REG_BCIMFTH6        	0x1B
#define REG_BCIMFTH7        	0x1C
#define REG_BCIMFTH8        	0x1D
#define REG_BCIMFTH9        	0x1E

/* Key codes for the Monitoring functions regsiter acess.*/
#define MFKEY1					0x57
#define MFKEY2					0x73
#define MFKEY3					0x9C
#define MFKEY4					0x3E
#define MFKEY5					0xD2
#define MFKEY6					0x7F
#define MFKEY7					0x6D
#define MFKEY8					0xEA
#define MFKEY9					0xC4
#define MFKEY10					0xBC
#define MFKEY11					0xC3
#define MFKEY12					0xF4
#define MFKEY13					0xE7 

//#define P0_BOARD                0x00
#define P1_BOARD                0x01

//电池是否存在的临界值
#define BATERRY_PRESENCE_P0    	512   //M1 P0  board
#define BATERRY_PRESENCE        245 //9130 p0 board
//#define BATERRY_PRESENCE        474 //9130 p2 board
#define REG_BCIWDKEY        	0x21
#define DISABLE_WATCHDOG    	0XF3

#define CHG_EN1_PIN             36
#define CHG_EN2_PIN             37
#define CHG_OVER_PIN            42
#define CHG_STATE_PIN       	43
#define VBAT_DET_PIN            163

#define CHG_EN1_GPIO_NAME        "gpio_37"
#define CHG_EN2_GPIO_NAME        "gpio_37"
#define CHG_OVERT_GPIO_NAME      "gpio_42"
#define CHG_STATE_GPIO_NAME      "gpio_43"
#define VBAT_DET_GPIO_NAME       "gpio_163"

#define TRUE                    1
#define FALSE                   0

#define SMARTPHONE_MAX_VOLT     (4160)
#define SMARTPHONE_MIN_VOLT     (3500)

#define IDCODE_31_24_REG        0x03
#define UNLOCK_TEST_REG         0x12

#define AC_CHARGER_CORRECTION_VAL      (200)
#define USB_CHARGER_CORRECTION_VAL     (150)
#define NO_CHARGER_CORRECTION_VAL      (50)
#define BATTERY_FULL_VOLT              (4157)
#define LINEARITY_SAMPLE_VAL           (3850)  

#define VDD1_VMODE_CFG           0x5F
#define VDD2_VMODE_CFG           0x6D
#define PRM_VC_CMD_VAL_0         0x48307230
#define PRM_VC_CMD_VAL_1         0x4830722c               


#ifdef __cplusplus
}
#endif

typedef enum
{
	charger_unplug = 0,
	charger_plug,
	usb_charge,
	ac_charge,
	usb_client_device,//如U盘、鼠标
}charger_status;

struct twl4030_bci_device_info {
	struct device		*dev;

	unsigned long		update_time;
	int			voltage_uV;
	int			bk_voltage_uV;
	int			current_uA;
	int			temp_C;
	int			charge_rsoc;
	int			charge_status;
	int			capacity;

	struct notifier_block	nb;
	struct power_supply	bat;
	struct power_supply	bk_bat;
	struct power_supply usb_bat;
	struct power_supply ac_bat;
	struct delayed_work	twl4030_bci_monitor_work;
	struct delayed_work	twl4030_bk_bci_monitor_work;
	struct delayed_work  charge_full_handle;
};

/* Ptr to thermistor table */
extern int *therm_tbl;
extern int usb_charger_flag;
extern int charger_plug_flag;
extern unsigned int g_mid_hardware_version;

void usb_device_insert_done(void);
void device_insert_interrupt_done(u8 clear, u8 set);
int twl4030_get_battery_voltage(void);
int twl4030battery_temperature(void);
int twl4030battery_current(void);
int twl4030backupbatt_voltage(void);
int twl4030charger_ac_en(int enable, int charge_device);
int twl4030charger_usb_en(int enable);
int twl4030_identify_insert_device(void);
int twl4030_identify_charge_device(void);
void system_stop_charge(void);
void charge_full_done(struct work_struct *work);
void detect_battery_status(void);
int setup_charge_current(int charging_source, int bci_charging_current);
int twl4030_hardware_init(void);
bool twl4030_battery_presence_identify(void);
void enter_cv_mode(int charge_device);
void get_hardware_version(void);
int twl4030battery_hw_presence_en(int enable);
int twl4030battery_hw_level_en(int enable);
int clear_n_set(u8 mod_no, u8 clear, u8 set, u8 reg);
void disable_usb_remove_flag_from_suspend(void);
int get_usb_remove_flag_from_suspend(void);
void set_charge_mode_to_usb(void);
void set_core_volt_to_retention(void);
#endif // __MPCOMM_H__

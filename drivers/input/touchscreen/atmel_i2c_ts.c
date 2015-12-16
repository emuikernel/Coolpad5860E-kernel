/*
 * AT42QT602240/ATMXT224 Touchscreen driver
 *
 * Copyright (C) 2010 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
//#include <linux/i2c/qt602240_ts.h>
#include <linux/earlysuspend.h>
//#include <linux/atmel_i2c_ts.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <plat/mux.h>//
#include <mach/mux.h>
#include <plat/yl_debug.h>//added by huangjiefeng
#include <plat/yl_params.h>

#include "atmel_i2c_ts.h"
#include "ft5x06_ts.h"
#include "touchscreen.h"

extern u8 is_p0;
extern int ft5x0x_tw_is_or_not_power_on;

u8 atmel_tw_is_active_status = 0;

static struct qt602240_data *data;


/****************sz**********/
#define TW_TAG  "TouchScreen: "
//#define TW_DEBUG        //shezhi
#ifdef TW_DEBUG	    
#define TW_DBG(fmt, ...) \
	    yl_touch_debug(LOG_DEBUG, TW_TAG pr_fmt(fmt), ##__VA_ARGS__)	    
#else
#define TW_DBG(fmt, ...) do{ \
	}while(0)
#endif

//#define TOUCH_INT_GPIO                  61


/* Version */
#define QT602240_VER_16			16
#define QT602240_VER_20			20
#define QT602240_VER_21			21
#define QT602240_VER_22			22
#define QT602240_VER_32			32

/* Slave addresses */
#define QT602240_APP_LOW		0x4a
#define QT602240_APP_HIGH		0x4b
#define QT602240_BOOT_LOW		0x24
#define QT602240_BOOT_HIGH		0x25

/* Firmware */
#define QT602240_FW_NAME		"qt602240.fw"

/* Registers */
#define QT602240_FAMILY_ID		0x00
#define QT602240_VARIANT_ID		0x01
#define QT602240_VERSION		0x02
#define QT602240_BUILD			0x03
#define QT602240_MATRIX_X_SIZE	0x04
#define QT602240_MATRIX_Y_SIZE	0x05
#define QT602240_OBJECT_NUM		0x06
#define QT602240_OBJECT_START	0x07

#define QT602240_OBJECT_SIZE		6

/* Object types */
#define QT602240_DEBUG_DIAGNOSTIC	37
#define QT602240_GEN_MESSAGE		5
#define QT602240_GEN_COMMAND		6
#define QT602240_GEN_POWER		    7
#define QT602240_GEN_ACQUIRE		8
#define QT602240_TOUCH_MULTI		9
#define QT602240_TOUCH_KEYARRAY		15
#define QT602240_TOUCH_PROXIMITY	23
#define QT602240_PROCI_GRIPFACE		20
#define QT602240_PROCG_NOISE		22
#define QT602240_PROCI_ONETOUCH		24
//#define QT602240_PROCI_TWOTOUCH		27
#define QT602240_SPT_COMMSCONFIG	18	/* firmware ver 21 over */
#define QT602240_SPT_GPIOPWM		19
#define QT602240_SPT_SELFTEST		25
//#define QT602240_SPT_CTECONFIG		28 now is 46
#define QT602240_SPT_USERDATA			 38	/* firmware ver 21 over */

#define QT602240_SPT_MESSAGECOUNT		 44

#define QT602240_PROCI_GRIPSUPPRESSION   40	/* firmware ver 16 over */
#define QT602240_PROCI_TOUCHSUPPRESSION  42	/* firmware ver 16 over */
#define QT602240_SPT_CTECONFIG           46	/* firmware ver 16 over */
#define QT602240_PROCI_STYLUS            47	/* firmware ver 16 over */
#define QT602240_PROCG_NOISESUPPRESSION  48	/* firmware ver 16 over */

/* QT602240_GEN_COMMAND field */
#define QT602240_COMMAND_RESET		0
#define QT602240_COMMAND_BACKUPNV	1
#define QT602240_COMMAND_CALIBRATE	2
#define QT602240_COMMAND_REPORTALL	3
#define QT602240_COMMAND_DIAGNOSTIC	5

/* QT602240_GEN_POWER field */
#define QT602240_POWER_IDLEACQINT	0
#define QT602240_POWER_ACTVACQINT	1
#define QT602240_POWER_ACTV2IDLETO	2

/* QT602240_GEN_ACQUIRE field */
#define QT602240_ACQUIRE_CHRGTIME			0
#define QT602240_ACQUIRE_TCHDRIFT			2
#define QT602240_ACQUIRE_DRIFTST			3
#define QT602240_ACQUIRE_TCHAUTOCAL			4
#define QT602240_ACQUIRE_SYNC				5
#define QT602240_ACQUIRE_ATCHCALST			6
#define QT602240_ACQUIRE_ATCHCALSTHR		7
#define QT602240_ACQUIRE_ATCHFRCCALTHR  	8
#define QT602240_ACQUIRE_ATCHFRCCALRATIO	9

/* QT602240_TOUCH_MULTI field */
#define QT602240_TOUCH_CTRL			0
#define QT602240_TOUCH_XORIGIN		1
#define QT602240_TOUCH_YORIGIN		2
#define QT602240_TOUCH_XSIZE		3
#define QT602240_TOUCH_YSIZE		4
#define QT602240_TOUCH_AKSCFG		5
#define QT602240_TOUCH_BLEN			6
#define QT602240_TOUCH_TCHTHR		7
#define QT602240_TOUCH_TCHDI		8
#define QT602240_TOUCH_ORIENT		9
#define QT602240_TOUCH_MGRTIMEOUT   10
#define QT602240_TOUCH_MOVHYSTI		11	
#define QT602240_TOUCH_MOVHYSTN		12
#define QT602240_TOUCH_MOVFILTER    13
#define QT602240_TOUCH_NUMTOUCH		14
#define QT602240_TOUCH_MRGHYST		15
#define QT602240_TOUCH_MRGTHR		16
#define QT602240_TOUCH_AMPHYST		17
#define QT602240_TOUCH_XRANGE_LSB	18
#define QT602240_TOUCH_XRANGE_MSB	19
#define QT602240_TOUCH_YRANGE_LSB	20
#define QT602240_TOUCH_YRANGE_MSB	21
#define QT602240_TOUCH_XLOCLIP		22
#define QT602240_TOUCH_XHICLIP		23
#define QT602240_TOUCH_YLOCLIP		24
#define QT602240_TOUCH_YHICLIP		25
#define QT602240_TOUCH_XEDGECTRL	26
#define QT602240_TOUCH_XEDGEDIST	27
#define QT602240_TOUCH_YEDGECTRL	28
#define QT602240_TOUCH_YEDGEDIST	29
#define QT602240_TOUCH_JUMPLIMIT	30	/* firmware ver 22 over */
/*The following regs are for mXT E version*/
#define	QT602240_TOUCH_TCHHYST		31
#define	QT602240_TOUCH_XPITCH		32
#define	QT602240_TOUCH_YPITCH		33
#define	QT602240_TOUCH_NEXTTCHDI	34

/* QT602240_TOUCH_KEYARRAY field */
#define QT602240_TOUCH_CTRL			0
#define QT602240_TOUCH_XORIGIN		1
#define QT602240_TOUCH_YORIGIN		2
#define QT602240_TOUCH_XSIZE		3
#define QT602240_TOUCH_YSIZE		4
#define QT602240_TOUCH_AKSCFG		5
#define QT602240_TOUCH_BLEN			6
#define QT602240_TOUCH_TCHTHR		7
#define QT602240_TOUCH_TCHDI		8

/* QT602240_PROCI_GRIP field */
#define QT602240_GRIP_CTRL		0
#define QT602240_GRIP_XLOGRIP	1
#define QT602240_GRIP_XHIGRIP	2
#define QT602240_GRIP_YLOGRIP	3
#define QT602240_GRIP_YHIGRIP	4
/* QT602240_PROCI_TOUCH SUPPRESSION field */
#define QT602240_TOUCH_CTRL			 0
#define QT602240_TOUCH_APPRTHR		 1
#define QT602240_TOUCH_MAXAPPRAREA   2
#define QT602240_TOUCH_MAXTCHAREA    3
#define QT602240_TOUCH_SUPSTRENGTH   4
#define QT602240_TOUCH_SUPEXTTO	     5
#define QT602240_TOUCH_MAXNUMTCHS    6
#define QT602240_TOUCH_SHAPESTRENGTH 7

/*QT602240_PROCI_STYLUS field*/
#define QT602240_PROC_STYLUS_CTRL 			0
#define QT602240_PROC_STYLUS_CONTMIN 		1
#define QT602240_PROC_STYLUS_CONTMAX 		2
#define QT602240_PROC_STYLUS_STABILITY 		3
#define QT602240_PROC_STYLUS_MAXTCHAREA 	4
#define QT602240_PROC_STYLUS_AMPLTHR 		5
#define QT602240_PROC_STYLUS_STYSHAPE 		6
#define QT602240_PROC_STYLUS_HOVERSUP 		7
#define QT602240_PROC_STYLUS_CONFTHR 		8
#define QT602240_PROC_STYLUS_SYNCSPERX 		9

/* QT602240_PROCI_NOISE field */
#define QT602240_NOISE_CTRL					0
#define QT602240_NOISE_CFG					1
#define QT602240_NOISE_CALCFG				2
#define QT602240_NOISE_BASEFREQ				3

#define QT602240_NOISE_MFFREQLSB			8
#define QT602240_NOISE_MFFREQMSB			9

#define QT602240_NOISE_GCACTVINVLDADCS		13
#define QT602240_NOISE_GCIDLEINVLDADCS		14

#define QT602240_NOISE_GCMAXADCSPERX        17
#define QT602240_NOISE_GCLIMITMIN			18
#define QT602240_NOISE_GCLIMITMAX			19
#define QT602240_NOISE_GCCOUNTMINTGTLSB		20
#define QT602240_NOISE_GCCOUNTMINTGTMSB		21
#define QT602240_NOISE_MFINVLDDIFFTHR		22
#define QT602240_NOISE_MFINCADCSPXTHRLSB	23
#define QT602240_NOISE_MFINCADCSPXTHRMSB	24
#define QT602240_NOISE_MFERRORTHRLSB		25
#define QT602240_NOISE_MFERRORTHRMSB		26
#define QT602240_NOISE_SELFREQMAX			27

/* QT602240_SPT_COMMSCONFIG */
#define QT602240_COMMS_CTRL				0
#define QT602240_COMMS_CMD				1

/* QT602240_SPT_CTECONFIG field */
#define QT602240_CTE_CTRL				0
#define QT602240_CTE_MODE				1
#define QT602240_CTE_IDLEGCAFDEPTH		2
#define QT602240_CTE_ACTVGCAFDEPTH		3
//#define QT602240_CTE_ADCSPERSYNC		4  ///DIKDJFFFFFKKKKK
#define QT602240_CTE_VOLTAGE         	4
#define QT602240_CTE_PULSESPERADC		5	/* firmware ver 21 over */
#define QT602240_CTE_XSLEW				6
#define QT602240_CTE_SYNCDELAYLSB		7
#define QT602240_CTE_SYNCDELAYMSB		8

#define QT602240_VOLTAGE_DEFAULT	2700000
#define QT602240_VOLTAGE_STEP		10000

/* Define for QT602240_GEN_COMMAND */
#define QT602240_BOOT_VALUE			0xa5
#define QT602240_BACKUP_VALUE		0x55
#define QT602240_BACKUP_TIME		25	/* msec */
#define QT602240_RESET_TIME			70	/* msec */
#define QT602240_FWRESET_TIME		175	/* msec */

/* Command to unlock bootloader */
#define QT602240_UNLOCK_CMD_MSB		0xaa
#define QT602240_UNLOCK_CMD_LSB		0xdc

/* Bootloader mode status */
#define QT602240_WAITING_BOOTLOAD_CMD	0xc0	/* valid 7 6 bit only */
#define QT602240_WAITING_FRAME_DATA		0x80	/* valid 7 6 bit only */
#define QT602240_FRAME_CRC_CHECK		0x02
#define QT602240_FRAME_CRC_FAIL			0x03
#define QT602240_FRAME_CRC_PASS			0x04
#define QT602240_APP_CRC_FAIL			0x40	/* valid 7 8 bit only */
#define QT602240_BOOT_STATUS_MASK		0x3f

/* Touch status */
#define QT602240_SUPPRESS		(1 << 1)
#define QT602240_AMP			(1 << 2)
#define QT602240_VECTOR			(1 << 3)
#define QT602240_MOVE			(1 << 4)
#define QT602240_RELEASE		(1 << 5)
#define QT602240_PRESS			(1 << 6)
#define QT602240_DETECT			(1 << 7)

/* keypad status */
#define QT602240_MENU		           1
#define QT602240_HOME			(1 << 1)
#define QT602240_BACK			(1 << 2)
#define QT602240_SEARCH			(1 << 3)

/* Touchscreen absolute values */
#define QT602240_MAX_XC			0x3FF
#define QT602240_MAX_YC			0x3FF
#define QT602240_MAX_AREA		0xff

//#define QT602240_MAX_FINGER		10

static const unsigned char crc8_tab[256] = {
           0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
           0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
           0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
           0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
           0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
           0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
           0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
           0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
           0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
           0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
           0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
           0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
           0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
           0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
           0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
           0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
           0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
           0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
           0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
           0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
           0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
           0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
           0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
           0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
           0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
           0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
           0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
           0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
           0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
           0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
           0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
           0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};     /* 8-bit table */

unsigned char crc8_accumulate(unsigned char crc8val, const unsigned char *s, int len)
{
	int i;

	 yl_touch_debug(LOG_DEBUG, "+rcrc8_accumulate");

	for (i = 0;  i < len;  i++)
		crc8val = crc8_tab[(crc8val) ^ s[i]];
	
	 yl_touch_debug(LOG_DEBUG, "-rcrc8_accumulate");

	return crc8val;
}

/* Initial register values recommended from chip vendor */
static const u8 init_vals_ver_20[] = {
	/* QT602240_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_GEN_POWER(7) */
	0x20, 0xff, 0x32,
	/* QT602240_GEN_ACQUIRE(8) */
	0x08, 0x05, 0x05, 0x00, 0x00, 0x00, 0x05, 0x14,
	/* QT602240_TOUCH_MULTI(9) */
	0x83, 0x00, 0x00, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x02, 0x00,
	0x00, 0x01, 0x01, 0x0e, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x64,
	/* QT602240_TOUCH_KEYARRAY(15) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* QT602240_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
	/* QT602240_PROCI_GRIPFACE(20) */
	0x00, 0x64, 0x64, 0x64, 0x64, 0x00, 0x00, 0x1e, 0x14, 0x04,
	0x1e, 0x00,
	/* QT602240_PROCG_NOISE(22) */
	0x05, 0x00, 0x00, 0x19, 0x00, 0xe7, 0xff, 0x04, 0x32, 0x00,
	0x01, 0x0a, 0x0f, 0x14, 0x00, 0x00, 0xe8,
	/* QT602240_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	/* QT602240_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	/* QT602240_PROCI_TWOTOUCH(27) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x00, 0x04, 0x08,
};

static const u8 init_vals_ver_21[] = {
	/* QT602240_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_GEN_POWER(7) */
	0x20, 0xff, 0x32,
	/* QT602240_GEN_ACQUIRE(8) */
	0x0a, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x23,
	/* QT602240_TOUCH_MULTI(9) */
	0x00, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00,
	0x00, 0x01, 0x01, 0x0e, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_TOUCH_KEYARRAY(15) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	/* QT602240_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_PROCI_GRIPFACE(20) */
	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x28, 0x04,
	0x0f, 0x0a,
	/* QT602240_PROCG_NOISE(22) */
	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x23, 0x00,
	0x00, 0x05, 0x0f, 0x19, 0x23, 0x2d, 0x03,
	/* QT602240_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	/* QT602240_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	/* QT602240_PROCI_TWOTOUCH(27) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x00, 0x08, 0x10, 0x00,
};

static const u8 init_vals_ver_22[] = {
	/* QT602240_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_GEN_POWER(7) */
	0x0a, 0x0a, 0x32,
	/* QT602240_GEN_ACQUIRE(8) */
	0x0a, 0x05, 0x05, 0x0a, 0x00, 0x00, 0x0a, 0x14,
	/* QT602240_TOUCH_MULTI(9) */
	0x83, 0x00, 0x00, 0x12, 0x0b, 0x00, 0x16, 0x23, 0x03, 0x05,
	0x00, 0x05, 0x05, 0x00, 0x04, 0x0a, 0x0a, 0x0a, 0xc0, 0x03,
	0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x5e, 0x1e, 0x1e, 0x1e,
	0x0a,
	/* QT602240_TOUCH_KEYARRAY(15) */
	0x03, 0x0f, 0x0b, 0x03, 0x01, 0x00, 0x01, 0x1e, 0x02, 0x00,
	0x00,
	/* QT602240_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_PROCI_GRIPFACE(20) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x0f, 0x14, 0x00,
	0x00, 0x00,
	/* QT602240_PROCG_NOISE(22) */
	0x1d, 0x00, 0x00, 0x19, 0x00, 0xe7, 0xff, 0x03, 0x14, 0x00,
	0x01, 0x00, 0x07, 0x0b, 0x0d, 0xff, 0x00,
	/* QT602240_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_SPT_SELFTEST(25) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	/* QT602240_PROCI_TWOTOUCH(27) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x02, 0x08, 0x08, 0x1e,
};

static const u8 init_vals_ver_32[] = {
	/* QT602240_GEN_COMMAND(6) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_GEN_POWER(7) */
	/*0xff, 0xff, 0x32,*/
        0x20, 0x0a, 0x32,
	/* QT602240_GEN_ACQUIRE(8) */
	0x0a, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	/* QT602240_TOUCH_MULTI(9) */
	0x83, 0x00, 0x00, 0x12, 0x0b, 0x00, 0x02, 0x23, 0x03, 0x05,
	0x00, 0x05, 0x05, 0x00, 0x04, 0x0a, 0x0a, 0x0a, 0xc0, 0x03,
	0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x5e, 0x1e, 0x1e, 0x1e,
	0x0a, 0x08,
	/* QT602240_TOUCH_KEYARRAY(15) */
	/*0x03, 0x0f, 0x0b, 0x03, 0x01, 0x00, 0x00, 0x1e, 0x02, 0x00,
	0x00,*/
    0x03, 0x0f, 0x0b, 0x03, 0x01, 0x00, 0x00, 0x1e, 0x02, 0x00,
	0x00,
      	/* QT602240_SPT_GPIOPWM(19) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_PROCI_GRIPFACE(20) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0f, 0x14, 0x04,
	0x14, 0x01,
	/* QT602240_PROCG_NOISE(22) */
	0x19, 0x00, 0x00, 0x19, 0x00, 0xe7, 0xff, 0x04, 0x14, 0x00,
	0x00, 0x00, 0x07, 0x0b, 0x0d, 0xff, 0x00,
	/* QT602240_TOUCH_PROXIMITY(23) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_PROCI_ONETOUCH(24) */
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_SPT_SELFTEST(25) */
	0x03, 0x00, 0x1c, 0x25, 0xcc, 0x10, 0x1c, 0x25, 0x88, 0x13,
	0x00, 0x00, 0x00, 0x00,
	///* QT602240_PROCI_TWOTOUCH(27) */
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* QT602240_SPT_CTECONFIG(28) */
	0x00, 0x00, 0x02, 0x08, 0x08, 0x1e,
};

static const u8 init_vals_ver_16_ofilm[] = {
	/*[DEBUG_DIAGNOSTIC_T37 INSTANCE 0] 264
	*/
    /*[SPT_USERDATA_T38 INSTANCE 0] 264*/   //--------
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*[GEN_POWERCONFIG_T7 INSTANCE 0] 272*/  //--------
    0xff, 0xff, 0x32,
    /*[GEN_ACQUISITIONCONFIG_T8 INSTANCE 0]275*/  //---------
//    32, 0x00, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
//	30, 0x00, 10, 10, 0x00, 0x00, 0, 0, 1, 0x00,
	18, 0x00, 20, 20, 0x00, 0x00, 255, 1, 0, 0x00,
    /*[TOUCH_MULTITOUCHSCREEN_T9 INSTANCE 0]285*///--------
    143, 0x00, 0x00, 0x11, 0x0a, 0x01, 0x10, 43, 3, 0x01,      //60->50->40->43  改这个值时，相应的改下board-cp5860e-touchscreen。c文件
    0x00, 0x01, 2, 0x00, 0x05, 0x0a, 0x0a, 0x0a, 0x1F, 0x03,  
    0xDF, 0x01, 8, 2, 18, 22, 176, 41, 158,75, 
	10, 10, 0x00, 0x00, 0x00,                                     //10->7->10  the lastest four
    /*[TOUCH_KEYARRAY_T15 INSTANCE 0]320*/ //---------
    0x83, 0x0D, 0x0A, 0x04, 0x01, 0x02, 0x00, 27, 3, 0x00,    //45->30->27
    0x00,
    /*[SPT_COMMSCONFIG_T18 INSTANCE 0]331*/ //---------
    0x00, 0x00,
    /*[SPT_GPIOPWM_T19 INSTANCE 0]333*/ //---------
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*[TOUCH_PROXIMITY_T23 INSTANCE 0]349*/ //---------
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00,
    /*[SPT_SELFTEST_T25 INSTANCE 0]364*/ //---------
    0x03, 0x00, 0x12, 0x66, 0x72, 0x56, 0x70, 0x62, 0x80, 0x57, 
    0x00, 0x00, 0x00, 0x00,
    /*[PROCI_GRIPSUPPRESSION_T40 INSTANCE 0]378*///---------
    0x00, 0x00, 0x00, 0x00, 0x00,
    /*[PROCI_TOUCHSUPPRESSION_T42 INSTANCE 0]383*///---------
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x01, 25, 40, 35, 0x00, 0x00, 0x00, 0x00,//3.22old
	0x00, 30, 40, 35, 0x00, 0x00, 5, 0x00,//3.22altered


    /*[SPT_CTECONFIG_T46 INSTANCE 0]391*////---------
    0x04, 0x01, 32, 32, 0x00, 0x00, 0x01, 0x00, 0x00,
    /*[PROCI_STYLUS_T47 INSTANCE 0]400*///---------
    0x00, 0x14, 0x3C, 0x05, 0x02, 0x32, 0x28, 0x00, 0x00, 0x28,
    //[PROCG_NOISESUPPRESSION_T48 INSTANCE 0]410 //======
/*    0x01, 0x00, 0x44, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x06, 0x06, 0x00, 0x00, 0x64, 0x04, 0x40, 
	0x0a, 0x00, 0x14, 0x05, 0x00, 0x26, 0x00, 0x14, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x10, 0x41, 0x03, 0x01, 0x01, 0x00,
    0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x0f, 0x0f, 0x9a, 0x3a, 0x91, 
    0x50, 0x64, 0x0f, 0x03,*/ //good 3.13
	1,128,226,3,0,0,0,0,3,3,
	16,40,0,12,12,0,0,60,4,64,
	20,0,35,5,0,34,0,0,0,0,
	0,0,0,0,0,60,3,1,2,0,
	5,10,10,8,2,18,22,176,41,158,
	75,10,10,0,
};

static const u8 init_vals_ver_16_truly[] = {
    /*[SPT_USERDATA_T38 INSTANCE 0] 264*/   //--------
//    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*[GEN_POWERCONFIG_T7 INSTANCE 0] 272*/  //--------
    20, 12, 50,
    /*[GEN_ACQUISITIONCONFIG_T8 INSTANCE 0]275*/  //---------
	20, 0, 10, 10, 0,0, 0, 40, 1, 0,
    /*[TOUCH_MULTITOUCHSCREEN_T9 INSTANCE 0]285*///--------
    143, 0, 0, 17, 10, 1, 16, 60, 2, 3,
    0, 1, 1, 0, 5, 10, 10, 10, 0, 0,  
    0, 0, 8, 2, 18, 22, 176, 41, 158,75, 
    10, 6, 0, 0, 0,
    /*[TOUCH_KEYARRAY_T15 INSTANCE 0]320*/ //---------
    131, 1,10, 4, 1, 1, 0, 45, 0, 0, 
    0,
    /*[SPT_COMMSCONFIG_T18 INSTANCE 0]331*/ //---------
    0, 0,
    /*[SPT_GPIOPWM_T19 INSTANCE 0]333*/ //---------
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0,
    /*[TOUCH_PROXIMITY_T23 INSTANCE 0]349*/ //---------
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0,
    /*[SPT_SELFTEST_T25 INSTANCE 0]364*/ //---------
    3, 0, 0x26, 0x65, 0x86, 0x55, 0x26, 0x65, 0x86, 0x55, 
    0x00, 0x00, 0x00, 0x00,
    /*[PROCI_GRIPSUPPRESSION_T40 INSTANCE 0]378*///---------
    0, 0, 0, 0, 0,
    /*[PROCI_TOUCHSUPPRESSION_T42 INSTANCE 0]383*///---------
	3, 30, 40, 35, 0, 0, 0, 0,
    /*[SPT_CTECONFIG_T46 INSTANCE 0]391*////---------
    4, 1, 16, 16, 0, 0, 0, 0, 0,
    /*[PROCI_STYLUS_T47 INSTANCE 0]400*///---------
    0, 20, 60, 5, 2, 50, 40, 0, 0, 40,
    //[PROCG_NOISESUPPRESSION_T48 INSTANCE 0]410 //
	3,128,226,0,0,0,0,0,8,15,
	16,35,0,12,6,0,0,50,4,64,
	20,0,15,4,0,34,0,20,0,0,
	0,0,0,0,0,50,3,1,1,0,
	5,10,10,8,2,18,22,176,41,158,
	75,10,6,3,
};

static const u8 table[] =      //0-ofilm 1-truly
	{
		3,3,3,3,3,3,3,3,3,3,   //1
		0,0,0,3,3,3,3,3,3,3,
		3,3,3,2,0,0,3,3,3,3,
		3,3,3,3,3,3,2,0,0,3,
		3,3,3,3,3,3,3,3,3,2,   //5
		0,0,3,3,3,3,3,3,3,3,
		3,3,2,0,0,3,3,3,3,3,
		3,3,3,3,3,0,0,0,3,3,
		3,3,3,3,3,3,3,3,0,0,
		0,3,3,3,3,3,3,3,3,3,   //10
		3,0,0,0,3,3,3,3,3,3,   //11
		3,3,3,3,0,0,0,3,3,3,
		3,3,3,3,3,3,3,0,0,0,
		3,3,3,3,3,3,3,3,3,3,
		0,0,0,3,3,3,3,3,3,3,   //15
		3,3,3,0,0,0,3,3,3,3,   //16
		3,3,3,3,3,3,0,0,0,3,
		3,3,3,3,3,3,3,3,3,1,
		0,0,3,3,3,3,3,3,3,3,
		3,3,1,0,0,3,3,3,3,3,   //20
		3,3,3,3,3,1,0,0,3,3,
		3,3,3,3,3,3,3,3,1,0,
		0,0,0,0,
	};

//const unsigned int touch_key[4] = {KEY_MENU,KEY_HOME,KEY_BACK,KEY_SEARCH}; 
const unsigned int touch_key_ofilm[4] = {KEY_SEARCH,KEY_BACK,KEY_HOME,KEY_MENU};  //ofilm
//const unsigned int touch_key[3] = {KEY_BACK, KEY_MENU,KEY_HOME};
const unsigned int touch_key_truly[4] = {KEY_MENU,KEY_HOME,KEY_BACK,KEY_SEARCH};   //truly

#ifdef CONFIG_HAS_EARLYSUSPEND
static void qt602240_early_suspend(struct early_suspend *h);
static void qt602240_late_resume(struct early_suspend *h);
#endif

static int is_stable_status = 0;
u16 T37_ref_data[220];
int write_tw_reference_data(void);
int read_tw_reference_data(void);
u32 touch_has_water(struct qt602240_data *data);
u32 waterproofalgorithm(struct qt602240_data *data);

static u8 atmel_kind = 0;

static struct work_struct cal_work;  
static u8 atmel_suspend = 0;         //1 is suspend,0 is resume
static u8 tw_pressed = 0;
static DEFINE_MUTEX(timer_lock);
static struct timer_list tw_cal_timer;

static int enter_release_timer = 0;

static u8 cal_tw_count = 6;

struct qt602240_object {
	u8 type;
	u16 start_address;
	u8 size;
	u8 instances;
	u8 num_report_ids;

	/* to map object and message */
	u8 max_reportid;
};

struct qt602240_message {
	u8 reportid;
	u8 message[7];
	u8 checksum;
};

static struct workqueue_struct *atmel_wq;


static bool qt602240_object_readable(unsigned int type)
{
	switch (type) {
	case QT602240_GEN_MESSAGE:
	case QT602240_GEN_COMMAND:
	case QT602240_GEN_POWER:
	case QT602240_GEN_ACQUIRE:
	case QT602240_TOUCH_MULTI:
	case QT602240_TOUCH_KEYARRAY:
	case QT602240_TOUCH_PROXIMITY:
	case QT602240_PROCI_GRIPSUPPRESSION:
	case QT602240_PROCI_TOUCHSUPPRESSION: 
	case QT602240_PROCI_STYLUS:
	case QT602240_PROCG_NOISESUPPRESSION:
//	case QT602240_PROCI_ONETOUCH:
//	case QT602240_PROCI_TWOTOUCH:
	case QT602240_SPT_COMMSCONFIG:
	case QT602240_SPT_GPIOPWM:
	case QT602240_SPT_SELFTEST:
	case QT602240_SPT_USERDATA:
	case QT602240_SPT_MESSAGECOUNT:
	case QT602240_SPT_CTECONFIG:
		return true;
	default:
		return false;
	}
}

static bool qt602240_object_writable(unsigned int type)
{
    switch (type) {
//        case QT602240_SPT_USERDATA:
        case QT602240_GEN_POWER:
        case QT602240_GEN_ACQUIRE:
        case QT602240_TOUCH_MULTI:
        case QT602240_TOUCH_KEYARRAY:
        case QT602240_SPT_COMMSCONFIG:
        case QT602240_SPT_GPIOPWM:
        case QT602240_TOUCH_PROXIMITY:
        case QT602240_SPT_SELFTEST:
        case QT602240_PROCI_GRIPSUPPRESSION:
        case QT602240_PROCI_TOUCHSUPPRESSION:
        case QT602240_SPT_CTECONFIG:
        case QT602240_PROCI_STYLUS:
        case QT602240_PROCG_NOISESUPPRESSION:
		return true;
	default:
		return false;
	}
}

static void qt602240_dump_message(struct device *dev,
				  struct qt602240_message *message)
{
/*    yl_touch_debug(LOG_DEBUG, "reportid:\t0x%x\n", message->reportid);
	yl_touch_debug(LOG_DEBUG, "message1:\t0x%x\n", message->message[0]);
	yl_touch_debug(LOG_DEBUG, "message2:\t0x%x\n", message->message[1]);
	yl_touch_debug(LOG_DEBUG, "message3:\t0x%x\n", message->message[2]);
	yl_touch_debug(LOG_DEBUG, "message4:\t0x%x\n", message->message[3]);
	yl_touch_debug(LOG_DEBUG, "message5:\t0x%x\n", message->message[4]);
	yl_touch_debug(LOG_DEBUG, "message6:\t0x%x\n", message->message[5]);
	yl_touch_debug(LOG_DEBUG, "message7:\t0x%x\n", message->message[6]);
	yl_touch_debug(LOG_DEBUG, "checksum:\t0x%x\n", message->checksum);*/
}

static int qt602240_check_bootloader(struct i2c_client *client,
				     unsigned int state)
{
	u8 val;

recheck:
	if (i2c_master_recv(client, &val, 1) != 1) {
		yl_touch_debug(LOG_DEBUG, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	switch (state) {
	case QT602240_WAITING_BOOTLOAD_CMD:
	case QT602240_WAITING_FRAME_DATA:
		val &= ~QT602240_BOOT_STATUS_MASK;
		break;
	case QT602240_FRAME_CRC_PASS:
		if (val == QT602240_FRAME_CRC_CHECK)
			goto recheck;
		break;
	default:
		return -EINVAL;
	}

	if (val != state) {
		yl_touch_debug(LOG_DEBUG, "Unvalid bootloader mode state\n");
		return -EINVAL;
	}

	return 0;
}

static int qt602240_unlock_bootloader(struct i2c_client *client)
{
	u8 buf[2];

	buf[0] = QT602240_UNLOCK_CMD_LSB;
	buf[1] = QT602240_UNLOCK_CMD_MSB;

	if (i2c_master_send(client, buf, 2) != 2) {
		yl_touch_debug(LOG_DEBUG, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int qt602240_fw_write(struct i2c_client *client,
			     const u8 *data, unsigned int frame_size)
{
	if (i2c_master_send(client, data, frame_size) != frame_size) {
		yl_touch_debug(LOG_DEBUG, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int __qt602240_read_reg(struct i2c_client *client,
			       u16 reg, u16 len, void *val)
{
	struct i2c_msg xfer[2];
	u8 buf[2];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = buf;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = val;

	if (i2c_transfer(client->adapter, xfer, 2) != 2) {
		yl_touch_debug(LOG_WARNING, "%s: i2c transfer failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int qt602240_read_reg(struct i2c_client *client, u16 reg, u8 *val)
{
	return __qt602240_read_reg(client, reg, 1, val);
}

static int qt602240_write_reg(struct i2c_client *client, u16 reg, u8 val)
{
	u8 buf[3];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;
	buf[2] = val;

	if (i2c_master_send(client, buf, 3) != 3) {
		yl_touch_debug(LOG_WARNING, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int qt602240_read_object_table(struct i2c_client *client,
				      u16 reg, u8 *object_buf)
{
	return __qt602240_read_reg(client, reg, QT602240_OBJECT_SIZE,
				   object_buf);
}

static struct qt602240_object *qt602240_get_object(struct qt602240_data *data, u8 type)
{
//	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);
	struct qt602240_object *object;
	int i;

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;
		if (object->type == type)
			return object;
	}

	yl_touch_debug(LOG_DEBUG, "Invalid object type\n");
	return NULL;
}

static int qt602240_read_message(struct qt602240_data *data,
				 struct qt602240_message *message)
{
	struct qt602240_object *object;
	u16 reg;

//	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	//yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_read message\n");
	object = qt602240_get_object(data, QT602240_GEN_MESSAGE);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	//yl_touch_debug(LOG_DEBUG, TW_TAG"object->start_address=0x%x\n",object->start_address);
	return __qt602240_read_reg(data->client, reg,
			sizeof(struct qt602240_message), message);
}

static int qt602240_read_object(struct qt602240_data *data,
				u8 type, u8 offset, u8 *val)
{
	struct qt602240_object *object;
	u16 reg;

//	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	object = qt602240_get_object(data, type);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return __qt602240_read_reg(data->client, reg + offset, 1, val);
}

static int qt602240_write_object(struct qt602240_data *data,
				 u8 type, u8 offset, u8 val)
{
	struct qt602240_object *object;
	u16 reg;

	object = qt602240_get_object(data, type);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return qt602240_write_reg(data->client, reg + offset, val);
}

#if 1
static void qt602240_input_report(struct qt602240_data *data, int single_id)
{
	int id;
	struct qt602240_finger *finger = data->finger;
	struct input_dev *input_dev = data->input_dev;

//	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	for (id = 0; id < QT602240_MAX_FINGER; id++) {
		if (!finger[id].status){
            // yl_touch_debug(LOG_DEBUG, TW_TAG "finger[id].x = %d finger[id].y = %d finger[id].area = %d id=%d\n",finger[id].x, finger[id].y, finger[id].area,id);
			continue;
		}

		input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR,
			finger[id].status != QT602240_RELEASE ?
			finger[id].area : 0);
		input_report_abs(input_dev, ABS_MT_POSITION_X,
				finger[id].x);
		input_report_abs(input_dev, ABS_MT_POSITION_Y,
				finger[id].y);
		input_mt_sync(input_dev);
        //yl_touch_debug(LOG_DEBUG, TW_TAG "finger[id].x = %d finger[id].y = %d finger[id].area = %d id=%d\n",finger[id].x, finger[id].y, finger[id].area,id);
		if (finger[id].status == QT602240_RELEASE)
			finger[id].status = 0;
	}
	input_sync(input_dev);
}
#endif
#if 0
static void qt602240_input_report(struct qt602240_data *data, int single_id)
{
	struct qt602240_finger *finger = data->finger;
	struct input_dev *input_dev = data->input_dev;
	int status = finger[single_id].status;
	int finger_num = 0;
	int id;

	for (id = 0; id < QT602240_MAX_FINGER; id++) {
		if (!finger[id].status)
			continue;

		input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR,
				finger[id].status != QT602240_RELEASE ?
				finger[id].area : 0);
		input_report_abs(input_dev, ABS_MT_POSITION_X,
				finger[id].x);
		input_report_abs(input_dev, ABS_MT_POSITION_Y,
				finger[id].y);
		input_mt_sync(input_dev);

		if (finger[id].status == QT602240_RELEASE)
			finger[id].status = 0;
		else
			finger_num++;
	}

	input_report_key(input_dev, BTN_TOUCH, finger_num > 0);

	if (status != QT602240_RELEASE) {
		input_report_abs(input_dev, ABS_X, finger[single_id].x);
		input_report_abs(input_dev, ABS_Y, finger[single_id].y);
	}

	input_sync(input_dev);
}
#endif

static u8 fiveupanddown = 0;


static void qt602240_input_touchevent(struct qt602240_data *data,
				      struct qt602240_message *message, int id)
{
	int x;
	int y;
    //int temp;
	int area;
	struct qt602240_finger *finger = data->finger;
//	struct device *dev = &data->client->dev;
	u8 status = message->message[0];

//	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	/* Check the touch is present on the screen */
    if(!status){
          //yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_ts released status= %d ,id = %d\n",status,id);
    }
	if (!(status & QT602240_DETECT)) { 
		if ((status & QT602240_RELEASE)||(status & QT602240_SUPPRESS)){
			yl_touch_debug(LOG_WARNING, "atmel [%d] released\n", id);
            finger[id].status = QT602240_RELEASE;
			qt602240_input_report(data, id);

			if(id == 0)
			{
				fiveupanddown &= 0x1e; 
			}
			else if(id == 1)
			{
				fiveupanddown &= 0x1d; 
			}
			else if(id == 2)
			{
				fiveupanddown &= 0x1b; 
			}
			else if(id == 3)
			{
				fiveupanddown &= 0x17; 
			}
			else if(id == 4)
			{
				fiveupanddown &= 0x0f; 
			}

			if( is_stable_status == 0 && ( !fiveupanddown ) )
			{
				tw_pressed = 0;
//				yl_touch_debug(LOG_WARNING, "tw_pressed:0:not key\n");
//				waterproofalgorithm(data);
//				if(is_stable_status == 0)                   //added for timer algorithm 4.26
//				{
//					mod_timer(&tw_cal_timer, jiffies+HZ);
//				}

//				yl_touch_debug(LOG_WARNING, "~~~~~~~~~\n");
				if( enter_release_timer >= 1 )
				{
					enter_release_timer = 2;
					yl_touch_debug(LOG_WARNING, "enter_release_timer == 1\n");
					cal_tw_count = 6;
					mod_timer(&tw_cal_timer, jiffies+2*HZ);
				}
				else
					waterproofalgorithm(data);
			}

        }
		return;
	}

	/* Check only AMP detection */
	if (!(status & (QT602240_PRESS | QT602240_MOVE)))
	{	
		//yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_ts_probe shao\n");
		return;
	}

	x = (message->message[1] << 2) | ((message->message[3] & ~0x3f) >> 6);
   	y = (message->message[2] << 2) | ((message->message[3] & ~0xf3) >> 2);

//	x = 480 - x;
//	y = 800 - y;

   	area = message->message[4];

	yl_touch_debug(LOG_DEBUG, "[%d] %s x: %d, y: %d, area: %d\n", id,
		status & QT602240_MOVE ? "moved" : "pressed",
		x, y, area);

	finger[id].status = status & QT602240_MOVE ?
				QT602240_MOVE : QT602240_PRESS;
	if(finger[id].status == QT602240_PRESS)
	{
		yl_touch_debug(LOG_WARNING, "[%d] %s x: %d, y: %d, area: %d\n", id,
			status & QT602240_MOVE ? "moved" : "pressed",
			x, y, area);

		if( enter_release_timer == 2 )
		{
			del_timer_sync(&tw_cal_timer);
			cancel_work_sync(&cal_work);
		}


		tw_pressed = 1;
		if(id == 0)
		{
			fiveupanddown |= 0x1; 
		}
		else if(id == 1)
		{
			fiveupanddown |= 0x2; 
		}
		else if(id == 2)
		{
			fiveupanddown |= 0x4; 
		}
		else if(id == 3)
		{
			fiveupanddown |= 0x8; 
		}
		else if(id == 4)
		{
			fiveupanddown |= 0x10; 
		}
	}

    // yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_ts x = %d y = %d area = %d\n",x, y, area);
	// yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_ts finger[id] status = %d\n",finger[id].status);
	finger[id].x = x;
	finger[id].y = y;
	finger[id].area = area;

	qt602240_input_report(data, id);
}

static void qt602240_input_keyevent(struct qt602240_data *data,
				      struct qt602240_message *message)
{
	//struct device *dev = &data->client->dev;
	u8 status = message->message[0];
    u8 keypad = message->message[1];
    static int key = 0;

//	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	/* Check the touch is present on the key*/
	if (status & QT602240_DETECT) {
		switch (keypad){
        case 0x01 : 
            key = 0;
            break;
		case 0x02 :
            key = 1;
            break;
		case 0x04 : 
            key = 2;
			break;
		case 0x08 : 
            key = 3;
			break;
		default:
			yl_touch_debug(LOG_DEBUG, "ERROR KEYPAD");
			return ;                          
        } 
//		yl_touch_debug(LOG_DEBUG, "%d\n",key );       
		if( atmel_kind == 0x60 ) 
		{     
			tw_pressed = 1;
			input_report_key(data->input_dev,touch_key_truly[key],1); 
			yl_touch_debug(LOG_WARNING, "truly key_type: %d,value: %d\n",touch_key_truly[key],1); 
		} 
		else
		{
			tw_pressed = 1;
			input_report_key(data->input_dev,touch_key_ofilm[key],1); 
			yl_touch_debug(LOG_WARNING, "ofilm key_type: %d,value: %d\n",touch_key_ofilm[key],1); 			
		}           	
	}
	else if ((status & QT602240_DETECT) == 0) {
		if( atmel_kind == 0x60 ) 
		{
			tw_pressed = 0;
			input_report_key(data->input_dev,touch_key_truly[key],0);
			yl_touch_debug(LOG_WARNING, "truly key_type: %d,value: %d\n",touch_key_truly[key],0); 
		}
		else
		{
			tw_pressed = 0;
			input_report_key(data->input_dev,touch_key_ofilm[key],0);
			yl_touch_debug(LOG_WARNING, "ofilm key_type: %d,value: %d\n",touch_key_ofilm[key],0); 
		}
	}
    input_sync(data->input_dev);
}

#if 1
void atmel_ts_release(struct qt602240_data *data)
{   
	//struct qt602240_finger *finger = data->finger;
    int id;
    int report_sync=0;

//	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

    for (id = 0; id < QT602240_MAX_FINGER; id++) 
    {
     	if((data->finger[id].status!=QT602240_RELEASE)&&(data->finger[id].status!= 0))
        {        
             input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			 input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 0);
			 input_report_abs(data->input_dev, ABS_MT_TOUCH_MINOR, 0);
			 input_mt_sync(data->input_dev);
			 report_sync++;
			 data->finger[id].status = QT602240_RELEASE;
	 	}
    }
    if(report_sync)
    {
        input_sync(data->input_dev);
    }
    //yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_do release\n");
}
#endif	

static void atmel_ts_work_func(struct work_struct *work) 
{
	int id;
	u8 reportid;
    u8 reportid_key;
	u8 max_reportid;
	u8 min_reportid;
	struct qt602240_message message;
	struct qt602240_object *object;
    struct qt602240_object *object_key;
	struct qt602240_data *data = container_of(work, struct qt602240_data, work);
	struct device *dev = &data->client->dev;
//	static u32 count = 0;
	u8 cal;

//	if( ( count % 200 ) == 0 )
//		printk(KERN_ERR "atmel tw genates %d interrupts!\n",count);
//	count++;

//	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

    //yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_ts_probe interrupt\n");
    //disable_irq_nosync(data->irq);
	do {
		//yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_do while\n");
		if (qt602240_read_message(data, &message)) {
			yl_touch_debug(LOG_DEBUG, "Failed to read message\n");
			break;
			//goto end;
		}
		//yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_do reportid\n");
		reportid = message.reportid;

		cal = (message.message[0] & 0x10);
		if( ( is_stable_status == 1 ) && ( reportid == 0x01 ) && ( cal != 0 ) )
		{
			yl_touch_debug(LOG_WARNING, "111111111111111\n");
			
			is_stable_status = 0;

			enter_release_timer = 1;

/*			printk(KERN_ALERT "111111111111111\n");

			qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 0);
			qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
			qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
			qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);*/

			waterproofalgorithm(data);

//			mod_timer(&tw_cal_timer, jiffies+HZ);
			break;
		}/**/

		/* whether reportid is thing of QT602240_TOUCH_MULTI */
		object = qt602240_get_object(data, QT602240_TOUCH_MULTI);
		if (!object)
			//goto end;
			break;

        /* whether reportid is thing of QT602240_TOUCH_KEYARRAY */
        object_key = qt602240_get_object(data, QT602240_TOUCH_KEYARRAY);
		if (!object_key)
			//goto end;
			break;


		max_reportid = object->max_reportid;
		min_reportid = max_reportid - object->num_report_ids + 1;
		id = reportid - min_reportid;
        reportid_key = (object_key)->max_reportid;
//		yl_touch_debug(LOG_DEBUG, "max_reportid = %d min_reportid = %d reportid = %d reportid_key = %d\n",max_reportid, min_reportid, reportid, reportid_key);

		if (reportid >= min_reportid && reportid <= max_reportid)
		{
//			yl_touch_debug(LOG_DEBUG, TW_TAG "atmel input touchevent\n");
			qt602240_input_touchevent(data, &message, id);
		}
		else
		{   
			if (reportid == reportid_key)                                               //  6.28
		 		  //yl_touch_debug(LOG_DEBUG, TW_TAG "atmel input key_touch messages\n");       //  6.28
		          qt602240_input_keyevent(data, &message);	     //  6.28
            else
		          qt602240_dump_message(dev, &message);
		}
	} while (reportid != 0xff);

      enable_irq(data->client->irq);
}

static irqreturn_t qt602240_interrupt(int irq, void *dev_id)
{
	struct qt602240_data *data = dev_id;

	//yl_touch_debug(LOG_DEBUG, "atmel_irq\n");
	disable_irq_nosync(data->client->irq);
	queue_work(atmel_wq, &data->work);
	
	return IRQ_HANDLED;
}

static int qt602240_check_reg_init(struct qt602240_data *data)
{
	int i, j;
	u8 *init_vals;
	int index = 0;
	u8 version = data->info.version;
	struct qt602240_object *object;
//	struct device *dev = &data->client->dev;

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	switch (version) {
	case QT602240_VER_16:
		if( atmel_kind == 0x60 ) 
		{     

			yl_touch_debug(LOG_WARNING, "truly init vals\n");
			init_vals = (u8 *)init_vals_ver_16_truly;
		}
		else
		{     
			yl_touch_debug(LOG_WARNING, "ofilm init vals\n");
			init_vals = (u8 *)init_vals_ver_16_ofilm;
		}
		break;
	case QT602240_VER_20:
		init_vals = (u8 *)init_vals_ver_20;
		break;
	case QT602240_VER_21:
		init_vals = (u8 *)init_vals_ver_21;
		break;
	case QT602240_VER_22:
		yl_touch_debug(LOG_DEBUG, "QT602240_VER_22 \n");
		init_vals = (u8 *)init_vals_ver_22;
        break;
    case QT602240_VER_32: 
        yl_touch_debug(LOG_DEBUG, "QT602240_VER_32 \n");                                          //shezhi 7.18
		init_vals = (u8 *)init_vals_ver_32;        
		break;
	default:
		yl_touch_debug(LOG_DEBUG, "Firmware version %d doesn't support\n", version);
		return -EINVAL;
	}

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;

		if (!qt602240_object_writable(object->type))
			continue;

		for (j = 0; j < object->size + 1; j++)
			qt602240_write_object(data, object->type, j,
					init_vals[index + j]);

		index += object->size + 1;
	}

	return 0;
}

static int qt602240_check_matrix_size(struct qt602240_data *data)
{
	int mode = -1;
	int error;
	u8 val;
//	struct device *dev = &data->client->dev;
	const struct qt602240_platform_data *pdata = data->pdata;

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);
	yl_touch_debug(LOG_DEBUG, "Number of X lines: %d\n", pdata->x_line);
	yl_touch_debug(LOG_DEBUG, "Number of Y lines: %d\n", pdata->y_line);

	switch (pdata->x_line) {
	case 0 ... 15:
		if (pdata->y_line <= 14)
			mode = 0;
		break;
	case 16:
		if (pdata->y_line <= 12)
			mode = 1;
		if (pdata->y_line == 13 || pdata->y_line == 14)
			mode = 0;
		break;
	case 17:
		if (pdata->y_line <= 11)
			mode = 2;
		if (pdata->y_line == 12 || pdata->y_line == 13)
			mode = 1;
		mode = 1;
		break;
	case 18:
		if (pdata->y_line <= 10)
			mode = 3;
		if (pdata->y_line == 11 || pdata->y_line == 12)
			mode = 2;
		break;
	case 19:
		if (pdata->y_line <= 9)
			mode = 4;
		if (pdata->y_line == 10 || pdata->y_line == 11)
			mode = 3;
		break;
	case 20:
		mode = 4;
	}

	if (mode < 0) {
		yl_touch_debug(LOG_DEBUG, "Invalid X/Y lines\n");
		return -EINVAL;
	}

	error = qt602240_read_object(data, QT602240_SPT_CTECONFIG,
				QT602240_CTE_MODE, &val);
    yl_touch_debug(LOG_DEBUG, "mode= %d,&val= %d\n",mode,val); //sz
	if (error)
		return error;

	if (mode == val)
		return 0;

	/* Change the CTE configuration */
	qt602240_write_object(data, QT602240_SPT_CTECONFIG,
			QT602240_CTE_CTRL, 1);
	qt602240_write_object(data, QT602240_SPT_CTECONFIG,
			QT602240_CTE_MODE, mode);
	qt602240_write_object(data, QT602240_SPT_CTECONFIG,
			QT602240_CTE_CTRL, 0);

	return 0;
}

#if 0
static int qt602240_make_highchg(struct qt602240_data *data)
{
	struct device *dev = &data->client->dev;
	int count = 10;
	int error;
	u8 val;

	/* Read dummy message to make high CHG pin */
	do {
		error = qt602240_read_object(data, QT602240_GEN_MESSAGE, 0, &val);
		if (error)
			return error;
	} while ((val != 0xff) && --count);

	if (!count) {
		dev_err(dev, "CHG pin isn't cleared\n");
		return -EBUSY;
	}

	return 0;
}
#endif

/*********************shezhi*********************************/
#if 1
static void qt602240_handle_pdata(struct qt602240_data *data)
{
	u8 voltage;
	const struct qt602240_platform_data *pdata = data->pdata;

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);


	/* Set touchscreen lines */
	qt602240_write_object(data, QT602240_TOUCH_MULTI, QT602240_TOUCH_XSIZE,
			pdata->x_line);
	qt602240_write_object(data, QT602240_TOUCH_MULTI, QT602240_TOUCH_YSIZE,
			pdata->y_line);

	/* Set touchscreen orient */
//	qt602240_write_object(data, QT602240_TOUCH_MULTI, QT602240_TOUCH_ORIENT,
//			pdata->orient);

	/* Set touchscreen burst length */
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_BLEN, pdata->blen);

	/* Set touchscreen threshold */
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_TCHTHR, pdata->threshold);

	/* Set touchscreen resolution */
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_XRANGE_LSB, (pdata->x_size - 1) & 0xff);
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_XRANGE_MSB, (pdata->x_size - 1) >> 8);
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_YRANGE_LSB, (pdata->y_size - 1) & 0xff);
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_YRANGE_MSB, (pdata->y_size - 1) >> 8);

	/* Set touchscreen voltage */
	if (data->info.version >= QT602240_VER_21 && pdata->voltage) {
		if (pdata->voltage < QT602240_VOLTAGE_DEFAULT) {
			voltage = (QT602240_VOLTAGE_DEFAULT - pdata->voltage) /
				QT602240_VOLTAGE_STEP;
			voltage = 0xff - voltage + 1;
		} else
			voltage = (pdata->voltage - QT602240_VOLTAGE_DEFAULT) /
				QT602240_VOLTAGE_STEP;

		qt602240_write_object(data, QT602240_SPT_CTECONFIG,
				QT602240_CTE_VOLTAGE, voltage);
	}
}
#endif

///***********************************shezhi *****************************/
static int qt602240_get_info(struct qt602240_data *data)
{
	int error;
	u8 val;
	struct i2c_client *client = data->client;
	struct qt602240_info *info = &data->info;

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	error = qt602240_read_reg(client, QT602240_FAMILY_ID, &val);
	if (error)
		return error;
	info->family_id = val;

	error = qt602240_read_reg(client, QT602240_VARIANT_ID, &val);
	if (error)
		return error;
	info->variant_id = val;

	error = qt602240_read_reg(client, QT602240_VERSION, &val);
	if (error)
		return error;
	info->version = val;

	error = qt602240_read_reg(client, QT602240_BUILD, &val);
	if (error)
		return error;
	info->build = val;

	error = qt602240_read_reg(client, QT602240_OBJECT_NUM, &val);
	if (error)
		return error;
	info->object_num = val;

	return 0;
}

static int qt602240_get_object_table(struct qt602240_data *data)
{
	int error;
	int i;
	u16 reg;
	u8 reportid = 0;
	u8 buf[QT602240_OBJECT_SIZE];
    u8 min_reportid = 0;//tl

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);
	yl_touch_debug(LOG_DEBUG, "\n------atmel get object table--------\n");
	yl_touch_debug(LOG_DEBUG, "data->info.object_num =%d\n",data->info.object_num);
	for (i = 0; i < data->info.object_num; i++) {
		struct qt602240_object *object = data->object_table + i;
		reg = QT602240_OBJECT_START + QT602240_OBJECT_SIZE * i;
		error = qt602240_read_object_table(data->client, reg, buf);
		if (error)
			return error;
        min_reportid = 0;//tl
		object->type = buf[0];
		object->start_address = (buf[2] << 8) | buf[1];
		object->size = buf[3];
		object->instances = buf[4];
		object->num_report_ids = buf[5];

        yl_touch_debug(LOG_DEBUG, "object->instances = %4d,object->start_address = %4d,object->size = %4d\n",
														object->instances,object->start_address,object->size);   //sz  
		
		if (object->num_report_ids) {
			reportid += object->num_report_ids *
					(object->instances + 1);
			object->max_reportid = reportid;
            min_reportid = object->max_reportid - object->num_report_ids + 1;
		}
        yl_touch_debug(LOG_DEBUG, "objectNum:%4d,object->type:%4d,object->max_reportid =%4d,min_reportid = %4d\n",
													i,object->type,object->max_reportid,min_reportid); //sz 
	}
	yl_touch_debug(LOG_DEBUG, "-------------end--------------\n\n");

	return 0;
}

static int qt602240_initialize(struct qt602240_data *data)
{
	int error;
	u8 val;
	struct i2c_client *client = data->client;
	struct qt602240_info *info = &data->info;

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	error = qt602240_get_info(data);
	if (error)
		return error;

	yl_touch_debug(LOG_DEBUG, 
			"Family ID: %d Variant ID: %d Version: %d Build: %d\n",
			info->family_id, info->variant_id, info->version,
			info->build);

	data->object_table = kcalloc(info->object_num,
				     sizeof(struct qt602240_data),
				     GFP_KERNEL);

	if (!data->object_table) {
		yl_touch_debug(LOG_DEBUG, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	/* Get object table information */
	error = qt602240_get_object_table(data);
	if (error)
		return error;


	qt602240_read_object(data, QT602240_SPT_USERDATA,0, &atmel_kind);
	printk(KERN_ALERT "atmel_kind:%d\n",atmel_kind);


    //yl_touch_debug(LOG_DEBUG, TW_TAG "atmel_ts_probe get_object_table\n");
	/* Check register init values */
	error = qt602240_check_reg_init(data);
	if (error)
		return error;

	/* Check X/Y matrix size */
	error = qt602240_check_matrix_size(data);
	if (error)
		return error;

#if 0
	error = qt602240_make_highchg(data);
	if (error)
		return error;
#endif

	qt602240_handle_pdata(data);

#if 1
	/* Backup to memory */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_BACKUPNV,
			QT602240_BACKUP_VALUE);
	msleep(QT602240_BACKUP_TIME);
#endif

	/* Soft reset */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_RESET, 1);
	msleep(QT602240_RESET_TIME);    //3.7


	qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);  //2012.3.2

    //qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
    //qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
    //qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
	/* Update matrix size at info struct */
	error = qt602240_read_reg(client, QT602240_MATRIX_X_SIZE, &val);
	if (error)
		return error;
	info->matrix_xsize = val;

	error = qt602240_read_reg(client, QT602240_MATRIX_Y_SIZE, &val);
	if (error)
		return error;
	info->matrix_ysize = val;

	yl_touch_debug(LOG_DEBUG, 
			"Matrix X Size: %d Matrix Y Size: %d Object Num: %d\n",
			info->matrix_xsize, info->matrix_ysize,
			info->object_num);
     
	return 0;
}

static ssize_t qt602240_object_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	int count = 0;
	int i, j;
	int error;
	u8 val;
	struct qt602240_object *object;
	struct qt602240_data *data = dev_get_drvdata(dev);

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;

		count += sprintf(buf + count,
				"Object Table Element %d(Type %d)\n",
				i + 1, object->type);

		if (!qt602240_object_readable(object->type)) {
			count += sprintf(buf + count, "\n");
			continue;
		}

		for (j = 0; j < object->size + 1; j++) {
			error = qt602240_read_object(data,
						object->type, j, &val);
			if (error)
				return error;

			count += sprintf(buf + count,
					"  Byte %d: 0x%x (%d)\n", j, val, val);
		}

		count += sprintf(buf + count, "\n");
	}

	return count;
}

static int qt602240_load_fw(struct device *dev, const char *fn)
{
	int ret;
	unsigned int frame_size;
	unsigned int pos = 0;
	struct qt602240_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	const struct firmware *fw = NULL;

	ret = request_firmware(&fw, fn, dev);
	if (ret) {
		yl_touch_debug(LOG_DEBUG, "Unable to open firmware %s\n", fn);
		return ret;
	}

	/* Change to the bootloader mode */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_RESET, QT602240_BOOT_VALUE);
	msleep(QT602240_RESET_TIME);

	/* Change to slave address of bootloader */
	if (client->addr == QT602240_APP_LOW)
		client->addr = QT602240_BOOT_LOW;
	else
		client->addr = QT602240_BOOT_HIGH;

	ret = qt602240_check_bootloader(client, QT602240_WAITING_BOOTLOAD_CMD);
	if (ret)
		goto out;

	/* Unlock bootloader */
	qt602240_unlock_bootloader(client);

	while (pos < fw->size) {
		ret = qt602240_check_bootloader(client,
						QT602240_WAITING_FRAME_DATA);
		if (ret)
			goto out;

		frame_size = ((*(fw->data + pos) << 8) | *(fw->data + pos + 1));

		/* We should add 2 at frame size as the the firmware data is not
		 * included the CRC bytes.
		 */
		frame_size += 2;

		/* Write one frame to device */
		qt602240_fw_write(client, fw->data + pos, frame_size);

		ret = qt602240_check_bootloader(client,
						QT602240_FRAME_CRC_PASS);
		if (ret)
			goto out;

		pos += frame_size;

		yl_touch_debug(LOG_DEBUG, "Updated %d bytes / %zd bytes\n", pos, fw->size);
	}

out:
	release_firmware(fw);

	/* Change to slave address of application */
	if (client->addr == QT602240_BOOT_LOW)
		client->addr = QT602240_APP_LOW;
	else
		client->addr = QT602240_APP_HIGH;

	return ret;
}

static ssize_t qt602240_update_fw_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned int version;
	int error;
	struct qt602240_data *data = dev_get_drvdata(dev);

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	if (sscanf(buf, "%u", &version) != 1) {
		yl_touch_debug(LOG_DEBUG, "Invalid values\n");
		return -EINVAL;
	}

	if (data->info.version < QT602240_VER_21 || version < QT602240_VER_21) {
		yl_touch_debug(LOG_DEBUG, "FW update supported starting with version 21\n");
		return -EINVAL;
	}

	disable_irq(data->irq);

	error = qt602240_load_fw(dev, QT602240_FW_NAME);
	if (error) {
		yl_touch_debug(LOG_DEBUG, "The firmware update failed(%d)\n", error);
		count = error;
	} else {
		yl_touch_debug(LOG_DEBUG, "The firmware update succeeded\n");

		/* Wait for reset */
		msleep(QT602240_FWRESET_TIME);

		kfree(data->object_table);
		data->object_table = NULL;

		qt602240_initialize(data);
	}

	enable_irq(data->irq);

	return count;
}

/*static DEVICE_ATTR(object, 0444, qt602240_object_show, NULL);
static DEVICE_ATTR(update_fw, 0664, NULL, qt602240_update_fw_store);

static struct attribute *qt602240_attrs[] = {
	&dev_attr_object.attr,
	&dev_attr_update_fw.attr,
	NULL
};

static const struct attribute_group qt602240_attr_group = {
	.attrs = qt602240_attrs,
};*/

static void qt602240_start(struct qt602240_data *data)
{
	/* Touch enable */
//	qt602240_write_object(data,QT602240_GEN_POWER,QT602240_POWER_IDLEACQINT, 0x20);
//	qt602240_write_object(data,QT602240_GEN_POWER,QT602240_POWER_ACTVACQINT, 0x0a);

//	qt602240_write_object(data,QT602240_GEN_POWER,QT602240_POWER_IDLEACQINT, 0xff);
//	qt602240_write_object(data,QT602240_GEN_POWER,QT602240_POWER_ACTVACQINT, 0xff);

	if( atmel_kind == 0x60 ) 
	{ 
		qt602240_write_object(data,QT602240_GEN_POWER,QT602240_POWER_IDLEACQINT, 20);
		qt602240_write_object(data,QT602240_GEN_POWER,QT602240_POWER_ACTVACQINT, 12);
	}
	else
	{
		qt602240_write_object(data,QT602240_GEN_POWER,QT602240_POWER_IDLEACQINT, 255);
		qt602240_write_object(data,QT602240_GEN_POWER,QT602240_POWER_ACTVACQINT, 255);
	}


	//qt602240_write_object(data,QT602240_TOUCH_MULTI, QT602240_TOUCH_CTRL, 0x83);
	//qt602240_write_object(data,QT602240_TOUCH_KEYARRAY, QT602240_TOUCH_CTRL, 0x03);
        
}

static void qt602240_stop(struct qt602240_data *data)
{
	/* Touch disable */
	//qt602240_write_object(data,QT602240_TOUCH_MULTI, QT602240_TOUCH_CTRL, 0);
	//qt602240_write_object(data,QT602240_TOUCH_KEYARRAY, QT602240_TOUCH_CTRL, 0);
	qt602240_write_object(data,QT602240_GEN_POWER,QT602240_POWER_IDLEACQINT, 0);
	qt602240_write_object(data, QT602240_GEN_POWER,QT602240_POWER_ACTVACQINT, 0);
}

static int qt602240_input_open(struct input_dev *dev)
{
	struct qt602240_data *data = input_get_drvdata(dev);

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	qt602240_start(data);

	return 0;
}

static void qt602240_input_close(struct input_dev *dev)
{
	struct qt602240_data *data = input_get_drvdata(dev);

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	qt602240_stop(data);
}

#if 0
static qt602240_interrupt_gpiocfg()
{
	int error=0;
        error = gpio_request(TOUCH_INT_GPIO, "touch");
            if(error<0)
            return error;
        error = gpio_direction_input(TOUCH_INT_GPIO);  
            if(error<0)
            gpio_free(TOUCH_INT_GPIO);      
}
#endif



/*******************************************************
   ********  1--active  0--not active***********
*******************************************************/
int qt602240_active(void)				   
{
   yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);
   return atmel_tw_is_active_status;
}

#define CONFIG_FILE_VERSION 0X02

/*******************************************************
***********check firmware if need update***********
*******************************************************/
int qt602240_firmware_need_update(void)
{
	u8 val;
	int ret = -1;

	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

	return 0;

	ret = qt602240_read_object(data, QT602240_SPT_USERDATA, 1, &val);
	if( ret )
		return -1;

	printk(KERN_ALERT "atmel tw config version = 0x%x \n", val);

	if (val != CONFIG_FILE_VERSION )
	{
		printk(KERN_ALERT "need update new version cfg\n");
		return 1;
	}
	else
	{
		printk(KERN_ALERT "the current version cfg 0x%x is latest.\n", val);
		return 0;
	}

//	return 0;
}

/*******************************************************
*********************do firmware update ***************
*******************************************************/
int qt602240_firmware_do_update(void)	  
{
	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

	if( qt602240_check_reg_init(data) == 0)
	{
		printk(KERN_ALERT "start to update config params\n");
		/* Backup to memory */
		qt602240_write_object(data, QT602240_SPT_USERDATA, 1, CONFIG_FILE_VERSION);

		qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_BACKUPNV,QT602240_BACKUP_VALUE);
		mdelay(QT602240_BACKUP_TIME);

		qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_RESET, 1);
		mdelay(QT602240_RESET_TIME); 
		return 0;
	}
	else
	{
		return -1;
	}

//	return 0;
}

extern int yl_params_kernel_read(char *buf,size_t count);
extern int yl_params_kernel_write(const char *buf,size_t count);
u8 atmel_tw_is_calibrated(void)
{
	char deviceinfo[512] = "DEVICE";
	device_info_type *dit;

	dit = (device_info_type*)deviceinfo;

	yl_params_kernel_read(deviceinfo, 512);

//	printk(KERN_ERR "%d,%c\n",460,deviceinfo[460]);
	printk(KERN_ALERT "%c\n",dit->twcalflag);	

	return 1;

	if( ( dit -> twcalflag ) == '3')
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

u8 atmel_tw_is_calibrated_1(void)
{
	char deviceinfo[512] = "DEVICE";

	yl_params_kernel_read(deviceinfo, 512);

	printk(KERN_ERR "%d,%c\n",460,deviceinfo[460]);

	if( deviceinfo[460] == '3')
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

u8 atmel_write_tw_cal_flag(void)
{
	char deviceinfo[512] = "DEVICE";
	device_info_type *dit;

	dit = (device_info_type*)deviceinfo;

	printk(KERN_ERR "%s\n",__func__);

	yl_params_kernel_read(deviceinfo, 512);
	dit -> twcalflag = '3';
//	deviceinfo[460] = '2';
	yl_params_kernel_write(deviceinfo,512);

	return 0;
}

/*******************************************************
*******************check if need calibrate***********
*******************************************************/

int qt602240_need_calibrate(void)				       
{
   yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

   return atmel_tw_is_calibrated();
}

/*******************************************************
 ******************system write "calibrate"************
*******************************************************/

int qt602240_calibrate(void)				      
{
	printk(KERN_ERR "%s\n",__func__);

	msleep(2000);

	disable_irq_nosync(data->irq);

	if( is_stable_status == 0 )
	{
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 255);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 1);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 0);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
	}

	// Calibrate 
	qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
	msleep(40);

	if( touch_has_water( data ) == 1 )
	{
		printk(KERN_ALERT "@@@@@\n");
		enable_irq(data->irq);
		return -1; 
	}

	if( write_tw_reference_data() == -1 )
	{
		printk(KERN_ALERT "~~~~\n");
		enable_irq(data->irq);
		return -1; 
	}

	printk(KERN_ERR "[Atmel]: Calibrate ok!\n");

	atmel_write_tw_cal_flag();

	if( is_stable_status == 0 )
	{
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
	}

	enable_irq(data->irq);

   	return 0;
}


/*******************************************************
 ******************get firmware version **************
*******************************************************/
int qt602240_get_firmware_version(char * version )
{
	u8 val;
    unsigned int uc_fw_version;
	struct i2c_client *client = data->client;
	int ret = -1;

	ret = qt602240_read_reg(client, QT602240_VERSION, &val);
	if( ret )
		return -1;
	
	uc_fw_version = val;
	
	return sprintf(version, "%s(0x%x):%d","Atmel:224E:V",uc_fw_version,0);
}

/*******************************************************
  ******************system write "reset"***************
*******************************************************/
int qt602240_reset_touchscreen(void)	
{
    yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

	disable_irq_nosync(data->irq);

	/* Soft reset */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_RESET, 1);
	msleep(QT602240_RESET_TIME);

	yl_touch_debug(LOG_DEBUG, "[Atmel]: Reset CTPM ok!\n");

	enable_irq(data->irq);
	return 1;
}

/*******************************************************
  ******************"handwrite" "normal" *************
*******************************************************/
touch_mode_type qt602240_get_mode(void)			    
{
   yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

   return MODE_NORMAL;
}

int qt602240_set_mode(touch_mode_type work_mode)			 
{
	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

    return MODE_NORMAL;
}

/*******************************************************
  ****************get "oreitation:X" ************
*******************************************************/
touch_oreitation_type qt602240_get_oreitation(void)				      
{
   yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

   return OREITATION_0 ;
}


int qt602240_set_oreitation(touch_oreitation_type oreitate)				    
{
   yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

   return OREITATION_0 ;
}


/*******************************************************
  ***************tw debug on or off *************
*******************************************************/
int qt602240_debug(int val)				
{
	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

   	return 1;
}


touchscreen_ops_tpye atmel_synaptics_ops=
{
	.touch_id					= 0,		
	.touch_type					= 1,
	.active						= qt602240_active,
	.firmware_need_update		= qt602240_firmware_need_update,
	.firmware_do_update			= qt602240_firmware_do_update,
	.need_calibrate				= qt602240_need_calibrate,
	.calibrate					= qt602240_calibrate,
	.get_firmware_version		= qt602240_get_firmware_version,
	.reset_touchscreen			= qt602240_reset_touchscreen,
	.get_mode					= qt602240_get_mode,
	.set_mode					= qt602240_set_mode,
	.get_oreitation				= qt602240_get_oreitation,
	.set_oreitation				= qt602240_set_oreitation,
	.read_regs					= NULL,
	.write_regs					= NULL,
	.debug						= qt602240_debug,
};


u8 old_cal_gen_data_and_copy_to_bakup(u16 crc_old,u16 crc_new,char info[])
{
	device_info_type *dit;
	char devinfo[512] = "DEVICE";

	yl_params_kernel_read(devinfo, 512);

	dit = (device_info_type*)devinfo;

	yl_touch_debug(LOG_WARNING, "\n%c\n",dit->twcalflag);	

	if( ( dit -> twcalflag ) != '3' )
	{
//		printk(KERN_ALERT "ggggggg\n");

		dit->twcalflag = '3';
		yl_params_kernel_write(devinfo,512);


		memcpy(&info[450],&crc_new,2);
		yl_params_kernel_write(info,512);

//		printk(KERN_ALERT "sssssssss\n");

		if( atmel_kind == 0x60 ) 
		{  
			memcpy(info,"baktwref1",10);
			yl_params_kernel_write(info,512);
		}
		else
		{  
			memcpy(info,"baktwref",9);
			yl_params_kernel_write(info,512);
		}

//		printk(KERN_ALERT "aaaaaaaaaaa\n");
		
		return 0;
	}
	else
		return 1;
}


int read_tw_reference_data_one(char *tw,char twinfo[])//read data into twinfo then put data into T37_ref_data;
{
	int i;
	u16 * point;
	u16 crcdata = 0;

//	printk(KERN_ALERT "%s\n",tw);

	memcpy(twinfo,tw,strlen(tw) + 1);
	yl_params_kernel_read(twinfo, 512);
	point = ( u16* )&twinfo[10];

	for( i = 0;i < 220;i++)
	{
		if( (*point) == 65535 )
		{
			yl_touch_debug(LOG_WARNING, "%s,%d\n",tw,*point);
			return -1;
		}
		T37_ref_data[i] = *point;
		//crcdata += T37_ref_data[i];
		yl_touch_debug(LOG_WARNING, "%s,i:%d,ref:%d\n",tw,i,T37_ref_data[i]);
		point++;
	}
	crcdata = crc8_accumulate(0, &twinfo[10], 440);

	yl_touch_debug(LOG_WARNING, "read crcdata:%d,*point:%d\n",(u16)(crcdata),(*point));
	if( (*point) != crcdata )
	{
		yl_touch_debug(LOG_WARNING, "*point:%d\n",*point);
		if( old_cal_gen_data_and_copy_to_bakup(*point,crcdata,twinfo) == 0 )
			return 0;
		return -1;
	}

	return 0;
}

int write_tw_reference_data_one(char *str1,char *str2)
{
	int i;
	u16 * point;
	u16 crcdata = 0;
	u8 count = 5;
	char twinfo[512];
	char twinfo_1[512];

//	printk(KERN_ALERT "%s,%s\n",str1,str2);
 
	memcpy(twinfo,str1,strlen(str1) + 1);
	point = ( u16* )&twinfo[10];

	for( i = 0;i < 220;i++)
	{
		yl_touch_debug(LOG_WARNING, "%d\n",T37_ref_data[i]);
		*point = T37_ref_data[i];
		point++;
		//crcdata += T37_ref_data[i];
	}
	crcdata = crc8_accumulate(0, &twinfo[10], 440);
	
	yl_touch_debug(LOG_WARNING, "write crcdata:%d\n",crcdata);
	*point = crcdata; 

	while(count > 0)
	{	
		yl_params_kernel_write(twinfo,512);
		if( read_tw_reference_data_one(str1,twinfo_1) == 0 )
			break;
		count--;
	}
	yl_touch_debug(LOG_WARNING, "count 1:%d\n",count);
	if(count == 0)
		return -1;

	count = 5;
	memcpy(twinfo,str2,strlen(str2)+1);
	while(count > 0)
	{
		yl_params_kernel_write(twinfo,512);
		if( read_tw_reference_data_one(str2,twinfo_1) == 0 )
			break;
		count--;
	}
	yl_touch_debug(LOG_WARNING, "count 2:%d\n",count);
	if(count == 0)
		return -1;
	else
		return 0;
}

int write_tw_reference_data(void)
{
	if( atmel_kind == 0x60 ) 
	{ 
		if( write_tw_reference_data_one("tw_ref1","baktwref1") == 0 )
		{
			yl_touch_debug(LOG_WARNING, "write tw_ref1\n");
			return 0;
		}
		else
		{
			yl_touch_debug(LOG_WARNING, "write tw_ref1 error\n");
			return -1;
		}
	}
	else
	{
		if( write_tw_reference_data_one("tw_ref","baktwref") == 0 )
//		if( write_tw_reference_data_one("tw_ref","tw_ref") == 0 )
		{
			yl_touch_debug(LOG_WARNING, "write tw_ref\n");
			return 0;
		}
		else
		{
			yl_touch_debug(LOG_WARNING, "write tw_ref error\n");
			return -1;
		}
	}
}



/*u8 write_tw_reference_data(void)
{
	int i;
	u16 * point;
	u16 crcdata = 0;
	u8 count = 5;

	if( atmel_kind == 0x60 ) 
	{   
		yl_touch_debug(LOG_WARNING, "truly write tw ref\n");
		char twinfo[512] = "tw_ref1";
		char twinfo_1[512] = "tw_ref1";
		point = ( u16* )&twinfo[10];

		for( i = 0;i < 220;i++)
		{
			yl_touch_debug(LOG_WARNING, "%d\n",T37_ref_data[i]);
			*point = T37_ref_data[i];
			point++;
			crcdata += T37_ref_data[i];
		}
		*point = crcdata; 
	
		while(count > 0)
		{	
			yl_params_kernel_write(twinfo,512);
			if( read_tw_reference_data_one("tw_ref1",twinfo_1) == 0 )
				break;
			count--;
		}

		count = 5;
		memcpy(twinfo,"baktwref1",10);
		while(count > 0)
		{
			yl_params_kernel_write(twinfo,512);
			if( read_tw_reference_data_one("baktwref1",twinfo_1) == 0 )
				break;
			count--;
		}
	}
	else
	{   
		yl_touch_debug(LOG_WARNING, "ofilm write tw ref\n");
		char twinfo[512] = "tw_ref";
		char twinfo_1[512] = "tw_ref";
		point = ( u16* )&twinfo[10];

		for( i = 0;i < 220;i++)
		{
			yl_touch_debug(LOG_WARNING, "%d\n",T37_ref_data[i]);
			*point = T37_ref_data[i];
			point++;
			crcdata += T37_ref_data[i];
		}
		*point = crcdata; 

		while(count > 0)
		{	
			yl_params_kernel_write(twinfo,512);
			if( read_tw_reference_data_one("tw_ref",twinfo_1) == 0 )
				break;
			count--;
		}

		count = 5;
		memcpy(twinfo,"baktwref",9);
		while(count > 0)
		{
			yl_params_kernel_write(twinfo,512);
			if( read_tw_reference_data_one("baktwref",twinfo_1) == 0 )
				break;
			count--;
		}
	}

	return 0;
}*/

typedef struct tw_map_data
{
	char tag[10];
	u16  data[0];
}TW_MAP_DATA;

int tw_cal_data_valid(char *str,char twdata[])   // 1 is valid;0 is not valid
{
	char tw[512] = {0,};
	u16  crcdata = *( (u16*)&twdata[450] );

//	printk(KERN_ALERT "crcdata:%d\n",crcdata);

	memcpy(tw,str,strlen(str) + 1);
	yl_params_kernel_read(tw, 512);
	TW_MAP_DATA * pp = (TW_MAP_DATA *)tw;

	yl_touch_debug(LOG_WARNING, "%s,%d\n", pp->tag,strcmp(pp->tag, str) );

	if( ( strcmp(pp->tag, str) == 0 ) && ( (pp -> data)[0] < 40000 )  && ( (pp -> data)[220] == crcdata ) )
	{
		yl_touch_debug(LOG_WARNING, "bakup data is valid\n");
		return 1;
	}
	else
	{
		yl_touch_debug(LOG_WARNING, "bakup data is not valid\n");
		return 0;
	}
}




int read_tw_reference_data(void)
{
	char twinfo[512];
	u8 count = 5;
	u8 tt = 3;

	if( atmel_kind == 0x60 ) 
	{ 	
		if( read_tw_reference_data_one("tw_ref1",twinfo) == 0 )
		{
//			printk(KERN_ALERT "read tw_ref1\n");

//			read_tw_reference_data_one("baktwref1",twinfo);//deleted after test 
			memcpy(twinfo,"baktwref1",10);
			while( ( ( tt = tw_cal_data_valid("baktwref1",twinfo) ) == 0 ) && ( count > 0 ) )
			{
				yl_params_kernel_write(twinfo,512);
				count --;
			}
			if( (tt == 0) && ( count == 0 ) )
				yl_touch_debug(LOG_WARNING, "Copy file fail from tw_ref1 to baktwref1\n");
			return 0;
		}
		else if( read_tw_reference_data_one("baktwref1",twinfo) == 0 )
		{
			yl_touch_debug(LOG_WARNING, "read baktwref1\n");
			memcpy(twinfo,"tw_ref1",8);
			yl_params_kernel_write(twinfo,512);
			return 0;
		}
		else
		{
			yl_touch_debug(LOG_WARNING, "read truly error from tw_ref1 and baktwref1\n");		
			return -1;
		}
	}
	else
	{
		if( read_tw_reference_data_one("tw_ref",twinfo) == 0 )
		{
//			printk(KERN_ALERT "read tw_ref\n");

                                       
//			read_tw_reference_data_one("baktwref",twinfo);//deleted after test 
			memcpy(twinfo,"baktwref",9);
			while( ( ( tt = tw_cal_data_valid("baktwref",twinfo) ) == 0 ) && ( count > 0 ) )
			{
				yl_params_kernel_write(twinfo,512);
				count --;
			}
			if( (tt == 0) && ( count == 0 ) )
				yl_touch_debug(LOG_WARNING, "Copy file fail from tw_ref to baktwref\n");
			return 0;
		}
		else if( read_tw_reference_data_one("baktwref",twinfo) == 0 )
		{
			yl_touch_debug(LOG_WARNING, "read baktwref\n");
			memcpy(twinfo,"tw_ref",8);
			yl_params_kernel_write(twinfo,512);
			return 0;
		}
		else
		{
			yl_touch_debug(LOG_WARNING, "read ofilm error from tw_ref and baktwref\n");
			return -1;
		}
	}	
}

/*u8 read_tw_reference_data(void)
{
	int i;
	u16 * point;
	u16 crcdata = 0;

	if( atmel_kind == 0x60 ) 
	{ 
		yl_touch_debug(LOG_WARNING, "truly read tw ref\n");
		char twinfo[512] = "tw_ref1";
		yl_params_kernel_read(twinfo, 512);
		point = ( u16* )&twinfo[10];

		for( i = 0;i < 220;i++)
		{
			T37_ref_data[i] = *point;
			if( T37_ref_data[i] == 65535 )
				return -1;
			crcdata += T37_ref_data[i];
			yl_touch_debug(LOG_WARNING, "i:%d,ref:%d\n",i,T37_ref_data[i]);
			point++;
		}
		if( (*point) != crcdata )
			return -1;
	}
	else
	{ 
		yl_touch_debug(LOG_WARNING, "ofilm read tw ref\n");
		char twinfo[512] = "tw_ref";
		yl_params_kernel_read(twinfo, 512);

		point = ( u16* )&twinfo[10];

		for( i = 0;i < 220;i++)
		{
			T37_ref_data[i] = *point;
			if( T37_ref_data[i] == 65535 )
				return -1;
			crcdata += T37_ref_data[i];
			yl_touch_debug(LOG_WARNING, "i:%d,ref:%d\n",i,T37_ref_data[i]);
			point++;
		}
		if( (*point) != crcdata )
			return -1;
	}
	return 0;
}*/

static u8 has_scheduled = 0;
static u8 enter_cal_mode = 0;

u32 touch_has_water(struct qt602240_data *data)
{
	u32 count = 1;
	u8 value1,value2;
	u8 page;
	int i,j=0;
	u8 cishu = 0;

	enter_cal_mode = 1;

	while( has_scheduled == 1 )
		msleep(1000);

start:
	while(cishu < 5)
	{
		count = 1;
		j = 0;

		printk(KERN_ALERT "==start==\n");

		qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
		msleep(40);

		qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x11);
		msleep(5);



		qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,1, &page);
		printk(KERN_ALERT "page:%d",page);

		for(;page >= 1;page--)
		{
			qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x02);
			msleep(10);
		}



		while( count <= 4 )
		{
			qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,1, &page);
			printk(KERN_ALERT "page:%d",page);
			for(i = 0;(i <= 63)&&(j < 220);i++)
			{

				if( atmel_kind == 0x60 ) 
				{ 
					if( ( table[j] & 0x2 ) == 0x2 )
					{
						qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,2 + i*2, &value1);
						qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,3 + i*2, &value2);	

						T37_ref_data[j] = value2*256+value1;
						if( T37_ref_data[j] < 20000 )
						{
							cishu ++;
							goto start;
						}
					}
					else
					{
						T37_ref_data[j] = 0;
					}
				}
				else
				{
					if( ( table[j] & 0x1 ) == 0x1 )
					{
						qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,2 + i*2, &value1);
						qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,3 + i*2, &value2);	

						T37_ref_data[j] = value2*256+value1;
						if( T37_ref_data[j] < 20000 )
						{
							cishu ++;						
							goto start;
						}
					}
					else
					{
						T37_ref_data[j] = 0;
					}
				}
				j++;
			}
			count++;
			qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x01);
			printk(KERN_ALERT "\n");	

			msleep(10);
		}

		enter_cal_mode = 0;
		return 0;
	}
	enter_cal_mode = 0;
	return 1;
}

char TPfacOccupWhichBit(char TP)
{
	if(TP == 0x60)   //truly
		return 0x2;
	else
		return 0x1;  //ofilm
}

void ReadCurData(struct qt602240_data *data,char TP,u16 *save)
{
	char bit,page;
	u8 value1,value2;
	char count = 1;
	u16  i,j = 0;

	bit = TPfacOccupWhichBit(TP);
//	printk(KERN_ERR "bit:%d",bit);

	qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x10);
	msleep(3);

	qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x11);
	msleep(3);

	while( count <= 4 )
	{
		qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,1, &page);
		yl_touch_debug(LOG_WARNING, "page:%d",page);

		for(i = 0;(i <= 63)&&(j < 220);i++)
		{
			if( ( table[j] & bit ) == bit )
			{
				qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,2 + i*2, &value1);
				qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,3 + i*2, &value2);	

				save[j] = value2*256 + value1;
//				printk(KERN_ERR "j:%d:%d",j,save[j]);
			} 
//			else
//				printk(KERN_ERR "j:%d:0",j);
			j++;
		}
		count++;
		qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x01);
		msleep(10);
	}
}
char CompareAndDecide(u16 *facval,s16 *curval,int num,char TP,int *minusmax,int *minusmin,int *positivemin,int *positivemax)//-5000,5000
{
	char old = 0,cur = 0;
	u16 i;
	int diff;
	char bit;
	char flag = 0;

//	printk(KERN_ERR "minusmax:%d,minusmin:%d,positivemin:%d,positivemax:%d\n",*minusmax,*minusmin,*positivemin,*positivemax);
	
	bit = TPfacOccupWhichBit(TP);

	for(i = 0;i < num;i++)
	{	
		if( ( table[i] & bit ) == bit )
		{
			diff = curval[i] - facval[i];
			//yl_touch_debug(LOG_WARNING, "diff:%d\n",diff);

			if( diff > 0 )
			{
				old = cur;
				cur = 1;
//				printk(KERN_ERR "i:%d,cur:%d,diff:%d,curval:%d,facval:%d\n",i,cur,diff,curval[i],facval[i]);

				if(diff < *positivemin)
					*positivemin = diff;
				else if(diff > *positivemax)
					*positivemax = diff;
			}
			else if( diff < 0 ) 
			{
				old = cur;
				cur = 2;
//				printk(KERN_ERR "i:%d,cur:%d,diff:%d,curval:%d,facval:%d\n",i,cur,diff,curval[i],facval[i]);

				if(diff > *minusmax)
					*minusmax = diff;
				else if(diff < *minusmin)
					*minusmin = diff;
			}

			if((i > 0) && (old != cur))
				flag = 1; 

			curval[i] = diff;
		}
//		else
//			printk(KERN_ERR "i:%d,cur:%d,diff:%d,curval:%d,facval:%d\n",i,0,0,0,0);
	}

//	printk(KERN_ERR "minusmax:%d,minusmin:%d,positivemin:%d,positivemax:%d\n",*minusmax,*minusmin,*positivemin,*positivemax);
	
	if(flag == 1)
		return 3;
	return cur;   
}

void allbinusval(s16 *save,s16 val,int num,char TP)
{
	int i;
	char bit;
	
	bit = TPfacOccupWhichBit(TP);

	printk(KERN_ERR "allbinusval:val:%d\n",val);

	for(i = 0;i < num;i++)
		if( ( table[i] & bit ) == bit )
		{
//			printk(KERN_ERR "save:%d before\n",save[i]);
			save[i] = save[i] - val ;
//			printk(KERN_ERR "save:%d after\n",save[i]);
		}
//		else
//			printk(KERN_ERR "hhhhhhhhhhhhhhhhhh\n");
}

u32 waterproofalgorithm(struct qt602240_data *data)
{
	u8 page;
	int i,j;
	u8 value1,value2;
	u16 current_value;
	u32 count = 1;
	u8 positive_flag = 0;
	j =  0;

	char retval = 0;

	int minusmax = -5000 ,positivemin = 5000;
	int minusmin = 0,     positivemax = 0;

	s16 savecurr[220] = {0,};

//	yl_touch_debug(LOG_WARNING, "has_scheduled:%d 1\n",has_scheduled);

	if( (has_scheduled == 1)  || (enter_cal_mode == 1) )
		return 1;

	has_scheduled = 1;

//	yl_touch_debug(LOG_WARNING, "has_scheduled:%d 2\n",has_scheduled);
	
	qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 255);
	qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 1);
	qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 0);
	qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
	msleep(3);

	ReadCurData(data,0x10,savecurr);
      
	retval = CompareAndDecide(T37_ref_data,savecurr,220,0x10,&minusmax,&minusmin,&positivemin,&positivemax);

	printk(KERN_ERR "minusmax:%d,minusmin:%d,positivemin:%d,positivemax:%d,retval:%d\n",minusmax,minusmin,positivemin,positivemax,retval);

	if(retval == 1)
		allbinusval(savecurr,positivemin,220,0x10);
	else if(retval == 2)
		allbinusval(savecurr,minusmax,220,0x10);

/*	int t;
	for(t = 0;t < 220;t++)
		printk(KERN_ERR "t:%d,%d\n",t,savecurr[t]);*/

	while( count <= 4 )
	{
		for(i = 0;(i <= 63)&&(j < 220);i++)
		{
			if( atmel_kind == 0x60 ) 
			{ 
				if( ( table[j] & 0x2 ) == 0x2 )
				{
					qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,2 + i*2, &value1);
					qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,3 + i*2, &value2);	
					current_value = value2*256 + value1;
					if( current_value > ( T37_ref_data[j] + 290 ) )
					{
						yl_touch_debug(LOG_WARNING, "j:%d,T37_ref_data[j]:%5d,current_value:%5d,diff:%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
						qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
						msleep(10);
						atmel_ts_release(data);
						has_scheduled = 0;

						qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
						qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
						qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
						qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
						yl_touch_debug(LOG_WARNING, "is_stable_status:%d -3\n",is_stable_status);
						return -3;
					}							
					else if( T37_ref_data[j] > ( current_value + 290 ) )
					{
						yl_touch_debug(LOG_WARNING, "j:%d,T37_ref_data[j]:%5d,current_value:%5d,diff:%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
						positive_flag = 1;		
					}									
				}
			}
			else  //ofilm
			{ 
				if( ( table[j] & 0x1 ) == 0x1 )
				{
					if( ( j == 179 ) || ( j == 192 ) || ( j == 205 ) || ( j == 218 ) ){
					
						if( savecurr[j] > 180  )
						{
//							printk(KERN_ERR "j:%3d,diff:%5d\n",j,savecurr[j]);
							yl_touch_debug(LOG_WARNING, "j:%3d,diff:%5d\n",j,savecurr[j]);
							qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
							msleep(10);
							atmel_ts_release(data);
							has_scheduled = 0;


							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
							yl_touch_debug(LOG_WARNING, "is_stable_status:%d -3\n",is_stable_status);
							return -3;
						}							
						else if( -savecurr[j] > 180 )
						{
//							printk(KERN_ERR "j:%3d,diff:%5d\n",j,savecurr[j]);
							yl_touch_debug(LOG_WARNING, "j:%3d,diff:%5d\n",j,savecurr[j]);
							positive_flag = 1;		
						}

					}
					else{
					
						if( savecurr[j] > 290 )
						{
//							printk(KERN_ERR "j:%3d,diff:%5d\n",j,savecurr[j]);
							yl_touch_debug(LOG_WARNING, "j:%3d,diff:%5d\n",j,savecurr[j]);
							qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
							msleep(10);
							atmel_ts_release(data);
							has_scheduled = 0;


							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
							yl_touch_debug(LOG_WARNING, "is_stable_status:%d -3\n",is_stable_status);
							return -3;
						}							
						else if( -savecurr[j] > 290 )
						{
//							printk(KERN_ERR "j:%3d,diff:%5d\n",j,savecurr[j]);							
							yl_touch_debug(LOG_WARNING, "j:%3d,diff:%5d\n",j,savecurr[j]);
							positive_flag = 1;		
						}

					}	//not key			
				}   //>0
			}	//ofilm		

			j++;
		}
		count++;
	}

//	yl_touch_debug(LOG_WARNING, "j=%d\n",j);
	if( tw_pressed == 1 )
	{
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);

		yl_touch_debug(LOG_WARNING, "is_stable_status:%d -2\n",is_stable_status);
		has_scheduled = 0;

//		yl_touch_debug(LOG_WARNING, "has_scheduled:%d 5\n",has_scheduled);
		return -2;
	}
	else if( positive_flag == 1 )
	{
		qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
		msleep(10);
		atmel_ts_release(data);

		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);

		yl_touch_debug(LOG_WARNING, "is_stable_status:%d -1\n",is_stable_status);
		has_scheduled = 0;

//		yl_touch_debug(LOG_WARNING, "has_scheduled:%d 6\n",has_scheduled);
		return -1;
	}

	is_stable_status = 1;
	printk(KERN_ERR "is_stable_status:%d\n",is_stable_status);
//	yl_touch_debug(LOG_WARNING, "is_stable_status:%d\n",is_stable_status);
	has_scheduled = 0;

//	yl_touch_debug(LOG_WARNING, "has_scheduled:%d 7\n",has_scheduled);
	return 0;
}







u32 waterproofalgorithm1(struct qt602240_data *data)
{
	u8 page;
	int i,j;
	u8 value1,value2;
	u16 current_value;
	u32 count = 1;
	u8 positive_flag = 0;
	j =  0;

//	yl_touch_debug(LOG_WARNING, "has_scheduled:%d 1\n",has_scheduled);

	if( (has_scheduled == 1)  || (enter_cal_mode == 1) )
		return 1;

	has_scheduled = 1;

//	yl_touch_debug(LOG_WARNING, "has_scheduled:%d 2\n",has_scheduled);
	
	qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 255);
	qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 1);
	qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 0);
	qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
	msleep(3);

	qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x10);
	msleep(3);

	qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x11);
	msleep(3);

//	qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,1, &page);//for test
//	printk(KERN_ALERT "page:%d",page);

//	qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,1, &page);
//	printk(KERN_ALERT "page:%d",page);

/*	for(;page >= 1;page--)
	{
		qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x02);
		msleep(10);
	}		
*/
	while( count <= 4 )
	{
		qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,1, &page);
		yl_touch_debug(LOG_WARNING, "page:%d",page);//for test
		for(i = 0;(i <= 63)&&(j < 220);i++)
		{
			if( atmel_kind == 0x60 ) 
			{ 
				if( ( table[j] & 0x2 ) == 0x2 )
				{
					qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,2 + i*2, &value1);
					qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,3 + i*2, &value2);	
					current_value = value2*256 + value1;
					if( current_value > ( T37_ref_data[j] + 290 ) )
					{
						yl_touch_debug(LOG_WARNING, "j:%d,T37_ref_data[j]:%5d,current_value:%5d,diff:%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
						qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
						msleep(10);
						atmel_ts_release(data);
						has_scheduled = 0;

						qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
						qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
						qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
						qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
						yl_touch_debug(LOG_WARNING, "is_stable_status:%d -3\n",is_stable_status);
						return -3;
					}							
					else if( T37_ref_data[j] > ( current_value + 290 ) )
					{
						yl_touch_debug(LOG_WARNING, "j:%d,T37_ref_data[j]:%5d,current_value:%5d,diff:%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
						positive_flag = 1;		
					}									
				}
			}
			else
			{ 
				if( ( table[j] & 0x1 ) == 0x1 )
				{
					qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,2 + i*2, &value1);
					qt602240_read_object(data, QT602240_DEBUG_DIAGNOSTIC,3 + i*2, &value2);	

					current_value = value2*256 + value1;

					if( ( j == 179 ) || ( j == 192 ) || ( j == 205 ) || ( j == 218 ) ){
					
						if( current_value > ( T37_ref_data[j] + 180 ) )
						{
							yl_touch_debug(LOG_WARNING, "j:%d,T37_ref_data[j]:%5d,current_value:%5d,diff:%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
							qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
							msleep(10);
							atmel_ts_release(data);
							has_scheduled = 0;

	//						yl_touch_debug(LOG_WARNING, "has_scheduled:%d 4\n",has_scheduled);

							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
							yl_touch_debug(LOG_WARNING, "is_stable_status:%d -3\n",is_stable_status);
							return -3;
						}							
						else if( T37_ref_data[j] > ( current_value + 180 ) )
						{
	//						yl_touch_debug(LOG_WARNING, "j:%d,T37_ref_data[j]:%5d,current_value:%5d,diff:%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
							yl_touch_debug(LOG_DEBUG, "%3d,%5d,%5d,%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
							positive_flag = 1;		
						}

					}
					else{
					
						if( current_value > ( T37_ref_data[j] + 290 ) )
						{
							yl_touch_debug(LOG_WARNING, "j:%d,T37_ref_data[j]:%5d,current_value:%5d,diff:%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
							qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
							msleep(10);
							atmel_ts_release(data);
							has_scheduled = 0;

	//						yl_touch_debug(LOG_WARNING, "has_scheduled:%d 4\n",has_scheduled);

							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
							qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);
							yl_touch_debug(LOG_WARNING, "is_stable_status:%d -3\n",is_stable_status);
							return -3;
						}							
						else if( T37_ref_data[j] > ( current_value + 290 ) )
						{
	//						yl_touch_debug(LOG_WARNING, "j:%d,T37_ref_data[j]:%5d,current_value:%5d,diff:%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
							yl_touch_debug(LOG_DEBUG, "%3d,%5d,%5d,%5d\n",j,T37_ref_data[j],current_value,current_value-T37_ref_data[j]);
							positive_flag = 1;		
						}

					}	//not key			
				}   //>0
			}	//ofilm		

			j++;
		}
		count++;
		qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_DIAGNOSTIC, 0x01);
		msleep(10);
	}

//	yl_touch_debug(LOG_WARNING, "j=%d\n",j);
	if( tw_pressed == 1 )
	{
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);

		yl_touch_debug(LOG_WARNING, "is_stable_status:%d -2\n",is_stable_status);
		has_scheduled = 0;

//		yl_touch_debug(LOG_WARNING, "has_scheduled:%d 5\n",has_scheduled);
		return -2;
	}
	else if( positive_flag == 1 )
	{
		qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
		msleep(10);
		atmel_ts_release(data);

		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALST, 5);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHCALSTHR, 40);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
		qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);

		yl_touch_debug(LOG_WARNING, "is_stable_status:%d -1\n",is_stable_status);
		has_scheduled = 0;

//		yl_touch_debug(LOG_WARNING, "has_scheduled:%d 6\n",has_scheduled);
		return -1;
	}

	is_stable_status = 1;
	yl_touch_debug(LOG_WARNING, "is_stable_status:%d\n",is_stable_status);
	has_scheduled = 0;

//	yl_touch_debug(LOG_WARNING, "has_scheduled:%d 7\n",has_scheduled);
	return 0;
}

static void cal_tw(struct work_struct *work)                     //added for timer algorithm 4.26
{
	yl_touch_debug(LOG_WARNING, "cal_tw function\n");

	if(is_stable_status == 0)
		waterproofalgorithm(data);

	mutex_lock(&timer_lock);
	if( (atmel_suspend == 0) && (is_stable_status == 0) )
	{
		mod_timer(&tw_cal_timer, jiffies+2*HZ);
	}
	mutex_unlock(&timer_lock);
}

static void cal_timer_func(unsigned long data)  //added for timer algorithm 4.26
{
	if( cal_tw_count > 0 )
	{
		yl_touch_debug(LOG_WARNING, "cal_tw_count:%d\n",cal_tw_count);
	//	yl_touch_debug(LOG_WARNING, "cal_timer_func\n");
		if(is_stable_status == 0)
		{
//			yl_touch_debug(LOG_WARNING, "cal_timer_func 1\n");
			if( atmel_suspend == 0 )
			{
//				yl_touch_debug(LOG_WARNING, "cal_timer_func 2\n");
				schedule_work(&cal_work);
				if( enter_release_timer == 0 )
					cal_tw_count--;
			}
		}
	}
	else
	{
		printk(KERN_ERR "%d\n",cal_tw_count);
		enter_release_timer = 1;
	}
}

static int __devinit qt602240_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int error=0;
//	struct qt602240_data *data;
	struct input_dev *input_dev;
	printk(KERN_ERR "2013.2.6\n");

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

    /**************sz start**************/
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		yl_touch_debug(LOG_DEBUG, "%s: need I2C_FUNC_I2C\n", __func__);
		error = -ENODEV;
		goto err_check_functionality_failed;
	}

    /**************sz  end****************/
   	if (!client->dev.platform_data)
		return -EINVAL;
        
	data = kzalloc(sizeof(struct qt602240_data), GFP_KERNEL);
    yl_touch_debug(LOG_DEBUG, "input devices register\n");  /**********sz********/
	input_dev = input_allocate_device();
	if (!data || !input_dev) {
		yl_touch_debug(LOG_DEBUG, "Failed to allocate memory\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	input_dev->name = "AT42QT602240/ATMXT224 Touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	input_dev->open = qt602240_input_open;
	input_dev->close = qt602240_input_close;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_capability(input_dev, EV_KEY, KEY_BACK);
	input_set_capability(input_dev, EV_KEY, KEY_MENU);
	input_set_capability(input_dev, EV_KEY, KEY_HOME);
	input_set_capability(input_dev, EV_KEY, KEY_SEARCH);


	/* For single touch */
        #if 0
	input_set_abs_params(input_dev, ABS_X,
			     0, QT602240_MAX_XC, 0, 0);
	input_set_abs_params(input_dev, ABS_Y,
			     0, QT602240_MAX_YC, 0, 0);
        #endif
        /********sz********************/
	/* For multi touch */
       #if 0
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
			     0, QT602240_MAX_AREA, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
			     0, QT602240_MAX_XC, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
			     0, QT602240_MAX_YC, 0, 0);
    #endif

    input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
		     	 0, QT602240_MAX_AREA, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
			     0, 480, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
			     0, 800, 0, 0);

	input_set_drvdata(input_dev, data);

	data->client = client;
	data->input_dev = input_dev;
	data->pdata = client->dev.platform_data;
	data->irq = client->irq;

	i2c_set_clientdata(client, data);


	///////////////////////////POWER ON/////////////////////////////
	omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_RST_GPIO, "ts_rst");
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0);
	mdelay(5);

	omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_PWR_EN_GPIO, "ts_pwr_en");
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 1);
	mdelay(10);

	omap_mux_init_signal("i2c2_scl", OMAP_PIN_INPUT);	
	omap_mux_init_signal("i2c2_sda", OMAP_PIN_INPUT);

	if( !is_p0 )
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P1, OMAP_PIN_INPUT_PULLUP);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P1, "ts_irq");
		gpio_direction_input(OMAP_FT5X06_IRQ_GPIO_P1);
	}
	else
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P0, OMAP_PIN_INPUT_PULLUP);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P0, "ts_irq");
		gpio_direction_input(OMAP_FT5X06_IRQ_GPIO_P0);
	}

    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 1);
	msleep(40);



	error = qt602240_initialize(data);
	if (error)
	{
		ft5x0x_tw_is_or_not_power_on = 1;
	    goto err_free_object;
	}

	atmel_wq = create_singlethread_workqueue("atmel_wq");
	INIT_WORK(&data->work, atmel_ts_work_func);

/*	if(1 == atmel_tw_is_calibrated_1())
		qt602240_calibrate();
	else*/
	read_tw_reference_data();


    error = request_irq(client->irq, qt602240_interrupt,IRQF_TRIGGER_LOW, client->dev.driver->name, data);
	if (error) {
		yl_touch_debug(LOG_DEBUG, "Failed to register interrupt\n");
		goto err_free_object;
	}     

    error = input_register_device(input_dev);
	if (error)
		goto err_free_irq;


	touchscreen_set_ops(&atmel_synaptics_ops);

/*	error = sysfs_create_group(&client->dev.kobj, &qt602240_attr_group);
	if (error)
		goto err_unregister_device;*/

    /*********************sz***********************/
    #ifdef CONFIG_HAS_EARLYSUSPEND         
	data->early_suspend.level   = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;//EARLY_SUSPEND_LEVEL_STOP_DRAWING;
	data->early_suspend.suspend = qt602240_early_suspend;
	data->early_suspend.resume  = qt602240_late_resume;
	register_early_suspend(&data->early_suspend);
    #endif
   /********************sz***********************/

	INIT_WORK(&cal_work, cal_tw);                                   //added for timer algorithm 4.26             
	init_timer(&tw_cal_timer);
	tw_cal_timer.expires = jiffies+HZ;
	tw_cal_timer.function = cal_timer_func;
	tw_cal_timer.data = 1;
	add_timer(&tw_cal_timer);

	atmel_tw_is_active_status = 1;
	return 0;

err_unregister_device:
    yl_touch_debug(LOG_DEBUG, "atmel err_unregister_device\n");
	input_unregister_device(input_dev);
	input_dev = NULL;
err_free_irq:
    yl_touch_debug(LOG_DEBUG, "atmel err_free_irq\n");
	free_irq(client->irq, data);
err_free_object:
    yl_touch_debug(LOG_DEBUG, "atmel err_free_object:\n");
	kfree(data->object_table);   
err_free_mem:
    yl_touch_debug(LOG_DEBUG, "atmel err_free_mem:\n");
	input_free_device(input_dev);
	kfree(data);
err_check_functionality_failed:
	return error;
}

static int __devexit qt602240_remove(struct i2c_client *client)
{
	struct qt602240_data *data = i2c_get_clientdata(client);

    unregister_early_suspend(&data->early_suspend);
//	sysfs_remove_group(&client->dev.kobj, &qt602240_attr_group);
	free_irq(data->irq, data);
	input_unregister_device(data->input_dev);
	kfree(data->object_table);
	kfree(data);

	return 0;
}

u8 is_atmel_power_on = 1;

static int qt602240_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct qt602240_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev = data->input_dev;
	struct irq_desc *desc = irq_to_desc(data->irq);//zfl for operating irq no effect 

	yl_touch_debug(LOG_WARNING, "Atmel224E:enter %s\n",__FUNCTION__);

	mutex_lock(&input_dev->mutex);
	cancel_work_sync(&data->work);

	if(likely( 0 == desc->depth ))
		disable_irq(data->irq);
	if(unlikely( 0 == desc->depth ))
	{
		yl_touch_debug(LOG_WARNING, "%s:%d,users=%d,irq=%d,desc->depth=%d\n",__func__,__LINE__,
								input_dev->users,gpio_get_value(OMAP_FT5X06_IRQ_GPIO_P0),desc->depth);//
		mdelay(3);
		disable_irq(data->irq);
	}

	/*if (input_dev->users)
		qt602240_stop(data);*/

	atmel_ts_release(data);/**///zhe li bu neng jia

	mutex_unlock(&input_dev->mutex);

	fiveupanddown = 0;

	omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_RST_GPIO, "ts_rst");
	gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0);

	if( !is_p0 )
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P1, OMAP_PIN_INPUT_PULLDOWN);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P1, "ts_irq");
		gpio_direction_output(OMAP_FT5X06_IRQ_GPIO_P1, 0);
	}
	else
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P0, OMAP_PIN_INPUT_PULLDOWN);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P0, "ts_irq");
		gpio_direction_output(OMAP_FT5X06_IRQ_GPIO_P0, 0);
	}

	omap_mux_init_signal(TS_FT5X06_I2C_SCL_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_I2C_SCL_GPIO, "i2c2_scl");
	gpio_direction_output(OMAP_FT5X06_I2C_SCL_GPIO, 0);
	
	omap_mux_init_signal(TS_FT5X06_I2C_SDA_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_I2C_SDA_GPIO, "i2c2_sda");
	gpio_direction_output(OMAP_FT5X06_I2C_SDA_GPIO, 0);

	omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_PWR_EN_GPIO, "ts_pwr_en");
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);/**/

	is_stable_status = 0;
	is_atmel_power_on = 0;
        
	return 0;
}

static int qt602240_resume(struct i2c_client *client)
{
	struct qt602240_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev = data->input_dev;
	struct irq_desc *desc = irq_to_desc(data->irq);//zfl for operating irq no effect 

	yl_touch_debug(LOG_WARNING, "Atmel224E:enter %s\n",__FUNCTION__);

	mutex_lock(&input_dev->mutex);
	/* Soft reset */
	//qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_RESET, 1);
	//mdelay(QT602240_RESET_TIME);
	//mdelay(QT602240_RESET_TIME);
	//qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);
	//qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALTHR, 1);
	//qt602240_write_object(data, QT602240_GEN_ACQUIRE,QT602240_ACQUIRE_ATCHFRCCALRATIO, 0);

	/*if (input_dev->users)
		qt602240_start(data);*/

//	mdelay(5);
	if( unlikely( desc->depth > 1 ) )
	{
		enable_irq(data->irq);
		yl_touch_debug(LOG_WARNING, "%s:%d,users=%d,irq level=%d,depth=%d\n",__func__,__LINE__,
							input_dev->users,gpio_get_value(OMAP_FT5X06_IRQ_GPIO_P0),desc->depth);//
		mdelay(10);
		if( unlikely( desc->depth ) )
			enable_irq(data->irq);
	}
	else
		enable_irq(data->irq);
	
	mutex_unlock(&input_dev->mutex);/**/

	atmel_tw_is_active_status = 1;

	enter_release_timer = 0;

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void qt602240_early_suspend(struct early_suspend *h)
{
	struct qt602240_data *data;

	atmel_suspend = 1;
	del_timer_sync(&tw_cal_timer);
	cancel_work_sync(&cal_work);
	tw_pressed = 0;
	cal_tw_count = 6;

	atmel_tw_is_active_status = 0;

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	data = container_of(h, struct qt602240_data, early_suspend);
	qt602240_suspend(data->client, PMSG_SUSPEND);/**/
}

void atmel_poweron(void)
{
	///////////////////////////POWER ON/////////////////////////////
	omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_RST_GPIO, "ts_rst");
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0);
	mdelay(5);

	omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_PWR_EN_GPIO, "ts_pwr_en");
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 1);
	mdelay(10);

	omap_mux_init_signal("i2c2_scl", OMAP_PIN_INPUT);	
	omap_mux_init_signal("i2c2_sda", OMAP_PIN_INPUT);

	if( !is_p0 )
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P1, OMAP_PIN_INPUT_PULLUP);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P1, "ts_irq");
		gpio_direction_input(OMAP_FT5X06_IRQ_GPIO_P1);
	}
	else
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P0, OMAP_PIN_INPUT_PULLUP);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P0, "ts_irq");
		gpio_direction_input(OMAP_FT5X06_IRQ_GPIO_P0);
	}

    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 1);
}

static void qt602240_late_resume(struct early_suspend *h)
{
	struct qt602240_data *data;

	yl_touch_debug(LOG_DEBUG, "Atmel224E:enter %s\n",__FUNCTION__);

	mod_timer(&tw_cal_timer, jiffies+HZ);

	///////////////////////////POWER ON/////////////////////////////
/*	omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_RST_GPIO, "ts_rst");
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0);
	mdelay(5);

	omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_PWR_EN_GPIO, "ts_pwr_en");
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 1);
	mdelay(10);

	omap_mux_init_signal("i2c2_scl", OMAP_PIN_INPUT);	
	omap_mux_init_signal("i2c2_sda", OMAP_PIN_INPUT);

	if( !is_p0 )
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P1, OMAP_PIN_INPUT_PULLUP);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P1, "ts_irq");
		gpio_direction_input(OMAP_FT5X06_IRQ_GPIO_P1);
	}
	else
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P0, OMAP_PIN_INPUT_PULLUP);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P0, "ts_irq");
		gpio_direction_input(OMAP_FT5X06_IRQ_GPIO_P0);
	}

    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 1);*/   //4.11 altered
//	msleep(40);	

	yl_touch_debug(LOG_WARNING, "is_atmel_power_on:%d\n",is_atmel_power_on);

	if( is_atmel_power_on == 0 )
	{
		printk(KERN_ALERT "not power on in lcd resume function\n");
		atmel_poweron();
		msleep(40);	
		is_atmel_power_on = 1;
	}

	atmel_suspend = 0;

	data = container_of(h, struct qt602240_data, early_suspend);

	qt602240_write_object(data, QT602240_GEN_COMMAND,QT602240_COMMAND_CALIBRATE, 1);

	// Soft reset 
	/*qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_RESET, 1);
	msleep(QT602240_RESET_TIME);*/

	qt602240_resume(data->client);/**/
}
#endif

static const struct i2c_device_id qt602240_id[] = {
	{ ATMEL_I2C_RMI_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, qt602240_id);

static struct i2c_driver qt602240_driver = {

	.probe		= qt602240_probe,
	.remove		= __devexit_p(qt602240_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= qt602240_suspend,
	.resume		= qt602240_resume,
#endif
	.id_table	= qt602240_id,
	.driver = {
		.name	= ATMEL_I2C_RMI_NAME,
		//.owner	= THIS_MODULE,
	},
	
};

static int __init qt602240_init(void)
{
	return i2c_add_driver(&qt602240_driver);
}

static void __exit qt602240_exit(void)
{
	i2c_del_driver(&qt602240_driver);
}

module_init(qt602240_init);
module_exit(qt602240_exit);

/* Module information */
MODULE_AUTHOR("Joonyoung Shim <jy0922.shim@samsung.com>");
MODULE_DESCRIPTION("AT42QT602240/ATMXT224 Touchscreen driver");
MODULE_LICENSE("GPL");

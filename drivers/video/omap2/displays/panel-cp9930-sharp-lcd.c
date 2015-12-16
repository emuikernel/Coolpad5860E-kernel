/*
 * drivers/video/omap2/displays/panel-cp9930-sharp-lcd.c
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
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <linux/spi/spi.h>

#include <mach/gpio.h>
#include <plat/misc.h>
#include <plat/yl_debug.h>//added by huangjiefeng
#include <plat/mux_omap.h>
#include <asm/mach-types.h>
#include <plat/control.h>

#include <plat/display.h>


#include <linux/backlight.h>
#include <linux/fb.h>

#include "panel-cp9930-sharp-lcd.h"

#include "./../../../input/touchscreen/ft5x06_ts.h"

 
#define OMAP36XX_DSS_PCLK_GPIO                    (66)
#define OMAP36XX_DSS_HSYNC_GPIO                   (67)
#define OMAP36XX_DSS_VSYNC_GPIO                   (68)
#define OMAP36XX_DSS_DE_GPIO                      (69)
#define OMAP36XX_DSS_D0_GPIO                      (70)
#define OMAP36XX_DSS_D1_GPIO                      (71)
#define OMAP36XX_DSS_D2_GPIO                      (72)
#define OMAP36XX_DSS_D3_GPIO                      (73)
#define OMAP36XX_DSS_D4_GPIO                      (74)
#define OMAP36XX_DSS_D5_GPIO                      (75)
#define OMAP36XX_DSS_D6_GPIO                      (76)
#define OMAP36XX_DSS_D7_GPIO                      (77)
#define OMAP36XX_DSS_D8_GPIO                      (78)
#define OMAP36XX_DSS_D9_GPIO                      (79)
#define OMAP36XX_DSS_D10_GPIO                     (80)
#define OMAP36XX_DSS_D11_GPIO                     (81)
#define OMAP36XX_DSS_D12_GPIO                     (82)
#define OMAP36XX_DSS_D13_GPIO                     (83)
#define OMAP36XX_DSS_D14_GPIO                     (84)
#define OMAP36XX_DSS_D15_GPIO                     (85)
#define OMAP36XX_DSS_D16_GPIO                     (86)
#define OMAP36XX_DSS_D17_GPIO                     (87)
#define OMAP36XX_DSS_D18_GPIO                     (88)
#define OMAP36XX_DSS_D19_GPIO                     (89)
#define OMAP36XX_DSS_D20_GPIO                     (90)
#define OMAP36XX_DSS_D21_GPIO                     (91)
#define OMAP36XX_DSS_D22_GPIO                     (92)
#define OMAP36XX_DSS_D23_GPIO                     (93)

#define LCD_RST                  			      (40)

#define SPI_CLK                  				  (171)
#define SPI_OUT                  				  (172)
#define SPI_IN                   				  (173)
#define LCD_SPI_CS               				  (174)

extern void turn_on_lcd_power(void);
extern void turn_off_lcd_power(void);
extern void zoom_pwm_config_pwmoff(void);
static DEFINE_SPINLOCK(panel_lock);
static int resume_in_sleep=0;
#if 0
static struct omap_video_timings cp_n930_sharp_lcd_panel_timings = {
  /* 800 x 480 @ 60 Hz  Reduced blanking VESA CVT 0.31M3-R */
  .x_res          = 480,
  .y_res          = 800,//960,
  .pixel_clock    = 30720,//36782,
  .hfp            = 12,
  .hsw            = 70,
  .hbp            = 70,
  .vfp            = 4,
  .vsw            = 2,
  .vbp            = 4
};
#endif

static struct omap_video_timings cp_n930_sharp_lcd_panel_timings = {
  /* 800 x 480 @ 60 Hz  Reduced blanking VESA CVT 0.31M3-R */
  .x_res          = 480,
  .y_res          = 800,//960,
  .pixel_clock    = 24116,//36782,
  .hfp            = 5,
  .hsw            = 5,
  .hbp            = 5,
  .vfp            = 5,
  .vsw            = 2,
  .vbp            = 5

  //.x_res          = 480,
  //.y_res          = 800,//960,
  //.pixel_clock    = 26116,//36782,
  //.hfp            = 100,
  //.hsw            = 8,
  //.hbp            = 100,
  //.vfp            = 20,
  //.vsw            = 1,
  //.vbp            = 6
};

typedef unsigned    char        UINT8;
typedef signed      short       INT16;
typedef unsigned    short       UINT16;
typedef signed      int         INT32;
typedef unsigned    int         UINT32;

static int cp_n930_sharp_lcd_spi_resume(struct spi_device *spi);
static int cp_n930_sharp_lcd_spi_suspend(struct spi_device *spi, pm_message_t mesg);


struct spi_device *cp_n930_sharp_lcd_spi_device;


struct cp_n930_sharp_lcd_driver_data
{ 
  struct backlight_device *bl_dev; 
  unsigned int    saved_bklight_level; 
}cp_n930_sharp_lcd_drv_data;

//******************************** CP8930 lcd **********************************************
#if 0 //gamma 2.2
//User define command list is available only set"SETEXC" command
static  char truly_hx8363_spi_data_extc[] = {
    0xB9,  //command:Set_EXTC 
    0xFF,  
    0x83,  
    0x63,  
};
static char truly_hx8363_spi_data_power[] = {
    0xB1,  //command:Set_POWER
    0x81,  //himax 1para
    0x30,  //himax 2para
    0x08,  //himax 3para
    0xB4,//0x34-->0xb4 shuaixz
    0x01,
    0x13,
    0x11,    //0x11,  //vsp BTP[4:0] 0x0d??? 0X11 VGH shuaixz recovery
    0x00,
    0x35,
    0x3E,
    0x16,
    0x16,
};
static char truly_hx8363_spi_data_sleepout[] = {
    0x11,  //command:Sleep Out
    //WaitTime(150); 
};

//This command is used to define the format of RGB picture data
//18 bit /pixel: X110XXXX
static char truly_hx8363_spi_data_colmod[] = {
    0x3A,  //command:COLMOD
    0x77,  
    
};
static char truly_hx8363_spi_data_mac[] = {
    0x36,  //command:Memory Access Control 
    0x02, 
};
static char truly_hx8363_spi_data_rgbif[] = {
    0xB3,  //command:Set_RGBIF 
    0x01,
};
static char truly_hx8363_spi_data_cyc[] = {
    0xB4,  //command:Set_CYC CPT 
    0x08,  // 2 Dot INV CPT  
    0x03,	
    0xe0,	
    0x30,
    0x01,
    0x12,
    0x64,
    0x01,
    0xff,
    0x00,
    0x00,  // v
};
static char truly_hx8363_spi_data_ptba[] = {
	0xBF, //command:Set_PTBA
    0x00,
    0x10,
};
static char truly_hx8363_spi_data_vcom[] = {
	0xB6, //command:Set_VCOM 
    0x00, //0x10
};
static char truly_hx8363_spi_data_panel[] = {
    0xCC, //command:Set_PANEL 
    0x01, //01 
    //WaitTime(5);
};
static char truly_hx8363_spi_data_gamma[] = {
    0xE0,	//command:Gamma 2.2 
    0x00,		   // 
    0x1E,		   // 
    0x63,		   // 23 
    0x72,		   // 32 
    0x36,		   // 
    0x3F,		   // 
    0x08,		   // 
    0xCC,		   // 
    0x0e,		   // 
    0x92,		   // 
    0x54,		   // 
    0x15,		   // 
    0x98,		   //18 
    0xcf,		   //4f 
    0x19,		   // 
    0x00,		   // 
    0x1E,		   // 
    0x63,		   // 23 
    0x72,		   // 32 
    0x36,		   // 
    0x3F,		   // 
    0x08,		   // 
    0xCC,		   // 
    0x0e,		   // 
    0x92,		   // 
    0x54,		   // 
    0x15,		   // 
    0x98,		   //18 
    0xcf,		   //4f 
    0x19,		   // 	 
    //WaitTime(5); 
};

//This command is used to set DGC LUT(128 parameter)
///1th parameter:DGC_EN = 1;
static char truly_hx8363_spi_data_dgc[] = {
    0xC1, //command:DGC 
    0x01, 

    0x06, 
    0x0D, 
    0x1E,
    0x2F,
    0x3F,
    0x51,
    0x60, 
    0x71, 
    0x80, 
    0x8B, 
    0x95, 
    0x9F, 
    0xA8, 
    0xB1, 
    0xBD, 
    0xC8, 
    0xCE, 
    0xD5, 
    0xDB, 
    0xE1, 
    0xE4, 
    0xE7, 
    0xEB, 
    0xEE, 
    0xF0, 
    0xF2, 
    0xF4, 
    0xF7, 
    0xF8, 
    0xFA, 
    0xFC, 
    0xFD, 
    0xFF, 
    0xB1, 
    0xDE, 
    0xAE, 
    0x90, 
    0xDB, 
    0x20, 
    0xFC, 
    0xA2, 
    0x02, 

    0x06, 
    0x0D, 
    0x1E, 
    0x2F, 
    0x3F, 
    0x51, 
    0x60, 
    0x71, 
    0x80, 
    0x8B, 
    0x95, 
    0x9F, 
    0xA8, 
    0xB1, 
    0xBD, 
    0xC8, 
    0xCE, 
    0xD5, 
    0xDB, 
    0xE1, 
    0xE4, 
    0xE7, 
    0xEB, 
    0xEE, 
    0xF0, 
    0xF2, 
    0xF4, 
    0xF7, 
    0xF8, 
    0xFA, 
    0xFC, 
    0xFD, 
    0xFF, 
    0xB1, 
    0xDE, 
    0xAE, 
    0x90, 
    0xDB, 
    0x20, 
    0xFC, 
    0xA2, 
    0x02, 

    0x06, 
    0x0D, 
    0x1E, 
    0x2F, 
    0x3F, 
    0x51, 
    0x60, 
    0x71, 
    0x80, 
    0x8B, 
    0x95, 
    0x9F, 
    0xA8, 
    0xB1, 
    0xBD, 
    0xC8, 
    0xCE, 
    0xD5, 
    0xDB, 
    0xE1, 
    0xE4, 
    0xE7, 
    0xEB, 
    0xEE, 
    0xF0, 
    0xF2, 
    0xF4, 
    0xF7, 
    0xF8, 
    0xFA, 
    0xFC, 
    0xFD, 
    0xFF, 
    0xB1, 
    0xDE, 
    0xAE, 
    0x90, 
    0xDB, 
    0x20, 
    0xFC, 
    0xA2, 
    0x02, 

    //WaitTime(5); 
};

static char truly_hx8363_spi_data_dispon[] = {
    0x29,
};
#endif

#if 1 //truly gamma 2.6
//User define command list is available only set"SETEXC" command
static  char truly_hx8363_spi_data_extc[] = {
    0xB9,  //command:Set_EXTC 
    0xFF,  
    0x83,  
    0x63,  
};
static char truly_hx8363_spi_data_power[] = {
    0xB1,  //command:Set_POWER
    0x81,  //himax 1para
    0x30,  //himax 2para
    0x08,  //himax 3para
    0x34,//0xB4,//0x34-->0xb4 shuaixz
    0x02,//0x01,
    0x13,
    0x16,//0x11,    //0x11,  //vsp BTP[4:0] 0x0d??? 0X11 VGH shuaixz recovery
    0x00,
    0x35,
    0x3E,
    0x16,
    0x16,
};
static char truly_hx8363_spi_data_sleepout[] = {
    0x11,  //command:Sleep Out
    //WaitTime(150); 
};

//This command is used to define the format of RGB picture data
//18 bit /pixel: X110XXXX
static char truly_hx8363_spi_data_colmod[] = {
    0x3A,  //command:COLMOD
    0x77,  
    
};
static char truly_hx8363_spi_data_mac[] = {
    0x36,  //command:Memory Access Control 
    0x02, 
};
static char truly_hx8363_spi_data_rgbif[] = {
    0xB3,  //command:Set_RGBIF 
    0x01,
};
static char truly_hx8363_spi_data_cyc[] = {
    0xB4,  //command:Set_CYC CPT 
    0x08,  // 2 Dot INV CPT  
    0x03,	
    0xe0,	
    0x30,
    0x01,
    0x12,
    0x64,
    0x01,
    0xff,
    0x00,
    0x00,  // v
};
static char truly_hx8363_spi_data_ptba[] = {
	0xBF, //command:Set_PTBA
    0x00,
    0x10,
};
static char truly_hx8363_spi_data_vcom[] = {
	0xB6, //command:Set_VCOM 
    0x17,//0x00, //0x10
};
static char truly_hx8363_spi_data_panel[] = {
    0xCC, //command:Set_PANEL 
    0x01, //01 
    //WaitTime(5);
};
static char truly_hx8363_spi_data_gamma[] = {
    0xE0,	//command:Gamma 2.6 
    0x00,		   // 
    0x1E,		   // 
    0x63,		   // 23 
    0x72,		   // 32 
    0x36,		   // 
    0x3F,		   // 
    0x08,		   // 
    0xCC,		   // 
    0x0e,		   // 
    0x92,		   // 
    0x54,		   // 
    0x15,		   // 
    0x98,		   //18 
    0xcf,		   //4f 
    0x19,		   // 
    0x00,		   // 
    0x1E,		   // 
    0x63,		   // 23 
    0x72,		   // 32 
    0x36,		   // 
    0x3F,		   // 
    0x08,		   // 
    0xCC,		   // 
    0x0e,		   // 
    0x92,		   // 
    0x54,		   // 
    0x15,		   // 
    0x98,		   //18 
    0xcf,		   //4f 
    0x19,		   // 	 
    //WaitTime(5); 
};

//This command is used to set DGC LUT(128 parameter)
///1th parameter:DGC_EN = 1;
static char truly_hx8363_spi_data_dgc[] = {
0xC1, //DGC 
0x01, 
0x07,
0x0D,
0x1C,
0x2C,
0x3B,
0x4C,
0x5C,
0x6B,
0x79,
0x86,
0x90,
0x9A,
0xA2,
0xAB,
0xB3,
0xBE,
0xC9,
0xCF,
0xD5,
0xDB,
0xE1,
0xE4,
0xE7,
0xEA,
0xED,
0xF0,
0xF2,
0xF4,
0xF7,
0xF9,
0xFB,
0xFD,
0xFF,
0xFF,
0x78,
0x48,
0xC3,
0x0E,
0xC7,
0xEF,
0x48,
0x01,

0x07,
0x0D,
0x1C,
0x2C,
0x3B,
0x4C,
0x5C,
0x6B,
0x79,
0x86,
0x90,
0x9A,
0xA2,
0xAB,
0xB3,
0xBE,
0xC9,
0xCF,
0xD5,
0xDB,
0xE1,
0xE4,
0xE7,
0xEA,
0xED,
0xF0,
0xF2,
0xF4,
0xF7,
0xF9,
0xFB,
0xFD,
0xFF,
0xFF,
0x78,
0x48,
0xC3,
0x0E,
0xC7,
0xEF,
0x48,
0x01,

0x07,
0x0D,
0x1C,
0x2C,
0x3B,
0x4C,
0x5C,
0x6B,
0x79,
0x86,
0x90,
0x9A,
0xA2,
0xAB,
0xB3,
0xBE,
0xC9,
0xCF,
0xD5,
0xDB,
0xE1,
0xE4,
0xE7,
0xEA,
0xED,
0xF0,
0xF2,
0xF4,
0xF7,
0xF9,
0xFB,
0xFD,
0xFF,
0xFF,
0x78,
0x48,
0xC3,
0x0E,
0xC7,
0xEF,
0x48,
0x01,


    //WaitTime(5); 
};

static char truly_hx8363_spi_data_dispon[] = {
    0x29,
};
#endif

#if 1 //BOYI 
//User define command list is available only set"SETEXC" command
static  char by_hx8363_spi_data_extc[] = {
    0xB9,  //command:Set_EXTC 
    0xFF,  
    0x83,  
    0x63,  
};
static char by_hx8363_spi_data_power[] = {
    0xB1,  //command:Set_POWER
    0x01,  //himax 1para
    0x34,  //himax 2para
    0x07,  //himax 3para
    0x33,//0xB4,//0x34-->0xb4 shuaixz
    0x02,//0x01,
    0x13,
    0x11,//0x11,    //0x11,  //vsp BTP[4:0] 0x0d??? 0X11 VGH shuaixz recovery
    0x00,
    0x35,
    0x3E,
    0x3F,
    0x3F,
};
static char by_hx8363_spi_data_sleepout[] = {
    0x11,  //command:Sleep Out
    //WaitTime(150); 
};

//This command is used to define the format of RGB picture data
//18 bit /pixel: X110XXXX
static char by_hx8363_spi_data_colmod[] = {
    0x3A,  //command:COLMOD
    0x70,  //0X77,//by ge
    
};
static char by_hx8363_spi_data_mac[] = {
    0x36,  //command:Memory Access Control 
    0x02, 
};
static char by_hx8363_spi_data_rgbif[] = {
    0xB3,  //command:Set_RGBIF 
    0x01,//by ge
};
static char by_hx8363_spi_data_cyc[] = {
    0xB4,  //command:Set_CYC CPT 
    0x08,  // 2 Dot INV CPT  
    0x0a,	
    0xc8,	
    0x0a,
    0x00,
    0x02,
    0x54,
    0x03,
    0x50,
    //0x00,
    //0x00,  // v
};
//static char by_hx8363_spi_data_ptba[] = {
//	0xBF, //command:Set_PTBA
 //   0x00,
 //   0x10,
//};
static char by_hx8363_spi_data_vcom[] = {
	0xB6, //command:Set_VCOM 
    0x38,//0x17,//0x00, //0x10 //by ge
};
static char by_hx8363_spi_data_panel[] = {
    0xCC, //command:Set_PANEL 
    0x01, //01 
    //WaitTime(5);
};
static char by_hx8363_spi_data_gamma[] = {
    0xE0,	//command:Gamma 2.2 
    0x00,    
    0x1E,    
    0x63,    
    0x15,    
    0x13,    
    0x30,    
    0x0C,    
    0xCF,    
    0x0F,    
    0xD5,    
    0x17,    
    0xD5,    
    0x96,    
    0xD1,    
    0x17,    
    0x00,    
    0x1E,    
    0x63,    
    0x15,    
    0x13,    
    0x30,    
    0x0C,    
    0xCF,    
    0x0F,    
    0xD5,    
    0x17,    
    0xD5,    
    0x96,    
    0xD1,    
    0x17, 
    //WaitTime(5); 
};

//This command is used to set DGC LUT(128 parameter)
///1th parameter:DGC_EN = 1;
static char by_hx8363_spi_data_dgc[] = {
0xC1, //DGC 
0x01, 

0x00,    
0x03,    
0x0E,    
0x17,    
0x1E,    
0x27,    
0x2E,    
0x38,    
0x41,    
0x49,    
0x50,    
0x58,    
0x5E,    
0x66,    
0x6E,    
0x77,    
0x7F,    
0x88,    
0x90,    
0x98,    
0xA0,    
0xA8,    
0xB0,    
0xB8,    
0xC2,    
0xCA,    
0xD1,    
0xD9,    
0xE2,    
0xE9,    
0xF1,    
0xF8,    
0xFF,    
0xBD,    
0x02,    
0x4C,    
0xCE,    
0xC5,    
0x87,    
0x52,    
0x6C,    
0x03,  

0x00,    
0x03,    
0x0E,    
0x17,    
0x1E,    
0x27,    
0x2E,    
0x38,    
0x41,    
0x49,    
0x50,    
0x58,    
0x5E,    
0x66,    
0x6E,    
0x77,    
0x7F,    
0x88,    
0x90,    
0x98,    
0xA0,    
0xA8,    
0xB0,    
0xB8,    
0xC2,    
0xCA,    
0xD1,    
0xD9,    
0xE2,    
0xE9,    
0xF1,    
0xF8,    
0xFF,    
0xBD,    
0x02,    
0x4C,    
0xCE,    
0xC5,    
0x87,    
0x52,    
0x6C,    
0x03,

0x00,    
0x03,    
0x0E,    
0x17,    
0x1E,    
0x27,    
0x2E,    
0x38,    
0x41,    
0x49,    
0x50,    
0x58,    
0x5E,    
0x66,    
0x6E,    
0x77,    
0x7F,    
0x88,    
0x90,    
0x98,    
0xA0,    
0xA8,    
0xB0,    
0xB8,    
0xC2,    
0xCA,    
0xD1,    
0xD9,    
0xE2,    
0xE9,    
0xF1,    
0xF8,   
0xFF,    
0xBD,    
0x02,    
0x4C,    
0xCE,    
0xC5,    
0x87,    
0x52,    
0x6C,    
0x03,

    //WaitTime(5); 
};

static char by_hx8363_spi_data_dispon[] = {
    0x29,
};
#endif


#if 0 //BOYI 8369
//User define command list is available only set"SETEXC" command
static  char byx_hx83639_spi_data_extc[] = {
    0xB9,  //command:Set_EXTC 
    0xFF,  
    0x83,  
    0x69,  
};
static char byx_hx8369_spi_data_power[] = {
	0xB1, //Set Power
	0x01,
	0x00,
	0x34,
	0x04,
	0x00,
	0x10,
	0x10,
	0x28,
	0x2F,
	0x3F,
	0x3F,
	0x01,
	0x22,
	0x01,
	0xE6,
	0xE6,
	0xE6,
	0xE6,
	0xE6,
};


static char byx_hx8369_spi_data_display[] = {
0xB2, // SET Display 480x800
0x00,
0x2B,
0x03,
0x03,
0x70,
0x00,
0xFF,
0x06,
0x00,
0x00,
0x00,
0x03,
0x03,
0x00,
0x01,
};

static char byx_hx8369_spi_data_cyc[] = {
0xB4, // SET Display CYC
0x03,
0x10,
0x80,
0x06,
0x02,
};

static char byx_hx8369_spi_data_vcom[] = {
0xB6,// SET VCOM
0x55,
0x55,
};

static char byx_hx8369_spi_data_DGC[] = {
                                                                            
0xc1,                                                                      
0x01,//DGC ON            

//R
0x00,//85%
0x07,
0x0F,
0x18,
0x1F,
0x28,
0x2F,
0x34,
0x3D,
0x45,
0x4D,
0x55,
0x5B,
0x62,
0x69,
0x71,
0x79,
0x80,
0x88,
0x8F,
0x96,
0x9E,
0xA6,
0xAE,
0xB6,
0xBD,
0xC5,
0xCB,
0xD1,
0xD9,
0xE1,
0xE7,
0xEE,
0x1E,
0x83,
0x34,
0xC9,
0x10,
0x40,
0x50,
0x04,
0x00,



//G
0x00,//85%
0x07,
0x0F,
0x18,
0x1F,0x01,                                                                      
0x00,                                                                      
0x34,                                                                      
0x02,                                                                      
0x00,                                                                      
0x0F,                                                                      
0x0F,                                                                      
0x28,                                                                      
0x2F,                                                                      
0x3F,                                                                      
0x3F,                                                                      
0x01,                                                                      
0x22,                                                                      
0x01,                                                                      
0xE6,                                                                      
0xE6,                                                                      
0xE6,                                                                      
0xE6,                                                                      
0xE6,   
0x28,
0x2F,
0x34,
0x3D,
0x45,
0x4D,
0x55,
0x5B,
0x62,
0x69,
0x71,
0x79,
0x80,
0x88,
0x8F,
0x96,
0x9E,
0xA6,
0xAE,
0xB6,
0xBD,
0xC5,
0xCB,
0xD1,
0xD9,
0xE1,
0xE7,
0xEE,
0x1E,
0x83,
0x34,
0xC9,
0x10,
0x40,
0x50,
0x04,
0x00,


//B
0x00,
0x08,
0x12,
0x1B,
0x23,
0x2C,
0x33,
0x3A,
0x44,
0x4C,
0x54,
0x5C,
0x63,
0x6B,
0x74,
0x7C,
0x84,
0x8C,
0x94,
0x9B,
0xA4,
0xAD,
0xB5,
0xBE,
0xC6,
0xCB,
0xD3,
0xDB,
0xE3,
0xE9,
0xF1,
0xF8,
0xFF,
0x34,
0xD1,
0x3D,
0x21,
0x13,
0xED,
0x7B,
0xBF,
0x03,

};


static char byx_hx8369_spi_data_panel[] = {
0xCC,
0x00,
};

static char byx_hx8369_spi_data_gip[] = {
0xD5, //SET GIP
0x00,
0x04,
0x03,
0x00,
0x01,
0x06,
0x20,
0x70,
0x11,
0x13,
0x00,
0x11,
0x00,
0x46,
0x00,
0x57,
0x01,
0x00,
0x44,
0x57,
0x44,
0x46,
0x07,
0x0F,
0x02,
0x03,
};
static char byx_hx8369_spi_data_gamma[] = {
0xE0, //SET GAMMA
0x0F,
0x11,
0x13,
0x0B,
0x08,
0x24,
0x23,
0x30,
0x07,
0x0D,
0x0F,
0x15,
0x17,
0x15,
0x15,
0x11,
0x17,
0x0F,
0x11,
0x13,
0x0B,
0x08,
0x24,
0x23,
0x30,
0x07,
0x0D,
0x0F,
0x15,
0x17,
0x15,
0x15,
0x11,
0x17,
};

static char byx_hx8369_spi_data_rgbif[] = {
    0xB3,  //command:Set_RGBIF 
    0x01,//by ge
};

static char byx_hx8369_spi_data_colmod[] = {
0x3A,//Set COLMOD
0x77,
};
static char byx_hx8369_spi_data_sleepout[] = {
0x11, //Sleep Out
};

//delayms(120);

static char byx_hx8369_spi_data_dispon[] = {
0x29, //Display On
};

#endif
#if 1 //BOYI 8369 CPT4.05_3
//User define command list is available only set"SETEXC" command
static  char byx_hx83639_spi_data_extc[] = {
    0xB9,  //command:Set_EXTC 
    0xFF,  
    0x83,  
    0x69,  
};
static char byx_hx8369_spi_data_power[] = {
	0xB1, //Set Power
0x01,                                                                      
0x00,                                                                      
0x34,                                                                      
0x03,                                                                      
0x00,                                                                      
0x10,                                                                      
0x10,                                                                      
0x28,                                                                      
0x2F,                                                                      
0x3F,                                                                      
0x3F,                                                                      
0x01,                                                                      
0x22,                                                                      
0x01,                                                                      
0xE6,                                                                      
0xE6,                                                                      
0xE6,                                                                      
0xE6,                                                                      
0xE6,  
};


static char byx_hx8369_spi_data_display[] = {
0xB2, // SET Display 480x800
0x00,                                                                      
0x2b,                                                                      
0x03,                                                                      
0x03,                                                                      
0x70,                                                                      
0x00,                                                                      
0xff,                                                                      
0x06,                                                                      
0x00,                                                                      
0x00,                                                                      
0x00,                                                                      
0x03,                                                                      
0x03,                                                                      
0x00,                                                                      
0x01,
};

static char byx_hx8369_spi_data_cyc[] = {
0xB4, // SET Display CYC
0x03,
0x10,
0x80,
0x06,
0x02,
};

static char byx_hx8369_spi_data_vcom[] = {
0xB6,// SET VCOM
0x45,
0x45,
};

static char byx_hx8369_spi_data_DGC[] = {
                                                                            
0xc1,                                                                      
0x01,//DGC ON            

//R
0x00,//85%
0x07,
0x0F,
0x18,
0x1F,
0x28,
0x2F,
0x34,
0x3D,
0x45,
0x4D,
0x55,
0x5B,
0x62,
0x69,
0x71,
0x79,
0x80,
0x88,
0x8F,
0x96,
0x9E,
0xA6,
0xAE,
0xB6,
0xBD,
0xC5,
0xCB,
0xD1,
0xD9,
0xE1,
0xE7,
0xEE,
0x1E,
0x83,
0x34,
0xC9,
0x10,
0x40,
0x50,
0x04,
0x00,



//G
0x00,//85%
0x07,
0x0F,
0x18,
0x1F,   
0x28,
0x2F,
0x34,
0x3D,
0x45,
0x4D,
0x55,
0x5B,
0x62,
0x69,
0x71,
0x79,
0x80,
0x88,
0x8F,
0x96,
0x9E,
0xA6,
0xAE,
0xB6,
0xBD,
0xC5,
0xCB,
0xD1,
0xD9,
0xE1,
0xE7,
0xEE,
0x1E,
0x83,
0x34,
0xC9,
0x10,
0x40,
0x50,
0x04,
0x00,


//B
0x00,
0x08,
0x12,
0x1B,
0x23,
0x2C,
0x33,
0x3A,
0x44,
0x4C,
0x54,
0x5C,
0x63,
0x6B,
0x74,
0x7C,
0x84,
0x8C,
0x94,
0x9B,
0xA4,
0xAD,
0xB5,
0xBE,
0xC6,
0xCB,
0xD3,
0xDB,
0xE3,
0xE9,
0xF1,
0xF8,
0xFF,
0x34,
0xD1,
0x3D,
0x21,
0x13,
0xED,
0x7B,
0xBF,
0x03,

};


static char byx_hx8369_spi_data_panel[] = {
0xCC,
0x00,
};

static char byx_hx8369_spi_data_gip[] = {
0xD5, //SET GIP
0x00,//1,
0x04,//2,
0x03,//3,
0x00,//4,
0x01,//5,
0x06,//6,
0x20,//7,
0x70,//8,
0x11,//9,
0x13,//10,
0x00,//11,
0x11,//12,
0x00,//13,
0x46,//14,
0x00,//15,
0x57,//16,
0x01,//17,
0x00,//18,
0x44,//19,
0x57,//20,
0x44,//21,
0x46,//22,
0x07,//23,
0x0f,//24,
0x02,//25,
0x03,//26,
};
static char byx_hx8369_spi_data_gamma[] = {
0xE0, //SET GAMMA
0x0C,                                                                      
0x1F,                                                                      
0x23,                                                                      
0x10,                                                                      
0x10,                                                                      
0x27,                                                                      
0x2B,                                                                      
0x35,                                                                      
0x07,                                                                      
0x0E,                                                                      
0x0E,                                                                      
0x15,                                                                      
0x16,                                                                      
0x14,                                                                      
0x16,                                                                      
0x12,                                                                      
0x18,                                                                      
0x0C,                                                                      
0x1F,                                                                      
0x23,                                                                      
0x10,                                                                      
0x10,                                                                      
0x27,                                                                      
0x2B,                                                                      
0x35,                                                                      
0x07,                                                                      
0x0E,                                                                      
0x0E,                                                                      
0x15,                                                                      
0x16,                                                                      
0x14,                                                                      
0x16,                                                                      
0x12,                                                                      
0x18,  
};

static char byx_hx8369_spi_data_rgbif[] = {
    0xB3,  //command:Set_RGBIF 
    0x01,//by ge
};

static char byx_hx8369_spi_data_colmod[] = {
0x3A,//Set COLMOD
0x77,
};
static char byx_hx8369_spi_data_sleepout[] = {
0x11, //Sleep Out
};

//delayms(120);

static char byx_hx8369_spi_data_dispon[] = {
0x29, //Display On
};

#endif

static char truly_spi_data_read[]={    
	0xb9,    
	0xff,    
	0x83,    
	0x63,
};

static char byx_hx8369_spi_data_read_one[]={    
	0xB9,    
	0xFF,    
	0x83,    
	0x69,
};

static char byx_hx8369_spi_data_read_two[] = {
	0XFE,
	0XF4,
};


void DelaySPI(UINT32 value)
{
	volatile UINT32 i;

	i = value;
	while(--i);
}


void usdelay(UINT32 value)
{
	volatile UINT32 i;

	i = value;
	while(--i);
}

void msdelay(UINT32 value)
{
	volatile UINT32 i;

	i = value;
	while(--i)udelay(1000);
}

static void GPIOSPIInit(void)                                                                                                      
{

    omap_mux_init_signal("gpio_171", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
    omap_mux_init_signal("gpio_172", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
    omap_mux_init_signal("gpio_173", OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN);
    omap_mux_init_signal("gpio_174", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW); 

    gpio_request(SPI_CLK, "spi_clk");
    gpio_request(SPI_OUT, "spi_out");
    gpio_request(SPI_IN, "spi_in");

    gpio_request(LCD_SPI_CS, "lcd_spi_cs");
    
    gpio_direction_input(SPI_IN);  
   
    gpio_direction_output(LCD_SPI_CS, 1);
    
}                                                                                                                         
static void GPIOSPIUNInit(void) 
{
    omap_mux_init_signal("gpio_171", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
    omap_mux_init_signal("gpio_172", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
    omap_mux_init_signal("gpio_173", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
    omap_mux_init_signal("gpio_174", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW); 
    omap_mux_init_signal("gpio_40", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
 
    gpio_request(LCD_RST, "lcd_reset");
    gpio_request(SPI_CLK, "spi_clk");
    gpio_request(SPI_OUT, "spi_out");
    gpio_request(SPI_IN, "spi_in");
    gpio_request(LCD_SPI_CS, "lcd_spi_cs");

	gpio_direction_output(LCD_RST, 0);
	gpio_direction_output(SPI_CLK, 0);
	gpio_direction_output(SPI_OUT, 0);
	gpio_direction_output(SPI_IN, 0);
	gpio_direction_output(LCD_SPI_CS, 0);
}
static void SetSDA(int HiLow)
{

	if(HiLow)
	{
        gpio_direction_output(SPI_OUT, 1);
	}
	else
	{
        gpio_direction_output(SPI_OUT, 0);
	}

}

static bool GetSDA(void)                
{
	UINT32 data;

    data = gpio_get_value(SPI_IN);

	if(data)      
	{
		return 1;
	}
	else                            
	{
		return 0;
	}
}
static void SetSCL(int HiLow)
{

	if(HiLow)
	{
        gpio_direction_output(SPI_CLK, 1);
	}
	else
	{
		gpio_direction_output(SPI_CLK, 0);
	}

}

static void LcdSetCS(int HiLow)
{
	if(HiLow)
	{
        gpio_direction_output(LCD_SPI_CS, 1);
	}
	else
	{
		gpio_direction_output(LCD_SPI_CS, 0);
	}


}

/*******************************************
call this function to write command
********************************************/
static void truly_hx8363_spi_write_cmd(char cmd)
{
	int bnum = 0;
	unsigned int bit;
	
 
	LcdSetCS(1);
	DelaySPI(20);
	
	SetSCL(0);
	DelaySPI(20);
	
	LcdSetCS(0);		
	DelaySPI(20);
	
	SetSDA(0);//cmd
	DelaySPI(20);
	
	SetSCL(1);		
	DelaySPI(20);		

        bnum = 8;	
        bit = 0x80;
	while (bnum) 
	{
		SetSCL(0);
		
		if (cmd & bit)
			SetSDA(1);
		else
			SetSDA(0);
		DelaySPI(20);//(130); 
		
		SetSCL(1);
		DelaySPI(20);//(260); 	
		
		bit >>= 1;
				
		bnum--;
	}
	LcdSetCS(1);
}

/*******************************************
call this function to write parameter
********************************************/
static void truly_hx8363_spi_write_data(char data)
{
	int bnum = 0;
	unsigned int bit;
	
 
	LcdSetCS(1);
	DelaySPI(20);
	
	SetSCL(0);
	DelaySPI(20);
	
	LcdSetCS(0);		
	DelaySPI(20);
	
	SetSDA(1);//data
	DelaySPI(20);
	
	SetSCL(1);		
	DelaySPI(20);		

        bnum = 8;	
        bit = 0x80;
	while (bnum) 
	{
		SetSCL(0);
		
		if (data & bit)
			SetSDA(1);
		else
			SetSDA(0);
		DelaySPI(20);//(130); 
		
		SetSCL(1);
		DelaySPI(20);//(260); 	
		
		bit >>= 1;

		bnum--;
	}
	LcdSetCS(1);
}

/*********************************************
The first one of data is command,
and the followings (total is num) are parametersthread_sleep
**********************************************/
static int truly_hx8363_spi_write (char *data, int num)
{

        char *bp;
	
	/*command type first*/
	bp = data;

	truly_hx8363_spi_write_cmd(*bp);     
                     

	/*followed by parameters bytes*/
	while ( num )
	{
	    bp+= 1;
		truly_hx8363_spi_write_data(*bp);
		num--;
	}
	return 0;

}

static char otp_read_id(char reg)
{     
	unsigned char x = 0 ;     
	unsigned char id = 0 ;     

	LcdSetCS(0);//gpio_out(spi_cs,0);     
	DelaySPI(20);//clk_busy_wait(1);     

	SetSCL(0);//gpio_out(spi_sclk,0);     
	DelaySPI(20);//clk_busy_wait(1);     

	SetSDA(0);//gpio_out(spi_mosi,0);     
	DelaySPI(20);//clk_busy_wait(1);     

	SetSCL(1);//gpio_out(spi_sclk,1);     
	for(x=0 ; x < 8 ;x++)     
	{         
		SetSCL(0);//gpio_out(spi_sclk,0);         
		if( (0x80 & reg) != 0)         
		{              
			SetSDA(1);//gpio_out(spi_mosi,1);         
		}	  
		else	  
		{	       
			SetSDA(0);//gpio_out(spi_mosi,0);	  
		}	  
		reg <<= 1 ;	  
		SetSCL(1);//gpio_out(spi_sclk,1);     
	} 
	
	for(x = 0 ; x < 8 ; x++)     
	{         
		id <<= 1 ;         
		SetSCL(0);//gpio_out(spi_sclk,0);	  
		DelaySPI(100);//clk_busy_wait(2000);	  
		if( (GetSDA()) !=0)	  
		{
			id++;	  
		}	  
		SetSCL(1);//gpio_out(spi_sclk,1);	  
		DelaySPI(100);//clk_busy_wait(2000);     
	}     
	LcdSetCS(1);//gpio_out(spi_cs,1);     
	return id ;    
}
static unsigned char himax8363_a_read_panel_id(unsigned char reg_add)
{    	
	unsigned char otp_id = 0 ;    /*add by ykl*/    	
	truly_hx8363_spi_write(truly_spi_data_read, sizeof(truly_spi_data_read)/sizeof(truly_spi_data_read[0]) - 1);
	otp_id = otp_read_id(reg_add);    	
	return otp_id ;
}

static unsigned char himax8369_a_read_panel_id(unsigned char reg_add)
{    	
	unsigned char otp_id = 0 ;       	
	truly_hx8363_spi_write(byx_hx8369_spi_data_read_one, sizeof(byx_hx8369_spi_data_read_one)/sizeof(byx_hx8369_spi_data_read_one[0]) - 1);
	truly_hx8363_spi_write(byx_hx8369_spi_data_read_two, sizeof(byx_hx8369_spi_data_read_two)/sizeof(byx_hx8369_spi_data_read_two[0]) - 1);
	otp_id = otp_read_id(reg_add);   
 	
	return otp_id ;
}

static void truly_hx8363_disp_on(void)
{
	int data_temp=0;
	unsigned char panel_id = 0 ;    

	unsigned char reg_add_himax8363 = 0xDA;	 
	unsigned char reg_add_himax8369 = 0xFF;	    
	
	LcdSetCS(1); 
	SetSCL(1);
	SetSDA(1);


	gpio_direction_output(LCD_RST, 0);
	//msdelay(100);
	msdelay(25);//yangliang modify 12.3.3
	gpio_direction_output(LCD_RST, 1);
 	//msdelay(100);
	msdelay(25);//yangliang modify 12.3.3


	panel_id = himax8369_a_read_panel_id(reg_add_himax8369);
	if(0x69==panel_id)
	{
		truly_hx8363_spi_write(byx_hx83639_spi_data_extc, sizeof(byx_hx83639_spi_data_extc)/sizeof(byx_hx83639_spi_data_extc[0]) - 1);
		truly_hx8363_spi_write(byx_hx8369_spi_data_power, sizeof(byx_hx8369_spi_data_power)/sizeof(byx_hx8369_spi_data_power[0]) - 1);
		truly_hx8363_spi_write(byx_hx8369_spi_data_display, sizeof(byx_hx8369_spi_data_display)/sizeof(byx_hx8369_spi_data_display[0]) - 1);

		truly_hx8363_spi_write(byx_hx8369_spi_data_cyc, sizeof(byx_hx8369_spi_data_cyc)/sizeof(byx_hx8369_spi_data_cyc[0]) - 1);
		truly_hx8363_spi_write(byx_hx8369_spi_data_vcom, sizeof(byx_hx8369_spi_data_vcom)/sizeof(byx_hx8369_spi_data_vcom[0]) - 1);
		truly_hx8363_spi_write(byx_hx8369_spi_data_panel, sizeof(byx_hx8369_spi_data_panel)/sizeof(byx_hx8369_spi_data_panel[0]) - 1);
		truly_hx8363_spi_write(byx_hx8369_spi_data_gip, sizeof(byx_hx8369_spi_data_gip)/sizeof(byx_hx8369_spi_data_gip[0]) - 1);
		truly_hx8363_spi_write(byx_hx8369_spi_data_gamma, sizeof(byx_hx8369_spi_data_gamma)/sizeof(byx_hx8369_spi_data_gamma[0]) - 1);
		truly_hx8363_spi_write(byx_hx8369_spi_data_DGC, sizeof(byx_hx8369_spi_data_DGC)/sizeof(byx_hx8369_spi_data_DGC[0]) - 1);
	
		truly_hx8363_spi_write(byx_hx8369_spi_data_colmod, sizeof(byx_hx8369_spi_data_colmod)/sizeof(byx_hx8369_spi_data_colmod[0]) - 1);
		truly_hx8363_spi_write(byx_hx8369_spi_data_sleepout, sizeof(byx_hx8369_spi_data_sleepout)/sizeof(byx_hx8369_spi_data_sleepout[0]) - 1);

		if(resume_in_sleep==0)
			msdelay(120);   //qian suspend  
		else
			msdelay(240);   //sheng suspend  

		truly_hx8363_spi_write(byx_hx8369_spi_data_dispon, sizeof(byx_hx8369_spi_data_dispon)/sizeof(byx_hx8369_spi_data_dispon[0]) - 1);
		msdelay(30);
	}
	else
	{
		panel_id = 0;
		panel_id = himax8363_a_read_panel_id(reg_add_himax8363);
		if(0x01==panel_id)
		{
			//boyi
			truly_hx8363_spi_write(by_hx8363_spi_data_extc, sizeof(by_hx8363_spi_data_extc)/sizeof(by_hx8363_spi_data_extc[0]) - 1);
	    		truly_hx8363_spi_write(by_hx8363_spi_data_power, sizeof(by_hx8363_spi_data_power)/sizeof(by_hx8363_spi_data_power[0]) - 1);
truly_hx8363_spi_write(by_hx8363_spi_data_sleepout, sizeof(by_hx8363_spi_data_sleepout)/sizeof(by_hx8363_spi_data_sleepout[0]) - 1);                   
   	 		msdelay(150);   //150ms 

	    		truly_hx8363_spi_write(by_hx8363_spi_data_colmod, sizeof(by_hx8363_spi_data_colmod)/sizeof(by_hx8363_spi_data_colmod[0]) - 1);
	    		truly_hx8363_spi_write(by_hx8363_spi_data_mac, sizeof(by_hx8363_spi_data_mac)/sizeof(by_hx8363_spi_data_mac[0]) - 1);
	    		truly_hx8363_spi_write(by_hx8363_spi_data_rgbif, sizeof(by_hx8363_spi_data_rgbif)/sizeof(by_hx8363_spi_data_rgbif[0]) - 1);
	    		truly_hx8363_spi_write(by_hx8363_spi_data_cyc, sizeof(by_hx8363_spi_data_cyc)/sizeof(by_hx8363_spi_data_cyc[0]) - 1);
	    		//truly_hx8363_spi_write(by_hx8363_spi_data_ptba, sizeof(by_hx8363_spi_data_ptba)/sizeof(by_hx8363_spi_data_ptba[0]) - 1);
	    		truly_hx8363_spi_write(by_hx8363_spi_data_vcom, sizeof(by_hx8363_spi_data_vcom)/sizeof(by_hx8363_spi_data_vcom[0]) - 1);
   		truly_hx8363_spi_write(by_hx8363_spi_data_panel, sizeof(by_hx8363_spi_data_panel)/sizeof(by_hx8363_spi_data_panel[0]) - 1);                
	    		msdelay(10);   //10ms 
		
		truly_hx8363_spi_write(by_hx8363_spi_data_gamma, sizeof(by_hx8363_spi_data_gamma)/sizeof(by_hx8363_spi_data_gamma[0]) - 1);                
    		msdelay(10);   //10ms 
		
    		truly_hx8363_spi_write(by_hx8363_spi_data_dgc, sizeof(by_hx8363_spi_data_dgc)/sizeof(by_hx8363_spi_data_dgc[0]) - 1);		
    		msdelay(10);   //10ms 
    
    		truly_hx8363_spi_write(by_hx8363_spi_data_dispon, sizeof(by_hx8363_spi_data_dispon)/sizeof(by_hx8363_spi_data_dispon[0]) - 1);
	
		}
		else
		{
		//truly
		truly_hx8363_spi_write(truly_hx8363_spi_data_extc, sizeof(truly_hx8363_spi_data_extc)/sizeof(truly_hx8363_spi_data_extc[0]) - 1);
    		truly_hx8363_spi_write(truly_hx8363_spi_data_power, sizeof(truly_hx8363_spi_data_power)/sizeof(truly_hx8363_spi_data_power[0]) - 1);
    		truly_hx8363_spi_write(truly_hx8363_spi_data_sleepout, sizeof(truly_hx8363_spi_data_sleepout)/sizeof(truly_hx8363_spi_data_sleepout[0]) - 1);                   
    		msdelay(150);   //150ms 

    		truly_hx8363_spi_write(truly_hx8363_spi_data_colmod, sizeof(truly_hx8363_spi_data_colmod)/sizeof(truly_hx8363_spi_data_colmod[0]) - 1);
    		truly_hx8363_spi_write(truly_hx8363_spi_data_mac, sizeof(truly_hx8363_spi_data_mac)/sizeof(truly_hx8363_spi_data_mac[0]) - 1);
    		truly_hx8363_spi_write(truly_hx8363_spi_data_rgbif, sizeof(truly_hx8363_spi_data_rgbif)/sizeof(truly_hx8363_spi_data_rgbif[0]) - 1);
    		truly_hx8363_spi_write(truly_hx8363_spi_data_cyc, sizeof(truly_hx8363_spi_data_cyc)/sizeof(truly_hx8363_spi_data_cyc[0]) - 1);
    		truly_hx8363_spi_write(truly_hx8363_spi_data_ptba, sizeof(truly_hx8363_spi_data_ptba)/sizeof(truly_hx8363_spi_data_ptba[0]) - 1);
    		truly_hx8363_spi_write(truly_hx8363_spi_data_vcom, sizeof(truly_hx8363_spi_data_vcom)/sizeof(truly_hx8363_spi_data_vcom[0]) - 1);
    		truly_hx8363_spi_write(truly_hx8363_spi_data_panel, sizeof(truly_hx8363_spi_data_panel)/sizeof(truly_hx8363_spi_data_panel[0]) - 1);                
    		msdelay(10);   //10ms 
		
    		truly_hx8363_spi_write(truly_hx8363_spi_data_gamma, sizeof(truly_hx8363_spi_data_gamma)/sizeof(truly_hx8363_spi_data_gamma[0]) - 1);                
    		msdelay(10);   //10ms 
		
    		truly_hx8363_spi_write(truly_hx8363_spi_data_dgc, sizeof(truly_hx8363_spi_data_dgc)/sizeof(truly_hx8363_spi_data_dgc[0]) - 1);		
    		msdelay(10);   //10ms 
    
    		truly_hx8363_spi_write(truly_hx8363_spi_data_dispon, sizeof(truly_hx8363_spi_data_dispon)/sizeof(truly_hx8363_spi_data_dispon[0]) - 1);
		}

	}

	printk(KERN_CRIT"hx8363_disp_on panel_id=<%d>\n",panel_id);
}

void lcd_disp_on(struct spi_device *spi)
{
	//extern void turn_on_pll2(void);
	//turn_on_pll2();
	//turn_off_lcd_power();
    GPIOSPIInit();
    //turn_on_lcd_power();  
	
 	omap_mux_init_signal("gpio_40", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW | OMAP_PULL_ENA | OMAP_PULL_UP);
	gpio_request(LCD_RST, "lcd_reset");
	gpio_direction_output(LCD_RST, 1);
	msdelay(25);//50 to 25 yangliang modify 12.3.3

	truly_hx8363_disp_on();

    printk(KERN_CRIT"lcd display on\n"); 
}

void lcd_disp_off(struct spi_device *spi)
{
     GPIOSPIUNInit();
    printk(KERN_CRIT"lcd display off\n"); 
}
//********************************* CP8930 lcd end *****************************************


static int cp_n930_sharp_lcd_panel_update_bl_status(struct backlight_device *dev) 
{ 
  struct omap_dss_device *dssdev =  dev_get_drvdata(&dev->dev); 
  unsigned int level = 0;
  
  yl_lcd_debug(LOG_INFO, "%s\n",__func__);
  
  if (dev->props.fb_blank == FB_BLANK_UNBLANK && dev->props.power == FB_BLANK_UNBLANK) 
  {
    level = dev->props.brightness; 
  }
  else 
  {
    level = 0;
  } 
  
  if (dssdev->set_backlight == NULL) 
  {
    return -ENODEV; 
  }
  
  return dssdev->set_backlight(dssdev, level);
} 
  
static int cp_n930_sharp_lcd_panel_get_bl_intensity(struct backlight_device *dev) 
{ 
  struct omap_dss_device *dssdev =  dev_get_drvdata(&dev->dev); 
  
 yl_lcd_debug(LOG_INFO, "%s\n",__func__);
  
  if (dssdev->get_backlight == NULL) 
  {
    return -ENODEV; 
  }
  return dssdev->get_backlight(dssdev); 
} 

static struct backlight_ops cp_n930_sharp_lcd_bl_ops = 
{ 
  .get_brightness = cp_n930_sharp_lcd_panel_get_bl_intensity, 
  .update_status  = cp_n930_sharp_lcd_panel_update_bl_status, 
}; 


static int cp_n930_sharp_lcd_panel_probe(struct omap_dss_device *dssdev)
{
  struct backlight_device *bldev; 
  
  DSSDBG("%s\n",__func__);

  dssdev->panel.config = OMAP_DSS_LCD_TFT | OMAP_DSS_LCD_IVS |
      OMAP_DSS_LCD_IHS;
  dssdev->panel.timings = cp_n930_sharp_lcd_panel_timings;
  dssdev->panel.recommended_bpp = 24;
  
  bldev =  backlight_device_register("zoom2", &dssdev->dev, dssdev, &cp_n930_sharp_lcd_bl_ops, NULL); 
  cp_n930_sharp_lcd_drv_data.bl_dev = bldev; 
  bldev->props.fb_blank = FB_BLANK_UNBLANK; 
  bldev->props.max_brightness = 255; 
  bldev->props.brightness = 100;
  return 0;
}

static void cp_n930_sharp_lcd_panel_remove(struct omap_dss_device *dssdev)
{
  return;
}

static int cp_n930_sharp_lcd_panel_enable(struct omap_dss_device *dssdev)
{
  int r = 0;
  
  DSSDBG("%s\n",__func__);

#ifdef CONFIG_OMAP2_DSS_DPI
  r = omapdss_dpi_display_enable(dssdev);
  if (r)
  {   
	printk(KERN_CRIT"%s:dpi enabled failed!\n",__func__);
	return r;
  }	
#endif
		
  if (dssdev->platform_enable)
    r = dssdev->platform_enable(dssdev);

  return r;
}

static int cp_n930_sharp_lcd_panel_disable(struct omap_dss_device *dssdev)
{

   DSSDBG("%s\n",__func__);

   if (dssdev->platform_disable)
   {
       dssdev->platform_disable(dssdev);
   }
#ifdef CONFIG_OMAP2_DSS_DPI   
  	mdelay(4);
	omapdss_dpi_display_disable(dssdev);  
#endif	
	return 0;
}

static void cp_n930_sharp_lcd_panel_on_dss_gpio_cfg(void)
{
	// configure gpio_66 to gpio_93 as mode0 DSS data lines and control signals
	omap_mux_init_signal("dss_hsync", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_vsync", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_acbias", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_pclk", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);

	omap_mux_init_signal("dss_data0.dss_data0", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data1.dss_data1", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data2.dss_data2", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data3.dss_data3", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data4.dss_data4", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data5.dss_data5", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data6.dss_data6", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data7.dss_data7", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data8.dss_data8", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data9.dss_data9", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data10.dss_data10", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data11.dss_data11", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data12.dss_data12", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data13.dss_data13", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data14.dss_data14", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data15.dss_data15", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data16.dss_data16", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data17.dss_data17", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data18.dss_data18", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data19.dss_data19", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data20.dss_data20", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data21.dss_data21", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data22.dss_data22", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	omap_mux_init_signal("dss_data23.dss_data23", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);
	}

static void cp_n930_sharp_lcd_panel_off_dss_gpio_cfg(void)
{
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

	gpio_request(OMAP36XX_DSS_D0_GPIO, "lcd_d0");
    gpio_request(OMAP36XX_DSS_D1_GPIO, "lcd_d1");
    gpio_request(OMAP36XX_DSS_D2_GPIO, "lcd_d2");
    gpio_request(OMAP36XX_DSS_D3_GPIO, "lcd_d3");
	gpio_request(OMAP36XX_DSS_D4_GPIO, "lcd_d4");
    gpio_request(OMAP36XX_DSS_D5_GPIO, "lcd_d5");
    gpio_request(OMAP36XX_DSS_D6_GPIO, "lcd_d6");
    gpio_request(OMAP36XX_DSS_D7_GPIO, "lcd_d7");
	gpio_request(OMAP36XX_DSS_D8_GPIO, "lcd_d8");
    gpio_request(OMAP36XX_DSS_D9_GPIO, "lcd_d9");
    gpio_request(OMAP36XX_DSS_D10_GPIO, "lcd_d10");
    gpio_request(OMAP36XX_DSS_D11_GPIO, "lcd_d11");
	gpio_request(OMAP36XX_DSS_D12_GPIO, "lcd_d12");
    gpio_request(OMAP36XX_DSS_D13_GPIO, "lcd_d13");
    gpio_request(OMAP36XX_DSS_D14_GPIO, "lcd_d14");
	gpio_request(OMAP36XX_DSS_D15_GPIO, "lcd_d15");
    gpio_request(OMAP36XX_DSS_D16_GPIO, "lcd_d16");
    gpio_request(OMAP36XX_DSS_D17_GPIO, "lcd_d17");
    gpio_request(OMAP36XX_DSS_D18_GPIO, "lcd_d18");
	gpio_request(OMAP36XX_DSS_D19_GPIO, "lcd_d19");
    gpio_request(OMAP36XX_DSS_D20_GPIO, "lcd_d20");
    gpio_request(OMAP36XX_DSS_D21_GPIO, "lcd_d21");
    gpio_request(OMAP36XX_DSS_D22_GPIO, "lcd_d22");
	gpio_request(OMAP36XX_DSS_D23_GPIO, "lcd_d23");

    gpio_request(OMAP36XX_DSS_HSYNC_GPIO, "lcd_hsync");
    gpio_request(OMAP36XX_DSS_VSYNC_GPIO, "lcd_vsync");
    gpio_request(OMAP36XX_DSS_PCLK_GPIO, "lcd_pclk");

    gpio_request(OMAP36XX_DSS_DE_GPIO, "lcd_de");

  // make the lcd hsync gpio high
  gpio_direction_output(OMAP36XX_DSS_HSYNC_GPIO, 0);
  
  // make the lcd vsync gpio high
  gpio_direction_output(OMAP36XX_DSS_VSYNC_GPIO, 0);
    
  // make the lcd data enable gpio high

  gpio_direction_output(OMAP36XX_DSS_DE_GPIO, 0);

  
  // make lcd pclk low
  gpio_direction_output(OMAP36XX_DSS_PCLK_GPIO, 0);
  
  // make lcd data lines d0-23 low
  gpio_direction_output(OMAP36XX_DSS_D0_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D1_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D2_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D3_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D4_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D5_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D6_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D7_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D8_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D9_GPIO , 0);
  gpio_direction_output(OMAP36XX_DSS_D10_GPIO, 0); 
  gpio_direction_output(OMAP36XX_DSS_D11_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D12_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D13_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D14_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D15_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D16_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D17_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D18_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D19_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D20_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D21_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D22_GPIO, 0);
  gpio_direction_output(OMAP36XX_DSS_D23_GPIO, 0);
}
extern void dispc_close_irqenable(void);

extern int zoom_panel_power_enable(int enable);
void lcd_panel_resume(void)
{
	turn_on_lcd_power();
	zoom_panel_power_enable(1);
	
	resume_in_sleep=1;		
}

static int cp_n930_sharp_lcd_panel_suspend(struct omap_dss_device *dssdev)
{
   DSSDBG("%s\n",__func__);
   
//	if (dssdev->platform_disable)
//    dssdev->platform_disable(dssdev);
  zoom_pwm_config_pwmoff();
  mdelay(1);

  lcd_disp_off(cp_n930_sharp_lcd_spi_device);
  turn_off_lcd_power();
  
  cp_n930_sharp_lcd_panel_off_dss_gpio_cfg();

 // dispc_close_irqenable();
  return cp_n930_sharp_lcd_panel_disable(dssdev);
}

extern int ft5x0x_tw_is_or_not_power_on;
extern u8 is_atmel_power_on;

static int cp_n930_sharp_lcd_panel_resume(struct omap_dss_device *dssdev)
{
	int r = 0;
  printk(KERN_CRIT"%s:enter\n",__func__);
  
 // cp_n930_sharp_lcd_panel_enable(dssdev);
  extern char stream_in_playing;
  extern char stream_in_a2dp;
  extern char stream_in_recording;
  char stream_active=stream_in_recording || stream_in_a2dp;

  if(stream_in_playing && (smartphone_calling_enable == 0))
	stream_active |= stream_in_playing;	
  printk("%s:stream_in_recording=%d,stream_in_a2dp=%d,===>%d\n",__func__,stream_in_recording,stream_in_a2dp,stream_active);
  
	  if(resume_in_sleep==0)
  {
	  turn_on_lcd_power();  
	  zoom_panel_power_enable(1);
  }
//  else resume_in_sleep=0;

  if(!stream_active)
  {
  	spin_lock(&panel_lock);
  }

#ifdef CONFIG_OMAP2_DSS_DPI
  r = omapdss_dpi_display_enable(dssdev);
  if (r)
  {   
	printk(KERN_CRIT"%s:dpi enabled failed!\n",__func__);
  }	
#endif
 
  
  cp_n930_sharp_lcd_panel_on_dss_gpio_cfg();


    lcd_disp_on(cp_n930_sharp_lcd_spi_device);
    resume_in_sleep=0;
	if( ft5x0x_tw_is_or_not_power_on == 1 )
		ft5x0x_poweron();
	else
	{
		atmel_poweron();
		is_atmel_power_on = 1;
	}
//	mdelay(60);//zhuhui altered

  mdelay(110); //yangliang restore 70 to 80 3.8  4.28 60-110
  if (dssdev->platform_enable)
    r = dssdev->platform_enable(dssdev);
  
  if(!stream_active)
  {
	spin_unlock(&panel_lock);    
  }


  return 0;
}

static struct omap_dss_driver cp_n930_sharp_lcd_driver = {
  .probe      = cp_n930_sharp_lcd_panel_probe,
  .remove     = cp_n930_sharp_lcd_panel_remove,
  .enable     = cp_n930_sharp_lcd_panel_enable,
  .disable    = cp_n930_sharp_lcd_panel_disable,
  .suspend    = cp_n930_sharp_lcd_panel_suspend,
  .resume     = cp_n930_sharp_lcd_panel_resume,

  .driver     = {
    .name     = "cp9930_sharp_lcd_panel",
    .owner    = THIS_MODULE,
  },
};


static int cp_n930_sharp_lcd_spi_probe(struct spi_device *spi)
{
	printk("cp_n930_sharp_lcd_spi_probe\n");
  cp_n930_sharp_lcd_spi_device = spi;


  //cp_n930_sharp_lcd_panel_on_dss_gpio_cfg();
  //lcd_disp_on(spi);
  // turn_on_lcd_power();  
  omap_dss_register_driver(&cp_n930_sharp_lcd_driver);
  return 0;
}

static int cp_n930_sharp_lcd_spi_remove(struct spi_device *spi)
{
  omap_dss_unregister_driver(&cp_n930_sharp_lcd_driver);

  return 0;
}

static int cp_n930_sharp_lcd_spi_suspend(struct spi_device *spi, pm_message_t mesg)
{
    return 0;
}

static int cp_n930_sharp_lcd_spi_resume(struct spi_device *spi)
{
    return 0;
}

static struct spi_driver cp_n930_sharp_lcd_spi_driver = {
  .probe    = cp_n930_sharp_lcd_spi_probe,
  .remove   = __devexit_p(cp_n930_sharp_lcd_spi_remove),
  .suspend  = cp_n930_sharp_lcd_spi_suspend,
  .resume   = cp_n930_sharp_lcd_spi_resume,
  .driver   = {
    .name   = "cp9930_lr388_spi",
    .bus    = &spi_bus_type,
    .owner  = THIS_MODULE,
  },
};

static int __init cp_n930_sharp_lcd_init(void)
{
   return spi_register_driver(&cp_n930_sharp_lcd_spi_driver);
}

static void __exit cp_n930_sharp_lcd_exit(void)
{
  return spi_unregister_driver(&cp_n930_sharp_lcd_spi_driver);
}


module_init(cp_n930_sharp_lcd_init);
module_exit(cp_n930_sharp_lcd_exit);

MODULE_AUTHOR("Tushar Bangoria");
MODULE_LICENSE("GPL");

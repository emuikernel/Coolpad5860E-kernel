/*
 * drivers/video/omap2/displays/panel-cp9930-sharp-lcd.h
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

#define LCD_XRES                                  (480)
#define LCD_YRES                                  (960)

// TYP, MIN and MAX pixel clock
#define LCD_PIXCLOCK_MIN                          (24700) 
#define LCD_PIXCLOCK_TYP                          (26000) 
#define LCD_PIXCLOCK_MAX                          (27300) 

// Current Pixel clock
#define LCD_PIXEL_CLOCK                           (LCD_PIXCLOCK_TYP)

// LCD horizontal timing
#define LCD_TIMING_H_HBP                          (24)
#define LCD_TIMING_H_HSW                          (16)
#define LCD_TIMING_H_HFP                          (16)

// LCD vertical timing
#define LCD_TIMING_V_VBP                          (3)  
#define LCD_TIMING_V_VSW                          (2)
#define LCD_TIMING_V_VFP                          (4) 

// LCD reset high and low duration
#define LCD_RESET_HIGH_DURATION_MS                (6)
#define LCD_RESET_LOW_DURATION_MS                 (2)

// LCD related defines (delay required for sending next commands after LCD suspend or resume command)
#define LCD_RESET_DELAY_MS                        (130)

// LCD command and data prefix to be send for config using spi
#define LCD_SPI_PRE_CMD_BYTES                     (0x0000)
#define LCD_SPI_PRE_DATA_BYTES                    (0x0100)

// LCD commands
#define LCD_DISPLAY_ON                            (0x29)
#define LCD_DISPLAY_OFF                           (0x28)
#define LCD_SLEEP_IN                              (0x10)
#define LCD_SLEEP_OUT                             (0x11)

#define REG_SHARP_LCD_ADDR_MODE_SETTINGS          (0x36) 
#define VAL_SHARP_LCD_ADDR_MODE_SETTINGS          (0x00)

#define REG_SHARP_LCD_PIX_FMT_SETTINGS            (0x3A)
#define VAL_SHARP_LCD_PIX_FMT_SETTINGS            (0x70)

#define REG_SHARP_LCD_CMD_ACCESS_CONTROL          (0xB0) 
#define VAL_SHARP_LCD_CMD_ACCESS_CONTROL_ENABLE   (0x00) 
#define VAL_SHARP_LCD_CMD_ACCESS_CONTROL_DISABLE  (0x03) 

#define REG_SHARP_LCD_BLC_SETTINGS                (0xB8)
#define VAL_SHARP_LCD_BLC_SETTINGS                (0x00) 

#define REG_SHARP_LCD_LED_PWM_AND_BRIGHTNESS      (0xB9)
#define VAL_SHARP_LCD_LED_PWM                     (0x00) 
#define VAL_SHARP_LCD_LED_BRIGHTNESS              (0x00)


#if 1
#define DSSDBG(format, ...) \
	if (1) \
		printk(KERN_CRIT "lcd-panel: " format, \
		## __VA_ARGS__)
#else /* DEBUG */
#define DSSDBG(format, ...)
#endif




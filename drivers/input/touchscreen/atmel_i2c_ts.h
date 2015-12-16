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
 */
 
#include <linux/interrupt.h>
#include <linux/earlysuspend.h>
#include <linux/i2c.h>
#ifndef __LINUX_QT602240_TS_H
#define __LINUX_QT602240_TS_H

/* Orient */
#define ATMEL_I2C_RMI_NAME          "qt602240_ts" 
#define QT602240_NORMAL				0x0
#define QT602240_DIAGONAL			0x1
#define QT602240_HORIZONTAL_FLIP	0x2
#define QT602240_ROTATED_90_COUNTER	0x3
#define QT602240_VERTICAL_FLIP		0x4
#define QT602240_ROTATED_90			0x5
#define QT602240_ROTATED_180		0x6
#define QT602240_DIAGONAL_COUNTER	0x7
#define QT602240_MAX_FINGER          5

struct qt602240_info {
	u8 family_id;
	u8 variant_id;
	u8 version;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 object_num;
};

struct qt602240_finger {
	int status;
	int x;
	int y;
	int area;
};


/* The platform data for the AT42QT602240/ATMXT224 touchscreen driver */
struct qt602240_platform_data {
	unsigned int x_line;
	unsigned int y_line;
	unsigned int x_size;
	unsigned int y_size;
	unsigned int blen;
	unsigned int threshold;
	unsigned int voltage;
//	unsigned char orient;
	
/****************sz************************/
    int  (*gpio_setup)(int enable);	
	int (*power) (unsigned long  on);		/* Only valid in first array entry */
	int (*dev_setup) (bool enable);         //added by shezhi for regulator 2011.05.08
	unsigned long  irqflags;
	unsigned long  flags;
/******************sz************************/	
};


struct qt602240_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	const struct qt602240_platform_data *pdata;
	struct qt602240_object *object_table;
	struct qt602240_info info;
	struct qt602240_finger finger[QT602240_MAX_FINGER];
	unsigned int irq;
	struct work_struct work;
    /*******************sz***********/
	uint16_t addr;   
	struct early_suspend early_suspend; 
	int use_irq;
	int enable_irq_state;
	uint32_t flags;                   
	/*******************sz***********/
};


#endif /* __LINUX_QT602240_TS_H */

/*
 * linux/arch/arm/mach-omap2/board-zoom2-wifi.h
 *
 * Copyright (C) 2010 Texas Instruments Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _BOARD_ZOOM2_WIFI_H
#define _BOARD_ZOOM2_WIFI_H

/*
#define ZOOM2_WIFI_PMENA_GPIO	101
#define ZOOM2_WIFI_IRQ_GPIO	162
*/
//yulong.huangjiefeng.2011-2-28
#define ZOOM2_WIFI_IRQ_GPIO	25
#define ZOOM2_WIFI_PMENA_GPIO	26
#define BT_EN_GPIO 27
//#define WL127X_FMEN_GPIO    27

void config_wlan_mux(void);

#endif /* _BOARD_ZOOM2_WIFI_H */

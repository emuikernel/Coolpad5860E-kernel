/*
 * misc.h
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
 
#ifndef _OMAP2_MCSPI_H
#define _OMAP2_MCSPI_H

#define OMAP2_MCSPI_MASTER 0
#define OMAP2_MCSPI_SLAVE  1

#define YL_UART_SWITCH_AP			          1
#define YL_UART_SWITCH_WIFI_LOG		          2
#define YL_UART_SWITCH_MODEM 	              3
#define YL_UART_SWITCH_WIFI_RF_TEST           4
#define YL_USB_SWITCH_MODEM                   5
#define YL_USB_SWITCH_AP                      6

#define YL_USB_VIA_ANA_MODEM    7            //added by zhuhui


//added for test omap clock by huangjiefeng in 20100809
#define   CHECK_NO_SUSPEND                1
#define   CHECK_WORK_CLOCK                2
#define   CHECK_POWER_DOMAIN              3

#define   CHAGNGE_NOTIFY_BATTERY_TIME     9
#define   SET_LOG_LEVEL1                  10
#define   SET_LOG_LEVEL2                  11
#define   PRINT_CP_CPU_IDLE               12
#define   CHECK_WAKE_LOCK_STATE           13
#define   SET_CHARGER_IDENTIFY_TIMEOUT    14
#define   OTHER_TEST                      15

//test suspend wakeup
#define   STRESS_TEST_SUSPEND_WAKEUP      8
#define   P0_BOARD                        0

extern int smartphone_calling_enable;
extern int omap_clock_test;
extern int test_suspend_wakeup;

struct omap2_mcspi_platform_config {
	unsigned short	num_cs;
		/* SPI is master or slave */
	unsigned short	mode;

	/* Use only DMA for data transfers */
	unsigned short	dma_mode;

	/* Force chip select mode */
	unsigned short	force_cs_mode;

	/* FIFO depth in bytes, max value 64 */
	unsigned short fifo_depth;

};

struct omap2_mcspi_device_config {
	unsigned turbo_mode:1;

	/* Do we want one channel enabled at the same time? */
	unsigned single_channel:1;

	/* SPI is master or slave */
	unsigned short mode:1;

	/* Use only DMA for data transfers */
	unsigned short dma_mode:1;

	/* Force chip select mode */
	unsigned short force_cs_mode:1;

	/* FIFO depth in bytes */
	unsigned short fifo_depth:8;
};

void wifi_gpio_suspend(void);
void wifi_gpio_resume(void);

void power_wl1271(int on);

int wl1271_is_power_off(void);
/*added by xiao 2010-8-4*/
int wifi_is_on(void);
int bt_is_on(void);

//added by huangjiefeng in 20110903
void yl_set_debug_enalbe(int modem_name, int value);
int yl_get_debug_enalbe(int modem_name);

/********end**********/

/*added by xiao for wl1271 power mamager 2010-5-6*/
struct wl1271_state_t {
	unsigned int wifi_state;
	spinlock_t lock_for_wl1271;
	unsigned int bt_state;
//	spinlock_t lock_for_bt;
	unsigned int WIFI_1V8;
};
/*added by xiao 2010-5-7 18.08*/
#define WL127X_BTEN_GPIO	27
#define ZOOM2_WIFI_PMENA_GPIO	26

int modem_get_battery_volt(void); 
bool get_battery_status(void);
unsigned int get_device_hardware_version(void);
void set_device_hardware_version(unsigned int version);
int get_timer_wakeup_status(void);
void set_timer_wakeup_flag(int flag);
void yl_usb_switch(int channel);
void yl_uart_switch(int channel);
void print_kernel_version(void);
void set_wakeup_source(int flag);
int test_charge_function(int enable, void *_twl);
int setup_charge_current(int charging_source, int bci_charging_current);
bool get_system_sts(void);

#endif

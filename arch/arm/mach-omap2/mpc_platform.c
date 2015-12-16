#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/init.h>
//#include "MPCOMM.h"

#if 0
#define   MODEM_WAKE_GPIO               127//唤醒TD output
#define   MODEM_COM_SEL1                128//在bootloader使用, input
#define   MODEM_COM_SEL2                129//在bootloader使用，Input, 这两个引脚用来控制TD是校条，综测，还是固件升级
#define   MODEM_SLEEP_TD2AP_GPIO        126//TD sleep状态引脚:input,TD高电平sleep
#define   MODEM_SLEEP_AP2TD_GPIO        155//AP sleep状态引脚:output, AP高电平sleep，需要确认待机会不会漏电!!!
#define   MODEM_RESET_GPIO              153//复位TD，output
#define   MODEM_ONOFF_GPIO              152//给TD上电，output
#define   MODEM_STATE_GPIO              163//查询TD模块状态，intput
#define   MODEM_RING_IRQ_GPIO           10
#endif

static struct resource yl_modem_platform_resource[] = 
{
#if 0
	[0] =
	{
		.start	    = MODEM_WAKE_GPIO,//output
		.end    	= MODEM_WAKE_GPIO,
		.flags		= IORESOURCE_IO,//IORESOURCE_MEM,
	},
	[1] =
	{
		.start		= MODEM_SLEEP_TD2AP_GPIO,//input
		.end    	= MODEM_SLEEP_TD2AP_GPIO,
		.flags		= IORESOURCE_IO,
	},
	[2] =
	{
		.start		= MODEM_SLEEP_AP2TD_GPIO,//output
		.end    	= MODEM_SLEEP_AP2TD_GPIO,
		.flags		= IORESOURCE_IO,
	},
	[3] =
	{
		.start		= MODEM_RESET_GPIO,//output
		.end    	= MODEM_RESET_GPIO,
		.flags		= IORESOURCE_IO,
	},
	[4] =
	{
		.start		= MODEM_ONOFF_GPIO,//output
		.end    	= MODEM_ONOFF_GPIO,
		.flags		= IORESOURCE_IO,
	},
	[5] =
	{
		.start		= MODEM_STATE_GPIO,//input
		.end    	= MODEM_STATE_GPIO,
		.flags		= IORESOURCE_IO,
	},
	[6] =
	{
		.start		= MODEM_COM_SEL1,//input
		.end    	= MODEM_COM_SEL1,
		.flags		= IORESOURCE_IO,
	},
	[7] =
	{
		.start		= MODEM_COM_SEL2,//input
		.end    	= MODEM_COM_SEL2,
		.flags		= IORESOURCE_IO,
	},
	[8] =
	{
		.start		= MODEM_RING_IRQ_GPIO,//input
		.end    	= MODEM_RING_IRQ_GPIO,
		.flags		= IORESOURCE_IRQ,
	},
#endif
};


static struct platform_device yl_modem_device = 
{
    .name			= "YULONG_MODEM",
    .id			    = -1,
    .num_resources  = ARRAY_SIZE(yl_modem_platform_resource),//资源数量
    .resource	    = yl_modem_platform_resource,
};

/************************************************************
  Function:         yl_modem_init()
  Description:      平台初始化函数
  Input:            none
  output：          none
  Return:           设备上下文
  History:
      <author>       <time>       <version>       <desc>
      黄捷峰 2009/4/20        1.0         build this module
************************************************************/
static int __init yl_modem_init(void)
{
	int RetVal = 0;
	printk(KERN_INFO"yl_modem_init()++\r\n");
	RetVal = platform_device_register(&yl_modem_device);
	if(0 != RetVal)//register fail
	{
		printk(KERN_INFO"yl_modem_init: platform register device is fail!\r\n");
		return RetVal;
	}
	printk(KERN_INFO"yl_modem_init()--\r\n");
	return RetVal;
}
arch_initcall(yl_modem_init);

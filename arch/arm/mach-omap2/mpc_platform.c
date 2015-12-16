#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/init.h>
//#include "MPCOMM.h"

#if 0
#define   MODEM_WAKE_GPIO               127//����TD output
#define   MODEM_COM_SEL1                128//��bootloaderʹ��, input
#define   MODEM_COM_SEL2                129//��bootloaderʹ�ã�Input, ������������������TD��У�����۲⣬���ǹ̼�����
#define   MODEM_SLEEP_TD2AP_GPIO        126//TD sleep״̬����:input,TD�ߵ�ƽsleep
#define   MODEM_SLEEP_AP2TD_GPIO        155//AP sleep״̬����:output, AP�ߵ�ƽsleep����Ҫȷ�ϴ����᲻��©��!!!
#define   MODEM_RESET_GPIO              153//��λTD��output
#define   MODEM_ONOFF_GPIO              152//��TD�ϵ磬output
#define   MODEM_STATE_GPIO              163//��ѯTDģ��״̬��intput
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
    .num_resources  = ARRAY_SIZE(yl_modem_platform_resource),//��Դ����
    .resource	    = yl_modem_platform_resource,
};

/************************************************************
  Function:         yl_modem_init()
  Description:      ƽ̨��ʼ������
  Input:            none
  output��          none
  Return:           �豸������
  History:
      <author>       <time>       <version>       <desc>
      �ƽݷ� 2009/4/20        1.0         build this module
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

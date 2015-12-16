/* 
 * drivers/input/touchscreen/ft5x0x_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver. 
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * VERSION      	DATE			AUTHOR
 *    1.0		  2010-01-05			WenFS
 *
 * note: only support mulititouch	Wenfs 2010-10-01
 */

//extern int change_cpu_freq_in_touch;
//int change_cpu_freq_in_touch = 0;   //deleted by huangjiefeng
//#define OMAP_SYNAPTICS_GPIO	   59 //105 	//129

#include <linux/i2c.h>
#include <linux/input.h>
#include "ft5x06_ts.h"
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/i2c/twl.h>
//#include <asm/jzsoc.h>
#include <plat/mux.h>
#include <plat/yl_debug.h>//added by huangjiefeng
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/slab.h>
#include <mach/mux.h>
#include <linux/irq.h>

//#include "touch_debug.h"
#include "touchscreen.h"

static struct i2c_client *this_client;
//static struct ft5x0x_ts_platform_data *pdata;
//const int touch_key_ft[2]={KEY_MENU,KEY_BACK};
const int touch_key_ft[4]={KEY_MENU,KEY_HOME,KEY_BACK,KEY_SEARCH};           /*+++++++++++altered by zhuhui 9.3 15:00++++++++++++++*/

extern u8 is_p0;

u8 tw_is_active_status = 0;
int ft5x0x_tw_is_or_not_power_on = 0;

#define CONFIG_FT5X0X_MULTITOUCH 1

#define FTS_SETTING_BUF_LEN      128

struct ts_event {
	u16	x1;
	u16	y1;
	u16	x2;
	u16	y2;
	u16	x3;
	u16	y3;
	u16	x4;
	u16	y4;
	u16	x5;
	u16	y5;
	u16	pressure;
    u8  touch_point;
};

struct ft5x0x_ts_data {
	struct input_dev	*input_dev;
	struct ts_event		event;
	struct work_struct 	pen_event_work;
	struct workqueue_struct *ts_workqueue;
	struct early_suspend	early_suspend;
	struct mutex device_mode_mutex;   /* Ensures that only one function can specify the Device Mode at a time. */
};


/***********************************************************************************************
Name	:	ft5x0x_i2c_rxdata 

Input	:	*rxdata
                     *length

Output	:	ret

function	:	

***********************************************************************************************/
static int ft5x0x_i2c_rxdata(char *rxdata, int length)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

    //msleep(1);
	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		yl_touch_debug(LOG_DEBUG, "msg %s i2c read error: %d\n", __func__, ret);
	
	return ret;
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int ft5x0x_i2c_txdata(char *txdata, int length)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

   	//msleep(1);
	ret = i2c_transfer(this_client->adapter, msg, 1);
	if (ret < 0)
		yl_touch_debug(LOG_DEBUG, "%s i2c write error: %d\n", __func__, ret);

	return ret;
}
/***********************************************************************************************
Name	:	 ft5x0x_write_reg

Input	:	addr -- address
                     para -- parameter

Output	:	

function	:	write register of ft5x0x

***********************************************************************************************/
static int ft5x0x_write_reg(u8 addr, u8 para)
{
    u8 buf[3];
    int ret = -1;

    buf[0] = addr;
    buf[1] = para;
    ret = ft5x0x_i2c_txdata(buf, 2);
    if (ret < 0) {
        yl_touch_debug(LOG_WARNING, "ft5x0x write reg failed! %#x ret: %d", buf[0], ret);
        return -1;
    }
    
    return 0;
}


/***********************************************************************************************
Name	:	ft5x0x_read_reg 

Input	:	addr
                     pdata

Output	:	

function	:	read register of ft5x0x

***********************************************************************************************/
static int ft5x0x_read_reg(u8 addr, u8 *pdata)
{
	int ret;
	u8 buf[2] = {0};	
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf,
		},
	};
        buf[0] = addr;
    //msleep(1);
	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
	{
		yl_touch_debug(LOG_DEBUG, "msg %s i2c read error: %d,addr=%d\n", __func__,ret,this_client->addr);
		printk(KERN_ERR "msg %s i2c read error: %d,addr=%d\n", __func__,ret,this_client->addr);
	}
    else 
	{
        yl_touch_debug(LOG_DEBUG, "msg %s i2c read ok: addr=%d\n", __func__,this_client->addr);
	}

	*pdata = buf[0];
	return ret;
  
}


/***********************************************************************************************
Name	:	 ft5x0x_read_fw_ver

Input	:	 void
                     

Output	:	 firmware version 	

function	:	 read TP firmware version

***********************************************************************************************/
static unsigned char ft5x0x_read_fw_ver(void)                //deleted by zhuhui for two firmware update
{
	unsigned char ver;
//    int i=0;
    int ret;
    ver=0;

    //for(;i<256;++i)
    //{
    //this_client->addr=i;
	ret=ft5x0x_read_reg(FT5X0X_REG_FIRMID, &ver);
    //}
	yl_touch_debug(LOG_DEBUG, "%s: %d\n", __func__,ret);
    if(ret<0) 
	{
		yl_touch_debug(LOG_DEBUG, "%s:######\n", __func__);
		return -1;
	}
	return(ver);
}/**/


#define CONFIG_SUPPORT_FTS_CTP_UPG


#ifdef CONFIG_SUPPORT_FTS_CTP_UPG

typedef enum
{
    ERR_OK,
    ERR_MODE,
    ERR_READID,
    ERR_ERASE,
    ERR_STATUS,
    ERR_ECC,
    ERR_DL_ERASE_FAIL,
    ERR_DL_PROGRAM_FAIL,
    ERR_DL_VERIFY_FAIL
}E_UPGRADE_ERR_TYPE;

typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit

#define FTS_NULL                0x0
#define FTS_TRUE                0x01
#define FTS_FALSE              0x0

#define I2C_CTPM_ADDRESS       0x38


void delay_qt_ms(unsigned long  w_ms)
{
    unsigned long i;
    unsigned long j;

    for (i = 0; i < w_ms; i++)
    {
        for (j = 0; j < 1000; j++)
        {
            udelay(1);
        }
    }
}


/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    
    ret=i2c_master_recv(this_client, pbt_buf, dw_lenth);

    if(ret<=0)
    {
        yl_touch_debug(LOG_DEBUG, "[TSP]i2c_read_interface error\n");
        return FTS_FALSE;
    }
  
    return FTS_TRUE;
}

/*
[function]: 
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    ret=i2c_master_send(this_client, pbt_buf, dw_lenth);
    if(ret<=0)
    {
        yl_touch_debug(LOG_DEBUG, "[TSP]i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
        return FTS_FALSE;
    }

    return FTS_TRUE;
}

/*
[function]: 
    send a command to ctpm.
[parameters]:
    btcmd[in]        :command code;
    btPara1[in]    :parameter 1;    
    btPara2[in]    :parameter 2;    
    btPara3[in]    :parameter 3;    
    num[in]        :the valid input parameter numbers, if only command code needed and no parameters followed,then the num is 1;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE btPara3,FTS_BYTE num)
{
    FTS_BYTE write_cmd[4] = {0};

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
    return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

/*
[function]: 
    write data to ctpm , the destination address is 0.
[parameters]:
    pbt_buf[in]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
    
    return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

/*
[function]: 
    read out data from ctpm,the destination address is 0.
[parameters]:
    pbt_buf[out]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
    return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}


/*
[function]: 
    burn the FW to ctpm.
[parameters]:(ref. SPEC)
    pbt_buf[in]    :point to Head+FW ;
    dw_lenth[in]:the length of the FW + 6(the Head length);    
    bt_ecc[in]    :the ECC of the FW
[return]:
    ERR_OK        :no error;
    ERR_MODE    :fail to switch to UPDATE mode;
    ERR_READID    :read id fail;
    ERR_ERASE    :erase chip fail;
    ERR_STATUS    :status error;
    ERR_ECC        :ecc error.
*/


#define    FTS_PACKET_LENGTH        128

static unsigned char CTPM_FW_OF[]=    //updated by zhuhui for firmware update
{
#include "offw.i"
};

static unsigned char CTPM_FW_OF_NEW[]=    //updated by zhuhui for firmware update
{
//#include "offwe.i"
#include "offwnew.i"
};

static unsigned char CTPM_FW_XL[]=   //added by zhuhui for firmware update
{
#include "xlfw.i"
};

int fts_ctpm_auto_clb(void)
{
    unsigned char uc_temp;
    unsigned char i ;

    yl_touch_debug(LOG_DEBUG, "[FTS] start auto CLB.\n");
    msleep(200);
    ft5x0x_write_reg(0, 0x40);  
    delay_qt_ms(100);   //make sure already enter factory mode
    ft5x0x_write_reg(2, 0x4);  //write command to start calibration
    delay_qt_ms(300);
    for(i=0;i<100;i++)
    {
        ft5x0x_read_reg(0,&uc_temp);
        if ( ((uc_temp&0x70)>>4) == 0x0)  //return to normal mode, calibration finish
        {
            break;
        }
        delay_qt_ms(200);
        yl_touch_debug(LOG_DEBUG, "[FTS] waiting calibration %d\n",i);
        
    }
    yl_touch_debug(LOG_DEBUG, "[FTS] calibration OK.\n");
    
    msleep(300);
    ft5x0x_write_reg(0, 0x40);  //goto factory mode
    delay_qt_ms(100);   //make sure already enter factory mode
    ft5x0x_write_reg(2, 0x5);  //store CLB result
    delay_qt_ms(300);
    ft5x0x_write_reg(0, 0x0); //return to normal mode 
    msleep(300);
    yl_touch_debug(LOG_DEBUG, "[FTS] store CLB result OK.\n");
    return 0;
}

E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    FTS_BYTE reg_val[2] = {0};
    FTS_DWRD i = 0;

    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp;
    FTS_DWRD  lenght;
    FTS_BYTE  packet_buf[FTS_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE bt_ecc;
    int      i_ret;

    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    ft5x0x_write_reg(0xfc,0xaa);
    delay_qt_ms(50);
     /*write 0x55 to register 0xfc*/
    ft5x0x_write_reg(0xfc,0x55);
    yl_touch_debug(LOG_DEBUG, "[TSP] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(30);   


    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );

    /*********Step 3:check READ-ID***********************/        
    cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
    if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
    {
        yl_touch_debug(LOG_DEBUG, "[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
        return ERR_READID;
        //i_is_new_protocol = 1;
    }

     /*********Step 4:erase app*******************************/
    cmd_write(0x61,0x00,0x00,0x00,1);
   
    delay_qt_ms(1500);
    yl_touch_debug(LOG_DEBUG, "[TSP] Step 4: erase. \n");

    /*********Step 5:write firmware(FW) to ctpm flash*********/
    bt_ecc = 0;
    yl_touch_debug(LOG_DEBUG, "[TSP] Step 5: start upgrade. \n");
    dw_lenth = dw_lenth - 8;
    packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    for (j=0;j<packet_number;j++)
    {
        temp = j * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        lenght = FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(lenght>>8);
        packet_buf[5] = (FTS_BYTE)lenght;

        for (i=0;i<FTS_PACKET_LENGTH;i++)
        {
            packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }
        
        byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);
        delay_qt_ms(FTS_PACKET_LENGTH/6 + 1);
        if ((j * FTS_PACKET_LENGTH % 1024) == 0)
        {
              yl_touch_debug(LOG_DEBUG, "[TSP] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
        }
    }

    if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
    {
        temp = packet_number * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;

        temp = (dw_lenth) % FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;

        for (i=0;i<temp;i++)
        {
            packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }

        byte_write(&packet_buf[0],temp+6);    
        delay_qt_ms(20);
    }

    //send the last six byte
    for (i = 0; i<6; i++)
    {
        temp = 0x6ffa + i;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        temp =1;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;
        packet_buf[6] = pbt_buf[ dw_lenth + i]; 
        bt_ecc ^= packet_buf[6];

        byte_write(&packet_buf[0],7);  
        delay_qt_ms(20);
    }

    /*********Step 6: read out checksum***********************/
    /*send the opration head*/
    cmd_write(0xcc,0x00,0x00,0x00,1);
    byte_read(reg_val,1);
    yl_touch_debug(LOG_DEBUG, "[TSP] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
    if(reg_val[0] != bt_ecc)
    {
        return ERR_ECC;
    }

    /*********Step 7: reset the new FW***********************/
    cmd_write(0x07,0x00,0x00,0x00,1);

//	msleep(300);//added by zhuhui

    return ERR_OK;
}

int get_tp_id(void)
{
    unsigned char uc_i2c_addr;             //I2C slave address (8 bit address)
    //unsigned char uc_io_voltage;           //IO Voltage 0---3.3v;	1----1.8v
    unsigned char uc_panel_factory_id;     //TP panel factory ID

    unsigned char buf[FTS_SETTING_BUF_LEN];
    FTS_BYTE reg_val[2] = {0};
    FTS_BYTE  auc_i2c_write_buf[10];
    //FTS_BYTE  packet_buf[FTS_SETTING_BUF_LEN + 6];
    FTS_DWRD i = 0;
    int      i_ret;

    uc_i2c_addr = 0x38;
    //uc_io_voltage = 0x1;//IO Voltage 0---3.3v;	1----1.8v
    uc_panel_factory_id = 0x5a;

    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    ft5x0x_write_reg(0xfc,0xaa);
    delay_qt_ms(50);
     /*write 0x55 to register 0xfc*/
    ft5x0x_write_reg(0xfc,0x55);
    yl_touch_debug(LOG_DEBUG, "[FTS] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(30);   

    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );

    /*********Step 3:check READ-ID***********************/        
    cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
    if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
    {
        yl_touch_debug(LOG_DEBUG, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
        return ERR_READID;
    }

    cmd_write(0xcd,0x0,0x00,0x00,1);
    byte_read(reg_val,1);
    yl_touch_debug(LOG_DEBUG, "bootloader version = 0x%x\n", reg_val[0]);


    /* --------- read current project setting  ---------- */
    //set read start address
    buf[0] = 0x3;
    buf[1] = 0x0;
    buf[2] = 0x78;
    buf[3] = 0x0;
    byte_write(buf, 4);
    byte_read(buf, FTS_SETTING_BUF_LEN);
    
    yl_touch_debug(LOG_DEBUG, "[FTS] old setting: uc_i2c_addr = 0x%x, uc_io_voltage = %d, uc_panel_factory_id = 0x%x\n",
        buf[0],  buf[2], buf[4]);
	return buf[4];
}

void off_then_on_tw(void)
{
	//turn off tw
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
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 1);//turn on tw
	mdelay(5);

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

	omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_RST_GPIO, "ts_rst");
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0); 
    mdelay(8);
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 1); 
	msleep(300);
}


int fts_ctpm_fw_upgrade_with_i_file(void)
{
    FTS_BYTE*     pbt_buf = FTS_NULL;
    int i_ret = 0;
	unsigned char uc_reg_value; //修改为了兼容不同的固件
	unsigned char chip_vendor_id;//added by zhuhui for two firmware update
    int ret = -1;//added by zhuhui for firmware update	

	uc_reg_value = ft5x0x_read_fw_ver();      //added by zhuhui for firmware update 
    yl_touch_debug(LOG_DEBUG, "[FST] Firmware version = 0x%x\n", uc_reg_value);
	printk(KERN_ERR"[FST] Firmware version = 0x%x\n", uc_reg_value);
    if(uc_reg_value==0xff)
    	return -1;

    ret=ft5x0x_read_reg(0xA8, &chip_vendor_id);//added by zhuhui for two firmware update
    yl_touch_debug(LOG_DEBUG, "[%s] chip_vendor_id:0x%x | ret:%d\n", __func__,chip_vendor_id,ret);
	printk(KERN_ERR"[%s] chip_vendor_id:0x%x | ret:%d\n", __func__,chip_vendor_id,ret);
    if(ret<0) 
    {
	    yl_touch_debug(LOG_DEBUG, "%s:######\n", __func__);
	    return -1;
    }

	if( ( chip_vendor_id == 0xa8 ) && ( uc_reg_value == 0xa6 ))
	{
		yl_touch_debug(LOG_DEBUG, "enter chip_vendor_id:0xa8 and uc_reg_value:0xa6\n");
		chip_vendor_id = get_tp_id();
	}
    
    //=========FW upgrade========================
	if( chip_vendor_id == 0x5A )
   	{
		yl_touch_debug(LOG_DEBUG, "\n\nenter chip_vendor_id==0x5a path\n\n");
		if( ( uc_reg_value < 0x11 ) || ( uc_reg_value == 0xa6 ) )
		{		
			yl_touch_debug(LOG_DEBUG, "\n\nenter XL firmware update!\n\n");

	   		pbt_buf = CTPM_FW_XL;
	  		//call the upgrade function
	   		i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW_XL));
			off_then_on_tw();
		}
		else 
		{
			i_ret = 0;
		}
  	}
   	else if( chip_vendor_id == 0x51 )
	{
		yl_touch_debug(LOG_DEBUG, "\n\nenter chip_vendor_id==0x51 path\n\n");
		if( uc_reg_value < 0xc)
		{
			yl_touch_debug(LOG_DEBUG, "\n\nenter OF firmware update!\n\n");

			pbt_buf = CTPM_FW_OF;
	  		//call the upgrade function
	   		i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW_OF));
			off_then_on_tw();
		}
		else if( ( (uc_reg_value > 0xc) && (uc_reg_value < 0x11) ) || ( uc_reg_value == 0xa6 ) )
		{
			yl_touch_debug(LOG_DEBUG, "\n\nenter OF new firmware update!\n\n");

			pbt_buf = CTPM_FW_OF_NEW;
	  		//call the upgrade function
	   		i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW_OF_NEW));
			off_then_on_tw();
		}
/*		else if( uc_reg_value == 0xf )
		{
			yl_touch_debug(LOG_DEBUG, "\n\nenter OF new firmware update!\n\n");

			pbt_buf = CTPM_FW_OF_NEW;
	  		//call the upgrade function
	   		i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW_OF_NEW));
		}*/
		else 
		{
			i_ret = 0;
		}
	}


    if (i_ret != 0)
    {
		//error handling ...
        //TBD
    }

    return i_ret;
}

unsigned char fts_ctpm_get_upg_ver(void)
{
    unsigned int ui_sz;

	unsigned char chip_vendor_id;//added by zhuhui for two firmware update
    int ret = -1;//added by zhuhui for firmware update	

    ret=ft5x0x_read_reg(0xA8, &chip_vendor_id);//added by zhuhui for two firmware update
    yl_touch_debug(LOG_DEBUG, "[%s] chip_vendor_id:0x%x | ret:%d\n", __func__,chip_vendor_id,ret);
    if(ret<0) 
    {
	    yl_touch_debug(LOG_DEBUG, "%s:######\n", __func__);
	    return -1;
    }

	if( chip_vendor_id == 0x5A )           //altered by zhuhui for two firmware update
	{
		yl_touch_debug(LOG_DEBUG, "%s:######%d,read XL FW VERSION\n", __func__,chip_vendor_id);

    	ui_sz = sizeof(CTPM_FW_XL);
    	if (ui_sz > 2)
    	{
        	return CTPM_FW_XL[ui_sz - 2];
    	}
    	else
    	{
        	//TBD, error handling?
        	return 0xff; //default value
    	}
	}
	else if( chip_vendor_id == 0x51 )
	{
		yl_touch_debug(LOG_DEBUG, "%s:######%d,read OF FW VERSION\n", __func__,chip_vendor_id);

    	ui_sz = sizeof(CTPM_FW_OF_NEW);
    	if (ui_sz > 2)
    	{
        	return CTPM_FW_OF_NEW[ui_sz - 2];
    	}
    	else
    	{
        	//TBD, error handling?
        	return 0xff; //default value
    	}		
	}
	else
		return 0xff;
}

#endif

static int yl_report_key(unsigned int key_state,unsigned int key_type)
{
	struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);

    if(key_state == 0x00)
    {
	     input_report_key(data->input_dev,key_type, 1);
	     input_sync(data->input_dev);
		 yl_touch_debug(LOG_WARNING, "key_type: %d,value: %d\n",key_type,1);
    }
    else if(key_state == 0x01)
    {
	     input_report_key(data->input_dev,key_type, 0);
	     input_sync(data->input_dev);
		 yl_touch_debug(LOG_WARNING, "key_type: %d,value: %d\n",key_type,0);
    }

    return 0;
}


/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void ft5x0x_ts_release(void)
{
	struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
#ifdef CONFIG_FT5X0X_MULTITOUCH	
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
#else
	input_report_abs(data->input_dev, ABS_PRESSURE, 0);
	input_report_key(data->input_dev, BTN_TOUCH, 0);
#endif
	input_sync(data->input_dev);

	yl_touch_debug(LOG_WARNING, "==%s==\n",__func__);
}

static int ft5x0x_read_data(void)
{
	static int is_up = 1;

	struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
//	u8 buf[14] = {0};
	u8 buf[32] = {0};
	int ret = -1;
//        int i=0;

	int eventstate = 1;
	static unsigned int key_type = 0;

#ifdef CONFIG_FT5X0X_MULTITOUCH
//	ret = ft5x0x_i2c_rxdata(buf, 13);
	ret = ft5x0x_i2c_rxdata(buf, 31);
#else
    ret = ft5x0x_i2c_rxdata(buf, 7);
#endif
    //for(;i<32;++i)
    //yl_touch_debug(LOG_DEBUG, "[%d]=0x%x",i,buf[i]);

    if (ret < 0) {
		yl_touch_debug(LOG_DEBUG, "%s read_data i2c_rxdata failed: %d\n", __func__, ret);
		return ret;
	}

	memset(event, 0, sizeof(struct ts_event));
//	event->touch_point = buf[2] & 0x03;// 0000 0011
	event->touch_point = buf[2] & 0x07;// 000 0111


/*		yl_touch_debug(KERN_ERR "\n%s,%d\n",__func__,buf[1]);                                 
        if(buf[1])
        {
         //yl_touch_debug(KERN_ERR "KEY [1]=%d",buf[1]);
         input_report_key(data->input_dev, touch_key_ft[buf[1] - 1], 1);
         input_report_key(data->input_dev, touch_key_ft[buf[1] - 1], 0);
         }*/                                             //deleted by zhuzhuhui  on 2011.9.5


	eventstate = buf[3];
	eventstate = (eventstate >> 6);

	yl_touch_debug(LOG_DEBUG, "\neventstate:%d,point:%d\n",eventstate,event->touch_point);

    if (event->touch_point == 0) {              //

		if( key_type && ( is_up == 0 ))
		{
			yl_report_key(eventstate,key_type);
            key_type = 0;
			is_up = 1;
		}

        ft5x0x_ts_release();
        return 1; 
    }

#ifdef CONFIG_FT5X0X_MULTITOUCH
    switch (event->touch_point) {
		case 5:
			event->x5 = (s16)(buf[0x1b] & 0x0F)<<8 | (s16)buf[0x1c];
			event->y5 = (s16)(buf[0x1d] & 0x0F)<<8 | (s16)buf[0x1e];
		case 4:
			event->x4 = (s16)(buf[0x15] & 0x0F)<<8 | (s16)buf[0x16];
			event->y4 = (s16)(buf[0x17] & 0x0F)<<8 | (s16)buf[0x18];
		case 3:
			event->x3 = (s16)(buf[0x0f] & 0x0F)<<8 | (s16)buf[0x10];
			event->y3 = (s16)(buf[0x11] & 0x0F)<<8 | (s16)buf[0x12];
		case 2:
			event->x2 = (s16)(buf[9] & 0x0F)<<8 | (s16)buf[10];
			event->y2 = (s16)(buf[11] & 0x0F)<<8 | (s16)buf[12];
		case 1:
			event->x1 = (s16)(buf[3] & 0x0F)<<8 | (s16)buf[4];
			event->y1 = (s16)(buf[5] & 0x0F)<<8 | (s16)buf[6];
            break;
		default:
		    return -1;
	}
#else
    if (event->touch_point == 1) {
    	event->x1 = (s16)(buf[3] & 0x0F)<<8 | (s16)buf[4];
		event->y1 = (s16)(buf[5] & 0x0F)<<8 | (s16)buf[6];
    }
#endif
    event->pressure = 200;
	
	if ((event->touch_point == 1) && ((event->y1) > 800)) 
	{
		yl_touch_debug(LOG_DEBUG, "zhuhui added 10.01:x->%d,y->%d\n",event->x1,event->y1);
		
		if( is_up == 1 )
		{
			if((event->x1) < 120)
			{	
//				yl_touch_debug(KERN_ERR "+++++++++key menu+++++++++\n");
//				input_report_key(data->input_dev, touch_key_ft[0], 1);

//					key_map = KEY_BACK;
//				      yl_report_key(data,KEY_BACK);

				key_type = touch_key_ft[0];
				yl_report_key(eventstate,touch_key_ft[0]);
			}
			else if((event->x1) > 360)
			{
//				yl_touch_debug(KERN_ERR "+++++++++key search+++++++++\n");
//				input_report_key(data->input_dev, touch_key_ft[3], 1);

				key_type = touch_key_ft[3];
				yl_report_key(eventstate,touch_key_ft[3]);
			}
			else if(( (event->x1) >= 120 ) && ( ( event->x1 ) <= 240 ))
			{
//				yl_touch_debug(KERN_ERR "+++++++++key home+++++++++\n");
//				input_report_key(data->input_dev, touch_key_ft[1], 1);

				key_type = touch_key_ft[1];
				yl_report_key(eventstate,touch_key_ft[1]);
			}
			else
			{
//				yl_touch_debug(KERN_ERR "+++++++++key back+++++++++\n");
//				input_report_key(data->input_dev, touch_key_ft[2], 1);

				key_type = touch_key_ft[2];
				yl_report_key(eventstate,touch_key_ft[2]);
			}

			is_up = 0;
		}
			
//		input_sync(data->input_dev);

        return 1;
    }

	//dev_dbg(&this_client->dev, "%s: 1:%d %d 2:%d %d \n", __func__,
	//	event->x1, event->y1, event->x2, event->y2);
	//printk("%d (%d, %d), (%d, %d)\n", event->touch_point, event->x1, event->y1, event->x2, event->y2);

    return 0;
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void ft5x0x_report_value(void)
{
	struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
//	u8 uVersion;

//		printk("==ft5x0x_report_value =\n");
#ifdef CONFIG_FT5X0X_MULTITOUCH
	switch(event->touch_point) {
		case 5:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x5);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y5);
			input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
			input_mt_sync(data->input_dev);
			yl_touch_debug(LOG_WARNING, "===x5 = %d,y5 = %d ====\n",event->x2,event->y2);
		case 4:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x4);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y4);
			input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
			input_mt_sync(data->input_dev);
			yl_touch_debug(LOG_WARNING, "===x4 = %d,y4 = %d ====\n",event->x2,event->y2);
		case 3:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x3);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y3);
			input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
			input_mt_sync(data->input_dev);
			yl_touch_debug(LOG_WARNING, "===x3 = %d,y3 = %d ====\n",event->x2,event->y2);
		case 2:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x2);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y2);
			input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
			input_mt_sync(data->input_dev);
			yl_touch_debug(LOG_WARNING, "===x2 = %d,y2 = %d ====\n",event->x2,event->y2);
		case 1:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x1);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y1);
			input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
			input_mt_sync(data->input_dev);
			yl_touch_debug(LOG_WARNING, "===x1 = %d,y1 = %d ====\n",event->x1,event->y1);
		default:
			//printk("==touch_point default =\n");
			break;
	}
#else	/* CONFIG_FT5X0X_MULTITOUCH*/
	if (event->touch_point == 1) {
			input_report_abs(data->input_dev, ABS_X, event->x1);
			input_report_abs(data->input_dev, ABS_Y, event->y1);
			input_report_abs(data->input_dev, ABS_PRESSURE, event->pressure);
	}
	input_report_key(data->input_dev, BTN_TOUCH, 1);
#endif	/* CONFIG_FT5X0X_MULTITOUCH*/
	input_sync(data->input_dev);

	//dev_dbg(&this_client->dev, "%s: 1:%d %d 2:%d %d \n", __func__,
	//	event->x1, event->y1, event->x2, event->y2);
}	/*end ft5x0x_report_value*/
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void ft5x0x_ts_pen_irq_work(struct work_struct *work)
{
	int ret = -1;

	static u32 count = 0;

	if( ( count % 200 ) == 0 )
		printk(KERN_ERR "ft5x0x tw genates %d interrupts!\n",count);

	count++;

//	printk("==work 1=\n");
	ret = ft5x0x_read_data();	
	if (ret == 0) {	
		ft5x0x_report_value();
	}
//	else printk("data package read error\n");
//	printk("==work 2=\n");
//    	msleep(1);
    enable_irq(this_client->irq);
	//enable_irq(IRQ_EINT(6));
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
	struct ft5x0x_ts_data *ft5x0x_ts = dev_id;
        disable_irq_nosync(this_client->irq);
   	//disable_irq(this_client->irq);		
//	disable_irq(IRQ_EINT(6));
	//printk("==int=-guo\n");       

//	printk("ggggggggggggggggggggg%skkkkkkkkkkkkkkk\n",__func__);

	if (!work_pending(&ft5x0x_ts->pen_event_work)) {//for temperature
		queue_work(ft5x0x_ts->ts_workqueue, &ft5x0x_ts->pen_event_work);
	}

	return IRQ_HANDLED;
}
#ifdef CONFIG_HAS_EARLYSUSPEND
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void ft5x0x_ts_suspend(struct early_suspend *handler)
{
	struct irq_desc *desc = irq_to_desc(this_client->irq);//added by zhuhui 11.16

	struct ft5x0x_ts_data *ts;
	ts =  container_of(handler, struct ft5x0x_ts_data, early_suspend);
	yl_touch_debug(LOG_WARNING, "==ft5x0x_ts_suspend=\n");
	//disable_irq(this_client->irq);

	tw_is_active_status = 0;

	disable_irq_nosync(this_client->irq);
 
	cancel_work_sync(&ts->pen_event_work);
	flush_workqueue(ts->ts_workqueue);

	if(unlikely(desc->depth == 0))
    {
    	yl_touch_debug(LOG_WARNING, "<ft5x0x_suspend> desc->depth == 0\n");
        disable_irq_nosync(this_client->irq); 
    }


	// ==set mode ==, 
//    ft5x0x_write_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
//	gpio_direction_output(OMAP_FT5X06_IRQ_GPIO, 1); 	
//	gpio_direction_output(OMAP_FT5X06_RST_GPIO, 1);

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
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);
}

void ft5x0x_poweron(void)
{
	///////////////////////////POWER ON/////////////////////////////
	omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_PWR_EN_GPIO, "ts_pwr_en");
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 1);
	mdelay(5);

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

	omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_RST_GPIO, "ts_rst");
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0); 
    mdelay(8);
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 1); 
}

/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void ft5x0x_ts_resume(struct early_suspend *handler)
{
     
#if 1

/*	omap_mux_init_signal("i2c2_scl", OMAP_PIN_INPUT);	
	omap_mux_init_signal("i2c2_sda", OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_I2C_SCL_GPIO, "i2c2_scl");
	gpio_request(OMAP_FT5X06_I2C_SDA_GPIO, "i2c2_sda");
    omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
    omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);

    omap_mux_init_gpio(OMAP_FT5X06_IRQ_GPIO, OMAP_PIN_INPUT_PULLUP);

    gpio_request(OMAP_FT5X06_I2C_SCL_GPIO, "i2c2_scl");
	gpio_request(OMAP_FT5X06_I2C_SDA_GPIO, "i2c2_sda");
    gpio_request(OMAP_FT5X06_IRQ_GPIO, "touch");
    //omap_mux_init_gpio(129, OMAP_PIN_INPUT_PULLUP|OMAP_PIN_OFF_OUTPUT_LOW);
    gpio_direction_input(OMAP_FT5X06_IRQ_GPIO);   
 

	omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_PWR_EN_GPIO, "ts_pwr_en");
    gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);    
    mdelay(5);	
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 1); 
    mdelay(5);*/

	struct irq_desc *desc = irq_to_desc(this_client->irq);//added by zhuhui 11.16

/*	omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_PWR_EN_GPIO, "ts_pwr_en");
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 1);
	mdelay(5);

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

	omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_RST_GPIO, "ts_rst");
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0); 
    mdelay(8);
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 1);*/   //4.11 altered  
//	msleep(300);
	msleep(230);
#endif
	yl_touch_debug(LOG_WARNING, "==ft5x0x_ts_resume=\n");

	if( unlikely( desc->depth > 1 ) )
	{
		enable_irq(this_client->irq);
		mdelay(10);
		if( unlikely( desc->depth ) )
			enable_irq(this_client->irq);
		yl_touch_debug(LOG_WARNING, "%s:%d,desc->depth=%d\n",__func__,__LINE__,desc->depth);
	}
	else
		enable_irq(this_client->irq);

	tw_is_active_status = 1;

	// wake the mode
//	__gpio_as_output(GPIO_FT5X0X_WAKE);		
//	__gpio_clear_pin(GPIO_FT5X0X_WAKE);		//set wake = 0,base on system
//	 msleep(100);
//	__gpio_set_pin(GPIO_FT5X0X_WAKE);			//set wake = 1,base on system
//	msleep(100);
//	enable_irq(this_client->irq);
}
#endif  //CONFIG_HAS_EARLYSUSPEND

/*static ssize_t ft5x0x_tpautoclb_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	fts_ctpm_auto_clb();  //start auto CLB
	return 1;
}

static ssize_t ft5x0x_tpautoclb_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	fts_ctpm_auto_clb();  //start auto CLB
	// place holder for future use 
	return -EPERM;
}
// sysfs 
static DEVICE_ATTR(ftstpautoclb, S_IRUGO|S_IWUSR, ft5x0x_tpautoclb_show, ft5x0x_tpautoclb_store);
static struct attribute *ft5x0x_attributes[] = {
	&dev_attr_ftstpautoclb.attr,
	NULL
};

static struct attribute_group ft5x0x_attribute_group = {
	.attrs = ft5x0x_attributes
};*/

/*******************************************************
   ********  1--active  0--not active***********
*******************************************************/
int ft5x06_active(void)				   
{
   printk("enter %s\n",__FUNCTION__);
   return tw_is_active_status;
}

/*******************************************************
***********check firmware if need update***********
*******************************************************/
int ft5x06_firmware_need_update(void)
{
	
   	unsigned char uc_fw_ver;
   	unsigned char uc_if_ver;

	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);
	
	uc_fw_ver = ft5x0x_read_fw_ver();
	yl_touch_debug(LOG_DEBUG, "[FTS]:IC:firmware version = 0x%x\n",uc_fw_ver);
	uc_if_ver = fts_ctpm_get_upg_ver();
	yl_touch_debug(LOG_DEBUG, "[FTS]:I FILE:firmware version = 0x%x\n",uc_if_ver);
	if(uc_if_ver == 0xff)
	{
		yl_touch_debug(LOG_DEBUG, "GET IFILE VERSION ERROR!\n");
		return 0;
	}
   	if( uc_fw_ver < uc_if_ver)
   		return 1;
   	else
		return 0;
 }


u8 clear_tw_cal_flag(void);

/*******************************************************
*********************do firmware update ***************
*******************************************************/
int ft5x06_firmware_do_update(void)	  
{
	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

	clear_tw_cal_flag();

   	if(fts_ctpm_fw_upgrade_with_i_file() == 0)
   	   return 0;
   	else
   	   return -1;
}


extern int yl_params_kernel_read(char *buf,size_t count);
extern int yl_params_kernel_write(const char *buf,size_t count);
u8 tw_is_calibrated(void)
{
	char deviceinfo[512] = "DEVICE";

	yl_params_kernel_read(deviceinfo, 512);

	printk(KERN_ERR "%d,%c\n",460,deviceinfo[460]);

	if( deviceinfo[460] == '1')
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

u8 clear_tw_cal_flag(void)
{
	char deviceinfo[512] = "DEVICE";

	printk(KERN_ERR "%s\n",__func__);

	yl_params_kernel_read(deviceinfo, 512);
	deviceinfo[460] = '0';
	yl_params_kernel_write(deviceinfo,512);

	return 0;
}

u8 write_tw_cal_flag(void)
{
	char deviceinfo[512] = "DEVICE";

	printk(KERN_ERR "%s\n",__func__);

	yl_params_kernel_read(deviceinfo, 512);
	deviceinfo[460] = '1';
	yl_params_kernel_write(deviceinfo,512);

	return 0;
}


/*******************************************************
*******************check if need calibrate***********
*******************************************************/

int ft5x06_need_calibrate(void)				       
{
   yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);
   return tw_is_calibrated();
}

/*******************************************************
 ******************system write "calibrate"************
*******************************************************/

int ft5x06_calibrate(void)				      
{
   	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);
   	if(!fts_ctpm_auto_clb())
	{
		write_tw_cal_flag();
   		return 0;
	}
	else
		return -1;
}

/*******************************************************
 ******************get firmware version **************
*******************************************************/
int ft5x06_get_firmware_version(char * version )
{
    unsigned int uc_fw_version;

	int ret = -1;
	unsigned char chip_vendor_id;
	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

    ret = ft5x0x_read_reg(0xA8, &chip_vendor_id);
	yl_touch_debug(LOG_DEBUG, "[%s] chip_vendor_id:0x%x | ret:%d\n", __func__,chip_vendor_id,ret);
    if(ret<0) 
    {
	    yl_touch_debug(LOG_DEBUG, "%s:######\n", __func__);
	    return -1;
    }
	
	uc_fw_version = ft5x0x_read_fw_ver();
	
	if(chip_vendor_id == 0x51){
		return sprintf(version, "%s(0x%x):%d","Oflim:FT5206:V",uc_fw_version,0);
		
	}
	if(chip_vendor_id == 0x5A){
		return sprintf(version, "%s(0x%x):%d","Xinli:FT5206:V", uc_fw_version,0);
	}
	return -1;
 }

/*******************************************************
  ******************system write "reset"***************
*******************************************************/
int ft5x06_reset_touchscreen(void)	
{
    yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);

	disable_irq_nosync(this_client->irq);

	/*write 0xaa to register 0xfc*/
	ft5x0x_write_reg(0xfc,0xaa);
	delay_qt_ms(50);
	/*write 0x55 to register 0xfc*/
	ft5x0x_write_reg(0xfc,0x55);
	yl_touch_debug(LOG_DEBUG, "[FTS]: Reset CTPM ok!\n");

	enable_irq(this_client->irq);
	return 1;
}

/*******************************************************
  ******************"handwrite" "normal" *************
*******************************************************/
touch_mode_type ft5x06_get_mode(void)			    
{
   yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);
   return MODE_NORMAL;
}

int ft5x06_set_mode(touch_mode_type work_mode)			 
{
	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);
    return 1;
}

/*******************************************************
  ****************get "oreitation:X" ************
*******************************************************/
touch_oreitation_type ft5x06_get_oreitation(void)				      
{
   yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);
   return 1;
}


int ft5x06_set_oreitation(touch_oreitation_type oreitate)				    
{
   yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);
   return 1;
}


/*******************************************************
  ***************tw debug on or off *************
*******************************************************/
int ft5x06_debug(int val)				
{
	yl_touch_debug(LOG_DEBUG, "enter %s\n",__FUNCTION__);
//   	tw_debug=val;
   	return 1;
}


touchscreen_ops_tpye synaptics_ops=
{
	.touch_id					= 0,		
	.touch_type					= 1,
	.active						= ft5x06_active,
	.firmware_need_update		= ft5x06_firmware_need_update,
	.firmware_do_update			= ft5x06_firmware_do_update,
	.need_calibrate				= ft5x06_need_calibrate,
	.calibrate					= ft5x06_calibrate,
	.get_firmware_version		= ft5x06_get_firmware_version,
	.reset_touchscreen			= ft5x06_reset_touchscreen,
	.get_mode					= ft5x06_get_mode,
	.set_mode					= ft5x06_set_mode,
	.get_oreitation				= ft5x06_get_oreitation,
	.set_oreitation				= ft5x06_set_oreitation,
	.read_regs					= NULL,
	.write_regs					= NULL,
	.debug						= ft5x06_debug,
};


/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/

static
 int 
ft5x0x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	struct input_dev *input_dev;
	int err = 0;

//	unsigned char uc_reg_value; //修改为了兼容不同的固件
	

        /*
        if(ft5x0x_read_fw_ver()<0)
        {
           printk(KERN_ERR "==ft5x0x_ts_probe ERROR \n");
           return -1;
        }
        */
	yl_touch_debug(LOG_DEBUG, "\n\n==ft5x0x_ts_probe=\n\n");
	if( ft5x0x_tw_is_or_not_power_on == 0 )
		return -1;
	//change_cpu_freq_in_touch = 5;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}
    this_client = client;
        
#if 1

	///////////////////////////POWER OFF/////////////////////////////
	// Set the ts_rst_gpio 
	omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_RST_GPIO, "ts_rst");
	gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0);

	// Set the ts_irq_gpio                 
	if( !is_p0 )
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P1, OMAP_PIN_INPUT_PULLDOWN);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P1, "ts_irq");
		gpio_direction_input(OMAP_FT5X06_IRQ_GPIO_P1);
	}
	else
	{
		omap_mux_init_signal(TS_IRQ_GPIO_NAME_P0, OMAP_PIN_INPUT_PULLDOWN);
		gpio_request(OMAP_FT5X06_IRQ_GPIO_P0, "ts_irq");
		gpio_direction_input(OMAP_FT5X06_IRQ_GPIO_P0);
	}


	omap_mux_init_signal(TS_FT5X06_I2C_SCL_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_I2C_SCL_GPIO, "i2c2_scl");
	gpio_direction_output(OMAP_FT5X06_I2C_SCL_GPIO, 0);

	omap_mux_init_signal(TS_FT5X06_I2C_SDA_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_I2C_SDA_GPIO, "i2c2_sda");
	gpio_direction_output(OMAP_FT5X06_I2C_SDA_GPIO, 0);


	omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_PWR_EN_GPIO, "ts_pwr_en");
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);
	mdelay(5);
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 1);
	mdelay(5);

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

	omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);
	gpio_request(OMAP_FT5X06_RST_GPIO, "ts_rst");
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0); 
    mdelay(8);
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 1);
	msleep(300);

/*	omap_mux_init_signal("i2c2_scl", OMAP_PIN_INPUT);	
	omap_mux_init_signal("i2c2_sda", OMAP_PIN_INPUT);
    gpio_request(OMAP_FT5X06_I2C_SCL_GPIO, "i2c2_scl");
	gpio_request(OMAP_FT5X06_I2C_SDA_GPIO, "i2c2_sda");

	omap_mux_init_signal(TS_PWR_EN_GPIO_NAME, OMAP_PIN_INPUT);
    omap_mux_init_signal(TS_RST_GPIO_NAME, OMAP_PIN_INPUT);

    omap_mux_init_gpio(OMAP_FT5X06_IRQ_GPIO, OMAP_PIN_INPUT_PULLUP);
      
    gpio_request(OMAP_FT5X06_IRQ_GPIO, "touch");
    //omap_mux_init_gpio(129, OMAP_PIN_INPUT_PULLUP|OMAP_PIN_OFF_OUTPUT_LOW);
    gpio_direction_input(OMAP_FT5X06_IRQ_GPIO);   

    gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 0);    
    mdelay(5);	
	gpio_direction_output(OMAP_FT5X06_PWR_EN_GPIO, 1); 
    mdelay(5);
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 0); 
    mdelay(8);
    gpio_direction_output(OMAP_FT5X06_RST_GPIO, 1);  */

#endif

    this_client->addr=0xb8;
    //get some register information

	yl_touch_debug(LOG_DEBUG, "==kzalloc=\n");
	ft5x0x_ts = kzalloc(sizeof(*ft5x0x_ts), GFP_KERNEL);
	if (!ft5x0x_ts)	{
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}
	
	i2c_set_clientdata(client, ft5x0x_ts);


	mutex_init(&ft5x0x_ts->device_mode_mutex);
	INIT_WORK(&ft5x0x_ts->pen_event_work, ft5x0x_ts_pen_irq_work);

	ft5x0x_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));  //for temperature
//	ft5x0x_ts->ts_workqueue = create_workqueue(dev_name(&client->dev));
	if (!ft5x0x_ts->ts_workqueue) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}

//	pdata = client->dev.platform_data;
//	if (pdata == NULL) {
//		dev_err(&client->dev, "%s: platform data is null\n", __func__);
//		goto exit_platform_data_null;
//	}
	
//	printk("==request_irq=\n");
    err = request_irq(client->irq, ft5x0x_ts_interrupt, IRQF_TRIGGER_LOW,//IRQF_TRIGGER_FALLING,//IRQF_TRIGGER_FALLING, 
                        "ft5x0x_ts",  //"synaptics-rmi-ts",  modified by guoguangyi
                            ft5x0x_ts);
//err = request_irq(client->irq, ft5x0x_ts_interrupt, IRQF_DISABLED, "ft5x0x_ts", ft5x0x_ts);
//	err = request_irq(IRQ_EINT(6), ft5x0x_ts_interrupt, IRQF_TRIGGER_FALLING, "ft5x0x_ts", ft5x0x_ts);
	if (err < 0) {
		dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}

//	__gpio_as_irq_fall_edge(pdata->intr);		//
	disable_irq_nosync(this_client->irq);
//	disable_irq(IRQ_EINT(6));

//	printk("==input_allocate_device=\n");
	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}
	
	ft5x0x_ts->input_dev = input_dev;

#ifdef CONFIG_FT5X0X_MULTITOUCH
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);

	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);
#else
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_PRESSURE, input_dev->absbit);
	set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev,ABS_PRESSURE, 0, PRESS_MAX, 0 , 0);
#endif

        //set_bit(EV_SYN, input_dev->evbit);
	//set_bit(EV_KEY, input_dev->evbit);
 
	input_set_capability(input_dev, EV_KEY, touch_key_ft[0]);	
    input_set_capability(input_dev, EV_KEY, touch_key_ft[1]);
    input_set_capability(input_dev, EV_KEY, touch_key_ft[2]);
	input_set_capability(input_dev, EV_KEY, touch_key_ft[3]);//added by zhuhui 2011.10.01
    //input_set_capability(input_dev, EV_KEY, touch_key[3]);

	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);

	input_dev->name		= "ft5x0x_ts";// add by guoguangyi FT5X0X_NAME;		//dev_name(&client->dev)
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
		"ft5x0x_ts_probe: failed to register input device: %s\n",
		dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}

	touchscreen_set_ops(&synaptics_ops);

#ifdef CONFIG_HAS_EARLYSUSPEND
	yl_touch_debug(LOG_DEBUG, "==register_early_suspend =\n");
	ft5x0x_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ft5x0x_ts->early_suspend.suspend = ft5x0x_ts_suspend;
	ft5x0x_ts->early_suspend.resume	= ft5x0x_ts_resume;
	register_early_suspend(&ft5x0x_ts->early_suspend);
#endif


//   if(uc_reg_value!=10)
//	fts_ctpm_fw_upgrade_with_i_file();


    
//wake the CTPM
//	__gpio_as_output(GPIO_FT5X0X_WAKE);		
//	__gpio_clear_pin(GPIO_FT5X0X_WAKE);		//set wake = 0,base on system
//	 msleep(100);
//	__gpio_set_pin(GPIO_FT5X0X_WAKE);			//set wake = 1,base on system
//	msleep(100);
//	ft5x0x_set_reg(0x88, 0x05); //5, 6,7,8
//	ft5x0x_set_reg(0x80, 30);
//	msleep(50);
   	enable_irq(this_client->irq);
	
   //create sysfs
/*   	err = sysfs_create_group(&client->dev.kobj, &ft5x0x_attribute_group);
   	if (0 != err)
   	{
		dev_err(&client->dev, "%s() - ERROR: sysfs_create_group() failed: %d\n", __FUNCTION__, err);
		sysfs_remove_group(&client->dev.kobj, &ft5x0x_attribute_group);
   	}
   	else
    {
        yl_touch_debug(LOG_DEBUG, "ft5x0x:%s() - sysfs_create_group() succeeded.\n", __FUNCTION__);
    }*/

	yl_touch_debug(LOG_DEBUG, "==probe over =\n");

	tw_is_active_status = 1;
    return 0;

exit_input_register_device_failed:
	input_free_device(input_dev);
exit_input_dev_alloc_failed:
	free_irq(client->irq, ft5x0x_ts);
//	free_irq(IRQ_EINT(6), ft5x0x_ts);
exit_irq_request_failed:
//exit_platform_data_null:
	cancel_work_sync(&ft5x0x_ts->pen_event_work);
	destroy_workqueue(ft5x0x_ts->ts_workqueue);
exit_create_singlethread:
	yl_touch_debug(LOG_DEBUG, "==singlethread error =\n");
	i2c_set_clientdata(client, NULL);
	kfree(ft5x0x_ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
{
	
	struct ft5x0x_ts_data *ft5x0x_ts = i2c_get_clientdata(client);
    yl_touch_debug(LOG_DEBUG, "==ft5x0x_ts_remove=\n");

	unregister_early_suspend(&ft5x0x_ts->early_suspend);
	mutex_destroy(&ft5x0x_ts->device_mode_mutex);
	free_irq(client->irq, ft5x0x_ts);
//	free_irq(IRQ_EINT(6), ft5x0x_ts);
	input_unregister_device(ft5x0x_ts->input_dev);
	kfree(ft5x0x_ts);
	cancel_work_sync(&ft5x0x_ts->pen_event_work);
	destroy_workqueue(ft5x0x_ts->ts_workqueue);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{ FT5X0X_NAME, 0 },{ }
};


MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver ft5x0x_ts_driver = {
	.probe		= ft5x0x_ts_probe,
	.remove		= __devexit_p(ft5x0x_ts_remove),
	.id_table	= ft5x0x_ts_id,
	.driver	= {
		.name	= FT5X0X_NAME,
		.owner	= THIS_MODULE,
	},
};

/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int __init ft5x0x_ts_init(void)
{

	int ret;
	yl_touch_debug(LOG_DEBUG, "\n==ft5x0x_ts_init==\n\n");
	ret = i2c_add_driver(&ft5x0x_ts_driver);
	yl_touch_debug(LOG_DEBUG, "ret=%d\n",ret);
	return ret;


//	return i2c_add_driver(&ft5x0x_ts_driver);
}

/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void __exit ft5x0x_ts_exit(void)
{

	yl_touch_debug(LOG_DEBUG, "==ft5x0x_ts_exit==\n");
	i2c_del_driver(&ft5x0x_ts_driver);

}

//module_init(ft5x0x_ts_init);//altered by zhuhui
late_initcall(ft5x0x_ts_init);//added by zhuhui
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<wenfs@Focaltech-systems.com>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");

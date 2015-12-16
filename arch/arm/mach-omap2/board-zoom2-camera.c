/*
 * linux/arch/arm/mach-omap2/board-zoom2-camera.c
 *
 * Copyright (C) 2007 Texas Instruments
 *
 * Modified from mach-omap2/board-generic.c
 *
 * Initial code: Syed Mohammed Khasim
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include "mux.h"

#if defined(CONFIG_TWL4030_CORE) && defined(CONFIG_VIDEO_OMAP3)

#include <linux/i2c/twl.h>

#include <asm/io.h>

#include <mach/gpio.h>
#include <plat/omap-pm.h>

static int cam_inited;
#include <media/v4l2-int-device.h>
#include <../drivers/media/video/omap34xxcam.h>
#include <../drivers/media/video/isp/ispreg.h>

#include "board-cp5860e-camera.h"

#define DEBUG_BASE		0x08000000

#define REG_SDP3430_FPGA_GPIO_2 (0x50)
#define FPGA_SPR_GPIO1_3v3	(0x1 << 14)
#define FPGA_GPIO6_DIR_CTRL	(0x1 << 6)

#define VAUX_2_8_V		0x09
#define VAUX_1_8_V		0x05
#define VAUX_DEV_GRP_P1		0x20
#define VAUX_DEV_GRP_NONE	0x00

#define CAMZOOM2_USE_XCLKB  	1

#define ISP_IMX046_MCLK		216000000

/* Sensor specific GPIO signals */
#define IMX046_RESET_GPIO  	98
#define IMX046_STANDBY_GPIO	58

//#define BOARD_ZOOM2_CAMERA 1

#ifdef BOARD_ZOOM2_CAMERA
#define DPRINTK_OV(format, ...)				\
  printk(KERN_ERR "zoom2_camera:%s: " format " ",__func__, ## __VA_ARGS__)
#else
#define DPRINTK_OV(format, ...)
#endif         

extern int flashlight_control(int mode, u8 intensity);//flashlight_control

#if defined(CONFIG_VIDEO_IMX046) || defined(CONFIG_VIDEO_IMX046_MODULE)
#include <media/imx046.h>
#include <../drivers/media/video/isp/ispcsi2.h>
#define IMX046_CSI2_CLOCK_POLARITY	0	/* +/- pin order */
#define IMX046_CSI2_DATA0_POLARITY	0	/* +/- pin order */
#define IMX046_CSI2_DATA1_POLARITY	0	/* +/- pin order */
#define IMX046_CSI2_CLOCK_LANE		1	 /* Clock lane position: 1 */
#define IMX046_CSI2_DATA0_LANE		2	 /* Data0 lane position: 2 */
#define IMX046_CSI2_DATA1_LANE		3	 /* Data1 lane position: 3 */
#define IMX046_CSI2_PHY_THS_TERM	2
#define IMX046_CSI2_PHY_THS_SETTLE	23
#define IMX046_CSI2_PHY_TCLK_TERM	0
#define IMX046_CSI2_PHY_TCLK_MISS	1
#define IMX046_CSI2_PHY_TCLK_SETTLE	14
#define IMX046_BIGGEST_FRAME_BYTE_SIZE	PAGE_ALIGN(ALIGN(3280, 0x20) * 2464 * 2)
#endif

#ifdef CONFIG_VIDEO_LV8093
#include <media/lv8093.h>
#define LV8093_PS_GPIO			7
/* GPIO7 is connected to lens PS pin through inverter */
#define LV8093_PWR_OFF			1
#define LV8093_PWR_ON			(!LV8093_PWR_OFF)
#endif


#ifdef CONFIG_VIDEO_LV8093
static int lv8093_lens_power_set(enum v4l2_power power)
{
	static enum v4l2_power previous_pwr = V4L2_POWER_OFF;

	switch (power) {
	case V4L2_POWER_ON:
		printk(KERN_DEBUG "lv8093_lens_power_set(ON)\n");
		if (previous_pwr == V4L2_POWER_OFF) {
			if (gpio_request(LV8093_PS_GPIO, "lv8093_ps") != 0) {
				printk(KERN_WARNING "Could not request GPIO %d"
					" for LV8093\n", LV8093_PS_GPIO);
				return -EIO;
			}

			gpio_set_value(LV8093_PS_GPIO, LV8093_PWR_OFF);
			gpio_direction_output(LV8093_PS_GPIO, true);
		}
		gpio_set_value(LV8093_PS_GPIO, LV8093_PWR_ON);
		break;
	case V4L2_POWER_OFF:
		printk(KERN_DEBUG "lv8093_lens_power_set(OFF)\n");
		gpio_free(LV8093_PS_GPIO);
		break;
	case V4L2_POWER_STANDBY:
		printk(KERN_DEBUG "lv8093_lens_power_set(STANDBY)\n");
		gpio_set_value(LV8093_PS_GPIO, LV8093_PWR_OFF);
		break;
	}
	previous_pwr = power;
	return 0;
}

static int lv8093_lens_set_prv_data(void *priv)
{
	struct omap34xxcam_hw_config *hwc = priv;

	hwc->dev_index = 2;
	hwc->dev_minor = 5;
	hwc->dev_type = OMAP34XXCAM_SLAVE_LENS;
	return 0;
}

struct lv8093_platform_data zoom2_lv8093_platform_data = {
	.power_set      = lv8093_lens_power_set,
	.priv_data_set  = lv8093_lens_set_prv_data,
};
#endif

#if defined(CONFIG_VIDEO_IMX046) || defined(CONFIG_VIDEO_IMX046_MODULE)

static struct omap34xxcam_sensor_config imx046_hwc = {
	.sensor_isp  = 0,
	.capture_mem = IMX046_BIGGEST_FRAME_BYTE_SIZE * 4,
	.ival_default	= { 1, 10 },
	.isp_if = ISP_CSIA,
};

static int imx046_sensor_set_prv_data(struct v4l2_int_device *s, void *priv)
{
	struct omap34xxcam_hw_config *hwc = priv;

	hwc->u.sensor		= imx046_hwc;
	hwc->dev_index		= 2;
	hwc->dev_minor		= 5;
	hwc->dev_type		= OMAP34XXCAM_SLAVE_SENSOR;

	return 0;
}

static struct isp_interface_config imx046_if_config = {
	.ccdc_par_ser 		= ISP_CSIA,
	.dataline_shift 	= 0x0,
	.hsvs_syncdetect 	= ISPCTRL_SYNC_DETECT_VSRISE,
	.strobe 		= 0x0,
	.prestrobe 		= 0x0,
	.shutter 		= 0x0,
	.wenlog 		= ISPCCDC_CFG_WENLOG_AND,
	.wait_hs_vs		= 0,
	.cam_mclk		= ISP_IMX046_MCLK,
	.raw_fmt_in		= ISPCCDC_INPUT_FMT_RG_GB,
	.u.csi.crc 		= 0x0,
	.u.csi.mode 		= 0x0,
	.u.csi.edge 		= 0x0,
	.u.csi.signalling 	= 0x0,
	.u.csi.strobe_clock_inv = 0x0,
	.u.csi.vs_edge 		= 0x0,
	.u.csi.channel 		= 0x0,
	.u.csi.vpclk 		= 0x2,
	.u.csi.data_start 	= 0x0,
	.u.csi.data_size 	= 0x0,
	.u.csi.format 		= V4L2_PIX_FMT_SGRBG10,
};


static int imx046_sensor_power_set(struct v4l2_int_device *s, enum v4l2_power power)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;
	struct isp_device *isp = dev_get_drvdata(vdev->cam->isp);
	struct isp_csi2_lanes_cfg lanecfg;
	struct isp_csi2_phy_cfg phyconfig;
	static enum v4l2_power previous_power = V4L2_POWER_OFF;
	static struct pm_qos_request_list *qos_request;
	int err = 0;

	switch (power) {
	case V4L2_POWER_ON:
		/* Power Up Sequence */
		printk(KERN_DEBUG "imx046_sensor_power_set(ON)\n");

		/*
		 * Through-put requirement:
		 * Set max OCP freq for 3630 is 200 MHz through-put
		 * is in KByte/s so 200000 KHz * 4 = 800000 KByte/s
		 */
		omap_pm_set_min_bus_tput(vdev->cam->isp,
					 OCP_INITIATOR_AGENT, 800000);

		/* Hold a constraint to keep MPU in C1 */
		omap_pm_set_max_mpu_wakeup_lat(&qos_request, 12);

		isp_csi2_reset(&isp->isp_csi2);

		lanecfg.clk.pol = IMX046_CSI2_CLOCK_POLARITY;
		lanecfg.clk.pos = IMX046_CSI2_CLOCK_LANE;
		lanecfg.data[0].pol = IMX046_CSI2_DATA0_POLARITY;
		lanecfg.data[0].pos = IMX046_CSI2_DATA0_LANE;
		lanecfg.data[1].pol = IMX046_CSI2_DATA1_POLARITY;
		lanecfg.data[1].pos = IMX046_CSI2_DATA1_LANE;
		lanecfg.data[2].pol = 0;
		lanecfg.data[2].pos = 0;
		lanecfg.data[3].pol = 0;
		lanecfg.data[3].pos = 0;
		isp_csi2_complexio_lanes_config(&isp->isp_csi2, &lanecfg);
		isp_csi2_complexio_lanes_update(&isp->isp_csi2, true);

		isp_csi2_ctrl_config_ecc_enable(&isp->isp_csi2, true);

		phyconfig.ths_term = IMX046_CSI2_PHY_THS_TERM;
		phyconfig.ths_settle = IMX046_CSI2_PHY_THS_SETTLE;
		phyconfig.tclk_term = IMX046_CSI2_PHY_TCLK_TERM;
		phyconfig.tclk_miss = IMX046_CSI2_PHY_TCLK_MISS;
		phyconfig.tclk_settle = IMX046_CSI2_PHY_TCLK_SETTLE;
		isp_csi2_phy_config(&isp->isp_csi2, &phyconfig);
		isp_csi2_phy_update(&isp->isp_csi2, true);

		isp_configure_interface(vdev->cam->isp, &imx046_if_config);

		/* Request and configure gpio pins */
		if (gpio_request(IMX046_RESET_GPIO, "imx046_rst") != 0)
			return -EIO;

		/* nRESET is active LOW. set HIGH to release reset */
		gpio_set_value(IMX046_RESET_GPIO, 1);

		/* set to output mode */
		gpio_direction_output(IMX046_RESET_GPIO, true);

		/* turn on analog power */
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,
				VAUX_1_8_V, TWL4030_VAUX4_DEDICATED);
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,
				VAUX_DEV_GRP_P1, TWL4030_VAUX4_DEV_GRP);
		udelay(100);

		/* have to put sensor to reset to guarantee detection */
		gpio_set_value(IMX046_RESET_GPIO, 0);
		udelay(1500);

		/* nRESET is active LOW. set HIGH to release reset */
		gpio_set_value(IMX046_RESET_GPIO, 1);
		udelay(300);
		break;
	case V4L2_POWER_OFF:
	case V4L2_POWER_STANDBY:
		printk(KERN_DEBUG "imx046_sensor_power_set(%s)\n",
			(power == V4L2_POWER_OFF) ? "OFF" : "STANDBY");
		/* Power Down Sequence */
		isp_csi2_complexio_power(&isp->isp_csi2, ISP_CSI2_POWER_OFF);

		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,
				VAUX_DEV_GRP_NONE, TWL4030_VAUX4_DEV_GRP);
		//twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,
		//		VAUX_DEV_GRP_NONE, TWL4030_VAUX2_DEV_GRP);
		gpio_free(IMX046_RESET_GPIO);

		/* Remove pm constraints */
		omap_pm_set_min_bus_tput(vdev->cam->isp, OCP_INITIATOR_AGENT, 0);
		omap_pm_set_max_mpu_wakeup_lat(&qos_request, -1);

		/* Make sure not to disable the MCLK twice in a row */
		if (previous_power == V4L2_POWER_ON)
			isp_disable_mclk(isp);
		break;
	}

	/* Save powerstate to know what was before calling POWER_ON. */
	previous_power = power;
	return err;
}

static u32 imx046_sensor_set_xclk(struct v4l2_int_device *s, u32 xclkfreq)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;

	return isp_set_xclk(vdev->cam->isp, xclkfreq, CAMZOOM2_USE_XCLKB);
}

static int imx046_csi2_lane_count(struct v4l2_int_device *s, int count)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;
	struct isp_device *isp = dev_get_drvdata(vdev->cam->isp);

	return isp_csi2_complexio_lanes_count(&isp->isp_csi2, count);
}

static int imx046_csi2_cfg_vp_out_ctrl(struct v4l2_int_device *s,
				       u8 vp_out_ctrl)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;
	struct isp_device *isp = dev_get_drvdata(vdev->cam->isp);

	return isp_csi2_ctrl_config_vp_out_ctrl(&isp->isp_csi2, vp_out_ctrl);
}

static int imx046_csi2_ctrl_update(struct v4l2_int_device *s, bool force_update)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;
	struct isp_device *isp = dev_get_drvdata(vdev->cam->isp);

	return isp_csi2_ctrl_update(&isp->isp_csi2, force_update);
}

static int imx046_csi2_cfg_virtual_id(struct v4l2_int_device *s, u8 ctx, u8 id)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;
	struct isp_device *isp = dev_get_drvdata(vdev->cam->isp);

	return isp_csi2_ctx_config_virtual_id(&isp->isp_csi2, ctx, id);
}

static int imx046_csi2_ctx_update(struct v4l2_int_device *s, u8 ctx,
				  bool force_update)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;
	struct isp_device *isp = dev_get_drvdata(vdev->cam->isp);

	return isp_csi2_ctx_update(&isp->isp_csi2, ctx, force_update);
}

static int imx046_csi2_calc_phy_cfg0(struct v4l2_int_device *s,
				     u32 mipiclk, u32 lbound_hs_settle,
				     u32 ubound_hs_settle)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;
	struct isp_device *isp = dev_get_drvdata(vdev->cam->isp);

	return isp_csi2_calc_phy_cfg0(&isp->isp_csi2, mipiclk,
				      lbound_hs_settle, ubound_hs_settle);
}

struct imx046_platform_data zoom2_imx046_platform_data = {
	.power_set            = imx046_sensor_power_set,
	.priv_data_set        = imx046_sensor_set_prv_data,
	.set_xclk             = imx046_sensor_set_xclk,
	.csi2_lane_count      = imx046_csi2_lane_count,
	.csi2_cfg_vp_out_ctrl = imx046_csi2_cfg_vp_out_ctrl,
	.csi2_ctrl_update     = imx046_csi2_ctrl_update,
	.csi2_cfg_virtual_id  = imx046_csi2_cfg_virtual_id,
	.csi2_ctx_update      = imx046_csi2_ctx_update,
	.csi2_calc_phy_cfg0   = imx046_csi2_calc_phy_cfg0,
};
#endif

#if (defined(CONFIG_VIDEO_MT9T113) || defined(CONFIG_VIDEO_MT9T113_MODULE))
//#include <../../../drivers/media/video/mt9t113.h>
#define MT9T113_BIGGEST_FRAME_BYTE_SIZE	PAGE_ALIGN(2048 * 1536 * 2)

#define ISP_MT9T113_MCLK		216000000

#define PIN_EN_CAMERA 109

static int config_camera_gpio_poweron(void);
static int config_camera_gpio_poweroff(void);


static struct omap34xxcam_sensor_config mt9t113_hwc = {
	.sensor_isp  = 1,
	.capture_mem = MT9T113_BIGGEST_FRAME_BYTE_SIZE * 2,
	.ival_default	= { 1, 30 },
};

static int mt9t113_sensor_set_prv_data(struct v4l2_int_device *s,void *priv)
{
	struct omap34xxcam_hw_config *hwc = priv;

	hwc->u.sensor.sensor_isp = mt9t113_hwc.sensor_isp;
	hwc->dev_index		= 2;  //2
	hwc->dev_minor		= 5;  //5
	hwc->dev_type		= OMAP34XXCAM_SLAVE_SENSOR;

	return 0;
}

static struct isp_interface_config mt9t113_if_config = {
	.ccdc_par_ser 		= ISP_PARLL,
	.dataline_shift 	= 0x1,		/*将OMAP3630的CAMERA端口下移2位，映射到CAM2-CAM9 add by zhanggang 2011-3-3*/
	.hsvs_syncdetect 	= ISPCTRL_SYNC_DETECT_VSRISE,
	.strobe 		= 0x0,
	.prestrobe 		= 0x0,
	.shutter 		= 0x0,
	.wenlog 		= ISPCCDC_CFG_WENLOG_AND,
	.wait_hs_vs		= 2,
    .cam_mclk		= ISP_MT9T113_MCLK,
	.u.par.par_bridge = 3,
	.u.par.par_clk_pol = 0,  // 0
};

static u32 mt9t113_sensor_set_xclk(struct v4l2_int_device *s, u32 xclkfreq);

static int mt9t113_sensor_power_set(struct v4l2_int_device *s, enum v4l2_power power)
{

	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;
	struct isp_device *isp = dev_get_drvdata(vdev->cam->isp);

	static enum v4l2_power previous_power = V4L2_POWER_OFF;
        static struct pm_qos_request_list *qos_request;

	int err = 0;
	
	DPRINTK_OV("previous_power = %d,mt9t113_sensor_power_set(%d)",previous_power,power);

	switch (power) {
	case V4L2_POWER_ON:
	            	DPRINTK_OV("========mt9t113 power_on  =============");
			/*0x0e是TWL4030_MODULE_PM_MASTER偏移指向�?地址，写c0,0c使能电源寄存器twl5030.pdf p276add by  zhanggang 2011-3-3*/
	#if 1		
			twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e); 
			twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);
			/* enable vpll2 to belong to P1 P2 P3 and to be 1.8V */
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x36);
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 7<<5, 0x33);
			/* clock twl5030 power registers */
			twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e); 
	#endif	
			omap_pm_set_min_bus_tput(vdev->cam->isp, OCP_INITIATOR_AGENT, 800000);
			
			/* Hold a constraint to keep MPU in C1 */
			//omap_pm_set_max_mpu_wakeup_lat(vdev->cam->isp, 12); //2.2
			omap_pm_set_max_mpu_wakeup_lat(&qos_request, 12);     //2.3

			isp_configure_interface(vdev->cam->isp,&mt9t113_if_config);
			
			config_camera_gpio_poweron();

			//cam2 bf3703 go to standby mode
			//pwdn:0:working,1:idle
			gpio_direction_output(110, 1);
			msleep(20); 

			//cam1 mt9t113 disable standby mode
			//gpio_167  pwdn,high enable
			gpio_direction_output(167, 0);			
			msleep(20); 
			
			gpio_direction_output(109, 1);//gpio_109 2.8_1.8V 
			msleep(20);
			
			mt9t113_sensor_set_xclk(s, 24000000);
			msleep(20); 

			omap_mux_init_signal("gpio_98",  OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//cam_reset 
			gpio_direction_output(98, 1);
			msleep(5);
			gpio_direction_output(98,0);
			msleep(5);	
			gpio_direction_output(98,1);
			DPRINTK_OV("reset camera");						
			msleep(20); //30

			break;

	case V4L2_POWER_OFF:	
	case V4L2_POWER_STANDBY:
			/*Let the sensor go into power down mode*/			
			
			/*Let the sensor go into power down mode*/	
			gpio_direction_output(167, 1); //go to pwdn
        	        msleep(2);
			gpio_direction_output(98,0);   //reset
			msleep(2);
		
			gpio_direction_output(167, 1); //go to pwdn
                        msleep(2);
            	        gpio_direction_output(109, 0);
			msleep(10);


            		DPRINTK_OV("========mt9t113 power_off_standby  =============");
			/* Power Down Sequence */
			isp_csi2_complexio_power(&isp->isp_csi2, ISP_CSI2_POWER_OFF);			
               			
			
			/* turn off csiphy pad power */
        /* unclock twl5030 power registers */
#if 1
        twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e); 
        twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);
        /* enable vpll2 to belong to P1 P2 P3 and to be 1.8V */
        twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, TWL4030_VAUX3_DEV_GRP);
//        twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0, 0x33);
        /* clock twl5030 power registers */
        twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e); 
#endif
        	      msleep(50);
        	
        	
                     /* Remove pm constraints */
		      omap_pm_set_min_bus_tput(vdev->cam->isp, OCP_INITIATOR_AGENT, 0);
		      omap_pm_set_max_mpu_wakeup_lat(&qos_request, -1); //2.3
			
			/* Make sure not to disable the MCLK twice in a row */
			if (previous_power == V4L2_POWER_ON){
				isp_disable_mclk(isp);
            		}        

           		
                        config_camera_gpio_poweroff();           
            		
            
			DPRINTK_OV("mt9t113_sensor_power_set(off)");
			break;
	}

	/* Save powerstate to know what was before calling POWER_ON. */
	previous_power = power;

	return err;
}

static u32 mt9t113_sensor_set_xclk(struct v4l2_int_device *s, u32 xclkfreq)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;

	return isp_set_xclk(vdev->cam->isp, xclkfreq, MT9T113_USE_XCLKA);
}


struct mt9t113_platform_data zoom2_mt9t113_platform_data = {
	.power_set            = mt9t113_sensor_power_set,
	.priv_data_set        = mt9t113_sensor_set_prv_data,
	.set_xclk             = mt9t113_sensor_set_xclk,
};

#endif


#if (defined(CONFIG_VIDEO_BF3703) || defined(CONFIG_VIDEO_BF3703_MODULE))
//#include <../../../drivers/media/video/bf3703.h>
#define BF3703_BIGGEST_FRAME_BYTE_SIZE	PAGE_ALIGN(640 * 480 * 2)

#define ISP_BF3703_MCLK		216000000


static int config_camera_gpio_poweron(void);
static int config_camera_gpio_poweroff(void);
   

static struct omap34xxcam_sensor_config bf3703_hwc = {
	.sensor_isp  = 1,
	.capture_mem = BF3703_BIGGEST_FRAME_BYTE_SIZE * 2,
	.ival_default	= { 1, 30 },
};

static int bf3703_sensor_set_prv_data(struct v4l2_int_device *s,void *priv)
{
	struct omap34xxcam_hw_config *hwc = priv;

	hwc->u.sensor.sensor_isp = bf3703_hwc.sensor_isp;
	hwc->dev_index		= 0;  //2    //2
	hwc->dev_minor		= 0;   //5
	hwc->dev_type		= OMAP34XXCAM_SLAVE_SENSOR;

	return 0;
}

static struct isp_interface_config bf3703_if_config = {
	.ccdc_par_ser 		= ISP_PARLL,
	.dataline_shift 	= 0x1,/*将OMAP3630的CAMERA端口下移2位，映射到CAM2-CAM9 add by zhanggang 2011-3-3*/
	.hsvs_syncdetect 	= ISPCTRL_SYNC_DETECT_VSRISE,//
	.strobe 		= 0x0,
	.prestrobe 		= 0x0,
	.shutter 		= 0x0,
	.wenlog 		= ISPCCDC_CFG_WENLOG_AND,
	.wait_hs_vs		= 2,
    .cam_mclk		= ISP_BF3703_MCLK,
	.u.par.par_bridge = 3,
	.u.par.par_clk_pol = 0, //0
};

static u32 bf3703_sensor_set_xclk(struct v4l2_int_device *s, u32 xclkfreq)
{
	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;

	return isp_set_xclk(vdev->cam->isp, xclkfreq, BF3703_USE_XCLKA);
}

static int bf3703_sensor_power_set(struct v4l2_int_device *s, enum v4l2_power power)
{

	struct omap34xxcam_videodev *vdev = s->u.slave->master->priv;
	struct isp_device *isp = dev_get_drvdata(vdev->cam->isp);

	static enum v4l2_power previous_power = V4L2_POWER_OFF;
        static struct pm_qos_request_list *qos_request;

	int err = 0;
	
	DPRINTK_OV("previous_power = %d,bf3703_sensor_power_set(%d)",previous_power,power);

	switch (power) {
	case V4L2_POWER_ON:
			/* turn on csiphy pad power */
			/* unclock twl5030 power registers */

			/*
			* Through-put requirement:
			* Set max OCP freq for 3630 is 200 MHz through-put
			* is in KByte/s so 200000 KHz * 4 = 800000 KByte/s
			*/
#if  1
			twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e); 
			twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);
			/* enable vpll2 to belong to P1 P2 P3 and to be 1.8V */
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x05, 0x36);
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 7<<5, 0x33);
			/* clock twl5030 power registers */
			twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e); 
#endif
			omap_pm_set_min_bus_tput(vdev->cam->isp, OCP_INITIATOR_AGENT, 800000);

			/* Hold a constraint to keep MPU in C1 */
			//omap_pm_set_max_mpu_wakeup_lat(vdev->cam->isp, 12); //2.2
			omap_pm_set_max_mpu_wakeup_lat(&qos_request, 12);     //2.3

			isp_configure_interface(vdev->cam->isp,&bf3703_if_config);

			config_camera_gpio_poweron();

			//cam1 mt9t113 go to standby	
			gpio_direction_output(167, 1); //cam1 pwdn
		        msleep(20);

			//cam2 bf3703 disable standby
                        //pwdn:0:working,1:pwdn
			gpio_direction_output(110, 0); //cam2 pwdn
                        msleep(5);

		        gpio_direction_output(109, 1); //gpio_109 2.8_1.8V
			msleep(5); 
	
			bf3703_sensor_set_xclk(s, 24000000);			
			msleep(20);
			
			break;

	case V4L2_POWER_OFF:	
	case V4L2_POWER_STANDBY:
	
            		DPRINTK_OV("========V4L2_V4L2_POWER_OFF =============");
			
			omap_mux_init_signal("gpio_110",  OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_110 output
			gpio_direction_output(110, 1);
                        
			/* Power Down Sequence */
			omap_mux_init_signal("gpio_109",  OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//gpio_109 2.8_1.8V 
                        gpio_request(109, "cam_enable");
			gpio_direction_output(109, 0);

			isp_csi2_complexio_power(&isp->isp_csi2, ISP_CSI2_POWER_OFF);
			

#if 	1		
			twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xc0, 0x0e); 
			twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0c, 0x0e);
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,0x00, TWL4030_VAUX3_DEV_GRP);
			//twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER,VAUX3_DEV_GRP_NONE, TWL4030_VAUX2_DEV_GRP);
			twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x00, 0x0e);
#endif
 msleep(50);
			/* Remove pm constraints */
			omap_pm_set_min_bus_tput(vdev->cam->isp, OCP_INITIATOR_AGENT, 0);
			 //omap_pm_set_max_mpu_wakeup_lat(vdev->cam->isp, -1); //2.2
                         omap_pm_set_max_mpu_wakeup_lat(&qos_request, -1); //2.3

			/* Make sure not to disable the MCLK twice in a row */
			if (previous_power == V4L2_POWER_ON){
				isp_disable_mclk(isp);
             }
			

            config_camera_gpio_poweroff();

			DPRINTK_OV("bf3703_sensor_power_set(STANDBY)");
			break;
	}

	/* Save powerstate to know what was before calling POWER_ON. */
	previous_power = power;

	return err;
}

struct BF3703_platform_data zoom2_bf3703_platform_data = {
	.power_set            = bf3703_sensor_power_set,
	.priv_data_set        = bf3703_sensor_set_prv_data,
	.set_xclk             = bf3703_sensor_set_xclk,
};

#endif

static int config_camera_gpio_poweron(void)
{
	DPRINTK_OV(" config_camera_gpio_poweron!");
	
	omap_mux_init_signal("cam_d2.cam_d2", OMAP_PIN_INPUT);		//D2
	omap_mux_init_signal("cam_d3.cam_d3", OMAP_PIN_INPUT);		//D3
	omap_mux_init_signal("cam_d4.cam_d4", OMAP_PIN_INPUT);		//D4
	omap_mux_init_signal("cam_d5.cam_d5", OMAP_PIN_INPUT);		//D5
	omap_mux_init_signal("cam_d6.cam_d6", OMAP_PIN_INPUT);		//D6
	omap_mux_init_signal("cam_d7.cam_d7", OMAP_PIN_INPUT);		//D7
	omap_mux_init_signal("cam_d8.cam_d8", OMAP_PIN_INPUT);		//D8
	omap_mux_init_signal("cam_d9.cam_d9", OMAP_PIN_INPUT);		//D9				

	omap_mux_init_signal("cam_hs.cam_hs", OMAP_PIN_INPUT);		// gpio_94
	omap_mux_init_signal("cam_vs.cam_vs", OMAP_PIN_INPUT);		// gpio_95

	omap_mux_init_signal("etk_d0.gpio_14", OMAP_PIN_INPUT);		//GPIO_14,scl
	omap_mux_init_signal("etk_d1.gpio_15", OMAP_PIN_INPUT);		//GPIO_15,sda
	
	omap_mux_init_signal("cam_xclka.cam_xclka", OMAP_PIN_OUTPUT);		//GPIO_96,mclk
	omap_mux_init_signal("cam_pclk.cam_pclk", OMAP_PIN_INPUT);		//GPIO_97,pclk	
		
	omap_mux_init_signal("gpio_109", OMAP_PIN_OUTPUT);//gpio_178,2.8_1.8V,high enable
	
	//omap_mux_init_signal("cam_fld.gpio_98", OMAP_PIN_OUTPUT); //gpio_98 output,reset

        //cam1 pwdn
	omap_mux_init_signal("gpio_167", OMAP_PIN_OUTPUT);//gpio_167 output,pwdn

        //cam2 pwdn
	omap_mux_init_signal("gpio_110", OMAP_PIN_OUTPUT);//cam2 pwdn,low enable
			

	return 0;
}

static int config_camera_gpio_poweroff(void)
{

	DPRINTK_OV("config_camera_gpio_poweroff ");
#if 1
	omap_mux_init_signal("cam_d2.gpio_101", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);  //D2
	gpio_request(101, "cam_d2");
	gpio_direction_output(101, 0);
	omap_mux_init_signal("cam_d3.gpio_102", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);  //D3
	gpio_request(102, "cam_d3");
	gpio_direction_output(102, 0);
	omap_mux_init_signal("cam_d4.gpio_103", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//D4
	gpio_request(103, "cam_d4");
	gpio_direction_output(103, 0);
	omap_mux_init_signal("cam_d5.gpio_104", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//D5
	gpio_request(104, "cam_d5");
	gpio_direction_output(104, 0);
	omap_mux_init_signal("cam_d6.gpio_105", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//D6
	gpio_request(105, "cam_d6");
	gpio_direction_output(105, 0);
	omap_mux_init_signal("cam_d7.gpio_106", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//D7
	gpio_request(106, "cam_d7");
	gpio_direction_output(106, 0);
	omap_mux_init_signal("cam_d8.gpio_107", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//D8
	gpio_request(107, "cam_d8");
	gpio_direction_output(107, 0);
	omap_mux_init_signal("cam_d9.gpio_108", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//D9
	gpio_request(108, "cam_d9");
	gpio_direction_output(108, 0);
	omap_mux_init_signal("cam_hs.gpio_94", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//hs
	gpio_request(94, "cam_hs");
	gpio_direction_output(94, 0);
	omap_mux_init_signal("cam_vs.gpio_95", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//vs	
	gpio_request(95, "cam_vs");
	gpio_direction_output(95, 0);
	omap_mux_init_signal("etk_d0.gpio_14", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//SCL
	gpio_request(14, "cam_scl");
	gpio_direction_output(14, 0);
	omap_mux_init_signal("etk_d1.gpio_15",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//SDA
	gpio_request(15, "cam_sda");
	gpio_direction_output(15, 0);
	omap_mux_init_signal("cam_xclka.gpio_96",OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//GPIO96,mclk
	gpio_request(96, "cam_mclk");
	gpio_direction_output(96, 0);
	omap_mux_init_signal("cam_pclk.gpio_97", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//GPIO97,pclk
	gpio_request(97, "cam_pclk");
	gpio_direction_output(97, 0);

	omap_mux_init_signal("gpio_109", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	//2.8_1.8V
	gpio_request(109, "cam_enable");
	gpio_direction_output(109, 0);


	//omap_mux_init_signal("gpio_167", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	 //cam1 pwdn
	//gpio_request(167, "cam_enable");
	//gpio_direction_output(167, 0);

	//omap_mux_init_signal("gpio_110", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);	 //cam2 pwdn
	//gpio_request(110, "cam_enable");
	//gpio_direction_output(110, 0);

#endif
	return 0;
}


#if (defined(CONFIG_VIDEO_MT9T113) || defined(CONFIG_VIDEO_MT9T113_MODULE))
#include <../drivers/media/video/adp1650c.h>

static int adp1650_gpio_init(void)
{
 
     omap_mux_init_signal("gpio_155", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW);//->adp1650_flash_en

     omap_mux_init_signal("hdq_sio.gpio_170", OMAP_PIN_OUTPUT);//->adp1650_strobe
     //omap_mux_init_signal("cam_strobe.gpio_126", OMAP_PIN_OUTPUT);//->adp1650_strobe
     //omap_mux_init_signal("cam_strobe.cam_strobe", OMAP_PIN_OUTPUT);//->adp1650_strobe

     //omap_mux_init_signal("gpio_152", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW); //->adp1650_gpio2
     omap_mux_init_signal("gpio_153", OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW); //->adp1650_gpio1

    return 0;

}

static int adp1650_set_prv_data(void *priv)
{
	struct omap34xxcam_hw_config *hwc = priv;

	hwc->dev_index = 2;
	hwc->dev_minor = 5;
	hwc->dev_type = OMAP34XXCAM_SLAVE_FLASH;

	return 0;
}

struct adp1650_platform_data zoom_adp1650_data = {
        .gpio_init        = adp1650_gpio_init,
	.priv_data_set    = adp1650_set_prv_data,
};

#endif


void __init zoom2_cam_init(void)
{
     printk("===enter in %s func===\n",__FUNCTION__);
    cam_inited = 0;

#if 0	
#if defined(CONFIG_VIDEO_IMX046) || defined(CONFIG_VIDEO_IMX046_MODULE)
	/* Request and configure gpio pins */
	if (gpio_request(IMX046_STANDBY_GPIO, "ov3640_standby_gpio") != 0) {
		printk(KERN_ERR "Could not request GPIO %d",
					IMX046_STANDBY_GPIO);
		return;
	}

	/* set to output mode */
	gpio_direction_output(IMX046_STANDBY_GPIO, true);
#endif

#else
	if (gpio_request(167, "mt9t113_standby_gpio") != 0) {
		printk(KERN_ERR "Could not request GPIO %d",
					167);
		return;
	}
	
      if (gpio_request(98, "mt9t113_reset_gpio") != 0) {
		printk(KERN_ERR "Could not request GPIO %d",
					98);
		return;
	}

      if (gpio_request(110, "bf3703_standby_gpio") != 0) {
		printk(KERN_ERR "Could not request GPIO %d",
					110);
		return;
	}

      if (gpio_request(109, "en_camera") != 0) {
		printk(KERN_ERR "Could not request GPIO %d",
					109);
		return;
	}

      if (gpio_request(155, "flash_en") != 0) {
		printk(KERN_ERR "Could not request GPIO %d",
					155);
		return;
	}

      if (gpio_request(153, "flash_gpio1") != 0) {
		printk(KERN_ERR "Could not request GPIO %d",
					153);
		return;
	}

      if (gpio_request(170, "cam_strobe") != 0) {
		printk(KERN_ERR "Could not request GPIO %d",
					170);
		return;
	}


#endif
	cam_inited = 1;
}
#endif

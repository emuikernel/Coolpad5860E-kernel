/*
 * drivers/media/video/bf3703.c
 *
 * Sony bf3703 sensor driver
 *
 *
 * Copyright (C) 2008 Hewlett Packard
 *
 * Leverage mt9p012.c
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/i2c.h>
#include <linux/delay.h>

#include <media/v4l2-int-device.h>

#include "omap34xxcam.h"
#include "isp/isp.h"
#include "isp/ispcsi2.h"

#include "../../../arch/arm/mach-omap2/board-cp5860e-camera.h"
#include "bf3703.h"

#define BF3703_DRIVER_NAME  "bf3703"
#define BF3703_MOD_NAME "BF3703: "

#define I2C_M_WR 0

#define BF3703_YUV_MODE 1

#define I2C_8BIT   1
#define I2C_16BIT  2
#define I2C_32BIT  4


static struct BF3703_sensor bf3703;
static struct i2c_driver bf3703sensor_i2c_driver;

/* list of image formats supported by bf3703 sensor */
static const struct v4l2_fmtdesc bf3703_formats[] = {
	{
		.description	= "Bayer10 (RGr/GbB)",
		.pixelformat	= V4L2_PIX_FMT_SRGGB10,
	}
};
#define BF3703_NUM_CAPTURE_FORMATS ARRAY_SIZE(bf3703_formats)

//#define BF3703_DEBUG 1

#ifdef BF3703_DEBUG
#define DPRINTK_OV(format, ...)				\
  printk(KERN_ERR "BF3703:%s: " format " ",__func__, ## __VA_ARGS__)
#else
#define DPRINTK_OV(format, ...)
#endif

static enum v4l2_power current_power_state;

static int BF3703_capture_mode = 0;
static int BF3703_after_capture = 0;
static int BF3703_set_power = 0;

extern void yl_set_debug_enalbe(int module_name, int value);

static int BF3703_power_off(struct v4l2_int_device *s);

/* List of image formats supported by BF3703sensor */
  const static struct v4l2_fmtdesc BF3703_formats[] = {
#if defined(BF3703_RAW_MODE)
    {
      .description	= "BF3703_RAW10",
      .pixelformat	= V4L2_PIX_FMT_SGRBG10,
    },
#elif defined(BF3703_YUV_MODE)
    {
      .description	= "YUYV,422",
      .pixelformat	= V4L2_PIX_FMT_UYVY,       
    },
#else    
    {
      /* Note:  V4L2 defines BF3703_RGB565 as:
       *
       *	Byte 0			  Byte 1
       *	g2 g1 g0 r4 r3 r2 r1 r0	  b4 b3 b2 b1 b0 g5 g4 g3
       *
       * We interpret BF3703_RGB565 as:
       *
       *	Byte 0			  Byte 1
       *	g2 g1 g0 b4 b3 b2 b1 b0	  r4 r3 r2 r1 r0 g5 g4 g3
       */
      .description	= "BF3703_RGB565, le",
      .pixelformat	= V4L2_PIX_FMT_RGB565,
    },
    {
      /* Note:  V4L2 defines RGB565X as:
       *
       *	Byte 0			  Byte 1
       *	b4 b3 b2 b1 b0 g5 g4 g3	  g2 g1 g0 r4 r3 r2 r1 r0
       *
       * We interpret RGB565X as:
       *
       *	Byte 0			  Byte 1
       *	r4 r3 r2 r1 r0 g5 g4 g3	  g2 g1 g0 b4 b3 b2 b1 b0
       */
      .description	= "BF3703_RGB565, be",
      .pixelformat	= V4L2_PIX_FMT_RGB565X,
    },
    {
      .description	= "YUYV (BF3703_YUV 4:2:2), packed",
      .pixelformat	= V4L2_PIX_FMT_YUYV,
    },
    {
      .description	= "UYVY, packed",
      .pixelformat	= V4L2_PIX_FMT_UYVY,
    },
    {
      /* Note:  V4L2 defines BF3703_RGB555 as:
       *
       *	Byte 0			  Byte 1
       *	g2 g1 g0 r4 r3 r2 r1 r0	  x  b4 b3 b2 b1 b0 g4 g3
       *
       * We interpret BF3703_RGB555 as:
       *
       *	Byte 0			  Byte 1
       *	g2 g1 g0 b4 b3 b2 b1 b0	  x  r4 r3 r2 r1 r0 g4 g3
       */
      .description	= "BF3703_RGB555, le",
      .pixelformat	= V4L2_PIX_FMT_RGB555,
    },
    {
      /* Note:  V4L2 defines RGB555X as:
       *
       *	Byte 0			  Byte 1
       *	x  b4 b3 b2 b1 b0 g4 g3	  g2 g1 g0 r4 r3 r2 r1 r0
       *
       * We interpret RGB555X as:
       *
       *	Byte 0			  Byte 1
       *	x  r4 r3 r2 r1 r0 g4 g3	  g2 g1 g0 b4 b3 b2 b1 b0
       */
      .description	= "BF3703_RGB555, be",
      .pixelformat	= V4L2_PIX_FMT_RGB555X,
    },
#endif
  };

#define NUM_CAPTURE_FORMATS (sizeof(BF3703_formats) / sizeof(BF3703_formats[0]))

/* register initialization tables for BF3703 */
#define BF3703_REG_TERM 0xFF	/* terminating list entry for reg */
#define BF3703_VAL_TERM 0xFF	/* terminating list entry for val */


/*
 * struct vcontrol - Video controls
 * @v4l2_queryctrl: V4L2 VIDIOC_QUERYCTRL ioctl structure
 * @current_value: current value of this control
 */
static struct vcontrol {
  struct v4l2_queryctrl qc;
  int current_value;
} video_control[] = {
#ifdef BF3703_YUV_MODE
  {
    {
      .id = V4L2_CID_EXPOSURE,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "Exposure",
      .minimum = BF3703_MIN_EXPOSURE,
      .maximum = BF3703_MAX_EXPOSURE,
      .step = BF3703_EXPOSURE_STEP,
      .default_value = BF3703_DEF_EXPOSURE,
    },
    .current_value = BF3703_DEF_EXPOSURE,
  }

#if 0
    {
      .id = V4L2_CID_BRIGHTNESS,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "Brightness",
      .minimum = BF3703_MIN_BRIGHT,
      .maximum = BF3703_MAX_BRIGHT,
      .step = BF3703_BRIGHT_STEP,
      .default_value = BF3703_DEF_BRIGHT,
    },
    .current_value = BF3703_DEF_BRIGHT,
  },
  {
    {
      .id = V4L2_CID_CONTRAST,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "Contrast",
      .minimum = BF3703_MIN_CONTRAST,
      .maximum = BF3703_MAX_CONTRAST,
      .step = BF3703_CONTRAST_STEP,
      .default_value = BF3703_DEF_CONTRAST,
    },
    .current_value = BF3703_DEF_CONTRAST,
  },
  {
    {
      .id = V4L2_CID_PRIVATE_BASE,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "Color Effects",
      .minimum = BF3703_MIN_COLOR,
      .maximum = BF3703_MAX_COLOR,
      .step = BF3703_COLOR_STEP,
      .default_value = BF3703_DEF_COLOR,
    },
    .current_value = BF3703_DEF_COLOR,
  }
 #endif
#endif
};

/*
 * find_vctrl - Finds the requested ID in the video control structure array
 * @id: ID of control to search the video control array.
 *
 * Returns the index of the requested ID from the control structure array
 */
static int find_vctrl(int id)
{
  int i = 0;

  if (id < V4L2_CID_BASE)
    return -EDOM;

  for (i = (ARRAY_SIZE(video_control) - 1); i >= 0; i--)
    if (video_control[i].qc.id == id)
      break;
  if (i < 0)
    i = -EINVAL;
  return i;
}

static int
bf3703_read_reg(struct i2c_client *client, u16 data_length, u8 reg, u8 *val)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[4];
	int retries = 0;

	if (!client->adapter)
		return -ENODEV;
	if (data_length != I2C_8BIT && data_length != I2C_16BIT
			&& data_length != I2C_32BIT)
		return -EINVAL;

read_retry:

	msg->addr = client->addr;
	msg->flags = 0;   //0;
	msg->len = 1;
	msg->buf = data;

	/* Write addr - high byte goes out first */
	data[0] = reg ;

	err = i2c_transfer(client->adapter, msg, 1);

	/* Read back data */
	if (err >= 0) {
		msg->len = data_length;
		msg->flags = I2C_M_RD;
		err = i2c_transfer(client->adapter, msg, 1);
	}
	if (err >= 0) {
		*val = 0;

		/* high byte comes first */
		if (data_length == I2C_8BIT)
			*val = data[0];
		else if (data_length == I2C_16BIT)
			*val = data[1] + (data[0] << 8);
		else
			*val = data[3] + (data[2] << 8) +
				(data[1] << 16) + (data[0] << 24);
		return 0;
	}
	
	if (retries < 2) {
		v4l_info(client, "Retrying I2C... %d", retries);
		retries++;
		mdelay(20);
		goto read_retry;
	}
	
	v4l_err(client, "read from offset 0x%x error %d", reg, err);
	return err;
}

/**
 * Write a value to a register in bf3703 sensor device.
 * @client: i2c driver client structure.
 * @reg: Address of the register to read value from.
 * @val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int bf3703_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	int err = 0;
	struct i2c_msg msg[1];
	unsigned char data[2];
	int retries = 0;
DPRINTK_OV("==Cleint: %x reg: %x  val: %x====",client->addr,reg,val);
	if (!client->adapter)
		return -ENODEV;

retry:
	msg->addr = client->addr;
	msg->flags = I2C_M_WR;
	msg->len = 2;  /* add address bytes */
	msg->buf = data;

	/* high byte goes out first */
	data[0] = reg;
	data[1] = val; 
	

	err = i2c_transfer(client->adapter, msg, 1);

	if (err >= 0)
		return 0;

	if (retries < 2) {
		v4l_info(client, "Retrying I2C... %d", retries);
		retries++;
		mdelay(20);
		goto retry;
	}

	return err;
}

/**s
 * Initialize a list of bf3703 registers.
 * The list of registers is terminated by the pair of values
 * {OV3640_REG_TERM, OV3640_VAL_TERM}.
 * @client: i2c driver client structure.
 * @reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int bf3703_write_regs(struct i2c_client *client,
			     const struct BF3703_reg reglist[])
{
	int err = 0; 
	u32 val =0;
	const struct BF3703_reg *list = reglist;

	while (!((list->reg == BF3703_REG_TERM)
		&& (list->val == BF3703_VAL_TERM))) {
		err = bf3703_write_reg(client, list->reg,
				list->val);
#if 0				
		bf3703_read_reg(client,1,list->reg, &val);
		printk(KERN_ERR "\n=======read reg:%x  value: %x======\n",list->reg,val);
#endif

		val =0;
		if (err)
		{
			DPRINTK_OV("=======writeERR reg:%x  value: %x======",list->reg,val);		
			return err;			
		}

		list++;
	}
	return 0;
}

/* Find the best match for a requested image capture size.  The best match
 * is chosen as the nearest match that has the same number or fewer pixels
 * as the requested size, or the smallest image size if the requested size
 * has fewer pixels than the smallest image.
 */
  static enum image_size_bf3703
BF3703_find_size(unsigned int width, unsigned int height)
{

  enum image_size_bf3703 isize;

  for (isize = 0; isize < ARRAY_SIZE(BF3703_sizes); isize++) {
    if ((BF3703_sizes[isize].height >= height) &&
        (BF3703_sizes[isize].width >= width)) {
      break;
    }
  }
  DPRINTK_OV("width = %d,height = %d,return %d",width,height,isize);
  return isize;
}




/* Detect if an BF3703 is present, returns a negative error number if no
 * device is detected, or pidl as version number if a device is detected.
 */
static int BF3703_detect(struct i2c_client *client)
{

	unsigned char  v1, v2;
        unsigned int v;
	int ret;

       // dev_err(&client->dev, "test 1\n");

	ret = bf3703_read_reg(client,1, 0x1c, &v1);		//Product ID MSB = 0x7f

        DPRINTK_OV("==0x1c :  %X ==",v1);
		
	ret = bf3703_read_reg(client, 1, 0x1d, &v2);	 //Product ID LSB = 0xa2	

        DPRINTK_OV("==0x1d :  %X ==",v2);

        v = v1<<8 | v2;

        DPRINTK_OV("==v :  %X ==",v);

        if(v == 0x7FA2)
        {
            return v;
        }

       return -ENODEV;

  
}

/*
 * Configure the BF3703 for a specified image size, pixel format, and frame
 * period.  xclk is the frequency (in Hz) of the xclk input to the BF3703.
 * fper is the frame period (in seconds) expressed as a fraction.
 * Returns zero if successful, or non-zero otherwise.
 * The actual frame period is returned in fper.
 */
static int BF3703_configure(struct v4l2_int_device *s)
{
  struct BF3703_sensor *sensor = s->priv;
  struct v4l2_pix_format *pix = &sensor->pix;
  struct i2c_client *client = sensor->i2c_client;
  enum image_size_bf3703 isize = ARRAY_SIZE(BF3703_sizes) - 1;
  struct bf3703_sensor_ext_params *extp = &(sensor->ext_params);
  
  int  err = 0;
 // int  is_reset = 1;

  enum pixel_format_bf3703 pfmt = BF3703_YUV;
  switch (pix->pixelformat) {

    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_RGB565X:
      pfmt = BF3703_RGB565;
      break;

    case V4L2_PIX_FMT_RGB555:
    case V4L2_PIX_FMT_RGB555X:
      pfmt = BF3703_RGB555;
      break;

    case V4L2_PIX_FMT_SGRBG10:
      pfmt = BF3703_RAW10;
      break;

    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_UYVY:
    default:
      pfmt = BF3703_YUV;
  }
  
  
    isize = BF3703_find_size(pix->width, pix->height);
    DPRINTK_OV("=====isize is : %d pix->width: %d pix->height: %d====",isize,pix->width, pix->height);

  if(BF3703_capture_mode){  
        DPRINTK_OV(">>>>>bf3703 go to capture "); 
        if(extp->exposure == BF3703_DEF_EXPOSURE)
        {
            bf3703_write_regs(client, BF3703_exposure_00); 
        }
        bf3703_write_regs(client, bf3703_xxx_preview[isize]);
        
        mdelay(100);//80 
        BF3703_capture_mode = 0;
        BF3703_after_capture = 1;
  }
  else if((!BF3703_after_capture) && (!BF3703_set_power))
  {  
        DPRINTK_OV(">>>>>bf3703 coming to preview initialization !");
	//bf3703_write_reg(client, 0x12, 0x80);
        mdelay(5);
              
        bf3703_write_regs(client, icBf3703Init);
        mdelay(100);//80
   
    	bf3703_write_regs(client, bf3703_xxx_preview[isize]);
        mdelay(20);//80 

   }
   else
   {
        DPRINTK_OV( ">>>>>bf3703 coming to preview !");        
        DPRINTK_OV( "before: exposure = %d",extp->exposure);
        if(extp->exposure <= 0 || extp->exposure > ARRAY_SIZE(BF3703_exposure))
            extp->exposure = BF3703_DEF_EXPOSURE;
        DPRINTK_OV( "bf3703:after: exposure = %d\n",extp->exposure);
        bf3703_write_regs(client, BF3703_exposure[extp->exposure - 1]);
        mdelay(10);
        bf3703_write_regs(client, bf3703_xxx_preview[isize]);
        mdelay(100);//80 
        BF3703_after_capture = 0;
   }

  DPRINTK_OV("pix->width = %d,pix->height = %d",pix->width,pix->height);   
  DPRINTK_OV("format = %d,size = %d",pfmt,isize); 

  /* Store image size */
  sensor->width = pix->width;
  sensor->height = pix->height;

  sensor->crop_rect.left = 0;
  sensor->crop_rect.width = pix->width;
  sensor->crop_rect.top = 0;
  sensor->crop_rect.height = pix->height;

  return err;
}


/* To get the cropping capabilities of BF3703 sensor
 * Returns zero if successful, or non-zero otherwise.
 */
static int ioctl_cropcap(struct v4l2_int_device *s,
    struct v4l2_cropcap *cropcap)
{
  struct BF3703_sensor *sensor = s->priv;

  DPRINTK_OV("sensor->width = %ld,sensor->height = %ld",sensor->width,sensor->height);
  cropcap->bounds.top = 0;
  cropcap->bounds.left = 0;
  cropcap->bounds.width = sensor->width;
  cropcap->bounds.height = sensor->height;
  cropcap->defrect = cropcap->bounds;
  cropcap->pixelaspect.numerator = 1;
  cropcap->pixelaspect.denominator = 1;
  sensor->crop_rect = cropcap->defrect;
  return 0;
}
 
/* To get the current crop window for of BF3703 sensor
 * Returns zero if successful, or non-zero otherwise.
 */
static int ioctl_g_crop(struct v4l2_int_device *s, struct  v4l2_crop *crop)
{
  struct BF3703_sensor *sensor = s->priv;
  DPRINTK_OV("\n");
  crop->c = sensor->crop_rect;
  return 0;
}

/* To set the crop window for of BF3703 sensor
 * Returns zero if successful, or non-zero otherwise.
 */
static int ioctl_s_crop(struct v4l2_int_device *s, struct  v4l2_crop *crop)
{
  return 0;
}


/*
 * ioctl_queryctrl - V4L2 sensor interface handler for VIDIOC_QUERYCTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @qc: standard V4L2 VIDIOC_QUERYCTRL ioctl structure
 *
 * If the requested control is supported, returns the control information
 * from the video_control[] array.  Otherwise, returns -EINVAL if the
 * control is not supported.
 */
static int ioctl_queryctrl(struct v4l2_int_device *s,
    struct v4l2_queryctrl *qc)
{
  int i;

  i = find_vctrl(qc->id);
  if (i == -EINVAL)
    qc->flags = V4L2_CTRL_FLAG_DISABLED;

  if (i < 0)
    return -EINVAL;

  *qc = video_control[i].qc;
  return 0;
}

/*
 * ioctl_g_ctrl - V4L2 sensor interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_G_CTRL ioctl structure
 *
 * If the requested control is supported, returns the control's current
 * value from the video_control[] array.  Otherwise, returns -EINVAL
 * if the control is not supported.
 */

static int ioctl_g_ctrl(struct v4l2_int_device *s,
    struct v4l2_control *vc)
{
  struct vcontrol *lvc;
  int i;
DPRINTK_OV("===entring ioctl_g_ctrl===");
  i = find_vctrl(vc->id);
  if (i < 0)
    return -EINVAL;
  lvc = &video_control[i];

  switch (vc->id) {
    case V4L2_CID_EXPOSURE:
      vc->value = lvc->current_value; //linwinx 20111219
      DPRINTK_OV("exposure vc->value = %d",vc->value);
      break; 
    case V4L2_CID_BRIGHTNESS:
      vc->value = lvc->current_value;
      break;
    case V4L2_CID_CONTRAST:
      vc->value = lvc->current_value;
      break;
    case V4L2_CID_PRIVATE_BASE:
      vc->value = lvc->current_value;
      break;

    default:
      break;   
  }
  return 0;
}

/*
 * ioctl_s_ctrl - V4L2 sensor interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the video_control[] array).  Otherwise,
 * returns -EINVAL if the control is not supported.
 */
static int ioctl_s_ctrl(struct v4l2_int_device *s,
    struct v4l2_control *vc)
{
  int err = 0;
  struct BF3703_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;
  struct bf3703_sensor_ext_params *extp = &(sensor->ext_params);
//  unsigned int reg3306 = 0;
#if 0
  if(vc->id != V4L2_CID_CAPTURE)
    return 0;
#endif

  DPRINTK_OV("===entring ioctl_s_ctrl===");
  DPRINTK_OV("vc->id = %d,   value = %d",vc->id,vc->value);
  
  //DPRINTK_OV("V4L2_CID_EFFECT = %d\n",V4L2_CID_EFFECT);
  switch(vc->id){
    case V4L2_CID_CAPTURE:
      BF3703_capture_mode = 1;
      break;
    case V4L2_CID_POWER_ON:
       BF3703_set_power = 1;
       printk(KERN_ERR ">>>>>>>>>>>>V4L2_CID_POWER_ON");
       break;
    case V4L2_CID_POWER_OFF:
        BF3703_set_power = 0;
        BF3703_capture_mode = 0;
        BF3703_after_capture = 0;
        printk(KERN_ERR ">>>>>>>>>>>V4L2_CID_POWER_OFF"); 
        BF3703_power_off(s);
        yl_set_debug_enalbe(7, 8);
        yl_set_debug_enalbe(1, 4);
        printk(KERN_ERR "BF3703:>>>open debug log\n"); 
       break;

    case V4L2_CID_EXPOSURE:
      if(vc->value <= 0 || vc->value > ARRAY_SIZE(BF3703_exposure))
        vc->value = BF3703_DEF_EXPOSURE;
      
      if(current_power_state == V4L2_POWER_ON){
        if(extp->exposure != vc->value)
        {
            err = bf3703_write_regs(client, BF3703_exposure[vc->value - 1]);
        }
      }	
      extp->exposure = vc->value;
      break;

#if 0
    case V4L2_CID_EFFECT:
      if(vc->value < 0 || vc->value > ARRAY_SIZE(BF3703_effects))
        vc->value = BF3703_DEF_EFFECT;
      extp->effect = vc->value;
      if(current_power_state == V4L2_POWER_ON){
//        err = bf3703_write_regs(client, BF3703_effects[vc->value]);
      }			
      break;
    case V4L2_CID_SCENE:
      break;

    case V4L2_CID_SHARPNESS:
      if(vc->value < 0 || vc->value > ARRAY_SIZE(BF3703_sharpness))
        vc->value = BF3703_DEF_SHARPNESS;
      extp->sharpness= vc->value;
      if(current_power_state == V4L2_POWER_ON){
        err = bf3703_write_regs(client, BF3703_sharpness[vc->value]);
      }	
      break;	
	
    case V4L2_CID_SATURATION:
      if(vc->value < 0 || vc->value > ARRAY_SIZE(BF3703_saturation))
        vc->value = BF3703_DEF_SATURATION;
      extp->saturation= vc->value;
      if(current_power_state == V4L2_POWER_ON){
//        err = bf3703_write_regs(client, BF3703_saturation[vc->value - 1]);
      }	
      break;
	
    case V4L2_CID_CONTRAST:
      if(vc->value < 0 || vc->value > ARRAY_SIZE(BF3703_contrast))
        vc->value = BF3703_DEF_CONTRAST;
      extp->contrast = vc->value;
      if(current_power_state == V4L2_POWER_ON){
        err = bf3703_write_regs(client, BF3703_contrast[vc->value - 1]);
      }			
      break;
      
    case V4L2_CID_BRIGHTNESS:
      if(vc->value < 0 || vc->value > ARRAY_SIZE(BF3703_brightness))
        vc->value = BF3703_DEF_BRIGHT;
      extp->brightness = vc->value;
      if(current_power_state == V4L2_POWER_ON){
        err = bf3703_write_regs(client, BF3703_brightness[vc->value - 1]);
      }			
      break;

    case V4L2_CID_DO_WHITE_BALANCE:
      if(vc->value < 0 || vc->value > ARRAY_SIZE(BF3703_whitebalance))
        vc->value = BF3703_DEF_WB;
      extp->white_balance = vc->value;
      if(current_power_state == V4L2_POWER_ON){
        bf3703_read_reg(client,1,0x3306,&reg3306);

        if(vc->value == 0){
          reg3306 &= ~(0x2);
        }else{
          reg3306 |= 0x2;
        }
//        bf3703_write_reg(client,0x3306,(u8)reg3306);
//        err = bf3703_write_regs(client, BF3703_whitebalance[vc->value]);
      }			
      break;

#endif
   default:
      break; 
       
  }
  DPRINTK_OV("%s return %d\n",__func__,err);
  
  return err;
  
}



/*
 * ioctl_enum_fmt_cap - Implement the CAPTURE buffer VIDIOC_ENUM_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @fmt: standard V4L2 VIDIOC_ENUM_FMT ioctl structure
 *
 * Implement the VIDIOC_ENUM_FMT ioctl for the CAPTURE buffer type.
 */
static int ioctl_enum_fmt_cap(struct v4l2_int_device *s,
    struct v4l2_fmtdesc *fmt)
{


  int index = fmt->index;
  enum v4l2_buf_type type = fmt->type;

  memset(fmt, 0, sizeof(*fmt));
  fmt->index = index;
  fmt->type = type;
  DPRINTK_OV("index = %d,type = %d",index,type);
  switch (fmt->type) {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
      if (index >= NUM_CAPTURE_FORMATS)
        return -EINVAL;
      break;
    default:
      return -EINVAL;
  }

  fmt->flags = BF3703_formats[index].flags;
  strlcpy(fmt->description, BF3703_formats[index].description,
      sizeof(fmt->description));
  fmt->pixelformat = BF3703_formats[index].pixelformat;

  return 0;
}


/*
 * ioctl_try_fmt_cap - Implement the CAPTURE buffer VIDIOC_TRY_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 VIDIOC_TRY_FMT ioctl structure
 *
 * Implement the VIDIOC_TRY_FMT ioctl for the CAPTURE buffer type.  This
 * ioctl is used to negotiate the image capture size and pixel format
 * without actually making it take effect.
 */

static int ioctl_try_fmt_cap(struct v4l2_int_device *s,
    struct v4l2_format *f)
{
  int ifmt;
  enum image_size_bf3703 isize = ARRAY_SIZE(BF3703_sizes) - 1; 
  struct v4l2_pix_format *pix = &f->fmt.pix;


  DPRINTK_OV("v4l2_pix_format(w:%d,h:%d)",pix->width,pix->height);

  if (pix->width > BF3703_sizes[isize].width)
    pix->width = BF3703_sizes[isize].width;
  if (pix->height > BF3703_sizes[isize].height)
    pix->height = BF3703_sizes[isize].height;

  isize = BF3703_find_size(pix->width, pix->height);
  pix->width = BF3703_sizes[isize].width;
  pix->height = BF3703_sizes[isize].height;

  for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) {
    if (pix->pixelformat == BF3703_formats[ifmt].pixelformat)
      break;
  }
  if (ifmt == NUM_CAPTURE_FORMATS)
    ifmt = 0;
  pix->pixelformat = BF3703_formats[ifmt].pixelformat;
  pix->field = V4L2_FIELD_NONE;
  pix->bytesperline = pix->width*2;
  pix->sizeimage = pix->bytesperline*pix->height;
  pix->priv = 0;
  switch (pix->pixelformat) {
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_UYVY:
    default:
      pix->colorspace = V4L2_COLORSPACE_JPEG;
      break;
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_RGB565X:
    case V4L2_PIX_FMT_RGB555:
    case V4L2_PIX_FMT_RGB555X:
      pix->colorspace = V4L2_COLORSPACE_SRGB;
      break;
  }
  return 0;
}


/*
 * ioctl_s_fmt_cap - V4L2 sensor interface handler for VIDIOC_S_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 VIDIOC_S_FMT ioctl structure
 *
 * If the requested format is supported, configures the HW to use that
 * format, returns error code if format not supported or HW can't be
 * correctly configured.
 */
static int ioctl_s_fmt_cap(struct v4l2_int_device *s,
    struct v4l2_format *f)
{
  struct BF3703_sensor *sensor = s->priv;
  struct v4l2_pix_format *pix = &f->fmt.pix;
  int rval;

  DPRINTK_OV("pix.pixformat = %x ",pix->pixelformat);
  DPRINTK_OV("pix.width = %d ",pix->width);
  DPRINTK_OV("pix.height = %d ",pix->height);

  rval = ioctl_try_fmt_cap(s, f);
  if (rval)
    return rval;

  sensor->pix = *pix;
  sensor->width = sensor->pix.width;
  sensor->height = sensor->pix.height;
  DPRINTK_OV("sensor->pix.pixformat = %x ",sensor->pix.pixelformat);
  DPRINTK_OV("sensor->pix.width = %d ",sensor->pix.width);
  DPRINTK_OV("sensor->pix.height = %d ",sensor->pix.height);

  return 0;
}

/*
 * ioctl_g_fmt_cap - V4L2 sensor interface handler for ioctl_g_fmt_cap
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 v4l2_format structure
 *
 * Returns the sensor's current pixel format in the v4l2_format
 * parameter.
 */
static int ioctl_g_fmt_cap(struct v4l2_int_device *s,
    struct v4l2_format *f)
{
  struct BF3703_sensor *sensor = s->priv;

  DPRINTK_OV("====entring BF3703 ioctl_g_fmt_cap====");

  f->fmt.pix = sensor->pix;
  DPRINTK_OV("f->fmt.pix.pixformat = %x,V4L2_PIX_FMT_YUYV = %x",f->fmt.pix.pixelformat,V4L2_PIX_FMT_YUYV);
  DPRINTK_OV("f->fmt.pix.width = %d",f->fmt.pix.width);
  DPRINTK_OV("f->fmt.pix.height = %d ",f->fmt.pix.height);

  return 0;
}

/*
 * ioctl_g_parm - V4L2 sensor interface handler for VIDIOC_G_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_G_PARM ioctl structure
 *
 * Returns the sensor's video CAPTURE parameters.
 */
static int ioctl_g_parm(struct v4l2_int_device *s,
    struct v4l2_streamparm *a)
{
  struct BF3703_sensor *sensor = s->priv;
  struct v4l2_captureparm *cparm = &a->parm.capture;

  DPRINTK_OV("\n");

  if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
    return -EINVAL;

  memset(a, 0, sizeof(*a));
  a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  cparm->capability = V4L2_CAP_TIMEPERFRAME;
  cparm->timeperframe = sensor->timeperframe;

  return 0;
}

/*
 * ioctl_s_parm - V4L2 sensor interface handler for VIDIOC_S_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_S_PARM ioctl structure
 *
 * Configures the sensor to use the input parameters, if possible.  If
 * not possible, reverts to the old parameters and returns the
 * appropriate error code.
 */
static int ioctl_s_parm(struct v4l2_int_device *s,
    struct v4l2_streamparm *a)
{
  int rval = 0;
  struct BF3703_sensor *sensor = s->priv;
  struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
  struct v4l2_fract timeperframe_old;
  int desired_fps;
  timeperframe_old = sensor->timeperframe;
  sensor->timeperframe = *timeperframe;

  desired_fps = timeperframe->denominator / timeperframe->numerator;
  if ((desired_fps < BF3703_MIN_FPS) || (desired_fps > BF3703_MAX_FPS))
    rval = -EINVAL;

  if (rval)
    sensor->timeperframe = timeperframe_old;
  else
    *timeperframe = sensor->timeperframe;

  DPRINTK_OV("frame rate = %d",desired_fps);

  return rval;
}

/*
 * ioctl_g_priv - V4L2 sensor interface handler for vidioc_int_g_priv_num
 * @s: pointer to standard V4L2 device structure
 * @p: void pointer to hold sensor's private data address
 *
 * Returns device's (sensor's) private data area address in p parameter
 */
static int ioctl_g_priv(struct v4l2_int_device *s, void *p)
{
  struct BF3703_sensor *sensor = s->priv;

  return sensor->pdata->priv_data_set(s,p);
}



static int __BF3703_power_off_standby(struct v4l2_int_device *s,
    enum v4l2_power on)
{
  struct BF3703_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;
  int rval;

  rval = sensor->pdata->power_set(s, on);
  if (rval < 0) {
    v4l_err(client, "Unable to set the power state:  sensor\n");
    return rval;
  }

  sensor->pdata->set_xclk(s, 0);
  return 0;
}

static int BF3703_power_off(struct v4l2_int_device *s)
{
  return __BF3703_power_off_standby(s, V4L2_POWER_OFF);
}

static int BF3703_power_standby(struct v4l2_int_device *s)
{
  return __BF3703_power_off_standby(s, V4L2_POWER_STANDBY);
}



static int BF3703_power_on(struct v4l2_int_device *s)
{

  struct BF3703_sensor *sensor = s->priv;
  struct i2c_client *client = sensor->i2c_client;
  int rval;

//  sensor->pdata->set_xclk(s, xclk_current);

  rval = sensor->pdata->power_set(s, V4L2_POWER_ON);
  if (rval < 0) {
    v4l_err(client, "Unable to set the power state:  sensor\n");
    sensor->pdata->set_xclk(s, 0);
    return rval;
  }

  return 0;
}

/*
 * ioctl_s_power - V4L2 sensor interface handler for vidioc_int_s_power_num
 * @s: pointer to standard V4L2 device structure
 * @on: power state to which device is to be set
 *
 * Sets devices power state to requrested state, if possible.
 */
static int ioctl_s_power(struct v4l2_int_device *s, enum v4l2_power on)
{

  switch (on) {
   case V4L2_POWER_ON:
      DPRINTK_OV(">>>>>>>>>>>>BF3703_power_on_pre");
      if((!BF3703_set_power) && (!BF3703_capture_mode) && (!BF3703_after_capture)) 
      {  
       printk(KERN_ERR ">>>>>>>>>>>>BF3703_power_on");
        BF3703_power_on(s);
      yl_set_debug_enalbe(7, 0);
      yl_set_debug_enalbe(1, 0);
      printk(KERN_ERR "BF3703:>>>close debug log\n"); 
      }
      BF3703_configure(s);
      break;

    case V4L2_POWER_OFF:
      printk(KERN_ERR "ioctl_s_power:>>>>>>>>>>>>BF3703_power_off");
      BF3703_set_power = 0;
      BF3703_capture_mode = 0;
      BF3703_after_capture = 0;
      BF3703_power_off(s);

      break;

    case V4L2_POWER_STANDBY:
      DPRINTK_OV(">>>>>>>>>>>>BF3703_power_standby_pre");
      break;
    default:
      break;
  }
  current_power_state = on;
  return 0;
}


/*
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 *
 * Initialize the sensor device (call BF3703_configure())
 */
static int ioctl_init(struct v4l2_int_device *s)
{
  DPRINTK_OV("\n");  	
  return 0;
}

/**
 * ioctl_dev_exit - V4L2 sensor interface handler for vidioc_int_dev_exit_num
 * @s: pointer to standard V4L2 device structure
 *
 * Delinitialise the dev. at slave detach.  The complement of ioctl_dev_init.
 */
static int ioctl_dev_exit(struct v4l2_int_device *s)
{

  DPRINTK_OV("\n");  				
  return 0;
}

/**
 * ioctl_dev_init - V4L2 sensor interface handler for vidioc_int_dev_init_num
 * @s: pointer to standard V4L2 device structure
 *
 * Initialise the device when slave attaches to the master.  Returns 0 if
 * BF3703 device could be found, otherwise returns appropriate error.
 */
static int ioctl_dev_init(struct v4l2_int_device *s)
{
  struct BF3703_sensor *sensor = s->priv;
  struct i2c_client *c = sensor->i2c_client;
  int err;
  
  err = BF3703_power_on(s);
  if (err)
    return -ENODEV;

  err = BF3703_detect(c);
  

  if (err < 0) {
    printk(KERN_ERR "BF3703:Unable to detect sensor\n");
    /*
     * Turn power off before leaving this function
     * If not, CAM powerdomain will on
     */
    BF3703_power_off(s);
    return err;
  }

  sensor->ver = err;
  printk(KERN_ERR "BF3703: Chip ID 0x%X detected\n", sensor->ver);
  err = BF3703_power_off(s);
  if(err)
  {
    return -ENODEV;
  }

  return 0;
}

/**
 * ioctl_enum_framesizes - V4L2 sensor if handler for vidioc_int_enum_framesizes
 * @s: pointer to standard V4L2 device structure
 * @frms: pointer to standard V4L2 framesizes enumeration structure
 *
 * Returns possible framesizes depending on choosen pixel format
 **/
static int ioctl_enum_framesizes(struct v4l2_int_device *s,
    struct v4l2_frmsizeenum *frms)
{
  int ifmt;
//  DPRINTK_OV("index=%d\n",frms->index);
  for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) {
    if (frms->pixel_format == BF3703_formats[ifmt].pixelformat)
      break;
  }
  /* Is requested pixelformat not found on sensor? */
  if (ifmt == NUM_CAPTURE_FORMATS)
    return -EINVAL;

  /* Do we already reached all discrete framesizes? */
  if (frms->index >= ARRAY_SIZE(BF3703_sizes))
    return -EINVAL;

  frms->type = V4L2_FRMSIZE_TYPE_DISCRETE;
  frms->discrete.width = BF3703_sizes[frms->index].width;
  frms->discrete.height = BF3703_sizes[frms->index].height;

//  DPRINTK_OV("discrete(%d,%d)\n",frms->discrete.width,frms->discrete.height);
  return 0;
}

/*QCIF,QVGA,CIF,VGA,SVGA can support up to 30fps*/
const struct v4l2_fract BF3703_frameintervals[] = {
  { .numerator = 1, .denominator = 15 },//QQVGA,160*120
  { .numerator = 1, .denominator = 15 },//QCIF,176*144
  { .numerator = 1, .denominator = 15 },//QVGA,320*240
  { .numerator = 1, .denominator = 15 },//CIF,352*288
  { .numerator = 1, .denominator = 15 },//VGA,640*480
};

static int ioctl_enum_frameintervals(struct v4l2_int_device *s,
    struct v4l2_frmivalenum *frmi)
{
  int ifmt;
  int isize = ARRAY_SIZE(BF3703_frameintervals) - 1;

  for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) {
    if (frmi->pixel_format == BF3703_formats[ifmt].pixelformat)
      break;
  }

//  DPRINTK_OV("frmi->width = %d,frmi->height = %d,frmi->index = %d,ifmt = %d\n", 
  //    frmi->width,frmi->height,frmi->index,ifmt);

  /* Is requested pixelformat not found on sensor? */
  if (ifmt == NUM_CAPTURE_FORMATS)
    return -EINVAL;

  /* Do we already reached all discrete framesizes? */
  if (frmi->index > isize)
    return -EINVAL;


  frmi->type = V4L2_FRMIVAL_TYPE_DISCRETE;
  frmi->discrete.numerator =
    BF3703_frameintervals[frmi->index].numerator;
  frmi->discrete.denominator =
    BF3703_frameintervals[frmi->index].denominator;

  return 0;

}

static struct v4l2_int_ioctl_desc bf3703_ioctl_desc[] = {
	{ .num = vidioc_int_enum_framesizes_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_enum_framesizes},
	{ .num = vidioc_int_enum_frameintervals_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_enum_frameintervals},
	{ .num = vidioc_int_dev_init_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_dev_init},
	{ .num = vidioc_int_dev_exit_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_dev_exit},
	{ .num = vidioc_int_s_power_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_power },
	{ .num = vidioc_int_g_priv_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_priv },
	{ .num = vidioc_int_init_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_init },
	{ .num = vidioc_int_enum_fmt_cap_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_enum_fmt_cap },
	{ .num = vidioc_int_try_fmt_cap_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_try_fmt_cap },
	{ .num = vidioc_int_g_fmt_cap_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_fmt_cap },
	{ .num = vidioc_int_s_fmt_cap_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_fmt_cap },
	{ .num = vidioc_int_g_parm_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_parm },
	{ .num = vidioc_int_s_parm_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_parm }, 
	{ .num = vidioc_int_queryctrl_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_queryctrl },
	{ .num = vidioc_int_g_ctrl_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_ctrl },
	{ .num = vidioc_int_s_ctrl_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_s_ctrl },
	{ .num = vidioc_int_g_crop_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_g_crop },
	{ .num = vidioc_int_s_crop_num,
	
	  .func = (v4l2_int_ioctl_func *)ioctl_s_crop },
	{ .num = vidioc_int_cropcap_num,
	  .func = (v4l2_int_ioctl_func *)ioctl_cropcap },
//	{ .num = vidioc_int_priv_g_pixclk_num,
//	  .func = (v4l2_int_ioctl_func *)ioctl_priv_g_pixclk },
//	{ .num = vidioc_int_priv_g_activesize_num,
//	  .func = (v4l2_int_ioctl_func *)ioctl_priv_g_activesize },
//	{ .num = vidioc_int_priv_g_fullsize_num,
//	  .func = (v4l2_int_ioctl_func *)ioctl_priv_g_fullsize },
//	{ .num = vidioc_int_priv_g_pixelsize_num,
//	  .func = (v4l2_int_ioctl_func *)ioctl_priv_g_pixelsize },
};

static struct v4l2_int_slave bf3703_slave = {
	.ioctls = bf3703_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(bf3703_ioctl_desc),
};

static struct v4l2_int_device bf3703_int_device = {
	.module = THIS_MODULE,
	.name = BF3703_DRIVER_NAME,
	.priv = &bf3703,
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &bf3703_slave,
	},
};

/**
 * bf3703_probe - sensor driver i2c probe handler
 * @client: i2c driver client device structure
 *
 * Register sensor as an i2c client device and V4L2
 * device.
 */
static int __devinit bf3703_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct BF3703_sensor *sensor = &bf3703;
	int err;

	if (i2c_get_clientdata(client))
		return -EBUSY;

	sensor->pdata = client->dev.platform_data;

	if (!sensor->pdata) {
		v4l_err(client, "no platform data?\n");
		return -ENODEV;
	}

	sensor->v4l2_int_device = &bf3703_int_device;
	sensor->i2c_client = client;

	i2c_set_clientdata(client, sensor);	


  /* Make the default preview format 640*480 BF3703_YUV */
  sensor->pix.width = BF3703_sizes[BF3703_VGA].width;
  sensor->pix.height = BF3703_sizes[BF3703_VGA].height;

  sensor->pix.pixelformat = V4L2_PIX_FMT_YUYV;

	err = v4l2_int_device_register(sensor->v4l2_int_device);
	if (err)
		i2c_set_clientdata(client, NULL);

	return 0;
}

/**
 * bf3703_remove - sensor driver i2c remove handler
 * @client: i2c driver client device structure
 *
 * Unregister sensor as an i2c client device and V4L2
 * device.  Complement of bf3703_probe().
 */
static int __exit
bf3703_remove(struct i2c_client *client)
{
	struct BF3703_sensor *sensor = i2c_get_clientdata(client);

	if (!client->adapter)
		return -ENODEV;	/* our client isn't attached */

	v4l2_int_device_unregister(sensor->v4l2_int_device);
	i2c_set_clientdata(client, NULL);

	return 0;
}

static const struct i2c_device_id bf3703_id[] = {
	{ BF3703_DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, bf3703_id);

static struct i2c_driver bf3703sensor_i2c_driver = {
	.driver = {
		.name = BF3703_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = bf3703_probe,
	.remove = __exit_p(bf3703_remove),
	.id_table = bf3703_id,
};

static struct BF3703_sensor bf3703 = {
	.timeperframe = {
		.numerator = 1,
		.denominator = 15,
	},
};

/**
 * bf3703sensor_init - sensor driver module_init handler
 *
 * Registers driver as an i2c client driver.  Returns 0 on success,
 * error code otherwise.
 */
static int __init bf3703sensor_init(void)
{
	int err;

	err = i2c_add_driver(&bf3703sensor_i2c_driver);
	if (err) {
		DPRINTK_OV("Failed to register");
		return err;
	}
	return 0;
}
late_initcall(bf3703sensor_init);    

/**
 * bf3703sensor_cleanup - sensor driver module_exit handler
 *
 * Unregisters/deletes driver as an i2c client driver.
 * Complement of bf3703sensor_init.
 */
static void __exit bf3703sensor_cleanup(void)
{
	i2c_del_driver(&bf3703sensor_i2c_driver);
}
module_exit(bf3703sensor_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("bf3703 camera sensor driver");


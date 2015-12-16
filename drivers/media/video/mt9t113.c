/*
 * drivers/media/video/mt9t113.c
 *
 * mt9t113 sensor driver 
 *
 * Leveraged code from the mt9t113.c 
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 * modified :daqing
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <media/v4l2-int-device.h>
//#include <linux/i2c/twl4030.h>
#include <mach/gpio.h>
#include "omap34xxcam.h"
#include "isp/isp.h"

#include "../../../arch/arm/mach-omap2/board-cp5860e-camera.h"
#include "mt9t113.h"

//#include "adp1650_flashlight.h"
#include "adp1650c.h"

#define MT9T113_DRIVER_NAME  "mt9t113"
#define MOD_NAME "MT9T113: "

#define I2C_M_WR 0
#define MT9T113_YUV_MODE 1

//#define MT9T113_DEBUG 1

#ifdef MT9T113_DEBUG
#define DPRINTK_OV(format, ...)				\
  printk(KERN_ERR "MT9T113:%s(): " format " ",__func__, ## __VA_ARGS__)
#else
#define DPRINTK_OV(format, ...)
#endif

#define SENSOR_IS_MT9T113  0
#define SENSOR_IS_S5K5CA    1

#define I2C_BURST_MODE 1

#define CHECK_TIME 120

//#define CONFIG_LOAD_FILE 1

static struct mt9t113_sensor mt9t113;
static struct i2c_driver mt9t113sensor_i2c_driver;
static struct i2c_client *mt9t113_i2c_client;
static enum v4l2_power current_power_state;

static int mt9t113_capture_mode = 0;
static int mt9t113_after_capture = 0; 
static int mt9t113_set_power = 0;

int sensor_version = 1;     //liuli add 2011-9-20
int sensor_byd_kerr = 0; 

static int test_mode = 0; 
static int load_path = 0;

static int camera_video = 0;

u16 exp_stab = 0;

extern void yl_set_debug_enalbe(int module_name, int value);

static int mt9t113_write_reg(struct i2c_client *client, u16 reg, u16 val) ;
static int mt9t113_power_off(struct v4l2_int_device *s);
static int mt9t113_read_reg(struct i2c_client *client, u16 data_length, u16 reg, u16 *val);

#ifdef CONFIG_LOAD_FILE

#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include <asm/uaccess.h>

static char *s5k5ca_regs_table = NULL;

static int s5k5ca_regs_table_size;

static int s5k5ca_regs_table_init(void)
{
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
	int i;
	int ret;
	mm_segment_t fs = get_fs();

	printk(KERN_ERR "%s %d\n", __func__, __LINE__);

	set_fs(get_ds());
   if(1){
        printk(KERN_ERR "load path = /data/mt9t113.txt");
	filp = filp_open("/data/mt9t113.txt", O_RDONLY, 0);
   }else{
        printk(KERN_ERR "load path = /mnt/sdcard/mt9t113.txt");
	filp = filp_open("/mnt/sdcard/mt9t113.txt", O_RDONLY, 0);
   }

	if (IS_ERR(filp)) {
		printk(KERN_ERR "file open error\n");
		return -1;
	}
	l = filp->f_path.dentry->d_inode->i_size;	
	printk(KERN_ERR "l = %ld\n", l);
	dp = kmalloc(l, GFP_KERNEL);
	if (dp == NULL) {
		printk(KERN_ERR "Out of Memory\n");
		filp_close(filp, current->files);
                return -1;
	}
	pos = 0;
	memset(dp, 0, l);
	ret = vfs_read(filp, (char __user *)dp, l, &pos);
	if (ret != l) {
		printk(KERN_ERR "Failed to read file ret = %d\n", ret);
		kfree(dp);
		filp_close(filp, current->files);
		return -1;
	}

	filp_close(filp, current->files);
	
	set_fs(fs);

	s5k5ca_regs_table = dp;
	
	s5k5ca_regs_table_size = l;

	*((s5k5ca_regs_table + s5k5ca_regs_table_size) - 1) = '\0';

	printk(KERN_ERR "s5k5ca_regs_table 0x%08x, %ld\n", dp, l);
      
        return 0;
}

void s5k5ca_regs_table_exit(void)
{
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
	if (s5k5ca_regs_table) {
		kfree(s5k5ca_regs_table);
		s5k5ca_regs_table = NULL;
	}	
}

static int s5k5ca_regs_table_write(char *name)
{
	char *start, *end, *reg, *data;	
	unsigned short addr, value;
	char reg_buf[7], data_buf[7];
        u16 tmp1, tmp2;
        u16 tmp_addr;

	*(reg_buf + 6) = '\0';
	*(data_buf + 6) = '\0';

	start = strstr(s5k5ca_regs_table, name);

	end = strstr(start, "};");
       
	while (1) {
                tmp1 = 0;
                tmp2  = 0;	
		/* Find Address */	
		reg = strstr(start,"{0x");		
		if (reg)
			start = (reg + 15);
		if ((reg == NULL) || (reg > end))
                {
                    printk(KERN_ERR "start = 0x%08x, end = 0x%08x\n", start, end);
		    break;
                }
		/* Write Value to Address */	
		if (reg != NULL) {
			memcpy(reg_buf, (reg + 1), 6);	
			memcpy(data_buf, (reg + 9), 6);	
			addr = (unsigned short)simple_strtoul(reg_buf, NULL, 16); 
			value = (unsigned short)simple_strtoul(data_buf, NULL, 16); 
			//printk(KERN_ERR "{0x%04X, 0x%04X},\n", addr, value);
				
			if (addr == 0x0000)
			{
		            mdelay(value);
		           // printk(KERN_ERR"delay 0x%04x, value 0x%04x\n", addr, value);
                           continue;		
			}	
			else if(addr == 0xFFFF)	
                        {
                           // printk(KERN_ERR "skip 0x%04x, value 0x%04x\n", addr, value);
                           continue;
                        }
			else
                        {		
			    mt9t113_write_reg(mt9t113_i2c_client, addr, value); //write regs

                           if(addr == 0x098E)//read regs
                            {                                
                                 printk(KERN_ERR "{0x%04X, 0x%04X},\n", addr, value);
                                 tmp_addr = value;
                            }  
                           else if(addr == 0x0990) 
                            {
                                 if(tmp_addr == 0x8400)
                                 { 
                                     mdelay(200);

                                 }
                                 mt9t113_write_reg(mt9t113_i2c_client, 0x098E, tmp_addr);
                                 mt9t113_read_reg(mt9t113_i2c_client, 2, 0x0990, &tmp1);                     
                                 printk(KERN_ERR "{0x%04X, 0x%04X},<\n", addr, tmp1);
                            }                        
                            else
                            {
                                 mt9t113_read_reg(mt9t113_i2c_client, 2, addr, &tmp2);
                                 printk(KERN_ERR "{0x%04X, 0x%04X},<\n", addr, tmp2);
                            }
                        }
		}
	}

	return 0;
}

#endif


extern struct device *omap_isp_device;
extern struct isp_reg isp_reg_list[]; 

/* List of image formats supported by MT9T113sensor */
const static struct v4l2_fmtdesc mt9t113_formats[] = {
#if defined(MT9T113_RAW_MODE)
    {
      .description	= "RAW10",
      .pixelformat	= V4L2_PIX_FMT_SGRBG10,
    },
#elif defined(MT9T113_YUV_MODE)
    {
      .description	= "YUYV,422",
      .pixelformat	= V4L2_PIX_FMT_UYVY,       
    },
#else    
    {
      /* Note:  V4L2 defines RGB565 as:
       *
       *	Byte 0			  Byte 1
       *	g2 g1 g0 r4 r3 r2 r1 r0	  b4 b3 b2 b1 b0 g5 g4 g3
       *
       * We interpret RGB565 as:
       *
       *	Byte 0			  Byte 1
       *	g2 g1 g0 b4 b3 b2 b1 b0	  r4 r3 r2 r1 r0 g5 g4 g3
       */
      .description	= "RGB565, le",
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
      .description	= "RGB565, be",
      .pixelformat	= V4L2_PIX_FMT_RGB565X,
    },
    {
      .description	= "YUYV (YUV 4:2:2), packed",
      .pixelformat	= V4L2_PIX_FMT_YUYV,
    },
    {
      .description	= "UYVY, packed",
      .pixelformat	= V4L2_PIX_FMT_UYVY,
    },
    {
      /* Note:  V4L2 defines RGB555 as:
       *
       *	Byte 0			  Byte 1
       *	g2 g1 g0 r4 r3 r2 r1 r0	  x  b4 b3 b2 b1 b0 g4 g3
       *
       * We interpret RGB555 as:
       *
       *	Byte 0			  Byte 1
       *	g2 g1 g0 b4 b3 b2 b1 b0	  x  r4 r3 r2 r1 r0 g4 g3
       */
      .description	= "RGB555, le",
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
      .description	= "RGB555, be",
      .pixelformat	= V4L2_PIX_FMT_RGB555X,
    },
#endif
  };

#define NUM_CAPTURE_FORMATS (sizeof(mt9t113_formats) / sizeof(mt9t113_formats[0]))

/* register initialization tables for mt9t113 */
#define MT9T113_REG_TERM 0xFFFF	/* terminating list entry for reg */
#define MT9T113_VAL_TERM 0xFF	/* terminating list entry for val */

/*
 * struct vcontrol - Video controls
 * @v4l2_queryctrl: V4L2 VIDIOC_QUERYCTRL ioctl structure
 * @current_value: current value of this control
 */
static struct vcontrol {
    struct v4l2_queryctrl qc;
    int current_value;
} video_control[] = {
#ifdef MT9T113_YUV_MODE
    {
      {
        .id = V4L2_CID_EXPOSURE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "Exposure",
        .minimum = MT9T113_MIN_EXPOSURE,
        .maximum = MT9T113_MAX_EXPOSURE,
        .step = MT9T113_EXPOSURE_STEP,
        .default_value = MT9T113_DEF_EXPOSURE,
      },
      .current_value = MT9T113_DEF_EXPOSURE,
    },
    {
      {
        .id = V4L2_CID_BRIGHTNESS,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "Brightness",
        .minimum = MT9T113_MIN_BRIGHT,
        .maximum = MT9T113_MAX_BRIGHT,
        .step = MT9T113_BRIGHT_STEP,
        .default_value = MT9T113_DEF_BRIGHT,
      },
      .current_value = MT9T113_DEF_BRIGHT,
    },
    {
      {
        .id = V4L2_CID_CONTRAST,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "Contrast",
        .minimum = MT9T113_MIN_CONTRAST,
        .maximum = MT9T113_MAX_CONTRAST,
        .step = MT9T113_CONTRAST_STEP,
        .default_value = MT9T113_DEF_CONTRAST,
      },
      .current_value = MT9T113_DEF_CONTRAST,
    },             
    {
      {
        .id = V4L2_CID_PRIVATE_BASE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "Color Effects",
        .minimum = MT9T113_MIN_COLOR,
        .maximum = MT9T113_MAX_COLOR,
        .step = MT9T113_COLOR_STEP,
        .default_value = MT9T113_DEF_COLOR,
      },
      .current_value = MT9T113_DEF_COLOR,
    },
    {
      {
        .id = V4L2_CID_DO_WHITE_BALANCE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "white balance",
        .minimum = MT9T113_MIN_WB,
        .maximum = MT9T113_MAX_WB,
        .step = MT9T113_WB_STEP,
        .default_value = MT9T113_DEF_WB,
      },
      .current_value = MT9T113_DEF_WB,
    },
    {
      {
        .id = V4L2_CID_ZOOM_RELATIVE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "zoom",
        .minimum = MT9T113_MIN_ZOOM,
        .maximum = MT9T113_MAX_ZOOM,
        .step = MT9T113_ZOOM_STEP,
        .default_value = MT9T113_DEF_ZOOM,
      },
      .current_value = MT9T113_DEF_ZOOM,
    }
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

/*
 * Read a value from a register in mt9t113 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9t113_read_reg(struct i2c_client *client, u16 data_length, u16 reg, u16 *val)
{
    int err = 0;
    struct i2c_msg msg[1];
    unsigned char data[2];
    
    if (!client->adapter)
        return -ENODEV;
    
    msg->addr = client->addr;
    msg->flags = I2C_M_WR;
    msg->len = 2;
    msg->buf = data;
    
    /* High byte goes out first */
    data[0] = (u8) (reg >> 8);
    data[1] = (u8) (reg & 0xff);
       
    err = i2c_transfer(client->adapter, msg, 1);
  
    if (err >= 0) {
        mdelay(3);
        msg->flags = I2C_M_RD;
        msg->len = data_length;
        err = i2c_transfer(client->adapter, msg, 1);
    }
   
    if (err >= 0) {
        *val = 0;
        /* High byte comes first */
        if (data_length == 1)
            *val = data[0];
        else if (data_length == 2)
            *val = data[1] + (data[0] << 8);
        else{
            DPRINTK_OV("read register error");
        }
        return 0;
    }
    dev_err(&client->dev, "read from register 0x%X error = %d\n", reg, err);
    return err;
}

/* Write a value to a register in mt9t113 sensor device.
 * @client: i2c driver client structure.
 * @reg: Address of the register to read value from.
 * @val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9t113_write_reg(struct i2c_client *client, u16 reg, u16 val)   //u8
{
    int err = 0;
    struct i2c_msg msg[1];
    unsigned char data[4]; 
    int retries = 0;
    
    if (!client->adapter)
        return -ENODEV;
retry:
    msg->addr = client->addr;
    msg->flags = I2C_M_WR;
    msg->len = 4; 
    msg->buf = data;
    
    /* high byte goes out first */
    data[0] = (u8) (reg >> 8);
    data[1] = (u8) (reg & 0xff);
    data[2] = (u8) (val >> 8);
    data[3] = (u8) (val & 0xff);
    
    err = i2c_transfer(client->adapter, msg, 1);
    //udelay(50);
    
    if (err >= 0)
        return 0;
    
    if (retries < 2) {
        dev_err(&client->dev, "write to offset 0x%x error %d", reg, err);
        DPRINTK_OV( "Retrying I2C... %d", retries);
        retries++;
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule_timeout(msecs_to_jiffies(20));
        goto retry;
    }
    
    return err;
}

/*
 * Initialize a list of mt9t113 registers.
 * The list of registers is terminated by the pair of values
 * @client: i2c driver client structure.
 * @reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int mt9t113_write_regs(struct i2c_client *client,
    const struct mt9t113_reg reglist[])
{
    int err = 0;
    //u32 valu = 0;
    const struct mt9t113_reg *next = reglist;

    while (!((next->reg == MT9T113_REG_TERM)
          && (next->val == MT9T113_VAL_TERM))) {

    if(next->reg == 0x0000){
      mdelay(next->val);
    }else{
        err = mt9t113_write_reg(client, next->reg, next->val);
    }
    //mt9t113_read_reg(client, 2, next->reg, &valu);
    //printk(KERN_ERR"address is:%0x ,val %0x, = %0x \n",next->reg,next->val,valu);   
    if (err)
      return err;
    next++;
  }
  return 0;
}

#ifdef I2C_BURST_MODE //dha23 100325

#define BURST_MODE_SET			1
#define BURST_MODE_END			2
#define NORMAL_MODE_SET			3
#define MAX_INDEX			1000
static int s5k5ca_sensor_burst_write_list(struct i2c_client *client, const struct mt9t113_reg *list, char *name)
{
    __u8 temp_buf[MAX_INDEX];
	int index_overflow = 1;
	int new_addr_start = 0;
	int burst_mode = NORMAL_MODE_SET;
	unsigned short pre_reg = 0;
	struct i2c_msg msg = { client->addr, 0, 4, temp_buf };
	int i=0, ret=0;
	unsigned int index = 0;
	
	//printk("s5k5ca_sensor_burst_write_list( %s ) \n", name); 
	//printk("[PGH] on write func s5k5ca_client->addr : %x\n", client->addr); //reduced startup time.     
#if 1
	//s5k5ca_regs_table_write(client, name);	
//#else

	for (i = 0; list[i].reg != 0xffff; i++)
	{
		if(list[i].reg == 0x0000)
		{
                        msleep(list[i].val);
			DPRINTK_OV("delay 0x%04x, val 0x%04x", list[i].reg, list[i].val);
		}	
		else
		{					
			if( list[i].reg == list[i+1].reg )
			{
				burst_mode = BURST_MODE_SET;
				if((list[i].reg != pre_reg) || (index_overflow == 1))
				{
					new_addr_start = 1;
					index_overflow = 0;
				}
			}
			else
			{
				if(burst_mode == BURST_MODE_SET)
				{
					burst_mode = BURST_MODE_END;
					if(index_overflow == 1)
					{
						new_addr_start = 1;
						index_overflow = 0;
					}
				}
				else
				{
					burst_mode = NORMAL_MODE_SET;
				}
			}

			if((burst_mode == BURST_MODE_SET) || (burst_mode == BURST_MODE_END))
			{
				if(new_addr_start == 1)
				{
					index = 0;
					memset(temp_buf, 0x00 ,1000);
					index_overflow = 0;

					temp_buf[index] = (list[i].reg >> 8);
					temp_buf[++index] = (list[i].reg & 0xFF);

					new_addr_start = 0;
				}
				
				temp_buf[++index] = (list[i].val >> 8);
				temp_buf[++index] = (list[i].val & 0xFF);
				
				if(burst_mode == BURST_MODE_END)
				{
					msg.len = ++index;

					ret = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
					if( ret < 0)
					{
						DPRINTK_OV("i2c_transfer fail ! ");
						return -1;
					}
				}
				else if( index >= MAX_INDEX-1 )
				{
					index_overflow = 1;
					msg.len = ++index;
					
					ret = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
					if( ret < 0)
					{
						DPRINTK_OV("I2C_transfer Fail !");
						return -1;
					}
				}
				
			}
			else
			{
				memset(temp_buf, 0x00 ,4);
			
				temp_buf[0] = (list[i].reg >> 8);
				temp_buf[1] = (list[i].reg & 0xFF);
				temp_buf[2] = (list[i].val >> 8);
				temp_buf[3] = (list[i].val & 0xFF);

				msg.len = 4;
				ret = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
				if( ret < 0)
				{
					DPRINTK_OV("I2C_transfer Fail !");
					return -1;
				}
			}
		}
		
		pre_reg = list[i].reg;
	}
#endif
	return ret;
}

#endif

/* Find the best match for a requested image capture size.  The best match
 * is chosen as the nearest match that has the same number or fewer pixels
 * as the requested size, or the smallest image size if the requested size
 * has fewer pixels than the smallest image.
 */
static enum image_size_ov
    mt9t113_find_size(unsigned int width, unsigned int height)
{

    enum image_size_ov isize;

    for (isize = 0; isize < ARRAY_SIZE(mt9t113_sizes); isize++) {
      if ((mt9t113_sizes[isize].height >= height) &&
          (mt9t113_sizes[isize].width >= width)) {
        break;
      }
    }
    DPRINTK_OV("width = %d,height = %d,return %d",width,height,isize);
    return isize;
}

//read status to check
static int check_status(struct i2c_client *client, u16 value)
{
    int check_time = 0;
    u16 tmp;
    u16 check_val = value;
    do{
         mt9t113_write_reg(client, 0x098E, 0x8400);
         mt9t113_read_reg(client, 2, 0x0990, &tmp);
         if(!(tmp & check_val)) break;
          check_time++;
    } while(check_time < CHECK_TIME);
    
    return 0;
}

/*
 * Configure the mt9t113 for a specified image size, pixel format, and frame
 * period.  xclk is the frequency (in Hz) of the xclk input to the MT9T113.
 * fper is the frame period (in seconds) expressed as a fraction.
 * Returns zero if successful, or non-zero otherwise.
 * The actual frame period is returned in fper.
 */
static int mt9t113_configure(struct v4l2_int_device *s)
{
    struct mt9t113_sensor *sensor = s->priv;
    struct v4l2_pix_format *pix = &sensor->pix;
    struct i2c_client *client = sensor->i2c_client;
    enum image_size_ov isize = ARRAY_SIZE(mt9t113_sizes) - 1;
    struct sensor_ext_params *extp = &(sensor->ext_params);

    //u16 sharp_val;
    //u16 satu_val;
    //u16 exp_val;
    int  err = 0;
    int check_time = 0;
    u16 tmp;
    
    enum pixel_format_ov pfmt = YUV;
    switch (pix->pixelformat) {
    
        case V4L2_PIX_FMT_RGB565:
        case V4L2_PIX_FMT_RGB565X:
            pfmt = RGB565;
            break;
    
        case V4L2_PIX_FMT_RGB555:
        case V4L2_PIX_FMT_RGB555X:
            pfmt = RGB555;
            break;
        
        case V4L2_PIX_FMT_SGRBG10:
            pfmt = RAW10;
            break;
        
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
        default:
            pfmt = YUV;
    }
      
    isize = mt9t113_find_size(pix->width, pix->height);
    DPRINTK_OV("=============isize =%d ", isize);
    DPRINTK_OV("sensor_version = %d",sensor_version);
    DPRINTK_OV(">>>>>>>test_mode = %d", test_mode); 
	if(sensor_version == SENSOR_IS_MT9T113){    
    	if(mt9t113_capture_mode){
                
               DPRINTK_OV(">>>>>mt9t113 go to capture ");      
               mt9t113_write_regs(client, mt9t113_svga_to_xxx[isize]);
               check_status(client, 0x0006);

               mt9t113_write_regs(client, mt9t113_for_capture); 
               check_status(client, 0x0002);
       
        	mt9t113_capture_mode = 0;
        	mt9t113_after_capture = 1;
              mdelay(60);
    	}
    	else if((!mt9t113_after_capture) && (!mt9t113_set_power))
    	{
            DPRINTK_OV( ">>>>>mt9t113 coming to preview initialization !");

            #ifdef CONFIG_LOAD_FILE
              if(!s5k5ca_regs_table_init()){
                   printk(KERN_ERR ">>>>>parameter table prepare ok!\n");
               }
            #endif
               mt9t113_write_regs(client, mt9t113_common_svgabase);  
            

               mt9t113_write_regs(client, mt9t113_xxx_preview[isize]);
               check_status(client, 0x0006);
            
        	mt9t113_write_regs(client, mt9t113_effects[extp->effect]);

         	mt9t113_write_regs(client, mt9t113_whitebalance[extp->white_balance]);

               if(isize == DVD_NTSC){
                   mt9t113_write_regs(client,mt9t113_20fps);
               }else if(isize == QCIF){
            	   mt9t113_write_regs(client,mt9t113_15fps);
               }else{
                   mt9t113_write_regs(client,mt9t113_auto_fps);
               }
               check_status(client, 0x0006);

        	mt9t113_write_regs(client, mt9t113_for_preview);
               check_status(client, 0x0001);

               mt9t113_write_reg(client, 0x098E, 0x8400);
               mt9t113_write_reg(client, 0x0990, 0x0005);
               check_status(client, 0x0005);
              mdelay(20);
        	
    	}
    	else
    	{     
               DPRINTK_OV( ">>>>>mt9t113 coming to preview !"); 

        	mt9t113_write_regs(client, mt9t113_for_preview);
                check_status(client, 0x0001);
	     	
                mt9t113_write_regs(client, mt9t113_xxx_preview[isize]);
                check_status(client, 0x0006);
            
             if(extp->exposure <= 0 || extp->exposure > ARRAY_SIZE(mt9t113_exposure))
                  extp->exposure = MT9T113_DEF_EXPOSURE;
             mt9t113_write_regs(client, mt9t113_exposure[extp->exposure - 1]);
            DPRINTK_OV("extp->exposure = %d",extp->exposure); 
               if(isize == DVD_NTSC){
                   mt9t113_write_regs(client,mt9t113_20fps);
               }else if(isize == QCIF){
            	   mt9t113_write_regs(client,mt9t113_15fps);
               }else{
                   mt9t113_write_regs(client,mt9t113_auto_fps);
               }
               check_status(client, 0x0006);

             DPRINTK_OV("extp->zoom = %d",extp->zoom); 
             if(!mt9t113_after_capture)
             {                  
                  if(extp->zoom != MT9T113_DEF_ZOOM)
                  {
                      mt9t113_write_regs(client, mt9t113_zoom[MT9T113_DEF_ZOOM]);
                      check_time = 0;
                      do{
                         mt9t113_write_reg(client, 0x098E, 0x8404);
                         mt9t113_read_reg(client, 2, 0x0990, &tmp);
                         if(!(tmp & 0x0006)) break;
                         check_time++;
                      } while(check_time < CHECK_TIME);
                      printk(KERN_ERR"mt9t113:>>>>do zoom\n");
                  }
             }

               mt9t113_write_reg(client, 0x098E, 0x8400);
               mt9t113_write_reg(client, 0x0990, 0x0005);
               check_status(client, 0x0005);

        	mt9t113_after_capture = 0;       	    	      
    	}

    }
    else
    {    
        if(mt9t113_capture_mode)
        {                      
            DPRINTK_OV( "********s5k5ca go to capture !***********"); 

            mt9t113_write_regs(client, s5k5cag_xxx_capture[isize]);
          
            mt9t113_capture_mode = 0;
            mt9t113_after_capture = 1;
        }
        else if((!mt9t113_after_capture) && (!mt9t113_set_power))
        {          
            DPRINTK_OV( "********s5k5ca first preview !***********"); 

           //mt9t113_write_regs(client, s5k5cag_common_svgabase); 
            s5k5ca_sensor_burst_write_list(client, s5k5cag_common_svgabase,"s5k5cag_common_svgabase");          
           
            //sharpness to reduce noise
            mt9t113_write_regs(client, s5k5cag_sharpness_default);

            mt9t113_write_regs(client, s5k5cag_effects[extp->effect]);

            mt9t113_write_regs(client, s5k5cag_xxx_preview[isize]); 
            mdelay(10);
            mt9t113_write_regs(client, s5k5cag_whitebalance[extp->white_balance]);   
            mdelay(100);

          /*
           while(check_time < 16)//for exposure stable
           {
               mt9t113_write_reg(client, 0x002C, 0x7000);
               mt9t113_write_reg(client, 0x002E, 0x247C);
               mt9t113_read_reg(client, 2, 0x0F12, &exp_stab);
               printk(KERN_ERR ">>>>>>>exp_stab value = %0X", exp_stab); 
               if(exp_stab)
               {
                  exp_stab = 0;
                  break;
               };
              check_time++;
           }
           */
         }
         else
         {
             DPRINTK_OV( "********s5k5ca go to preview !***********"); 

             mt9t113_write_regs(client, s5k5cag_xxx_preview[isize]);
             mdelay(10);
          
            if(extp->exposure <= 0 || extp->exposure > ARRAY_SIZE(s5k5cag_exposure))
               extp->exposure = MT9T113_DEF_EXPOSURE;  
            mt9t113_write_regs(client, s5k5cag_exposure[extp->exposure - 1]); 
           
             DPRINTK_OV("extp->zoom = %d",extp->zoom); 
             if(!mt9t113_after_capture)
             {   
                  if(extp->zoom != MT9T113_DEF_ZOOM)
                  {
                     mt9t113_write_regs(client, s5k5cag_zoom[MT9T113_DEF_ZOOM]);
                  }
             }

             mt9t113_after_capture = 0;       
 
         }

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

/* Detect if an mt9t113 is present, returns a negative error number if no
 * device is detected, or pidl as version number if a device is detected.
 */
static int mt9t113_detect(struct i2c_client *client)
{
    u16 pid = 0;
    DPRINTK_OV("\n");
    
    if (!client)
        return -ENODEV;
    
    if (mt9t113_read_reg(client, 2, MT9T113_ID_ADDR, &pid))
        return -ENODEV;
    
    DPRINTK_OV( "Read MT9T113 chip id: (0x%04X)", pid);

    if (pid == MT9T113_CHIP_ID) 
    {
        DPRINTK_OV("Detect success (0x%04X)", pid);
        return pid;
    }
    else
    {   
        pid = 0;    
        if(mt9t113_read_reg(client, 2, S5K5CA_ID_ADDR, &pid))
	{
            return -ENODEV;
        }

        DPRINTK_OV( "Read S5K5CA chip id: (0x%04X)", pid);  

        if (pid == S5K5CA_CHIP_ID) 
        {
            DPRINTK_OV("Detect success (0x%04X)", pid);
            return pid;
        }

    }
   
    return -ENODEV;
}

 int is_low_light(void)
{
   int is_low_light = 0;
   u16 cur_light;

   if (!mt9t113_i2c_client->adapter)
        return -ENODEV;	/* our client isn't attached */

if(sensor_version == SENSOR_IS_MT9T113)
{
   mt9t113_write_reg(mt9t113_i2c_client, 0x098e, 0xA80B);
   mt9t113_read_reg(mt9t113_i2c_client, 2, 0x0990, &cur_light);
   DPRINTK_OV(KERN_ERR "MT9T113: light value = 0x%04X", cur_light); 
   printk(KERN_ERR "Camera Marked(SUNNY)--->>>MT9T113:Found sensor MT9T113!\n");
   
  if(cur_light <= 0x0022)
  {
      printk(KERN_ERR "MT9T113: low light case, open flashlight"); 
      is_low_light = 1;
  }
  else
  {      
      is_low_light = 0;
  }
}
else
{
   mt9t113_write_reg(mt9t113_i2c_client, 0x002C, 0x7000);
   mt9t113_write_reg(mt9t113_i2c_client, 0x002E, 0x2448);
   mt9t113_read_reg(mt9t113_i2c_client, 2, 0x0F12, &cur_light);
   DPRINTK_OV("S5K5CA: light value = 0X%04X", cur_light); 
   printk(KERN_ERR "Camera Marked(BYD)--->>>S5K5CA:Found sensor S5K5CA!\n");
   
  if(cur_light >= 0 && cur_light <= 0x0030 )
  {
      printk(KERN_ERR "S5K5CA: low light case, open flashlight"); 
      is_low_light = 1;
  }
  else
  {      
      is_low_light = 0;
  }
}

  return is_low_light;
}
/* To get the cropping capabilities of mt9t113 sensor
 * Returns zero if successful, or non-zero otherwise.
 */
static int ioctl_cropcap(struct v4l2_int_device *s,
    struct v4l2_cropcap *cropcap)
{
    struct mt9t113_sensor *sensor = s->priv;
    
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

/* To get the current crop window for of mt9t113 sensor
 * Returns zero if successful, or non-zero otherwise.
 */
static int ioctl_g_crop(struct v4l2_int_device *s, struct  v4l2_crop *crop)
{
    struct mt9t113_sensor *sensor = s->priv;
    DPRINTK_OV("\n");
    crop->c = sensor->crop_rect;
    return 0;
}

/* To set the crop window for of mt9t113 sensor
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
    
    i = find_vctrl(vc->id);
    if (i < 0)
        return -EINVAL;
    lvc = &video_control[i];
    
    switch (vc->id) {
        case V4L2_CID_BRIGHTNESS:
            vc->value = lvc->current_value;
            break;
         
        case V4L2_CID_CONTRAST:
            vc->value = lvc->current_value;
            break;

        case V4L2_CID_PRIVATE_BASE:
            vc->value = lvc->current_value;
            break;

        case V4L2_CID_EXPOSURE:
            vc->value = lvc->current_value;
            break;

        case V4L2_CID_DO_WHITE_BALANCE:
            vc->value = lvc->current_value;
            break;

        case V4L2_CID_ZOOM_RELATIVE:
            vc->value = lvc->current_value;
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
    struct mt9t113_sensor *sensor = s->priv;
    struct i2c_client *client = sensor->i2c_client;
    struct sensor_ext_params *extp = &(sensor->ext_params);
    #if 0
        if(vc->id != V4L2_CID_CAPTURE)
            return 0;
    #endif

    DPRINTK_OV("vc->id = %d,value = %d",vc->id,vc->value);
    DPRINTK_OV("sensor_version = %d",sensor_version);
    if (sensor_version == SENSOR_IS_MT9T113){
        switch(vc->id){
            case V4L2_CID_CAPTURE:
                mt9t113_capture_mode = 1;
                break;
            case V4L2_CID_POWER_ON:
                mt9t113_set_power = 1;
                printk(KERN_ERR "mt9t113:>>>V4L2_CID_POWER_ON\n");
                break;
            case V4L2_CID_POWER_OFF:
                 mt9t113_set_power = 0;
                 mt9t113_capture_mode = 0;
                 mt9t113_after_capture = 0;
                 printk(KERN_ERR "mt9t113:>>>V4L2_CID_POWER_OFF\n"); 
                 mt9t113_power_off(s);
                  yl_set_debug_enalbe(1, 4);
				 yl_set_debug_enalbe(7, 8);
                 printk(KERN_ERR "mt9t113:>>>open debug log\n"); 

                break;
            case V4L2_CID_CAMERA_VIDEO:
                camera_video = vc->value;                   
                DPRINTK_OV("mt9t113:>>>camera_video = %d", camera_video); 
                break;

            case V4L2_CID_EFFECT:
                DPRINTK_OV("1vc->id = %d,extp->effect = %d",vc->id,vc->value); 
                if(vc->value < 0 || vc->value > ARRAY_SIZE(mt9t113_effects))
                    vc->value = MT9T113_DEF_EFFECT;
                extp->effect = vc->value;
                if(current_power_state == V4L2_POWER_ON){
                DPRINTK_OV("2vc->id = %d,extp->effect = %d",vc->id,vc->value); 
                    err = mt9t113_write_regs(client, mt9t113_effects[vc->value]);
                }			
                break;

            case V4L2_CID_SCENE:
                break;
  
            case V4L2_CID_SATURATION:       
                if(vc->value <= 0 || vc->value > ARRAY_SIZE(mt9t113_saturation))
                     vc->value = MT9T113_DEF_SATURATION;
                extp->saturation= vc->value;
                if(current_power_state == V4L2_POWER_ON){
                    err = mt9t113_write_regs(client, mt9t113_saturation[vc->value - 1]);
                }	
                break;	

            case V4L2_CID_EXPOSURE: 
             #ifndef CONFIG_LOAD_FILE
                DPRINTK_OV("1vc->id = %d,extp->exposure = %d",vc->id,vc->value);     
                if(vc->value <= 0 || vc->value > ARRAY_SIZE(mt9t113_exposure))
                    vc->value = MT9T113_DEF_EXPOSURE;

                if(current_power_state == V4L2_POWER_ON){
                    DPRINTK_OV("2vc->id = %d,extp->exposure = %d",vc->id,vc->value); 
                    if(extp->exposure != vc->value)
                    {
                        err = mt9t113_write_regs(client, mt9t113_exposure[vc->value - 1]);
                    }
                }
                extp->exposure = vc->value;
             #else
              if(vc->value != 3 ){
                   printk(KERN_ERR "mt9t113_set_parameter1");        
                   s5k5ca_regs_table_write("mt9t113_set_parameter1");
               }
             #endif
                break;

            case V4L2_CID_BRIGHTNESS:
                if(vc->value <= 0 || vc->value > ARRAY_SIZE(mt9t113_brightness))
                    vc->value = MT9T113_DEF_BRIGHT;
                extp->brightness = vc->value;
                if(current_power_state == V4L2_POWER_ON){
                    err = mt9t113_write_regs(client, mt9t113_brightness[vc->value - 1]);
                }			
                break;

            case V4L2_CID_DO_WHITE_BALANCE:
                DPRINTK_OV("1vc->id = %d,extp->white_balance = %d",vc->id,vc->value);  
                if(vc->value < 0 || vc->value > ARRAY_SIZE(mt9t113_whitebalance))
                    vc->value = MT9T113_DEF_WB;
                extp->white_balance = vc->value;
                if(current_power_state == V4L2_POWER_ON){  
                DPRINTK_OV("2vc->id = %d,extp->white_balance = %d",vc->id,vc->value);      
                    err = mt9t113_write_regs(client, mt9t113_whitebalance[vc->value]);
                }			
                break;

            case V4L2_CID_ZOOM_RELATIVE:
                 DPRINTK_OV("++++++++V4L2_CID_ZOOM_RELATIVE = %d",vc->value);
                if(vc->value < 0 || vc->value > ARRAY_SIZE(mt9t113_zoom))
                 vc->value = MT9T113_DEF_ZOOM;
                extp->zoom = vc->value;
                if(current_power_state == V4L2_POWER_ON){       
                  err = mt9t113_write_regs(client, mt9t113_zoom[vc->value]);                 
                }			
                break;

             case V4L2_CID_TEST_MODE:   
                 DPRINTK_OV("++++++++V4L2_CID_TEST_MODE = %d",vc->value);      
                 //test_mode = vc->value;   
                  test_mode = 0;  
                  load_path = vc->value;
                 break;

             default:
                  break;

          }
    }else{
        switch(vc->id){
            case V4L2_CID_CAPTURE:
                mt9t113_capture_mode = 1;
                break;
            case V4L2_CID_POWER_ON:
                mt9t113_set_power = 1;
                printk(KERN_ERR "mt9t113:>>>V4L2_CID_POWER_ON\n");
                break;
            case V4L2_CID_POWER_OFF:
                 mt9t113_set_power = 0;
                 mt9t113_capture_mode = 0;
                 mt9t113_after_capture = 0;
                 printk(KERN_ERR "mt9t113:>>>V4L2_CID_POWER_OFF\n"); 
                 mt9t113_power_off(s);
                 yl_set_debug_enalbe(1, 4);
				 yl_set_debug_enalbe(7, 8);
                 printk(KERN_ERR "mt9t113:>>>open debug log\n"); 

                break;
            case V4L2_CID_CAMERA_VIDEO:
                camera_video = vc->value;                   
                DPRINTK_OV("mt9t113:>>>camera_video = %d", camera_video); 
                break;
        
            case V4L2_CID_EXPOSURE:
                DPRINTK_OV("1vc->id = %d,extp->exposure = %d",vc->id,vc->value);  
                if(vc->value <= 0 || vc->value > ARRAY_SIZE(s5k5cag_exposure))
                    vc->value = MT9T113_DEF_EXPOSURE;             
                if(current_power_state == V4L2_POWER_ON){
                DPRINTK_OV("2vc->id = %d,extp->exposure = %d",vc->id,vc->value);  
                    if(extp->exposure != vc->value)
                    {
                        err = mt9t113_write_regs(client, s5k5cag_exposure[vc->value - 1]);
                    }
                }	
                extp->exposure = vc->value;	
                break;
       
            case V4L2_CID_EFFECT:
                DPRINTK_OV("1vc->id = %d,extp->effect = %d",vc->id,vc->value); 
                if(vc->value < 0 || vc->value > ARRAY_SIZE(s5k5cag_effects))
                    vc->value = MT9T113_DEF_EFFECT;             
                if(current_power_state == V4L2_POWER_ON){
                DPRINTK_OV("2vc->id = %d,extp->effect = %d",vc->id,vc->value); 
                    err = mt9t113_write_regs(client, s5k5cag_effects[vc->value]);
                }
                extp->effect = vc->value;			
                break;

            case V4L2_CID_SCENE:
                break;
  
            case V4L2_CID_SATURATION:       
                if(vc->value <= 0 || vc->value > ARRAY_SIZE(s5k5cag_saturation))
                     vc->value = MT9T113_DEF_SATURATION;
                extp->saturation= vc->value;
                if(current_power_state == V4L2_POWER_ON){
                    err = mt9t113_write_regs(client, s5k5cag_saturation[vc->value - 1]);
                }	
                break;	
          
            case V4L2_CID_BRIGHTNESS:
                if(vc->value <= 0 || vc->value > ARRAY_SIZE(s5k5cag_brightness))
                    vc->value = MT9T113_DEF_BRIGHT;
                extp->brightness = vc->value;
                if(current_power_state == V4L2_POWER_ON){
                    err = mt9t113_write_regs(client, s5k5cag_brightness[vc->value - 1]);
                }			
                break;

            case V4L2_CID_DO_WHITE_BALANCE:
                DPRINTK_OV("1vc->id = %d,extp->white_balance = %d",vc->id,vc->value); 
                if(vc->value < 0 || vc->value > ARRAY_SIZE(s5k5cag_whitebalance))
                    vc->value = MT9T113_DEF_WB;
                extp->white_balance = vc->value;
                if(current_power_state == V4L2_POWER_ON){   
                DPRINTK_OV("2vc->id = %d,extp->white_balance = %d",vc->id,vc->value);     
                    err = mt9t113_write_regs(client, s5k5cag_whitebalance[vc->value]);
                   // msleep(100);
                }			
                break;

            case V4L2_CID_ZOOM_RELATIVE:
                 DPRINTK_OV("++++++++V4L2_CID_ZOOM_RELATIVE = %d",vc->value);
                if(vc->value < 0 || vc->value > ARRAY_SIZE(s5k5cag_zoom))
                 vc->value = MT9T113_DEF_ZOOM;
                extp->zoom = vc->value;
                if(current_power_state == V4L2_POWER_ON){       
                   err = mt9t113_write_regs(client, s5k5cag_zoom[vc->value]);
                  
                }			
                break;

             case V4L2_CID_TEST_MODE:   
                 DPRINTK_OV("++++++++V4L2_CID_TEST_MODE = %d",vc->value);      
                // test_mode = vc->value;   
                  test_mode = 0;     
                 break;

             default:
                  break;
          }
    }
      DPRINTK_OV("%s return %d",__func__,err);
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
    
    fmt->flags = mt9t113_formats[index].flags;
    strlcpy(fmt->description, mt9t113_formats[index].description,
        sizeof(fmt->description));
    fmt->pixelformat = mt9t113_formats[index].pixelformat;
    
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
    enum image_size_ov isize = ARRAY_SIZE(mt9t113_sizes) - 1; 
    struct v4l2_pix_format *pix = &f->fmt.pix;
    
    
    DPRINTK_OV("v4l2_pix_format(w:%d,h:%d)",pix->width,pix->height);
    
    if (pix->width > mt9t113_sizes[isize].width)
        pix->width = mt9t113_sizes[isize].width;
    if (pix->height > mt9t113_sizes[isize].height)
        pix->height = mt9t113_sizes[isize].height;
    
    isize = mt9t113_find_size(pix->width, pix->height);
    pix->width = mt9t113_sizes[isize].width;
    pix->height = mt9t113_sizes[isize].height;
    
    for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) {
        if (pix->pixelformat == mt9t113_formats[ifmt].pixelformat)
            break;
    }
    if (ifmt == NUM_CAPTURE_FORMATS)
        ifmt = 0;
    pix->pixelformat = mt9t113_formats[ifmt].pixelformat;
    pix->field = V4L2_FIELD_NONE;
    pix->bytesperline = pix->width*2;
    pix->sizeimage = pix->bytesperline*pix->height;
    DPRINTK_OV("+++++pix->sizeimage = %d",pix->sizeimage);
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
    struct mt9t113_sensor *sensor = s->priv;
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
    struct mt9t113_sensor *sensor = s->priv;
    f->fmt.pix = sensor->pix;
    DPRINTK_OV("f->fmt.pix.pixformat = %x,V4L2_PIX_FMT_YUYV = %x",f->fmt.pix.pixelformat,V4L2_PIX_FMT_YUYV);
    DPRINTK_OV("f->fmt.pix.width = %d ",f->fmt.pix.width);
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
    struct mt9t113_sensor *sensor = s->priv;
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
    struct mt9t113_sensor *sensor = s->priv;
    struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
    struct v4l2_fract timeperframe_old;
    int desired_fps;
    timeperframe_old = sensor->timeperframe;
    sensor->timeperframe = *timeperframe;
    
    desired_fps = timeperframe->denominator / timeperframe->numerator;
    if ((desired_fps < MT9T113_MIN_FPS) || (desired_fps > MT9T113_MAX_FPS))
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
    struct mt9t113_sensor *sensor = s->priv;
    
    return sensor->pdata->priv_data_set(s,p);
}


static int __mt9t113_power_off_standby(struct v4l2_int_device *s,
    enum v4l2_power on)
{
    struct mt9t113_sensor *sensor = s->priv;
    struct i2c_client *client = sensor->i2c_client;
    int rval;

    rval = sensor->pdata->power_set(s, on);
    if (rval < 0) {
        v4l_err(client, "Unable to set the power state: "
            MT9T113_DRIVER_NAME " sensor\n");
        return rval;
    }
    
    sensor->pdata->set_xclk(s, 0);
     
    return 0;
}

static int mt9t113_power_off(struct v4l2_int_device *s)
{
    return __mt9t113_power_off_standby(s, V4L2_POWER_OFF);
}

static int mt9t113_power_standby(struct v4l2_int_device *s)
{
    return __mt9t113_power_off_standby(s, V4L2_POWER_STANDBY);
}



static int mt9t113_power_on(struct v4l2_int_device *s)
{
    struct mt9t113_sensor *sensor = s->priv;
    struct i2c_client *client = sensor->i2c_client;
    int rval;
    
    //sensor->pdata->set_xclk(s, xclk_current);
    
    rval = sensor->pdata->power_set(s, V4L2_POWER_ON);
    if (rval < 0) {
        v4l_err(client, "Unable to set the power state: "
            MT9T113_DRIVER_NAME " sensor\n");
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
            DPRINTK_OV( ">>>>>>>>>>>>mt9t113_power_on_pre");
            if((!mt9t113_set_power) && (!mt9t113_capture_mode) && (!mt9t113_after_capture)) 
            {  
             DPRINTK_OV( ">>>>>>>>>>>>mt9t113_power_on");
              mt9t113_power_on(s);
	      yl_set_debug_enalbe(1, 0);
			yl_set_debug_enalbe(7, 0);
              printk(KERN_ERR "mt9t113:>>>close debug log\n"); 
            }
            mt9t113_configure(s);
            break;
        
        case V4L2_POWER_OFF:
             printk(KERN_ERR "mt9t113:>>>>>>>>>>>>mt9t113_power_off");
             mt9t113_set_power = 0;
             mt9t113_capture_mode = 0;
             mt9t113_after_capture = 0;
             mt9t113_power_off(s);
            break;

        case V4L2_POWER_STANDBY:
            DPRINTK_OV(">>>>>>>>>>>>mt9t113_power_standby_pre");
            break;

        default:
            break;
    }

    current_power_state = on;

    DPRINTK_OV("current_power_state = %d", current_power_state);
    DPRINTK_OV("mt9t113_capture_mode = %d", mt9t113_capture_mode);

    return 0;
}


/*
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 *
 * Initialize the sensor device (call mt9t113_configure())
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

/* adjust the BYD or KERR module
 * 
 */
static int mt9t113_read_gpio(struct i2c_client *client)
{
    u16 uid = 0;
    
    if (!client)
        return -ENODEV;

    mt9t113_write_reg(client, 0xFCFC, 0xD000);
    mt9t113_write_reg(client, 0x108E, 0x0033); //select GPIO as function mode //3333:use GPIO1-GPIO4;0033:GPIO1-GPIO2
    mt9t113_write_reg(client, 0x1090, 0x8888); //select GPIO1-GPIO4 as GPI mode

    if (mt9t113_read_reg(client, 2, 0x100C, &uid)){
        printk(KERN_ERR "S5K5CA: Can't read gpio value\n");
        mt9t113_write_reg(client, 0x108E, 0x0000);  //return to Tri state after check gpio
        sensor_byd_kerr = -1; //none
    } else {
        printk(KERN_ERR "S5K5CA: module_id =0x%04X\n",uid);
        mt9t113_write_reg(client, 0x108E, 0x0000);  //return to Tri state after check gpio
        if(uid == 0x0002){
            sensor_byd_kerr = 1; //byd
        } else {
            sensor_byd_kerr = 2; //kerr
        }       
    }

     return sensor_byd_kerr;   
}

/**
 * ioctl_dev_init - V4L2 sensor interface handler for vidioc_int_dev_init_num
 * @s: pointer to standard V4L2 device structure
 *
 * Initialise the device when slave attaches to the master.  Returns 0 if
 * mt9t113 device could be found, otherwise returns appropriate error.
 */
static int ioctl_dev_init(struct v4l2_int_device *s)
{
    struct mt9t113_sensor *sensor = s->priv;
    struct i2c_client *c = sensor->i2c_client;
    int ret;
    int byd_kerr = 0;
    
    ret = mt9t113_power_on(s);
    if (ret)
        return -ENODEV;
    
    ret = mt9t113_detect(c);
	if(ret == MT9T113_CHIP_ID)
	{
            printk(KERN_ERR "MT9T113(SUNNY):Found sensor MT9T113!\n");
	    sensor_version = SENSOR_IS_MT9T113;
	}
	else if(ret == S5K5CA_CHIP_ID)
   	{          
	    sensor_version = SENSOR_IS_S5K5CA;
            byd_kerr = mt9t113_read_gpio(c);
            if(byd_kerr == 1){
               printk(KERN_ERR "S5K5CA(BYD):Found sensor S5K5CA!\n");
            } else if (byd_kerr == 2){
               printk(KERN_ERR "S5K5CA(KERR):Found sensor S5K5CA!\n");
            } else {
              printk(KERN_ERR "S5K5CA(NONE):Found sensor S5K5CA!\n");
            }

	}
	else
	{
            printk(KERN_ERR "CAMERA:Unable to detect the sensor\n");        
            //Turn power off before leaving this function
            //If not, CAM powerdomain will on      
            mt9t113_power_off(s);
            return -ENODEV;
    }
    
    sensor->ver = ret;
    DPRINTK_OV("chip version 0x%04X detected",
        sensor->ver);
    ret = mt9t113_power_off(s);
    if(ret)
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
    
    DPRINTK_OV("index=%d",frms->index);
    for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) {
        if (frms->pixel_format == mt9t113_formats[ifmt].pixelformat)
            break;
    }
    /* Is requested pixelformat not found on sensor? */
    if (ifmt == NUM_CAPTURE_FORMATS)
        return -EINVAL;
    
    /* Do we already reached all discrete framesizes? */
    if (frms->index >= ARRAY_SIZE(mt9t113_sizes))
        return -EINVAL;
    
    frms->type = V4L2_FRMSIZE_TYPE_DISCRETE;
    frms->discrete.width = mt9t113_sizes[frms->index].width;
    frms->discrete.height = mt9t113_sizes[frms->index].height;
    
    DPRINTK_OV("discrete(%d,%d)",frms->discrete.width,frms->discrete.height);
    return 0;
}

/*QCIF,QVGA,CIF,VGA,SVGA can support up to 30fps*/
const struct v4l2_fract mt9t113_frameintervals[] = {
    { .numerator = 1, .denominator = 15 },//SQCIF,128*96
    { .numerator = 1, .denominator = 15 },//QCIF,176*144
    { .numerator = 1, .denominator = 15 },//QVGA,320*240
    { .numerator = 1, .denominator = 15 },//CIF,352*288
    { .numerator = 1, .denominator = 15 },//VGA,680*480
    { .numerator = 1, .denominator = 20 },//DVD,720*480
    { .numerator = 1, .denominator = 15 },//SVGA,800*600
    { .numerator = 2, .denominator = 15 },//XGA,1024*768
    { .numerator = 2, .denominator = 15 },//1M,1280*960
    { .numerator = 2, .denominator = 15 },//2M,1600*1200
    { .numerator = 2, .denominator = 15 },//3M,2048*1536
};

static int ioctl_enum_frameintervals(struct v4l2_int_device *s,
    struct v4l2_frmivalenum *frmi)
{
    int ifmt;
    int isize = ARRAY_SIZE(mt9t113_frameintervals) - 1;
    
    for (ifmt = 0; ifmt < NUM_CAPTURE_FORMATS; ifmt++) {
        if (frmi->pixel_format == mt9t113_formats[ifmt].pixelformat)
            break;
    }
    
    DPRINTK_OV("frmi->width = %d,frmi->height = %d,frmi->index = %d,ifmt = %d", \
        frmi->width,frmi->height,frmi->index,ifmt);
    
    /* Is requested pixelformat not found on sensor? */
    if (ifmt == NUM_CAPTURE_FORMATS)
        return -EINVAL;
    
    /* Do we already reached all discrete framesizes? */
    if (frmi->index > isize)
        return -EINVAL;
    
    
    frmi->type = V4L2_FRMIVAL_TYPE_DISCRETE;
    frmi->discrete.numerator =
        mt9t113_frameintervals[frmi->index].numerator;
    frmi->discrete.denominator =
        mt9t113_frameintervals[frmi->index].denominator;
    
    return 0;

}

static struct v4l2_int_ioctl_desc mt9t113_ioctl_desc[] = {
    {vidioc_int_enum_framesizes_num,
        (v4l2_int_ioctl_func *)ioctl_enum_framesizes},
    {vidioc_int_enum_frameintervals_num,
        (v4l2_int_ioctl_func *)ioctl_enum_frameintervals},
    {vidioc_int_dev_init_num,
        (v4l2_int_ioctl_func *)ioctl_dev_init},
    {vidioc_int_dev_exit_num,
        (v4l2_int_ioctl_func *)ioctl_dev_exit},
    {vidioc_int_s_power_num,
        (v4l2_int_ioctl_func *)ioctl_s_power},
    {vidioc_int_g_priv_num,
        (v4l2_int_ioctl_func *)ioctl_g_priv},
    {vidioc_int_init_num,
        (v4l2_int_ioctl_func *)ioctl_init},
    {vidioc_int_enum_fmt_cap_num,
        (v4l2_int_ioctl_func *)ioctl_enum_fmt_cap},
    {vidioc_int_try_fmt_cap_num,
        (v4l2_int_ioctl_func *)ioctl_try_fmt_cap},
    {vidioc_int_g_fmt_cap_num,
        (v4l2_int_ioctl_func *)ioctl_g_fmt_cap},
    {vidioc_int_s_fmt_cap_num,
        (v4l2_int_ioctl_func *)ioctl_s_fmt_cap},
    {vidioc_int_g_parm_num,
        (v4l2_int_ioctl_func *)ioctl_g_parm},
    {vidioc_int_s_parm_num,
        (v4l2_int_ioctl_func *)ioctl_s_parm},
    {vidioc_int_queryctrl_num,
        (v4l2_int_ioctl_func *)ioctl_queryctrl},
    {vidioc_int_g_ctrl_num,
        (v4l2_int_ioctl_func *)ioctl_g_ctrl},
    {vidioc_int_s_ctrl_num,
        (v4l2_int_ioctl_func *)ioctl_s_ctrl},
    { vidioc_int_g_crop_num,
        (v4l2_int_ioctl_func *)ioctl_g_crop},
    {vidioc_int_s_crop_num,
        (v4l2_int_ioctl_func *)ioctl_s_crop},
    { vidioc_int_cropcap_num,
        (v4l2_int_ioctl_func *)ioctl_cropcap},
};

static struct v4l2_int_slave mt9t113_slave = {
    .ioctls		= mt9t113_ioctl_desc,
    .num_ioctls	= ARRAY_SIZE(mt9t113_ioctl_desc),
};

static struct v4l2_int_device mt9t113_int_device = {
    .module	= THIS_MODULE,
    .name	= MT9T113_DRIVER_NAME,
    .priv	= &mt9t113,
    .type	= v4l2_int_type_slave,
    .u	= {
      .slave = &mt9t113_slave,
    },
};

/*
 * mt9t113_probe - sensor driver i2c probe handler
 * @client: i2c driver client device structure
 *
 * Register sensor as an i2c client device and V4L2
 * device.
 */
  static int __init
mt9t113_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct mt9t113_sensor *sensor = &mt9t113;
    int err;
    
    DPRINTK_OV("entering ");
    
    if (i2c_get_clientdata(client))
        return -EBUSY;
    
    sensor->pdata = client->dev.platform_data;
    
    if (!sensor->pdata) {
        dev_err(&client->dev, "No platform data?\n");
        return -ENODEV;
    }
    
    sensor->v4l2_int_device = &mt9t113_int_device;
    sensor->i2c_client = client;
    
    i2c_set_clientdata(client, sensor);
    mt9t113_i2c_client = client;

  #if 1
    /* Make the default preview format 640*480 YUV */
    sensor->pix.width = mt9t113_sizes[VGA].width;
    sensor->pix.height = mt9t113_sizes[VGA].height;
  #else
    sensor->pix.width = mt9t113_sizes[UXGA].width;
    sensor->pix.height = mt9t113_sizes[UXGA].height;
  #endif
  
    sensor->pix.pixelformat = V4L2_PIX_FMT_YUYV;
  
    err = v4l2_int_device_register(sensor->v4l2_int_device);
    if (err)
        i2c_set_clientdata(client, NULL);
  
    DPRINTK_OV("exit ");

    return 0;
}

/*
 * mt9t113_remove - sensor driver i2c remove handler
 * @client: i2c driver client device structure
 *
 * Unregister sensor as an i2c client device and V4L2
 * device. Complement of mt9t113_probe().
 */
  static int __exit
mt9t113_remove(struct i2c_client *client)
{
    struct mt9t113_sensor *sensor = i2c_get_clientdata(client);
    
    if (!client->adapter)
        return -ENODEV;	/* our client isn't attached */
    
    v4l2_int_device_unregister(sensor->v4l2_int_device);
    i2c_set_clientdata(client, NULL);
    
    return 0;
}

static const struct i2c_device_id mt9t113_id[] = {
    { MT9T113_DRIVER_NAME, 0 },
    { },
};
MODULE_DEVICE_TABLE(i2c, mt9t113_id);

static struct i2c_driver mt9t113sensor_i2c_driver = {
    .driver = {
      .name	= MT9T113_DRIVER_NAME,
      .owner = THIS_MODULE,
    },
    .probe	= mt9t113_probe,
    .remove	= __exit_p(mt9t113_remove),
    .id_table = mt9t113_id,
};

static struct mt9t113_sensor mt9t113 = {
    .timeperframe = {
      .numerator = 1,
      .denominator = 15,
    },
    .state = SENSOR_NOT_DETECTED,
};

/*
 * mt9t113sensor_init - sensor driver module_init handler
 *
 * Registers driver as an i2c client driver.  Returns 0 on success,
 * error code otherwise.
 */
static int __init mt9t113sensor_init(void)
{
    int err;
    
    DPRINTK_OV("\n");
    err = i2c_add_driver(&mt9t113sensor_i2c_driver);
    if (err) {
      DPRINTK_OV(KERN_ERR "Failed to register" MT9T113_DRIVER_NAME ".");
      return  err;
    }
    return 0;
}
late_initcall(mt9t113sensor_init);

/*
 * mt9t113sensor_cleanup - sensor driver module_exit handler
 *
 * Unregisters/deletes driver as an i2c client driver.
 * Complement of mt9t113sensor_init.
 */
static void __exit mt9t113sensor_cleanup(void)
{
    i2c_del_driver(&mt9t113sensor_i2c_driver);
}
module_exit(mt9t113sensor_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MT9T113 camera sensor driver");



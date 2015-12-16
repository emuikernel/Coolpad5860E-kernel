/*
 * imx073_regs.h 
 *
 * Register definitions for the IMX073 Sensor.
 *
 * Leverage MT9P012.h
 *
 * Copyright (C) 2008 Hewlett Packard.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#ifndef BF3703_H
#define BF3703_H

//added by bf3703
#define auto_frame_rate
//added by bf3703
#define _REVYUV_ 0


/* IMX073 has 8/16/32 I2C registers */
#define I2C_8BIT			1
#define I2C_16BIT			2
#define I2C_32BIT			4

/* Terminating list entry for reg */
#define I2C_REG_TERM		0xFFFF
/* Terminating list entry for val */
#define I2C_VAL_TERM		0xFFFFFFFF
/* Terminating list entry for len */
#define I2C_LEN_TERM		0xFFFF

/* Used registers */
#define VAUX4_2_8_V		0x09
//#define ENABLE_VAUX4_DEV_GRP    0xE0
//#define VAUX4_DEV_GRP_P1		0x20
//#define VAUX4_DEV_GRP_NONE	0x00



/* Register initialization tables for BF3703 */
/* Terminating list entry for reg */
//#define BF3703_REG_TERM		0xFFFF
/* Terminating list entry for val */
//#define BF3703_VAL_TERM		0xFF



#define BF3703_CSI2_VIRTUAL_ID	0x1

#define DEBUG_BASE		0x08000000



/* Sensor specific GPIO signals */
#define BF3703_RESET_GPIO  	98        //154
#define BF3703_STANDBY_GPIO	167        //110


/* FPS Capabilities */
#define BF3703_MIN_FPS			5
#define BF3703_DEF_FPS			15
#define BF3703_MAX_FPS			30

/*brightness*/
#define BF3703_MIN_BRIGHT		1
#define BF3703_MAX_BRIGHT		5
#define BF3703_DEF_BRIGHT		3
#define BF3703_BRIGHT_STEP		1

/*contrast*/
#define BF3703_DEF_CONTRAST		3
#define BF3703_MIN_CONTRAST		1
#define BF3703_MAX_CONTRAST		5
#define BF3703_CONTRAST_STEP	1

/*saturation*/
#define BF3703_DEF_SATURATION	3
#define BF3703_MIN_SATURATION   1
#define BF3703_MAX_SATURATION	5
#define BF3703_SATURATION_STEP	1

/*exposure*/
#define BF3703_DEF_EXPOSURE	    3
#define BF3703_MIN_EXPOSURE	    1
#define BF3703_MAX_EXPOSURE	    5
#define BF3703_EXPOSURE_STEP	1

/*sharpness*/
#define BF3703_DEF_SHARPNESS	3
#define BF3703_MIN_SHARPNESS	1
#define BF3703_MAX_SHARPNESS	5
#define BF3703_SHARPNESS_STEP	1

#define BF3703_DEF_COLOR		0
#define BF3703_MIN_COLOR		0
#define BF3703_MAX_COLOR		2
#define BF3703_COLOR_STEP		1

/*effect*/
#define BF3703_DEF_EFFECT		0
#define BF3703_MIN_EFFECT	    0
#define BF3703_MAX_EFFECT	    7
#define BF3703_EFFECT_STEP		1

/*white balance*/
#define BF3703_DEF_WB		0
#define BF3703_MIN_WB	    1
#define BF3703_MAX_WB	    4
#define BF3703_WB_STEP   	1


#define BF3703_BLACK_LEVEL_10BIT	16

#define SENSOR_DETECTED		1
#define SENSOR_NOT_DETECTED	0

/* NOTE: Set this as 0 for enabling SoC mode */
//#define BF3703_RAW_MODE	1
#define BF3703_YUV_MODE 1

/* XCLK Frequency in Hz*/
#define BF3703_XCLK_MIN		24000000
#define BF3703_XCLK_MAX		24000000

/* ------------------------------------------------------ */

/* define a structure for BF3703 register initialization values */
struct BF3703_reg {
	unsigned char reg;
	unsigned char val;
};


#if 1
struct capture_size_bf3703 {
	unsigned long width;
	unsigned long height;
};

enum image_size_bf3703 {
    BF3703_QQVGA, /*160*120*/
    BF3703_QCIF, /*176*144*/
    BF3703_QVGA, /*320*240*/
    BF3703_CIF, /*352*288*/
    BF3703_VGA, /*640*480*/     
};


enum pixel_format_bf3703 {
	BF3703_YUV,
	BF3703_RGB565,
	BF3703_RGB555,
	BF3703_RAW10
};

#endif


//#define OV_NUM_PIXEL_FORMATS		4
//#define OV_NUM_FPS			3

#define V4L2_CID_EFFECT (V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_SCENE (V4L2_CID_PRIVATE_BASE + 2)
#define V4L2_CID_CAPTURE (V4L2_CID_PRIVATE_BASE + 3)
#define V4L2_CID_TEST_MODE	(V4L2_CID_PRIVATE_BASE + 4)
#define V4L2_CID_POWER_ON (V4L2_CID_PRIVATE_BASE + 5)
#define V4L2_CID_POWER_OFF (V4L2_CID_PRIVATE_BASE + 6)


typedef enum
{
    BF3703_EXPOSURE_MODE_EXP_AUTO,
    BF3703_EXPOSURE_MODE_EXP_MACRO,
    BF3703_EXPOSURE_MODE_EXP_PORTRAIT,
    BF3703_EXPOSURE_MODE_EXP_LANDSCAPE,
    BF3703_EXPOSURE_MODE_EXP_SPORTS,
    BF3703_EXPOSURE_MODE_EXP_NIGHT,
    BF3703_EXPOSURE_MODE_EXP_NIGHT_PORTRAIT,
    BF3703_EXPOSURE_MODE_EXP_BACKLIGHTING,
    BF3703_EXPOSURE_MODE_EXP_MANUAL
} BF3703_EXPOSURE_MODE_VALUES;

typedef enum {
    BF3703_SENSOR_SCENE_MODE_AUTO = 0,
    BF3703_SENSOR_SCENE_MODE_SUNNY = 1,
    BF3703_SENSOR_SCENE_MODE_CLOUDY = 2,
    BF3703_SENSOR_SCENE_MODE_OFFICE = 3,
    BF3703_SENSOR_SCENE_MODE_HOME = 4,
    BF3703_SENSOR_SCENE_MODE_NIGHT = 5
}BF3703_SENSOR_SCENE_MODE;

typedef enum{
    BF3703_SENSOR_WHITE_BALANCE_AUTO = 0,
    BF3703_SENSOR_WHITE_BALANCE_DAYLIGHT = 1,
    BF3703_SENSOR_WHITE_BALANCE_CLOUDY = 2,
    BF3703_SENSOR_WHITE_BALANCE_INCANDESCENT = 3,
    BF3703_SENSOR_WHITE_BALANCE_FLUORESCENT = 4,
} BF3703_SENSOR_WHITE_BALANCE_MODE;

typedef enum {
    BF3703_SENSOR_EFFECT_NORMAL = 0,
    BF3703_SENSOR_EFFECT_MONO = 1,
    BF3703_SENSOR_EFFECT_NEGATIVE = 2,
    BF3703_SENSOR_EFFECT_SEPIA =3,
    BF3703_SENSOR_EFFECT_BLUISH = 4,
    BF3703_SENSOR_EFFECT_GREEN = 5,
    BF3703_SENSOR_EFFECT_REDDISH = 6,
    BF3703_SENSOR_EFFECT_YELLOWISH = 7
}BF3703_SENSOR_EFFECTS;


struct bf3703_sensor_ext_params{
    BF3703_SENSOR_EFFECTS effect;
    BF3703_SENSOR_WHITE_BALANCE_MODE white_balance;
    BF3703_SENSOR_SCENE_MODE scene_mode;
    int brightness;
    int contrast;
    int saturation;
    int exposure;
    int sharpness;
};

/**
 * struct BF3703_sensor - main structure for storage of sensor information
 * @pdata: access functions and data for platform level information
 * @v4l2_int_device: V4L2 device structure structure
 * @i2c_client: iic client device structure
 * @pix: V4L2 pixel format information structure
 * @timeperframe: time per frame expressed as V4L fraction
 * @isize: base image size
 * @ver: BF3703 chip version
 * @width: configured width
 * @height: configuredheight
 * @vsize: vertical size for the image
 * @hsize: horizontal size for the image
 * @crop_rect: crop rectangle specifying the left,top and width and height
 */
struct BF3703_sensor {
	const struct BF3703_platform_data *pdata;
	struct v4l2_int_device *v4l2_int_device;
	struct i2c_client *i2c_client;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	struct bf3703_sensor_ext_params ext_params;
	int isize;
	int ver;
	int fps;
	unsigned long width;
	unsigned long height;
	unsigned long vsize;
	unsigned long hsize;
	struct v4l2_rect crop_rect;
	int state;
};






const static struct BF3703_reg icBf3703Init[]  = 
{
	//BF3703 INI   
	//{0x12 , 0x80},
	{0x11 , 0x80},
        {0x20 , 0x40}, //
	{0x09 , 0x8A}, //03 //1x:0x00 2x:0x45 3x:0x8A 4x:0xcf
	{0x13 , 0x00},
	{0x01 , 0x13},
	{0x02 , 0x25},
	{0x8c , 0x02},//01 :devided by 2  02 :devided by 1
	{0x8d , 0xfa},//fd //cb: devided by 2  fd :devided by 1
	{0x87 , 0x1a},
	{0x13 , 0x07},
	
	//POLARITY of Signal
	{0x15 , 0x00},
	{0x3a , 0x02},	  
	
	//black level  , ���ϵ�ƫ���и��� , �����Ҫ��ѡ��ʹ��
#if 0
	{0x05 , 0x1f},
	{0x06 , 0x60},
	{0x14 , 0x1f},
	{0x27 , 0x03},
	{0x06 , 0xe0},
#endif
	
	//lens shading
	{0x35 , 0x85},//68//4b
	{0x65 , 0x85},//68//5b
	{0x66 , 0x85},//62//54
	{0x36 , 0x05},
	{0x37 , 0xe8},//f6
	{0x38 , 0x46},
	{0x9b , 0xff},//f6
	{0x9c , 0x46},
	{0xbc , 0x11},//01
	{0xbd , 0x08},//f6
	{0xbe , 0x46},
	
	//AE
	{0x82 , 0x14},
	{0x83 , 0x23},
	{0x9a , 0x23},//the same as 0x83
	{0x84 , 0x1a},
	{0x85 , 0x20},
	{0x89 , 0x04},//02 :devided by 2  04 :devided by 1
	{0x8a , 0x08},//04: devided by 2  05 :devided by 1
	{0x86 , 0x28},//the same as 0x7b
	{0x96 , 0x26},//AE speed//a6
	{0x97 , 0x0c},//AE speed
	{0x98 , 0x18},//AE speed
	//AE target
	{0x24 , 0x6a},//�������  0x6a  //66
	{0x25 , 0x7a},//�������  0x7a  //76
	{0x94 , 0x0a},//INT_OPEN  
	{0x80 , 0x78},//55 [0]=0:60hz [0]=1:50hz 
	{0x8b , 0x01},//djj add
	
	//denoise 
	{0x70 , 0x6f},//denoise
	{0x72 , 0x4f},//denoise
	{0x73 , 0x2f},//denoise
	{0x74 , 0x27},//denoise
	{0x77 , 0x90},//ȥ���������
	{0x7a , 0x4e},//denoise in	low light , 0x8e\0x4e\0x0e
	{0x7b , 0x28},//the same as 0x86
	
	//black level
	{0X1F , 0x20},//G target
	{0X22 , 0x20},//R target
	{0X26 , 0x20},//B target
	//ģ�ⲿ�ֲ���
	{0X16 , 0x00},//����ú�ɫ���岻���ڣ��е�ƫ�죬��0x16дΪ0x03���е����	  
	{0xbb , 0x20},  // deglitch  ��xclk����
	{0xeb , 0x30},
	{0xf5 , 0x21},
	{0xe1 , 0x3c},
	{0xbb , 0x20},
	{0X2f , 0Xf6},//66
	{0x06 , 0xe0},
	
	
	
	//anti black sun spot
	{0x61 , 0xd3},//0x61[3]=0 black sun disable
	{0x79 , 0x48},//0x79[7]=0 black sun disable
	
	//contrast
	{0x56 , 0x40},
	
	//Gamma
	
	{0x3b , 0x60},//auto gamma offset adjust in  low light	
	{0x3c , 0x20},//auto gamma offset adjust in  low light	
	
	{0x39 , 0x80},	
	/*//gamma1
	{0x3f , 0xb0},
	{0X40 , 0X88},
	{0X41 , 0X74},
	{0X42 , 0X5E},
	{0X43 , 0X4c},
	{0X44 , 0X44},
	{0X45 , 0X3E},
	{0X46 , 0X39},
	{0X47 , 0X35},
	{0X48 , 0X31},
	{0X49 , 0X2E},
	{0X4b , 0X2B},
	{0X4c , 0X29},
	{0X4e , 0X25},
	{0X4f , 0X22},
	{0X50 , 0X1F},*/
	
	/*gamma2  ���ع�Ⱥã����v�
	{0x3f , 0xb0},
	{0X40 , 0X9b},
	{0X41 , 0X88},
	{0X42 , 0X6e},
	{0X43 , 0X59},
	{0X44 , 0X4d},
	{0X45 , 0X45},
	{0X46 , 0X3e},
	{0X47 , 0X39},
	{0X48 , 0X35},
	{0X49 , 0X31},
	{0X4b , 0X2e},
	{0X4c , 0X2b},
	{0X4e , 0X26},
	{0X4f , 0X23},
	{0X50 , 0X1F},
	*/
	/*//gamma3 �������� �ҽ׷ֲ���
	{0X3f , 0Xb0},
	{0X40 , 0X60},
	{0X41 , 0X60},
	{0X42 , 0X66},
	{0X43 , 0X57},
	{0X44 , 0X4c},
	{0X45 , 0X43},
	{0X46 , 0X3c},
	{0X47 , 0X37},
	{0X48 , 0X33},
	{0X49 , 0X2f},
	{0X4b , 0X2c},
	{0X4c , 0X29},
	{0X4e , 0X25},
	{0X4f , 0X22},
	{0X50 , 0X20},*/
	
	//gamma 4   low noise   
	{0X3f , 0Xa8},
	{0X40 , 0X48},
	{0X41 , 0X54},
	{0X42 , 0X4E},
	{0X43 , 0X44},
	{0X44 , 0X3E},
	{0X45 , 0X39},
	{0X46 , 0X35},
	{0X47 , 0X31},
	{0X48 , 0X2E},
	{0X49 , 0X2B},
	{0X4b , 0X29},
	{0X4c , 0X27},
	{0X4e , 0X23},
	{0X4f , 0X20},
	{0X50 , 0X20},
	
	
	//color matrix
	{0x51 , 0x0d},
	{0x52 , 0x21},
	{0x53 , 0x14},
	{0x54 , 0x15},
	{0x57 , 0x8d},
	{0x58 , 0x78},
	{0x59 , 0x5f},
	{0x5a , 0x84},
	{0x5b , 0x25},
	{0x5D , 0x95},
	{0x5C , 0x0e},
	
	/* 
	
	// color  ����
	{0x51 , 0x0e},
	{0x52 , 0x16},
	{0x53 , 0x07},
	{0x54 , 0x1a},
	{0x57 , 0x9d},
	{0x58 , 0x82},
	{0x59 , 0x71},
	{0x5a , 0x8d},
	{0x5b , 0x1c},
	{0x5D , 0x95},
	{0x5C , 0x0e},
	//  
	
	
	
	//����
	{0x51 , 0x08},
	{0x52 , 0x0E},
	{0x53 , 0x06},
	{0x54 , 0x12},
	{0x57 , 0x82},
	{0x58 , 0x70},
	{0x59 , 0x5C},
	{0x5a , 0x77},
	{0x5b , 0x1B},
	{0x5c , 0x0e},//0x5c[3:0] low light color coefficient��smaller  , lower noise
	{0x5d , 0x95},
	
	
	//color ��
	{0x51 , 0x03},
	{0x52 , 0x0d},
	{0x53 , 0x0b},
	{0x54 , 0x14},
	{0x57 , 0x59},
	{0x58 , 0x45},
	{0x59 , 0x41},
	{0x5a , 0x5f},
	{0x5b , 0x1e},
	{0x5c , 0x0e},//0x5c[3:0] low light color coefficient��smaller  , lower noise
	{0x5d , 0x95},
	*/
	
	{0x60 , 0x20},//color open in low light 
	//AWB
	{0x6a , 0x01},//����ɫƫɫ����0x6aдΪ0x81.
	{0x23 , 0x66},//Green gain
	{0xa0 , 0x03},//0xa0д0x03����ɫ�����죻0xa0д0x07����ɫ�����ڣ�//07
	
	{0xa1 , 0X41},//
	{0xa2 , 0X0e},
	{0xa3 , 0X26},
	{0xa4 , 0X0d},
	//cool color
	{0xa5 , 0x28},//The upper limit of red gain 
	
	
	/*warm color
	{0xa5 , 0x2d},
	*/
	{0xa6 , 0x04},
	{0xa7 , 0x80},//BLUE Target
	{0xa8 , 0x80},//RED Target
	{0xa9 , 0x28},
	{0xaa , 0x28},
	{0xab , 0x28},
	{0xac , 0x3c},
	{0xad , 0xf0},
	{0xc8 , 0x16},//18
	{0xc9 , 0x1e},//20
	{0xca , 0x17},
	{0xcb , 0x1f},
	{0xaf , 0x00},		  
	{0xc5 , 0x16},//18	  
	{0xc6 , 0x00},
	{0xc7 , 0x1e},//20	  
	{0xae , 0x83},//����ջ���ƫ6�����˼Ĵ���0xaeдΪ0x81��
	{0xcc , 0x30},
	{0xcd , 0x70},
	{0xee , 0x4c},// P_TH
	
	// color saturation
	{0xb0 , 0xd0},
	{0xb1 , 0xb0},//c0
	{0xb2 , 0xb0},
	
	{0x8e , 0x07},
	{0x8f , 0x79},
	/* // ���Ͷ�����
	{0xb1 , 0xd0},
	{0xb2 , 0xc0},	  
	*/
	{0xb3 , 0x86},//88
	
	//anti webcamera banding //50Hz
	{0x9d , 0x99},

	//anti webcamera banding //60Hz
	{0x9e , 0x71},

        //framerate set
        {0x2b , 0x64},
       //15fps
        {0x8e , 0x05},//3
        {0x8f , 0x4d},//fc
        {0x92 , 0xfe},
        {0x93 , 0x01},
     /*  
        //20fps
        {0x8e , 0x02},
        {0x8f , 0xfd},
        {0x92 , 0xff},
        {0x93 , 0x00},
     */  
	//switch direction
	{0x1e , 0x00},//00:normal  10:IMAGE_V_MIRROR   20:IMAGE_H_MIRROR  30:IMAGE_HV_MIRROR

        {0x55 , 0x2A}, //exposure

        {0x0b , 0x03}, //skip 3 frames

        {0xff , 0xff},    
};


/* Array of image sizes supported by BF3703.  These must be ordered from
 * smallest image size to largest.
 */
const static struct capture_size_bf3703 BF3703_sizes[] = {
    /*QQVGA,capture & video*/
    {160,120},
    /*QCIF,capture & video*/
    {176,144},
    /* QVGA,capture & video */
    {320,240},
    /*CIF,video*/
    {352,288},
    /*VGA,capture & video*/
    {640,480},  
};

const static struct BF3703_reg BF3703_qqvga_preview[] = {  //160x120
    {0x17,0x29},
    {0x18,0x79},
    {0x19,0x1e},
    {0x1a,0x5a},
    {0x03,0xf0},
    {0x12,0x10},
                                               
    {0xFF,0xFF},
};

const static struct BF3703_reg BF3703_qcif_preview[] = {  //176x144
#if 0
    {0x17,0x24},    //QCIF
    {0x18,0x7c},    //QCIF
    {0x19,0x18},    //QCIF
    {0x1a,0x60},    //QCIF
    {0x03,0x00},    //QCIF
    {0x12,0x10},    //YUV 
#else
    {0x17,0x36},
    {0x18,0x8e},
    {0x19,0x1b},
    {0x1a,0x63},
    {0x03,0xf0},
    {0x12,0x10},
#endif
    {0xFF,0xFF},
};

const static struct BF3703_reg BF3703_qvga_preview[] = {   //320x240          
    {0x17,0x00},
    {0x18,0xa0},
    {0x19,0x00},
    {0x1a,0x78},
    {0x03,0x00},
    {0x12,0x10},

    {0xFF,0xFF},
};

const static struct BF3703_reg BF3703_cif_preview[] = {   //352x258
#if 0
    {0x17,0x24},    //QCIF
    {0x18,0x7c},    //QCIF
    {0x19,0x18},    //QCIF
    {0x1a,0x60},    //QCIF
    {0x03,0x00},    //QCIF
    {0x12,0x00},    //YUV 
#else
    {0x17,0x36},
    {0x18,0x8e},
    {0x19,0x1b},
    {0x1a,0x63},
    {0x03,0xf0},
    {0x12,0x00},

    {0x8e,0x03}, //need to change back to default at other case
    {0x8f,0xfc},
#endif


                                              
    {0xFF,0xFF},
};

const static struct BF3703_reg BF3703_vga_preview[] = {  //640x480
    {0x17,0x00},
    {0x18,0xa0},
    {0x19,0x00},
    {0x1a,0x78},
    {0x03,0x00},
    {0x12,0x00},
                                                
    {0xFF,0xFF},
};


const static struct BF3703_reg* bf3703_xxx_preview[] ={
    BF3703_qqvga_preview,
    BF3703_qcif_preview,
    BF3703_qvga_preview,
    BF3703_cif_preview,
    BF3703_vga_preview,
};

const static struct BF3703_reg BF3703_exposure_00[]={   
    {0x55,0x00},// Y_RGB_OFFSET[Manual EV+2] //a0
    {0xFF,0xFF}
};
const static struct BF3703_reg BF3703_exposure_1[]={  
    {0x55,0xA8},// Y_RGB_OFFSET[Manual EV+2]
    {0xFF,0xFF}
};
const static struct BF3703_reg BF3703_exposure_2[]={  
    {0x55,0x90},// Y_RGB_OFFSET[Manual EV+2]
    {0xFF,0xFF}
};
const static struct BF3703_reg BF3703_exposure_3[]={   
    {0x55,0x2A},// Y_RGB_OFFSET[Manual EV+2] //a0
    {0xFF,0xFF}
};
const static struct BF3703_reg BF3703_exposure_4[]={   
    {0x55,0x38},// Y_RGB_OFFSET[Manual EV+2]
    {0xFF,0xFF}
};
const static struct BF3703_reg BF3703_exposure_5[]={   
    {0x55,0x48},// Y_RGB_OFFSET[Manual EV+2]
    {0xFF,0xFF}
    
};

const static struct BF3703_reg* BF3703_exposure[]={
    BF3703_exposure_1,
    BF3703_exposure_2,
    BF3703_exposure_3,
    BF3703_exposure_4,
    BF3703_exposure_5
};

#if 0
const static struct BF3703_reg BF3703_contrast_1[]={  
    {0x56,0x20},// Y_RGB_OFFSET[Manual EV+2]
    {0xFF,0xFF}
};
const static struct BF3703_reg BF3703_contrast_2[]={  
    {0x56,0x20},// Y_RGB_OFFSET[Manual EV+2]
    {0xFF,0xFF}
};
const static struct BF3703_reg BF3703_contrast_3[]={   
    {0x56,0x40},// Y_RGB_OFFSET[Manual EV+2]
    {0xFF,0xFF}
};
const static struct BF3703_reg BF3703_contrast_4[]={   
    {0x56,0x80},// Y_RGB_OFFSET[Manual EV+2]
    {0xFF,0xFF}
};
const static struct BF3703_reg BF3703_contrast_5[]={   
    {0x56,0xA0},// Y_RGB_OFFSET[Manual EV+2]
    {0xFF,0xFF}
    
};

const static struct BF3703_reg* BF3703_contrast[]={
    BF3703_contrast_1,
    BF3703_contrast_2,
    BF3703_contrast_3,
    BF3703_contrast_4,
    BF3703_contrast_5
};
#endif


#endif /* ifndef IMX073_REGS_H */

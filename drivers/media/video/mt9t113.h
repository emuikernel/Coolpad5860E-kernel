/*
 * drivers/media/video/mt9t113.h
 *
 * Register definitions for the MT9T113 CameraChip.
 *
 * Author: Pallavi Kulkarni (ti.com)
 *
 * Copyright (C) 2008 Texas Instruments.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#ifndef MT9T113_H
#define MT9T113_H


#define VAUX4_2_8_V		0x09
#define ENABLE_VAUX4_DEV_GRP    0xE0
#define VAUX4_DEV_GRP_P1		0x20
#define VAUX4_DEV_GRP_NONE	0x00



/* Register initialization tables for mt9t113 */
/* Terminating list entry for reg */
#define MT9T113_REG_TERM		0xFFFF
/* Terminating list entry for val */
#define MT9T113_VAL_TERM		0xFF


#define MT9T113_CSI2_VIRTUAL_ID	0x1

#define DEBUG_BASE		0x08000000



/* Sensor specific GPIO signals */
#define MT9T113_RESET_GPIO  	98        //154
#define MT9T113_STANDBY_GPIO	167        //110


/* FPS Capabilities */
#define MT9T113_MIN_FPS			5
#define MT9T113_DEF_FPS			15
#define MT9T113_MAX_FPS			30

/*brightness*/
#define MT9T113_MIN_BRIGHT		1
#define MT9T113_MAX_BRIGHT		5
#define MT9T113_DEF_BRIGHT		3
#define MT9T113_BRIGHT_STEP		1

/*contrast*/
#define MT9T113_DEF_CONTRAST		3
#define MT9T113_MIN_CONTRAST		1
#define MT9T113_MAX_CONTRAST		5
#define MT9T113_CONTRAST_STEP	1

/*saturation*/
#define MT9T113_DEF_SATURATION	3
#define MT9T113_MIN_SATURATION   1
#define MT9T113_MAX_SATURATION	5
#define MT9T113_SATURATION_STEP	1

/*exposure*/
#define MT9T113_DEF_EXPOSURE	    3
#define MT9T113_MIN_EXPOSURE	    1
#define MT9T113_MAX_EXPOSURE	    5
#define MT9T113_EXPOSURE_STEP	1

/*sharpness*/
#define MT9T113_DEF_SHARPNESS	3
#define MT9T113_MIN_SHARPNESS	1
#define MT9T113_MAX_SHARPNESS	5
#define MT9T113_SHARPNESS_STEP	1

#define MT9T113_DEF_COLOR		0
#define MT9T113_MIN_COLOR		0
#define MT9T113_MAX_COLOR		2
#define MT9T113_COLOR_STEP		1

/*effect*/
#define MT9T113_DEF_EFFECT		0
#define MT9T113_MIN_EFFECT	    0
#define MT9T113_MAX_EFFECT	   4
#define MT9T113_EFFECT_STEP		1

/*white balance*/
#define MT9T113_DEF_WB		0
#define MT9T113_MIN_WB	    1
#define MT9T113_MAX_WB	    5    
#define MT9T113_WB_STEP   	1

/*zoom*/
#define MT9T113_DEF_ZOOM		0
#define MT9T113_MIN_ZOOM	    1
#define MT9T113_MAX_ZOOM	    6    
#define MT9T113_ZOOM_STEP   	1

#define MT9T113_BLACK_LEVEL_10BIT	16

#define SENSOR_DETECTED		1
#define SENSOR_NOT_DETECTED	0

/* NOTE: Set this as 0 for enabling SoC mode */
//#define MT9T113_RAW_MODE	1
#define MT9T113_YUV_MODE 1

/*product ID for mt9t113*/
#define MT9T113_CHIP_ID	 0x7a78   
#define S5K5CA_CHIP_ID	 0x05CA

/*REGISTERs address*/
#define MT9T113_ID_ADDR     0x0040
#define S5K5CA_ID_ADDR      0x1006


/* XCLK Frequency in Hz*/
//#define MT9T113_XCLK_MIN		24000000
//#define MT9T113_XCLK_MAX		24000000

/* ------------------------------------------------------ */

/* define a structure for mt9t113 register initialization values */
struct mt9t113_reg {
    unsigned short reg;
    unsigned short val;
	//unsigned char val;
};


struct capture_size_ov {
    unsigned long width;
    unsigned long height;
};

enum image_size_ov {
    SQCIF, /*128*96*/
    QCIF, /*176*144*/
    QVGA, /*320*240*/
    CIF, /*352*288*/
    VGA, /*640*480*/    
    DVD_NTSC, /*720*480*/
    SVGA, /*800*600*/
    XGA, /*1024*768*/
    ONE_M, /*1280*960*/
    UXGA,  /*1600*1200*/
    QXGA,  /*2048*1536*/
};

/* Array of image sizes supported by MT9T113.  These must be ordered from
 * smallest image size to largest.
 */
const static struct capture_size_ov mt9t113_sizes[] = {
    /*SQCIF,capture & video*/
    {128,96},
    /*QCIF,capture & video*/
    {176,144},
    /* QVGA,capture & video */
    {320,240},
    /*CIF,video*/
    {352,288},
    /*VGA,capture & video*/
    {640,480}, 
    /*DVD-video NTSC,video*/
    {720,480},
    /*SVGA*,capture*/
    {800,600},
    /*capture*/
    {1024,768},
    /*capture*/
    {1280,960},
    /*UXGA*/
    {1600,1200},
    /*QXGA*/
    {2048,1536},
};

enum pixel_format_ov {
    YUV,
    RGB565,
    RGB555,
    RAW10
};

#define OV_NUM_PIXEL_FORMATS		4
#define OV_NUM_FPS			3

#define V4L2_CID_EFFECT (V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_SCENE (V4L2_CID_PRIVATE_BASE + 2)
#define V4L2_CID_CAPTURE (V4L2_CID_PRIVATE_BASE + 3)
#define V4L2_CID_TEST_MODE	(V4L2_CID_PRIVATE_BASE + 4)
#define V4L2_CID_POWER_ON (V4L2_CID_PRIVATE_BASE + 5)
#define V4L2_CID_POWER_OFF (V4L2_CID_PRIVATE_BASE + 6)
#define V4L2_CID_CAMERA_VIDEO (V4L2_CID_PRIVATE_BASE + 7)

#define V4L2_CID_FLASH_STROBE	(V4L2_CID_CAMERA_CLASS_BASE+19)


const static struct mt9t113_reg mt9t113_common_svgabase[] = {

      //MCLK=24MHz, PCLK=50MHz, preveiw 640x480, capture 2048 x 1536  
    //XMCLK=24000000
    //DELAY=500
    //Reset
    {0x0018, 0x4129}, // STANDBY_CONTROL_AND_STATUS
    {0x0000, 0x0001}, //DELAY=1
                                            
    {0x0018, 0x4029}, // STANDBY_CONTROL_AND_STATUS
    {0x0000, 0x0005}, //DELAY=5

    //[Pll]
    {0x0010, 0x0219},     //PLL Dividers = 537
    {0x0012, 0x0070},     //PLL P Dividers = 112
    {0x002A, 0x7474},     //PLL P Dividers 4-5-6 = 29812
    {0x0022, 0x0140},     //Reference clock count for 20 us = 320
    {0x001E, 0x0677},     //Pad Slew Rate = 1911
    {0x3B84, 0x018C},     //I2C Master Clock Divider = 396
    {0x0018, 0x402E},     //Out of Standby = 16424   0x4028
    {0x0000, 0x001E}, //DELAY=30
    //[Timing]
    {0x098E, 0x4800},     //Row Start (A)
    {0x0990, 0x0010},     //      = 16
    {0x098E, 0x4802},     //Column Start (A)
    {0x0990, 0x0010},     //      = 16
    {0x098E, 0x4804},     //Row End (A)
    {0x0990, 0x062D},     //      = 1581
    {0x098E, 0x4806},     //Column End (A)
    {0x0990, 0x082D},     //      = 2093
    {0x098E, 0x4808},     //Base Frame Lines (A)
    {0x0990, 0x0646}, //5EE    //      = 1518
    {0x098E, 0x480A},     //Line Length (A)
    {0x0990, 0x0D07},     //      = 3335
    {0x098E, 0x480C},     //Fine Correction (A)
    {0x0990, 0x0399},     //      = 921
    {0x098E, 0x480E},     //Row Speed (A)
    {0x0990, 0x0111},     //      = 273
    {0x098E, 0x4810},     //Read Mode (A)
    {0x0990, 0x046C},     //      = 1132
    {0x098E, 0x4812},     //Fine IT Min (A)
    {0x0990, 0x0510},     //      = 1296
    {0x098E, 0x4814},     //Fine IT Max Margin (A)
    {0x0990, 0x01BA},     //      = 442
    {0x098E, 0x482D},     //Row Start (B)
    {0x0990, 0x0018},     //      = 24
    {0x098E, 0x482F},     //Column Start (B)
    {0x0990, 0x0018},     //      = 24
    {0x098E, 0x4831},     //Row End (B)
    {0x0990, 0x0627},     //      = 1575
    {0x098E, 0x4833},     //Column End (B)
    {0x0990, 0x0827},     //      = 2087
    {0x098E, 0x4835},     //Base Frame Lines (B)
    {0x0990, 0x065D},     //      = 1629
    {0x098E, 0x4837},     //Line Length (B)
    {0x0990, 0x1A35},     //      = 6709
    {0x098E, 0x4839},     //Fine Correction (B)
    {0x0990, 0x019F},     //      = 415
    {0x098E, 0x483B},     //Row Speed (B)
    {0x0990, 0x0111},     //      = 273
    {0x098E, 0x483D},     //Read Mode (B)
    {0x0990, 0x0024},     //      = 36
    {0x098E, 0x483F},     //Fine IT Min (B)
    {0x0990, 0x0266},     //      = 614
    {0x098E, 0x4841},     //Fine IT Max Margin (B)
    {0x0990, 0x010A},     //      = 266
    {0x098E, 0xB81A},     //fd_zone_height
    {0x0990, 0x0006},     //      = 6
    {0x098E, 0x481A},     //fd_period_50Hz (A)
    {0x0990, 0x00F0},     //      = 240
    {0x098E, 0x481C},     //fd_period_60Hz (A)
    {0x0990, 0x00C8},     //      = 200
    {0x098E, 0xC81E},     //fd_search_f1_50hz (A)
    {0x0990, 0x0020},     //      = 32
    {0x098E, 0xC81F},     //fd_search_f2_50hz (A)
    {0x0990, 0x0022},     //      = 34
    {0x098E, 0xC820},     //fd_search_f1_60hz (A)
    {0x0990, 0x0027},     //      = 39
    {0x098E, 0xC821},     //fd_search_f2_60hz (A)
    {0x0990, 0x0029},     //      = 41
    {0x098E, 0x4847},     //fd_period_50Hz (B)
    {0x0990, 0x0077},     //      = 119
    {0x098E, 0x4849},     //fd_period_60Hz (B)
    {0x0990, 0x0063},     //      = 99
    {0x098E, 0xC84B},     //fd_search_f1_50hz (B)
    {0x0990, 0x000F},     //      = 15
    {0x098E, 0xC84C},     //fd_search_f2_50hz (B)
    {0x0990, 0x0011},     //      = 17
    {0x098E, 0xC84D},     //fd_search_f1_60hz (B)
    {0x0990, 0x0012},     //      = 18
    {0x098E, 0xC84E},     //fd_search_f2_60hz (B)
    {0x0990, 0x0014},     //      = 20
    {0x098E, 0x6800},     //Output Width (A)
    {0x0990, 0x0288},     //      = 640
    {0x098E, 0x6802},     //Output Height (A)
    {0x0990, 0x01E6},     //      = 480
    {0x098E, 0x6804},     //FOV Width (A)
    {0x0990, 0x0400},     //      = 1024
    {0x098E, 0x6806},     //FOV Height (A)
    {0x0990, 0x0300},     //      = 768
    {0x098E, 0xE892},     //JPEG Mode (A)
    {0x0990, 0x0000},     //      = 0
    {0x098E, 0x6C00},     //Output Width (B)
    {0x0990, 0x0804},     //      = 2048
    {0x098E, 0x6C02},     //Output Height (B)
    {0x0990, 0x0608},     //      = 1536
    {0x098E, 0x6C04},     //FOV Width (B)
    {0x0990, 0x0804},     //      = 2048
    {0x098E, 0x6C06},     //FOV Height (B)
    {0x0990, 0x0608},     //      = 1536
    {0x098E, 0xEC92},     //JPEG Mode (B)
    {0x0990, 0x0000},     //      = 0
	
    {0x3172, 0x0033}, // ANALOG_CONTROL2
  
    //TX
    {0x3C86, 0x00E1}, // OB_PCLK1_CONFIG
    {0x3C20, 0x0000}, // TX_SS_CONTROL

   //AE
    {0x098E, 0x6820},     // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_FDZONE]
    {0x0990, 0x0007},     // MCU_DATA_0
    {0x098E, 0x6822},     // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_AGAIN]
    {0x0990, 0x0064},     // MCU_DATA_0
    {0x098E, 0x6824},     // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_DGAIN]
    {0x0990, 0x0080},     // MCU_DATA_0
    {0x098E, 0xE826},     // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_BASE_TARGET]
    {0x0990, 0x003C},     // MCU_DATA_0                                                              //0x0045
    {0x098E, 0x6829},     // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MIN_VIRT_DGAIN]
    {0x0990, 0x0080},     // MCU_DATA_0
    {0x098E, 0x682B},     // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MAX_VIRT_DGAIN]
    {0x0990, 0x0080},     // MCU_DATA_0
    {0x098E, 0x682D},     // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MIN_VIRT_AGAIN]
    {0x0990, 0x0038},     // MCU_DATA_0
    {0x098E, 0x486F},     // MCU_ADDRESS [CAM1_CTL_MAX_ANALOG_GAIN]
    {0x0990, 0x0120},     // MCU_DATA_0
    {0x098E, 0x4871},     // MCU_ADDRESS [CAM1_CTL_MIN_ANALOG_GAIN]
    {0x0990, 0x0038},     // MCU_DATA_0
    {0x098E, 0x682F},     // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_AE_MAX_VIRT_AGAIN]
    {0x0990, 0x0120},     // MCU_DATA_0
    {0x098E, 0x6815},     // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_50HZ]
    {0x0990, 0x000A},     // MCU_DATA_0    0x000D
    {0x098E, 0x6817},     // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_60HZ]
    {0x0990, 0x000D},     // MCU_DATA_0
     //FD_set
    {0x098E, 0xA005},     // MCU_ADDRESS [FD_FDPERIOD_SELECT]
    {0x0990, 0x0001},     // MCU_DATA_0
    {0x098E, 0x680F},     // MCU_ADDRESS [PRI_A_CONFIG_FD_ALGO_ENTER]
    {0x0990, 0x0003},     // MCU_DATA_0
    {0x098E, 0xA006},     // MCU_ADDRESS [FD_SMOOTH_COUNTER]
    {0x0990, 0x0008},     // MCU_DATA_0
    {0x098E, 0xA007},     // MCU_ADDRESS [FD_STAT_MIN]
    {0x0990, 0x0003},     // MCU_DATA_0
    {0x098E, 0xA008},     // MCU_ADDRESS [FD_STAT_MAX]
    {0x0990, 0x0005},     // MCU_DATA_0
    {0x098E, 0xA00A},     // MCU_ADDRESS [FD_MIN_AMPLITUDE]
    {0x0990, 0x0000},     // MCU_DATA_0
     //AWB   
                                                        
{0x098E, 0x4873}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_0]         
{0x0990, 0x012B}, 	// MCU_DATA_0                             
{0x098E, 0x4875}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_1]         
{0x0990, 0xFF8A}, 	// MCU_DATA_0                             
{0x098E, 0x4877}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_2]         
{0x0990, 0x004A}, 	// MCU_DATA_0                             
{0x098E, 0x4879}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_3]         
{0x0990, 0xFFC7}, 	// MCU_DATA_0                             
{0x098E, 0x487B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_4]         
{0x0990, 0x014E}, 	// MCU_DATA_0                             
{0x098E, 0x487D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_5]         
{0x0990, 0xFFE9}, 	// MCU_DATA_0                             
{0x098E, 0x487F}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_6]         
{0x0990, 0xFFEB}, 	// MCU_DATA_0                             
{0x098E, 0x4881}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_7]         
{0x0990, 0xFF58}, 	// MCU_DATA_0                             
{0x098E, 0x4883}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_8]         
{0x0990, 0x01BB}, 	// MCU_DATA_0                             
{0x098E, 0x4885}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_9]         
{0x0990, 0x001B}, 	// MCU_DATA_0                             
{0x098E, 0x4887}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_10]        
{0x0990, 0x0050}, 	// MCU_DATA_0                             
{0x098E, 0x4889}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_0]        
{0x0990, 0x007D}, 	// MCU_DATA_0                             
{0x098E, 0x488B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_1]        
{0x0990, 0xFFF5}, 	// MCU_DATA_0                             
{0x098E, 0x488D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_2]        
{0x0990, 0xFF8B}, 	// MCU_DATA_0                             
{0x098E, 0x488F}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_3]        
{0x0990, 0x001C}, 	// MCU_DATA_0                             
{0x098E, 0x4891}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_4]        
{0x0990, 0xFFCB}, 	// MCU_DATA_0                             
{0x098E, 0x4893}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_5]        
{0x0990, 0x0017}, 	// MCU_DATA_0                             
{0x098E, 0x4895}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_6]        
{0x0990, 0x000E}, 	// MCU_DATA_0                             
{0x098E, 0x4897}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_7]        
{0x0990, 0x0039}, 	// MCU_DATA_0                             
{0x098E, 0x4899}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_8]        
{0x0990, 0xFFB8}, 	// MCU_DATA_0                             
{0x098E, 0x489B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_9]        
{0x0990, 0x0016}, 	// MCU_DATA_0                             
{0x098E, 0x489D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_10]       
{0x0990, 0xFFE3}, 	// MCU_DATA_0                             
{0x098E, 0x6857}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_X_START] 
{0x0990, 0x0000}, 	// MCU_DATA_0                             
{0x098E, 0x6859}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_Y_START] 
{0x0990, 0x0000}, 	// MCU_DATA_0                             
{0x098E, 0x685B}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_X_END]   
{0x0990, 0x03FF}, 	// MCU_DATA_0                             
{0x098E, 0x685D}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_Y_END]   
{0x0990, 0x02FF}, 	// MCU_DATA_0                             
{0x098E, 0x8400}, 	// MCU_ADDRESS [SEQ_CMD]                  
{0x0990, 0x0005}, 	// MCU_DATA_0                             

    {0x098E, 0xE876},     //MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0090},//b6     //MCU_DATA_0
        
    //[Patch_5_2]
    {0x0982, 0x0000},	  // ACCESS_CTL_STAT
    {0x098A, 0x0A80},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x3C3C},	
    {0x0992, 0xCE05},	
    {0x0994, 0x1F1F},	
    {0x0996, 0x0204},	
    {0x0998, 0x0CCC},	 
    {0x099A, 0x33D4},	 
    {0x099C, 0x30ED},	 
    {0x099E, 0x00FC},	 
    {0x098A, 0x0A90},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0590},	 
    {0x0992, 0xBDA8},	 
    {0x0994, 0x93CE},	 
    {0x0996, 0x051F},	 
    {0x0998, 0x1F02},	 
    {0x099A, 0x0110},	 
    {0x099C, 0xCC33},	 
    {0x099E, 0xD830},	 
    {0x098A, 0x0AA0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xED02},	 
    {0x0992, 0xCC05},	 
    {0x0994, 0xB8ED},	 
    {0x0996, 0x00C6},	 
    {0x0998, 0x06BD},	 
    {0x099A, 0xA8B1},	 
    {0x099C, 0xCE05},	 
    {0x099E, 0x1F1F},	 
    {0x098A, 0x0AB0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0208},	 
    {0x0992, 0x0CCC},	 
    {0x0994, 0x33D6},	 
    {0x0996, 0x30ED},	 
    {0x0998, 0x00FC},	 
    {0x099A, 0x0592},	 
    {0x099C, 0xBDA8},	 
    {0x099E, 0x93CC},	 
    {0x098A, 0x0AC0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x33F4},	
    {0x0992, 0x30ED},	
    {0x0994, 0x02CC},	
    {0x0996, 0xFFE9},	
    {0x0998, 0xED00},	
    {0x099A, 0xFC05},	
    {0x099C, 0x94C4},	
    {0x099E, 0x164F},	
    {0x098A, 0x0AD0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xBDA9},	
    {0x0992, 0x0ACE},	
    {0x0994, 0x051F},	
    {0x0996, 0x1F02},	
    {0x0998, 0x020A},	
    {0x099A, 0xCC32},	
    {0x099C, 0x1030},	
    {0x099E, 0xED00},	
    {0x098A, 0x0AE0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x4FBD},	
    {0x0992, 0xA8E4},	
    {0x0994, 0x3838},	
    {0x0996, 0x393C},	
    {0x0998, 0x3CFC},	
    {0x099A, 0x0322},	
    {0x099C, 0xB303},	
    {0x099E, 0x2030},	
    {0x098A, 0x0AF0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xED02},	
    {0x0992, 0xCE03},	
    {0x0994, 0x141F},	
    {0x0996, 0x0408},  
    {0x0998, 0x3ECE},	
    {0x099A, 0x0314},	
    {0x099C, 0x1F0B},	
    {0x099E, 0x0134},	
    {0x098A, 0x0B00},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x30EC},	
    {0x0992, 0x0227},	
    {0x0994, 0x2F83},	
    {0x0996, 0x0000},	
    {0x0998, 0x2C18},	
    {0x099A, 0xF603},	
    {0x099C, 0x244F},	
    {0x099E, 0xED00},	
    {0x098A, 0x0B10},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xFC03},	
    {0x0992, 0x20A3},	
    {0x0994, 0x00B3},	
    {0x0996, 0x0322},  
    {0x0998, 0x241A},	
    {0x099A, 0xFC03},	
    {0x099C, 0x22FD},	
    {0x099E, 0x0320},	
    {0x098A, 0x0B20},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x2012},	
    {0x0992, 0xF603},	
    {0x0994, 0x244F},	
    {0x0996, 0xF303},	
    {0x0998, 0x20B3},	
    {0x099A, 0x0322},	
    {0x099C, 0x2306},	
    {0x099E, 0xFC03},	
    {0x098A, 0x0B30},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x22FD},	
    {0x0992, 0x0320},	
    {0x0994, 0xBD7D},	
    {0x0996, 0x9038},	
    {0x0998, 0x3839},	
    {0x099A, 0x3C3C},	
    {0x099C, 0xFC07},	
    {0x099E, 0x4327},	
    {0x098A, 0x0B40},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x5FDE},	
    {0x0992, 0x431F},  
    {0x0994, 0xB410},  
    {0x0996, 0x563C},	
    {0x0998, 0xFC07},	
    {0x099A, 0x4130},	
    {0x099C, 0xED00},	
    {0x099E, 0x3CCC},	
    {0x098A, 0x0B50},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0008},	
    {0x0992, 0x30ED},	
    {0x0994, 0x00FC},	
    {0x0996, 0x0743},	
    {0x0998, 0xBDAA},	
    {0x099A, 0x7C38},  
    {0x099C, 0x38BD},	
    {0x099E, 0xE9E4},	
    {0x098A, 0x0B60},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x30ED},	
    {0x0992, 0x02CC},	
    {0x0994, 0x0064},	
    {0x0996, 0xED00},	
    {0x0998, 0xCC01},	
    {0x099A, 0x00BD},	
    {0x099C, 0xAA7C},	
    {0x099E, 0xFD03},	
    {0x098A, 0x0B70},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x103C},	
    {0x0992, 0xFC07},	
    {0x0994, 0x4530},	
    {0x0996, 0xED00},	
    {0x0998, 0x3CCC},	
    {0x099A, 0x0008},	
    {0x099C, 0x30ED},	
    {0x099E, 0x00FC},	
    {0x098A, 0x0B80},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0743},	
    {0x0992, 0xBDAA},	
    {0x0994, 0x7C38},	
    {0x0996, 0x38BD},	
    {0x0998, 0xE9E4},	
    {0x099A, 0x30ED},	
    {0x099C, 0x02CC},	
    {0x099E, 0x0064},	
    {0x098A, 0x0B90},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xED00},	
    {0x0992, 0xCC01},	
    {0x0994, 0x00BD},	
    {0x0996, 0xAA7C},	
    {0x0998, 0xFD03},	
    {0x099A, 0x1220},	
    {0x099C, 0x03BD},	
    {0x099E, 0x7993},	
    {0x098A, 0x0BA0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x3838},	
    {0x0992, 0x390F},	
    {0x0994, 0xF601},	
    {0x0996, 0x05C1},	
    {0x0998, 0x0326},	
    {0x099A, 0x14F6},	
    {0x099C, 0x0106},	
    {0x099E, 0xC106},	
    {0x098A, 0x0BB0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x260D},	
    {0x0992, 0xF630},	
    {0x0994, 0x4DC4},	
    {0x0996, 0xF0CA},	
    {0x0998, 0x08F7},	
    {0x099A, 0x304D},	
    {0x099C, 0xBD0B},	
    {0x099E, 0xC10E},	
    {0x098A, 0x0BC0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x39F6},	
    {0x0992, 0x304D},	
    {0x0994, 0xC4F0},	
    {0x0996, 0xCA09},	
    {0x0998, 0xF730},	
    {0x099A, 0x4DDE},	
    {0x099C, 0xF218},	
    {0x099E, 0xCE0A},	
    {0x098A, 0x0BD0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x00CC},	
    {0x0992, 0x001D},	
    {0x0994, 0xBDB5},	
    {0x0996, 0x31DE},	
    {0x0998, 0xA818},	
    {0x099A, 0xCE0A},	
    {0x099C, 0x1ECC},	
    {0x099E, 0x001D},	
    {0x098A, 0x0BE0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xBDB5},	
    {0x0992, 0x31DE},	
    {0x0994, 0xA618},	
    {0x0996, 0xCE0A},	
    {0x0998, 0x3CCC},  
    {0x099A, 0x0013},	
    {0x099C, 0xBDB5},	
    {0x099E, 0x31CC},	
    {0x098A, 0x0BF0},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0A80},	
    {0x0992, 0xFD0A},	
    {0x0994, 0x0ECC},	
    {0x0996, 0x0AE7},	
    {0x0998, 0xFD0A},	
    {0x099A, 0x30CC},	
    {0x099C, 0x0B3A},	
    {0x099E, 0xFD0A},	
    {0x098A, 0x0C00},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x4CCC},  
    {0x0992, 0x0A00},  
    {0x0994, 0xDDF2},  
    {0x0996, 0xCC0A},  
    {0x0998, 0x1EDD},  
    {0x099A, 0xA8CC},  
    {0x099C, 0x0A3C},  
    {0x099E, 0xDDA6},  
    {0x098A, 0x0C10},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0xC601},	
    {0x0992, 0xF701},	
    {0x0994, 0x0CF7},	
    {0x0996, 0x010D},	
    {0x098A, 0x8C18},	  // PHYSICAL_ADDR_ACCESS
    {0x0990, 0x0039},	  // MCU_DATA_0
    {0x098E, 0x0012},	  // MCU_ADDRESS [MON_ADDR]
    {0x0990, 0x0BA3},	  // MCU_DATA_0
    {0x098E, 0x0003},	  // MCU_ADDRESS [MON_ALGO]
    {0x0990, 0x0004},	  // MCU_DATA_0

    {0x0000, 0x0020}, // DELAY=100
    
    //POLL_FIELD=MON_RAM_PATCH_ID,==0,DELAY=10,TIMEOUT=100         // wait for the patch to complete initialization 
    
    //Char_settings]
    {0x3ED6, 0x0F00},     // RESERVED
    {0x3EF2, 0xD965},     // RESERVED
    {0x3FD2, 0xD965},     // RESERVED
    {0x3EF8, 0x7F7F},     // RESERVED
    {0x3ED8, 0x7F1D},     // RESERVED
    {0x3172, 0x0033},     // RESERVED
    {0x3EEA, 0x0200},     // RESERVED
    {0x3EE2, 0x0050},     // RESERVED
    {0x316A, 0x8200},     // RESERVED
    {0x316C, 0x8200},     // RESERVED
    {0x3EFC, 0xA8E8},     // RESERVED
    {0x3EFE, 0x130D},     // RESERVED
    // Additional Optimized Settings
    {0x3180, 0xB3FF},     // RESERVED
    {0x30B2, 0xC000},     // RESERVED
    {0x30BC, 0x0384},     // RESERVED
    {0x30C0, 0x1220},     // RESERVED
     // Low_Power_Mode
    {0x3170, 0x000A},     //Dynamic pwr setting
    {0x3174, 0x8060},     //Dynamic pwr setting
    {0x3ECC, 0x22B0},     //Dynamic pwr setting
    {0x098E, 0x482B},     //LP Mode (A)
    {0x0990, 0x22B0},     //
    {0x098E, 0x4858},     //LP Mode (B)
    {0x0990, 0x22B0},   
    {0x317A, 0x000A},   
    {0x098E, 0x4822},   
    {0x0990, 0x000A},   
    {0x098E, 0x4824},   
    {0x0990, 0x000A},  
    {0x098E, 0x484F},   
    {0x0990, 0x000A},   
    {0x098E, 0x4851},   
    {0x0990, 0x000A}, 

//// 
//[Lens Correction 85% 03/13/12 13:37:14]
//cwf
{0x3210, 0x01B0}, 	// COLOR_PIPELINE_CONTROL
{0x3640, 0x01D0}, 	// P_G1_P0Q0
{0x3642, 0x1B8E}, 	// P_G1_P0Q1
{0x3644, 0x2D91}, 	// P_G1_P0Q2
{0x3646, 0x60CC}, 	// P_G1_P0Q3
{0x3648, 0xDC31}, 	// P_G1_P0Q4
{0x364A, 0x01F0}, 	// P_R_P0Q0
{0x364C, 0x9E8E}, 	// P_R_P0Q1
{0x364E, 0x7FD0}, 	// P_R_P0Q2
{0x3650, 0x14F0}, 	// P_R_P0Q3
{0x3652, 0x90B1}, 	// P_R_P0Q4
{0x3654, 0x0290}, 	// P_B_P0Q0
{0x3656, 0x74CE}, 	// P_B_P0Q1
{0x3658, 0x680F}, 	// P_B_P0Q2
{0x365A, 0xD64F}, 	// P_B_P0Q3
{0x365C, 0x9D2E}, 	// P_B_P0Q4
{0x365E, 0x01B0}, 	// P_G2_P0Q0
{0x3660, 0xC38E}, 	// P_G2_P0Q1
{0x3662, 0x3C91}, 	// P_G2_P0Q2
{0x3664, 0x474F}, 	// P_G2_P0Q3
{0x3666, 0xEF11}, 	// P_G2_P0Q4
{0x3680, 0x8FCD}, 	// P_G1_P1Q0
{0x3682, 0xAFAE}, 	// P_G1_P1Q1
{0x3684, 0xE9AF}, 	// P_G1_P1Q2
{0x3686, 0x562E}, 	// P_G1_P1Q3
{0x3688, 0x5551}, 	// P_G1_P1Q4
{0x368A, 0x808D}, 	// P_R_P1Q0
{0x368C, 0x2A2E}, 	// P_R_P1Q1
{0x368E, 0x92AE}, 	// P_R_P1Q2
{0x3690, 0xD00F}, 	// P_R_P1Q3
{0x3692, 0x4370}, 	// P_R_P1Q4
{0x3694, 0x2FCD}, 	// P_B_P1Q0
{0x3696, 0x090F}, 	// P_B_P1Q1
{0x3698, 0x59CF}, 	// P_B_P1Q2
{0x369A, 0x9770}, 	// P_B_P1Q3
{0x369C, 0xA111}, 	// P_B_P1Q4
{0x369E, 0x1D8D}, 	// P_G2_P1Q0
{0x36A0, 0xEE8E}, 	// P_G2_P1Q1
{0x36A2, 0x4D8F}, 	// P_G2_P1Q2
{0x36A4, 0x0430}, 	// P_G2_P1Q3
{0x36A6, 0xE970}, 	// P_G2_P1Q4
{0x36C0, 0x4851}, 	// P_G1_P2Q0
{0x36C2, 0x0B90}, 	// P_G1_P2Q1
{0x36C4, 0xAAD1}, 	// P_G1_P2Q2
{0x36C6, 0xEFEE}, 	// P_G1_P2Q3
{0x36C8, 0x9B93}, 	// P_G1_P2Q4
{0x36CA, 0x38D1}, 	// P_R_P2Q0
{0x36CC, 0xF30F}, 	// P_R_P2Q1
{0x36CE, 0xB7EC}, 	// P_R_P2Q2
{0x36D0, 0x2911}, 	// P_R_P2Q3
{0x36D2, 0xD153}, 	// P_R_P2Q4
{0x36D4, 0x0451}, 	// P_B_P2Q0
{0x36D6, 0x0410}, 	// P_B_P2Q1
{0x36D8, 0x3E71}, 	// P_B_P2Q2
{0x36DA, 0x42CF}, 	// P_B_P2Q3
{0x36DC, 0xE5D3}, 	// P_B_P2Q4
{0x36DE, 0x3AB1}, 	// P_G2_P2Q0
{0x36E0, 0xBF8F}, 	// P_G2_P2Q1
{0x36E2, 0xBF50}, 	// P_G2_P2Q2
{0x36E4, 0x2491}, 	// P_G2_P2Q3
{0x36E6, 0xDED3}, 	// P_G2_P2Q4
{0x3700, 0x2F8C}, 	// P_G1_P3Q0
{0x3702, 0x760E}, 	// P_G1_P3Q1
{0x3704, 0x0F12}, 	// P_G1_P3Q2
{0x3706, 0xDF6F}, 	// P_G1_P3Q3
{0x3708, 0xBF53}, 	// P_G1_P3Q4
{0x370A, 0x758E}, 	// P_R_P3Q0
{0x370C, 0x862D}, 	// P_R_P3Q1
{0x370E, 0xC48F}, 	// P_R_P3Q2
{0x3710, 0xD5CF}, 	// P_R_P3Q3
{0x3712, 0x2B8F}, 	// P_R_P3Q4
{0x3714, 0xCE4F}, 	// P_B_P3Q0
{0x3716, 0x9A90}, 	// P_B_P3Q1
{0x3718, 0x35AF}, 	// P_B_P3Q2
{0x371A, 0x08F1}, 	// P_B_P3Q3
{0x371C, 0x2451}, 	// P_B_P3Q4
{0x371E, 0x0469}, 	// P_G2_P3Q0
{0x3720, 0x58AF}, 	// P_G2_P3Q1
{0x3722, 0xB050}, 	// P_G2_P3Q2
{0x3724, 0xC7D1}, 	// P_G2_P3Q3
{0x3726, 0x1052}, 	// P_G2_P3Q4
{0x3740, 0xD992}, 	// P_G1_P4Q0
{0x3742, 0xE8D1}, 	// P_G1_P4Q1
{0x3744, 0xE033}, 	// P_G1_P4Q2
{0x3746, 0x53B2}, 	// P_G1_P4Q3
{0x3748, 0x6D15}, 	// P_G1_P4Q4
{0x374A, 0x9072}, 	// P_R_P4Q0
{0x374C, 0x79D1}, 	// P_R_P4Q1
{0x374E, 0xC3B3}, 	// P_R_P4Q2
{0x3750, 0xF7B2}, 	// P_R_P4Q3
{0x3752, 0x5595}, 	// P_R_P4Q4
{0x3754, 0xA6F2}, 	// P_B_P4Q0
{0x3756, 0xBF12}, 	// P_B_P4Q1
{0x3758, 0x9B14}, 	// P_B_P4Q2
{0x375A, 0x3072}, 	// P_B_P4Q3
{0x375C, 0x09F6}, 	// P_B_P4Q4
{0x375E, 0xBA72}, 	// P_G2_P4Q0
{0x3760, 0x1812}, 	// P_G2_P4Q1
{0x3762, 0xDE74}, 	// P_G2_P4Q2
{0x3764, 0xD892}, 	// P_G2_P4Q3
{0x3766, 0x52D6}, 	// P_G2_P4Q4
{0x3782, 0x02F0}, 	// CENTER_ROW
{0x3784, 0x03A0}, 	// CENTER_COLUMN
{0x3210, 0x01B8}, 	// COLOR_PIPELINE_CONTROL
///
 /*   
    //low_light
    {0x098E, 0x4918},	   // MCU_ADDRESS [CAM1_LL_START_GAIN_METRIC]
    {0x0990, 0x0039},	   // MCU_DATA_0
    {0x098E, 0x491A},	   // MCU_ADDRESS [CAM1_LL_STOP_GAIN_METRIC]
    {0x0990, 0x0100},	   // MCU_DATA_0
    {0x098E, 0x6872},	   // MCU_ADDRESS [PRI_A_CONFIG_LL_START_BRIGHTNESS]
    {0x0990, 0x0005},	   // MCU_DATA_0
    {0x098E, 0x6874},	   // MCU_ADDRESS [PRI_A_CONFIG_LL_STOP_BRIGHTNESS]
    {0x0990, 0x008C},	   // MCU_DATA_0
    {0x098E, 0x4956},	   // MCU_ADDRESS [CAM1_LL_DC_START_GAIN_METRIC]
    {0x0990, 0x0040},	   // MCU_DATA_0
    {0x098E, 0x4958},	   // MCU_ADDRESS [CAM1_LL_DC_STOP_GAIN_METRIC]
    {0x0990, 0x0100},	   // MCU_DATA_0
    {0x098E, 0x495A},	   // MCU_ADDRESS [CAM1_LL_DC_START]
    {0x0990, 0x0000},	   // MCU_DATA_0
    {0x098E, 0x495C},	   // MCU_ADDRESS [CAM1_LL_DC_STOP]
    {0x0990, 0x0000},	   // MCU_DATA_0
    {0x098E, 0x495E},	   // MCU_ADDRESS [CAM1_LL_CDC_AGG_START_GAIN_METRIC]
    {0x0990, 0x0040},	   // MCU_DATA_0
    {0x098E, 0x4960},	   // MCU_ADDRESS [CAM1_LL_CDC_AGG_STOP_GAIN_METRIC]
    {0x0990, 0x0100},	   // MCU_DATA_0
    {0x098E, 0xC962},	   // MCU_ADDRESS [CAM1_LL_CDC_AGG_START]
    {0x0990, 0x0000},	   // MCU_DATA_0
    {0x098E, 0xC963},	   // MCU_ADDRESS [CAM1_LL_CDC_AGG_STOP]
    {0x0990, 0x0003},	   // MCU_DATA_0
    {0x098E, 0x4964},	   // MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_START_GAIN_METRIC]
    {0x0990, 0x0040},	   // MCU_DATA_0
    {0x098E, 0x4966},	   // MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_STOP_GAIN_METRIC]
    {0x0990, 0x0100},	   // MCU_DATA_0
    {0x098E, 0x4968},	   // MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_T3START]
    {0x0990, 0x0001},	   // MCU_DATA_0
    {0x098E, 0x496A},	   // MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_T3STOP]
    {0x0990, 0x0001},	   // MCU_DATA_0
    {0x098E, 0x496C},	   // MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_T4START]
    {0x0990, 0x0014},	   // MCU_DATA_0
    {0x098E, 0x496E},	   // MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_T4STOP]
    {0x0990, 0x000C},	   // MCU_DATA_0
    {0x098E, 0xC970},	   // MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_TO_START]
    {0x0990, 0x0004},	   // MCU_DATA_0
    {0x098E, 0xC971},	   // MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_TO_STOP]
    {0x0990, 0x000F},	   // MCU_DATA_0
    {0x098E, 0x4972},	   // MCU_ADDRESS [CAM1_LL_CDC_DARK_START_GAIN_METRIC]
    {0x0990, 0x0040},	   // MCU_DATA_0
    {0x098E, 0x4974},	   // MCU_ADDRESS [CAM1_LL_CDC_DARK_STOP_GAIN_METRIC]
    {0x0990, 0x0100},	   // MCU_DATA_0
    {0x098E, 0x4976},	   // MCU_ADDRESS [CAM1_LL_CDC_DARK_T3START]
    {0x0990, 0x0001},	   // MCU_DATA_0
    {0x098E, 0x4978},	   // MCU_ADDRESS [CAM1_LL_CDC_DARK_T3STOP]
    {0x0990, 0x0001},	   // MCU_DATA_0
    {0x098E, 0x497A},	   // MCU_ADDRESS [CAM1_LL_CDC_DARK_T4START]
    {0x0990, 0x00C8},	   // MCU_DATA_0
    {0x098E, 0x497C},	   // MCU_ADDRESS [CAM1_LL_CDC_DARK_T4STOP]
    {0x0990, 0x003C},	   // MCU_DATA_0
    {0x098E, 0xC97E},	   // MCU_ADDRESS [CAM1_LL_CDC_DARK_TO_START]
    {0x0990, 0x0004},	   // MCU_DATA_0
    {0x098E, 0xC97F},	   // MCU_ADDRESS [CAM1_LL_CDC_DARK_TO_STOP]
    {0x0990, 0x000F},	   // MCU_DATA_0
    {0x098E, 0x491C},	   // MCU_ADDRESS [CAM1_LL_GRB_START_GAIN_METRIC]
    {0x0990, 0x0040},	   // MCU_DATA_0
    {0x098E, 0x491E},	   // MCU_ADDRESS [CAM1_LL_GRB_STOP_GAIN_METRIC]
    {0x0990, 0x0100},	   // MCU_DATA_0
    {0x098E, 0xC920},	   // MCU_ADDRESS [CAM1_LL_GRB_SLOPE_START]
    {0x0990, 0x000B},	   // MCU_DATA_0
    {0x098E, 0xC921},	   // MCU_ADDRESS [CAM1_LL_GRB_SLOPE_STOP]
    {0x0990, 0x002C},	   // MCU_DATA_0
    {0x098E, 0xC922},	   // MCU_ADDRESS [CAM1_LL_GRB_OFFSET_START]
    {0x0990, 0x0007},	   // MCU_DATA_0
    {0x098E, 0xC923},	   // MCU_ADDRESS [CAM1_LL_GRB_OFFSET_STOP]
    {0x0990, 0x001D},	   // MCU_DATA_0
    {0x098E, 0x4926},	   // MCU_ADDRESS [CAM1_LL_SFFB_START_ANALOG_GAIN]
    {0x0990, 0x0020},	   // MCU_DATA_0   //0039
    {0x098E, 0x4928},	   // MCU_ADDRESS [CAM1_LL_SFFB_END_ANALOG_GAIN]
    {0x0990, 0x0020},	   // MCU_DATA_0   //00A0
    {0x098E, 0x492A},	   // MCU_ADDRESS [CAM1_LL_SFFB_RAMP_START]
    {0x0990, 0x0082},	   // MCU_DATA_0
    {0x098E, 0x492C},	   // MCU_ADDRESS [CAM1_LL_SFFB_RAMP_STOP]
    {0x0990, 0x0040},	   // MCU_DATA_0
    {0x098E, 0x492E},	   // MCU_ADDRESS [CAM1_LL_SFFB_SLOPE_START]
    {0x0990, 0x0015},	   // MCU_DATA_0
    {0x098E, 0x4930},	   // MCU_ADDRESS [CAM1_LL_SFFB_SLOPE_STOP]
    {0x0990, 0x0015},	   // MCU_DATA_0
    {0x098E, 0x4932},	   // MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH1START]
    {0x0990, 0x0002},	   // MCU_DATA_0
    {0x098E, 0x4934},	   // MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH1STOP]
    {0x0990, 0x0004},	   // MCU_DATA_0
    {0x098E, 0x4936},	   // MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH2START]
    {0x0990, 0x0008},	   // MCU_DATA_0
    {0x098E, 0x4938},	   // MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH2STOP]
    {0x0990, 0x0009},	   // MCU_DATA_0
    {0x098E, 0x493A},	   // MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH3START]
    {0x0990, 0x000C},	   // MCU_DATA_0
    {0x098E, 0x493C},	   // MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH3STOP]
    {0x0990, 0x000D},	   // MCU_DATA_0
    {0x098E, 0x493E},	   // MCU_ADDRESS [CAM1_LL_SFFB_MAX_THRESH_START]
    {0x0990, 0x0015},	   // MCU_DATA_0
    {0x098E, 0x4940},	   // MCU_ADDRESS [CAM1_LL_SFFB_MAX_THRESH_STOP]
    {0x0990, 0x0013},	   // MCU_DATA_0
    {0x098E, 0xC944},	   // MCU_ADDRESS [CAM1_LL_SFFB_FLATNESS_START]
    {0x0990, 0x0023},	   // MCU_DATA_0
    {0x098E, 0xC945},	   // MCU_ADDRESS [CAM1_LL_SFFB_FLATNESS_STOP]
    {0x0990, 0x007F},	   // MCU_DATA_0
    {0x098E, 0xC946},	   // MCU_ADDRESS [CAM1_LL_SFFB_TRANSITION_START]
    {0x0990, 0x0007},	   // MCU_DATA_0
    {0x098E, 0xC947},	   // MCU_ADDRESS [CAM1_LL_SFFB_TRANSITION_STOP]
    {0x0990, 0x0001},	   // MCU_DATA_0
    {0x098E, 0xC948},	   // MCU_ADDRESS [CAM1_LL_SFFB_SOBEL_FLAT_START]
    {0x0990, 0x0002},	   // MCU_DATA_0
    {0x098E, 0xC949},	   // MCU_ADDRESS [CAM1_LL_SFFB_SOBEL_FLAT_STOP]
    {0x0990, 0x0002},	   // MCU_DATA_0
    {0x098E, 0xC94A},	   // MCU_ADDRESS [CAM1_LL_SFFB_SOBEL_SHARP_START]
    {0x0990, 0x00FF},	   // MCU_DATA_0
    {0x098E, 0xC94B},	   // MCU_ADDRESS [CAM1_LL_SFFB_SOBEL_SHARP_STOP]
    {0x0990, 0x00FF},	   // MCU_DATA_0
    {0x098E, 0xC906},	   // MCU_ADDRESS [CAM1_LL_DM_EDGE_TH_START]
    {0x0990, 0x0006},	   // MCU_DATA_0
    {0x098E, 0xC907},	   // MCU_ADDRESS [CAM1_LL_DM_EDGE_TH_STOP]
    {0x0990, 0x0028},	   // MCU_DATA_0
    {0x098E, 0xBC02},	   // MCU_ADDRESS [LL_MODE]
    {0x0990, 0x0005},	   // MCU_DATA_0
    {0x098E, 0xC908},	   // MCU_ADDRESS [CAM1_LL_AP_KNEE_START]
    {0x0990, 0x0006},	   // MCU_DATA_0
    {0x098E, 0xC909},	   // MCU_ADDRESS [CAM1_LL_AP_KNEE_STOP]
    {0x0990, 0x0028},	   // MCU_DATA_0
    {0x098E, 0xC90A},	   // MCU_ADDRESS [CAM1_LL_AP_MANTISSA_START]
    {0x0990, 0x0007},	   // MCU_DATA_0
    {0x326C, 0x0F0A},	   // APERTURE_PARAMETERS_2D
    {0x098E, 0xC94C},	   // MCU_ADDRESS [CAM1_LL_DELTA_GAIN]
    {0x0990, 0x0003},	   // MCU_DATA_0
    {0x098E, 0xC94E},	   // MCU_ADDRESS [CAM1_LL_DELTA_THRESHOLD_START]
    {0x0990, 0x003C},	   // MCU_DATA_0
    {0x098E, 0xC94F},	   // MCU_ADDRESS [CAM1_LL_DELTA_THRESHOLD_STOP]
    {0x0990, 0x0064},	   // MCU_DATA_0
    {0x098E, 0xE877},	   // MCU_ADDRESS [PRI_A_CONFIG_LL_END_SATURATION]
    {0x0990, 0x0050},	   // MCU_DATA_0
 */
//gamma
    {0x098E, 0x3C42},	   // MCU_ADDRESS [LL_START_GAMMA_FTB]
    {0x0990, 0x004B},	   // MCU_DATA_0
    {0x098E, 0x3C44},	   // MCU_ADDRESS [LL_STOP_GAMMA_FTB]
    {0x0990, 0x0037},	   // MCU_DATA_0
    {0x098E, 0x4912},	   // MCU_ADDRESS [CAM1_LL_START_GAMMA_BM]
    {0x0990, 0x0001},	   // MCU_DATA_0
    {0x098E, 0x4914},	   // MCU_ADDRESS [CAM1_LL_MID_GAMMA_BM]
    {0x0990, 0x015E},	   // MCU_DATA_0
    {0x098E, 0x4916},	   // MCU_ADDRESS [CAM1_LL_STOP_GAMMA_BM]
    {0x0990, 0x03E8},	   // MCU_DATA_0
    
   {0x098E, 0xBC09},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_0]
   {0x0990, 0x0000},    // MCU_DATA_0
   {0x098E, 0xBC0A},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_1]
   {0x0990, 0x001D},    // MCU_DATA_0
   {0x098E, 0xBC0B},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_2]
   {0x0990, 0x002B},    // MCU_DATA_0
   {0x098E, 0xBC0C},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_3]
   {0x0990, 0x0041},    // MCU_DATA_0
   {0x098E, 0xBC0D},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_4]
   {0x0990, 0x0064},    // MCU_DATA_0
   {0x098E, 0xBC0E},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_5]
   {0x0990, 0x0083},    // MCU_DATA_0
   {0x098E, 0xBC0F},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_6]
   {0x0990, 0x009C},    // MCU_DATA_0
   {0x098E, 0xBC10},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_7]
   {0x0990, 0x00AF},    // MCU_DATA_0
   {0x098E, 0xBC11},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_8]
   {0x0990, 0x00BD},    // MCU_DATA_0
   {0x098E, 0xBC12},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_9]
   {0x0990, 0x00C9},    // MCU_DATA_0
   {0x098E, 0xBC13},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_10]
   {0x0990, 0x00D3},    // MCU_DATA_0
   {0x098E, 0xBC14},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_11]
   {0x0990, 0x00DB},    // MCU_DATA_0
   {0x098E, 0xBC15},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_12]
   {0x0990, 0x00E2},    // MCU_DATA_0
   {0x098E, 0xBC16},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_13]
   {0x0990, 0x00E8},    // MCU_DATA_0
   {0x098E, 0xBC17},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_14]
   {0x0990, 0x00ED},    // MCU_DATA_0
   {0x098E, 0xBC18},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_15]
   {0x0990, 0x00F2},    // MCU_DATA_0
   {0x098E, 0xBC19},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_16]
   {0x0990, 0x00F7},    // MCU_DATA_0
   {0x098E, 0xBC1A},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_17]
   {0x0990, 0x00FB},    // MCU_DATA_0
   {0x098E, 0xBC1B},    // MCU_ADDRESS [LL_GAMMA_CONTRAST_CURVE_18]
   {0x0990, 0x00FF},    // MCU_DATA_0
   {0x098E, 0x8400},    // MCU_ADDRESS [SEQ_CMD]
   {0x0990, 0x0005},    // MCU_DATA_0
   
       
    {0x098E, 0xAC02},	   // MCU_ADDRESS [AWB_MODE]
    {0x0990, 0x0006},	   // MCU_DATA_0
    {0x098E, 0x2800},	   // MCU_ADDRESS [AE_TRACK_STATUS]
    {0x0990, 0x001C},	   // MCU_DATA_0
    {0x098E, 0x8400},	   // MCU_ADDRESS
    {0x0990, 0x0006},	   // MCU_DATA_0
            
    //Optimized
    //50Hz
    {0x098E, 0x2003}, // MCU_ADDRESS [FD_ALGO]
    {0x0990, 0x0002}, // MCU_DATA_0
    {0x098E, 0xA005}, // MCU_ADDRESS [FD_FDPERIOD_SELECT]
    {0x0990, 0x0000}, // MCU_DATA_0   0:60Hz  1:50Hz
    //sharpness
    {0x326a, 0x1408}, 	// COLOR_PIPELINE_CONTROL
    
    {0x098E, 0x8400}, //Refresh Sequencer Mode
    {0x0990, 0x0006}, //      = 6
    //{0x0000, 0x0020}, //DELAY=200
    
    //liuli rotate 180
    {0x098E, 0x4810},          // MCU_ADDRESS [CAM1_CTX_A_READ_MODE][Rotate 180 Flip]
    {0x0990, 0x046F},          // MCU_DATA_0
    {0x098E, 0x483D},         // MCU_ADDRESS [CAM1_CTX_B_READ_MODE]
    {0x0990, 0x0027},          // MCU_DATA_0
    {0x098E, 0x8400},          // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},          // MCU_DATA_0
    
    {0x0018, 0x002A}, // STANDBY_CONTROL_AND_STATUS
    {0x0000, 0x0040}, // DELAY=100    
    //STATE= Detect Master Clock, 1
    {0xA807, 0x0000},//2c
    
    //saturation
    {0x098E, 0xC997}, 	// MCU_ADDRESS [CAM1_SYS_UV_COLOR_BOOST]
    {0x0990, 0x0005}, 	// MCU_DATA_0
    {0x098E, 0x5C03}, 	// MCU_ADDRESS [SYS_ALGO]
    {0x0990, 0x000B}, 	// MCU_DATA_0
    
   // {0x098E, 0x48BE}, 	// MCU_ADDRESS [CAM1_AWB_RECIP_YSCALE]
   // {0x0990, 0x0F5B}, 	// MCU_DATA_0

    {0x098E, 0x48BE}, 	// MCU_ADDRESS [CAM1_AWB_RECIP_YSCALE]
    {0x0990, 0x10F5}, 	// MCU_DATA_0
    {0x098E, 0x48BA}, 	// MCU_ADDRESS [CAM1_AWB_RECIP_YSCALE]
    {0x0990, 0x0017}, 	// MCU_DATA_0
    {0x098E, 0x8400}, 	// MCU_ADDRESS [CAM1_AWB_RECIP_YSCALE]
    {0x0990, 0x0005}, 	// MCU_DATA_0

     {0x0000, 0x0010}, // DELAY=100 


//wxj
{0x098E, 0x4918}, 	// MCU_ADDRESS [CAM1_LL_START_GAIN_METRIC]
{0x0990, 0x0040}, 	// MCU_DATA_0
{0x098E, 0x491A}, 	// MCU_ADDRESS [CAM1_LL_STOP_GAIN_METRIC]
{0x0990, 0x01F4}, 	// MCU_DATA_0
{0x098E, 0x6872}, 	// MCU_ADDRESS [PRI_A_CONFIG_LL_START_BRIGHTNESS]
{0x0990, 0x0005}, 	// MCU_DATA_0
{0x098E, 0x6874}, 	// MCU_ADDRESS [PRI_A_CONFIG_LL_STOP_BRIGHTNESS]
{0x0990, 0x008C}, 	// MCU_DATA_0
{0x098E, 0x4956}, 	// MCU_ADDRESS [CAM1_LL_DC_START_GAIN_METRIC]
{0x0990, 0x0040}, 	// MCU_DATA_0
{0x098E, 0x4958}, 	// MCU_ADDRESS [CAM1_LL_DC_STOP_GAIN_METRIC]
{0x0990, 0x01F4}, 	// MCU_DATA_0
{0x098E, 0x495A}, 	// MCU_ADDRESS [CAM1_LL_DC_START]
{0x0990, 0x0000}, 	// MCU_DATA_0
{0x098E, 0x495C}, 	// MCU_ADDRESS [CAM1_LL_DC_STOP]
{0x0990, 0x0000}, 	// MCU_DATA_0
{0x098E, 0x495E}, 	// MCU_ADDRESS [CAM1_LL_CDC_AGG_START_GAIN_METRIC]
{0x0990, 0x0040}, 	// MCU_DATA_0
{0x098E, 0x4960}, 	// MCU_ADDRESS [CAM1_LL_CDC_AGG_STOP_GAIN_METRIC]
{0x0990, 0x01F4}, 	// MCU_DATA_0
{0x098E, 0xC962}, 	// MCU_ADDRESS [CAM1_LL_CDC_AGG_START]
{0x0990, 0x0000}, 	// MCU_DATA_0
{0x098E, 0xC963}, 	// MCU_ADDRESS [CAM1_LL_CDC_AGG_STOP]
{0x0990, 0x0003}, 	// MCU_DATA_0
{0x098E, 0x4964}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_START_GAIN_METRIC]
{0x0990, 0x0040}, 	// MCU_DATA_0
{0x098E, 0x4966}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_STOP_GAIN_METRIC]
{0x0990, 0x01F4}, 	// MCU_DATA_0
{0x098E, 0x4968}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_T3START]
{0x0990, 0x0001}, 	// MCU_DATA_0
{0x098E, 0x496A}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_T3STOP]
{0x0990, 0x0001}, 	// MCU_DATA_0
{0x098E, 0x496C}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_T4START]
{0x0990, 0x00C8}, 	// MCU_DATA_0
{0x098E, 0x496E}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_T4STOP]
{0x0990, 0x003C}, 	// MCU_DATA_0
{0x098E, 0xC970}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_TO_START]
{0x0990, 0x0004}, 	// MCU_DATA_0
{0x098E, 0xC971}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_TO_STOP]
{0x0990, 0x000F}, 	// MCU_DATA_0
{0x098E, 0x4972}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_START_GAIN_METRIC]
{0x0990, 0x0040}, 	// MCU_DATA_0
{0x098E, 0x4974}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_STOP_GAIN_METRIC]
{0x0990, 0x01F4}, 	// MCU_DATA_0
{0x098E, 0x4976}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_T3START]
{0x0990, 0x0001}, 	// MCU_DATA_0
{0x098E, 0x4978}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_T3STOP]
{0x0990, 0x0001}, 	// MCU_DATA_0
{0x098E, 0x497A}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_T4START]
{0x0990, 0x00C8}, 	// MCU_DATA_0
{0x098E, 0x497C}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_T4STOP]
{0x0990, 0x003C}, 	// MCU_DATA_0
{0x098E, 0xC97E}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_TO_START]
{0x0990, 0x0004}, 	// MCU_DATA_0
{0x098E, 0xC97F}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_TO_STOP]
{0x0990, 0x000F}, 	// MCU_DATA_0
{0x098E, 0x491C}, 	// MCU_ADDRESS [CAM1_LL_GRB_START_GAIN_METRIC]
{0x0990, 0x0040}, 	// MCU_DATA_0
{0x098E, 0x491E}, 	// MCU_ADDRESS [CAM1_LL_GRB_STOP_GAIN_METRIC]
{0x0990, 0x01F4}, 	// MCU_DATA_0
{0x098E, 0xC920}, 	// MCU_ADDRESS [CAM1_LL_GRB_SLOPE_START]
{0x0990, 0x000B}, 	// MCU_DATA_0
{0x098E, 0xC921}, 	// MCU_ADDRESS [CAM1_LL_GRB_SLOPE_STOP]
{0x0990, 0x002C}, 	// MCU_DATA_0
{0x098E, 0xC922}, 	// MCU_ADDRESS [CAM1_LL_GRB_OFFSET_START]
{0x0990, 0x0007}, 	// MCU_DATA_0
{0x098E, 0xC923}, 	// MCU_ADDRESS [CAM1_LL_GRB_OFFSET_STOP]
{0x0990, 0x001D}, 	// MCU_DATA_0
{0x098E, 0x4926}, 	// MCU_ADDRESS [CAM1_LL_SFFB_START_ANALOG_GAIN]
{0x0990, 0x0040}, 	// MCU_DATA_0
{0x098E, 0x4928}, 	// MCU_ADDRESS [CAM1_LL_SFFB_END_ANALOG_GAIN]
{0x0990, 0x01F4}, 	// MCU_DATA_0
{0x098E, 0x492A}, 	// MCU_ADDRESS [CAM1_LL_SFFB_RAMP_START]
{0x0990, 0x0078}, 	// MCU_DATA_0
{0x098E, 0x492C}, 	// MCU_ADDRESS [CAM1_LL_SFFB_RAMP_STOP]
{0x0990, 0x0040}, 	// MCU_DATA_0
{0x098E, 0x492E}, 	// MCU_ADDRESS [CAM1_LL_SFFB_SLOPE_START]
{0x0990, 0x0015}, 	// MCU_DATA_0
{0x098E, 0x4930}, 	// MCU_ADDRESS [CAM1_LL_SFFB_SLOPE_STOP]
{0x0990, 0x0015}, 	// MCU_DATA_0
{0x098E, 0x4932}, 	// MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH1START]
{0x0990, 0x0002}, 	// MCU_DATA_0
{0x098E, 0x4934}, 	// MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH1STOP]
{0x0990, 0x0003}, 	// MCU_DATA_0
{0x098E, 0x4936}, 	// MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH2START]
{0x0990, 0x0008}, 	// MCU_DATA_0
{0x098E, 0x4938}, 	// MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH2STOP]
{0x0990, 0x0009}, 	// MCU_DATA_0
{0x098E, 0x493A}, 	// MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH3START]
{0x0990, 0x000C}, 	// MCU_DATA_0
{0x098E, 0x493C}, 	// MCU_ADDRESS [CAM1_LL_SFFB_LOW_THRESH3STOP]
{0x0990, 0x000D}, 	// MCU_DATA_0
{0x098E, 0x493E}, 	// MCU_ADDRESS [CAM1_LL_SFFB_MAX_THRESH_START]
{0x0990, 0x0015}, 	// MCU_DATA_0
{0x098E, 0x4940}, 	// MCU_ADDRESS [CAM1_LL_SFFB_MAX_THRESH_STOP]
{0x0990, 0x0013}, 	// MCU_DATA_0
{0x098E, 0xC944}, 	// MCU_ADDRESS [CAM1_LL_SFFB_FLATNESS_START]
{0x0990, 0x0023}, 	// MCU_DATA_0
{0x098E, 0xC945}, 	// MCU_ADDRESS [CAM1_LL_SFFB_FLATNESS_STOP]
{0x0990, 0x007F}, 	// MCU_DATA_0
{0x098E, 0xC946}, 	// MCU_ADDRESS [CAM1_LL_SFFB_TRANSITION_START]
{0x0990, 0x0007}, 	// MCU_DATA_0
{0x098E, 0xC947}, 	// MCU_ADDRESS [CAM1_LL_SFFB_TRANSITION_STOP]
{0x0990, 0x0001}, 	// MCU_DATA_0
{0x098E, 0xC948}, 	// MCU_ADDRESS [CAM1_LL_SFFB_SOBEL_FLAT_START]
{0x0990, 0x000A}, 	// MCU_DATA_0
{0x098E, 0xC949}, 	// MCU_ADDRESS [CAM1_LL_SFFB_SOBEL_FLAT_STOP]
{0x0990, 0x0002}, 	// MCU_DATA_0
{0x098E, 0xC94A}, 	// MCU_ADDRESS [CAM1_LL_SFFB_SOBEL_SHARP_START]
{0x0990, 0x00FF}, 	// MCU_DATA_0
{0x098E, 0xC94B}, 	// MCU_ADDRESS [CAM1_LL_SFFB_SOBEL_SHARP_STOP]
{0x0990, 0x00FF}, 	// MCU_DATA_0
{0x098E, 0xC906}, 	// MCU_ADDRESS [CAM1_LL_DM_EDGE_TH_START]
{0x0990, 0x000A}, 	// MCU_DATA_0
{0x098E, 0xC907}, 	// MCU_ADDRESS [CAM1_LL_DM_EDGE_TH_STOP]
{0x0990, 0x0050}, 	// MCU_DATA_0
{0x098E, 0xBC02}, 	// MCU_ADDRESS [LL_MODE]
{0x0990, 0x0005}, 	// MCU_DATA_0
{0x098E, 0xC908}, 	// MCU_ADDRESS [CAM1_LL_AP_KNEE_START]
{0x0990, 0x000A}, 	// MCU_DATA_0
{0x098E, 0xC909}, 	// MCU_ADDRESS [CAM1_LL_AP_KNEE_STOP]
{0x0990, 0x0050}, 	// MCU_DATA_0
{0x098E, 0xC90A}, 	// MCU_ADDRESS [CAM1_LL_AP_MANTISSA_START]
{0x0990, 0x0007}, 	// MCU_DATA_0
{0x326C, 0x0F0A}, 	// APERTURE_PARAMETERS_2D
{0x098E, 0xC94C},	   // MCU_ADDRESS [CAM1_LL_DELTA_GAIN]
{0x0990, 0x0003},	   // MCU_DATA_0
{0x098E, 0xC94E},	   // MCU_ADDRESS [CAM1_LL_DELTA_THRESHOLD_START]
{0x0990, 0x003C},	   // MCU_DATA_0
{0x098E, 0xC94F},	   // MCU_ADDRESS [CAM1_LL_DELTA_THRESHOLD_STOP]
{0x0990, 0x0064},	   // MCU_DATA_0
{0x098E, 0xE877},	   // MCU_ADDRESS [PRI_A_CONFIG_LL_END_SATURATION]
{0x0990, 0x0050},	   // MCU_DATA_0

{0x098E, 0x4918}, 	// MCU_ADDRESS [CAM1_LL_START_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x491A}, 	// MCU_ADDRESS [CAM1_LL_STOP_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x4926}, 	// MCU_ADDRESS [CAM1_LL_SFFB_START_ANALOG_GAIN]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x4928}, 	// MCU_ADDRESS [CAM1_LL_SFFB_END_ANALOG_GAIN]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x4956}, 	// MCU_ADDRESS [CAM1_LL_DC_START_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x4958}, 	// MCU_ADDRESS [CAM1_LL_DC_STOP_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x495E}, 	// MCU_ADDRESS [CAM1_LL_CDC_AGG_START_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x4960}, 	// MCU_ADDRESS [CAM1_LL_CDC_AGG_STOP_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x4964}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_START_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x4966}, 	// MCU_ADDRESS [CAM1_LL_CDC_BRIGHT_STOP_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x4972}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_START_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0
{0x098E, 0x4974}, 	// MCU_ADDRESS [CAM1_LL_CDC_DARK_STOP_GAIN_METRIC]
{0x0990, 0x0FFF}, 	// MCU_DATA_0

{0x098E, 0xAC02}, 	// MCU_ADDRESS [AWB_MODE]
{0x0990, 0x0004}, 	// MCU_DATA_0
{0x098E, 0xE877}, 	// MCU_ADDRESS [PRI_A_CONFIG_LL_END_SATURATION]
{0x0990, 0x0050}, 	// MCU_DATA_0

{0x098E, 0x48B8},          // MCU_ADDRESS [SEQ_CMD]
{0x0990, 0x002A},          // MCU_DATA_0

{0x098E, 0x48BA},          // MCU_ADDRESS [SEQ_CMD]
{0x0990, 0x0017},          // MCU_DATA_0

{0x098E, 0xA805},          // MCU_ADDRESS [SEQ_CMD]
{0x0990, 0x0000},          // MCU_DATA_0

{0x098E, 0x48CA},          // MCU_ADDRESS [SEQ_CMD]
{0x0990, 0xFFFF},          // MCU_DATA_0
{0x098E, 0xE851},          // MCU_ADDRESS [SEQ_CMD]
{0x0990, 0x0050},          // MCU_DATA_0
{0x098E, 0xE853},          // MCU_ADDRESS [SEQ_CMD]
{0x0990, 0x00a6},          // MCU_DATA_0

{0x098E, 0xC944},          // MCU_ADDRESS [SEQ_CMD]
{0x0990, 0x007F},          // MCU_DATA_0
{0x098E, 0xC945},          // MCU_ADDRESS [SEQ_CMD]
{0x0990, 0x007F},          // MCU_DATA_0


///
//[AWB AND CCM]
{0x098E, 0x4873}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_0]
{0x0990, 0x0205}, 	// MCU_DATA_0
{0x098E, 0x4875}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_1]
{0x0990, 0xFECA}, 	// MCU_DATA_0
{0x098E, 0x4877}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_2]
{0x0990, 0x0031}, 	// MCU_DATA_0
{0x098E, 0x4879}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_3]
{0x0990, 0xFFBC}, 	// MCU_DATA_0
{0x098E, 0x487B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_4]
{0x0990, 0x0156}, 	// MCU_DATA_0
{0x098E, 0x487D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_5]
{0x0990, 0xFFEE}, 	// MCU_DATA_0
{0x098E, 0x487F}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_6]
{0x0990, 0xFFC5}, 	// MCU_DATA_0
{0x098E, 0x4881}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_7]
{0x0990, 0xFF21}, 	// MCU_DATA_0
{0x098E, 0x4883}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_8]
{0x0990, 0x021A}, 	// MCU_DATA_0
/*
{0x098E, 0x4885}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_9]
{0x0990, 0x0020}, 	// MCU_DATA_0
{0x098E, 0x4887}, 	// MCU_ADDRESS [CAM1_AWB_CCM_L_10]
{0x0990, 0x0051}, 	// MCU_DATA_0
*/
{0x098E, 0x4889}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_0]
{0x0990, 0x000B}, 	// MCU_DATA_0
{0x098E, 0x488B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_1]
{0x0990, 0x0055}, 	// MCU_DATA_0
{0x098E, 0x488D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_2]
{0x0990, 0xFFA0}, 	// MCU_DATA_0
{0x098E, 0x488F}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_3]
{0x0990, 0xFFF6}, 	// MCU_DATA_0
{0x098E, 0x4891}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_4]
{0x0990, 0x000D}, 	// MCU_DATA_0
{0x098E, 0x4893}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_5]
{0x0990, 0xFFFD}, 	// MCU_DATA_0
{0x098E, 0x4895}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_6]
{0x0990, 0x0035}, 	// MCU_DATA_0
{0x098E, 0x4897}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_7]
{0x0990, 0x0068}, 	// MCU_DATA_0
{0x098E, 0x4899}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_8]
{0x0990, 0xFF62}, 	// MCU_DATA_0
/*
{0x098E, 0x489B}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_9]
{0x0990, 0x0013}, 	// MCU_DATA_0
{0x098E, 0x489D}, 	// MCU_ADDRESS [CAM1_AWB_CCM_RL_10]
{0x0990, 0xFFDE},  	//0xFFD3 // MCU_DATA_0
{0x098E, 0x48B8}, 	// MCU_ADDRESS [CAM1_AWB_X_SHIFT]
{0x0990, 0x0020}, 	// MCU_DATA_0
{0x098E, 0x48BA}, 	// MCU_ADDRESS [CAM1_AWB_Y_SHIFT]
{0x0990, 0x000F}, 	// MCU_DATA_0
{0x098E, 0x48BC}, 	// MCU_ADDRESS [CAM1_AWB_RECIP_XSCALE]
{0x0990, 0x0080}, 	// MCU_DATA_0
{0x098E, 0x48BE}, 	// MCU_ADDRESS [CAM1_AWB_RECIP_YSCALE]
{0x0990, 0x00AB}, 	// MCU_DATA_0
{0x098E, 0x48C0}, 	// MCU_ADDRESS [CAM1_AWB_ROT_CENTER_X]
{0x0990, 0x03F5}, 	// MCU_DATA_0
{0x098E, 0x48C2}, 	// MCU_ADDRESS [CAM1_AWB_ROT_CENTER_Y]
{0x0990, 0x03E7}, 	// MCU_DATA_0
{0x098E, 0xC8C4}, 	// MCU_ADDRESS [CAM1_AWB_ROT_SIN]
{0x0990, 0x0036}, 	// MCU_DATA_0
{0x098E, 0xC8C5}, 	// MCU_ADDRESS [CAM1_AWB_ROT_COS]
{0x0990, 0x0022}, 	// MCU_DATA_0
{0x098E, 0x48C6}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_0]
{0x0990, 0x0000}, 	// MCU_DATA_0
{0x098E, 0x48C8}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_1]
{0x0990, 0x0000}, 	// MCU_DATA_0
{0x098E, 0x48CA}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_2]
{0x0990, 0x1110}, 	// MCU_DATA_0
{0x098E, 0x48CC}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_3]
{0x0990, 0x0000}, 	// MCU_DATA_0
{0x098E, 0x48CE}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_4]
{0x0990, 0x0000}, 	// MCU_DATA_0
{0x098E, 0x48D0}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_5]
{0x0990, 0x0001}, 	// MCU_DATA_0
{0x098E, 0x48D2}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_6]
{0x0990, 0x1221}, 	// MCU_DATA_0
{0x098E, 0x48D4}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_7]
{0x0990, 0x1000}, 	// MCU_DATA_0
{0x098E, 0x48D6}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_8]
{0x0990, 0x0000}, 	// MCU_DATA_0
{0x098E, 0x48D8}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_9]
{0x0990, 0x1112}, 	// MCU_DATA_0
{0x098E, 0x48DA}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_10]
{0x0990, 0x2233}, 	// MCU_DATA_0
{0x098E, 0x48DC}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_11]
{0x0990, 0x2110}, 	// MCU_DATA_0
{0x098E, 0x48DE}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_12]
{0x0990, 0x0012}, 	// MCU_DATA_0
{0x098E, 0x48E0}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_13]
{0x0990, 0x2334}, 	// MCU_DATA_0
{0x098E, 0x48E2}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_14]
{0x0990, 0x3344}, 	// MCU_DATA_0
{0x098E, 0x48E4}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_15]
{0x0990, 0x4310}, 	// MCU_DATA_0
{0x098E, 0x48E6}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_16]
{0x0990, 0x0134}, 	// MCU_DATA_0
{0x098E, 0x48E8}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_17]
{0x0990, 0x4555}, 	// MCU_DATA_0
{0x098E, 0x48EA}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_18]
{0x0990, 0x4335}, 	// MCU_DATA_0
{0x098E, 0x48EC}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_19]
{0x0990, 0x5421}, 	// MCU_DATA_0
{0x098E, 0x48EE}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_20]
{0x0990, 0x0134}, 	// MCU_DATA_0
{0x098E, 0x48F0}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_21]
{0x0990, 0x5555}, 	// MCU_DATA_0
{0x098E, 0x48F2}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_22]
{0x0990, 0x4223}, 	// MCU_DATA_0
{0x098E, 0x48F4}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_23]
{0x0990, 0x4421}, 	// MCU_DATA_0
{0x098E, 0x48F6}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_24]
{0x0990, 0x0124}, 	// MCU_DATA_0
{0x098E, 0x48F8}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_25]
{0x0990, 0x4444}, 	// MCU_DATA_0
{0x098E, 0x48FA}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_26]
{0x0990, 0x2112}, 	// MCU_DATA_0
{0x098E, 0x48FC}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_27]
{0x0990, 0x2210}, 	// MCU_DATA_0
{0x098E, 0x48FE}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_28]
{0x0990, 0x0012}, 	// MCU_DATA_0
{0x098E, 0x4900}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_29]
{0x0990, 0x2111}, 	// MCU_DATA_0
{0x098E, 0x4902}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_30]
{0x0990, 0x1000}, 	// MCU_DATA_0
{0x098E, 0x4904}, 	// MCU_ADDRESS [CAM1_AWB_WEIGHT_TABLE_31]
{0x0990, 0x1100}, 	// MCU_DATA_0ee
{0x098E, 0xAC3B}, 	// MCU_ADDRESS [AWB_R_RATIO_PRE_AWB]
{0x0990, 0x0055}, 	// MCU_DATA_0
{0x098E, 0xAC3C}, 	// MCU_ADDRESS [AWB_B_RATIO_PRE_AWB]
{0x0990, 0x0025}, 	// MCU_DATA_0
{0x098E, 0xAC37}, 	// MCU_ADDRESS [AWB_R_SCENE_RATIO_LOWER]
{0x0990, 0x0036}, 	// MCU_DATA_0
{0x098E, 0xAC38}, 	// MCU_ADDRESS [AWB_R_SCENE_RATIO_UPPER]
{0x0990, 0x005E}, 	// MCU_DATA_0
{0x098E, 0xAC39}, 	// MCU_ADDRESS [AWB_B_SCENE_RATIO_LOWER]
{0x0990, 0x0026}, 	// MCU_DATA_0
{0x098E, 0xAC3A}, 	// MCU_ADDRESS [AWB_B_SCENE_RATIO_UPPER]
{0x0990, 0x004B}, 	// MCU_DATA_0

{0x098E, 0xE853}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_K_B_L]
{0x0990, 0x0099}, 	// MCU_DATA_0
{0x098E, 0xE854}, 	// MCU_ADDRESS [PRI_A_CONFIG_AWB_K_R_R]
{0x0990, 0x0092}, 	// MCU_DATA_0   
////
*/
{0x098E, 0x8400}, 	// MCU_ADDRESS [SEQ_CMD]
{0x0990, 0x0006}, 	// MCU_DATA_0
//{0x0000, 0x0020},
{0x098E, 0x8400}, 	// MCU_ADDRESS [SEQ_CMD]
{0x0990, 0x0005}, 	// MCU_DATA_0
{0x0000, 0x0030},

{0xFFFF, 0xFF},
};

const static struct mt9t113_reg mt9t113_for_preview[] = {
    {0x098E, 0xEC09}, // MCU_ADDRESS [PRI_B_NUM_OF_FRAMES_RUN]
    {0x0990, 0x0005}, // MCU_DATA_0
    {0x098E, 0x8400}, // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0001}, // MCU_DATA_0
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg mt9t113_for_capture[] = {
    {0x098E, 0xEC09}, // MCU_ADDRESS [PRI_B_NUM_OF_FRAMES_RUN]
    {0x0990, 0x0000}, // MCU_DATA_0
    {0x098E, 0x8400}, // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0002}, // MCU_DATA_0
    //{0x3400, 0x7A24}, //MIPI_CONTROL
    {0xFFFF, 0xFF}
};

/*adjust to 8~18fps*/
const static struct mt9t113_reg mt9t113_auto_fps[] = {
#if 1
{0x098E, 0x4808},
{0x0990, 0x0646},//5A0 //0x0CFE
{0x098E, 0x6815},
{0x0990, 0x000A},
{0x098E, 0x6817},
{0x0990, 0x000D},
{0x098E, 0x8400},
{0x0990, 0x0006},
#endif

#if 0
  {0x098E, 0x480A},     // MCU_ADDRESS [CAM1_CTX_A_LINE_LENGTH_PCLK]
  {0x0990, 0x0ab7},  //ab7   // MCU_DATA_0
  {0x098E, 0x6815},      // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_50HZ]
  {0x0990, 0x000a},      // 8 MCU_DATA_0
  {0x098E, 0x6817},      // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_60HZ]
  {0x0990, 0x000c},      // a MCU_DATA_0
  {0x098E, 0x6820},      // MCU_ADDRESS [PRI_A_CONFIG_AE_TRACK_TARGET_FDZONE]
  {0x0990, 0x0005},      // MCU_DATA_0
  {0x098E, 0x2800},      // MCU_ADDRESS [AE_TRACK_STATUS]
  {0x0990, 0x001C},     // MCU_DATA_0
  {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
  {0x0990, 0x0006},      // MCU_DATA_0
#endif
    {0xFFFF,0xFF}
};

/*adjust to 15fps*/
const static struct mt9t113_reg mt9t113_15fps[] = {
#if 1
    {0x098E, 0x4808}, // MCU_ADDRESS [CAM1_CTX_A_FRAME_LENGTH_LINES]
    {0x0990, 0x0646}, //0x067F// MCU_DATA_0
    {0x098E, 0x6815}, // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_50HZ]
    {0x0990, 0x0006}, // MCU_DATA_0
    {0x098E, 0x6817}, // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_60HZ]
    {0x0990, 0x0008}, // MCU_DATA_0
    {0x098E, 0x8400}, // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006}, // MCU_DATA_0
#endif
    {0xFFFF,0xFF}
};

/*adjust to 20fps*/ //for 720x480
const static struct mt9t113_reg mt9t113_20fps[] = {
#if 1
    {0x098E, 0x4808}, // MCU_ADDRESS [CAM1_CTX_A_FRAME_LENGTH_LINES]
    {0x0990, 0x04DF}, //0x067F// MCU_DATA_0
    {0x098E, 0x6820},      // ae_target_zone
    {0x0990, 0x0005},      //
    {0x098E, 0x6815}, // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_50HZ]
    {0x0990, 0x0005}, // MCU_DATA_0
    {0x098E, 0x6817}, // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_60HZ]
    {0x0990, 0x0006}, // MCU_DATA_0
    {0x098E, 0x8400}, // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006}, // MCU_DATA_0
#endif
    {0xFFFF,0xFF}
};

/*adjust to 7.5fps*/
const static struct mt9t113_reg mt9t113_7_5fps[] = {
#if 1
    {0x098E, 0x4808}, // MCU_ADDRESS [CAM1_CTX_A_FRAME_LENGTH_LINES]
    {0x0990, 0x0CFE}, // MCU_DATA_0
    {0x098E, 0x6815}, // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_50HZ]
    {0x0990, 0x000D}, // MCU_DATA_0
    {0x098E, 0x6817}, // MCU_ADDRESS [PRI_A_CONFIG_FD_MAX_FDZONE_60HZ]
    {0x0990, 0x0010}, // MCU_DATA_0
    {0x098E, 0x8400}, // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006}, // MCU_DATA_0    
#endif
    {0xFFFF,0xFF}
};


const static struct mt9t113_reg mt9t113_sqcif_preview[] = {
    {0xFFFF,0xFF}  
};

/*SVGA(800*600) to QCIF(176*144)*/
const static struct mt9t113_reg mt9t113_qcif_preview[] = {  
    {0x098E,0x6800},      // MCU_ADDRESS [PRI_A_IMAGE_WIDTH]
    {0x0990,0x00B8},      // MCU_DATA_0  176   //B0
    {0x098E,0x6802},      // MCU_ADDRESS [PRI_A_IMAGE_HEIGHT]
    {0x0990,0x0096},      // MCU_DATA_0  144  //90
    {0x098E,0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},      // MCU_DATA_0	
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg mt9t113_qvga_preview[] = {
    {0x098E,0x6800},      // MCU_ADDRESS [PRI_A_IMAGE_WIDTH]
    {0x0990,0x0148},      // MCU_DATA_0  320   //140
    {0x098E,0x6802},      // MCU_ADDRESS [PRI_A_IMAGE_HEIGHT]
    {0x0990,0x00F6},      // MCU_DATA_0  240  //F0
    {0x098E,0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},      // MCU_DATA_0	
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to CIF(352*288)*/
const static struct mt9t113_reg mt9t113_cif_preview[] = { 
    {0x098E,0x6800},      // MCU_ADDRESS [PRI_A_IMAGE_WIDTH]
    {0x0990,0x0160},      // MCU_DATA_0  352
    {0x098E,0x6802},      // MCU_ADDRESS [PRI_A_IMAGE_HEIGHT]
    {0x0990,0x0120},      // MCU_DATA_0  288
    {0x098E,0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},      // MCU_DATA_0			   
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to CIF(640*480)*/
const static struct mt9t113_reg mt9t113_vga_preview[] = {
    {0x098E,0x6800},      // MCU_ADDRESS [PRI_A_IMAGE_WIDTH]
    {0x0990,0x0288},      // MCU_DATA_0  640  //280
    {0x098E,0x6802},      // MCU_ADDRESS [PRI_A_IMAGE_HEIGHT]
    {0x0990,0x01E6},      // MCU_DATA_0  480   //1E0
    {0x098E,0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},      // MCU_DATA_0	
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to dvd-video ntsc(720*480)*/
const static struct mt9t113_reg mt9t113_dvd_ntsc_preview[] = { 
    {0x098E,0x6800},      // MCU_ADDRESS [PRI_A_IMAGE_WIDTH]
    {0x0990,0x02D8},      // MCU_DATA_0  720
    {0x098E,0x6802},      // MCU_ADDRESS [PRI_A_IMAGE_HEIGHT]
    {0x0990,0x01E6},      // MCU_DATA_0  480
    {0x098E,0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},      // MCU_DATA_0		   
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to SVGA,for consistence*/
const static struct mt9t113_reg mt9t113_svga_preview[] = {  
    {0x098E,0x6800},      // MCU_ADDRESS [PRI_A_IMAGE_WIDTH]
    {0x0990,0x0328},      // MCU_DATA_0  800  //320
    {0x098E,0x6802},      // MCU_ADDRESS [PRI_A_IMAGE_HEIGHT]
    {0x0990,0x0264},      // MCU_DATA_0  600   //258
    {0x098E,0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},      // MCU_DATA_0	
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to XGA(1024*768)*/
const static struct mt9t113_reg mt9t113_xga_preview[] = {
    {0xFFFF, 0xFF}
};

/*SVGA(800*600) to 1M(1280*960)*/
const static struct mt9t113_reg mt9t113_1M_preview[] = { 
    {0xFFFF, 0xFF}
};

/*UXGA(1600*1200)*/
const static struct mt9t113_reg mt9t113_uxga_preview[] = {
    {0xFFFF, 0xFF}
};

/*QXGA(2048*1536)*/
const static struct mt9t113_reg mt9t113_qxga_preview[] = {
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg* mt9t113_xxx_preview[] ={
    mt9t113_sqcif_preview,
    mt9t113_qcif_preview,
    mt9t113_qvga_preview,
    mt9t113_cif_preview,
    mt9t113_vga_preview,
    mt9t113_dvd_ntsc_preview,
    mt9t113_svga_preview,
    mt9t113_xga_preview,
    mt9t113_1M_preview,
    mt9t113_uxga_preview,
    mt9t113_qxga_preview
};


//128*96
const static struct mt9t113_reg mt9t113_svga_to_sqcif[]={
    
    {0xFFFF,0xFF}   
};

/*SVGA(800*600) to QCIF(176*144)*/
const static struct mt9t113_reg mt9t113_svga_to_qcif[] = {
    {0x098E, 0x6C00}, // MCU_ADDRESS [PRI_B_IMAGE_WIDTH]
    {0x0990, 0x00B6}, // MCU_DATA_0  //140
    {0x098E, 0x6C02}, // MCU_ADDRESS [PRI_B_IMAGE_HEIGHT]
    {0x0990, 0x0096}, // MCU_DATA_0    //F0   
    {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},      // MCU_DATA_0
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to QVGA(320*240)*/
const static struct mt9t113_reg mt9t113_svga_to_qvga[] = {
    {0x098E, 0x6C00}, // MCU_ADDRESS [PRI_B_IMAGE_WIDTH]
    {0x0990, 0x0144}, // MCU_DATA_0  //140
    {0x098E, 0x6C02}, // MCU_ADDRESS [PRI_B_IMAGE_HEIGHT]
    {0x0990, 0x00F3}, // MCU_DATA_0    //F0
    {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},      // MCU_DATA_0
    {0xFFFF,0xFF}    
};

/*SVGA(800*600) to CIF(352*288)*/
const static struct mt9t113_reg mt9t113_svga_to_cif[] = {
    
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to CIF(640*480)*/
const static struct mt9t113_reg mt9t113_svga_to_vga[] = {
    {0x098E, 0x6C00}, // MCU_ADDRESS [PRI_B_IMAGE_WIDTH]
    {0x0990, 0x0288}, // MCU_DATA_0   //280
    {0x098E, 0x6C02}, // MCU_ADDRESS [PRI_B_IMAGE_HEIGHT]
    {0x0990, 0x01E6}, // MCU_DATA_0   //1E0
    {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},      // MCU_DATA_0	
    {0xFFFF,0xFF}
};


/*SVGA(800*600) to dvd-video ntsc(720*480)*/
const static struct mt9t113_reg mt9t113_svga_to_dvd_ntsc[] = {
    {0x098E, 0x6C00}, // MCU_ADDRESS [PRI_B_IMAGE_WIDTH]
    {0x0990, 0x02D8}, // MCU_DATA_0  //320
    {0x098E, 0x6C02}, // MCU_ADDRESS [PRI_B_IMAGE_HEIGHT]
    {0x0990, 0x01E6}, // MCU_DATA_0  //258
    {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},      // MCU_DATA_0
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to SVGA,for consistence*/
const static struct mt9t113_reg mt9t113_svga_to_svga[] = {
    {0x098E, 0x6C00}, // MCU_ADDRESS [PRI_B_IMAGE_WIDTH]
    {0x0990, 0x0328}, // MCU_DATA_0  //320
    {0x098E, 0x6C02}, // MCU_ADDRESS [PRI_B_IMAGE_HEIGHT]
    {0x0990, 0x0264}, // MCU_DATA_0  //258
    {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},      // MCU_DATA_0
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to XGA(1024*768)*/
const static struct mt9t113_reg mt9t113_svga_to_xga[] = {
    {0x098E, 0x6C00}, // MCU_ADDRESS [PRI_B_IMAGE_WIDTH]
    {0x0990, 0x0408}, // MCU_DATA_0
    {0x098E, 0x6C02}, // MCU_ADDRESS [PRI_B_IMAGE_HEIGHT]
    {0x0990, 0x0306}, // MCU_DATA_0  
    {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},      // MCU_DATA_0
    {0xFFFF, 0xFF}
};

/*SVGA(800*600) to 1M(1280*960)*/
const static struct mt9t113_reg mt9t113_svga_to_1M[] = {
    {0x098E, 0x6C00}, // MCU_ADDRESS [PRI_B_IMAGE_WIDTH]
    {0x0990, 0x0508}, // MCU_DATA_0
    {0x098E, 0x6C02}, // MCU_ADDRESS [PRI_B_IMAGE_HEIGHT]
    {0x0990, 0x03C6}, // MCU_DATA_0   
    {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},      // MCU_DATA_0
    {0xFFFF, 0xFF}
};

/*UXGA(1600*1200)*/
const static struct mt9t113_reg mt9t113_svga_to_uxga[] = {
    {0x098E, 0x6C00}, // MCU_ADDRESS [PRI_B_IMAGE_WIDTH]
    {0x0990, 0x0648}, // MCU_DATA_0  //640
    {0x098E, 0x6C02}, // MCU_ADDRESS [PRI_B_IMAGE_HEIGHT]
    {0x0990, 0x04B6}, // MCU_DATA_0   //4B0
    {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},      // MCU_DATA_0
    {0xFFFF, 0xFF}
};


/*QXGA(2048*1536)*/
const static struct mt9t113_reg mt9t113_svga_to_qxga[] = {
    {0x098E, 0x6C00}, // MCU_ADDRESS [PRI_B_IMAGE_WIDTH]
    {0x0990, 0x0804}, // MCU_DATA_0  //800
    {0x098E, 0x6C02}, // MCU_ADDRESS [PRI_B_IMAGE_HEIGHT]
    {0x0990, 0x0608}, // MCU_DATA_0   //600
    {0x098E, 0x8400},      // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006},      // MCU_DATA_0
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg* mt9t113_svga_to_xxx[] ={
    mt9t113_svga_to_sqcif,
    mt9t113_svga_to_qcif,
    mt9t113_svga_to_qvga,
    mt9t113_svga_to_cif,
    mt9t113_svga_to_vga,
    mt9t113_svga_to_dvd_ntsc,
    mt9t113_svga_to_svga,
    mt9t113_svga_to_xga,
    mt9t113_svga_to_1M,
    mt9t113_svga_to_uxga,
    mt9t113_svga_to_qxga
};


const static struct mt9t113_reg mt9t113_zoom_1[]={   //1.0X
    {0x098E, 0xDC0B}, 	// MCU_ADDRESS [SYS_SCALE_MODE]
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0x098E, 0x5C0C}, 	// MCU_ADDRESS [SYS_ZOOM_FACTOR]
    {0x0990, 0x0064}, 	// MCU_DATA_0
    {0x098E, 0x8404}, 	// MCU_ADDRESS [SEQ_RESUME_CMD]
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0xFFFF, 0xFF}    
};
const static struct mt9t113_reg mt9t113_zoom_2[]={  //1.1X
    {0x098E, 0xDC0B}, 	// MCU_ADDRESS [SYS_SCALE_MODE]
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0x098E, 0x5C0C}, 	// MCU_ADDRESS [SYS_ZOOM_FACTOR]
    {0x0990, 0x006E}, 	// MCU_DATA_0
    {0x098E, 0x8404}, 	// MCU_ADDRESS [SEQ_RESUME_CMD]
    {0x0990, 0x0006}, 	// MCU_DATA_0  
    {0xFFFF, 0xFF}    
};
const static struct mt9t113_reg mt9t113_zoom_3[]={   //1.2X
    {0x098E, 0xDC0B}, 	// MCU_ADDRESS [SYS_SCALE_MODE]
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0x098E, 0x5C0C}, 	// MCU_ADDRESS [SYS_ZOOM_FACTOR]
    {0x0990, 0x0078}, 	// MCU_DATA_0
    {0x098E, 0x8404}, 	// MCU_ADDRESS [SEQ_RESUME_CMD]
    {0x0990, 0x0006}, 	// MCU_DATA_0  
    {0xFFFF, 0xFF}     
};
const static struct mt9t113_reg mt9t113_zoom_4[]={    //1.3X
    {0x098E, 0xDC0B}, 	// MCU_ADDRESS [SYS_SCALE_MODE]
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0x098E, 0x5C0C}, 	// MCU_ADDRESS [SYS_ZOOM_FACTOR]
    {0x0990, 0x0082}, 	// MCU_DATA_0
    {0x098E, 0x8404}, 	// MCU_ADDRESS [SEQ_RESUME_CMD]
    {0x0990, 0x0006}, 	// MCU_DATA_0  
    {0xFFFF, 0xFF}  
};
const static struct mt9t113_reg mt9t113_zoom_5[]={   //1.4X  
    {0x098E, 0xDC0B}, 	// MCU_ADDRESS [SYS_SCALE_MODE]
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0x098E, 0x5C0C}, 	// MCU_ADDRESS [SYS_ZOOM_FACTOR]
    {0x0990, 0x008C}, 	// MCU_DATA_0
    {0x098E, 0x8404}, 	// MCU_ADDRESS [SEQ_RESUME_CMD]
    {0x0990, 0x0006}, 	// MCU_DATA_0 
    {0xFFFF, 0xFF}     
};
const static struct mt9t113_reg mt9t113_zoom_6[]={    //1.5X 
    {0x098E, 0xDC0B}, 	// MCU_ADDRESS [SYS_SCALE_MODE]
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0x098E, 0x5C0C}, 	// MCU_ADDRESS [SYS_ZOOM_FACTOR]
    {0x0990, 0x0096}, 	// MCU_DATA_0
    {0x098E, 0x8404}, 	// MCU_ADDRESS [SEQ_RESUME_CMD]
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0xFFFF, 0xFF}     
};
const static struct mt9t113_reg mt9t113_zoom_7[]={     //1.6X
    {0x098E, 0xDC0B}, 	// MCU_ADDRESS [SYS_SCALE_MODE]
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0x098E, 0x5C0C}, 	// MCU_ADDRESS [SYS_ZOOM_FACTOR]
    {0x0990, 0x00A0}, 	// MCU_DATA_0
    {0x098E, 0x8404}, 	// MCU_ADDRESS [SEQ_RESUME_CMD]
    {0x0990, 0x0006}, 	// MCU_DATA_0  
    {0xFFFF, 0xFF}     
};

const static struct mt9t113_reg* mt9t113_zoom[]={
    mt9t113_zoom_1,
    mt9t113_zoom_2,
    mt9t113_zoom_3,
    mt9t113_zoom_4,
    mt9t113_zoom_5,
    mt9t113_zoom_6,
    mt9t113_zoom_7,
};

const static struct mt9t113_reg mt9t113_effect_off[]={
    {0x098E, 0xE887}, // MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SELECT_FX]
    {0x0990, 0x0000}, // MCU_DATA_0
    {0x098E, 0xEC87}, // MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SELECT_FX]
    {0x0990, 0x0000}, // MCU_DATA_0
    {0x098E, 0x8400}, // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0006}, // MCU_DATA_0
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg mt9t113_effect_mono[]={
    {0x098E,0xE887},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SELECT_FX][Special Effect-Black/White]
    {0x0990,0x0001},// MCU_DATA_0
    {0x098E,0xEC87},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SELECT_FX]
    {0x0990,0x0001},// MCU_DATA_0
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0	
    {0xFFFF,0xFF}
}; 

const static struct mt9t113_reg mt9t113_effect_negative[]={
    {0x098E,0xE887},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SELECT_FX][Special Effect-Negative]
    {0x0990,0x0003},// MCU_DATA_0
    {0x098E,0xEC87},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SELECT_FX]
    {0x0990,0x0003},// MCU_DATA_0
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg mt9t113_effect_sepia[]={
    {0x098E,0xE887},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SELECT_FX][Special Effect-Purple]
    {0x0990,0x0002},// MCU_DATA_0
    {0x098E,0xEC87},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SELECT_FX]
    {0x0990,0x0002},// MCU_DATA_0
    {0x098E,0xE889},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SEPIA_CR]
    {0x0990,0x001C},// MCU_DATA_0
    {0x098E,0xE88A},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SEPIA_CB]
    {0x0990,0x00E6},// MCU_DATA_0
    {0x098E,0xEC89},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SEPIA_CR]
    {0x0990,0x001C},// MCU_DATA_0
    {0x098E,0xEC8A},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SEPIA_CB]
    {0x0990,0x00E6},// MCU_DATA_0
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0	
    {0xFFFF,0xFF}
};
/*
const static struct mt9t113_reg mt9t113_effect_bluish[]={
#if 0
    {0x098E,0xE887},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SELECT_FX]
    {0x0990,0x0002},// MCU_DATA_0
    {0x098E,0xEC87},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SELECT_FX]
    {0x0990,0x0002},// MCU_DATA_0
    {0x098E,0xE889},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SEPIA_CR]
    {0x0990,0x00E2},// MCU_DATA_0
    {0x098E,0xE88A},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SEPIA_CB]
    {0x0990,0x0030},// MCU_DATA_0
    {0x098E,0xEC89},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SEPIA_CR]
    {0x0990,0x00E2},// MCU_DATA_0
    {0x098E,0xEC8A},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SEPIA_CB]
    {0x0990,0x0030},// MCU_DATA_0
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0	
    {0xFFFF,0xFF}
#endif

    {0x098E, 0xE887}, 	// MCU_ADDRESS
    {0x0990, 0x0002}, 	// MCU_DATA_0
    {0x098E, 0xEC87}, 	// MCU_ADDRESS
    {0x0990, 0x0002}, 	// MCU_DATA_0
    {0x098E, 0xE889}, 	// MCU_ADDRESS
    {0x0990, 0x00A2}, 	// MCU_DATA_0
    {0x098E, 0xE88A}, 	// MCU_ADDRESS
    {0x0990, 0x0030}, 	// MCU_DATA_0
    {0x098E, 0xEC89}, 	// MCU_ADDRESS
    {0x0990, 0x00A2}, 	// MCU_DATA_0
    {0x098E, 0xEC8A}, 	// MCU_ADDRESS
    {0x0990, 0x0030}, 	// MCU_DATA_0
    {0x098E, 0x8400}, 	// MCU_ADDRESS
    {0x0990, 0x0006}, 	// MCU_DATA_0
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg mt9t113_effect_reddish[]={
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_effect_yellowish[]={
    {0x098E,0xE887},// MCU_ADDRESS                                     
    {0x0990,0x0002},// MCU_DATA_0 [PRI_B_CONFIG_SYSCTRL_SELECT_FX]    
    {0x098E,0xEC87},// MCU_ADDRESS                                     
    {0x0990,0x0002},// MCU_DATA_0 [PRI_A_CONFIG_SYSCTRL_SEPIA_CR]     
    {0x098E,0xE889},// MCU_ADDRESS                                     
    {0x0990,0x001E},// MCU_DATA_0 [PRI_A_CONFIG_SYSCTRL_SEPIA_CB]     
    {0x098E,0xE88A},// MCU_ADDRESS                                     
    {0x0990,0x009C},// MCU_DATA_0 [PRI_B_CONFIG_SYSCTRL_SEPIA_CR]     
    {0x098E,0xEC89},// MCU_ADDRESS                                     
    {0x0990,0x001E},// MCU_DATA_0 [PRI_B_CONFIG_SYSCTRL_SEPIA_CB]     
    {0x098E,0xEC8A},// MCU_ADDRESS                                     
    {0x0990,0x009C},// MCU_DATA_0 [SEQ_CMD]                           
    {0x098E,0x8400},// MCU_ADDRESS                                     
    {0x0990,0x0006},// MCU_DATA_0	 
    {0xFFFF,0xFF}
};
*/

const static struct mt9t113_reg mt9t113_effect_green_aqua[]={

//will be modified later for real aqua effect, now it's too green //linwinx 2011.11.15
    {0x098E,0xE887},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SELECT_FX]
    {0x0990,0x0002},// MCU_DATA_0
    {0x098E,0xEC87},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SELECT_FX]
    {0x0990,0x0002},// MCU_DATA_0
    {0x098E,0xE889},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SEPIA_CR]
    {0x0990,0x00CE},// MCU_DATA_0
    {0x098E,0xE88A},// MCU_ADDRESS [PRI_A_CONFIG_SYSCTRL_SEPIA_CB]
    {0x0990,0x0012},// MCU_DATA_0
    {0x098E,0xEC89},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SEPIA_CR]
    {0x0990,0x00CE},// MCU_DATA_0
    {0x098E,0xEC8A},// MCU_ADDRESS [PRI_B_CONFIG_SYSCTRL_SEPIA_CB]
    {0x0990,0x0012},// MCU_DATA_0
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0

    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg* mt9t113_effects[]={
    mt9t113_effect_off,
    mt9t113_effect_mono,
    mt9t113_effect_negative,
    mt9t113_effect_sepia,
    mt9t113_effect_green_aqua
};

const static struct mt9t113_reg mt9t113_wb_auto[]={
    {0x098E,0x6848},// MCU_ADDRESS [PRI_A_CONFIG_AWB_ALGO_RUN][Auto WB]
    {0x0990,0x003F},// MCU_DATA_0
    {0x098E,0x6865},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_ENTER]
    {0x0990,0x801F},// MCU_DATA_0
    {0x098E,0x6867},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_RUN]
    {0x0990,0x12F7},// MCU_DATA_0

    {0x0000,0x0010}, 
    
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0
    {0xFFFF,0xFF}   
}; 
 
const static struct mt9t113_reg mt9t113_wb_daylight[]={
    {0x098E,0x6848},// MCU_ADDRESS [PRI_A_CONFIG_AWB_ALGO_RUN][Manual WB-A28]
    {0x0990,0x0000},// MCU_DATA_0
    {0x098E,0x6865},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_ENTER]
    {0x0990,0x0000},// MCU_DATA_0
    {0x098E,0x6867},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_RUN]
    {0x0990,0x0000},// MCU_DATA_0
                                       
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0

    {0x098E,0xAC3B},// MCU_ADDRESS [AWB_R_RATIO_PRE_AWB]
    {0x0990,0x0041},// MCU_DATA_0
    {0x098E,0xAC3C},// MCU_ADDRESS [AWB_B_RATIO_PRE_AWB]
    {0x0990,0x0046},// MCU_DATA_0

    {0x0000,0x0010}, 
    
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg mt9t113_wb_cloudy[]={
    {0x098E,0x6848},// MCU_ADDRESS [PRI_A_CONFIG_AWB_ALGO_RUN][Manual WB-A28]
    {0x0990,0x0000},// MCU_DATA_0
    {0x098E,0x6865},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_ENTER]
    {0x0990,0x0000},// MCU_DATA_0
    {0x098E,0x6867},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_RUN]
    {0x0990,0x0000},// MCU_DATA_0

    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0

    {0x098E,0xAC3B},// MCU_ADDRESS [AWB_R_RATIO_PRE_AWB]
    {0x0990,0x0044},// MCU_DATA_0
    {0x098E,0xAC3C},// MCU_ADDRESS [AWB_B_RATIO_PRE_AWB]
    {0x0990,0x0051},// MCU_DATA_0

    {0x0000,0x0010}, 
    
    {0xFFFF,0xFF}

};

const static struct mt9t113_reg mt9t113_wb_incandescent[]={
    {0x098E,0x6848},// MCU_ADDRESS [PRI_A_CONFIG_AWB_ALGO_RUN][Manual WB-A28]
    {0x0990,0x0000},// MCU_DATA_0
    {0x098E,0x6865},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_ENTER]
    {0x0990,0x0000},// MCU_DATA_0
    {0x098E,0x6867},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_RUN]
    {0x0990,0x0000},// MCU_DATA_0
                                       
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0

    {0x098E,0xAC3B},// MCU_ADDRESS [AWB_R_RATIO_PRE_AWB]
    {0x0990,0x0059},// MCU_DATA_0
    {0x098E,0xAC3C},// MCU_ADDRESS [AWB_B_RATIO_PRE_AWB]
    {0x0990,0x002B},// MCU_DATA_0

    {0x0000,0x0010}, 
    
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg mt9t113_wb_fluorescent[]={
    {0x098E,0x6848},// MCU_ADDRESS [PRI_A_CONFIG_AWB_ALGO_RUN][Manual WB-CWF]
    {0x0990,0x0000},// MCU_DATA_0
    {0x098E,0x6865},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_ENTER]
    {0x0990,0x0000},// MCU_DATA_0
    {0x098E,0x6867},// MCU_ADDRESS [PRI_A_CONFIG_STAT_ALGO_RUN]
    {0x0990,0x0000},// MCU_DATA_0

    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0

    {0x098E,0xAC3B},// MCU_ADDRESS [AWB_R_RATIO_PRE_AWB]
    {0x0990,0x0049},// MCU_DATA_0
    {0x098E,0xAC3C},// MCU_ADDRESS [AWB_B_RATIO_PRE_AWB]
    {0x0990,0x002A},// MCU_DATA_0

    {0x0000,0x0010}, 
    
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg* mt9t113_whitebalance[]={
    mt9t113_wb_auto,
    mt9t113_wb_daylight,
    mt9t113_wb_cloudy,
    mt9t113_wb_incandescent, 
    mt9t113_wb_fluorescent, 
};


const static struct mt9t113_reg mt9t113_brightness_1[]={  
    {0x337E,0xE000},// Y_RGB_OFFSET[Manual EV-2]  
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_brightness_2[]={  
    {0x337E,0xF000},// Y_RGB_OFFSET[Manual EV-1]
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_brightness_3[]={   
    {0x337E,0x0000},// Y_RGB_OFFSET[Manual EV0]
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_brightness_4[]={   
    {0x337E,0x1000},// Y_RGB_OFFSET[Manual EV+1]
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_brightness_5[]={   
    {0x337E,0x2000},// Y_RGB_OFFSET[Manual EV+2]
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg* mt9t113_brightness[]={
    mt9t113_brightness_1,
    mt9t113_brightness_2,
    mt9t113_brightness_3,
    mt9t113_brightness_4,
    mt9t113_brightness_5
};

const static struct mt9t113_reg mt9t113_saturation_1[]={   
    {0x098E, 0xE876},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x003F},     // MCU_DATA_0
    {0x098E, 0xE877},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x003F},     // MCU_DATA_0 
    {0xFFFF,0xFF}    
};
const static struct mt9t113_reg mt9t113_saturation_2[]={  
    {0x098E, 0xE876},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x0060},     // MCU_DATA_0
    {0x098E, 0xE877},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x0060},     // MCU_DATA_0     
    {0xFFFF,0xFF}    
};
const static struct mt9t113_reg mt9t113_saturation_3[]={   
    {0x098E, 0xE876},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x00B0},     // MCU_DATA_0
    {0x098E, 0xE877},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x00B0},     // MCU_DATA_0    
    {0xFFFF,0xFF}     
};
const static struct mt9t113_reg mt9t113_saturation_4[]={    
    {0x098E, 0xE876},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x00D8},    // MCU_DATA_0
    {0x098E, 0xE877},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x00D8},    // MCU_DATA_0   
    {0xFFFF,0xFF}  
};
const static struct mt9t113_reg mt9t113_saturation_5[]={     
    {0x098E, 0xE876},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x00F8},     // MCU_DATA_0
    {0x098E, 0xE877},    // MCU_ADDRESS [PRI_A_CONFIG_LL_START_SATURATION]
    {0x0990, 0x00F8},     // MCU_DATA_0  
    {0xFFFF,0xFF}     
};

const static struct mt9t113_reg* mt9t113_saturation[]={
    mt9t113_saturation_1,
    mt9t113_saturation_2,
    mt9t113_saturation_3,
    mt9t113_saturation_4,
    mt9t113_saturation_5
};


const static struct mt9t113_reg mt9t113_exposure_1[]={  
    {0x337E,0xD000},// Y_RGB_OFFSET[Manual EV-2]  
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_exposure_2[]={  
    {0x337E,0xF000},// Y_RGB_OFFSET[Manual EV-1]
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_exposure_3[]={   
    {0x337E,0x0000},// Y_RGB_OFFSET[Manual EV0]
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_exposure_4[]={   
    {0x337E,0x2000},// Y_RGB_OFFSET[Manual EV+1]
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_exposure_5[]={   
    {0x337E,0x4000},// Y_RGB_OFFSET[Manual EV+2]
    {0x098E,0x8400},// MCU_ADDRESS [SEQ_CMD]
    {0x0990,0x0006},// MCU_DATA_0
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg* mt9t113_exposure[]={
    mt9t113_exposure_1,
    mt9t113_exposure_2,
    mt9t113_exposure_3,
    mt9t113_exposure_4,
    mt9t113_exposure_5
};

#if 0  
const static struct mt9t113_reg mt9t113_sharpness_auto[]={
    //{0x3306,0x00},       
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_sharpness_1[]={
    {0x3371,0x00},       
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_sharpness_2[]={
    {0x3371,0x01},        
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_sharpness_3[]={
    {0x3371,0x02},          
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_sharpness_4[]={
    {0x3371,0x03},       
    {0xFFFF,0xFF}
};
const static struct mt9t113_reg mt9t113_sharpness_5[]={
    {0x3371,0x04},    
    {0xFFFF,0xFF}
};

const static struct mt9t113_reg* mt9t113_sharpness[]={
    mt9t113_sharpness_auto,
    mt9t113_sharpness_1,
    mt9t113_sharpness_2,
    mt9t113_sharpness_3,
    mt9t113_sharpness_4,
    mt9t113_sharpness_5
};
#endif

const static struct mt9t113_reg s5k5cag_common_svgabase[] = {
{0x0010, 0x0001},	// Reset
{0x1030, 0x0000},	// Clear host interrupt so main will wait
{0x0014, 0x0001},	// ARM go
{0x0000, 0x0050}, // Wait100mSec

{0x0028, 0xD000},	
{0x002A, 0x1082},	
{0x0F12, 0x0155},	// [9:8] D4, [7:6] D3, [5:4] D2, [3:2] D1, [1:0] D0
{0x0F12, 0x0155},	// [9:8] D9, [7:6] D8, [5:4] D7, [3:2] D6, [1:0] D5
{0x0F12, 0x1555},	// [5:4] GPIO3, [3:2] GPIO2, [1:0] GPIO1
{0x0F12, 0x0555},	//05d5// [11:10] SDA, [9:8] SCA, [7:6] PCLK, [3:2] VSYNC, [1:0] HSYNC    		
{0x0028, 0x7000},	// Start T&P part 
{0x002A, 0x2CF8},
{0x0F12, 0xB510},
{0x0F12, 0x490F},
{0x0F12, 0x2000},
{0x0F12, 0x8048},
{0x0F12, 0x8088},
{0x0F12, 0x490E},
{0x0F12, 0x480E},
{0x0F12, 0xF000},
{0x0F12, 0xF949},
{0x0F12, 0x490E},
{0x0F12, 0x480E},
{0x0F12, 0x6341},
{0x0F12, 0x490E},
{0x0F12, 0x38C0},
{0x0F12, 0x63C1},
{0x0F12, 0x490E},
{0x0F12, 0x6301},
{0x0F12, 0x490E},
{0x0F12, 0x3040},
{0x0F12, 0x6181},
{0x0F12, 0x490D},
{0x0F12, 0x480E},
{0x0F12, 0xF000},
{0x0F12, 0xF93A},
{0x0F12, 0x490D},
{0x0F12, 0x480E},
{0x0F12, 0xF000},
{0x0F12, 0xF936},
{0x0F12, 0xBC10},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0x0000},
{0x0F12, 0x1080},
{0x0F12, 0xD000},
{0x0F12, 0x2D69},
{0x0F12, 0x7000},
{0x0F12, 0x89A9},
{0x0F12, 0x0000},
{0x0F12, 0x2DBB},
{0x0F12, 0x7000},
{0x0F12, 0x0140},
{0x0F12, 0x7000},
{0x0F12, 0x2DED},
{0x0F12, 0x7000},
{0x0F12, 0x2E65},
{0x0F12, 0x7000},
{0x0F12, 0x2E79},
{0x0F12, 0x7000},
{0x0F12, 0x2E4D},
{0x0F12, 0x7000},
{0x0F12, 0x013D},
{0x0F12, 0x0001},
{0x0F12, 0x2F03},
{0x0F12, 0x7000},
{0x0F12, 0x5823},
{0x0F12, 0x0000},
{0x0F12, 0xB570},
{0x0F12, 0x6804},
{0x0F12, 0x6845},
{0x0F12, 0x6881},
{0x0F12, 0x6840},
{0x0F12, 0x2900},
{0x0F12, 0x6880},
{0x0F12, 0xD007},
{0x0F12, 0x4976},
{0x0F12, 0x8949},
{0x0F12, 0x084A},
{0x0F12, 0x1880},
{0x0F12, 0xF000},
{0x0F12, 0xF914},
{0x0F12, 0x80A0},
{0x0F12, 0xE000},
{0x0F12, 0x80A0},
{0x0F12, 0x88A0},
{0x0F12, 0x2800},
{0x0F12, 0xD010},
{0x0F12, 0x68A9},
{0x0F12, 0x6828},
{0x0F12, 0x084A},
{0x0F12, 0x1880},
{0x0F12, 0xF000},
{0x0F12, 0xF908},
{0x0F12, 0x8020},
{0x0F12, 0x1D2D},
{0x0F12, 0xCD03},
{0x0F12, 0x084A},
{0x0F12, 0x1880},
{0x0F12, 0xF000},
{0x0F12, 0xF901},
{0x0F12, 0x8060},
{0x0F12, 0xBC70},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0x2000},
{0x0F12, 0x8060},
{0x0F12, 0x8020},
{0x0F12, 0xE7F8},
{0x0F12, 0xB510},
{0x0F12, 0xF000},
{0x0F12, 0xF8FC},
{0x0F12, 0x4865},
{0x0F12, 0x4966},
{0x0F12, 0x8800},
{0x0F12, 0x4A66},
{0x0F12, 0x2805},
{0x0F12, 0xD003},
{0x0F12, 0x4B65},
{0x0F12, 0x795B},
{0x0F12, 0x2B00},
{0x0F12, 0xD005},
{0x0F12, 0x2001},
{0x0F12, 0x8008},
{0x0F12, 0x8010},
{0x0F12, 0xBC10},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0x2800},
{0x0F12, 0xD1FA},
{0x0F12, 0x2000},
{0x0F12, 0x8008},
{0x0F12, 0x8010},
{0x0F12, 0xE7F6},
{0x0F12, 0xB570},
{0x0F12, 0x0004},
{0x0F12, 0x485D},
{0x0F12, 0x2C00},
{0x0F12, 0x8D00},
{0x0F12, 0xD001},
{0x0F12, 0x2501},
{0x0F12, 0xE000},
{0x0F12, 0x2500},
{0x0F12, 0x4E5B},
{0x0F12, 0x4328},
{0x0F12, 0x8030},
{0x0F12, 0x207D},
{0x0F12, 0x00C0},
{0x0F12, 0xF000},
{0x0F12, 0xF8DE},
{0x0F12, 0x4858},
{0x0F12, 0x2C00},
{0x0F12, 0x8C40},
{0x0F12, 0x0329},
{0x0F12, 0x4308},
{0x0F12, 0x8130},
{0x0F12, 0x4856},
{0x0F12, 0x2C00},
{0x0F12, 0x8A40},
{0x0F12, 0x01A9},
{0x0F12, 0x4308},
{0x0F12, 0x80B0},
{0x0F12, 0x2C00},
{0x0F12, 0xD00B},
{0x0F12, 0x4853},
{0x0F12, 0x8A01},
{0x0F12, 0x4853},
{0x0F12, 0xF000},
{0x0F12, 0xF8BD},
{0x0F12, 0x4953},
{0x0F12, 0x8809},
{0x0F12, 0x4348},
{0x0F12, 0x0400},
{0x0F12, 0x0C00},
{0x0F12, 0xF000},
{0x0F12, 0xF8C4},
{0x0F12, 0x0020},
{0x0F12, 0xF000},
{0x0F12, 0xF8C9},
{0x0F12, 0x484F},
{0x0F12, 0x7004},
{0x0F12, 0xE7AF},
{0x0F12, 0xB510},
{0x0F12, 0x0004},
{0x0F12, 0xF000},
{0x0F12, 0xF8CA},
{0x0F12, 0x6020},
{0x0F12, 0x494C},
{0x0F12, 0x8B49},
{0x0F12, 0x0789},
{0x0F12, 0xD0BD},
{0x0F12, 0x0040},
{0x0F12, 0x6020},
{0x0F12, 0xE7BA},
{0x0F12, 0xB510},
{0x0F12, 0xF000},
{0x0F12, 0xF8C7},
{0x0F12, 0x4848},
{0x0F12, 0x8880},
{0x0F12, 0x0601},
{0x0F12, 0x4840},
{0x0F12, 0x1609},
{0x0F12, 0x8281},
{0x0F12, 0xE7B0},
{0x0F12, 0xB5F8},
{0x0F12, 0x000F},
{0x0F12, 0x4C3A},
{0x0F12, 0x3420},
{0x0F12, 0x2500},
{0x0F12, 0x5765},
{0x0F12, 0x0039},
{0x0F12, 0xF000},
{0x0F12, 0xF8BF},
{0x0F12, 0x9000},
{0x0F12, 0x2600},
{0x0F12, 0x57A6},
{0x0F12, 0x4C38},
{0x0F12, 0x42AE},
{0x0F12, 0xD01B},
{0x0F12, 0x4D3D},
{0x0F12, 0x8AE8},
{0x0F12, 0x2800},
{0x0F12, 0xD013},
{0x0F12, 0x4832},
{0x0F12, 0x8A01},
{0x0F12, 0x8B80},
{0x0F12, 0x4378},
{0x0F12, 0xF000},
{0x0F12, 0xF881},
{0x0F12, 0x89A9},
{0x0F12, 0x1A41},
{0x0F12, 0x4837},
{0x0F12, 0x3820},
{0x0F12, 0x8AC0},
{0x0F12, 0x4348},
{0x0F12, 0x17C1},
{0x0F12, 0x0D89},
{0x0F12, 0x1808},
{0x0F12, 0x1280},
{0x0F12, 0x8AA1},
{0x0F12, 0x1A08},
{0x0F12, 0x82A0},
{0x0F12, 0xE003},
{0x0F12, 0x88A8},
{0x0F12, 0x0600},
{0x0F12, 0x1600},
{0x0F12, 0x82A0},
{0x0F12, 0x2014},
{0x0F12, 0x5E20},
{0x0F12, 0x42B0},
{0x0F12, 0xD011},
{0x0F12, 0xF000},
{0x0F12, 0xF89F},
{0x0F12, 0x1D40},
{0x0F12, 0x00C3},
{0x0F12, 0x1A18},
{0x0F12, 0x214B},
{0x0F12, 0xF000},
{0x0F12, 0xF863},
{0x0F12, 0x211F},
{0x0F12, 0xF000},
{0x0F12, 0xF89E},
{0x0F12, 0x2114},
{0x0F12, 0x5E61},
{0x0F12, 0x0FC9},
{0x0F12, 0x0149},
{0x0F12, 0x4301},
{0x0F12, 0x4826},
{0x0F12, 0x81C1},
{0x0F12, 0x9800},
{0x0F12, 0xBCF8},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0xB5F1},
{0x0F12, 0xB082},
{0x0F12, 0x2500},
{0x0F12, 0x4822},
{0x0F12, 0x9001},
{0x0F12, 0x2400},
{0x0F12, 0x2028},
{0x0F12, 0x4368},
{0x0F12, 0x4A21},
{0x0F12, 0x4917},
{0x0F12, 0x1882},
{0x0F12, 0x39E0},
{0x0F12, 0x1847},
{0x0F12, 0x9200},
{0x0F12, 0x0066},
{0x0F12, 0x19B8},
{0x0F12, 0x9A01},
{0x0F12, 0x3060},
{0x0F12, 0x8B01},
{0x0F12, 0x5BB8},
{0x0F12, 0x8812},
{0x0F12, 0xF000},
{0x0F12, 0xF884},
{0x0F12, 0x9900},
{0x0F12, 0x5388},
{0x0F12, 0x1C64},
{0x0F12, 0x2C14},
{0x0F12, 0xDBF1},
{0x0F12, 0x1C6D},
{0x0F12, 0x2D03},
{0x0F12, 0xDBE5},
{0x0F12, 0x9802},
{0x0F12, 0x6800},
{0x0F12, 0x0600},
{0x0F12, 0x0E00},
{0x0F12, 0xF000},
{0x0F12, 0xF87E},
{0x0F12, 0xBCFE},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0x0000},
{0x0F12, 0x0C3C},
{0x0F12, 0x7000},
{0x0F12, 0x26E8},
{0x0F12, 0x7000},
{0x0F12, 0x6100},
{0x0F12, 0xD000},
{0x0F12, 0x6500},
{0x0F12, 0xD000},
{0x0F12, 0x1A7C},
{0x0F12, 0x7000},
{0x0F12, 0x2C2C},
{0x0F12, 0x7000},
{0x0F12, 0xF400},
{0x0F12, 0xD000},
{0x0F12, 0x167C},
{0x0F12, 0x7000},
{0x0F12, 0x3368},
{0x0F12, 0x7000},
{0x0F12, 0x1D6C},
{0x0F12, 0x7000},
{0x0F12, 0x40A0},
{0x0F12, 0x00DD},
{0x0F12, 0xF520},
{0x0F12, 0xD000},
{0x0F12, 0x2C29},
{0x0F12, 0x7000},
{0x0F12, 0x1A54},
{0x0F12, 0x7000},
{0x0F12, 0x1564},
{0x0F12, 0x7000},
{0x0F12, 0xF2A0},
{0x0F12, 0xD000},
{0x0F12, 0x2440},
{0x0F12, 0x7000},
{0x0F12, 0x05A0},
{0x0F12, 0x7000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x1A3F},
{0x0F12, 0x0001},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xF004},
{0x0F12, 0xE51F},
{0x0F12, 0x1F48},
{0x0F12, 0x0001},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x24BD},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xF53F},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xF5D9},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x013D},
{0x0F12, 0x0001},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xF5C9},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xFAA9},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x36DD},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x36ED},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x3723},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x5823},
{0x0F12, 0x0000},
{0x0F12, 0x7D3E},
{0x0F12, 0x0000},
{0x0028, 0x7000},
{0x002A, 0x157A},	
{0x0F12, 0x0001},	
{0x002A, 0x1578},	
{0x0F12, 0x0001},	
{0x002A, 0x1576},	
{0x0F12, 0x0020},	
{0x002A, 0x1574},	
{0x0F12, 0x0006},
{0x002A, 0x156E},	
{0x0F12, 0x0001},	
{0x002A, 0x1568},	
{0x0F12, 0x00FC},    	  	
{0x002A, 0x155A},     	
{0x0F12, 0x01CC},	
{0x002A, 0x157E},	
{0x0F12, 0x0C80},	
{0x0F12, 0x0578},	
{0x002A, 0x157C},	
{0x0F12, 0x0190},	
{0x002A, 0x1570},	
{0x0F12, 0x00A0},	
{0x0F12, 0x0010},	
{0x002A, 0x12C4},	
{0x0F12, 0x006A},	
{0x002A, 0x12C8},	
{0x0F12, 0x08AC},	
{0x0F12, 0x0050},	
{0x002A, 0x1696},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x00C6},	
{0x0F12, 0x00C6},	
{0x002A, 0x1690},
{0x0F12, 0x0001},	
{0x002A, 0x12B0},	
{0x0F12, 0x0055},	
{0x0F12, 0x005A},	
{0x002A, 0x337A},	
{0x0F12, 0x0006},	
{0x002A, 0x169E},	
{0x0F12, 0x000A},	
{0x0028, 0xD000},	
{0x002A, 0xF406},	
{0x0F12, 0x1000},	
{0x002A, 0xF40A},	
{0x0F12, 0x6998},	
{0x002A, 0xF418},	
{0x0F12, 0x0078},	
{0x0F12, 0x04FE},	
{0x002A, 0xF52C},	
{0x0F12, 0x8800},	
{0x0028, 0x7000},
{0x002A, 0x12D2},
{0x0F12, 0x0003},	
{0x0F12, 0x0003},	
{0x0F12, 0x0003},	
{0x0F12, 0x0003},	
{0x0F12, 0x0884},	
{0x0F12, 0x08CF},	
{0x0F12, 0x0500},	
{0x0F12, 0x054B},	
{0x0F12, 0x0001},	
{0x0F12, 0x0001},	
{0x0F12, 0x0001},	
{0x0F12, 0x0001},	
{0x0F12, 0x0885},	
{0x0F12, 0x0467},	
{0x0F12, 0x0501},	
{0x0F12, 0x02A5},	
{0x0F12, 0x0001},	
{0x0F12, 0x046A},	
{0x0F12, 0x0001},	
{0x0F12, 0x02A8},	
{0x0F12, 0x0885},	
{0x0F12, 0x08D0},	
{0x0F12, 0x0501},	
{0x0F12, 0x054C},	
{0x0F12, 0x0006},	
{0x0F12, 0x0020},	
{0x0F12, 0x0006},	
{0x0F12, 0x0020},	
{0x0F12, 0x0881},	
{0x0F12, 0x0463},	
{0x0F12, 0x04FD},	
{0x0F12, 0x02A1},	
{0x0F12, 0x0006},	
{0x0F12, 0x0489},	
{0x0F12, 0x0006},	
{0x0F12, 0x02C7},	
{0x0F12, 0x0881},	
{0x0F12, 0x08CC},	
{0x0F12, 0x04FD},	
{0x0F12, 0x0548},	
{0x0F12, 0x03A2},	
{0x0F12, 0x01D3},	
{0x0F12, 0x01E0},	
{0x0F12, 0x00F2},	
{0x0F12, 0x03F2},	
{0x0F12, 0x0223},	
{0x0F12, 0x0230},	
{0x0F12, 0x0142},	
{0x0F12, 0x03A2},	
{0x0F12, 0x063C},	
{0x0F12, 0x01E0},	
{0x0F12, 0x0399},	
{0x0F12, 0x03F2},	
{0x0F12, 0x068C},	
{0x0F12, 0x0230},	
{0x0F12, 0x03E9},	
{0x0F12, 0x0002},	
{0x0F12, 0x0002},	
{0x0F12, 0x0002},	
{0x0F12, 0x0002},	
{0x0F12, 0x003C},	
{0x0F12, 0x003C},	
{0x0F12, 0x003C},	
{0x0F12, 0x003C},	
{0x0F12, 0x01D3},	
{0x0F12, 0x01D3},	
{0x0F12, 0x00F2},	
{0x0F12, 0x00F2},	
{0x0F12, 0x020B},	
{0x0F12, 0x024A},	
{0x0F12, 0x012A},	
{0x0F12, 0x0169},	
{0x0F12, 0x0002},	
{0x0F12, 0x046B},	
{0x0F12, 0x0002},	
{0x0F12, 0x02A9},	
{0x0F12, 0x0419},	
{0x0F12, 0x04A5},	
{0x0F12, 0x0257},	
{0x0F12, 0x02E3},	
{0x0F12, 0x0630},	
{0x0F12, 0x063C},	
{0x0F12, 0x038D},	
{0x0F12, 0x0399},	
{0x0F12, 0x0668},	
{0x0F12, 0x06B3},	
{0x0F12, 0x03C5},	
{0x0F12, 0x0410},	
{0x0F12, 0x0001},	
{0x0F12, 0x0001},	
{0x0F12, 0x0001},	
{0x0F12, 0x0001},	
{0x0F12, 0x03A2},	
{0x0F12, 0x01D3},	
{0x0F12, 0x01E0},	
{0x0F12, 0x00F2},	
{0x0F12, 0x0000},	
{0x0F12, 0x0461},	
{0x0F12, 0x0000},	
{0x0F12, 0x029F},	
{0x0F12, 0x0000},	
{0x0F12, 0x063C},	
{0x0F12, 0x0000},	
{0x0F12, 0x0399},	
{0x0F12, 0x003D},	
{0x0F12, 0x003D},	
{0x0F12, 0x003D},	
{0x0F12, 0x003D},	
{0x0F12, 0x01D0},	
{0x0F12, 0x01D0},	
{0x0F12, 0x00EF},	
{0x0F12, 0x00EF},	
{0x0F12, 0x020C},	
{0x0F12, 0x024B},	
{0x0F12, 0x012B},	
{0x0F12, 0x016A},	
{0x0F12, 0x039F},	
{0x0F12, 0x045E},	
{0x0F12, 0x01DD},	
{0x0F12, 0x029C},	
{0x0F12, 0x041A},	
{0x0F12, 0x04A6},	
{0x0F12, 0x0258},	
{0x0F12, 0x02E4},	
{0x0F12, 0x062D},	
{0x0F12, 0x0639},	
{0x0F12, 0x038A},	
{0x0F12, 0x0396},	
{0x0F12, 0x0669},	
{0x0F12, 0x06B4},	
{0x0F12, 0x03C6},	
{0x0F12, 0x0411},	
{0x0F12, 0x087C},	
{0x0F12, 0x08C7},	
{0x0F12, 0x04F8},	
{0x0F12, 0x0543},	
{0x0F12, 0x0040},	
{0x0F12, 0x0040},	
{0x0F12, 0x0040},	
{0x0F12, 0x0040},	
{0x0F12, 0x01D0},	
{0x0F12, 0x01D0},	
{0x0F12, 0x00EF},	
{0x0F12, 0x00EF},	
{0x0F12, 0x020F},	
{0x0F12, 0x024E},	
{0x0F12, 0x012E},	
{0x0F12, 0x016D},	
{0x0F12, 0x039F},	
{0x0F12, 0x045E},	
{0x0F12, 0x01DD},	
{0x0F12, 0x029C},	
{0x0F12, 0x041D},	
{0x0F12, 0x04A9},	
{0x0F12, 0x025B},	
{0x0F12, 0x02E7},	
{0x0F12, 0x062D},	
{0x0F12, 0x0639},	
{0x0F12, 0x038A},	
{0x0F12, 0x0396},	
{0x0F12, 0x066C},	
{0x0F12, 0x06B7},	
{0x0F12, 0x03C9},	
{0x0F12, 0x0414},	
{0x0F12, 0x087C},	
{0x0F12, 0x08C7},	
{0x0F12, 0x04F8},	
{0x0F12, 0x0543},	
{0x0F12, 0x0040},	
{0x0F12, 0x0040},	
{0x0F12, 0x0040},	
{0x0F12, 0x0040},	
{0x0F12, 0x01D0},	
{0x0F12, 0x01D0},	
{0x0F12, 0x00EF},	
{0x0F12, 0x00EF},	
{0x0F12, 0x020F},	
{0x0F12, 0x024E},	
{0x0F12, 0x012E},	
{0x0F12, 0x016D},	
{0x0F12, 0x039F},	
{0x0F12, 0x045E},	
{0x0F12, 0x01DD},	
{0x0F12, 0x029C},	
{0x0F12, 0x041D},	
{0x0F12, 0x04A9},	
{0x0F12, 0x025B},	
{0x0F12, 0x02E7},	
{0x0F12, 0x062D},	
{0x0F12, 0x0639},	
{0x0F12, 0x038A},	
{0x0F12, 0x0396},	
{0x0F12, 0x066C},	
{0x0F12, 0x06B7},	
{0x0F12, 0x03C9},	
{0x0F12, 0x0414},	
{0x0F12, 0x087C},	
{0x0F12, 0x08C7},	
{0x0F12, 0x04F8},	
{0x0F12, 0x0543},	
{0x0F12, 0x003D},	
{0x0F12, 0x003D},	
{0x0F12, 0x003D},	
{0x0F12, 0x003D},	
{0x0F12, 0x01D2},	
{0x0F12, 0x01D2},	
{0x0F12, 0x00F1},	
{0x0F12, 0x00F1},	
{0x0F12, 0x020C},	
{0x0F12, 0x024B},	
{0x0F12, 0x012B},	
{0x0F12, 0x016A},	
{0x0F12, 0x03A1},	
{0x0F12, 0x0460},	
{0x0F12, 0x01DF},	
{0x0F12, 0x029E},	
{0x0F12, 0x041A},	
{0x0F12, 0x04A6},	
{0x0F12, 0x0258},	
{0x0F12, 0x02E4},	
{0x0F12, 0x062F},	
{0x0F12, 0x063B},	
{0x0F12, 0x038C},	
{0x0F12, 0x0398},	
{0x0F12, 0x0669},	
{0x0F12, 0x06B4},	
{0x0F12, 0x03C6},	
{0x0F12, 0x0411},	
{0x0F12, 0x087E},	
{0x0F12, 0x08C9},	
{0x0F12, 0x04FA},	
{0x0F12, 0x0545},	
{0x0F12, 0x03A2},	
{0x0F12, 0x01D3},	
{0x0F12, 0x01E0},	
{0x0F12, 0x00F2},	
{0x0F12, 0x03AF},	
{0x0F12, 0x01E0},	
{0x0F12, 0x01ED},	
{0x0F12, 0x00FF},	
{0x0F12, 0x0000},	
{0x0F12, 0x0461},	
{0x0F12, 0x0000},	
{0x0F12, 0x029F},	
{0x0F12, 0x0000},	
{0x0F12, 0x046E},	
{0x0F12, 0x0000},	
{0x0F12, 0x02AC},	
{0x0F12, 0x0000},	
{0x0F12, 0x063C},	
{0x0F12, 0x0000},	
{0x0F12, 0x0399},	
{0x0F12, 0x0000},	
{0x0F12, 0x0649},	
{0x0F12, 0x0000},	
{0x0F12, 0x03A6},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x03AA},	
{0x0F12, 0x01DB},	
{0x0F12, 0x01E8},	
{0x0F12, 0x00FA},	
{0x0F12, 0x03B7},	
{0x0F12, 0x01E8},	
{0x0F12, 0x01F5},	
{0x0F12, 0x0107},	
{0x0F12, 0x0000},	
{0x0F12, 0x0469},	
{0x0F12, 0x0000},	
{0x0F12, 0x02A7},	
{0x0F12, 0x0000},	
{0x0F12, 0x0476},	
{0x0F12, 0x0000},	
{0x0F12, 0x02B4},	
{0x0F12, 0x0000},	
{0x0F12, 0x0644},	
{0x0F12, 0x0000},	
{0x0F12, 0x03A1},	
{0x0F12, 0x0000},	
{0x0F12, 0x0651},	
{0x0F12, 0x0000},	
{0x0F12, 0x03AE},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0001},	
{0x0F12, 0x0001},	
{0x0F12, 0x0001},	
{0x0F12, 0x0001},	
{0x0F12, 0x000F},	
{0x0F12, 0x000F},	
{0x0F12, 0x000F},	
{0x0F12, 0x000F},	
{0x0F12, 0x05AD},	
{0x0F12, 0x03DE},	
{0x0F12, 0x030A},	
{0x0F12, 0x021C},	
{0x0F12, 0x062F},	
{0x0F12, 0x0460},	
{0x0F12, 0x038C},	
{0x0F12, 0x029E},	
{0x0F12, 0x07FC},	
{0x0F12, 0x0847},	
{0x0F12, 0x0478},	
{0x0F12, 0x04C3},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	

#if 0   /////Af config
{0x002a, 0x01de},
{0x0f12, 0x0003},
{0x002a, 0x01e4},
{0x0f12, 0x0041},
{0x002a, 0x1196},
{0x0f12, 0x0000},
{0x002a, 0x01e8},
{0x0f12, 0x2a0c},
{0x0f12, 0x0190},

{0x002a, 0x025a},
{0x0f12, 0x0100},
{0x0f12, 0x00e3},
{0x0f12, 0x0200},
{0x0f12, 0x0238},
{0x0f12, 0x018c},
{0x0f12, 0x0166},
{0x0f12, 0x00e6},
{0x0f12, 0x0132},
{0x0f12, 0x0001},
{0x002a, 0x0586},
{0x0f12, 0x00ff},
{0x002a, 0x115e},
{0x0f12, 0x0003},
{0x002a, 0x10d4},
{0x0f12, 0x1000},
{0x002a, 0x10de},
{0x0f12, 0x0004},
{0x002a, 0x106c},
{0x0f12, 0x0202},
{0x002a, 0x10ca},
{0x0f12, 0x00c0},
{0x002a, 0x1060},
{0x0f12, 0x003f},
{0x0f12, 0x6c3f},
{0x002a, 0x10f4},
{0x0f12, 0x0280},
{0x002a, 0x1100},
{0x0f12, 0x0305},
{0x0f12, 0x0320},
{0x002a, 0x1154},
{0x0f12, 0x0060},
{0x002a, 0x10e2},
{0x0f12, 0x0000},
{0x002a, 0x1072},
{0x0f12, 0x003f},
{0x002a, 0x1074},
{0x0f12, 0x000b},
{0x0f12, 0x0000},
{0x0f12, 0x0043},
{0x0f12, 0x0048},
{0x0f12, 0x004c},
{0x0f12, 0x0050},
{0x0f12, 0x0054},
{0x0f12, 0x0058},
{0x0f12, 0x005c},
{0x0f12, 0x0060},
{0x0f12, 0x0064},
{0x0f12, 0x0068},
{0x0f12, 0x006c},
{0x002a, 0x0252},
{0x0f12, 0x0003},
#endif
//============================================================
// ISP-FE Setting
//============================================================
{0x002A, 0x158A},
{0x0F12, 0xEAF0},
{0x002A, 0x15C6},
{0x0F12, 0x0020},
{0x0F12, 0x0060},
{0x002A, 0x15BC},
{0x0F12, 0x0200},	
{0x002A, 0x1608}, 
{0x0F12, 0x0100},	
{0x0F12, 0x0100},	
{0x0F12, 0x0100},	
{0x0F12, 0x0100},	
{0x002A, 0x0454},
{0x0F12, 0x0055},
{0x0F12, 0x0001},	
{0x0F12, 0x0140},	
{0x0F12, 0x00F0},	
{0x0F12, 0x0000},	
{0x002A, 0x0F70},
{0x0F12, 0x0042},	// #TVAR_ae_BrAve
{0x002A, 0x0F76},
{0x0F12, 0x000f},	//Disable illumination & contrast  // #ae_StatMode  //5
{0x002A, 0x0F7E},
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0303},	
{0x0F12, 0x0303},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0303},	
{0x0F12, 0x0303},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0303},	
{0x0F12, 0x0303},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0303},	
{0x0F12, 0x0303},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x0F12, 0x0101},	
{0x002A, 0x0C18},
{0x0F12, 0x0001},	
{0x002A, 0x04D2},
{0x0F12, 0x067F},	
#if 0 
{0x002A, 0x06CE}, //TVAR_ash_GASAlpha   
{0x0F12, 0x00ED}, 
{0x0F12, 0x00EB},
{0x0F12, 0x00EB},
{0x0F12, 0x00E9},
{0x0F12, 0x00B0}, 
#else //zhao
{0x002A, 0x06CE}, //TVAR_ash_GASAlpha   
{0x0F12, 0x0100}, 
{0x0F12, 0x00F0},
{0x0F12, 0x00F0},
{0x0F12, 0x0100},

{0x0F12, 0x0100}, 
#endif
{0x0F12, 0x00EB},
{0x0F12, 0x00EB},
{0x0F12, 0x00F5},
 
{0x0F12, 0x00e0}, 
{0x0F12, 0x00EB},
{0x0F12, 0x00EB},
{0x0F12, 0x00F5},
 
{0x0F12, 0x00E0}, 
{0x0F12, 0x00EB},
{0x0F12, 0x00EB},
{0x0F12, 0x0100},

{0x0F12, 0x00ED}, 
{0x0F12, 0x00EB},
{0x0F12, 0x00EB},
{0x0F12, 0x00E9},
{0x0F12, 0x00ED}, 
{0x0F12, 0x00EB},
{0x0F12, 0x00EB},
{0x0F12, 0x00E9},
{0x0F12, 0x00ED}, 
{0x0F12, 0x00EB},
{0x0F12, 0x00EB},
{0x0F12, 0x00E9}, 
{0x0F12, 0x00ED}, 
{0x0F12, 0x00EB},
{0x0F12, 0x00EB},
{0x0F12, 0x00E9},
{0x0F12, 0x002D},
{0x0F12, 0x0016},
{0x0F12, 0x0016},
{0x0F12, 0x0000},
{0x0F12, 0x002D},
{0x0F12, 0x0016},
{0x0F12, 0x0016},
{0x0F12, 0x0000},
{0x0F12, 0x002D},
{0x0F12, 0x0016},
{0x0F12, 0x0016},
{0x0F12, 0x0000},
{0x0F12, 0x002D},
{0x0F12, 0x0016},
{0x0F12, 0x0016},
{0x0F12, 0x0000},
{0x0F12, 0x002D},
{0x0F12, 0x0016},
{0x0F12, 0x0016},
{0x0F12, 0x0000},
{0x0F12, 0x002D},
{0x0F12, 0x0016},
{0x0F12, 0x0016},
{0x0F12, 0x0000},
{0x0F12, 0x002D},
{0x0F12, 0x0016},
{0x0F12, 0x0016},
{0x0F12, 0x0000},
{0x0F12, 0x002D},
{0x0F12, 0x0016},
{0x0F12, 0x0016},
{0x0F12, 0x0000},
{0x002A, 0x06B4},
{0x0F12, 0x0001},
{0x002A, 0x075A},  
{0x0F12, 0x0000},  
{0x0F12, 0x0400},  
{0x0F12, 0x0300},  
{0x0F12, 0x0010},  
{0x0F12, 0x0011},  
{0x002A, 0x06C6},
{0x0F12, 0x00ED},  
{0x0F12, 0x00EB},  
{0x0F12, 0x00EB},  
{0x0F12, 0x00E9},  
{0x002A, 0x0E3C},
{0x0F12, 0x00C0},  
{0x002A, 0x074E},
{0x0F12, 0x0000},  
{0x002A, 0x0754},
{0x0F12, 0x347C},
{0x0F12, 0x7000},
{0x002A, 0x347C},
{0x0F12, 0x01D1},  //  1.019D
{0x0F12, 0x018D},  //   0163
{0x0F12, 0x015A},  //   0137
{0x0F12, 0x012A},  //   010B
{0x0F12, 0x010C},  //   00EB
{0x0F12, 0x00FE},  //   00D7
{0x0F12, 0x00FA},  //   00D0
{0x0F12, 0x0102},  //   00D6
{0x0F12, 0x011C},  //   00EC
{0x0F12, 0x014F},  //   0119
{0x0F12, 0x0188},  //   014C
{0x0F12, 0x01BB},  //   0181
{0x0F12, 0x02D0},  //   01CE
{0x0F12, 0x018C},  //   017E
{0x0F12, 0x0157},  //   0149
{0x0F12, 0x0116},  //   010E
{0x0F12, 0x00E3},  //   00DC
{0x0F12, 0x00C3},  //   00B7
{0x0F12, 0x00B4},  //   00A4
{0x0F12, 0x00AF},  //   009E
{0x0F12, 0x00BC},  //   00A3
{0x0F12, 0x00D8},  //   00BD
{0x0F12, 0x010A},  //   00E6
{0x0F12, 0x014F},  //   0125
{0x0F12, 0x0194},  //   0169
{0x0F12, 0x01DA},  //   019C
{0x0F12, 0x0162},  //   014F
{0x0F12, 0x0119},  //   010E
{0x0F12, 0x00D6},  //   00CD
{0x0F12, 0x009E},  //   009B
{0x0F12, 0x007E},  //   0076
{0x0F12, 0x006B},  //   0061
{0x0F12, 0x006B},  //   0058
{0x0F12, 0x007A},  //   0063
{0x0F12, 0x009A},  //   007E
{0x0F12, 0x00C7},  //   00A9
{0x0F12, 0x0110},  //   00E7
{0x0F12, 0x0168},  //   0136
{0x0F12, 0x01B3},  //   017E
{0x0F12, 0x0134},  //   0129
{0x0F12, 0x00E8},  //   00E1
{0x0F12, 0x00A3},  //   009F
{0x0F12, 0x006E},  //   006B
{0x0F12, 0x0049},  //   0046
{0x0F12, 0x0034},  //   0030
{0x0F12, 0x0034},  //   0029
{0x0F12, 0x0043},  //   0033
{0x0F12, 0x0065},  //   004F
{0x0F12, 0x0099},  //   007F
{0x0F12, 0x00DF},  //   00BD
{0x0F12, 0x0136},  //   0111
{0x0F12, 0x018D},  //   015D
{0x0F12, 0x0114},  //   0110
{0x0F12, 0x00C8},  //   00C6
{0x0F12, 0x0081},  //   0082
{0x0F12, 0x004A},  //   004B
{0x0F12, 0x0025},  //   0026
{0x0F12, 0x0012},  //   0011
{0x0F12, 0x0010},  //   000C
{0x0F12, 0x0021},  //   0016
{0x0F12, 0x0044},  //   0032
{0x0F12, 0x0078},  //   0061
{0x0F12, 0x00C1},  //   00A1
{0x0F12, 0x0119},  //   00F4
{0x0F12, 0x0173},  //   014C
{0x0F12, 0x0107},  //   0102
{0x0F12, 0x00B8},  //   00BB
{0x0F12, 0x0071},  //   0075
{0x0F12, 0x003A},  //   003F
{0x0F12, 0x0014},  //   0019
{0x0F12, 0x0002},  //   0005
{0x0F12, 0x0000},  //   0000
{0x0F12, 0x0010},  //   000A
{0x0F12, 0x0035},  //   0025
{0x0F12, 0x006B},  //   0055
{0x0F12, 0x00B2},  //   0098
{0x0F12, 0x010C},  //   00EA
{0x0F12, 0x016A},  //   0143
{0x0F12, 0x0109},  //   0106
{0x0F12, 0x00B9},  //   00BF
{0x0F12, 0x0072},  //   007B
{0x0F12, 0x003B},  //   0043
{0x0F12, 0x0016},  //   001F
{0x0F12, 0x0004},  //   000D
{0x0F12, 0x0002},  //   0006
{0x0F12, 0x0013},  //   0010
{0x0F12, 0x0038},  //   002C
{0x0F12, 0x0074},  //   005D
{0x0F12, 0x00B6},  //   009D
{0x0F12, 0x010D},  //   00F2
{0x0F12, 0x016B},  //   0147
{0x0F12, 0x0119},  //   0115
{0x0F12, 0x00C9},  //   00D2
{0x0F12, 0x0083},  //   008C
{0x0F12, 0x004F},  //   0059
{0x0F12, 0x002B},  //   0034
{0x0F12, 0x0016},  //   0022
{0x0F12, 0x0016},  //   001B
{0x0F12, 0x0028},  //   0027
{0x0F12, 0x004D},  //   0047
{0x0F12, 0x0088},  //   0077
{0x0F12, 0x00D0},  //   00B6
{0x0F12, 0x0123},  //   0108
{0x0F12, 0x0177},  //   015F
{0x0F12, 0x0132},  //   0136
{0x0F12, 0x00EC},  //   00F1
{0x0F12, 0x00A7},  //   00AE
{0x0F12, 0x0076},  //   007C
{0x0F12, 0x004E},  //   0058
{0x0F12, 0x003D},  //   0046
{0x0F12, 0x003C},  //   0040
{0x0F12, 0x004E},  //   004E
{0x0F12, 0x0073},  //   006C
{0x0F12, 0x00AC},  //   009C
{0x0F12, 0x00F3},  //   00DB
{0x0F12, 0x0148},  //   012F
{0x0F12, 0x0195},  //   017C
{0x0F12, 0x0163},  //   015C
{0x0F12, 0x011F},  //   0120
{0x0F12, 0x00D9},  //   00DF
{0x0F12, 0x00AB},  //   00AF
{0x0F12, 0x0086},  //   008F
{0x0F12, 0x0075},  //   007D
{0x0F12, 0x0075},  //   0079
{0x0F12, 0x0089},  //   0084
{0x0F12, 0x00AB},  //   00A3
{0x0F12, 0x00E2},  //   00D1
{0x0F12, 0x012A},  //   0110
{0x0F12, 0x017B},  //   015E
{0x0F12, 0x01B8},  //   019A
{0x0F12, 0x0195},  //   0178
{0x0F12, 0x0157},  //   0144
{0x0F12, 0x0113},  //   010C
{0x0F12, 0x00E3},  //   00DF
{0x0F12, 0x00C1},  //   00C1
{0x0F12, 0x00B4},  //   00B3
{0x0F12, 0x00B7},  //   00B0
{0x0F12, 0x00C7},  //   00BC
{0x0F12, 0x00EA},  //   00D6
{0x0F12, 0x0121},  //   0103
{0x0F12, 0x0162},  //   0144
{0x0F12, 0x01B0},  //   0187
{0x0F12, 0x01DA},  //   01C2
{0x0F12, 0x018C},  //   0167
{0x0F12, 0x014A},  //   013A
{0x0F12, 0x0121},  //   010D
{0x0F12, 0x00F5},  //   00E5
{0x0F12, 0x00D7},  //   00C6
{0x0F12, 0x00CA},  //   00B7
{0x0F12, 0x00C5},  //   00B0
{0x0F12, 0x00CA},  //   00B6
{0x0F12, 0x00E4},  //   00C9
{0x0F12, 0x010B},  //   00EC
{0x0F12, 0x013C},  //   011C
{0x0F12, 0x0168},  //   014B
{0x0F12, 0x0260},  //   0192
{0x0F12, 0x0151},  //   0155
{0x0F12, 0x011A},  //   0125
{0x0F12, 0x00E3},  //   00EE
{0x0F12, 0x00B7},  //   00BF
{0x0F12, 0x009B},  //   00A2
{0x0F12, 0x008C},  //   008D
{0x0F12, 0x008B},  //   0087
{0x0F12, 0x0093},  //   008F
{0x0F12, 0x00A8},  //   00A1
{0x0F12, 0x00D2},  //   00C5
{0x0F12, 0x0107},  //   00F8
{0x0F12, 0x0140},  //   0135
{0x0F12, 0x0181},  //   0166
{0x0F12, 0x0129},  //   012F
{0x0F12, 0x00E7},  //   00F2
{0x0F12, 0x00AE},  //   00B6
{0x0F12, 0x0083},  //   0089
{0x0F12, 0x0066},  //   0068
{0x0F12, 0x0056},  //   0055
{0x0F12, 0x0055},  //   004F
{0x0F12, 0x0061},  //   0058
{0x0F12, 0x0079},  //   006E
{0x0F12, 0x009E},  //   0092
{0x0F12, 0x00D6},  //   00C5
{0x0F12, 0x011B},  //   0109
{0x0F12, 0x015E},  //   0147
{0x0F12, 0x0102},  //   010D
{0x0F12, 0x00BC},  //   00C9
{0x0F12, 0x0086},  //   008E
{0x0F12, 0x005C},  //   0061
{0x0F12, 0x003C},  //   003E
{0x0F12, 0x002C},  //   002A
{0x0F12, 0x002B},  //   0025
{0x0F12, 0x0038},  //   002F
{0x0F12, 0x0053},  //   0047
{0x0F12, 0x0079},  //   006F
{0x0F12, 0x00AF},  //   00A2
{0x0F12, 0x00F9},  //   00E9
{0x0F12, 0x0141},  //   0130
{0x0F12, 0x00E4},  //   00F3
{0x0F12, 0x00A1},  //   00B1
{0x0F12, 0x006B},  //   0076
{0x0F12, 0x003F},  //   0045
{0x0F12, 0x0020},  //   0022
{0x0F12, 0x000F},  //   000F
{0x0F12, 0x000D},  //   000A
{0x0F12, 0x001C},  //   0015
{0x0F12, 0x003A},  //   002E
{0x0F12, 0x0061},  //   0058
{0x0F12, 0x0099},  //   008D
{0x0F12, 0x00E0},  //   00D4
{0x0F12, 0x012D},  //   011A
{0x0F12, 0x00D9},  //   00E9
{0x0F12, 0x0093},  //   00A7
{0x0F12, 0x005B},  //   0068
{0x0F12, 0x0030},  //   0038
{0x0F12, 0x0011},  //   0017
{0x0F12, 0x0002},  //   0004
{0x0F12, 0x0000},  //   0000
{0x0F12, 0x000F},  //   000B
{0x0F12, 0x002D},  //   0025
{0x0F12, 0x0058},  //   004F
{0x0F12, 0x008E},  //   0084
{0x0F12, 0x00D8},  //   00CB
{0x0F12, 0x0127},  //   0117
{0x0F12, 0x00D8},  //   00EA
{0x0F12, 0x0092},  //   00A8
{0x0F12, 0x005B},  //   006E
{0x0F12, 0x0030},  //   003D
{0x0F12, 0x0012},  //   001B
{0x0F12, 0x0003},  //   0009
{0x0F12, 0x0002},  //   0006
{0x0F12, 0x0011},  //   0010
{0x0F12, 0x0031},  //   002B
{0x0F12, 0x0060},  //   0056
{0x0F12, 0x0093},  //   008B
{0x0F12, 0x00DC},  //   00D1
{0x0F12, 0x0129},  //   011B
{0x0F12, 0x00E7},  //   00F9
{0x0F12, 0x00A1},  //   00B6
{0x0F12, 0x0068},  //   007D
{0x0F12, 0x003F},  //   004E
{0x0F12, 0x0022},  //   002D
{0x0F12, 0x0012},  //   001C
{0x0F12, 0x0013},  //   0019
{0x0F12, 0x0023},  //   0025
{0x0F12, 0x0043},  //   0042
{0x0F12, 0x0072},  //   006C
{0x0F12, 0x00A7},  //   00A0
{0x0F12, 0x00EA},  //   00E6
{0x0F12, 0x0135},  //   0130
{0x0F12, 0x0100},  //   0114
{0x0F12, 0x00BD},  //   00D5
{0x0F12, 0x0085},  //   0099
{0x0F12, 0x005D},  //   006D
{0x0F12, 0x003F},  //   004E
{0x0F12, 0x0032},  //   003E
{0x0F12, 0x0035},  //   003C
{0x0F12, 0x0045},  //   0049
{0x0F12, 0x0064},  //   0065
{0x0F12, 0x0091},  //   008D
{0x0F12, 0x00CA},  //   00C2
{0x0F12, 0x0107},  //   0109
{0x0F12, 0x014D},  //   014C
{0x0F12, 0x0127},  //   0135
{0x0F12, 0x00E9},  //   00FC
{0x0F12, 0x00B1},  //   00C2
{0x0F12, 0x0089},  //   0099
{0x0F12, 0x006F},  //   007D
{0x0F12, 0x0062},  //   006F
{0x0F12, 0x0066},  //   006D
{0x0F12, 0x0077},  //   007C
{0x0F12, 0x0094},  //   0095
{0x0F12, 0x00BF},  //   00BC
{0x0F12, 0x00F9},  //   00F1
{0x0F12, 0x0137},  //   0135
{0x0F12, 0x016A},  //   016E
{0x0F12, 0x0153},  //   0154
{0x0F12, 0x011A},  //   011D
{0x0F12, 0x00E3},  //   00E9
{0x0F12, 0x00BD},  //   00C2
{0x0F12, 0x00A1},  //   00A7
{0x0F12, 0x0099},  //   009C
{0x0F12, 0x009D},  //   009B
{0x0F12, 0x00AC},  //   00A8
{0x0F12, 0x00CB},  //   00C2
{0x0F12, 0x00F7},  //   00E8
{0x0F12, 0x012B},  //   011C
{0x0F12, 0x016D},  //   015C
{0x0F12, 0x0188},  //   018F
{0x0F12, 0x016E},  //   0158
{0x0F12, 0x0135},  //   012B
{0x0F12, 0x010B},  //   0100
{0x0F12, 0x00E4},  //   00DA
{0x0F12, 0x00CA},  //   00BF
{0x0F12, 0x00C2},  //   00AE
{0x0F12, 0x00C2},  //   00AD
{0x0F12, 0x00CD},  //   00B8
{0x0F12, 0x00E9},  //   00D2
{0x0F12, 0x0119},  //   00FB
{0x0F12, 0x0149},  //   012C
{0x0F12, 0x0177},  //   015B
{0x0F12, 0x0262},  //   01A0
{0x0F12, 0x013F},  //   0150
{0x0F12, 0x010E},  //   011F
{0x0F12, 0x00D9},  //   00E7
{0x0F12, 0x00AF},  //   00BA
{0x0F12, 0x0095},  //   009D
{0x0F12, 0x0089},  //   008C
{0x0F12, 0x008B},  //   008B
{0x0F12, 0x009A},  //   0095
{0x0F12, 0x00B4},  //   00AF
{0x0F12, 0x00E1},  //   00D6
{0x0F12, 0x0119},  //   010E
{0x0F12, 0x0151},  //   014C
{0x0F12, 0x0192},  //   017C
{0x0F12, 0x011F},  //   012E
{0x0F12, 0x00DE},  //   00EE
{0x0F12, 0x00A9},  //   00B4
{0x0F12, 0x007E},  //   0088
{0x0F12, 0x0062},  //   0068
{0x0F12, 0x0054},  //   0055
{0x0F12, 0x0057},  //   0050
{0x0F12, 0x0069},  //   005E
{0x0F12, 0x0085},  //   007A
{0x0F12, 0x00AD},  //   00A4
{0x0F12, 0x00E7},  //   00DA
{0x0F12, 0x012F},  //   0121
{0x0F12, 0x016F},  //   0161
{0x0F12, 0x0102},  //   010B
{0x0F12, 0x00BA},  //   00C9
{0x0F12, 0x0084},  //   008E
{0x0F12, 0x005B},  //   0061
{0x0F12, 0x003C},  //   003F
{0x0F12, 0x002C},  //   002B
{0x0F12, 0x002D},  //   0028
{0x0F12, 0x003E},  //   0034
{0x0F12, 0x005D},  //   0052
{0x0F12, 0x0087},  //   007D
{0x0F12, 0x00BF},  //   00B4
{0x0F12, 0x0108},  //   00F9
{0x0F12, 0x014F},  //   0141
{0x0F12, 0x00E8},  //   00F9
{0x0F12, 0x00A3},  //   00B3
{0x0F12, 0x006D},  //   0079
{0x0F12, 0x0040},  //   0048
{0x0F12, 0x0021},  //   0024
{0x0F12, 0x0010},  //   0010
{0x0F12, 0x000F},  //   000C
{0x0F12, 0x0020},  //   0018
{0x0F12, 0x0041},  //   0035
{0x0F12, 0x006C},  //   0062
{0x0F12, 0x00A4},  //   009A
{0x0F12, 0x00E9},  //   00DF
{0x0F12, 0x0137},  //   0128
{0x0F12, 0x00DF},  //   00F2
{0x0F12, 0x009A},  //   00AE
{0x0F12, 0x0061},  //   0071
{0x0F12, 0x0035},  //   003E
{0x0F12, 0x0013},  //   001B
{0x0F12, 0x0002},  //   0005
{0x0F12, 0x0000},  //   0000
{0x0F12, 0x0011},  //   000C
{0x0F12, 0x0031},  //   0029
{0x0F12, 0x005E},  //   0053
{0x0F12, 0x0092},  //   008A
{0x0F12, 0x00DB},  //   00D1
{0x0F12, 0x0129},  //   0118
{0x0F12, 0x00E2},  //   00F4
{0x0F12, 0x009B},  //   00B2
{0x0F12, 0x0064},  //   0076
{0x0F12, 0x0036},  //   0044
{0x0F12, 0x0015},  //   0020
{0x0F12, 0x0003},  //   000B
{0x0F12, 0x0001},  //   0005
{0x0F12, 0x0010},  //   000F
{0x0F12, 0x0031},  //   002C
{0x0F12, 0x005D},  //   0055
{0x0F12, 0x0091},  //   008A
{0x0F12, 0x00D7},  //   00CF
{0x0F12, 0x0124},  //   0117
{0x0F12, 0x00F3},  //   0106
{0x0F12, 0x00AB},  //   00C2
{0x0F12, 0x0072},  //   0088
{0x0F12, 0x0046},  //   0057
{0x0F12, 0x0025},  //   0033
{0x0F12, 0x0013},  //   001F
{0x0F12, 0x0010},  //   0017
{0x0F12, 0x001F},  //   0021
{0x0F12, 0x003D},  //   003C
{0x0F12, 0x006A},  //   0065
{0x0F12, 0x009F},  //   0099
{0x0F12, 0x00DF},  //   00DC
{0x0F12, 0x012B},  //   0125
{0x0F12, 0x010E},  //   0125
{0x0F12, 0x00CA},  //   00E2
{0x0F12, 0x0090},  //   00A4
{0x0F12, 0x0066},  //   0077
{0x0F12, 0x0044},  //   0053
{0x0F12, 0x0034},  //   003F
{0x0F12, 0x0031},  //   0038
{0x0F12, 0x003E},  //   0042
{0x0F12, 0x005A},  //   005B
{0x0F12, 0x0086},  //   0081
{0x0F12, 0x00BA},  //   00B3
{0x0F12, 0x00F8},  //   00F8
{0x0F12, 0x013B},  //   013D
{0x0F12, 0x0137},  //   0148
{0x0F12, 0x00F8},  //   010C
{0x0F12, 0x00C0},  //   00D2
{0x0F12, 0x0094},  //   00A4
{0x0F12, 0x0075},  //   0084
{0x0F12, 0x0064},  //   0071
{0x0F12, 0x0061},  //   006A
{0x0F12, 0x006D},  //   0072
{0x0F12, 0x0087},  //   0089
{0x0F12, 0x00AE},  //   00AC
{0x0F12, 0x00E3},  //   00DE
{0x0F12, 0x0121},  //   011E
{0x0F12, 0x0157},  //   015A
{0x0F12, 0x0161},  //   0167
{0x0F12, 0x0129},  //   0130
{0x0F12, 0x00F0},  //   00FC
{0x0F12, 0x00C9},  //   00D1
{0x0F12, 0x00AA},  //   00B5
{0x0F12, 0x009B},  //   00A2
{0x0F12, 0x0099},  //   009D
{0x0F12, 0x00A3},  //   00A2
{0x0F12, 0x00BB},  //   00B8
{0x0F12, 0x00E1},  //   00D9
{0x0F12, 0x0113},  //   0106
{0x0F12, 0x0151},  //   0140
{0x0F12, 0x0173},  //   0174
{0x0F12, 0x0156},  //   0139
{0x0F12, 0x0121},  //   0111
{0x0F12, 0x00F7},  //   00EC
{0x0F12, 0x00D6},  //   00C6
{0x0F12, 0x00C1},  //   00AF
{0x0F12, 0x00B9},  //   00A4
{0x0F12, 0x00BA},  //   00A2
{0x0F12, 0x00BF},  //   00AD
{0x0F12, 0x00D7},  //   00C2
{0x0F12, 0x0101},  //   00E6
{0x0F12, 0x0134},  //   0111
{0x0F12, 0x0159},  //   0141
{0x0F12, 0x0243},  //   017D
{0x0F12, 0x011F},  //   012B
{0x0F12, 0x00F3},  //   00FF
{0x0F12, 0x00C3},  //   00CD
{0x0F12, 0x00A1},  //   00A5
{0x0F12, 0x008B},  //   008F
{0x0F12, 0x0085},  //   0082
{0x0F12, 0x0086},  //   0082
{0x0F12, 0x0091},  //   0089
{0x0F12, 0x00A7},  //   00A0
{0x0F12, 0x00CE},  //   00C2
{0x0F12, 0x0103},  //   00F2
{0x0F12, 0x0135},  //   012C
{0x0F12, 0x016C},  //   0156
{0x0F12, 0x00F7},  //   0102
{0x0F12, 0x00C1},  //   00CB
{0x0F12, 0x0091},  //   009B
{0x0F12, 0x006F},  //   0075
{0x0F12, 0x005B},  //   005D
{0x0F12, 0x0051},  //   004F
{0x0F12, 0x0057},  //   004E
{0x0F12, 0x0064},  //   0059
{0x0F12, 0x007A},  //   006F
{0x0F12, 0x0099},  //   0091
{0x0F12, 0x00CF},  //   00BE
{0x0F12, 0x0110},  //   00FD
{0x0F12, 0x014C},  //   0134
{0x0F12, 0x00D1},  //   00E1
{0x0F12, 0x009A},  //   00A5
{0x0F12, 0x006D},  //   0075
{0x0F12, 0x004D},  //   004F
{0x0F12, 0x0035},  //   0035
{0x0F12, 0x002B},  //   0028
{0x0F12, 0x002C},  //   0025
{0x0F12, 0x0039},  //   0030
{0x0F12, 0x0051},  //   0048
{0x0F12, 0x0072},  //   006C
{0x0F12, 0x00A2},  //   009A
{0x0F12, 0x00E8},  //   00D6
{0x0F12, 0x0124},  //   0119
{0x0F12, 0x00B7},  //   00CA
{0x0F12, 0x007F},  //   0090
{0x0F12, 0x0053},  //   005C
{0x0F12, 0x0031},  //   0036
{0x0F12, 0x001C},  //   001B
{0x0F12, 0x000F},  //   000D
{0x0F12, 0x000F},  //   000B
{0x0F12, 0x001C},  //   0015
{0x0F12, 0x0035},  //   002A
{0x0F12, 0x0057},  //   004F
{0x0F12, 0x0084},  //   007C
{0x0F12, 0x00C6},  //   00B9
{0x0F12, 0x0109},  //   00FA
{0x0F12, 0x00AA},  //   00BF
{0x0F12, 0x0072},  //   0086
{0x0F12, 0x0046},  //   0053
{0x0F12, 0x0024},  //   002C
{0x0F12, 0x000D},  //   0010
{0x0F12, 0x0003},  //   0002
{0x0F12, 0x0000},  //   0000
{0x0F12, 0x000D},  //   0007
{0x0F12, 0x0024},  //   001D
{0x0F12, 0x0047},  //   0040
{0x0F12, 0x0073},  //   006B
{0x0F12, 0x00B4},  //   00A8
{0x0F12, 0x00FD},  //   00EC
{0x0F12, 0x00AB},  //   00C4
{0x0F12, 0x0071},  //   0089
{0x0F12, 0x0045},  //   0057
{0x0F12, 0x0024},  //   002F
{0x0F12, 0x000D},  //   0015
{0x0F12, 0x0002},  //   0008
{0x0F12, 0x0000},  //   0003
{0x0F12, 0x000A},  //   000B
{0x0F12, 0x0023},  //   001E
{0x0F12, 0x0047},  //   003F
{0x0F12, 0x006E},  //   006B
{0x0F12, 0x00AA},  //   00A6
{0x0F12, 0x00F3},  //   00E5
{0x0F12, 0x00B9},  //   00D2
{0x0F12, 0x007E},  //   0097
{0x0F12, 0x0050},  //   0065
{0x0F12, 0x0031},  //   0041
{0x0F12, 0x001B},  //   0027
{0x0F12, 0x000F},  //   0018
{0x0F12, 0x000D},  //   0014
{0x0F12, 0x0017},  //   001A
{0x0F12, 0x002E},  //   002E
{0x0F12, 0x0052},  //   004F
{0x0F12, 0x007C},  //   0076
{0x0F12, 0x00B2},  //   00B3
{0x0F12, 0x00F5},  //   00F1
{0x0F12, 0x00D4},  //   00EE
{0x0F12, 0x009B},  //   00B3
{0x0F12, 0x006E},  //   0082
{0x0F12, 0x0051},  //   005D
{0x0F12, 0x0038},  //   0043
{0x0F12, 0x002D},  //   0036
{0x0F12, 0x002A},  //   0031
{0x0F12, 0x0035},  //   0037
{0x0F12, 0x0049},  //   004B
{0x0F12, 0x006A},  //   0067
{0x0F12, 0x0097},  //   0092
{0x0F12, 0x00CA},  //   00CD
{0x0F12, 0x0105},  //   0107
{0x0F12, 0x0100},  //   0110
{0x0F12, 0x00C9},  //   00DA
{0x0F12, 0x009A},  //   00AA
{0x0F12, 0x007D},  //   0086
{0x0F12, 0x0068},  //   006F
{0x0F12, 0x005C},  //   0061
{0x0F12, 0x005A},  //   005B
{0x0F12, 0x0063},  //   0061
{0x0F12, 0x0072},  //   0072
{0x0F12, 0x0092},  //   008D
{0x0F12, 0x00BF},  //   00B6
{0x0F12, 0x00F6},  //   00F1
{0x0F12, 0x011D},  //   0129
{0x0F12, 0x0126},  //   0134
{0x0F12, 0x00FE},  //   0102
{0x0F12, 0x00CE},  //   00D2
{0x0F12, 0x00B0},  //   00B0
{0x0F12, 0x009B},  //   009A
{0x0F12, 0x0094},  //   008D
{0x0F12, 0x0093},  //   0089
{0x0F12, 0x0096},  //   008C
{0x0F12, 0x00A9},  //   0099
{0x0F12, 0x00C6},  //   00B2
{0x0F12, 0x00F3},  //   00D9
{0x0F12, 0x012B},  //   010E
{0x0F12, 0x0135},  //   0142
{0x002A, 0x0D30},
{0x0F12, 0x02A7},
{0x0F12, 0x0343},
{0x002A, 0x06B8},
{0x0F12, 0x00D0}, //AWBAshCord[0] Horizon 00C7
{0x0F12, 0x0100}, //AWBAshCord[1] Inca    00d9
{0x0F12, 0x011A}, //AWBAshCord[2] WW      0110
{0x0F12, 0x012A}, //AWBAshCord[3] CW    
{0x0F12, 0x0159}, //AWBAshCord[4] D50     
{0x0F12, 0x0179}, //AWBAshCord[5] D65     
{0x0F12, 0x018c}, //AWBAshCord[6] D75     
		//================================================================================================
		// SET CCM
		//================================================================================================ 
{0x002A, 0x0698},
{0x0F12, 0x33A4},
{0x0F12, 0x7000},
{0x002A, 0x33A4},
//lixiaowen
{0x0F12, 0x01E5},//0x01F9}, 
{0x0F12, 0xFF61},//0xFF7B},
{0x0F12, 0xFF95},//0xFFE6}, 
{0x0F12, 0xFF04},//0xFF3E}, 
{0x0F12, 0x0248},//0x01D1}, 
{0x0F12, 0xFEB5},//0xFFA1}, 
{0x0F12, 0xFF8E},//0x001F},
{0x0F12, 0xFFB8},//0xFFBA},
{0x0F12, 0x0261},//0x025A},
{0x0F12, 0x01C6},//0x0161}, 
{0x0F12, 0x0187},//0x012E},
{0x0F12, 0xFE88},//0xFEC3}, 
{0x0F12, 0x01A9}, 
{0x0F12, 0xFED5},
{0x0F12, 0x013B},
{0x0F12, 0xFF12},
{0x0F12, 0x01E2},
{0x0F12, 0x010B},

{0x0F12, 0x01E5},//0x01F9}, 
{0x0F12, 0xFF61},//0xFF7B},
{0x0F12, 0xFF95},//0xFFE6}, 
{0x0F12, 0xFF04},//0xFF3E}, 
{0x0F12, 0x0248},//0x01D1}, 
{0x0F12, 0xFEB5},//0xFFA1}, 
{0x0F12, 0xFF8E},//0x001F},
{0x0F12, 0xFFB8},//0xFFBA},
{0x0F12, 0x0261},//0x025A},
{0x0F12, 0x01C6},//0x0161}, 
{0x0F12, 0x0187},//0x012E},
{0x0F12, 0xFE88},//0xFEC3}, 
{0x0F12, 0x01A9}, 
{0x0F12, 0xFED5},
{0x0F12, 0x013B},
{0x0F12, 0xFF12},
{0x0F12, 0x01E2},
{0x0F12, 0x010B},
/*
{0x0F12, 0x01F9},  
{0x0F12, 0xFF7B},  
{0x0F12, 0xFFE6},  
{0x0F12, 0xFF3E},  
{0x0F12, 0x01D1},  
{0x0F12, 0xFFA1},  
{0x0F12, 0x001F},  
{0x0F12, 0xFFBA},  
{0x0F12, 0x025A},  
{0x0F12, 0x0161},  
{0x0F12, 0x012E},  
{0x0F12, 0xFEC3},  
{0x0F12, 0x01C1},  
{0x0F12, 0xFF31},  
{0x0F12, 0x015E},  
{0x0F12, 0xFF42},  
{0x0F12, 0x01BD},  
{0x0F12, 0x0100}, 
*/ 
{0x0F12, 0x01F9},
{0x0F12, 0xFF7B},
{0x0F12, 0xFFE6},
{0x0F12, 0xFF3E},
{0x0F12, 0x01D1},
{0x0F12, 0xFFA1},
{0x0F12, 0x001F},
{0x0F12, 0xFFBA},
{0x0F12, 0x025A},
{0x0F12, 0x0161},
{0x0F12, 0x012E},
{0x0F12, 0xFEC3},
{0x0F12, 0x01C1},
{0x0F12, 0xFF31},
{0x0F12, 0x015E},
{0x0F12, 0xFF42},
{0x0F12, 0x01BD},
{0x0F12, 0x0100},
{0x0F12, 0x01F9},
{0x0F12, 0xFF7B},
{0x0F12, 0xFFE6},
{0x0F12, 0xFF3E},
{0x0F12, 0x01D1},
{0x0F12, 0xFFA1},
{0x0F12, 0x001F},
{0x0F12, 0xFFBA},
{0x0F12, 0x025A},
{0x0F12, 0x0161},
{0x0F12, 0x012E},
{0x0F12, 0xFEC3},
{0x0F12, 0x01C1},
{0x0F12, 0xFF31},
{0x0F12, 0x015E},
{0x0F12, 0xFF42},
{0x0F12, 0x01BD},
{0x0F12, 0x0100},
{0x0F12, 0x01F9},
{0x0F12, 0xFF7B},
{0x0F12, 0xFFE6},
{0x0F12, 0xFF3E},
{0x0F12, 0x01D1},
{0x0F12, 0xFFA1},
{0x0F12, 0x001F},
{0x0F12, 0xFFBA},
{0x0F12, 0x025A},
{0x0F12, 0x0161},
{0x0F12, 0x012E},
{0x0F12, 0xFEC3},
{0x0F12, 0x01C1},
{0x0F12, 0xFF31},
{0x0F12, 0x015E},
{0x0F12, 0xFF42},
{0x0F12, 0x01BD},
{0x0F12, 0x0100},
{0x0F12, 0x01F9}, 
{0x0F12, 0xFF7B}, 
{0x0F12, 0xFFE6}, 
{0x0F12, 0xFF3E}, 
{0x0F12, 0x01D1}, 
{0x0F12, 0xFFA1}, 
{0x0F12, 0x001F}, 
{0x0F12, 0xFFBA}, 
{0x0F12, 0x025A}, 
{0x0F12, 0x0161}, 
{0x0F12, 0x012E}, 
{0x0F12, 0xFEC3}, 
{0x0F12, 0x01C1}, 
{0x0F12, 0xFF31}, 
{0x0F12, 0x015E}, 
{0x0F12, 0xFF42}, 
{0x0F12, 0x01BD}, 
{0x0F12, 0x0100}, //#TVAR_wbt_pBaseCcms[107]                                                                  
{0x002A, 0x06A0},                                       
{0x0F12, 0x3380},
{0x0F12, 0x7000},

//#Outdoor CCM                                       
{0x002A, 0x3380},                                    
{0x0F12, 0x0207},//01F9 //#TVAR_wbt_pOutdoorCcm[0]   
{0x0F12, 0xFF72},//FF7B //#TVAR_wbt_pOutdoorCcm[1]   
{0x0F12, 0xFFE1},//FFE6 //#TVAR_wbt_pOutdoorCcm[2]   
{0x0F12, 0xFE66},//FF3E //#TVAR_wbt_pOutdoorCcm[3]   
{0x0F12, 0x018C},//01D1 //#TVAR_wbt_pOutdoorCcm[4]   
{0x0F12, 0xFDFD},//FFA1 //#TVAR_wbt_pOutdoorCcm[5]   
{0x0F12, 0x001F},//001F //#TVAR_wbt_pOutdoorCcm[6]   
{0x0F12, 0xFFBA},//FFBA //#TVAR_wbt_pOutdoorCcm[7]   
{0x0F12, 0x025A},//025A //#TVAR_wbt_pOutdoorCcm[8]   
{0x0F12, 0xFFA3},//0161 //#TVAR_wbt_pOutdoorCcm[9]   
{0x0F12, 0x0048},//012E //#TVAR_wbt_pOutdoorCcm[10]  
{0x0F12, 0xFDE3},//FEC3 //#TVAR_wbt_pOutdoorCcm[11]  
{0x0F12, 0x01C1},//01C1 //#TVAR_wbt_pOutdoorCcm[12]  
{0x0F12, 0xFF31},//FF31 //#TVAR_wbt_pOutdoorCcm[13]  
{0x0F12, 0x015E},//015E //#TVAR_wbt_pOutdoorCcm[14]  
{0x0F12, 0xFF42},//FF42 //#TVAR_wbt_pOutdoorCcm[15]  
{0x0F12, 0x01BD},//01BD //#TVAR_wbt_pOutdoorCcm[16]  
{0x0F12, 0x0100},//0100 //#TVAR_wbt_pOutdoorCcm[17]  

#if 0
//#Outdoor CCM 
{0x002A, 0x3380},
{0x0F12, 0x01F9},
{0x0F12, 0xFF7B},
{0x0F12, 0xFFE6},
{0x0F12, 0xFF3E},
{0x0F12, 0x01D1},
{0x0F12, 0xFFA1},
{0x0F12, 0x001F},
{0x0F12, 0xFFBA},
{0x0F12, 0x025A},
{0x0F12, 0x0161},
{0x0F12, 0x012E},
{0x0F12, 0xFEC3},
{0x0F12, 0x01C1},
{0x0F12, 0xFF31},
{0x0F12, 0x015E},
{0x0F12, 0xFF42},
{0x0F12, 0x01BD},
{0x0F12, 0x0100},
#endif
		
{0x002A, 0x0C48},  //White balance  
{0x0F12, 0x038B},	
{0x0F12, 0x03C0},	
{0x0F12, 0x033D},	
{0x0F12, 0x03C5},	
{0x0F12, 0x0303},	
{0x0F12, 0x03AE},	
{0x0F12, 0x02CF},	
{0x0F12, 0x0387},	
{0x0F12, 0x02A0},	
{0x0F12, 0x0360},	
{0x0F12, 0x027C},	
{0x0F12, 0x0335},	
{0x0F12, 0x025D},	
{0x0F12, 0x030A},	
{0x0F12, 0x0243},	
{0x0F12, 0x02E5},	
{0x0F12, 0x0227},	
{0x0F12, 0x02BD},	
{0x0F12, 0x020E},	
{0x0F12, 0x029E},	
{0x0F12, 0x01F7},	
{0x0F12, 0x027F},	
{0x0F12, 0x01E3},	
{0x0F12, 0x0262},	
{0x0F12, 0x01D1},	
{0x0F12, 0x024D},	
{0x0F12, 0x01BD},	
{0x0F12, 0x0232},	
{0x0F12, 0x01B2},	
{0x0F12, 0x021A},	
{0x0F12, 0x01B3},	
{0x0F12, 0x0201},	
{0x0F12, 0x01BC},	
{0x0F12, 0x01DD},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0005},	
{0x0F12, 0x0000},
{0x002A, 0x0CA0},
{0x0F12, 0x011A},	
{0x0F12, 0x0000},
{0x002A, 0x0CE0},
{0x0F12, 0x0376},	
{0x0F12, 0x03F4},	
{0x0F12, 0x0304},	
{0x0F12, 0x03F4},	
{0x0F12, 0x029A},	
{0x0F12, 0x03E6},	
{0x0F12, 0x024E},	
{0x0F12, 0x039A},	
{0x0F12, 0x020E},	
{0x0F12, 0x034C},	
{0x0F12, 0x01E0},	
{0x0F12, 0x02FF},	
{0x0F12, 0x01AD},	
{0x0F12, 0x02B8},	
{0x0F12, 0x018A},	
{0x0F12, 0x0284},	
{0x0F12, 0x0187},	
{0x0F12, 0x025A},	
{0x0F12, 0x018D},	
{0x0F12, 0x01F6},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0006},	
{0x0F12, 0x0000},
{0x002A, 0x0D18},
{0x0F12, 0x00FA},	
{0x0F12, 0x0000},
{0x002A, 0x0CA4},
{0x0F12, 0x026F},	
{0x0F12, 0x029C},	
{0x0F12, 0x0238},	
{0x0F12, 0x0284},	
{0x0F12, 0x0206},	
{0x0F12, 0x0250},	
{0x0F12, 0x01D6},	
{0x0F12, 0x0226},	
{0x0F12, 0x01BC},	
{0x0F12, 0x01F6},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0006},	
{0x0F12, 0x0000},
{0x002A, 0x0CDC},
{0x0F12, 0x0212},	
{0x0F12, 0x0000},	
{0x002A, 0x0D1C},	
{0x0F12, 0x034D},	
{0x0F12, 0x0000},	
{0x002A, 0x0D20},	
{0x0F12, 0x016C},	
{0x0F12, 0x0000},	
{0x002A, 0x0D24},	
{0x0F12, 0x49D5},	
{0x0F12, 0x0000},	
{0x002A, 0x0D46},	
{0x0F12, 0x0500}, //0470///yellow issue 0570	4c0 500 540
{0x002A, 0x0D5C},	
{0x0F12, 0x0534},	
{0x002A, 0x0D2C},	
{0x0F12, 0x0131},	
{0x0F12, 0x012C},	
{0x002A, 0x0E4A},
{0x0F12, 0x0002},	
{0x002A, 0x0E32},	
{0x0F12, 0x00A6},	
{0x0F12, 0x00C3},	
{0x002A, 0x0E22},
{0x0F12, 0x0EC6},	
{0x0F12, 0x0F3B},	
{0x0F12, 0x0FC7},	
{0x0F12, 0x107E},	
{0x0F12, 0x110E},	
{0x0F12, 0x1198},	
{0x0F12, 0x00B2},	
{0x0F12, 0x00B8},	
{0x002A, 0x0E1C},
{0x0F12, 0x02F4},	
{0x0F12, 0x0347},	
{0x0F12, 0x0390},	
{0x002A, 0x0DD4},
{0x0F12, 0xFFFE},	
{0x0F12, 0xFFFE},	
{0x0F12, 0x0000},	
{0x0F12, 0x012C},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0xFFFD},	
{0x0F12, 0x0078},	
{0x0F12, 0x0078},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0xFFFC},	
{0x0F12, 0xFFFC},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x000C},	
{0x0F12, 0x0006},	
{0x0F12, 0x0000},	
{0x0F12, 0x0320},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x000C},	
{0x0F12, 0xFF9C},	
{0x0F12, 0xFF9C},	
{0x0F12, 0x0000},	
{0x0F12, 0x00C8},	
{0x0F12, 0x0C00},	
{0x0F12, 0x000C},	
{0x0F12, 0x0006},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x0F12, 0x0000},	
{0x002A, 0x0E4A},
{0x0F12, 0x0000},	// #awbb_GridEnable   
{0x002A, 0x0dcc},
{0x0F12, 0x0001},	// #fast awb    

//2012-02-16 add
//AWB init gain
{0x002A, 0x0E44},
{0x0F12, 0x051C},//0x053C //awbb_GainsInit_0_  R_gain	
{0x0F12, 0x0400}, //awbb_GainsInit_1_	G_GAIN
{0x0F12, 0x0678}, //5F8//61C //awbb_GainsInit_2_	B_gain    
   
		//================================================================================================
		// SET gamma
		//================================================================================================
#if 0
//old gamma
{0x002A, 0x3288},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020F},
{0x0F12, 0x0232},
{0x0F12, 0x0273},
{0x0F12, 0x02AF},
{0x0F12, 0x0309},
{0x0F12, 0x0355},
{0x0F12, 0x0394},
{0x0F12, 0x03CE},
{0x0F12, 0x03FF},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020F},
{0x0F12, 0x0232},
{0x0F12, 0x0273},
{0x0F12, 0x02AF},
{0x0F12, 0x0309},
{0x0F12, 0x0355},
{0x0F12, 0x0394},
{0x0F12, 0x03CE},
{0x0F12, 0x03FF},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020F},
{0x0F12, 0x0232},
{0x0F12, 0x0273},
{0x0F12, 0x02AF},
{0x0F12, 0x0309},
{0x0F12, 0x0355},
{0x0F12, 0x0394},
{0x0F12, 0x03CE},
{0x0F12, 0x03FF},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020F},
{0x0F12, 0x0232},
{0x0F12, 0x0273},
{0x0F12, 0x02AF},
{0x0F12, 0x0309},
{0x0F12, 0x0355},
{0x0F12, 0x0394},
{0x0F12, 0x03CE},
{0x0F12, 0x03FF},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020F},
{0x0F12, 0x0232},
{0x0F12, 0x0273},
{0x0F12, 0x02AF},
{0x0F12, 0x0309},
{0x0F12, 0x0355},
{0x0F12, 0x0394},
{0x0F12, 0x03CE},
{0x0F12, 0x03FF},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020F},
{0x0F12, 0x0232},
{0x0F12, 0x0273},
{0x0F12, 0x02AF},
{0x0F12, 0x0309},
{0x0F12, 0x0355},
{0x0F12, 0x0394},
{0x0F12, 0x03CE},
{0x0F12, 0x03FF}, 
{0x002A, 0x06A6}, 
{0x0F12, 0x00CA}, //SARR_AwbCcmCord[0]  
{0x0F12, 0x00EA}, //SARR_AwbCcmCord[1]  
{0x0F12, 0x0110}, //SARR_AwbCcmCord[2]  
{0x0F12, 0x0124}, //SARR_AwbCcmCord[3]  
{0x0F12, 0x0160}, //SARR_AwbCcmCord[4]  
{0x0F12, 0x0180}, //SARR_AwbCcmCord[5] 
#endif

/*
//new gamma
{0x002A, 0x3288},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020D},
{0x0F12, 0x022B},
{0x0F12, 0x025F},
{0x0F12, 0x0285},
{0x0F12, 0x02C5},
{0x0F12, 0x031A},
{0x0F12, 0x038F},
{0x0F12, 0x03BF},                              
{0x0F12, 0x03D9},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020D},
{0x0F12, 0x022B},
{0x0F12, 0x025F},
{0x0F12, 0x0285},
{0x0F12, 0x02C5},
{0x0F12, 0x031A},
{0x0F12, 0x038F},
{0x0F12, 0x03BF},
{0x0F12, 0x03D9},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020D},
{0x0F12, 0x022B},
{0x0F12, 0x025F},
{0x0F12, 0x0285},
{0x0F12, 0x02C5},
{0x0F12, 0x031A},
{0x0F12, 0x038F},
{0x0F12, 0x03BF},
{0x0F12, 0x03D9},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020D},
{0x0F12, 0x022B},
{0x0F12, 0x025F},
{0x0F12, 0x0285},
{0x0F12, 0x02C5},
{0x0F12, 0x031A},
{0x0F12, 0x038F},
{0x0F12, 0x03BF},
{0x0F12, 0x03D9},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020D},
{0x0F12, 0x022B},
{0x0F12, 0x025F},
{0x0F12, 0x0285},
{0x0F12, 0x02C5},
{0x0F12, 0x031A},
{0x0F12, 0x038F},
{0x0F12, 0x03BF},
{0x0F12, 0x03D9},
{0x0F12, 0x0000},
{0x0F12, 0x0004},
{0x0F12, 0x0010},
{0x0F12, 0x002A},
{0x0F12, 0x0062},
{0x0F12, 0x00D5},
{0x0F12, 0x0138},
{0x0F12, 0x0161},
{0x0F12, 0x0186},
{0x0F12, 0x01BC},
{0x0F12, 0x01E8},
{0x0F12, 0x020D},
{0x0F12, 0x022B},
{0x0F12, 0x025F},
{0x0F12, 0x0285},
{0x0F12, 0x02C5},
{0x0F12, 0x031A},
{0x0F12, 0x038F},
{0x0F12, 0x03BF},
{0x0F12, 0x03D9}, 
{0x002A, 0x06A6}, 
{0x0F12, 0x00CA}, //SARR_AwbCcmCord[0]  
{0x0F12, 0x00EA}, //SARR_AwbCcmCord[1]  
{0x0F12, 0x0110}, //SARR_AwbCcmCord[2]  
{0x0F12, 0x0124}, //SARR_AwbCcmCord[3]  
{0x0F12, 0x0160}, //SARR_AwbCcmCord[4]  
{0x0F12, 0x0180}, //SARR_AwbCcmCord[5]  
*/

//from net
{0x002A, 0x3288},	//saRR_usDualGammaLutRGBIndoor  //                                
{0x0F12, 0x0000}, //	saRR_usDualGammaLutRGBIndoor[0] //[0] //                                                 
{0x0F12, 0x0008}, //  saRR_usDualGammaLutRGBIndoor[0] //[1] //                                                       
{0x0F12, 0x0013}, //  saRR_usDualGammaLutRGBIndoor[0] //[2] //                                                       
{0x0F12, 0x002C}, //  saRR_usDualGammaLutRGBIndoor[0] //[3] //                                                       
{0x0F12, 0x0062}, //  saRR_usDualGammaLutRGBIndoor[0] //[4] //                                                       
{0x0F12, 0x00CD}, //  saRR_usDualGammaLutRGBIndoor[0] //[5] //                                                       
{0x0F12, 0x0129}, //  saRR_usDualGammaLutRGBIndoor[0] //[6] //                                                       
{0x0F12, 0x0151}, //  saRR_usDualGammaLutRGBIndoor[0] //[7] //                                                       
{0x0F12, 0x0174}, //  saRR_usDualGammaLutRGBIndoor[0] //[8] //                                                       
{0x0F12, 0x01AA}, //  saRR_usDualGammaLutRGBIndoor[0] //[9] //                                                       
{0x0F12, 0x01D7}, //  saRR_usDualGammaLutRGBIndoor[0] //[10] //                                                      
{0x0F12, 0x01FE}, //  saRR_usDualGammaLutRGBIndoor[0] //[11] //                                                      
{0x0F12, 0x0221}, //  saRR_usDualGammaLutRGBIndoor[0] //[12] //                                                      
{0x0F12, 0x025D}, //  saRR_usDualGammaLutRGBIndoor[0] //[13] //                                                      
{0x0F12, 0x0291}, //  saRR_usDualGammaLutRGBIndoor[0] //[14] //                                                      
{0x0F12, 0x02EB}, //  saRR_usDualGammaLutRGBIndoor[0] //[15] //                                                      
{0x0F12, 0x033A}, //  saRR_usDualGammaLutRGBIndoor[0] //[16] //                                                      
{0x0F12, 0x0380}, //  saRR_usDualGammaLutRGBIndoor[0] //[17] //                                                      
{0x0F12, 0x03C2}, //  saRR_usDualGammaLutRGBIndoor[0] //[18] //                                                      
{0x0F12, 0x03FF}, //  saRR_usDualGammaLutRGBIndoor[0] //[19] //                                                      
{0x0F12, 0x0000}, //  saRR_usDualGammaLutRGBIndoor[1] //[0] //                                                       
{0x0F12, 0x0008}, //  saRR_usDualGammaLutRGBIndoor[1] //[1] //                                                       
{0x0F12, 0x0013}, //  saRR_usDualGammaLutRGBIndoor[1] //[2] //                                                       
{0x0F12, 0x002C}, //  saRR_usDualGammaLutRGBIndoor[1] //[3] //                                                       
{0x0F12, 0x0062}, //  saRR_usDualGammaLutRGBIndoor[1] //[4] //                                                       
{0x0F12, 0x00CD}, //  saRR_usDualGammaLutRGBIndoor[1] //[5] //                                                       
{0x0F12, 0x0129}, //  saRR_usDualGammaLutRGBIndoor[1] //[6] //                                                       
{0x0F12, 0x0151}, //  saRR_usDualGammaLutRGBIndoor[1] //[7] //                                                       
{0x0F12, 0x0174}, //  saRR_usDualGammaLutRGBIndoor[1] //[8] //                                                       
{0x0F12, 0x01AA}, //  saRR_usDualGammaLutRGBIndoor[1] //[9] //                                                       
{0x0F12, 0x01D7}, //  saRR_usDualGammaLutRGBIndoor[1] //[10] //                                                      
{0x0F12, 0x01FE}, //  saRR_usDualGammaLutRGBIndoor[1] //[11] //                                                      
{0x0F12, 0x0221}, //  saRR_usDualGammaLutRGBIndoor[1] //[12] //                                                      
{0x0F12, 0x025D}, //  saRR_usDualGammaLutRGBIndoor[1] //[13] //                                                      
{0x0F12, 0x0291}, //  saRR_usDualGammaLutRGBIndoor[1] //[14] //                                                      
{0x0F12, 0x02EB}, //  saRR_usDualGammaLutRGBIndoor[1] //[15] //                                                      
{0x0F12, 0x033A}, //  saRR_usDualGammaLutRGBIndoor[1] //[16] //                                                      
{0x0F12, 0x0380}, //  saRR_usDualGammaLutRGBIndoor[1] //[17] //                                                      
{0x0F12, 0x03C2}, //  saRR_usDualGammaLutRGBIndoor[1] //[18] //                                                      
{0x0F12, 0x03FF}, //  saRR_usDualGammaLutRGBIndoor[1] //[19] //                                                      
{0x0F12, 0x0000}, //  saRR_usDualGammaLutRGBIndoor[2] //[0] //                                                       
{0x0F12, 0x0008}, //  saRR_usDualGammaLutRGBIndoor[2] //[1] //                                                       
{0x0F12, 0x0013}, //  saRR_usDualGammaLutRGBIndoor[2] //[2] //                                                       
{0x0F12, 0x002C}, //  saRR_usDualGammaLutRGBIndoor[2] //[3] //                                                       
{0x0F12, 0x0062}, //  saRR_usDualGammaLutRGBIndoor[2] //[4] //                                                       
{0x0F12, 0x00CD}, //  saRR_usDualGammaLutRGBIndoor[2] //[5] //                                                       
{0x0F12, 0x0129}, //  saRR_usDualGammaLutRGBIndoor[2] //[6] //                                                       
{0x0F12, 0x0151}, //  saRR_usDualGammaLutRGBIndoor[2] //[7] //                                                       
{0x0F12, 0x0174}, //  saRR_usDualGammaLutRGBIndoor[2] //[8] //                                                       
{0x0F12, 0x01AA}, //  saRR_usDualGammaLutRGBIndoor[2] //[9] //                                                       
{0x0F12, 0x01D7}, //  saRR_usDualGammaLutRGBIndoor[2] //[10] //                                                      
{0x0F12, 0x01FE}, //  saRR_usDualGammaLutRGBIndoor[2] //[11] //                                                      
{0x0F12, 0x0221}, //  saRR_usDualGammaLutRGBIndoor[2] //[12] //                                                      
{0x0F12, 0x025D}, //  saRR_usDualGammaLutRGBIndoor[2] //[13] //                                                      
{0x0F12, 0x0291}, //  saRR_usDualGammaLutRGBIndoor[2] //[14] //                                                      
{0x0F12, 0x02EB}, //  saRR_usDualGammaLutRGBIndoor[2] //[15] //                                                      
{0x0F12, 0x033A}, //  saRR_usDualGammaLutRGBIndoor[2] //[16] //                                                      
{0x0F12, 0x0380}, //  saRR_usDualGammaLutRGBIndoor[2] //[17] //                                                      
{0x0F12, 0x03C2}, //  saRR_usDualGammaLutRGBIndoor[2] //[18] //                                                      
{0x0F12, 0x03FF}, //  saRR_usDualGammaLutRGBIndoor[2] //[19] //                                                      
                                               
                                               
{0x0F12, 0x0000},	//  saRR_usDualGammaLutRGBOutdoor[0] //[0] //
{0x0F12, 0x0008},	//  saRR_usDualGammaLutRGBOutdoor[0] //[1] //                                                
{0x0F12, 0x0013},	//  saRR_usDualGammaLutRGBOutdoor[0] //[2] //                                                
{0x0F12, 0x002C},	//  saRR_usDualGammaLutRGBOutdoor[0] //[3] //                                                
{0x0F12, 0x0062},	//  saRR_usDualGammaLutRGBOutdoor[0] //[4] //                                                
{0x0F12, 0x00CD},	//  saRR_usDualGammaLutRGBOutdoor[0] //[5] //                                                
{0x0F12, 0x0129},	//  saRR_usDualGammaLutRGBOutdoor[0] //[6] //                                                
{0x0F12, 0x0151},	//  saRR_usDualGammaLutRGBOutdoor[0] //[7] //                                                
{0x0F12, 0x0174},	//  saRR_usDualGammaLutRGBOutdoor[0] //[8] //                                                
{0x0F12, 0x01AA},	//  saRR_usDualGammaLutRGBOutdoor[0] //[9] //                                                
{0x0F12, 0x01D7},	//  saRR_usDualGammaLutRGBOutdoor[0] //[10] //                                               
{0x0F12, 0x01FE},	//  saRR_usDualGammaLutRGBOutdoor[0] //[11] //                                               
{0x0F12, 0x0221},	//  saRR_usDualGammaLutRGBOutdoor[0] //[12] //                                               
{0x0F12, 0x025D},	//  saRR_usDualGammaLutRGBOutdoor[0] //[13] //                                               
{0x0F12, 0x0291},	//  saRR_usDualGammaLutRGBOutdoor[0] //[14] //                                               
{0x0F12, 0x02EB},	//  saRR_usDualGammaLutRGBOutdoor[0] //[15] //                                               
{0x0F12, 0x033A},	//  saRR_usDualGammaLutRGBOutdoor[0] //[16] //                                               
{0x0F12, 0x0380},	//  saRR_usDualGammaLutRGBOutdoor[0] //[17] //                                               
{0x0F12, 0x03C2},	//  saRR_usDualGammaLutRGBOutdoor[0] //[18] //                                               
{0x0F12, 0x03FF},	//  saRR_usDualGammaLutRGBOutdoor[0] //[19] //                                               
{0x0F12, 0x0000},	//  saRR_usDualGammaLutRGBOutdoor[1] //[0] //                                                
{0x0F12, 0x0008},	//  saRR_usDualGammaLutRGBOutdoor[1] //[1] //                                                
{0x0F12, 0x0013},	//  saRR_usDualGammaLutRGBOutdoor[1] //[2] //                                                
{0x0F12, 0x002C},	//  saRR_usDualGammaLutRGBOutdoor[1] //[3] //                                                
{0x0F12, 0x0062},	//  saRR_usDualGammaLutRGBOutdoor[1] //[4] //                                                
{0x0F12, 0x00CD},	//  saRR_usDualGammaLutRGBOutdoor[1] //[5] //                                                
{0x0F12, 0x0129},	//  saRR_usDualGammaLutRGBOutdoor[1] //[6] //                                                
{0x0F12, 0x0151},	//  saRR_usDualGammaLutRGBOutdoor[1] //[7] //                                                
{0x0F12, 0x0174},	//  saRR_usDualGammaLutRGBOutdoor[1] //[8] //                                                
{0x0F12, 0x01AA},	//  saRR_usDualGammaLutRGBOutdoor[1] //[9] //                                                
{0x0F12, 0x01D7},	//  saRR_usDualGammaLutRGBOutdoor[1] //[10] //                                               
{0x0F12, 0x01FE},	//  saRR_usDualGammaLutRGBOutdoor[1] //[11] //                                               
{0x0F12, 0x0221},	//  saRR_usDualGammaLutRGBOutdoor[1] //[12] //                                               
{0x0F12, 0x025D},	//  saRR_usDualGammaLutRGBOutdoor[1] //[13] //                                               
{0x0F12, 0x0291},	//  saRR_usDualGammaLutRGBOutdoor[1] //[14] //                                               
{0x0F12, 0x02EB},	//  saRR_usDualGammaLutRGBOutdoor[1] //[15] //                                               
{0x0F12, 0x033A},	//  saRR_usDualGammaLutRGBOutdoor[1] //[16] //                                               
{0x0F12, 0x0380},	//  saRR_usDualGammaLutRGBOutdoor[1] //[17] //                                               
{0x0F12, 0x03C2},	//  saRR_usDualGammaLutRGBOutdoor[1] //[18] //                                               
{0x0F12, 0x03FF},	//  saRR_usDualGammaLutRGBOutdoor[1] //[19] //                                               
{0x0F12, 0x0000},	//  saRR_usDualGammaLutRGBOutdoor[2] //[0] //                                                
{0x0F12, 0x0008},	//  saRR_usDualGammaLutRGBOutdoor[2] //[1] //                                                
{0x0F12, 0x0013},	//  saRR_usDualGammaLutRGBOutdoor[2] //[2] //                                                
{0x0F12, 0x002C},	//  saRR_usDualGammaLutRGBOutdoor[2] //[3] //                                                
{0x0F12, 0x0062},	//  saRR_usDualGammaLutRGBOutdoor[2] //[4] //                                                
{0x0F12, 0x00CD},	//  saRR_usDualGammaLutRGBOutdoor[2] //[5] //                                                
{0x0F12, 0x0129},	//  saRR_usDualGammaLutRGBOutdoor[2] //[6] //                                                
{0x0F12, 0x0151},	//  saRR_usDualGammaLutRGBOutdoor[2] //[7] //                                                
{0x0F12, 0x0174},	//  saRR_usDualGammaLutRGBOutdoor[2] //[8] //                                                
{0x0F12, 0x01AA},	//  saRR_usDualGammaLutRGBOutdoor[2] //[9] //                                                
{0x0F12, 0x01D7},	//  saRR_usDualGammaLutRGBOutdoor[2] //[10] //                                               
{0x0F12, 0x01FE},	//  saRR_usDualGammaLutRGBOutdoor[2] //[11] //                                               
{0x0F12, 0x0221},	//  saRR_usDualGammaLutRGBOutdoor[2] //[12] //                                               
{0x0F12, 0x025D},	//  saRR_usDualGammaLutRGBOutdoor[2] //[13] //                                               
{0x0F12, 0x0291},	//  saRR_usDualGammaLutRGBOutdoor[2] //[14] //                                               
{0x0F12, 0x02EB},	//  saRR_usDualGammaLutRGBOutdoor[2] //[15] //                                               
{0x0F12, 0x033A},	//  saRR_usDualGammaLutRGBOutdoor[2] //[16] //                                               
{0x0F12, 0x0380},	//  saRR_usDualGammaLutRGBOutdoor[2] //[17] //                                               
{0x0F12, 0x03C2},	//  saRR_usDualGammaLutRGBOutdoor[2] //[18] //                                               
{0x0F12, 0x03FF},	//  saRR_usDualGammaLutRGBOutdoor[2] //[19] //                                               
                                                                                            
{0x002A, 0x06A6},                           
{0x0F12, 0x00C0},	//saRR_AwbCcmCord[0] //                      
{0x0F12, 0x00E0},	//saRR_AwbCcmCord[1] //                      
{0x0F12, 0x0110},	//saRR_AwbCcmCord[2] //                      
{0x0F12, 0x0139},	//saRR_AwbCcmCord[3] //                      
{0x0F12, 0x0166},	//saRR_AwbCcmCord[4] //                      
{0x0F12, 0x019F},	//saRR_AwbCcmCord[5] //     

//================================================================================================
// SET AFIT
//================================================================================================                               
{0x002A, 0x0764},
{0x0F12, 0x0041},
{0x0F12, 0x0063},
{0x0F12, 0x00BB},
{0x0F12, 0x0193},
{0x0F12, 0x02BC},
{0x002A, 0x0770},
{0x0F12, 0x07C4},
{0x0F12, 0x7000},
{0x002A, 0x07C4},
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[0]  Modifyed by Alex for Newton Rings 20100419
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[1]  
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[2]  
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[3]  
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[4]  
{0x0F12, 0x00C4},		//00C4 00C4  //TVAR_afit_pBaseVals[5]  
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[6]  
{0x0F12, 0x009C},		//009C 009C  //TVAR_afit_pBaseVals[7]  
{0x0F12, 0x017C},		//017C 017C  //TVAR_afit_pBaseVals[8]  
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[9]  
{0x0F12, 0x000C},		//000C 000C  //TVAR_afit_pBaseVals[10] 
{0x0F12, 0x0010},		//0010 0010  //TVAR_afit_pBaseVals[11] 
{0x0F12, 0x0104},		//0104 0104  //TVAR_afit_pBaseVals[12] 
{0x0F12, 0x03E8},		//03E8 03E8  //TVAR_afit_pBaseVals[13] 
{0x0F12, 0x0023},		//0023 0023  //TVAR_afit_pBaseVals[14] 
{0x0F12, 0x012C},		//012C 012C  //TVAR_afit_pBaseVals[15] 
{0x0F12, 0x0070},		//0070 0070  //TVAR_afit_pBaseVals[16] 
{0x0F12, 0x0010},		//0010 0010  //TVAR_afit_pBaseVals[17] 
{0x0F12, 0x0010},		//0010 0010  //TVAR_afit_pBaseVals[18] 
{0x0F12, 0x01AA},		//01AA 01AA  //TVAR_afit_pBaseVals[19] 
{0x0F12, 0x0064},		//0064 0064  //TVAR_afit_pBaseVals[20] 
{0x0F12, 0x0064},		//0064 0064  //TVAR_afit_pBaseVals[21] 
{0x0F12, 0x000A},		//000A 000A  //TVAR_afit_pBaseVals[22] 
{0x0F12, 0x000A},		//000A 000A  //TVAR_afit_pBaseVals[23] 
{0x0F12, 0x003C},		//003C 003C  //TVAR_afit_pBaseVals[24] 
{0x0F12, 0x0024},		//0024 0024  //TVAR_afit_pBaseVals[25] 
{0x0F12, 0x002A},		//002A 002A  //TVAR_afit_pBaseVals[26] 
{0x0F12, 0x0024},		//0024 0024  //TVAR_afit_pBaseVals[27] 
{0x0F12, 0x002A},		//002A 002A  //TVAR_afit_pBaseVals[28] 
{0x0F12, 0x0024},		//0024 0024  //TVAR_afit_pBaseVals[29] 
{0x0F12, 0x0A24},		//0A24 0A24  //TVAR_afit_pBaseVals[30] 
{0x0F12, 0x1701},		//1701 1701  //TVAR_afit_pBaseVals[31] 
{0x0F12, 0x0229},		//0229 0229  //TVAR_afit_pBaseVals[32] 
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[33] 
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[34] 
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[35] 
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[36] 
{0x0F12, 0x00FF},		//00FF 00FF  //TVAR_afit_pBaseVals[37] 
{0x0F12, 0x043B},		//043B 043B  //TVAR_afit_pBaseVals[38] 
{0x0F12, 0x1414},		//1414 1414  //TVAR_afit_pBaseVals[39] 
{0x0F12, 0x0301},		//0301 0301  //TVAR_afit_pBaseVals[40] 
{0x0F12, 0xFF07},		//FF07 FF07  //TVAR_afit_pBaseVals[41] 
{0x0F12, 0x051E},		//051E 051E  //TVAR_afit_pBaseVals[42] 
{0x0F12, 0x0A1E},		//0A1E 0A1E  //TVAR_afit_pBaseVals[43] 
{0x0F12, 0x0F0F},		//0F0F 0F0F  //TVAR_afit_pBaseVals[44] 
{0x0F12, 0x0A05},		//0A05 0A05  //TVAR_afit_pBaseVals[45] 
{0x0F12, 0x0A3C},		//0A3C 0A3C  //TVAR_afit_pBaseVals[46] 
{0x0F12, 0x0A28},		//0A28 0A28  //TVAR_afit_pBaseVals[47] 
{0x0F12, 0x0002},		//0002 0002  //TVAR_afit_pBaseVals[48] 
{0x0F12, 0x00FF},		//00FF 00FF  //TVAR_afit_pBaseVals[49] 
{0x0F12, 0x1002},		//1002 1002  //TVAR_afit_pBaseVals[50] 
{0x0F12, 0x001D},		//001D 001D  //TVAR_afit_pBaseVals[51] 
{0x0F12, 0x0900},		//0900 0900  //TVAR_afit_pBaseVals[52] 
{0x0F12, 0x0600},		//0600 0600  //TVAR_afit_pBaseVals[53] 
{0x0F12, 0x0504},		//0504 0504  //TVAR_afit_pBaseVals[54] 
{0x0F12, 0x0305},		//0305 0305  //TVAR_afit_pBaseVals[55] 
{0x0F12, 0x5003},		//5003 5003  //TVAR_afit_pBaseVals[56] 
{0x0F12, 0x006E},		//006E 006E  //TVAR_afit_pBaseVals[57] 
{0x0F12, 0x0078},		//0078 0078  //TVAR_afit_pBaseVals[58] 
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[59] 
{0x0F12, 0x1414},		//1414 1414  //TVAR_afit_pBaseVals[60] 
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[61] 
{0x0F12, 0x5002},		//5002 5002  //TVAR_afit_pBaseVals[62] 
{0x0F12, 0x7850},		//7850 7850  //TVAR_afit_pBaseVals[63] 
{0x0F12, 0x2878},		//2878 2878  //TVAR_afit_pBaseVals[64] 
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[65] 
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[66] 
{0x0F12, 0x1E0C},		//1E0C 1E0C  //TVAR_afit_pBaseVals[67] 
{0x0F12, 0x070A},		//070A 070A  //TVAR_afit_pBaseVals[68] 
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[69] 
{0x0F12, 0x5004},		//5004 5004  //TVAR_afit_pBaseVals[70] 
{0x0F12, 0x0F40},		//0F40 0F40  //TVAR_afit_pBaseVals[71] 
{0x0F12, 0x400F},		//400F 400F  //TVAR_afit_pBaseVals[72] 
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[73] 
{0x0F12, 0x1E03},		//1E03 1E03  //TVAR_afit_pBaseVals[74] 
{0x0F12, 0x011E},		//011E 011E  //TVAR_afit_pBaseVals[75] 
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[76] 
{0x0F12, 0x5050},		//5050 5050  //TVAR_afit_pBaseVals[77] 
{0x0F12, 0x7878},		//7878 7878  //TVAR_afit_pBaseVals[78] 
{0x0F12, 0x0028},		//0028 0028  //TVAR_afit_pBaseVals[79] 
{0x0F12, 0x030A},		//030A 030A  //TVAR_afit_pBaseVals[80] 
{0x0F12, 0x0714},		//0714 0714  //TVAR_afit_pBaseVals[81] 
{0x0F12, 0x0A1E},		//0A1E 0A1E  //TVAR_afit_pBaseVals[82] 
{0x0F12, 0xFF07},		//FF07 FF07  //TVAR_afit_pBaseVals[83] 
{0x0F12, 0x0432},		//0432 0432  //TVAR_afit_pBaseVals[84] 
{0x0F12, 0x4050},		//4050 4050  //TVAR_afit_pBaseVals[85] 
{0x0F12, 0x0F0F},		//0F0F 0F0F  //TVAR_afit_pBaseVals[86] 
{0x0F12, 0x0440},		//0440 0440  //TVAR_afit_pBaseVals[87] 
{0x0F12, 0x0302},		//0302 0302  //TVAR_afit_pBaseVals[88] 
{0x0F12, 0x1E1E},		//1E1E 1E1E  //TVAR_afit_pBaseVals[89] 
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[90] 
{0x0F12, 0x5001},		//5001 5001  //TVAR_afit_pBaseVals[91] 
{0x0F12, 0x7850},		//7850 7850  //TVAR_afit_pBaseVals[92] 
{0x0F12, 0x2878},		//2878 2878  //TVAR_afit_pBaseVals[93] 
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[94] 
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[95] 
{0x0F12, 0x1E07},		//1E07 1E07  //TVAR_afit_pBaseVals[96] 
{0x0F12, 0x070A},		//070A 070A  //TVAR_afit_pBaseVals[97] 
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[98] 
{0x0F12, 0x5004},		//5004 5004  //TVAR_afit_pBaseVals[99] 
{0x0F12, 0x0F40},		//0F40 0F40  //TVAR_afit_pBaseVals[100]
{0x0F12, 0x400F},		//400F 400F  //TVAR_afit_pBaseVals[101]
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[102]
{0x0F12, 0x0003},		//0003 0003  //TVAR_afit_pBaseVals[103]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[104]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[105]
{0x0F12, 0x0008},//0x0000 zhao 0008	//0000 0000  //TVAR_afit_pBaseVals[106]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[107]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[108]
{0x0F12, 0x00C4},		//00C4 00C4  //TVAR_afit_pBaseVals[109]
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[110]
{0x0F12, 0x009C},		//009C 009C  //TVAR_afit_pBaseVals[111]
{0x0F12, 0x017C},		//017C 017C  //TVAR_afit_pBaseVals[112]
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[113]
{0x0F12, 0x000C},		//000C 000C  //TVAR_afit_pBaseVals[114]
{0x0F12, 0x0010},		//0010 0010  //TVAR_afit_pBaseVals[115]
{0x0F12, 0x0104},		//0104 0104  //TVAR_afit_pBaseVals[116]
{0x0F12, 0x03E8},		//03E8 03E8  //TVAR_afit_pBaseVals[117]
{0x0F12, 0x0023},		//0023 0023  //TVAR_afit_pBaseVals[118]
{0x0F12, 0x012C},		//012C 012C  //TVAR_afit_pBaseVals[119]
{0x0F12, 0x0070},		//0070 0070  //TVAR_afit_pBaseVals[120]
{0x0F12, 0x0008},		//0008 0008  //TVAR_afit_pBaseVals[121]
{0x0F12, 0x0008},		//0008 0008  //TVAR_afit_pBaseVals[122]
{0x0F12, 0x01AA},		//01AA 01AA  //TVAR_afit_pBaseVals[123]
{0x0F12, 0x003C},		//003C 003C  //TVAR_afit_pBaseVals[124]
{0x0F12, 0x003C},		//003C 003C  //TVAR_afit_pBaseVals[125]
{0x0F12, 0x0005},		//0005 0005  //TVAR_afit_pBaseVals[126]
{0x0F12, 0x0005},		//0005 0005  //TVAR_afit_pBaseVals[127]
{0x0F12, 0x0050},		//0050 0050  //TVAR_afit_pBaseVals[128]
{0x0F12, 0x0024},		//0024 0024  //TVAR_afit_pBaseVals[129]
{0x0F12, 0x002A},		//002A 002A  //TVAR_afit_pBaseVals[130]
{0x0F12, 0x0024},		//0024 0024  //TVAR_afit_pBaseVals[131]
{0x0F12, 0x002A},		//002A 002A  //TVAR_afit_pBaseVals[132]
{0x0F12, 0x0024},		//0024 0024  //TVAR_afit_pBaseVals[133]
{0x0F12, 0x0A24},		//0A24 0A24  //TVAR_afit_pBaseVals[134]
{0x0F12, 0x1701},		//1701 1701  //TVAR_afit_pBaseVals[135]
{0x0F12, 0x0229},		//0229 0229  //TVAR_afit_pBaseVals[136]
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[137]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[138]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[139]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[140]
{0x0F12, 0x00FF},		//00FF 00FF  //TVAR_afit_pBaseVals[141]
{0x0F12, 0x043B},		//043B 043B  //TVAR_afit_pBaseVals[142]
{0x0F12, 0x1414},		//1414 1414  //TVAR_afit_pBaseVals[143]
{0x0F12, 0x0301},		//0301 0301  //TVAR_afit_pBaseVals[144]
{0x0F12, 0xFF07},		//FF07 FF07  //TVAR_afit_pBaseVals[145]
{0x0F12, 0x051E},		//051E 051E  //TVAR_afit_pBaseVals[146]
{0x0F12, 0x0A1E},		//0A1E 0A1E  //TVAR_afit_pBaseVals[147]
{0x0F12, 0x0F0F},		//0F0F 0F0F  //TVAR_afit_pBaseVals[148]
{0x0F12, 0x0A03},		//0A03 0A03  //TVAR_afit_pBaseVals[149]
{0x0F12, 0x0A3C},		//0A3C 0A3C  //TVAR_afit_pBaseVals[150]
{0x0F12, 0x0A28},		//0A28 0A28  //TVAR_afit_pBaseVals[151]
{0x0F12, 0x0002},		//0002 0002  //TVAR_afit_pBaseVals[152]
{0x0F12, 0x00FF},		//00FF 00FF  //TVAR_afit_pBaseVals[153]
{0x0F12, 0x1102},		//1102 1102  //TVAR_afit_pBaseVals[154]
{0x0F12, 0x001D},		//001D 001D  //TVAR_afit_pBaseVals[155]
{0x0F12, 0x0900},		//0900 0900  //TVAR_afit_pBaseVals[156]
{0x0F12, 0x0600},		//0600 0600  //TVAR_afit_pBaseVals[157]
{0x0F12, 0x0504},		//0504 0504  //TVAR_afit_pBaseVals[158]
{0x0F12, 0x0305},		//0305 0305  //TVAR_afit_pBaseVals[159]
{0x0F12, 0x6403},		//6403 6403  //TVAR_afit_pBaseVals[160]
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[161]
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[162]
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[163]
{0x0F12, 0x1919},		//1919 1919  //TVAR_afit_pBaseVals[164]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[165]
{0x0F12, 0x3C02},		//3C02 3C02  //TVAR_afit_pBaseVals[166]
{0x0F12, 0x553C},		//553C 553C  //TVAR_afit_pBaseVals[167]
{0x0F12, 0x2855},		//2855 2855  //TVAR_afit_pBaseVals[168]
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[169]
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[170]
{0x0F12, 0x1E0C},		//1E0C 1E0C  //TVAR_afit_pBaseVals[171]
{0x0F12, 0x070A},		//070A 070A  //TVAR_afit_pBaseVals[172]
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[173]
{0x0F12, 0x5004},		//5004 5004  //TVAR_afit_pBaseVals[174]
{0x0F12, 0x0F40},		//0F40 0F40  //TVAR_afit_pBaseVals[175]
{0x0F12, 0x400F},		//400F 400F  //TVAR_afit_pBaseVals[176]
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[177]
{0x0F12, 0x1E03},		//1E03 1E03  //TVAR_afit_pBaseVals[178]
{0x0F12, 0x011E},		//011E 011E  //TVAR_afit_pBaseVals[179]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[180]
{0x0F12, 0x3232},		//3232 3232  //TVAR_afit_pBaseVals[181]
{0x0F12, 0x3C3C},		//3C3C 3C3C  //TVAR_afit_pBaseVals[182]
{0x0F12, 0x0028},		//0028 0028  //TVAR_afit_pBaseVals[183]
{0x0F12, 0x030A},		//030A 030A  //TVAR_afit_pBaseVals[184]
{0x0F12, 0x0714},		//0714 0714  //TVAR_afit_pBaseVals[185]
{0x0F12, 0x0A1E},		//0A1E 0A1E  //TVAR_afit_pBaseVals[186]
{0x0F12, 0xFF07},		//FF07 FF07  //TVAR_afit_pBaseVals[187]
{0x0F12, 0x0432},		//0432 0432  //TVAR_afit_pBaseVals[188]
{0x0F12, 0x4050},		//4050 4050  //TVAR_afit_pBaseVals[189]
{0x0F12, 0x0F0F},		//0F0F 0F0F  //TVAR_afit_pBaseVals[190]
{0x0F12, 0x0440},		//0440 0440  //TVAR_afit_pBaseVals[191]
{0x0F12, 0x0302},		//0302 0302  //TVAR_afit_pBaseVals[192]
{0x0F12, 0x1E1E},		//1E1E 1E1E  //TVAR_afit_pBaseVals[193]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[194]
{0x0F12, 0x3201},		//3201 3201  //TVAR_afit_pBaseVals[195]
{0x0F12, 0x3C32},		//3C32 3C32  //TVAR_afit_pBaseVals[196]
{0x0F12, 0x283C},		//283C 283C  //TVAR_afit_pBaseVals[197]
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[198]
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[199]
{0x0F12, 0x1E07},		//1E07 1E07  //TVAR_afit_pBaseVals[200]
{0x0F12, 0x070A},		//070A 070A  //TVAR_afit_pBaseVals[201]
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[202]
{0x0F12, 0x5004},		//5004 5004  //TVAR_afit_pBaseVals[203]
{0x0F12, 0x0F40},		//0F40 0F40  //TVAR_afit_pBaseVals[204]
{0x0F12, 0x400F},		//400F 400F  //TVAR_afit_pBaseVals[205]
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[206]
{0x0F12, 0x0003},		//0003 0003  //TVAR_afit_pBaseVals[207]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[208]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[209]
{0x0F12, 0x000a},//0x0000 zhao 000A		//0000 0000  //TVAR_afit_pBaseVals[210]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[211]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[212]
{0x0F12, 0x00C4},		//00C4 00C4  //TVAR_afit_pBaseVals[213]
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[214]
{0x0F12, 0x009C},		//009C 009C  //TVAR_afit_pBaseVals[215]
{0x0F12, 0x017C},		//017C 017C  //TVAR_afit_pBaseVals[216]
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[217]
{0x0F12, 0x000C},		//000C 000C  //TVAR_afit_pBaseVals[218]
{0x0F12, 0x0010},		//0010 0010  //TVAR_afit_pBaseVals[219]
{0x0F12, 0x0104},		//0104 0104  //TVAR_afit_pBaseVals[220]
{0x0F12, 0x03E8},		//03E8 03E8  //TVAR_afit_pBaseVals[221]
{0x0F12, 0x0023},		//0023 0023  //TVAR_afit_pBaseVals[222]
{0x0F12, 0x012C},		//012C 012C  //TVAR_afit_pBaseVals[223]
{0x0F12, 0x0070},		//0070 0070  //TVAR_afit_pBaseVals[224]
{0x0F12, 0x0004},		//0004 0004  //TVAR_afit_pBaseVals[225]
{0x0F12, 0x0004},		//0004 0004  //TVAR_afit_pBaseVals[226]
{0x0F12, 0x01AA},		//01AA 01AA  //TVAR_afit_pBaseVals[227]
{0x0F12, 0x001E},		//001E 001E  //TVAR_afit_pBaseVals[228]
{0x0F12, 0x001E},		//001E 001E  //TVAR_afit_pBaseVals[229]
{0x0F12, 0x0005},		//0005 0005  //TVAR_afit_pBaseVals[230]
{0x0F12, 0x0005},		//0005 0005  //TVAR_afit_pBaseVals[231]
{0x0F12, 0x0064},		//0064 0064  //TVAR_afit_pBaseVals[232]
{0x0F12, 0x0024},		//0024 0024  //TVAR_afit_pBaseVals[233]
{0x0F12, 0x002A},		//002A 002A  //TVAR_afit_pBaseVals[234]
{0x0F12, 0x0024},		//0024 0024  //TVAR_afit_pBaseVals[235]
{0x0F12, 0x002A},		//002A 002A  //TVAR_afit_pBaseVals[236]
{0x0F12, 0x0024},		//0024 0024  //TVAR_afit_pBaseVals[237]
{0x0F12, 0x0A24},		//0A24 0A24  //TVAR_afit_pBaseVals[238]
{0x0F12, 0x1701},		//1701 1701  //TVAR_afit_pBaseVals[239]
{0x0F12, 0x0229},		//0229 0229  //TVAR_afit_pBaseVals[240]
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[241]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[242]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[243]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[244]
{0x0F12, 0x00FF},		//00FF 00FF  //TVAR_afit_pBaseVals[245]
{0x0F12, 0x043B},		//043B 043B  //TVAR_afit_pBaseVals[246]
{0x0F12, 0x1414},		//1414 1414  //TVAR_afit_pBaseVals[247]
{0x0F12, 0x0301},		//0301 0301  //TVAR_afit_pBaseVals[248]
{0x0F12, 0xFF07},		//FF07 FF07  //TVAR_afit_pBaseVals[249]
{0x0F12, 0x051E},		//051E 051E  //TVAR_afit_pBaseVals[250]
{0x0F12, 0x0A1E},		//0A1E 0A1E  //TVAR_afit_pBaseVals[251]
{0x0F12, 0x0F0F},		//0F0F 0F0F  //TVAR_afit_pBaseVals[252]
{0x0F12, 0x0A04},		//0A04 0A04  //TVAR_afit_pBaseVals[253]
{0x0F12, 0x0A3C},		//0A3C 0A3C  //TVAR_afit_pBaseVals[254]
{0x0F12, 0x0528},		//0528 0528  //TVAR_afit_pBaseVals[255]
{0x0F12, 0x0002},		//0002 0002  //TVAR_afit_pBaseVals[256]
{0x0F12, 0x00FF},		//00FF 00FF  //TVAR_afit_pBaseVals[257]
{0x0F12, 0x1002},		//1002 1002  //TVAR_afit_pBaseVals[258]
{0x0F12, 0x001D},		//001D 001D  //TVAR_afit_pBaseVals[259]
{0x0F12, 0x0900},		//0900 0900  //TVAR_afit_pBaseVals[260]
{0x0F12, 0x0600},		//0600 0600  //TVAR_afit_pBaseVals[261]
{0x0F12, 0x0504},		//0504 0504  //TVAR_afit_pBaseVals[262]
{0x0F12, 0x0305},		//0305 0305  //TVAR_afit_pBaseVals[263]
{0x0F12, 0x6403},		//6403 7803  //TVAR_afit_pBaseVals[264]//7803
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[265]
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[266]
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[267]
{0x0F12, 0x2323},		//2323 2323  //TVAR_afit_pBaseVals[268]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[269]
{0x0F12, 0x2A02},		//2A02 2A02  //TVAR_afit_pBaseVals[270]
{0x0F12, 0x462A},		//462A 462A  //TVAR_afit_pBaseVals[271]
{0x0F12, 0x2846},		//2846 2846  //TVAR_afit_pBaseVals[272]
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[273]
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[274]
{0x0F12, 0x1E0C},		//1E0C 1E0C  //TVAR_afit_pBaseVals[275]
{0x0F12, 0x070A},		//070A 070A  //TVAR_afit_pBaseVals[276]
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[277]
{0x0F12, 0x8804},		//6404 5A04  //TVAR_afit_pBaseVals[278]
{0x0F12, 0x0F78},		//0F4A 0F40  //TVAR_afit_pBaseVals[279]
{0x0F12, 0x400F},		//400F 400F  //TVAR_afit_pBaseVals[280]
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[281]
{0x0F12, 0x2303},		//2303 2303  //TVAR_afit_pBaseVals[282]
{0x0F12, 0x0123},		//0123 0123  //TVAR_afit_pBaseVals[283]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[284]
{0x0F12, 0x0000},		//0000 262A  //TVAR_afit_pBaseVals[285]
{0x0F12, 0x0000},		//0000 2C2C  //TVAR_afit_pBaseVals[286]
{0x0F12, 0x0028},		//0028 0028  //TVAR_afit_pBaseVals[287]
{0x0F12, 0x030A},		//030A 030A  //TVAR_afit_pBaseVals[288]
{0x0F12, 0x0714},		//0714 0714  //TVAR_afit_pBaseVals[289]
{0x0F12, 0x0A1E},		//0A1E 0A1E  //TVAR_afit_pBaseVals[290]
{0x0F12, 0xFF07},		//FF07 FF07  //TVAR_afit_pBaseVals[291]
{0x0F12, 0x0432},		//0432 0432  //TVAR_afit_pBaseVals[292]
{0x0F12, 0x4C5F},		//4050 4050  //TVAR_afit_pBaseVals[293]
{0x0F12, 0x0F0F},		//0F0F 0F0F  //TVAR_afit_pBaseVals[294]
{0x0F12, 0x0440},		//0440 0440  //TVAR_afit_pBaseVals[295]
{0x0F12, 0x0302},		//0302 0302  //TVAR_afit_pBaseVals[296]
{0x0F12, 0x2323},		//2323 2323  //TVAR_afit_pBaseVals[297]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[298]
{0x0F12, 0x2A01},		//2A01 2A01  //TVAR_afit_pBaseVals[299]
{0x0F12, 0x2C26},		//2C26 2C26  //TVAR_afit_pBaseVals[300]
{0x0F12, 0x282C},		//282C 282C  //TVAR_afit_pBaseVals[301]
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[302]
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[303]
{0x0F12, 0x1E07},		//1E07 1E07  //TVAR_afit_pBaseVals[304]
{0x0F12, 0x070A},		//070A 070A  //TVAR_afit_pBaseVals[305]
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[306]
{0x0F12, 0x5004},		//5004 5004  //TVAR_afit_pBaseVals[307]
{0x0F12, 0x0F40},		//0F40 0F40  //TVAR_afit_pBaseVals[308]
{0x0F12, 0x400F},		//400F 400F  //TVAR_afit_pBaseVals[309]
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[310]
{0x0F12, 0x0003},		//0003 0003  //TVAR_afit_pBaseVals[311]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[312]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[313]
{0x0F12, 0x000a},//0x0000 zhao	000a	//0000 0000  //TVAR_afit_pBaseVals[314]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[315]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[316]
{0x0F12, 0x00C4},		//00C4 00C4  //TVAR_afit_pBaseVals[317]
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[318]
{0x0F12, 0x009C},		//009C 009C  //TVAR_afit_pBaseVals[319]
{0x0F12, 0x017C},		//017C 017C  //TVAR_afit_pBaseVals[320]
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[321]
{0x0F12, 0x000C},		//000C 000C  //TVAR_afit_pBaseVals[322]
{0x0F12, 0x0010},		//0010 0010  //TVAR_afit_pBaseVals[323]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[324]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[325]
{0x0F12, 0x0023},		//0023 0023  //TVAR_afit_pBaseVals[326]
{0x0F12, 0x012C},		//012C 012C  //TVAR_afit_pBaseVals[327]
{0x0F12, 0x0070},		//0070 0070  //TVAR_afit_pBaseVals[328]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[329]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[330]
{0x0F12, 0x01AA},		//01AA 01AA  //TVAR_afit_pBaseVals[331]
{0x0F12, 0x001E},		//001E 001E  //TVAR_afit_pBaseVals[332]
{0x0F12, 0x001E},		//001E 001E  //TVAR_afit_pBaseVals[333]
{0x0F12, 0x000A},		//000A 000A  //TVAR_afit_pBaseVals[334]
{0x0F12, 0x000A},		//000A 000A  //TVAR_afit_pBaseVals[335]
{0x0F12, 0x00E6},		//00E6 00E6  //TVAR_afit_pBaseVals[336]
{0x0F12, 0x0032},		//0032 0032  //TVAR_afit_pBaseVals[337]
{0x0F12, 0x0064},		//0064 0032  //TVAR_afit_pBaseVals[338]
{0x0F12, 0x0064},		//0064 0028  //TVAR_afit_pBaseVals[339]
{0x0F12, 0x0064},		//0064 0032  //TVAR_afit_pBaseVals[340]
{0x0F12, 0x0064},		//0064 0028  //TVAR_afit_pBaseVals[341]
{0x0F12, 0x0A24},		//0A24 0A24  //TVAR_afit_pBaseVals[342]
{0x0F12, 0x1701},		//1701 1701  //TVAR_afit_pBaseVals[343]
{0x0F12, 0x0229},		//0229 0229  //TVAR_afit_pBaseVals[344]
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[345]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[346]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[347]
{0x0F12, 0x0504},		//0504 0504  //TVAR_afit_pBaseVals[348]
{0x0F12, 0x00FF},		//00FF 00FF  //TVAR_afit_pBaseVals[349]
{0x0F12, 0x043B},		//043B 043B  //TVAR_afit_pBaseVals[350]
{0x0F12, 0x1414},		//1414 1414  //TVAR_afit_pBaseVals[351]
{0x0F12, 0x0301},		//0301 0301  //TVAR_afit_pBaseVals[352]
{0x0F12, 0xFF07},		//FF07 FF07  //TVAR_afit_pBaseVals[353]
{0x0F12, 0x051E},		//051E 051E  //TVAR_afit_pBaseVals[354]
{0x0F12, 0x0A1E},		//0A1E 0A1E  //TVAR_afit_pBaseVals[355]
{0x0F12, 0x0F0F},		//0F0F 0F0F  //TVAR_afit_pBaseVals[356]
{0x0F12, 0x0A04},		//0A04 0A04  //TVAR_afit_pBaseVals[357]
{0x0F12, 0x0A3C},		//0A3C 0A3C  //TVAR_afit_pBaseVals[358]
{0x0F12, 0x0532},		//0532 0532  //TVAR_afit_pBaseVals[359]
{0x0F12, 0x0002},		//0002 0002  //TVAR_afit_pBaseVals[360]
{0x0F12, 0x00FF},		//00FF 00FF  //TVAR_afit_pBaseVals[361]
{0x0F12, 0x1002},		//1002 1002  //TVAR_afit_pBaseVals[362]
{0x0F12, 0x001D},		//001D 001D  //TVAR_afit_pBaseVals[363]
{0x0F12, 0x0900},		//0900 0900  //TVAR_afit_pBaseVals[364]
{0x0F12, 0x0600},		//0600 0600  //TVAR_afit_pBaseVals[365]
{0x0F12, 0x0504},		//0504 0504  //TVAR_afit_pBaseVals[366]
{0x0F12, 0x0305},		//0305 0305  //TVAR_afit_pBaseVals[367]
{0x0F12, 0x6402},		//6402 7802  //TVAR_afit_pBaseVals[368]//7802
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[369]
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[370]
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[371]
{0x0F12, 0x2328},		//2328 2328  //TVAR_afit_pBaseVals[372]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[373]
{0x0F12, 0x2A02},		//2A02 2A02  //TVAR_afit_pBaseVals[374]
{0x0F12, 0x2628},		//2628 2628  //TVAR_afit_pBaseVals[375]
{0x0F12, 0x2826},		//2826 2826  //TVAR_afit_pBaseVals[376]
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[377]
{0x0F12, 0x1903},		//1903 1903  //TVAR_afit_pBaseVals[378]
{0x0F12, 0x1E0F},		//1E0F 1E0F  //TVAR_afit_pBaseVals[379]
{0x0F12, 0x070A},		//070A 070A  //TVAR_afit_pBaseVals[380]
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[381]
{0x0F12, 0xC804},		//8204 7804  //TVAR_afit_pBaseVals[382]
{0x0F12, 0x0FAA},		//0F4A 0F40  //TVAR_afit_pBaseVals[383]
{0x0F12, 0x400F},		//400F 400F  //TVAR_afit_pBaseVals[384]
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[385]
{0x0F12, 0x2803},		//2803 2803  //TVAR_afit_pBaseVals[386]
{0x0F12, 0x0123},		//0123 0123  //TVAR_afit_pBaseVals[387]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[388]
{0x0F12, 0x0000},		//0000 2024  //TVAR_afit_pBaseVals[389]
{0x0F12, 0x0000},		//0000 1C1C  //TVAR_afit_pBaseVals[390]
{0x0F12, 0x0028},		//0028 0028  //TVAR_afit_pBaseVals[391]
{0x0F12, 0x030A},		//030A 030A  //TVAR_afit_pBaseVals[392]
{0x0F12, 0x1414},		//1414 0A0A  //TVAR_afit_pBaseVals[393]
{0x0F12, 0x0A00},		//0A00 0A2D  //TVAR_afit_pBaseVals[394]
{0x0F12, 0xFF07},		//FF07 FF07  //TVAR_afit_pBaseVals[395]
{0x0F12, 0x0432},		//0432 0432  //TVAR_afit_pBaseVals[396]
{0x0F12, 0x5F6E},		//4050 4050  //TVAR_afit_pBaseVals[397]
{0x0F12, 0x0000},		//0000 0F0F  //TVAR_afit_pBaseVals[398]
{0x0F12, 0x0440},		//0440 0440  //TVAR_afit_pBaseVals[399]
{0x0F12, 0x0302},		//0302 0302  //TVAR_afit_pBaseVals[400]
{0x0F12, 0x2328},		//2328 2328  //TVAR_afit_pBaseVals[401]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[402]
{0x0F12, 0x0001},		//0001 3C01  //TVAR_afit_pBaseVals[403]
{0x0F12, 0x0000},		//0000 1C3C  //TVAR_afit_pBaseVals[404]
{0x0F12, 0x2800},		//2800 281C  //TVAR_afit_pBaseVals[405]
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[406]
{0x0F12, 0x0A03},		//0A03 0A03  //TVAR_afit_pBaseVals[407]
{0x0F12, 0x2D0A},		//2D0A 2D0A  //TVAR_afit_pBaseVals[408]
{0x0F12, 0x070A},		//070A 070A  //TVAR_afit_pBaseVals[409]
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[410]
{0x0F12, 0x5004},		//5004 5004  //TVAR_afit_pBaseVals[411]
{0x0F12, 0x0040},		//0040 0F40  //TVAR_afit_pBaseVals[412]
{0x0F12, 0x4000},		//4000 400F  //TVAR_afit_pBaseVals[413]
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[414]
{0x0F12, 0x0003},		//0003 0003  //TVAR_afit_pBaseVals[415]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[416]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[417]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[418]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[419]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[420]
{0x0F12, 0x00C4},		//00C4 00C4  //TVAR_afit_pBaseVals[421]
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[422]
{0x0F12, 0x009C},		//009C 009C  //TVAR_afit_pBaseVals[423]
{0x0F12, 0x017C},		//017C 017C  //TVAR_afit_pBaseVals[424]
{0x0F12, 0x03FF},		//03FF 03FF  //TVAR_afit_pBaseVals[425]
{0x0F12, 0x000C},		//000C 000C  //TVAR_afit_pBaseVals[426]
{0x0F12, 0x0010},		//0010 0010  //TVAR_afit_pBaseVals[427]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[428]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[429]
{0x0F12, 0x003C},		//003C 003C  //TVAR_afit_pBaseVals[430]
{0x0F12, 0x006F},		//006F 006F  //TVAR_afit_pBaseVals[431]
{0x0F12, 0x0070},		//0070 0070  //TVAR_afit_pBaseVals[432]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[433]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[434]
{0x0F12, 0x01AA},		//01AA 01AA  //TVAR_afit_pBaseVals[435]
{0x0F12, 0x0014},		//0014 0014  //TVAR_afit_pBaseVals[436]
{0x0F12, 0x0014},		//0014 0014  //TVAR_afit_pBaseVals[437]
{0x0F12, 0x000A},		//000A 000A  //TVAR_afit_pBaseVals[438]
{0x0F12, 0x000A},		//000A 000A  //TVAR_afit_pBaseVals[439]
{0x0F12, 0x0122},		//0122 0122  //TVAR_afit_pBaseVals[440]
{0x0F12, 0x003C},		//003C 003C  //TVAR_afit_pBaseVals[441]
{0x0F12, 0x0064},		//0064 0032  //TVAR_afit_pBaseVals[442]
{0x0F12, 0x0064},		//0064 0023  //TVAR_afit_pBaseVals[443]
{0x0F12, 0x0064},		//0064 0023  //TVAR_afit_pBaseVals[444]
{0x0F12, 0x0064},		//0064 0032  //TVAR_afit_pBaseVals[445]
{0x0F12, 0x0A24},		//0A24 0A24  //TVAR_afit_pBaseVals[446]
{0x0F12, 0x1701},		//1701 1701  //TVAR_afit_pBaseVals[447]
{0x0F12, 0x0229},		//0229 0229  //TVAR_afit_pBaseVals[448]
{0x0F12, 0x1403},		//1403 1403  //TVAR_afit_pBaseVals[449]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[450]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[451]
{0x0F12, 0x0505},		//0505 0505  //TVAR_afit_pBaseVals[452]
{0x0F12, 0x00FF},		//00FF 00FF  //TVAR_afit_pBaseVals[453]
{0x0F12, 0x043B},		//043B 043B  //TVAR_afit_pBaseVals[454]
{0x0F12, 0x1414},		//1414 1414  //TVAR_afit_pBaseVals[455]
{0x0F12, 0x0301},		//0301 0301  //TVAR_afit_pBaseVals[456]
{0x0F12, 0xFF07},		//FF07 FF07  //TVAR_afit_pBaseVals[457]
{0x0F12, 0x051E},		//051E 051E  //TVAR_afit_pBaseVals[458]
{0x0F12, 0x0A1E},		//0A1E 0A1E  //TVAR_afit_pBaseVals[459]
{0x0F12, 0x0000},		//0000 0000  //TVAR_afit_pBaseVals[460]
{0x0F12, 0x0A04},		//0A04 0A04  //TVAR_afit_pBaseVals[461]
{0x0F12, 0x0A3C},		//0A3C 0A3C  //TVAR_afit_pBaseVals[462]
{0x0F12, 0x0532},		//0532 0532  //TVAR_afit_pBaseVals[463]
{0x0F12, 0x0002},		//0002 0002  //TVAR_afit_pBaseVals[464]
{0x0F12, 0x0096},		//0096 0096  //TVAR_afit_pBaseVals[465]
{0x0F12, 0x1002},		//1002 1002  //TVAR_afit_pBaseVals[466]
{0x0F12, 0x001E},		//001E 001E  //TVAR_afit_pBaseVals[467]
{0x0F12, 0x0900},		//0900 0900  //TVAR_afit_pBaseVals[468]
{0x0F12, 0x0600},		//0600 0600  //TVAR_afit_pBaseVals[469]
{0x0F12, 0x0504},		//0504 0504  //TVAR_afit_pBaseVals[470]
{0x0F12, 0x0305},		//0305 0305  //TVAR_afit_pBaseVals[471]
{0x0F12, 0x6402},		//6402 7D02  //TVAR_afit_pBaseVals[472]//7D02
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[473]
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[474]
{0x0F12, 0x0080},		//0080 0080  //TVAR_afit_pBaseVals[475]
{0x0F12, 0x5050},		//5050 5050  //TVAR_afit_pBaseVals[476]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[477]
{0x0F12, 0x1C02},		//1C02 1C02  //TVAR_afit_pBaseVals[478]
{0x0F12, 0x191C},		//191C 191C  //TVAR_afit_pBaseVals[479]
{0x0F12, 0x2819},		//2819 2819  //TVAR_afit_pBaseVals[480]
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[481]
{0x0F12, 0x1E03},		//1E03 1E03  //TVAR_afit_pBaseVals[482]
{0x0F12, 0x1E0F},		//1E0F 1E0F  //TVAR_afit_pBaseVals[483]
{0x0F12, 0x0508},		//0508 0508  //TVAR_afit_pBaseVals[484]
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[485]
{0x0F12, 0xD204},		//8C04 8204  //TVAR_afit_pBaseVals[486]
{0x0F12, 0x14BA},		//1454 1448  //TVAR_afit_pBaseVals[487]
{0x0F12, 0x4015},		//4015 4015  //TVAR_afit_pBaseVals[488]
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[489]
{0x0F12, 0x5003},		//5003 5003  //TVAR_afit_pBaseVals[490]
{0x0F12, 0x0150},		//0150 0150  //TVAR_afit_pBaseVals[491]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[492]
{0x0F12, 0x0000},		//0000 1E1E  //TVAR_afit_pBaseVals[493]
{0x0F12, 0x0000},		//0000 1212  //TVAR_afit_pBaseVals[494]
{0x0F12, 0x0028},		//0028 0028  //TVAR_afit_pBaseVals[495]
{0x0F12, 0x030A},		//030A 030A  //TVAR_afit_pBaseVals[496]
{0x0F12, 0x1E1E},		//1E1E 0A10  //TVAR_afit_pBaseVals[497]
{0x0F12, 0x0800},		//0800 0819  //TVAR_afit_pBaseVals[498]
{0x0F12, 0xFF05},		//FF05 FF05  //TVAR_afit_pBaseVals[499]
{0x0F12, 0x0432},		//0432 0432  //TVAR_afit_pBaseVals[500]
{0x0F12, 0x6978},		//4052 4052  //TVAR_afit_pBaseVals[501]
{0x0F12, 0x0000},		//0000 1514  //TVAR_afit_pBaseVals[502]
{0x0F12, 0x0440},		//0440 0440  //TVAR_afit_pBaseVals[503]
{0x0F12, 0x0302},		//0302 0302  //TVAR_afit_pBaseVals[504]
{0x0F12, 0x5050},		//5050 5050  //TVAR_afit_pBaseVals[505]
{0x0F12, 0x0101},		//0101 0101  //TVAR_afit_pBaseVals[506]
{0x0F12, 0x0001},		//0001 1E01  //TVAR_afit_pBaseVals[507]
{0x0F12, 0x0000},		//0000 121E  //TVAR_afit_pBaseVals[508]
{0x0F12, 0x2800},		//2800 2812  //TVAR_afit_pBaseVals[509]
{0x0F12, 0x0A00},		//0A00 0A00  //TVAR_afit_pBaseVals[510]
{0x0F12, 0x1003},		//1003 1003  //TVAR_afit_pBaseVals[511]
{0x0F12, 0x190A},		//190A 190A  //TVAR_afit_pBaseVals[512]
{0x0F12, 0x0508},		//0508 0508  //TVAR_afit_pBaseVals[513]
{0x0F12, 0x32FF},		//32FF 32FF  //TVAR_afit_pBaseVals[514]
{0x0F12, 0x5204},		//5204 5204  //TVAR_afit_pBaseVals[515]
{0x0F12, 0x0040},		//0040 1440  //TVAR_afit_pBaseVals[516]
{0x0F12, 0x4000},		//4000 4015  //TVAR_afit_pBaseVals[517]
{0x0F12, 0x0204},		//0204 0204  //TVAR_afit_pBaseVals[518]
{0x0F12, 0x0003},		//0003 0003  //TVAR_afit_pBaseVals[519] 
{0x0F12, 0x7F7A},  //// #afit_pConstBaseVals[0]
{0x0F12, 0x7F9D},  //// #afit_pConstBaseVals[1]
{0x0F12, 0xBEFC},  //// #afit_pConstBaseVals[2]
{0x0F12, 0xF7BC},  //// #afit_pConstBaseVals[3]
{0x0F12, 0x7E06},  //// #afit_pConstBaseVals[4]
{0x0F12, 0x0053},  //// #afit_pConstBaseVals[5]

{0x002A, 0x0664},  //
{0x0F12, 0x013E},  ////seti_uContrastCenter
{0x002A, 0x04D6},  //
{0x0F12, 0x0001},  //// #REG_TC_DBG_ReInitCmd
{0x0028, 0xD000},  //
{0x002A, 0x1102},  //
{0x0F12, 0x00C0},  //// Use T&P index 22 and 23
{0x002A, 0x113C},  //
{0x0F12, 0x267C},  //// Trap 22 address 0x71aa
{0x0F12, 0x2680},  //// Trap 23 address 0x71b4
{0x002A, 0x1142},  //
{0x0F12, 0x00C0},  //// Trap Up Set (trap Addr are > 0x10000) 
{0x002A, 0x117C},  //
{0x0F12, 0x2CE8},  //// Patch 22 address (TrapAndPatchOpCodes array index 22) 
{0x0F12, 0x2CeC},  //// Patch 23 address (TrapAndPatchOpCodes array index 23) 
{0x0028, 0x7000},  // start add MSW
{0x002A, 0x2CE8},  // start add LSW
{0x0F12, 0x0007},  // Modify LSB to control AWBB_YThreshLow
{0x0F12, 0x00e2},  // 
{0x0F12, 0x0005},  // Modify LSB to control AWBB_YThreshLowBrLow
{0x0F12, 0x00e2},  // 
{0x002A, 0x337A}, 
{0x0F12, 0x0006},  // #Tune_TP_atop_dbus_reg // 6 is the default HW value

{0x002A, 0x0D44},  //
{0x0F12, 0x0020},  ////seti_uContrastCenter
//===========================================================
//ISP-FE Setting END
//===========================================================
  
//================================================================================================
// SET PLL
//================================================================================================
// How to set
// 1. MCLK
//hex(CLK you want) * 1000)
// 2. System CLK
//hex((CLK you want) * 1000 / 4)
// 3. PCLK
//hex((CLK you want) * 1000 / 4)
//================================================================================================
// Set input CLK // 24MHz
{0x002A, 0x01CC},
{0x0F12, 0x5DC0}, 
{0x0F12, 0x0000}, 
{0x002A, 0x01EE},
{0x0F12, 0x0002},
{0x002A, 0x01F6},

{0x0F12, 0x2ee0},//2710	// #REG_TC_IPRM_OpClk4KHz_0        40M
{0x0F12, 0x2ea0},	// #REG_TC_IPRM_MinOutRate4KHz_0   48M
{0x0F12, 0x2ee0},	// #REG_TC_IPRM_MaxOutRate4KHz_0
 
{0x0F12, 0x1F40},	// #REG_TC_IPRM_OpClk4KHz_1        32M
{0x0F12, 0x2ea0},	// #REG_TC_IPRM_MinOutRate4KHz_0   48M
{0x0F12, 0x2ee0},	// #REG_TC_IPRM_MaxOutRate4KHz_0

/*
{0x0F12, 0x1F40},    //REG_TC_IPRM_OpClk4KHz_0                   	      
{0x0F12, 0x32A8},	//3A88    //REG_TC_IPRM_MinOutRate4KHz_0            
{0x0F12, 0x32E8},	//3AA8    //REG_TC_IPRM_MaxOutRate4KHz_0  52M
*/
/*
{0x0F12, 0x2ee0},    //REG_TC_IPRM_OpClk4KHz_0                   	      
{0x0F12, 0x32A8},	//3A88    //REG_TC_IPRM_MinOutRate4KHz_0            
{0x0F12, 0x32E8},	//3AA8    //REG_TC_IPRM_MaxOutRate4KHz_0  52M
*/
/* 
{0x0F12, 0x2ee0}, // #REG_TC_IPRM_OpClk4KHz_1        mclk=40M                
{0x0F12, 0x3A88}, // #REG_TC_IPRM_MinOutRate4KHz_1                            
{0x0F12, 0x3AA8}, // #REG_TC_IPRM_MaxOutRate4KHz_1   pclk=60M 
*/ 

// Update PLL
{0x002A, 0x0208},
{0x0F12, 0x0001}, // #REG_TC_IPRM_InitParamsUpdated
{0x0000, 0x0032}, // Wait50mSec
//============================================================
// Frame rate setting 
//============================================================
// How to set
// 1. Exposure value
// dec2hex((1 / (frame rate you want(ms))) * 100d * 4d)
// 2. Analog Digital gain
// dec2hex((Analog gain you want) * 256d)
//============================================================
/*
// Set preview exposure time
{0x002A, 0x0530},
{0x0F12, 0x4E20}, //50ms // 5DC0 // #lt_uMaxExp1 			60ms
{0x0F12, 0x0000},                            
{0x0F12, 0x5DC0}, //60ms //6D60 // #lt_uMaxExp2 			70ms                            
{0x0F12, 0x0000},                            
{0x002A, 0x167C},                            
{0x0F12, 0x7D00}, // 80ms 9C40 // #evt1_lt_uMaxExp3 	100ms                             
{0x0F12, 0x0000},                             
{0x0F12, 0x3880}, // #evt1_lt_uMaxExp4 	120ms                            
{0x0F12, 0x0001},                             
// Set capture exposure time                            
{0x002A, 0x0538},                           
{0x0F12, 0x5DC0},// #lt_uCapMaxExp1			60ms                             
{0x0F12, 0x0000},                                                         
{0x0F12, 0x6D60},// #lt_uCapMaxExp2      70ms                             
{0x0F12, 0x0000},
{0x002A, 0x1684},
{0x0F12, 0x9C40},// #evt1_lt_uCapMaxExp3 100ms                            
{0x0F12, 0x0000},                                                         
{0x0F12, 0x3880},// #evt1_lt_uCapMaxExp4 120ms                            
{0x0F12, 0x0001},
// Set gain                            
{0x002A, 0x0540},                            
{0x0F12, 0x0150}, // #lt_uMaxAnGain1                            
{0x0F12, 0x0280}, // #lt_uMaxAnGain2                            
{0x002A, 0x168C},                            
{0x0F12, 0x02A0}, // #evt1_lt_uMaxAnGain3                            
{0x0F12, 0x0700}, // #evt1_lt_uMaxAnGain4                             
{0x002A, 0x0544},
{0x0F12, 0x0100}, // #lt_uMaxDigGain
{0x0F12, 0x1000}, // #lt_uMaxTotGain
{0x002A, 0x1694},
{0x0F12, 0x0001}, // #evt1_senHal_bExpandForbid
{0x002A, 0x051A},
{0x0F12, 0x0111}, // #lt_uLimitHigh 
{0x0F12, 0x00F0}, // #lt_uLimitLow   
*/

  // Set preview exposure time           
    {0x002A, 0x0530},                              
    {0x0F12, 0x5DC0}, //#lt_uMaxExp1 60ms          
    {0x0F12, 0x0000},                              
    {0x0F12, 0x6D60}, //#lt_uMaxExp2 70ms          
    {0x0F12, 0x0000},                              
    {0x002A, 0x167C},                              
    {0x0F12, 0x9C40}, //#evt1_lt_uMaxExp3 100ms    
    {0x0F12, 0x0000},                              
    {0x0F12, 0x3880},//BB80 //#evt1_lt_uMaxExp4 120ms    
    {0x0F12, 0x0001},//0000                              
    // Set capture exposure time           
    {0x002A, 0x0538},                              
    {0x0F12, 0x5DC0}, //#lt_uCapMaxExp160ms        
    {0x0F12, 0x0000},                              
    {0x0F12, 0x6D60}, //#lt_uCapMaxExp270ms        
    {0x0F12, 0x0000},                              
    {0x002A, 0x1684},                              
    {0x0F12, 0x9C40}, //#evt1_lt_uCapMaxExp3 100ms 
    {0x0F12, 0x0000},                              
    {0x0F12, 0xBB80}, //#evt1_lt_uCapMaxExp4 120ms 
    {0x0F12, 0x0000},                              
    // Set gain                         
    {0x002A, 0x0540},                           
    {0x0F12, 0x0150}, //#lt_uMaxAnGain1         
    {0x0F12, 0x0280}, //#lt_uMaxAnGain2         
    {0x002A, 0x168C},                           
    {0x0F12, 0x02A0}, //#evt1_lt_uMaxAnGain3    
    {0x0F12, 0x0800}, //#evt1_lt_uMaxAnGain4    
                                      
    {0x002A, 0x0544},                           
    {0x0F12, 0x0100}, //#lt_uMaxDigGain         
    {0x0F12, 0x0A00}, //A00 //#lt_uMaxTotGain         
    
    {0x002A, 0x1694},                                 
    {0x0F12, 0x0001}, //#evt1_senHal_bExpandForbid    //expand forbidde zone
   /* 
    {0x002A, 0x051A},
    {0x0F12, 0x0111}, // #lt_uLimitHigh 
    {0x0F12, 0x00F0}, // #lt_uLimitLow  
   */                                                                
    //Auto Flicker 50Hz Start                                         
    {0x0028, 0x7000},                                                          
    {0x002A, 0x0C18}, //#AFC_Default60Hz                                       
    {0x0F12, 0x0001}, // 1: Auto Flicker 60Hz start 0: Auto Flicker 50Hz start 
    {0x002A, 0x04D2}, //#REG_TC_DBG_AutoAlgEnBits                              
    {0x0F12, 0x067F},                                                          
    //================================================================================================             
    // SET PREVIEW CONFIGURATION_0                                                                                
    // # Foramt : YUV422                                                                                      
    // # Size: SVGA                                                                                         
    // # FPS : 7.5-15fps for normal mode                                                                             
    //================================================================================================             
    {0x002A, 0x026C},                                                              
    {0x0F12, 0x0288}, //#REG_0TC_PCFG_usWidth//1024   //280                                         
    {0x0F12, 0x01E6}, //#REG_0TC_PCFG_usHeight //768    026E  //1E0                                           
    {0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270                                                 
    {0x0F12, 0x2ee0}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272                                                
    {0x0F12, 0x2ea0}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274                                               
    {0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276         
    {0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027                        
    {0x0F12, 0x0052}, //0052 //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A  
    {0x0F12, 0x4000}, //#REG_0TC_PCFG_OIFMask                                                                     
    {0x0F12, 0x0400}, //#REG_0TC_PCFG_usJpegPacketSize                                                                 
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets                                                              
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd                                                
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType                                                              
    {0x0F12, 0x0001}, //#REG_0TC_PCFG_FrRateQualityType    
    {0x0F12, 0x03E8}, //0580  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //10fps    
    {0x0F12, 0x01F4}, //01c6  //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //15fps   
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput                                                                  
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation                                                                   
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur                                                              
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp                                                              
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex                                                       
    {0x0F12, 0x0003}, //#REG_0TC_PCFG_uPrevMirror   //3      //0
    {0x0F12, 0x0003}, //#REG_0TC_PCFG_uCaptureMirror //3     //0
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation  
    //================================================================================================
    // APPLY PREVIEW CONFIGURATION & RUN PREVIEW                                                      
    //================================================================================================
    {0x002A, 0x023C},                                                                                         
    {0x0F12, 0x0000}, // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0                        
    {0x002A, 0x0240},                                                                                         
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevOpenAfterChange                                                       
    {0x002A, 0x0230},                                                                                         
    {0x0F12, 0x0001}, // #REG_TC_GP_NewConfigSync // Update preview configuration                             
    {0x002A, 0x023E},                                                                                         
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged                                                         
    {0x002A, 0x0220},                                                                                         
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreview // Start preview                                            
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreviewChanged                                                      
    //================================================================================================
    // SET CAPTURE CONFIGURATION_0                                                                    
    // # Foramt :yuv                                                                                 
    // # Size: QXGA                                                                                   
    // # FPS : 5fps                                                                             
    //================================================================================================
    {0x002A, 0x035C},                                                                                         
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG                                                        
    {0x0F12, 0x0800}, //#REG_0TC_CCFG_usWidth                                                                 
    {0x0F12, 0x0600}, //#REG_0TC_CCFG_usHeight                                                                
    {0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV  9:JPEG                                                     
    {0x0F12, 0x2ee0}, //#REG_0TC_CCFG_usMaxOut4KHzRate                                                        
    {0x0F12, 0x2ea0}, //#REG_0TC_CCFG_usMinOut4KHzRate                                                        
    {0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88                                                          
    {0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88                                                               
    {0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask                                                                 
    {0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison                                                        
    {0x0F12, 0x03C0}, //#REG_0TC_CCFG_usJpegPacketSize                                                        
    {0x0F12, 0x047E}, //08fc //#REG_0TC_CCFG_usJpegTotalPackets                                                      
    {0x0F12, 0x0001}, //#REG_0TC_CCFG_uClockInd                                                               
    {0x0F12, 0x0002}, //#REG_0TC_CCFG_usFrTimeType                                                            
    {0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType                                                       
    {0x0F12, 0x07D0}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //5fps    0x07D0                                      
    {0x0F12, 0x07D0}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //5fps                                           
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput                                                            
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation                                                             
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur                                                              
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp                                                              
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex                                                       
    //================================================================================================ 
    // SET CAPTURE CONFIGURATION_1                                                                    
    // # Foramt :yuv                                                                                  
    // # Size: QXGA                                                                                   
    // # FPS : 5~7.5fps                                                                             
    //================================================================================================
    {0x002A, 0x038C},                                                                                         
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG                                                        
    {0x0F12, 0x0800}, //#REG_0TC_CCFG_usWidth                                                                 
    {0x0F12, 0x0600}, //#REG_0TC_CCFG_usHeight                                                                
    {0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV  9:JPEG                                                     
    {0x0F12, 0x2ee0}, //#REG_0TC_CCFG_usMaxOut4KHzRate                                                        
    {0x0F12, 0x2ea0}, //#REG_0TC_CCFG_usMinOut4KHzRate                                                        
    {0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88                                                          
    {0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88                                                               
    {0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask                                                                 
    {0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison                                                        
    {0x0F12, 0x03C0}, //#REG_0TC_CCFG_usJpegPacketSize                                                        
    {0x0F12, 0x047E}, //08fc //#REG_0TC_CCFG_usJpegTotalPackets                                                      
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd                                                               
    {0x0F12, 0x0002}, //#REG_0TC_CCFG_usFrTimeType                                                            
    {0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType                                                       
    {0x0F12, 0x07D0}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //5fps   //  7D0                                     
    {0x0F12, 0x07D0}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //5fps     // 7D0                                    
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput                                                            
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation                                                             
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur                                                              
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp                                                              
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex                                                    
    //================================================================================================ 
    {0x0028, 0xD000},                            
    {0x002A, 0x1000},                            
    {0x0F12, 0x0001},                            
    {0x0000, 0x0020},// Wait10mSec   //B2                 
    {0x0028, 0x7000},     

/*
//<CAMTUNING_METERING_FLAT  >
{0xfcfc, 0xd000},
{0x0028, 0x7000},
 
{0x002a, 0x0F7E}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101}, 
{0x0f12, 0x0101},                      
 */   
/*
// <CAMTUNING_METERING_CENTER>
{0xfcfc, 0xd000},
{0x0028, 0x7000},             
                                                          
{0x002A, 0x0F7E},        
{0x0F12, 0x0000},  //0000
{0x0F12, 0x0000},  //0000
{0x0F12, 0x0000},  //0000
{0x0F12, 0x0000},  //0000
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0201},  //0201
{0x0F12, 0x0102},  //0102
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0202},  //0202
{0x0F12, 0x0202},  //0202
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0202},  //0202
{0x0F12, 0x0202},  //0202
{0x0F12, 0x0101},  //0101
{0x0F12, 0x0201},  //0101
{0x0F12, 0x0202},  //0202
{0x0F12, 0x0202},  //0202
{0x0F12, 0x0102},  //0101
{0x0F12, 0x0201},  //0201
{0x0F12, 0x0202},  //0202
{0x0F12, 0x0202},  //0202
{0x0F12, 0x0102},  //0102
{0x0F12, 0x0101},  //0201
{0x0F12, 0x0101},  //0202
{0x0F12, 0x0101},  //0202
{0x0F12, 0x0101},  //0102     
{0x0000, 0x000F},// Wait10mSec                    
{0x0028, 0x7000},   
*/                                                          
{0xFFFF, 0xFF}
};


const static struct mt9t113_reg s5k5cag_for_preview[] = {
	{0x0028, 0x7000},
	{0x002A, 0x023C},
	{0x0F12, 0x0001}, // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
	{0x002A, 0x0240},
	{0x0F12, 0x0001}, // #REG_TC_GP_PrevOpenAfterChange
	{0x002A, 0x0230},
	{0x0F12, 0x0001}, // #REG_TC_GP_NewConfigSync // Update preview configuration
	{0x002A, 0x023E},
	{0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged
	{0x002A, 0x0220},
	{0x0F12, 0x0001}, // #REG_TC_GP_EnablePreview // Start preview
	{0x0F12, 0x0001}, // #REG_TC_GP_EnablePreviewChanged
        {0xFFFF, 0xFF}
};


const static struct mt9t113_reg s5k5cag_for_capture[] = {
    {0x0028, 0x7000}, 
    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig
    {0x0F12, 0x0000},  
    {0x0F12, 0x0001}, //#REG_TC_GP_CapConfigChanged
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync
    {0x0F12, 0x0001}, 
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture
    {0x0F12, 0x0001}, 
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged 
    {0xFFFF, 0xFF}
};


/*SVGA(800*600) to QCIF(128*96)*/
const static struct mt9t113_reg s5k5cag_128x96_preview[] = {
    {0xFFFF, 0xFF}  
};


/*SVGA(800*600) to QCIF(176*144)*/
const static struct mt9t113_reg s5k5cag_176x144_preview[] = {  
	{0x0028, 0x7000},
        {0x002A, 0x026C},                                                                     
        {0x0F12, 0x00B8}, //#REG_0TC_PCFG_usWidth//1024                                       
        {0x0F12, 0x0096}, //#REG_0TC_PCFG_usHeight //768    026E  
        {0x002A, 0x0288},
        {0x0F12, 0x029A}, //0x029A  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //20fps   //10 
        {0x0F12, 0x029A}, //14D //01c6  //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //27.5fps   //14 //0x01f3//0x02CA
        {0x002A, 0x023E},
        {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged 
        {0x0000, 0x0010}, // delay

	{0x002A, 0x023C},
	{0x0F12, 0x0000}, // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
	{0x002A, 0x0240},
	{0x0F12, 0x0001}, // #REG_TC_GP_PrevOpenAfterChange
	{0x002A, 0x0230},
	{0x0F12, 0x0001}, // #REG_TC_GP_NewConfigSync // Update preview configuration
	{0x002A, 0x023E},
	{0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged
	{0x002A, 0x0220},
	{0x0F12, 0x0001}, // #REG_TC_GP_EnablePreview // Start preview
	{0x0F12, 0x0001}, // #REG_TC_GP_EnablePreviewChanged
        {0x0000, 0x0014}, // delay

        {0xFFFF, 0xFF}
};

/*SVGA(800*600) to QVGA(320*240)*/
const static struct mt9t113_reg s5k5cag_320x240_preview[] = {
	{0x0028, 0x7000},
        {0x002A, 0x026C},                                                                     
        {0x0F12, 0x0148}, //#REG_0TC_PCFG_usWidth//1024                                       
        {0x0F12, 0x00F6}, //#REG_0TC_PCFG_usHeight //768    026E 
        {0x002A, 0x0288},
        {0x0F12, 0x029A}, //0580  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //10fps   //10 
        {0x0F12, 0x01F4}, //01c6  //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //20fps   //14 //0x01f3//0x02CA
        {0x002A, 0x023E},
        {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged 
        {0x0000, 0x0010}, // delay

	{0x002A, 0x023C},
	{0x0F12, 0x0000}, // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
	{0x002A, 0x0240},
	{0x0F12, 0x0001}, // #REG_TC_GP_PrevOpenAfterChange
	{0x002A, 0x0230},
	{0x0F12, 0x0001}, // #REG_TC_GP_NewConfigSync // Update preview configuration
	{0x002A, 0x023E},
	{0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged
	{0x002A, 0x0220},
	{0x0F12, 0x0001}, // #REG_TC_GP_EnablePreview // Start preview
	{0x0F12, 0x0001}, // #REG_TC_GP_EnablePreviewChanged
        {0x0000, 0x0014}, // delay
        {0xFFFF, 0xFF}
};

/*SVGA(800*600) to CIF(352*288)*/
const static struct mt9t113_reg s5k5cag_352x288_preview[] = {  
    {0x0028, 0x7000}, 
    {0x002A, 0x026C},                                                                     
    {0x0F12, 0x0168}, //#REG_0TC_PCFG_usWidth//1024                                       
    {0x0F12, 0x0126}, //#REG_0TC_PCFG_usHeight //768    026E  
    {0x002A, 0x0288},
    {0x0F12, 0x029A}, //0580  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //10fps   //10 
    {0x0F12, 0x01F4}, //01c6  //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //20fps   //14 //0x01f3//0x02CA  
    {0x002A, 0x023E},
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged 
    {0x0000, 0x0010}, // delay
                            
    {0x002A, 0x023C},                                                                     
    {0x0F12, 0x0000}, // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0    
    {0x002A, 0x0240},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevOpenAfterChange                                   
    {0x002A, 0x0230},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_NewConfigSync // Update preview configuration         
    {0x002A, 0x023E},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged                                     
    {0x002A, 0x0220},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreview // Start preview                        
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreviewChanged 
    {0x0000, 0x0014}, // delay  
    {0xFFFF, 0xFF}
};

/*SVGA(800*600) to VGA(640*480)*/
const static struct mt9t113_reg s5k5cag_640x480_preview[] = {
    {0x0028, 0x7000}, 
    {0x002A, 0x026C},                                                                     
    {0x0F12, 0x0288}, //#REG_0TC_PCFG_usWidth//1024                                       
    {0x0F12, 0x01E6}, //#REG_0TC_PCFG_usHeight //768    026E  
    {0x002A, 0x0288},
    {0x0F12, 0x03E8}, //0x01F4 //029A  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //20fps   //10 
    {0x0F12, 0x01F4}, //0x016B //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //20fps   //27.5fps //0x01f3//0x02CA 
    {0x002A, 0x023E},
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged 
    {0x0000, 0x0010}, // delay 
                            
    {0x002A, 0x023C},                                                                     
    {0x0F12, 0x0000}, // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0    
    {0x002A, 0x0240},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevOpenAfterChange                                   
    {0x002A, 0x0230},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_NewConfigSync // Update preview configuration         
    {0x002A, 0x023E},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged                                     
    {0x002A, 0x0220},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreview // Start preview                        
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreviewChanged   
    {0x0000, 0x0034}, // delay 0014
    {0xFFFF, 0xFF}
};

/*SVGA(800*600) to dvd-video ntsc(720*480)*/
const static struct mt9t113_reg s5k5cag_720x480_preview[] = {
    {0x0028, 0x7000},
    {0x002A, 0x026C},                                                                     
    {0x0F12, 0x02D8}, //#REG_0TC_PCFG_usWidth//1024                                       
    {0x0F12, 0x01E6}, //#REG_0TC_PCFG_usHeight //768    026E 
    {0x002A, 0x0288},
    {0x0F12, 0x01F4}, //0580  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 0x01F4=20fps 0x029A=15fps
    {0x0F12, 0x01F4}, //01c6  //#REG_0TC_PCFG_usMinFrTimeMsecMult10 
    {0x002A, 0x023E},
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged 
    {0x0000, 0x0010}, // delay
                            
    {0x002A, 0x023C},                                                                     
    {0x0F12, 0x0000}, // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0    
    {0x002A, 0x0240},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevOpenAfterChange                                   
    {0x002A, 0x0230},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_NewConfigSync // Update preview configuration         
    {0x002A, 0x023E},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged                                     
    {0x002A, 0x0220},                                                                     
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreview // Start preview                        
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreviewChanged 
    {0x0000, 0x0020}, // delay
    {0xFFFF,0xFF}
};

/*SVGA(800*600) to SVGA,for consistence*/
const static struct mt9t113_reg s5k5cag_800x600_preview[] = {  
	{0x0028, 0x7000},
        {0x002A, 0x026C},                                                                     
        {0x0F12, 0x0328}, //#REG_0TC_PCFG_usWidth//1024                                       
        {0x0F12, 0x025E}, //#REG_0TC_PCFG_usHeight //768    026E 
        {0x002A, 0x0288},
        {0x0F12, 0x03E8}, //029A  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //10fps   //10 
        {0x0F12, 0x029A}, //1A5 //01c6  //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //20fps  0x0190 = 25fps
        {0x002A, 0x023E},
	{0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged 
        {0x0000, 0x0010}, // delay
	
        {0x002A, 0x023C},
	{0x0F12, 0x0000}, // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
	{0x002A, 0x0240},
	{0x0F12, 0x0001}, // #REG_TC_GP_PrevOpenAfterChange
	{0x002A, 0x0230},
	{0x0F12, 0x0001}, // #REG_TC_GP_NewConfigSync // Update preview configuration
	{0x002A, 0x023E},
	{0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged
	{0x002A, 0x0220},
	{0x0F12, 0x0001}, // #REG_TC_GP_EnablePreview // Start preview
	{0x0F12, 0x0001}, // #REG_TC_GP_EnablePreviewChanged
        {0x0000, 0x0020}, // delay
        {0xFFFF, 0xFF}
};

/*VGA(640*480) to XGA(1024*768)*/
const static struct mt9t113_reg s5k5cag_1024x768_preview[] = {
    {0xFFFF, 0xFF}
};

/*VGA(640*480) to 1M(1280*960)*/
const static struct mt9t113_reg s5k5cag_1280x960_preview[] = { 
    {0xFFFF, 0xFF}
};

/*UXGA(1600*1200)*/
const static struct mt9t113_reg s5k5cag_1600x1200_preview[] = {
    {0xFFFF, 0xFF}
};

/*QXGA(2048*1536)*/
const static struct mt9t113_reg s5k5cag_2048x1536_preview[] = {
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg* s5k5cag_xxx_preview[] ={
    s5k5cag_128x96_preview,
    s5k5cag_176x144_preview,
    s5k5cag_320x240_preview,
    s5k5cag_352x288_preview,
    s5k5cag_640x480_preview,
    s5k5cag_720x480_preview,
    s5k5cag_800x600_preview,
    s5k5cag_1024x768_preview,
    s5k5cag_1280x960_preview,
    s5k5cag_1600x1200_preview,
    s5k5cag_2048x1536_preview
};


//128*96
const static struct mt9t113_reg s5k5cag_128x96_capture[]={
    
    {0xFFFF, 0xFF}   
};

/*SVGA(800*600) to QCIF(176*144)*/
const static struct mt9t113_reg s5k5cag_176x144_capture[] = {
   
    {0xFFFF, 0xFF}
};

/*SVGA(800*600) to QVGA(320*240)*/
const static struct mt9t113_reg s5k5cag_320x240_capture[] = {
    {0x0028, 0x7000},                                                                      
    {0x002A, 0x035C},                                                                      
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG                                     
    {0x0F12, 0x0148}, //#REG_0TC_CCFG_usWidth    //140                                          
    {0x0F12, 0x00F6}, //#REG_0TC_CCFG_usHeight   //F0  
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001}, 
    {0x0000, 0x0010}, // delay                                      

                                 
    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig                                      
    {0x0F12, 0x0000},                                                                    
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync                                         
    {0x0F12, 0x0001},                                                                   
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001},                                                                   
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture                                        
    {0x0F12, 0x0001},                                                                   
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged  
    {0x0000, 0x0030}, // delay      
    {0xFFFF, 0xFF}    
};

/*SVGA(800*600) to CIF(352*288)*/
const static struct mt9t113_reg s5k5cag_352x288_capture[] = {
    
    {0xFFFF, 0xFF}
};

/*SVGA(800*600) to vga(640*480)*/
const static struct mt9t113_reg s5k5cag_640x480_capture[] = {
    {0x0028, 0x7000},                                                                      
    {0x002A, 0x035C},                                                                      
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG                                     
    {0x0F12, 0x0288}, //#REG_0TC_CCFG_usWidth   //280                                            
    {0x0F12, 0x01E6}, //#REG_0TC_CCFG_usHeight  //1E0  
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001}, 
    {0x0000, 0x0010}, // delay                                          
                                  
    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig                                      
    {0x0F12, 0x0000},                                                                      
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync                                         
    {0x0F12, 0x0001},                                                                   
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001},                                                                   
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture                                        
    {0x0F12, 0x0001},                                                                   
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged  
    {0x0000, 0x0020}, // delay         
    {0xFFFF, 0xFF}
};


/*SVGA(800*600) to dvd-video ntsc(720*480)*/
const static struct mt9t113_reg s5k5cag_720x480_capture[] = {
    {0x0028, 0x7000},                                                                      
    {0x002A, 0x035C},                                                                      
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG                                     
    {0x0F12, 0x02D8}, //#REG_0TC_CCFG_usWidth   //280                                            
    {0x0F12, 0x01E6}, //#REG_0TC_CCFG_usHeight  //1E0  
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001}, 
    {0x0000, 0x0010}, // delay                                        
                                      
    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig                                      
    {0x0F12, 0x0000},                                                                      
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync                                         
    {0x0F12, 0x0001},                                                                   
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001},                                                                   
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture                                        
    {0x0F12, 0x0001},                                                                   
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged  
    {0x0000, 0x0030}, // delay         
    {0xFFFF, 0xFF}

};

/*SVGA(800*600) to SVGA,for consistence*/
const static struct mt9t113_reg s5k5cag_800x600_capture[] = {
    {0x0028, 0x7000},                                                                      
    {0x002A, 0x035C},                                                                      
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG                                     
    {0x0F12, 0x0328}, //#REG_0TC_CCFG_usWidth   //320                                           
    {0x0F12, 0x025E}, //#REG_0TC_CCFG_usHeight  //258 
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001}, 
    {0x0000, 0x0010}, // delay                                            
                                 
    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig                                      
    {0x0F12, 0x0000},                                                                      
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync                                         
    {0x0F12, 0x0001},                                                                   
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001},                                                                   
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture                                        
    {0x0F12, 0x0001},                                                                   
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged
    {0x0000, 0x0040}, // delay
    {0xFFFF, 0xFF}
};

/*SVGA(800*600) to XGA(1024*768)*/
const static struct mt9t113_reg s5k5cag_1024x768_capture[] = {
    {0x0028, 0x7000},                                                                         
    {0x002A, 0x035C},                                                                         
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG                                        
    {0x0F12, 0x0408}, //#REG_0TC_CCFG_usWidth                                                 
    {0x0F12, 0x0306}, //#REG_0TC_CCFG_usHeight  
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001}, 
    {0x0000, 0x0010}, // delay                                                
                                     
    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig                                         
    {0x0F12, 0x0000},                                                                         
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync                                            
    {0x0F12, 0x0001},                                                                         
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                        
    {0x0F12, 0x0001},                                                                      
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture                                           
    {0x0F12, 0x0001},                                                                      
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged  
    {0x0000, 0x0050}, // delay 
    {0xFFFF, 0xFF}
};

/*SVGA(800*600) to 1M(1280*960)*/
const static struct mt9t113_reg s5k5cag_1280x960_capture[] = {
    {0x0028, 0x7000},                                                                         
    {0x002A, 0x035C},                                                                         
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG                                        
    {0x0F12, 0x0508}, //#REG_0TC_CCFG_usWidth                                                 
    {0x0F12, 0x03C6}, //#REG_0TC_CCFG_usHeight   
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001}, 
    {0x0000, 0x0010}, // delay                                               
                                     
    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig                                         
    {0x0F12, 0x0000},                                                                         
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync                                            
    {0x0F12, 0x0001},                                                                         
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                        
    {0x0F12, 0x0001},                                                                      
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture                                           
    {0x0F12, 0x0001},                                                                      
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged  
    {0x0000, 0x0050}, // delay  
    {0xFFFF, 0xFF}
};

/*UXGA(1600*1200)*/
const static struct mt9t113_reg s5k5cag_1600x1200_capture[] = {
    {0x0028, 0x7000},
    {0x002A, 0x035C},                                                                       
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG                                      
    {0x0F12, 0x0648}, //#REG_0TC_CCFG_usWidth                                               
    {0x0F12, 0x04B6}, //#REG_0TC_CCFG_usHeight  
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001}, 
    {0x0000, 0x0010}, // delay                                             
                               
    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig                                       
    {0x0F12, 0x0000},                                                                       
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync                                          
    {0x0F12, 0x0001},                                                                       
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                      
    {0x0F12, 0x0001},                                                                    
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture                                         
    {0x0F12, 0x0001},                                                                    
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged  
    {0x0000, 0x0050}, // delay 
    {0xFFFF, 0xFF}
};

/*QXGA(2048*1536)*/
const static struct mt9t113_reg s5k5cag_2048x1536_capture[] = {
#if 1
    {0x0028, 0x7000},    
    {0x002A, 0x035C},                                        
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG       
    {0x0F12, 0x0800}, //#REG_0TC_CCFG_usWidth                
    {0x0F12, 0x0600}, //#REG_0TC_CCFG_usHeight 
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged                                     
    {0x0F12, 0x0001}, 
    {0x0000, 0x0020}, // delay  

    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig  
    {0x0F12, 0x0000},                                 
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync    
    {0x0F12, 0x0001},                                 
    {0x002a, 0x0246}, //#REG_TC_GP_CapConfigChanged
    {0x0F12, 0x0001},                              
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture   
    {0x0F12, 0x0001},                              
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged  
    //{0x0000, 0x012C}, //delay 300ms
   // {0x0000, 0x00FA}, //delay 250ms 
   // {0x0000, 0x00C8}, //delay 200ms 
   // {0x0000, 0x015E}, //delay 350ms
   {0x0000, 0x0190}, //delay 400ms
#else
    {0x0028, 0x7000}, 
    {0x002a, 0x0244}, //#REG_TC_GP_ActiveCapConfig
    {0x0F12, 0x0001},  
    {0x0F12, 0x0001}, //#REG_TC_GP_CapConfigChanged
    {0x002a, 0x0230}, //#REG_TC_GP_NewConfigSync
    {0x0F12, 0x0001}, 
    {0x002a, 0x0224}, //#REG_TC_GP_EnableCapture
    {0x0F12, 0x0001}, 
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged 
    {0x0000, 0x0100}, //delay 256ms

#endif
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg* s5k5cag_xxx_capture[] ={
    s5k5cag_128x96_capture,
    s5k5cag_176x144_capture,
    s5k5cag_320x240_capture,
    s5k5cag_352x288_capture,
    s5k5cag_640x480_capture,
    s5k5cag_720x480_capture,
    s5k5cag_800x600_capture,
    s5k5cag_1024x768_capture,
    s5k5cag_1280x960_capture,
    s5k5cag_1600x1200_capture,
    s5k5cag_2048x1536_capture
};

/*HBlank for 2048x1536*/
const static struct mt9t113_reg s5k5cag_capture_HBlank_QXGA[] = {
    {0x0028, 0x7000},
    {0x002A, 0x12c8},                                        
    {0x0F12, 0x08FC}, 
    {0xFFFF, 0xFF}
};

/*HBlank for default*/
const static struct mt9t113_reg s5k5cag_capture_HBlank_Default[] = {
    {0x0028, 0x7000},
    {0x002A, 0x12c8},                                        
    {0x0F12, 0x08AC}, 
    {0xFFFF, 0xFF}
};

/*5fps*/
const static struct mt9t113_reg s5k5cag_capture_5fps[] = {
    {0x0028, 0x7000},                                                                       
    {0x002A, 0x037A},                                                                       
    {0x0F12, 0x07D0}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //5fps                                          
    {0x0F12, 0x07D0}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //5fps                                             
    {0x0000, 0x000A},
    {0xFFFF, 0xFF}
};

/*7.5fps*/
const static struct mt9t113_reg s5k5cag_capture_7_5fps[] = {
    {0x0028, 0x7000},                                                                       
    {0x002A, 0x037A},                                                                       
    {0x0F12, 0x0535}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps                                          
    {0x0F12, 0x0535}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //7.5fps                                             
    {0x0000, 0x000A},
    {0xFFFF, 0xFF}
};



/*10~20fps*/
const static struct mt9t113_reg s5k5cag_preview_10_20fps[] = {
    {0x0028, 0x7000},                                                                       
    {0x002A, 0x0288},
    {0x0F12, 0x03E8}, //0x01F4 //029A  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //20fps   //10 
    {0x0F12, 0x01F4}, //0x016B //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //20fps   //27.5fps //0x01f3//0x02CA 
    {0x002A, 0x023E},
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged 
    {0x0000, 0x0010}, // delay 
    {0xFFFF, 0xFF}
};
/*20fps*/
const static struct mt9t113_reg s5k5cag_preview_20fps[] = {
    {0x0028, 0x7000},                                                                       
    {0x002A, 0x0288},
    {0x0F12, 0x01F4}, //0x01F4 //029A  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //20fps   //10 
    {0x0F12, 0x01F4}, //0x016B //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //20fps   //27.5fps //0x01f3//0x02CA 
    {0x002A, 0x023E},
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged 
    {0x0000, 0x0010}, // delay 
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg s5k5cag_effect_off[]={
    {0x0028, 0x7000},	
    {0x002A, 0x021E}, //REG_TC_GP_SpecialEffects
    {0x0F12, 0x0000},  
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg s5k5cag_effect_mono[]={
    {0x0028, 0x7000},	
    {0x002A, 0x021E}, //REG_TC_GP_SpecialEffects
    {0x0F12, 0x0001}, 
    {0xFFFF, 0xFF}
}; 

const static struct mt9t113_reg s5k5cag_effect_negative[]={
    {0x0028, 0x7000},	
    {0x002A, 0x021E}, //REG_TC_GP_SpecialEffects
    {0x0F12, 0x0003}, 
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg s5k5cag_effect_sepia[]={
    {0x0028, 0x7000},	
    {0x002A, 0x021E}, //REG_TC_GP_SpecialEffects
    {0x0F12, 0x0004}, 	
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg s5k5cag_effect_green_aqua[]={
#if 1
    {0x0028, 0x7000},	
    {0x002A, 0x021E}, //REG_TC_GP_SpecialEffects
    {0x0F12, 0x0005}, 
#endif
};

/*
const static struct mt9t113_reg s5k5cag_effect_emboss[]={
    {0x0028, 0x7000},	
    {0x002A, 0x021E}, //REG_TC_GP_SpecialEffects
    {0x0F12, 0x0008}, 	
    {0xFFFF, 0xFF}
};
*/

const static struct mt9t113_reg* s5k5cag_effects[]={
    s5k5cag_effect_off,
    s5k5cag_effect_mono,
    s5k5cag_effect_negative,
    s5k5cag_effect_sepia,
    s5k5cag_effect_green_aqua
};


/*AWB init gain*/
const static struct mt9t113_reg s5k5cag_awb_gain[] = {
//AWB init gain
{0x002A, 0x0E44},
{0x0F12, 0x053C},//0x053C //awbb_GainsInit_0_  R_gain	
{0x0F12, 0x0400}, //awbb_GainsInit_1_	G_GAIN
{0x0F12, 0x05F8}, //61C //awbb_GainsInit_2_	B_gain   
};

/*flicker_mode=Manual(50Mhz&60Mhz)*/
const static struct mt9t113_reg s5k5cag_wb_auto[]={
    {0x0028, 0x7000},                                            
    {0x002A, 0X04D2}, //REG_TC_DBG_AutoAlgEnBits                 
    {0x0F12, 0x067F}, 
    {0xFFFF, 0xFF}   
}; 
 
const static struct mt9t113_reg s5k5cag_wb_daylight[]={
    {0x0028, 0x7000},		                             
    {0x002A, 0x04D2},                                 
    {0x0F12, 0x0677}, // AWB Off                   
    {0x002A, 0x04A0}, //#REG_SF_USER_Rgain         
    {0x0F12, 0x0530},                                 
    {0x0F12, 0x0001},                                 
    {0x0F12, 0x0400}, //REG_SF_USER_Ggain           
    {0x0F12, 0x0001},		                             
    {0x0F12, 0x0590}, // #REG_SF_USER_Bgain         
    {0x0F12, 0x0001},         
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg s5k5cag_wb_cloudy[]={
    {0x0028, 0x7000},		                               
    {0x002A, 0x04D2},                                  
    {0x0F12, 0x0677}, // AWB Off                     
    {0x002A, 0x04A0}, //#REG_SF_USER_Rgain           
    {0x0F12, 0x0540},                                  
    {0x0F12, 0x0001},                                  
    {0x0F12, 0x0400}, //REG_SF_USER_Ggain            
    {0x0F12, 0x0001},		                               
    {0x0F12, 0x0500}, // #REG_SF_USER_Bgain          	
    {0x0F12, 0x0001}, 
    {0xFFFF, 0xFF}

};

const static struct mt9t113_reg s5k5cag_wb_incandescent[]={
    {0x0028, 0x7000},		                               
    {0x002A, 0x04D2},                                  
    {0x0F12, 0x0677}, // AWB Off                    
    {0x002A, 0x04A0}, //#REG_SF_USER_Rgain           
    {0x0F12, 0x0400},                                  
    {0x0F12, 0x0001},                                  
    {0x0F12, 0x040D}, //REG_SF_USER_Ggain            
    {0x0F12, 0x0001},		                               
    {0x0F12, 0x0888}, // #REG_SF_USER_Bgain          
    {0x0F12, 0x0001}, 
    {0xFFFF, 0xFF}
};

/*CAM_WB_Florescent_TL84*/
const static struct mt9t113_reg s5k5cag_wb_fluorescent[]={
    {0x0028, 0x7000},		                               
    {0x002A, 0x04D2},                                  
    {0x0F12, 0x0677}, // AWB Off                    
    {0x002A, 0x04A0}, //#REG_SF_USER_Rgain           
    {0x0F12, 0x0490},                                  
    {0x0F12, 0x0001},                                  
    {0x0F12, 0x0400}, //REG_SF_USER_Ggain            
    {0x0F12, 0x0001},		                               
    {0x0F12, 0x0760}, // #REG_SF_USER_Bgain          
    {0x0F12, 0x0001},  
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg* s5k5cag_whitebalance[]={
    s5k5cag_wb_auto,
    s5k5cag_wb_daylight,
    s5k5cag_wb_cloudy,
    s5k5cag_wb_incandescent, 
    s5k5cag_wb_fluorescent,
};

const static struct mt9t113_reg s5k5cag_exposure_1[]={  
    {0x0028, 0x7000}, 
    {0x002A, 0x020C}, 
    {0x0F12, 0x0000}, 

    {0x0028, 0x7000},
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x0016}, //0x0019	 
    {0xFFFF, 0xFF}
};
const static struct mt9t113_reg s5k5cag_exposure_2[]={  
    {0x0028, 0x7000}, 
    {0x002A, 0x020C}, 
    {0x0F12, 0x0000}, 

    {0x0028, 0x7000}, 
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x0025}, // 0x0025	
    {0xFFFF, 0xFF}
};
const static struct mt9t113_reg s5k5cag_exposure_3[]={ 
    {0x0028, 0x7000}, 
    {0x002A, 0x020C}, 
    {0x0F12, 0x0000}, 
  
    {0x0028, 0x7000}, 	
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x002F},  //2D //46 //3D
    {0xFFFF, 0xFF}
};
const static struct mt9t113_reg s5k5cag_exposure_4[]={  
    {0x0028, 0x7000}, 
    {0x002A, 0x020C}, 
    {0x0F12, 0x002D}, 
 
    {0x0028, 0x7000}, 
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x0049}, //0x0059	
    {0xFFFF, 0xFF}
};
const static struct mt9t113_reg s5k5cag_exposure_5[]={  
    {0x0028, 0x7000}, 
    {0x002A, 0x020C}, 
    {0x0F12, 0x0058}, 
 
    {0x0028, 0x7000},    
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x0060}, //0x0075	
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg* s5k5cag_exposure[]={
    s5k5cag_exposure_1,
    s5k5cag_exposure_2,
    s5k5cag_exposure_3,
    s5k5cag_exposure_4,
    s5k5cag_exposure_5
};

const static struct mt9t113_reg s5k5cag_brightness_1[]={  
    {0x0028, 0x7000},
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x0019}, //0x000D	 
    {0xFFFF, 0xFF}
};
const static struct mt9t113_reg s5k5cag_brightness_2[]={  
    {0x0028, 0x7000}, 
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x0025}, // 0x0025	
    {0xFFFF, 0xFF}
};
const static struct mt9t113_reg s5k5cag_brightness_3[]={   
    {0x0028, 0x7000}, 	
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x003D},   
    {0xFFFF, 0xFF}
};
const static struct mt9t113_reg s5k5cag_brightness_4[]={   
    {0x0028, 0x7000}, 
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x0059}, //0x0059	
    {0xFFFF, 0xFF}
};
const static struct mt9t113_reg s5k5cag_brightness_5[]={   
    {0x0028, 0x7000},    
    {0x002A, 0x0F70}, //#TVAR_ae_BrAve //ae target
    {0x0F12, 0x0075}, //0x0075	
    {0xFFFF, 0xFF}
};

const static struct mt9t113_reg* s5k5cag_brightness[]={
    s5k5cag_brightness_1,
    s5k5cag_brightness_2,
    s5k5cag_brightness_3,
    s5k5cag_brightness_4,
    s5k5cag_brightness_5
};

const static struct mt9t113_reg s5k5cag_saturation_1[]={   
    {0x0028, 0x7000},                                              
    {0x002a, 0x0210},               
    {0x0f12, 0xFFB0}, 
    {0xFFFF, 0xFF}    
};
const static struct mt9t113_reg s5k5cag_saturation_2[]={  
    {0x0028, 0x7000},                                                
    {0x002a, 0x0210},               
    {0x0f12, 0xFFD8},      
    {0xFFFF, 0xFF}    
};
const static struct mt9t113_reg s5k5cag_saturation_3[]={   
    {0x0028, 0x7000},  
    {0x002a, 0x0210},  
    {0x0f12, 0x0000},    
    {0xFFFF, 0xFF}     
};
const static struct mt9t113_reg s5k5cag_saturation_4[]={    
    {0x0028, 0x7000},               
    {0x002a, 0x0210},               
    {0x0f12, 0x0028},    
    {0xFFFF, 0xFF}  
};
const static struct mt9t113_reg s5k5cag_saturation_5[]={     
    {0x0028, 0x7000},               
    {0x002a, 0x0210},               
    {0x0f12, 0x0050},   
    {0xFFFF, 0xFF}     
};

const static struct mt9t113_reg* s5k5cag_saturation[]={
    s5k5cag_saturation_1,
    s5k5cag_saturation_2,
    s5k5cag_saturation_3,
    s5k5cag_saturation_4,
    s5k5cag_saturation_5
};

/*
const static struct mt9t113_reg s5k5cag_contrast_1[]={   
    {0x0028, 0x7000},                                              
    {0x002a, 0x020E},               
    {0x0f12, 0xFFC4}, 
    {0xFFFF, 0xFF}    
};
const static struct mt9t113_reg s5k5cag_contrast_2[]={  
    {0x0028, 0x7000},                                                
    {0x002a, 0x020E},           
    {0x0f12, 0xFFEC},      
    {0xFFFF, 0xFF}    
};
const static struct mt9t113_reg s5k5cag_contrast_3[]={   
    {0x0028, 0x7000},  
    {0x002a, 0x020E}, 
    {0x0f12, 0x0000},    
    {0xFFFF, 0xFF}     
};
const static struct mt9t113_reg s5k5cag_contrast_4[]={    
    {0x0028, 0x7000},               
    {0x002a, 0x020E},              
    {0x0f12, 0x0028},    
    {0xFFFF, 0xFF}  
};
const static struct mt9t113_reg s5k5cag_contrast_5[]={     
    {0x0028, 0x7000},               
    {0x002a, 0x020E},              
    {0x0f12, 0x003C},   
    {0xFFFF, 0xFF}     
};

const static struct mt9t113_reg* s5k5cag_contrast[]={
    s5k5cag_contrast_1,
    s5k5cag_contrast_2,
    s5k5cag_contrast_3,
    s5k5cag_contrast_4,
    s5k5cag_contrast_5
};
*/

const static struct mt9t113_reg s5k5cag_zoom_1[]={   
    {0x0028, 0x7000},                                              
    {0x002a, 0x0474},
    {0x0f12, 0x0100},           
    {0x002a, 0x0466}, 
    {0x0f12, 0x0002},
    {0xFFFF, 0xFF}    
};
const static struct mt9t113_reg s5k5cag_zoom_2[]={  
    {0x0028, 0x7000},                                              
    {0x002a, 0x0474},
    {0x0f12, 0x0119},           
    {0x002a, 0x0466}, 
    {0x0f12, 0x0002},    
    {0xFFFF, 0xFF}    
};
const static struct mt9t113_reg s5k5cag_zoom_3[]={   
    {0x0028, 0x7000},                                              
    {0x002a, 0x0474},
    {0x0f12, 0x0133},           
    {0x002a, 0x0466}, 
    {0x0f12, 0x0002},   
    {0xFFFF, 0xFF}     
};
const static struct mt9t113_reg s5k5cag_zoom_4[]={    
    {0x0028, 0x7000},                                              
    {0x002a, 0x0474},
    {0x0f12, 0x014c},           
    {0x002a, 0x0466}, 
    {0x0f12, 0x0002},   
    {0xFFFF, 0xFF}  
};
const static struct mt9t113_reg s5k5cag_zoom_5[]={     
    {0x0028, 0x7000},                                              
    {0x002a, 0x0474},
    {0x0f12, 0x0166},           
    {0x002a, 0x0466}, 
    {0x0f12, 0x0002},  
    {0xFFFF, 0xFF}     
};
const static struct mt9t113_reg s5k5cag_zoom_6[]={     
    {0x0028, 0x7000},                                              
    {0x002a, 0x0474},
    {0x0f12, 0x0180},           
    {0x002a, 0x0466}, 
    {0x0f12, 0x0002},  
    {0xFFFF, 0xFF}     
};
const static struct mt9t113_reg s5k5cag_zoom_7[]={     
    {0x0028, 0x7000},                                              
    {0x002a, 0x0474},
    {0x0f12, 0x0199},           
    {0x002a, 0x0466}, 
    {0x0f12, 0x0002},   
    {0xFFFF, 0xFF}     
};

const static struct mt9t113_reg* s5k5cag_zoom[]={
    s5k5cag_zoom_1,
    s5k5cag_zoom_2,
    s5k5cag_zoom_3,
    s5k5cag_zoom_4,
    s5k5cag_zoom_5,
    s5k5cag_zoom_6,
    s5k5cag_zoom_7,
};

const static struct mt9t113_reg s5k5cag_sharpness_default[]={     
    {0x0028, 0x7000},                                              
    {0x002A, 0x0212},
    {0x0F12, 0xFFF0}, //fff5   
    {0xFFFF, 0xFF}     
};

typedef enum
{
    EXPOSURE_MODE_EXP_AUTO,
    EXPOSURE_MODE_EXP_MACRO,
    EXPOSURE_MODE_EXP_PORTRAIT,
    EXPOSURE_MODE_EXP_LANDSCAPE,
    EXPOSURE_MODE_EXP_SPORTS,
    EXPOSURE_MODE_EXP_NIGHT,
    EXPOSURE_MODE_EXP_NIGHT_PORTRAIT,
    EXPOSURE_MODE_EXP_BACKLIGHTING,
    EXPOSURE_MODE_EXP_MANUAL
} EXPOSURE_MODE_VALUES;

typedef enum {
    SENSOR_SCENE_MODE_AUTO = 0,
    SENSOR_SCENE_MODE_DAYLIGHT = 1,
    SENSOR_SCENE_MODE_CLOUDY = 2,
    SENSOR_SCENE_MODE_INCANDESCENT = 3,
    SENSOR_SCENE_MODE_FLUORESCENT = 4,
}SENSOR_SCENE_MODE;

typedef enum{
    SENSOR_WHITE_BALANCE_AUTO = 0,
    SENSOR_WHITE_BALANCE_DAYLIGHT = 1,
    SENSOR_WHITE_BALANCE_CLOUDY = 2,
    SENSOR_WHITE_BALANCE_INCANDESCENT = 3,
    SENSOR_WHITE_BALANCE_FLUORESCENT = 4,
    SENSOR_WHITE_BALANCE_NIGHT = 5     
} SENSOR_WHITE_BALANCE_MODE;

typedef enum {
    SENSOR_EFFECT_NORMAL = 0,
    SENSOR_EFFECT_MONO = 1,
    SENSOR_EFFECT_NEGATIVE = 2,
    SENSOR_EFFECT_SEPIA =3,

    SENSOR_EFFECT_BLUISH = 4,
    SENSOR_EFFECT_GREEN = 5,
    SENSOR_EFFECT_REDDISH = 6,
    SENSOR_EFFECT_YELLOWISH = 7

}SENSOR_EFFECTS;


struct sensor_ext_params{
    SENSOR_EFFECTS effect;
    SENSOR_WHITE_BALANCE_MODE white_balance;
    SENSOR_SCENE_MODE scene_mode;
    int brightness;
    int contrast;
    int saturation;
    int exposure;
    int sharpness;
    int zoom;
};

/**
 * struct mt9t113_sensor - main structure for storage of sensor information
 * @pdata: access functions and data for platform level information
 * @v4l2_int_device: V4L2 device structure structure
 * @i2c_client: iic client device structure
 * @pix: V4L2 pixel format information structure
 * @timeperframe: time per frame expressed as V4L fraction
 * @isize: base image size
 * @ver: mt9t113 chip version
 * @width: configured width
 * @height: configuredheight
 * @vsize: vertical size for the image
 * @hsize: horizontal size for the image
 * @crop_rect: crop rectangle specifying the left,top and width and height
 */
struct mt9t113_sensor {
	const struct mt9t113_platform_data *pdata;
	struct v4l2_int_device *v4l2_int_device;
	struct i2c_client *i2c_client;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	struct sensor_ext_params ext_params;
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


#endif /* ifndef MT9T113_H */




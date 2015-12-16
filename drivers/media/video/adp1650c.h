/*                                                      
 * Definitions for adp1650 led flash chip.              
 */                                                     
                                                        
#ifndef LEDFLASH_ADP1650_H                              
#define LEDFLASH_ADP1650_H   
                           
#include <linux/videodev2.h>                                                          
#include <mach/isp_user.h>                                                                
#include <media/v4l2-int-device.h>     
                                                       
#include <linux/ioctl.h>
#include <linux/earlysuspend.h> 

#define ADP1650_DRIVER_NAME         "spotlight" 
//#define ADP1650_DRIVER_NAME	    "adp1650"  
#define ADP1650_I2C_ADDR             0x60 //(0x60 >> 1)   

/* Flash and privacy (indicator) light controls */
#define V4L2_CID_FLASH_STROBE			(V4L2_CID_CAMERA_CLASS_BASE+19)
#define V4L2_CID_FLASH_TIMEOUT			(V4L2_CID_CAMERA_CLASS_BASE+20)
#define V4L2_CID_FLASH_INTENSITY		(V4L2_CID_CAMERA_CLASS_BASE+21)
#define V4L2_CID_TORCH_INTENSITY		(V4L2_CID_CAMERA_CLASS_BASE+22)
#define V4L2_CID_INDICATOR_INTENSITY		(V4L2_CID_CAMERA_CLASS_BASE+23)
#define V4L2_CID_TORCH_ON                       (V4L2_CID_CAMERA_CLASS_BASE+24) 
#define V4L2_CID_FLASH_OFF                      (V4L2_CID_CAMERA_CLASS_BASE+25)   
#define V4L2_CID_FLASH_ON                       (V4L2_CID_CAMERA_CLASS_BASE+26)  
 
#define FLASHLIGHT_OFF   0 
#define FLASHLIGHT_TORCH 1 
#define FLASHLIGHT_FLASH 2 
#define FLASHLIGHT_NUM   3                               
                                                        
#define MAX_FAILURE_COUNT               3              
                                                       
/* ADP1650 register address */                         
#define INFO_REG                        (uint8_t) 0x00 
#define TIMER_REG                       (uint8_t) 0x02 
#define CURRENT_SET_REG                 (uint8_t) 0x03 
#define OUTPUT_MODE_REG                 (uint8_t) 0x04 
#define FAULT_INFO_REG                  (uint8_t) 0x05 
#define INPUT_CTRL_REG                  (uint8_t) 0x06 
#define AD_MOD_REG                      (uint8_t) 0x07 
#define AD_ADC_REG                      (uint8_t) 0x08 
#define BATT_LOW_REG                    (uint8_t) 0x09 
                                                       
/* VREF and timer register */                          
#define TIMER_100MS			(uint8_t) 0x00
#define TIMER_200MS			(uint8_t) 0x01
#define TIMER_300MS			(uint8_t) 0x02
#define TIMER_400MS			(uint8_t) 0x03
#define TIMER_600MS                     (uint8_t) 0x05
#define TIMER_900MS                     (uint8_t) 0x08
#define TIMER_1000MS                    (uint8_t) 0x09
#define TIMER_1200MS                    (uint8_t) 0x0b
#define TIMER_1400MS                    (uint8_t) 0x0d
#define TIMER_1600MS			(uint8_t) 0x0f
                                                       
/* Current set register */                             
#define CUR_FL_900MA			( (uint8_t) (0x0C<<3) )
#define CUR_FL_700MA			( (uint8_t) (0x08<<3) )
#define CUR_FL_600MA			( (uint8_t) (0x06<<3) )
#define CUR_FL_550MA			( (uint8_t) (0x05<<3) )
#define CUR_FL_500MA			( (uint8_t) (0x04<<3) )
#define CUR_FL_400MA			( (uint8_t) (0x02<<3) )
#define CUR_FL_300MA		        ( (uint8_t) (0x00<<3) )
                                                       
#define CUR_TOR_25MA                    (uint8_t) 0x00 
#define CUR_TOR_50MA                    (uint8_t) 0x01 
#define CUR_TOR_75MA                    (uint8_t) 0x02 
#define CUR_TOR_100MA                   (uint8_t) 0x03 
#define CUR_TOR_125MA                   (uint8_t) 0x04 
#define CUR_TOR_150MA                   (uint8_t) 0x05 
#define CUR_TOR_175MA                   (uint8_t) 0x06 
#define CUR_TOR_200MA                   (uint8_t) 0x07 
                                                       
/* Output mode register */                             
#define OUTPUT_MODE_DEF                 (uint8_t) 0xa0 
#define OUTPUT_MODE_ASSIST              (uint8_t) 0x0a 
#define OUTPUT_MODE_FLASH               (uint8_t) 0x0f   


#define FLASH_STROBE	(170)
#define FLASH_EN	(155)
#define TORCH_EN_GPIO1	(153)  

enum ioctl_flashmode_flags { 
        IOCTL_MODE_FACTORY_FLASH = 10000,
        IOCTL_MODE_OFF = 10001, 
        IOCTL_MODE_TORCH = 10002, 
        IOCTL_MODE_ON = 10003, 
        IOCTL_MODE_FLASH = 10004,
        IOCTL_MODE_PRE_FLASH = 10005,        
};                                                         
 
enum flashlight_mode_flags { 
        FL_MODE_OFF = 0, 
        FL_MODE_ON, 
        FL_MODE_TORCH, 
        FL_MODE_FLASH, 
        FL_MODE_PRE_FLASH, 
        FL_MODE_TORCH_LED_A, 
        FL_MODE_TORCH_LED_B, 
        FL_MODE_TORCH_LEVEL_1, 
        FL_MODE_TORCH_LEVEL_2, 
        FL_MODE_CAMERA_EFFECT_FLASH, 
        FL_MODE_CAMERA_EFFECT_PRE_FLASH, 
}; 
 
struct flashlight_platform_data { 
        void (*gpio_init) (void); 
        uint32_t torch; 
        uint32_t flash; 
        uint32_t flash_adj; 
        uint32_t torch_set1; 
        uint32_t torch_set2; 
        uint32_t flash_duration_ms; 
        uint8_t led_count; /* 0: 1 LED, 1: 2 LED */ 
        uint32_t chip_model; 
}; 

struct adp1650_platform_data {                                                                                             
	int (*gpio_init) (void);                                                       
	int (*priv_data_set)(void *priv);                       
}; 
                                                                                                                       
int adp1650_flashlight_control(int mode); 
                                             
extern  int is_low_light(void);                                                   
#endif                                                 

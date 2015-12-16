/*
 * ALSA SoC TWL4030 codec driver
 *
 * Author:      Steve Sakoman, <steve@sakoman.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include <mach/gpio.h>    //yuanyufang add 2010-12-27
#include <mach/mux.h>    //yuanyufang add 2010-12-27
#include <mach/cp5860e_audio.h>//added by huangjiefeng
#include <plat/misc.h>    //yuanyufang add 2010-12-27
#include <plat/yl_debug.h>

#include "twl4030.h"

#define GPIO_2			2    //yuanyufang add 2010-12-27
#define GPIO_97			97    //yuanyufang add 2010-12-27
#define GPIO_22			22    //yuanyufang add 2010-12-27
#define GPIO_6			6    //yuanyufang add 2011-3-14
#define GPIO_127		127    //yuanyufang add 2010-12-27
#define GPIO_HEAD_DET			194    //yuanyufang add 2010-12-27

#define CALLING_HEADSET_MODE		1    //yuanyufang add 2010-12-27
#define CALLING_EARPIECE_MODE		2    //yuanyufang add 2010-12-27
#define CALLING_SPEAKER_MODE		3    //yuanyufang add 2010-12-27

#define AUDIOELECTRIC_AUTOTEST

/*
 * twl4030 register cache & default register settings
 */
static const u8 twl4030_reg[TWL4030_CACHEREGNUM] = {
	0x00, /* this register not used		*/
	0x92, /* REG_CODEC_MODE		(0x1)	*/ 
	0xc3, /* REG_OPTION		(0x2)	*/
	0x00, /* REG_UNKNOWN		(0x3)	*/
	0x00, /* REG_MICBIAS_CTL	(0x4)	*/
	0x20, /* REG_ANAMICL		(0x5)	*/
	0x00, /* REG_ANAMICR		(0x6)	*/
	0x04, /* REG_AVADC_CTL		(0x7)	*/
	0x00, /* REG_ADCMICSEL		(0x8)	*/
	0x00, /* REG_DIGMIXING		(0x9)	*/
	0x01, /* REG_ATXL1PGA		(0xA)	*/
	0x12, /* REG_ATXR1PGA		(0xB)	*//*本地耳机=10,其它=12*/
	0x00, /* REG_AVTXL2PGA		(0xC)	*/
	0x00, /* REG_AVTXR2PGA		(0xD)	*/
	0x01, /* REG_AUDIO_IF		(0xE)	*/
	0x04, /* REG_VOICE_IF		(0xF)	*/
	0x00, /* REG_ARXR1PGA		(0x10)	*/
	0x00, /* REG_ARXL1PGA		(0x11)	*/
	0x6c, /* REG_ARXR2PGA		(0x12)	*/
	0x6c, /* REG_ARXL2PGA		(0x13)	*/
	0x00, /* REG_VRXPGA		(0x14)	*/
	0x00, /* REG_VSTPGA		(0x15)	*/
	0x00, /* REG_VRX2ARXPGA		(0x16)	*/
	0x00, /* REG_AVDAC_CTL		(0x17)	*/
	0x00, /* REG_ARX2VTXPGA		(0x18)	*/
	0x00, /* REG_ARXL1_APGA_CTL	(0x19)	*/
	0x00, /* REG_ARXR1_APGA_CTL	(0x1A)	*/
	0x92, /* REG_ARXL2_APGA_CTL	(0x1B)	*/ /* 外放=1b 外放即免提      */
	0x92, /* REG_ARXR2_APGA_CTL	(0x1C)	*/ /* 外放=1b 外放即免提      */
	0x00, /* REG_ATX2ARXPGA		(0x1D)	*/
	0x00, /* REG_BT_IF		(0x1E)	*/
	0x00, /* REG_BTPGA		(0x1F)	*/
	0x00, /* REG_BTSTPGA		(0x20)	*/
	0x31, /* REG_EAR_CTL		(0x21)	*/ 
	0x24, /* REG_HS_SEL		(0x22)	*/
	0x0f, /* REG_HS_GAIN_SET	(0x23)	*/ 
	0x00, /* REG_HS_POPN_SET	(0x24)	*/
	0x00, /* REG_PREDL_CTL		(0x25)	*/ /*外放=14*/
	0x00, /* REG_PREDR_CTL		(0x26)	*/ /*外放=14*/
	0x00, /* REG_PRECKL_CTL		(0x27)	*/
	0x00, /* REG_PRECKR_CTL		(0x28)	*/
	0x00, /* REG_HFL_CTL		(0x29)	*/
	0x00, /* REG_HFR_CTL		(0x2A)	*/
	0x00, /* REG_ALC_CTL		(0x2B)	*/
	0x00, /* REG_ALC_SET1		(0x2C)	*/
	0x00, /* REG_ALC_SET2		(0x2D)	*/
	0x00, /* REG_BOOST_CTL		(0x2E)	*/
	0x00, /* REG_SOFTVOL_CTL	(0x2F)	*/
	0x00, /* REG_DTMF_FREQSEL	(0x30)	*/
	0x00, /* REG_DTMF_TONEXT1H	(0x31)	*/
	0x00, /* REG_DTMF_TONEXT1L	(0x32)	*/
	0x00, /* REG_DTMF_TONEXT2H	(0x33)	*/
	0x00, /* REG_DTMF_TONEXT2L	(0x34)	*/
	0x00, /* REG_DTMF_TONOFF	(0x35)	*/
	0x00, /* REG_DTMF_WANONOFF	(0x36)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_H	(0x37)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_M	(0x38)	*/
	0x00, /* REG_I2S_RX_SCRAMBLE_L	(0x39)	*/
	0x16, /* REG_APLL_CTL		(0x3A)	*/
	0x00, /* REG_DTMF_CTL		(0x3B)	*/
	0x00, /* REG_DTMF_PGA_CTL2	(0x3C)	*/
	0x00, /* REG_DTMF_PGA_CTL1	(0x3D)	*/
	0x00, /* REG_MISC_SET_1		(0x3E)	*/
	0x00, /* REG_PCMBTMUX		(0x3F)	*/
	0x00, /* not used		(0x40)	*/
	0x00, /* not used		(0x41)	*/
	0x00, /* not used		(0x42)	*/
	0x00, /* REG_RX_PATH_SEL	(0x43)	*/
	0x00, /* REG_VDL_APGA_CTL	(0x44)	*/
	0x00, /* REG_VIBRA_CTL		(0x45)	*/
	0x00, /* REG_VIBRA_SET		(0x46)	*/
	0x00, /* REG_VIBRA_PWM_SET	(0x47)	*/
	0x09, /* REG_ANAMIC_GAIN	(0x48)	*/ /*yuanyufang modify 2010-12-27, before is 0x00,*/
	0x00, /* REG_MISC_SET_2		(0x49)	*/
	0x00, /* REG_SW_SHADOW		(0x4A)	- Shadow, non HW register */
};
#ifdef CHECK_SUSPEND_REG
static  u8 twl4030_reg_suspend[TWL4030_CACHEREGNUM] = {0};
static  u8 twl4030_reg_resume[TWL4030_CACHEREGNUM] = {0};
#endif
static struct snd_soc_device *twl4030_socdev;
static int	hsmic_bias_opened = 0;
static int  disable_headset_plug_irq = 0;

static inline unsigned int twl4030_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg);
static int twl4030_write(struct snd_soc_codec *codec,
			unsigned int reg, unsigned int value);
static int calling_speaker(struct snd_soc_codec *codec);
static int calling_headset(struct snd_soc_codec *codec);
static int dump_twl4030_regs(void);
static int twl4030_gpio2_config(void);
static int twl4030_gpio1_config(void);
static int twl4030_gpio6_config(void);
extern int spk_unmute;
extern int ear_unmute;
#ifdef CONFIG_SWITCH_GPIO
extern int get_headset_plug_irq_number(void);
extern int get_headset_plug_status(void);
extern int  IsEarPhoneInsert(void);
#endif

#ifdef CONFIG_SOUND_ALC108
extern int alc108_enable_earphone(int flag);
extern int get_alc108_earphone();
extern int alc108_enable_speaker(int flag);
extern int get_alc108_speaker();
#endif

#ifdef CONFIG_SOUND_WM9093
extern int wm9093_enable_earphone(int flag);
extern int get_wm9093_earphone();
extern int wm9093_enable_speaker(int flag);
extern int get_wm9093_speaker();
#endif

#define DISABLE_HEADSET_PLUG_IRQ			0
#define ENABLE_HEADSET_PLUG_IRQ				1

struct substream_item {
	struct list_head started;
	struct list_head configured;
	struct snd_pcm_substream *substream;
	int use256FS;
};

/* codec private data */
struct twl4030_priv {
	struct mutex mutex;

	unsigned int extClock;
	unsigned int bypass_state;
	unsigned int codec_powered;
	unsigned int codec_muted;

	struct list_head started_list;
	struct list_head config_list;

	unsigned int sysclk;

	/* Headset output state handling */
	unsigned int hsl_enabled;
	unsigned int hsr_enabled;

	struct snd_pcm_hw_params params;
};

/*
 * read twl4030 register cache
 */
static inline unsigned int twl4030_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u8 *cache = codec->reg_cache;

	if (reg >= TWL4030_CACHEREGNUM)
		return -EIO;

	return cache[reg];
}

/*
 * write twl4030 register cache
 */
static inline void twl4030_write_reg_cache(struct snd_soc_codec *codec,
						u8 reg, u8 value)
{
	u8 *cache = codec->reg_cache;

	if (reg >= TWL4030_CACHEREGNUM)
		return;

	cache[reg] = value;
}

/*
 * write to the twl4030 register space
 */
static int twl4030_write(struct snd_soc_codec *codec,
			unsigned int reg, unsigned int value)
{
	twl4030_write_reg_cache(codec, reg, value);
	if (likely(reg < TWL4030_REG_SW_SHADOW))
		return twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, value,
					    reg);
	else
		return 0;
}

#if 1
static int headset_det_gpio_suspend(void)
{
	u8 data;

	twl_i2c_read_u8(TWL4030_MODULE_GPIO, &data, 0x1c);
	data |= (1<<2);
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, data, 0x1c);

	twl_i2c_read_u8(TWL4030_MODULE_GPIO, &data, 0x22);
	data |= (1<<2);
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, data, 0x22);
	return 0;
}


static int headset_det_gpio_resume(void)
{
	u8 data;
	twl_i2c_read_u8(TWL4030_MODULE_GPIO, &data, 0x1c);
	data &= (~(1<<2));
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, data, 0x1c);

	twl_i2c_read_u8(TWL4030_MODULE_GPIO, &data, 0x22);
	data &= (~(1<<2));
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, data, 0x22);
	return 0;
}

static int enable_headset_plug_irq(int enable)
{
	int headset_plug_irq = 0;

    #ifdef CONFIG_SWITCH_GPIO
	headset_plug_irq = get_headset_plug_irq_number();
	#endif

	if(headset_plug_irq)
	{
		if(enable == DISABLE_HEADSET_PLUG_IRQ && disable_headset_plug_irq == 0)
		{
	//		printk("disable headset_plug_irq = %d in %s func\n",headset_plug_irq,__FUNCTION__);
			//modified when calling this irq always jumping, by guotao, 2010-09-02
			headset_det_gpio_suspend();
	//		disable_irq(headset_plug_irq);
			disable_headset_plug_irq = 1;
		}
		else if(disable_headset_plug_irq == 1)
		{
	//		printk("enable headset_plug_irq = %d in %s func\n",headset_plug_irq,__FUNCTION__);
			//modified when calling this irq always jumping, by guotao, 2010-09-02
			headset_det_gpio_resume();
	//		enable_irq(headset_plug_irq);
			disable_headset_plug_irq = 0;
		}
	}
	else
		yl_debug("headset_plug_irq is zero in %s func\n",__FUNCTION__);

	return 0;
}
#endif

 char stream_in_playing=0;
 char stream_in_recording=0;
 char stream_in_a2dp=0;

static void twl4030_codec_enable(struct snd_soc_codec *codec, int enable)
{
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 mode;

    //通话中禁止关codec
    if(!enable && (smartphone_calling_enable == 1))
    {
        return;
    }
    
    //通话中禁止关codec
    if(!enable && (stream_in_playing == 1))
    {
        return;
    }
	
	if (enable == twl4030->codec_powered)
		return;

	mode = twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE);
	if (enable)
	{
		mode |= TWL4030_CODECPDZ;
		twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
	}
	else
	{
	    enable_headset_plug_irq(DISABLE_HEADSET_PLUG_IRQ);
		mode &= ~TWL4030_CODECPDZ;
		if(ear_unmute)
		{
			
			wm9093_enable_earphone(0);
			mdelay(60);
			twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
			wm9093_enable_earphone(1);
		}
		if(spk_unmute)
		{
			
			wm9093_enable_speaker(0);
			mdelay(60);
			twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
			wm9093_enable_speaker(1);
		}
		
		if(!ear_unmute && !spk_unmute) twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
	}

	
	twl4030->codec_powered = enable;

	
	if (enable)
	{	
	    //mdelay(200);
		enable_headset_plug_irq(ENABLE_HEADSET_PLUG_IRQ);
	}
		
	/* REVISIT: this delay is present in TI sample drivers */
	/* but there seems to be no TRM requirement for it     */
	udelay(10);
}

static void twl4030_init_chip(struct snd_soc_codec *codec)
{
	u8 *cache = codec->reg_cache;
	int i;

	/* clear CODECPDZ prior to setting register defaults */
	twl4030_codec_enable(codec, 0);

	/* set all audio section registers to reasonable defaults */
	for (i = TWL4030_REG_OPTION; i <= TWL4030_REG_MISC_SET_2; i++)
		twl4030_write(codec, i,	cache[i]);

}

static void twl4030_codec_mute(struct snd_soc_codec *codec, int mute)
{
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 reg_val;
	
    //通话中禁止静音
    if(mute && smartphone_calling_enable)
    {
        return;
    }

	if (mute == twl4030->codec_muted)
		return;

	if (mute) {
		/* Bypass the reg_cache and mute the volumes
		 * Headset mute is done in it's own event handler
		 * Things to mute:  Earpiece, PreDrivL/R, CarkitL/R
		 */
		 
		#if 0 
		reg_val = twl4030_read_reg_cache(codec, TWL4030_REG_EAR_CTL);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
					reg_val & (~TWL4030_EAR_GAIN),
					TWL4030_REG_EAR_CTL);

		#endif
		
		reg_val = twl4030_read_reg_cache(codec, TWL4030_REG_PREDL_CTL);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
					reg_val & (~TWL4030_PREDL_GAIN),
					TWL4030_REG_PREDL_CTL);
		reg_val = twl4030_read_reg_cache(codec, TWL4030_REG_PREDR_CTL);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
					reg_val & (~TWL4030_PREDR_GAIN),
					TWL4030_REG_PREDR_CTL);	  //modified by guotao			//modified by guotao, 2010-01-18

		reg_val = twl4030_read_reg_cache(codec, TWL4030_REG_PRECKL_CTL);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
					reg_val & (~TWL4030_PRECKL_GAIN),
					TWL4030_REG_PRECKL_CTL);
		reg_val = twl4030_read_reg_cache(codec, TWL4030_REG_PRECKR_CTL);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
					reg_val & (~TWL4030_PRECKR_GAIN),
					TWL4030_REG_PRECKR_CTL);

		yl_debug("before mute TWL4030_REG_APLL_CTL in %s func\n",__FUNCTION__);
#ifdef NEED_DISABLE_PLL
		/* Disable PLL */
		reg_val = twl4030_read_reg_cache(codec, TWL4030_REG_APLL_CTL);
		reg_val &= ~TWL4030_APLL_EN;
		twl4030_write(codec, TWL4030_REG_APLL_CTL, reg_val);
#endif
	} else {
		/* Restore the volumes
		 * Headset mute is done in it's own event handler
		 * Things to restore:  Earpiece, PreDrivL/R, CarkitL/R
		 */
		twl4030_write(codec, TWL4030_REG_EAR_CTL,
			twl4030_read_reg_cache(codec, TWL4030_REG_EAR_CTL));

		twl4030_write(codec, TWL4030_REG_PREDL_CTL,
			twl4030_read_reg_cache(codec, TWL4030_REG_PREDL_CTL));
		twl4030_write(codec, TWL4030_REG_PREDR_CTL,
			twl4030_read_reg_cache(codec, TWL4030_REG_PREDR_CTL));

		twl4030_write(codec, TWL4030_REG_PRECKL_CTL,
			twl4030_read_reg_cache(codec, TWL4030_REG_PRECKL_CTL));
		twl4030_write(codec, TWL4030_REG_PRECKR_CTL,
			twl4030_read_reg_cache(codec, TWL4030_REG_PRECKR_CTL));

		yl_debug("before un-mute TWL4030_REG_APLL_CTL in %s func\n",__FUNCTION__);
#ifdef NEED_DISABLE_PLL		
		/* Enable PLL */
		reg_val = twl4030_read_reg_cache(codec, TWL4030_REG_APLL_CTL);
		reg_val |= TWL4030_APLL_EN;
		twl4030_write(codec, TWL4030_REG_APLL_CTL, reg_val);
#endif
	}

	twl4030->codec_muted = mute;
}

static void twl4030_power_up(struct snd_soc_codec *codec)
{
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 anamicl, regmisc1, byte = 0;
	int i = 0;

	if (twl4030->codec_powered)
		return;

	/* set CODECPDZ to turn on codec */
	twl4030_codec_enable(codec, 1);

	/* initiate offset cancellation */
	anamicl = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	twl4030_write(codec, TWL4030_REG_ANAMICL,
		anamicl | TWL4030_CNCL_OFFSET_START);

	/* wait for offset cancellation to complete */
	do {
		/* this takes a little while, so don't slam i2c */
		udelay(2000);
		twl_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &byte,
				    TWL4030_REG_ANAMICL);
	} while ((i++ < 100) &&
		 ((byte & TWL4030_CNCL_OFFSET_START) ==
		  TWL4030_CNCL_OFFSET_START));

	/* Make sure that the reg_cache has the same value as the HW */
	twl4030_write_reg_cache(codec, TWL4030_REG_ANAMICL, byte);

	/* anti-pop when changing analog gain */
	regmisc1 = twl4030_read_reg_cache(codec, TWL4030_REG_MISC_SET_1);
	twl4030_write(codec, TWL4030_REG_MISC_SET_1,
		regmisc1 | TWL4030_SMOOTH_ANAVOL_EN);

	/* toggle CODECPDZ as per TRM */
	twl4030_codec_enable(codec, 0);
	twl4030_codec_enable(codec, 1);
}

/*
 * Unconditional power down
 */
static void twl4030_power_down(struct snd_soc_codec *codec)
{
	/* power down */
	twl4030_codec_enable(codec, 0);
}

/* Earpiece */
static const struct snd_kcontrol_new twl4030_dapm_earpiece_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_EAR_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_EAR_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_EAR_CTL, 2, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_EAR_CTL, 3, 1, 0),
};

/* PreDrive Left */
static const struct snd_kcontrol_new twl4030_dapm_predrivel_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PREDL_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_PREDL_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_PREDL_CTL, 2, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_PREDL_CTL, 3, 1, 0),
};

/* PreDrive Right */
static const struct snd_kcontrol_new twl4030_dapm_predriver_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PREDR_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_PREDR_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_PREDR_CTL, 2, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_PREDR_CTL, 3, 1, 0),
};

/* Headset Left */
static const struct snd_kcontrol_new twl4030_dapm_hsol_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_HS_SEL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_HS_SEL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_HS_SEL, 2, 1, 0),
};

/* Headset Right */
static const struct snd_kcontrol_new twl4030_dapm_hsor_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_HS_SEL, 3, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_HS_SEL, 4, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_HS_SEL, 5, 1, 0),
};

/* Carkit Left */
static const struct snd_kcontrol_new twl4030_dapm_carkitl_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PRECKL_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_PRECKL_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_PRECKL_CTL, 2, 1, 0),
};

/* Carkit Right */
static const struct snd_kcontrol_new twl4030_dapm_carkitr_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PRECKR_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_PRECKR_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_PRECKR_CTL, 2, 1, 0),
};

/* Handsfree Left */
static const char *twl4030_handsfreel_texts[] =
		{"Voice", "AudioL1", "AudioL2", "AudioR2"};

static const struct soc_enum twl4030_handsfreel_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_HFL_CTL, 0,
			ARRAY_SIZE(twl4030_handsfreel_texts),
			twl4030_handsfreel_texts);

static const struct snd_kcontrol_new twl4030_dapm_handsfreel_control =
SOC_DAPM_ENUM("Route", twl4030_handsfreel_enum);

/* Handsfree Left virtual mute */
static const struct snd_kcontrol_new twl4030_dapm_handsfreelmute_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_SW_SHADOW, 0, 1, 0);

/* Handsfree Right */
static const char *twl4030_handsfreer_texts[] =
		{"Voice", "AudioR1", "AudioR2", "AudioL2"};

static const struct soc_enum twl4030_handsfreer_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_HFR_CTL, 0,
			ARRAY_SIZE(twl4030_handsfreer_texts),
			twl4030_handsfreer_texts);

static const struct snd_kcontrol_new twl4030_dapm_handsfreer_control =
SOC_DAPM_ENUM("Route", twl4030_handsfreer_enum);

/* Handsfree Right virtual mute */
static const struct snd_kcontrol_new twl4030_dapm_handsfreermute_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_SW_SHADOW, 1, 1, 0);

/* Vibra */
/* Vibra audio path selection */
static const char *twl4030_vibra_texts[] =
		{"AudioL1", "AudioR1", "AudioL2", "AudioR2"};

static const struct soc_enum twl4030_vibra_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 2,
			ARRAY_SIZE(twl4030_vibra_texts),
			twl4030_vibra_texts);

static const struct snd_kcontrol_new twl4030_dapm_vibra_control =
SOC_DAPM_ENUM("Route", twl4030_vibra_enum);

/* Vibra path selection: local vibrator (PWM) or audio driven */
static const char *twl4030_vibrapath_texts[] =
		{"Local vibrator", "Audio"};

static const struct soc_enum twl4030_vibrapath_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 4,
			ARRAY_SIZE(twl4030_vibrapath_texts),
			twl4030_vibrapath_texts);

static const struct snd_kcontrol_new twl4030_dapm_vibrapath_control =
SOC_DAPM_ENUM("Route", twl4030_vibrapath_enum);

/* Left analog microphone selection */
static const struct snd_kcontrol_new twl4030_dapm_analoglmic_controls[] = {
	SOC_DAPM_SINGLE("Main Mic Capture Switch",
			TWL4030_REG_ANAMICL, 0, 1, 0),
	SOC_DAPM_SINGLE("Headset Mic Capture Switch",
			TWL4030_REG_ANAMICL, 1, 1, 0),
	SOC_DAPM_SINGLE("AUXL Capture Switch",
			TWL4030_REG_ANAMICL, 2, 1, 0),
	SOC_DAPM_SINGLE("Carkit Mic Capture Switch",
			TWL4030_REG_ANAMICL, 3, 1, 0),
};

/* Right analog microphone selection */
static const struct snd_kcontrol_new twl4030_dapm_analogrmic_controls[] = {
	SOC_DAPM_SINGLE("Sub Mic Capture Switch", TWL4030_REG_ANAMICR, 0, 1, 0),
	SOC_DAPM_SINGLE("AUXR Capture Switch", TWL4030_REG_ANAMICR, 2, 1, 0),
};

/* TX1 L/R Analog/Digital microphone selection */
static const char *twl4030_micpathtx1_texts[] =
		{"Analog", "Digimic0"};

static const struct soc_enum twl4030_micpathtx1_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_ADCMICSEL, 0,
			ARRAY_SIZE(twl4030_micpathtx1_texts),
			twl4030_micpathtx1_texts);

static const struct snd_kcontrol_new twl4030_dapm_micpathtx1_control =
SOC_DAPM_ENUM("Route", twl4030_micpathtx1_enum);

/* TX2 L/R Analog/Digital microphone selection */
static const char *twl4030_micpathtx2_texts[] =
		{"Analog", "Digimic1"};

static const struct soc_enum twl4030_micpathtx2_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_ADCMICSEL, 2,
			ARRAY_SIZE(twl4030_micpathtx2_texts),
			twl4030_micpathtx2_texts);

static const struct snd_kcontrol_new twl4030_dapm_micpathtx2_control =
SOC_DAPM_ENUM("Route", twl4030_micpathtx2_enum);

/* Analog bypass for AudioR1 */
static const struct snd_kcontrol_new twl4030_dapm_abypassr1_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXR1_APGA_CTL, 2, 1, 0);

/* Analog bypass for AudioL1 */
static const struct snd_kcontrol_new twl4030_dapm_abypassl1_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXL1_APGA_CTL, 2, 1, 0);

/* Analog bypass for AudioR2 */
static const struct snd_kcontrol_new twl4030_dapm_abypassr2_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXR2_APGA_CTL, 2, 1, 0);

/* Analog bypass for AudioL2 */
static const struct snd_kcontrol_new twl4030_dapm_abypassl2_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXL2_APGA_CTL, 2, 1, 0);

/* Analog bypass for Voice */
static const struct snd_kcontrol_new twl4030_dapm_abypassv_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_VDL_APGA_CTL, 2, 1, 0);

/* Digital bypass gain, 0 mutes the bypass */
static const unsigned int twl4030_dapm_dbypass_tlv[] = {
	TLV_DB_RANGE_HEAD(2),
	0, 3, TLV_DB_SCALE_ITEM(-2400, 0, 1),
	4, 7, TLV_DB_SCALE_ITEM(-1800, 600, 0),
};

/* Digital bypass left (TX1L -> RX2L) */
static const struct snd_kcontrol_new twl4030_dapm_dbypassl_control =
	SOC_DAPM_SINGLE_TLV("Volume",
			TWL4030_REG_ATX2ARXPGA, 3, 7, 0,
			twl4030_dapm_dbypass_tlv);

/* Digital bypass right (TX1R -> RX2R) */
static const struct snd_kcontrol_new twl4030_dapm_dbypassr_control =
	SOC_DAPM_SINGLE_TLV("Volume",
			TWL4030_REG_ATX2ARXPGA, 0, 7, 0,
			twl4030_dapm_dbypass_tlv);

/*
 * Voice Sidetone GAIN volume control:
 * from -51 to -10 dB in 1 dB steps (mute instead of -51 dB)
 */
static DECLARE_TLV_DB_SCALE(twl4030_dapm_dbypassv_tlv, -5100, 100, 1);

/* Digital bypass voice: sidetone (VUL -> VDL)*/
static const struct snd_kcontrol_new twl4030_dapm_dbypassv_control =
	SOC_DAPM_SINGLE_TLV("Volume",
			TWL4030_REG_VSTPGA, 0, 0x29, 0,
			twl4030_dapm_dbypassv_tlv);

static int micpath_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct soc_enum *e = (struct soc_enum *)w->kcontrols->private_value;
	unsigned char adcmicsel, micbias_ctl;

	adcmicsel = twl4030_read_reg_cache(w->codec, TWL4030_REG_ADCMICSEL);
	micbias_ctl = twl4030_read_reg_cache(w->codec, TWL4030_REG_MICBIAS_CTL);
	/* Prepare the bits for the given TX path:
	 * shift_l == 0: TX1 microphone path
	 * shift_l == 2: TX2 microphone path */
	if (e->shift_l) {
		/* TX2 microphone path */
		if (adcmicsel & TWL4030_TX2IN_SEL)
			micbias_ctl |= TWL4030_MICBIAS2_CTL; /* digimic */
		else
			micbias_ctl &= ~TWL4030_MICBIAS2_CTL;
	} else {
		/* TX1 microphone path */
		if (adcmicsel & TWL4030_TX1IN_SEL)
			micbias_ctl |= TWL4030_MICBIAS1_CTL; /* digimic */
		else
			micbias_ctl &= ~TWL4030_MICBIAS1_CTL;
	}

	twl4030_write(w->codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);
	return 0;
}

/*
 * Output PGA builder:
 * Handle the muting and unmuting of the given output (turning off the
 * amplifier associated with the output pin)
 * On mute bypass the reg_cache and mute the volume
 * On unmute: restore the register content
 * Outputs handled in this way:  Earpiece, PreDrivL/R, CarkitL/R
 */
#define TWL4030_OUTPUT_PGA(pin_name, reg, mask)				\
static int pin_name##pga_event(struct snd_soc_dapm_widget *w,		\
		struct snd_kcontrol *kcontrol, int event)		\
{									\
	u8 reg_val;							\
									\
	switch (event) {						\
	case SND_SOC_DAPM_POST_PMU:					\
		twl4030_write(w->codec, reg,				\
			twl4030_read_reg_cache(w->codec, reg));		\
		break;							\
	case SND_SOC_DAPM_POST_PMD:					\
		reg_val = twl4030_read_reg_cache(w->codec, reg);	\
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,	\
					reg_val & (~mask),		\
					reg);				\
		break;							\
	}								\
	return 0;							\
}

TWL4030_OUTPUT_PGA(earpiece, TWL4030_REG_EAR_CTL, TWL4030_EAR_GAIN);
TWL4030_OUTPUT_PGA(predrivel, TWL4030_REG_PREDL_CTL, TWL4030_PREDL_GAIN);
TWL4030_OUTPUT_PGA(predriver, TWL4030_REG_PREDR_CTL, TWL4030_PREDR_GAIN);
TWL4030_OUTPUT_PGA(carkitl, TWL4030_REG_PRECKL_CTL, TWL4030_PRECKL_GAIN);
TWL4030_OUTPUT_PGA(carkitr, TWL4030_REG_PRECKR_CTL, TWL4030_PRECKR_GAIN);

static void handsfree_ramp(struct snd_soc_codec *codec, int reg, int ramp)
{
	unsigned char hs_ctl;

	hs_ctl = twl4030_read_reg_cache(codec, reg);

	if (ramp) {
		/* HF ramp-up */
		hs_ctl |= TWL4030_HF_CTL_REF_EN;
		twl4030_write(codec, reg, hs_ctl);
		udelay(10);
		hs_ctl |= TWL4030_HF_CTL_RAMP_EN;
		twl4030_write(codec, reg, hs_ctl);
		udelay(40);
		hs_ctl |= TWL4030_HF_CTL_LOOP_EN;
		hs_ctl |= TWL4030_HF_CTL_HB_EN;
		twl4030_write(codec, reg, hs_ctl);
	} else {
		/* HF ramp-down */
		hs_ctl &= ~TWL4030_HF_CTL_LOOP_EN;
		hs_ctl &= ~TWL4030_HF_CTL_HB_EN;
		twl4030_write(codec, reg, hs_ctl);
		hs_ctl &= ~TWL4030_HF_CTL_RAMP_EN;
		twl4030_write(codec, reg, hs_ctl);
		udelay(40);
		hs_ctl &= ~TWL4030_HF_CTL_REF_EN;
		twl4030_write(codec, reg, hs_ctl);
	}
}

static int handsfreelpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		handsfree_ramp(w->codec, TWL4030_REG_HFL_CTL, 1);
		break;
	case SND_SOC_DAPM_POST_PMD:
		handsfree_ramp(w->codec, TWL4030_REG_HFL_CTL, 0);
		break;
	}
	return 0;
}

static int handsfreerpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		handsfree_ramp(w->codec, TWL4030_REG_HFR_CTL, 1);
		break;
	case SND_SOC_DAPM_POST_PMD:
		handsfree_ramp(w->codec, TWL4030_REG_HFR_CTL, 0);
		break;
	}
	return 0;
}

#ifdef TEST_PCM_PHONE
static int local_mic_record(struct snd_soc_codec *codec)
{
	unsigned char vdl_apga, anamic_val;
			
	//modified by guotao, 2010-04-02, reg 0x05
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	anamic_val = 0x11;		//enable MAINMIC_EN, enable MICAMPL_EN
	twl4030_write(codec, TWL4030_REG_ANAMICL, anamic_val);

	//modified by guotao, 2010-03-30, reg 0x17
	vdl_apga = twl4030_read_reg_cache(codec, TWL4030_REG_AVDAC_CTL);
	vdl_apga = 0x00;
	twl4030_write(codec, TWL4030_REG_AVDAC_CTL, vdl_apga);
	
	//modified by guotao, 2010-03-30, reg 0x48
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMIC_GAIN);
	anamic_val = 0x2d;		//adjust mic gain
	twl4030_write(codec, TWL4030_REG_ANAMIC_GAIN, anamic_val);
	
	return 0;
}

static int pcm_calling_common_regs(struct snd_soc_codec *codec)
{
	unsigned char micbias_ctl, anamic_val;
	unsigned char vdl_apga;
	unsigned char codec_mode;
	unsigned char vif_value,reg_value;

	if(codec == NULL)
	{
		printk("codec is null in %s func\n",__FUNCTION__);
		return -1;
	}

//	gpio_set_value(GPIO_2,1);			//disable PCM transfer channel

	//modified by guotao, 2010-03-25, reg 0x02
	codec_mode = twl4030_read_reg_cache(codec, TWL4030_REG_OPTION);		
	codec_mode |= 0x10;		//enable ARXL1_VRX_EN;
	twl4030_write(codec, TWL4030_REG_OPTION, codec_mode);

	//modified by guotao, 2010-03-25, reg 0x0f
	vif_value = twl4030_read_reg_cache(codec, TWL4030_REG_VOICE_IF);
	vif_value = 0xe1;
	twl4030_write(codec, TWL4030_REG_VOICE_IF, vif_value);

	//modified by guotao, 2010-04-02, reg 0x04
	micbias_ctl = twl4030_read_reg_cache(codec, TWL4030_REG_MICBIAS_CTL);
	micbias_ctl &=  (~TWL4030_MICBIAS1_EN);
	micbias_ctl |=  TWL4030_MICBIAS2_EN;
	twl4030_write(codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);

	//modified by guotao, 2010-04-02, reg 0x05
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	anamic_val = 0x11;		//enable MAINMIC_EN, enable MICAMPL_EN
	twl4030_write(codec, TWL4030_REG_ANAMICL, anamic_val);

	//modified by guotao, 2010-04-02, reg 0x07
	reg_value = twl4030_read_reg_cache(codec, TWL4030_REG_AVADC_CTL);
	reg_value |= 0x02;
	twl4030_write(codec, TWL4030_REG_AVADC_CTL, reg_value);

	//modified by guotao, 2010-03-25, reg 0x14
	vdl_apga = twl4030_read_reg_cache(codec, TWL4030_REG_VRXPGA);
	vdl_apga = 0x25;
	twl4030_write(codec, TWL4030_REG_VRXPGA, vdl_apga);

	//modified by guotao, 2010-03-30, reg 0x17
	vdl_apga = twl4030_read_reg_cache(codec, TWL4030_REG_AVDAC_CTL);
	vdl_apga |= 0x10;
	twl4030_write(codec, TWL4030_REG_AVDAC_CTL, vdl_apga);

	//modified by guotao, 2010-03-25, reg 0x44
	vdl_apga = twl4030_read_reg_cache(codec, TWL4030_REG_VDL_APGA_CTL);
	vdl_apga = 0x53;		//enable VDL_DA_EN, VDL_PDZ //Modified 73-->43 volume 73=e=-16db 53=a=-8db by sunruichen 20110601
	twl4030_write(codec, TWL4030_REG_VDL_APGA_CTL, vdl_apga);
 
	//modified by guotao, 2010-03-25, reg 0x3f
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_PCMBTMUX);
	anamic_val = 0x00;
	twl4030_write(codec, TWL4030_REG_PCMBTMUX, anamic_val);

//	gpio_set_value(GPIO_2,0);			//enable PCM transfer channel

	return 0;
}

static int pcm_record_calling_regs(struct snd_soc_codec *codec)
{
	unsigned char reg_value = 0;

	if(codec == NULL)
	{
		printk("codec is null in %s func\n",__FUNCTION__);
		return -1;
	}

	//modified by guotao, 2010-04-02, reg 0x05
	reg_value = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	reg_value = 0x11;		
	twl4030_write(codec, TWL4030_REG_ANAMICL, reg_value);

	//modified by guotao, 2010-04-02, reg 0x06
	reg_value = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICR);
	reg_value = 0x11;		
	twl4030_write(codec, TWL4030_REG_ANAMICR, reg_value);

	//modified by guotao, 2010-01-18, reg 0x44
	reg_value = twl4030_read_reg_cache(codec, TWL4030_REG_VDL_APGA_CTL);
	reg_value |= 0x04;	//enable VDL_FM_EN
	twl4030_write(codec, TWL4030_REG_VDL_APGA_CTL, reg_value);

	//modified by guotao, 2010-03-25, reg 0x3e
	reg_value = twl4030_read_reg_cache(codec, TWL4030_REG_MISC_SET_1);
	reg_value |= 0x20;		//enable FMLOOP_EN
	twl4030_write(codec, TWL4030_REG_MISC_SET_1, reg_value);

	return 0;
}
#endif

static int adjust_mic_db_regs(struct snd_soc_codec *codec, int enable)
{
	u8 *cache = codec->reg_cache;

	if(enable)
	{
		//modified by guotao, 2010-08-02, reg 0x2c
		//	0X2C    ALC_MAX_LIMIT       0X0     ( -9  dB)  
		//          ALC_MIN_LIMIT     	0X4    	( -24 dB)  
		twl4030_write(codec, TWL4030_REG_ALC_SET1, cache[TWL4030_REG_ALC_SET1] & 0xf8 | 0x04);

		twl4030_write(codec, TWL4030_REG_VDL_APGA_CTL, cache[TWL4030_REG_VDL_APGA_CTL] | 0x05);//enable VDL_FM_EN, VDL_PDZ,
	}
	else
	{
		twl4030_write(codec, TWL4030_REG_ALC_SET1, cache[TWL4030_REG_ALC_SET1] & 0xf8);

		twl4030_write(codec, TWL4030_REG_VDL_APGA_CTL, cache[TWL4030_REG_VDL_APGA_CTL] & (~0x05));//clear VDL_FM_EN, VDL_PDZ,
	}
	return 0;
}

static int calling_end(struct snd_soc_codec *codec)
{
	unsigned char anamic_val;
	unsigned char hs_sel;
	unsigned char ear_ctl;
	unsigned char pred_value;
	u8 *cache = codec->reg_cache;

	printk("enter calling end!\n");
	if(codec==NULL)
	{
		yl_debug("codec is NULL in %s func,failure to exit\n",__FUNCTION__);
		return -EINVAL;
	}

	twl4030_write(codec, TWL4030_REG_MISC_SET_1, cache[TWL4030_REG_MISC_SET_1]&(~0x20));

	adjust_mic_db_regs(codec,0);

	twl4030_write(codec, TWL4030_REG_HS_SEL, cache[TWL4030_REG_HS_SEL]&(~0x09));

	twl4030_write(codec, TWL4030_REG_PREDL_CTL, cache[TWL4030_REG_PREDL_CTL]&(~0x01));
	
	twl4030_write(codec, TWL4030_REG_PREDR_CTL, cache[TWL4030_REG_PREDR_CTL]&(~0x01));

	twl4030_write(codec, TWL4030_REG_EAR_CTL, cache[TWL4030_REG_EAR_CTL]&(~0x01));

	return 0;
}

static int calling_earpiece(struct snd_soc_codec *codec)
{
	unsigned char anamic_val;
	unsigned char hs_sel;
	unsigned char ear_ctl;
	unsigned char pred_value;


	if(codec==NULL)
	{
		yl_debug("codec is NULL in %s func,failure to exit\n",__FUNCTION__);
		return -EINVAL;
	}

	//modified by guotao, 2010-01-22, reg 0x3E
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_MISC_SET_1);
	anamic_val = 0x22;		//enable FMLOOP_EN
	twl4030_write(codec, TWL4030_REG_MISC_SET_1, anamic_val);

	adjust_mic_db_regs(codec,1);


	//modified by guotao, 2010-01-28, reg 0x22
	hs_sel = twl4030_read_reg_cache(codec, TWL4030_REG_HS_SEL);
	hs_sel &= (~0x09);		//disable HSOR_VOICE_EN, HSOL_VOICE_EN, HSOR_AR2_EN, HSOL_AL2_EN
	twl4030_write(codec, TWL4030_REG_HS_SEL, hs_sel);

	//fengchunsong
	pred_value = twl4030_read_reg_cache(codec, TWL4030_REG_PREDL_CTL);
	pred_value &= (~0x01);		//disable PREDL_VOICE_EN
	twl4030_write(codec, TWL4030_REG_PREDL_CTL, pred_value);
	
	//modified by guotao, 2010-01-18
	pred_value = twl4030_read_reg_cache(codec, TWL4030_REG_PREDR_CTL);
	pred_value &= (~0x01);		//disable PREDR_VOICE_EN
	twl4030_write(codec, TWL4030_REG_PREDR_CTL, pred_value);

	//modified by guotao, 2010-01-18, reg 0x21 
	//for CP9130_T0_BOARD
	ear_ctl = twl4030_read_reg_cache(codec, TWL4030_REG_EAR_CTL);
	ear_ctl |= 0x35;		//enable EAR_VOICE_EN
	twl4030_write(codec, TWL4030_REG_EAR_CTL, ear_ctl);

	return 0;
}


static int calling_speaker(struct snd_soc_codec *codec)
{
	unsigned char anamic_val;
	unsigned char hs_sel;
	unsigned char pred_value;


	if(codec==NULL)
	{
		yl_debug("codec is NULL in %s func,failure to exit\n",__FUNCTION__);
		return -EINVAL;
	}

#ifdef TEST_PCM_PHONE
	pcm_calling_common_regs(codec);
	pcm_record_calling_regs(codec);
#else


#ifdef M1_P1_BOARD	
	//modified by guotao, 2010-01-14, reg 0x04
	micbias_ctl = twl4030_read_reg_cache(codec, TWL4030_REG_MICBIAS_CTL);
	micbias_ctl |=  TWL4030_MICBIAS1_EN;
	micbias_ctl &=  (~TWL4030_HSMICBIAS_EN);
	twl4030_write(codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);

	//modified by guotao, 2010-01-16, reg 0x05
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	anamic_val |= 0x11;		//enable MAINMIC_EN, enable MICAMPL_EN
	twl4030_write(codec, TWL4030_REG_ANAMICL, anamic_val);
#else
	//nothing to do, input path config in alsa_omap3.cpp
	//for CP9130 T0 board
#endif

	//modified by guotao, 2010-01-22, reg 0x3E
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_MISC_SET_1);
	anamic_val = 0x22;		//enable FMLOOP_EN
	twl4030_write(codec, TWL4030_REG_MISC_SET_1, anamic_val);

	adjust_mic_db_regs(codec,1);

#endif		//end of TEST_PCM_PHONE

	//modified by guotao, 2010-01-28, reg 0x22
	hs_sel = twl4030_read_reg_cache(codec, TWL4030_REG_HS_SEL);
	hs_sel &= (~0x09);		//disable HSOR_VOICE_EN, HSOL_VOICE_EN, HSOR_AR2_EN, HSOL_AL2_EN
	twl4030_write(codec, TWL4030_REG_HS_SEL, hs_sel);

	//modified by guotao, 2010-01-18, reg 0x25 
	pred_value = twl4030_read_reg_cache(codec, TWL4030_REG_PREDL_CTL);
	//fengchunsong change
	pred_value |= 0x01;		//enable PREDL_VOICE_EN
	//pred_value = 0x11;		//enable PREDL_VOICE_EN
	twl4030_write(codec, TWL4030_REG_PREDL_CTL, pred_value);
	
	//modified by guotao, 2010-01-18, reg 0x26
	pred_value = twl4030_read_reg_cache(codec, TWL4030_REG_PREDR_CTL);
	pred_value |= 0x01;		//enable PREDR_VOICE_EN
	//pred_value = 0x11;		//enable PREDL_VOICE_EN
	twl4030_write(codec, TWL4030_REG_PREDR_CTL, pred_value);
	
	return 0;
}


static int calling_headset(struct snd_soc_codec *codec)
{
	unsigned char anamic_val;
	unsigned char hs_sel,hs_pop,hs_gain;
	unsigned char pred_value;

	
	if(codec==NULL)
	{
		yl_debug("codec is NULL in %s func,failure to exit\n",__FUNCTION__);
		return -EINVAL;
	}

#ifdef TEST_PCM_PHONE
	pcm_calling_common_regs(codec);
	pcm_record_calling_regs(codec);
#else

#ifdef M1_P1_BOARD	
	//modified by guotao, 2010-01-16, reg 0x05
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	anamic_val = 0x11;		//enable MAINMIC_EN, enable MICAMPL_EN
	twl4030_write(codec, TWL4030_REG_ANAMICL, anamic_val);
#else
	//nothing to do, input path config in alsa_omap3.cpp
	//for CP9130 T0 board
#endif


	//modified by guotao, 2010-01-22, reg 0x3E
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_MISC_SET_1);
	anamic_val = 0x22;		//enable FMLOOP_EN
	twl4030_write(codec, TWL4030_REG_MISC_SET_1, anamic_val);

	adjust_mic_db_regs(codec,1);

#endif	//end of TEST_PCM_PHONE

	//modified by guotao, 2010-04-15, reg 0x21, for earpiece
	hs_sel = twl4030_read_reg_cache(codec, TWL4030_REG_EAR_CTL);
	hs_sel &= 0xfe;		//0x01, enable EAR_VOICE_EN
	twl4030_write(codec, TWL4030_REG_EAR_CTL, hs_sel);// 1--->0 ?

	//modified by guotao, 2010-01-28, reg 0x22, for headset
	hs_sel = twl4030_read_reg_cache(codec, TWL4030_REG_HS_SEL);
	hs_sel |= 0x09;		//0x09, enable HSOR_VOICE_EN, HSOL_VOICE_EN, HSOR_AR2_EN, HSOL_AL2_EN
	twl4030_write(codec, TWL4030_REG_HS_SEL, hs_sel);

	hs_pop = twl4030_read_reg_cache(codec, TWL4030_REG_HS_POPN_SET);

#if 1    //modified by guotao, 2010-04-27
	/* Headset ramp-up according to the TRM */
	if(hs_pop & TWL4030_RAMP_EN)
		;
	else
#endif
	{
		hs_gain = twl4030_read_reg_cache(codec, TWL4030_REG_HS_GAIN_SET);
		/* Headset ramp-up according to the TRM */
		hs_pop |= TWL4030_VMID_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		
		twl4030_write(codec, TWL4030_REG_HS_GAIN_SET, hs_gain);
		
		hs_pop |= TWL4030_RAMP_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);

		/* Wait ramp delay time + 1, so the VMID can settle */
		mdelay(10);
	}

	//modified by guotao, 2010-01-18
	pred_value = twl4030_read_reg_cache(codec, TWL4030_REG_PREDL_CTL);
	pred_value &= (~0x01);		//disable PREDL_VOICE_EN
	twl4030_write(codec, TWL4030_REG_PREDL_CTL, pred_value);
	
	//modified by guotao, 2010-01-18
	pred_value = twl4030_read_reg_cache(codec, TWL4030_REG_PREDR_CTL);
	pred_value &= (~0x01);		//disable PREDR_VOICE_EN
	twl4030_write(codec, TWL4030_REG_PREDR_CTL, pred_value);

	return 0;
}

//modified by guotao, 2010-01-13
#define 	RXL2_SEL_EN		(1 << 5) 
#define 	RXR2_SEL_EN		(1 << 4) 
//#define 	NEED_SELECT_CALLING_PATH			1

int headset_amp_en(int enable)
{
	/* Base values for ramp delay calculation: 2^19 - 2^26 */
	unsigned int ramp_base[] = {524288, 1048576, 2097152, 4194304,
				    8388608, 16777216, 33554432, 67108864};
	unsigned char hs_gain, hs_pop;
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	static char headset_amp_state=0xff;

	if(headset_amp_state==enable)
	{
		printk("headset_amp_state == enable,do nothing\n");
		return 0;
	}
	headset_amp_state=enable;

	hs_gain = twl4030_read_reg_cache(codec, TWL4030_REG_HS_GAIN_SET);
	hs_pop = twl4030_read_reg_cache(codec, TWL4030_REG_HS_POPN_SET);

	if (enable) {

		hs_pop |= TWL4030_VMID_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);

		twl4030_write(codec, TWL4030_REG_HS_GAIN_SET, hs_gain);

		hs_pop |= TWL4030_RAMP_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		/* Wait ramp delay time + 1, so the VMID can settle */
		mdelay((ramp_base[(hs_pop & TWL4030_RAMP_DELAY) >> 2] / twl4030->sysclk) + 1);
	
		//mdelay(20);

		yl_debug("set GPIO_97 to  %d, in %s func\n",enable,__FUNCTION__);
		//Nicho add 2009-11-30
		//gpio_set_value(GPIO_97, 1);			//turn on headset amplifier
		if(cp5860e_speaker_ic == ALC108_IC)
		{
			alc108_enable_earphone(1);
		}
		else
		{
			wm9093_enable_earphone(1);
		}

	} else {
 
		yl_debug("set GPIO_97 to  %d, in %s func\n",enable,__FUNCTION__);
		
		//Nicho add 2009-11-30
		//gpio_set_value(GPIO_97, 0);			//turn off headset amplifier
		if(cp5860e_speaker_ic == ALC108_IC)
		{
			alc108_enable_earphone(0);
		}
		else
		{
			wm9093_enable_earphone(0);
		}

			//mdelay(100);

#if 1
		/* Headset ramp-down _not_ according to
		 * the TRM, but in a way that it is working */
		hs_pop &= ~TWL4030_RAMP_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		/* Wait ramp delay time + 1, so the VMID can settle */
		//mdelay((ramp_base[(hs_pop & TWL4030_RAMP_DELAY) >> 2] /
		//	twl4030->sysclk) + 1);
		/* Bypass the reg_cache to mute the headset */
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
					hs_gain & (~0x0f),
					TWL4030_REG_HS_GAIN_SET);

		hs_pop &= ~TWL4030_VMID_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
#endif

	}
	
	return 0;
}

static void headset_ramp(struct snd_soc_codec *codec, int ramp)
{
	struct snd_soc_device *socdev = codec->socdev;
	struct twl4030_setup_data *setup = socdev->codec_data;
	unsigned char hs_pop;
	unsigned char  rx_mix_val, anamic_val;//added by guotao,2010-12-27
	unsigned char micbias_ctl = 0;//added by guotao,2010-12-27
	int	earphone_insert = 0;//added by guotao,2010-12-27
	
	//modified by guotao, 2010-01-13
	rx_mix_val = twl4030_read_reg_cache(codec, TWL4030_REG_RX_PATH_SEL);
	
	//modified by guotao, 2010-01-14
	micbias_ctl = twl4030_read_reg_cache(codec, TWL4030_REG_MICBIAS_CTL);
	
	//modified by guotao, 2010-01-16
	anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	
	/* Enable external mute control, this dramatically reduces
	 * the pop-noise */
	if (setup && setup->hs_extmute) {
		if (setup->set_hs_extmute) {
			setup->set_hs_extmute(1);
		} else {
			hs_pop |= TWL4030_EXTMUTE;
			twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		}
	}

	if (ramp||(smartphone_calling_enable == 1)) {
		/* Headset ramp-up according to the TRM */
		headset_amp_en(1);

		/* Disable external mute */
		if (setup && setup->hs_extmute) {
			if (setup->set_hs_extmute) {
				setup->set_hs_extmute(0);
			} else {
				hs_pop &= ~TWL4030_EXTMUTE;
				twl4030_write(codec, TWL4030_REG_HS_POPN_SET,
						hs_pop);
			}
		}
		
        	#ifdef CONFIG_SWITCH_GPIO
		earphone_insert = IsEarPhoneInsert();
		#endif
		
		if(earphone_insert==0)		//earphone inserted
		{
			//cancel the mix for headset, return to stereo mode ,modified by guotao, 2010-01-13
			rx_mix_val &= (~RXL2_SEL_EN);
			rx_mix_val &= (~RXR2_SEL_EN);
			twl4030_write(codec, TWL4030_REG_RX_PATH_SEL, rx_mix_val);
			//enable +2V2_VHSMIC, modified by guotao, 2010-01-14
			micbias_ctl |= TWL4030_HSMICBIAS_EN;
			twl4030_write(codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);		

		}
	} 
	else 
	{
        
        #ifdef CONFIG_SWITCH_GPIO
		earphone_insert = IsEarPhoneInsert();
		#endif
		if(earphone_insert==1)		//earphone not inserted
		{
			//Nicho add 2009-11-30
			//gpio_set_value(GPIO_97, 0); 		//turn off headset amplifier
			if(cp5860e_speaker_ic == ALC108_IC)
			{
				alc108_enable_earphone(0);
			}
			else
			{
				wm9093_enable_earphone(0);
			}
		
			//for speaker, return to mono mode ,modified by guotao, 2010-01-13
			rx_mix_val |= RXL2_SEL_EN;
			rx_mix_val |= RXR2_SEL_EN;
			twl4030_write(codec, TWL4030_REG_RX_PATH_SEL, rx_mix_val);
		
			micbias_ctl |=	TWL4030_MICBIAS1_EN;
			twl4030_write(codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);		
		
		}

		yl_debug("before write TWL4030_REG_HS_POPN_SET reg in %s func,location 2\n",__FUNCTION__);	
		headset_amp_en(0);		
	}
}

static int headsetlpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct twl4030_priv *twl4030 = w->codec->private_data;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* Do the ramp-up only once */
		if (!twl4030->hsr_enabled)
			headset_ramp(w->codec, 1);

		twl4030->hsl_enabled = 1;
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* Do the ramp-down only if both headsetL/R is disabled */
		if (!twl4030->hsr_enabled)
			headset_ramp(w->codec, 0);

		twl4030->hsl_enabled = 0;
		break;
	}
	return 0;
}

static int headsetrpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct twl4030_priv *twl4030 = w->codec->private_data;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* Do the ramp-up only once */
		if (!twl4030->hsl_enabled)
			headset_ramp(w->codec, 1);

		twl4030->hsr_enabled = 1;
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* Do the ramp-down only if both headsetL/R is disabled */
		if (!twl4030->hsl_enabled)
			headset_ramp(w->codec, 0);

		twl4030->hsr_enabled = 0;
		break;
	}
	return 0;
}

static int bypass_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct soc_mixer_control *m =
		(struct soc_mixer_control *)w->kcontrols->private_value;
	struct twl4030_priv *twl4030 = w->codec->private_data;
	unsigned char reg, misc;

	reg = twl4030_read_reg_cache(w->codec, m->reg);

	/*
	 * bypass_state[0:3] - analog HiFi bypass
	 * bypass_state[4]   - analog voice bypass
	 * bypass_state[5]   - digital voice bypass
	 * bypass_state[6:7] - digital HiFi bypass
	 */
	if (m->reg == TWL4030_REG_VSTPGA) {
		/* Voice digital bypass */
		if (reg)
			twl4030->bypass_state |= (1 << 5);
		else
			twl4030->bypass_state &= ~(1 << 5);
	} else if (m->reg <= TWL4030_REG_ARXR2_APGA_CTL) {
		/* Analog bypass */
		if (reg & (1 << m->shift))
			twl4030->bypass_state |=
				(1 << (m->reg - TWL4030_REG_ARXL1_APGA_CTL));
		else
			twl4030->bypass_state &=
				~(1 << (m->reg - TWL4030_REG_ARXL1_APGA_CTL));
	} else if (m->reg == TWL4030_REG_VDL_APGA_CTL) {
		/* Analog voice bypass */
		if (reg & (1 << m->shift))
			twl4030->bypass_state |= (1 << 4);
		else
			twl4030->bypass_state &= ~(1 << 4);
	} else {
		/* Digital bypass */
		if (reg & (0x7 << m->shift))
			twl4030->bypass_state |= (1 << (m->shift ? 7 : 6));
		else
			twl4030->bypass_state &= ~(1 << (m->shift ? 7 : 6));
	}

	/* Enable master analog loopback mode if any analog switch is enabled*/
	misc = twl4030_read_reg_cache(w->codec, TWL4030_REG_MISC_SET_1);
	if (twl4030->bypass_state & 0x1F)
		misc |= TWL4030_FMLOOP_EN;
	else
		misc &= ~TWL4030_FMLOOP_EN;
	twl4030_write(w->codec, TWL4030_REG_MISC_SET_1, misc);

	if (w->codec->bias_level == SND_SOC_BIAS_STANDBY) {
		if (twl4030->bypass_state)
			twl4030_codec_mute(w->codec, 0);
		else
			twl4030_codec_mute(w->codec, 1);
	}
	return 0;
}

#define SOC_SINGLE_GPIO(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_gpio, .get = snd_soc_get_gpio,\
	.put = snd_soc_put_gpio, \
	.private_value =  (unsigned long)NULL, }


/**
 * snd_soc_info_gpio - single mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_info_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_gpio);

/**
 * snd_soc_get_gpio - single mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_get_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	//ucontrol->value.integer.value[0] = gpio_get_value(GPIO_97);
	if(cp5860e_speaker_ic == ALC108_IC)
	{
		ucontrol->value.integer.value[0] = get_alc108_earphone();
	}
	else
	{
		ucontrol->value.integer.value[0] = get_wm9093_earphone();
	}
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_gpio);

/**
 * snd_soc_put_gpio - single mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_put_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	unsigned short ramp;			//define the value to write
	
	ramp = ucontrol->value.integer.value[0];		
	ucontrol->value.integer.value[1] = 0;
	
	headset_amp_en(ramp);
	
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_gpio);



#define SOC_SPEAKER_GPIO(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_speaker_gpio, .get = snd_soc_get_speaker_gpio,\
	.put = snd_soc_put_speaker_gpio, \
	.private_value = (unsigned long) NULL, }


/**
 * snd_soc_info_speaker_gpio - single mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_info_speaker_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_speaker_gpio);

/**
 * snd_soc_get_speaker_gpio - single mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_get_speaker_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	//ucontrol->value.integer.value[0] = gpio_get_value(GPIO_22);
	if(cp5860e_speaker_ic == ALC108_IC)
	{
		ucontrol->value.integer.value[0] = get_alc108_speaker();
	}
	else
	{
		ucontrol->value.integer.value[0] = get_wm9093_speaker();
	}
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_speaker_gpio);

/**
 * snd_soc_put_speaker_gpio - single mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_put_speaker_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	unsigned short set_value;
	set_value = ucontrol->value.integer.value[0];		
	yl_debug("set GPIO_22 to  %d, in %s func\n",set_value,__FUNCTION__);
	//gpio_set_value(GPIO_22,set_value);
	if(cp5860e_speaker_ic == ALC108_IC)
	{
		alc108_enable_speaker(set_value);
	}
	else
	{
		wm9093_enable_speaker(set_value);
	}
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_speaker_gpio);


#define SOC_HEADSET_DET_GPIO(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_headset_det_gpio, .get = snd_soc_get_headset_det_gpio,\
	.put = snd_soc_put_headset_det_gpio, \
	.private_value =  (unsigned long)NULL, }


/**
 * snd_soc_info_headset_det_gpio - single mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_info_headset_det_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_headset_det_gpio);

/**
 * snd_soc_get_headset_det_gpio - single mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_get_headset_det_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
#if 0	
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	
	unsigned char micbias_ctl = 0;

	//it need always enable the TWL4030_HSMICBIAS_EN
	//modified by guotao, 2010-01-14
	micbias_ctl = twl4030_read_reg_cache(codec, TWL4030_REG_MICBIAS_CTL);
	micbias_ctl |= TWL4030_HSMICBIAS_EN;
	twl4030_write(codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);
	mdelay(50);
#endif

    #ifdef CONFIG_SWITCH_GPIO
	ucontrol->value.integer.value[0] = get_headset_plug_status();
    #else
	ucontrol->value.integer.value[0] = gpio_get_value(GPIO_HEAD_DET);
	#endif
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_headset_det_gpio);

/**
 * snd_soc_put_headset_det_gpio - single mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_put_headset_det_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	unsigned short set_value;
	
	set_value = ucontrol->value.integer.value[0];
//	printk("set GPIO_HEAD_DET to  %d, in %s func\n",set_value,__FUNCTION__);
	gpio_set_value(GPIO_HEAD_DET,set_value);
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_headset_det_gpio);

/**
 * snd_soc_put_a2dp_state - single mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a single mixer control.
 *

 * Returns 0 for success.
 */
int snd_soc_info_a2dp_state(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_a2dp_state);

/**
 * snd_soc_get_a2dp_state - single mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information

 *
 * Callback to get the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_get_a2dp_state(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = stream_in_a2dp;
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_a2dp_state);

int snd_soc_put_a2dp_state(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	stream_in_a2dp = ucontrol->value.integer.value[0];
	ucontrol->value.integer.value[1] = 0;
	
	yl_debug("set stream_in_a2dp=%d in %s func\n",stream_in_a2dp,__FUNCTION__);

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_a2dp_state);


#define SOC_A2DP_STATE(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_a2dp_state, .get = snd_soc_get_a2dp_state,\
	.put = snd_soc_put_a2dp_state, \
	.private_value = (unsigned long) NULL, }

#define SOC_MIC_SELECT_GPIO(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_mic_select_gpio, .get = snd_soc_get_mic_select_gpio,\
	.put = snd_soc_put_mic_select_gpio, \
	.private_value = (unsigned long) NULL, }


/**
 * snd_soc_info_mic_select_gpio - single mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_info_mic_select_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_mic_select_gpio);

/**
 * snd_soc_get_mic_select_gpio - single mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_get_mic_select_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	omap_mux_init_signal("gpio_6", OMAP_PIN_INPUT);
	ucontrol->value.integer.value[0] = gpio_get_value(GPIO_6);
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_mic_select_gpio);

/**
 * snd_soc_put_mic_select_gpio - single mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_put_mic_select_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	u8 *cache = codec->reg_cache;
	unsigned short set_value = 0;
	//int  earphone_insert = 0;
	 int mic_level=gpio_get_value(GPIO_6);
	 u8 reg_value;
	set_value = ucontrol->value.integer.value[0];
	ucontrol->value.integer.value[1] = 0;

	if(mic_level==set_value)
		return 0;
	
	yl_debug("set GPIO_6 in %s func\n",__FUNCTION__);
	
    #ifdef CONFIG_SWITCH_GPIO
	//earphone_insert = IsEarPhoneInsert();
	#endif
//关sub mic输入
	twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, (cache[TWL4030_REG_ANAMICR]&0xee), TWL4030_REG_ANAMICR);
	twl_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &reg_value, TWL4030_REG_MICBIAS_CTL);
       twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, reg_value & ~(TWL4030_HSMICBIAS_EN | TWL4030_MICBIAS1_EN), TWL4030_REG_MICBIAS_CTL);

	if(set_value==0)		//earphone inserted
	{
		gpio_set_value(GPIO_6,0);			//select headset mic input when record
	}
	else
	{
		gpio_set_value(GPIO_6,1);			//select local mic input when record	
	}	
	//开sub mic输入
	twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, reg_value, TWL4030_REG_MICBIAS_CTL);
	twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, cache[TWL4030_REG_ANAMICR], TWL4030_REG_ANAMICR);

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_mic_select_gpio);

void codec_input_ctrl(int off)
{
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	u8 *cache = codec->reg_cache;

	//关sub mic输入
	if(off)
	{
			twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, (cache[TWL4030_REG_ANAMICR]&0xee), TWL4030_REG_ANAMICR);
		       twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, cache[TWL4030_REG_MICBIAS_CTL] & ~(TWL4030_HSMICBIAS_EN | TWL4030_MICBIAS1_EN), TWL4030_REG_MICBIAS_CTL);
	}
	else
	{
		//开sub mic输入
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, cache[TWL4030_REG_MICBIAS_CTL] , TWL4030_REG_MICBIAS_CTL);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, cache[TWL4030_REG_ANAMICR], TWL4030_REG_ANAMICR);
	}

}


#define SOC_HEADSET_MIC_ENABLE(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_headset_mic_enable, .get = snd_soc_get_headset_mic_enable,\
	.put = snd_soc_put_headset_mic_enable, \
	.private_value = (unsigned long) NULL, }


/**
 * snd_soc_info_headset_mic_enable - single mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_info_headset_mic_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_headset_mic_enable);

/**
 * snd_soc_get_headset_mic_enable - single mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_get_headset_mic_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	u8 *cache = codec->reg_cache;
	
	ucontrol->value.integer.value[0] = (cache[TWL4030_REG_MICBIAS_CTL]>>2)&0x01;
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_headset_mic_enable);

/**
 * snd_soc_put_headset_mic_enable - single mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_put_headset_mic_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	u8 *cache = codec->reg_cache;
	u8 reg_val=0;
	int  earphone_insert = 0;
	unsigned short set_value = 0;
	
	set_value = ucontrol->value.integer.value[0];
	ucontrol->value.integer.value[1] = 0;
	earphone_insert = IsEarPhoneInsert();
	printk("set_value=%d, earphone_insert=%d\n",set_value,earphone_insert);

	if(set_value || !earphone_insert)//开耳机mic或者耳机插入状态，都开micbias
	{
		reg_val=cache[TWL4030_REG_MICBIAS_CTL] |TWL4030_HSMICBIAS_EN;
	}
	else
	{
		reg_val=cache[TWL4030_REG_MICBIAS_CTL] & (~TWL4030_HSMICBIAS_EN);
	}
	twl4030_write(codec, TWL4030_REG_MICBIAS_CTL, reg_val);
	printk("headset mic bias -->0x%x\n",reg_val);
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_headset_mic_enable);
	
#define		GSM_CDMA_PCM_SELECT_GPIO			22


#define SOC_SCO_PHONE_SELECT_GPIO(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_sco_phone_select_gpio, .get = snd_soc_get_sco_phone_select_gpio,\
	.put = snd_soc_put_sco_phone_select_gpio, \
	.private_value = (unsigned long) NULL, }


/**
 * snd_soc_info_sco_phone_select_gpio - single mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_info_sco_phone_select_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_sco_phone_select_gpio);

/**
 * snd_soc_get_sco_phone_select_gpio - single mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_get_sco_phone_select_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = gpio_get_value(GSM_CDMA_PCM_SELECT_GPIO);
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_sco_phone_select_gpio);

/**
 * snd_soc_put_sco_phone_select_gpio - single mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_put_sco_phone_select_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
//	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	unsigned short set_value = 0;

	
	set_value = ucontrol->value.integer.value[0];
	ucontrol->value.integer.value[1] = 0;

	yl_debug("set GSM_CDMA_PCM_SELECT_GPIO %d in %s func\n",set_value,__FUNCTION__);

	if(set_value==0)		//connect CDMA pcm transfer
		gpio_set_value(GSM_CDMA_PCM_SELECT_GPIO,0);			//select PCM_SW_EN to 0 for CDMA 6085
	else
	{
		gpio_set_value(GSM_CDMA_PCM_SELECT_GPIO,1);			//select PCM_SW_EN to 1 for GSM locosto
	}	
		
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_sco_phone_select_gpio);

#define 	GPIO_USB_SW_SEL1		21
#define		GPIO_USB_SW_SEL			155
#define 	USB_SELECT_AP			0
#define 	USB_SELECT_GSM			1
#define		USB_SELECT_CDMA			2

#define SOC_USB_SELECT_GPIO(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_usb_select_gpio, .get = snd_soc_get_usb_select_gpio,\
	.put = snd_soc_put_usb_select_gpio, \
	.private_value = (unsigned long) NULL, }


/**
 * snd_soc_info_usb_select_gpio - single mixer info callback

 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_info_usb_select_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 3;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 2;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_usb_select_gpio);

/**
 * snd_soc_get_usb_select_gpio - single mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_get_usb_select_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	unsigned short get_value = 0;
	unsigned short get_value2 = 0;		
	
	ucontrol->value.integer.value[0] = 0;
	ucontrol->value.integer.value[1] = 0;

	get_value = gpio_get_value(GPIO_USB_SW_SEL);
	if(get_value == 0)
	{
		ucontrol->value.integer.value[0] = USB_SELECT_AP;
	}
	else if(get_value == 1)
	{
		get_value2 = gpio_get_value(GPIO_USB_SW_SEL1);
		if(get_value2 == 0)
			ucontrol->value.integer.value[0] = USB_SELECT_GSM;
		else if(get_value2 == 1)
			ucontrol->value.integer.value[0] = USB_SELECT_CDMA;	
	}
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_usb_select_gpio);

/**
 * snd_soc_put_usb_select_gpio - single mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a single mixer control.
 *
 * Returns 0 for success.
 */
int snd_soc_put_usb_select_gpio(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
//	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	unsigned short set_value = 0;

	set_value = ucontrol->value.integer.value[0];
	ucontrol->value.integer.value[1] = 0;

	switch(set_value)
	{
		case USB_SELECT_AP:
			gpio_set_value(GPIO_USB_SW_SEL,0);	
			printk("set USB_SELECT_AP in %s func\n",__FUNCTION__);
		break;
		case USB_SELECT_GSM:
			gpio_set_value(GPIO_USB_SW_SEL,1);	
			gpio_set_value(GPIO_USB_SW_SEL1,0);	
			printk("set USB_SELECT_GSM in %s func\n",__FUNCTION__);
		break;	
		case USB_SELECT_CDMA:
			gpio_set_value(GPIO_USB_SW_SEL,1);	
			gpio_set_value(GPIO_USB_SW_SEL1,1);	
			printk("set USB_SELECT_CDMA in %s func\n",__FUNCTION__);
		break;
		default:
		break;
	}
	
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_usb_select_gpio);



#define SOC_CALL_HEADSET_INCALL_CONFIG(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_headset_incall_config, .get = snd_soc_get_headset_incall_config,\
	.put = snd_soc_put_headset_incall_config, \
	.private_value =  (unsigned long)NULL, }
	


int snd_soc_info_headset_incall_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
//	printk("enter %s func, location 1\n",__FUNCTION__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_headset_incall_config);


int snd_soc_get_headset_incall_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
//	printk("enter %s func, location 1\n",__FUNCTION__);
	ucontrol->value.integer.value[0] = 0;
	ucontrol->value.integer.value[1] = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_headset_incall_config);


int snd_soc_put_headset_incall_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	unsigned short set_value;
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	
	set_value = ucontrol->value.integer.value[0];

//	printk("enter %s func, set_value = %d, location 1\n",__FUNCTION__,set_value);

	if(set_value)
		calling_headset(codec);
	else calling_end(codec);
		
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_headset_incall_config);

#define SOC_CALL_EARPIECE_INCALL_CONFIG(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_earpiece_incall_config, .get = snd_soc_get_earpiece_incall_config,\
	.put = snd_soc_put_earpiece_incall_config, \
	.private_value =  (unsigned long)NULL, }
	


int snd_soc_info_earpiece_incall_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
//	printk("enter %s func, location 1\n",__FUNCTION__);
	if(uinfo==NULL)
	{
		yl_debug("uinfo is NULL in %s func,failure to exit\n",__FUNCTION__);
		return -EINVAL;
	}
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_earpiece_incall_config);


int snd_soc_get_earpiece_incall_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
//	printk("enter %s func, location 1\n",__FUNCTION__);
	if(ucontrol==NULL)
	{
		yl_debug("uinfo is NULL in %s func,failure to exit\n",__FUNCTION__);
		return -EINVAL;
	}
	ucontrol->value.integer.value[0] = 0;
	ucontrol->value.integer.value[1] = 0;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_earpiece_incall_config);


int snd_soc_put_earpiece_incall_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	unsigned short set_value;
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	
	set_value = ucontrol->value.integer.value[0];

//	printk("enter %s func, set_value = %d, location 1\n",__FUNCTION__,set_value);

	if(set_value)
	{
		calling_earpiece(codec);
		//gpio_set_value(GPIO_6,1);					//select local mic input when calling earpiece,guotao 2010-03-12
	}
	else calling_end(codec);

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_earpiece_incall_config);


#define SOC_CALL_SPEAKER_INCALL_CONFIG(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_speaker_incall_config, .get = snd_soc_get_speaker_incall_config,\
	.put = snd_soc_put_speaker_incall_config, \
	.private_value =  (unsigned long)NULL, }
	


int snd_soc_info_speaker_incall_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
//	printk("enter %s func, location 1\n",__FUNCTION__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_speaker_incall_config);


int snd_soc_get_speaker_incall_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
//	printk("enter %s func, location 1\n",__FUNCTION__);
	ucontrol->value.integer.value[0] = 0;
	ucontrol->value.integer.value[1] = 0;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_speaker_incall_config);


int snd_soc_put_speaker_incall_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	unsigned short set_value;
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	
	set_value = ucontrol->value.integer.value[0];

//	printk("enter %s func, set_value = %d, location 1\n",__FUNCTION__,set_value);

	if(set_value)
	{
		calling_speaker(codec);
		//gpio_set_value(GPIO_6,1);					//select local mic input when calling speaker,guotao 2010-03-12
	}	
	else calling_end(codec);

	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_speaker_incall_config);

#ifdef AUDIOELECTRIC_AUTOTEST //yuanyufang for audioelectric

static int mic_loopback_select(struct snd_soc_codec *codec,int select )
{
	unsigned char anamic_val;

	if(codec==NULL)
	{
		yl_debug("codec is NULL in %s func,failure to exit\n",__FUNCTION__);
		return -EINVAL;
	}

    if(select==1)
    {
		//modified by guotao, 2010-01-22, reg 0x3E
		anamic_val = 0x22;		//enable FMLOOP_EN
		twl4030_write(codec, TWL4030_REG_MISC_SET_1, anamic_val);

	    anamic_val = 0x15;  //0x15; //
		twl4030_write(codec, TWL4030_REG_ARXR2_APGA_CTL, anamic_val);

		anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICR);
	    anamic_val |= 0x10;
		twl4030_write(codec, TWL4030_REG_ANAMICR, anamic_val);
     }
     else
     {
		anamic_val = 0x00;		//disable FMLOOP_EN
		twl4030_write(codec, TWL4030_REG_MISC_SET_1, anamic_val);

	    anamic_val = 0x10;
		twl4030_write(codec, TWL4030_REG_ARXR2_APGA_CTL, anamic_val);

		anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICR);
	    anamic_val &= (~0x10);
		twl4030_write(codec, TWL4030_REG_ANAMICR, anamic_val);
	 }
}

#define SOC_MIC_LOOPBACK_CONFIG(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_mic_loopback_config, .get = snd_soc_get_mic_loopback_config,\
	.put = snd_soc_put_mic_loopback_config, \
	.private_value =  (unsigned long)NULL, }

int snd_soc_info_mic_loopback_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}

int snd_soc_get_mic_loopback_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = 0;
	ucontrol->value.integer.value[1] = 0;
	return 0;
}

int snd_soc_put_mic_loopback_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	unsigned short set_value;
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;

	set_value = ucontrol->value.integer.value[0];

//	printk("enter %s func, set_value = %d, location 1\n",__FUNCTION__,set_value);

	mic_loopback_select(codec,set_value);

	return 0;
}

static int hsmic_loopback_select(struct snd_soc_codec *codec,int select )
{
	unsigned char anamic_val;

	if(codec==NULL)
	{
		yl_debug("codec is NULL in %s func,failure to exit\n",__FUNCTION__);
		return -EINVAL;
	}

    if(select==1)
    {
		//modified by guotao, 2010-01-22, reg 0x3E
		anamic_val = 0x22;		//enable FMLOOP_EN
		twl4030_write(codec, TWL4030_REG_MISC_SET_1, anamic_val);

	    anamic_val = 0x15;  //0x15; //
		twl4030_write(codec, TWL4030_REG_ARXL2_APGA_CTL, anamic_val);

		anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	    anamic_val |= 0x10;
		twl4030_write(codec, TWL4030_REG_ANAMICL, anamic_val);
     }
     else
     {
		anamic_val = 0x00;		//disable FMLOOP_EN
		twl4030_write(codec, TWL4030_REG_MISC_SET_1, anamic_val);

	    anamic_val = 0x13;
		twl4030_write(codec, TWL4030_REG_ARXL2_APGA_CTL, anamic_val);

		anamic_val = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	    anamic_val &= (~0x10);
		twl4030_write(codec, TWL4030_REG_ANAMICL, anamic_val);
	 }
}

#define SOC_HSMIC_LOOPBACK_CONFIG(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_hsmic_loopback_config, .get = snd_soc_get_hsmic_loopback_config,\
	.put = snd_soc_put_hsmic_loopback_config, \
	.private_value =  (unsigned long)NULL, }

int snd_soc_info_hsmic_loopback_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}

int snd_soc_get_hsmic_loopback_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = 0;
	ucontrol->value.integer.value[1] = 0;
	return 0;
}

int snd_soc_put_hsmic_loopback_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	unsigned short set_value;
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;

	set_value = ucontrol->value.integer.value[0];

//	printk("enter %s func, set_value = %d, location 1\n",__FUNCTION__,set_value);

	hsmic_loopback_select(codec,set_value);

	return 0;
}
#endif

#define SOC_CALL_TWL4030_REGS_CONFIG(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_twl4030_regs_config, .get = snd_soc_get_twl4030_regs_config,\
	.put = snd_soc_put_twl4030_regs_config, \
	.private_value =  (unsigned long)NULL, }
	


int snd_soc_info_twl4030_regs_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
//	printk("enter %s func, location 1\n",__FUNCTION__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_info_twl4030_regs_config);


int snd_soc_get_twl4030_regs_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
//	printk("enter %s func, location 1\n",__FUNCTION__);
	ucontrol->value.integer.value[0] = 0;
	ucontrol->value.integer.value[1] = 0;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_get_twl4030_regs_config);


int snd_soc_put_twl4030_regs_config(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{	
	unsigned short set_value;
	
	set_value = ucontrol->value.integer.value[0];

//	printk("enter %s func, set_value = %d, location 1\n",__FUNCTION__,set_value);

	if(set_value)
		dump_twl4030_regs();
	
	return 0;
}
EXPORT_SYMBOL_GPL(snd_soc_put_twl4030_regs_config);

/*
 * Some of the gain controls in TWL (mostly those which are associated with
 * the outputs) are implemented in an interesting way:
 * 0x0 : Power down (mute)
 * 0x1 : 6dB
 * 0x2 : 0 dB
 * 0x3 : -6 dB
 * Inverting not going to help with these.
 * Custom volsw and volsw_2r get/put functions to handle these gain bits.
 */
#define SOC_DOUBLE_TLV_TWL4030(xname, xreg, shift_left, shift_right, xmax,\
			       xinvert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = (xname),\
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_volsw, \
	.get = snd_soc_get_volsw_twl4030, \
	.put = snd_soc_put_volsw_twl4030, \
	.private_value = (unsigned long)&(struct soc_mixer_control) \
		{.reg = xreg, .shift = shift_left, .rshift = shift_right,\
		 .max = xmax, .invert = xinvert} }
#define SOC_DOUBLE_R_TLV_TWL4030(xname, reg_left, reg_right, xshift, xmax,\
				 xinvert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = (xname),\
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_volsw_2r, \
	.get = snd_soc_get_volsw_r2_twl4030,\
	.put = snd_soc_put_volsw_r2_twl4030, \
	.private_value = (unsigned long)&(struct soc_mixer_control) \
		{.reg = reg_left, .rreg = reg_right, .shift = xshift, \
		 .rshift = xshift, .max = xmax, .invert = xinvert} }
#define SOC_SINGLE_TLV_TWL4030(xname, xreg, xshift, xmax, xinvert, tlv_array) \
	SOC_DOUBLE_TLV_TWL4030(xname, xreg, xshift, xshift, xmax, \
			       xinvert, tlv_array)

static int snd_soc_get_volsw_twl4030(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int shift = mc->shift;
	unsigned int rshift = mc->rshift;
	int max = mc->max;
	int mask = (1 << fls(max)) - 1;

	ucontrol->value.integer.value[0] =
		(snd_soc_read(codec, reg) >> shift) & mask;
	if (ucontrol->value.integer.value[0])
		ucontrol->value.integer.value[0] =
			max + 1 - ucontrol->value.integer.value[0];

	if (shift != rshift) {
		ucontrol->value.integer.value[1] =
			(snd_soc_read(codec, reg) >> rshift) & mask;
		if (ucontrol->value.integer.value[1])
			ucontrol->value.integer.value[1] =
				max + 1 - ucontrol->value.integer.value[1];
	}

	return 0;
}

static int snd_soc_put_volsw_twl4030(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int shift = mc->shift;
	unsigned int rshift = mc->rshift;
	int max = mc->max;
	int mask = (1 << fls(max)) - 1;
	unsigned short val, val2, val_mask;

	val = (ucontrol->value.integer.value[0] & mask);

	val_mask = mask << shift;
	if (val)
		val = max + 1 - val;
	val = val << shift;
	if (shift != rshift) {
		val2 = (ucontrol->value.integer.value[1] & mask);
		val_mask |= mask << rshift;
		if (val2)
			val2 = max + 1 - val2;
		val |= val2 << rshift;
	}
	return snd_soc_update_bits(codec, reg, val_mask, val);
}

static int snd_soc_get_volsw_r2_twl4030(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int reg2 = mc->rreg;
	unsigned int shift = mc->shift;
	int max = mc->max;
	int mask = (1<<fls(max))-1;

	ucontrol->value.integer.value[0] =
		(snd_soc_read(codec, reg) >> shift) & mask;
	ucontrol->value.integer.value[1] =
		(snd_soc_read(codec, reg2) >> shift) & mask;

	if (ucontrol->value.integer.value[0])
		ucontrol->value.integer.value[0] =
			max + 1 - ucontrol->value.integer.value[0];
	if (ucontrol->value.integer.value[1])
		ucontrol->value.integer.value[1] =
			max + 1 - ucontrol->value.integer.value[1];

	return 0;
}

static int snd_soc_put_volsw_r2_twl4030(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int reg2 = mc->rreg;
	unsigned int shift = mc->shift;
	int max = mc->max;
	int mask = (1 << fls(max)) - 1;
	int err;
	unsigned short val, val2, val_mask;

	val_mask = mask << shift;
	val = (ucontrol->value.integer.value[0] & mask);
	val2 = (ucontrol->value.integer.value[1] & mask);

	if (val)
		val = max + 1 - val;
	if (val2)
		val2 = max + 1 - val2;

	val = val << shift;
	val2 = val2 << shift;

	err = snd_soc_update_bits(codec, reg, val_mask, val);
	if (err < 0)
		return err;

	err = snd_soc_update_bits(codec, reg2, val_mask, val2);
	return err;
}

/* Codec operation modes */
static const char *twl4030_op_modes_texts[] = {
	"Option 2 (voice/audio)", "Option 1 (audio)"
};

static const struct soc_enum twl4030_op_modes_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_CODEC_MODE, 0,
			ARRAY_SIZE(twl4030_op_modes_texts),
			twl4030_op_modes_texts);

static int snd_soc_put_twl4030_opmode_enum_double(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct twl4030_priv *twl4030 = codec->private_data;
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	unsigned short val;
	unsigned short mask, bitmask;

	if (!list_empty(&twl4030->config_list)) {
		printk(KERN_ERR "twl4030 operation mode cannot be "
			"changed on-the-fly\n");
		return -EBUSY;
	}

	for (bitmask = 1; bitmask < e->max; bitmask <<= 1)
		;
	if (ucontrol->value.enumerated.item[0] > e->max - 1)
		return -EINVAL;

	val = ucontrol->value.enumerated.item[0] << e->shift_l;
	mask = (bitmask - 1) << e->shift_l;
	if (e->shift_l != e->shift_r) {
		if (ucontrol->value.enumerated.item[1] > e->max - 1)
			return -EINVAL;
		val |= ucontrol->value.enumerated.item[1] << e->shift_r;
		mask |= (bitmask - 1) << e->shift_r;
	}

	return snd_soc_update_bits(codec, e->reg, mask, val);
}

static int dump_twl4030_regs(void)
{	
	unsigned char get_value = 0;
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	int i = 0;
	

	if(codec==NULL)
	{
		return -EINVAL;
	}

	for(i = 0; i <= 0x4a; i++)
	{
		get_value = twl4030_read_reg_cache(codec, i);
		yl_debug("reg  = 0x%2.2x, value = 0x%2.2x\n", i, get_value);
	}
	
	return 0;
}

/*
 * FGAIN volume control:
 * from -62 to 0 dB in 1 dB steps (mute instead of -63 dB)
 */
static DECLARE_TLV_DB_SCALE(digital_fine_tlv, -6300, 100, 1);

/*
 * CGAIN volume control:
 * 0 dB to 12 dB in 6 dB steps
 * value 2 and 3 means 12 dB
 */
static DECLARE_TLV_DB_SCALE(digital_coarse_tlv, 0, 600, 0);

/*
 * Voice Downlink GAIN volume control:
 * from -37 to 12 dB in 1 dB steps (mute instead of -37 dB)
 */
static DECLARE_TLV_DB_SCALE(digital_voice_downlink_tlv, -3700, 100, 1);

/*
 * Analog playback gain
 * -24 dB to 12 dB in 2 dB steps
 */
static DECLARE_TLV_DB_SCALE(analog_tlv, -2400, 200, 0);

/*
 * Gain controls tied to outputs
 * -6 dB to 6 dB in 6 dB steps (mute instead of -12)
 */
static DECLARE_TLV_DB_SCALE(output_tvl, -1200, 600, 1);

/*
 * Gain control for earpiece amplifier
 * 0 dB to 12 dB in 6 dB steps (mute instead of -6)
 */
static DECLARE_TLV_DB_SCALE(output_ear_tvl, -600, 600, 1);//deleted by guotao, 2010-06-23

/*
 * Capture gain after the ADCs
 * from 0 dB to 31 dB in 1 dB steps
 */
static DECLARE_TLV_DB_SCALE(digital_capture_tlv, 0, 100, 0);

/*
 * Gain control for input amplifiers
 * 0 dB to 30 dB in 6 dB steps
 */
static DECLARE_TLV_DB_SCALE(input_gain_tlv, 0, 600, 0);

/* AVADC clock priority */
static const char *twl4030_avadc_clk_priority_texts[] = {
	"Voice high priority", "HiFi high priority"
};

static const struct soc_enum twl4030_avadc_clk_priority_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_AVADC_CTL, 2,
			ARRAY_SIZE(twl4030_avadc_clk_priority_texts),
			twl4030_avadc_clk_priority_texts);

static const char *twl4030_rampdelay_texts[] = {
	"27/20/14 ms", "55/40/27 ms", "109/81/55 ms", "218/161/109 ms",
	"437/323/218 ms", "874/645/437 ms", "1748/1291/874 ms",
	"3495/2581/1748 ms"
};

static const struct soc_enum twl4030_rampdelay_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_HS_POPN_SET, 2,
			ARRAY_SIZE(twl4030_rampdelay_texts),
			twl4030_rampdelay_texts);

/* Vibra H-bridge direction mode */
static const char *twl4030_vibradirmode_texts[] = {
	"Vibra H-bridge direction", "Audio data MSB",
};

static const struct soc_enum twl4030_vibradirmode_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 5,
			ARRAY_SIZE(twl4030_vibradirmode_texts),
			twl4030_vibradirmode_texts);

/* Vibra H-bridge direction */
static const char *twl4030_vibradir_texts[] = {
	"Positive polarity", "Negative polarity",
};

static const struct soc_enum twl4030_vibradir_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 1,
			ARRAY_SIZE(twl4030_vibradir_texts),
			twl4030_vibradir_texts);

static const struct snd_kcontrol_new twl4030_snd_controls[] = {
	/* Codec operation mode control */
	SOC_ENUM_EXT("Codec Operation Mode", twl4030_op_modes_enum,
		snd_soc_get_enum_double,
		snd_soc_put_twl4030_opmode_enum_double),

	/* Common playback gain controls */
	SOC_DOUBLE_R_TLV("DAC1 Digital Fine Playback Volume",
		TWL4030_REG_ARXL1PGA, TWL4030_REG_ARXR1PGA,
		0, 0x3f, 0, digital_fine_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Digital Fine Playback Volume",
		TWL4030_REG_ARXL2PGA, TWL4030_REG_ARXR2PGA,
		0, 0x3f, 0, digital_fine_tlv),

	SOC_DOUBLE_R_TLV("DAC1 Digital Coarse Playback Volume",
		TWL4030_REG_ARXL1PGA, TWL4030_REG_ARXR1PGA,
		6, 0x2, 0, digital_coarse_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Digital Coarse Playback Volume",
		TWL4030_REG_ARXL2PGA, TWL4030_REG_ARXR2PGA,
		6, 0x2, 0, digital_coarse_tlv),

	SOC_DOUBLE_R_TLV("DAC1 Analog Playback Volume",
		TWL4030_REG_ARXL1_APGA_CTL, TWL4030_REG_ARXR1_APGA_CTL,
		3, 0x12, 1, analog_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Analog Playback Volume",
		TWL4030_REG_ARXL2_APGA_CTL, TWL4030_REG_ARXR2_APGA_CTL,
		3, 0x12, 1, analog_tlv),
	SOC_DOUBLE_R("DAC1 Analog Playback Switch",
		TWL4030_REG_ARXL1_APGA_CTL, TWL4030_REG_ARXR1_APGA_CTL,
		1, 1, 0),
	SOC_DOUBLE_R("DAC2 Analog Playback Switch",
		TWL4030_REG_ARXL2_APGA_CTL, TWL4030_REG_ARXR2_APGA_CTL,
		1, 1, 0),

	/* Common voice downlink gain controls */
	SOC_SINGLE_TLV("DAC Voice Digital Downlink Volume",
		TWL4030_REG_VRXPGA, 0, 0x31, 0, digital_voice_downlink_tlv),

	SOC_SINGLE_TLV("DAC Voice Analog Downlink Volume",
		TWL4030_REG_VDL_APGA_CTL, 3, 0x12, 1, analog_tlv),

	SOC_SINGLE("DAC Voice Analog Downlink Switch",
		TWL4030_REG_VDL_APGA_CTL, 1, 1, 0),

	//modified by guotao, 2010-04-27
	SOC_SINGLE("Analog L2 Playback Switch",
		TWL4030_REG_ARXL2_APGA_CTL, 0, 1,  0),
	SOC_SINGLE("Analog R2 Playback Switch",
		TWL4030_REG_ARXR2_APGA_CTL, 0, 1,  0),
	
	//modified by guotao, 2010-04-29
	SOC_SINGLE("Digital R2 Playback Mixer Enable",
		TWL4030_REG_AVDAC_CTL, 2, 1, 0),
	SOC_SINGLE("Digital L2 Playback Mixer Enable",
		TWL4030_REG_AVDAC_CTL, 3, 1, 0),
	
	
	//modified by guotao , 2010-01-28
	SOC_SINGLE("HS_SEL HSOL_AL2_EN Ctrl",			
		TWL4030_REG_HS_SEL, 2, 1,  0),
	
	SOC_SINGLE("HS_SEL HSOR_AR2_EN Ctrl",
		TWL4030_REG_HS_SEL, 5, 1,  0),
	
	//modified by guotao , 2010-04-17
	SOC_SINGLE("RX_PATH_SEL RXL2_SEL Ctrl", 		
		TWL4030_REG_RX_PATH_SEL, 5, 1,	0),
	
	SOC_SINGLE("RX_PATH_SEL RXR2_SEL Ctrl",
		TWL4030_REG_RX_PATH_SEL, 4, 1,	0),
	
	SOC_SINGLE("Mic Bias 1 EN", TWL4030_REG_MICBIAS_CTL, 0, 1, 0),
	SOC_SINGLE("Mic Bias 2 EN", TWL4030_REG_MICBIAS_CTL, 1, 1, 0),
	//SOC_SINGLE("Headset Mic Bias EN", TWL4030_REG_MICBIAS_CTL, 2, 1, 0),
	SOC_HEADSET_MIC_ENABLE("Headset Mic Bias EN", -1, 0, 1, 0, analog_tlv),

	SOC_SINGLE_GPIO("Headset Amp Enable Ctrl", -1, 0, 2, 0, analog_tlv),		//reg = -1, means no reg address
	SOC_SPEAKER_GPIO("Speaker Amp Enable Ctrl", -1, 0, 2, 0, analog_tlv),		//reg = -1, means no reg address
	SOC_MIC_SELECT_GPIO("Mic Select Ctrl", -1, 0, 2, 0, analog_tlv),		//reg = -1, means no reg address
	SOC_A2DP_STATE("a2dp state", -1, 0, 2, 0, analog_tlv),
	SOC_USB_SELECT_GPIO("USB Select Ctrl", -1, 0, 3, 0, analog_tlv),		//3: means it have max 3 group selects
	SOC_HEADSET_DET_GPIO("GPIO_HEADSET_DET Ctrl", -1, 0, 2, 0, analog_tlv),
	SOC_SCO_PHONE_SELECT_GPIO("GPIO PCM_SW_EN Ctrl", -1, 0, 2, 0, analog_tlv),
	
	SOC_CALL_TWL4030_REGS_CONFIG("Read TWL4030 Regs", -1, 0, 2, 0, analog_tlv),
	
	SOC_CALL_HEADSET_INCALL_CONFIG("Calling Headset Config", -1, 0, 2, 0, analog_tlv),
	
	SOC_CALL_SPEAKER_INCALL_CONFIG("Calling Speaker Config", -1, 0, 2, 0, analog_tlv),
	
	SOC_CALL_EARPIECE_INCALL_CONFIG("Calling Earpiece Config", -1, 0, 2, 0, analog_tlv),

#ifdef AUDIOELECTRIC_AUTOTEST
    SOC_MIC_LOOPBACK_CONFIG("Mic Loopback Config", -1, 0, 2, 0, analog_tlv),
    SOC_HSMIC_LOOPBACK_CONFIG("HsMic Loopback Config", -1, 0, 2, 0, analog_tlv),
#endif

	/* Separate output gain controls */
	SOC_DOUBLE_R_TLV_TWL4030("PreDriv Playback Volume",
		TWL4030_REG_PREDL_CTL, TWL4030_REG_PREDR_CTL,
		4, 3, 0, output_tvl),

    //modiifed headset volume is high problem, by guotao, 2010-09-24
	SOC_DOUBLE_TLV_TWL4030("Headset Analog Playback Volume",
		TWL4030_REG_HS_GAIN_SET, 0, 2, 3, 0, output_tvl),

	SOC_DOUBLE_R_TLV_TWL4030("Carkit Playback Volume",
		TWL4030_REG_PRECKL_CTL, TWL4030_REG_PRECKR_CTL,
		4, 3, 0, output_tvl),

	SOC_SINGLE_TLV_TWL4030("Earpiece PGA Volume",
		TWL4030_REG_EAR_CTL, 4, 3, 0, output_ear_tvl),
	
	SOC_SINGLE_TLV2("Earpiece Playback Volume",
		TWL4030_REG_VDL_APGA_CTL, 3, 0x04, 0x0c, 1, analog_tlv),		//min: 0x0b-->0x07改为->1, max: 0x12-->0x0e改为-->8 /*Modified by sunruichen 20110601*/
	
	/* Common capture gain controls */ 
	SOC_DOUBLE_R_TLV("TX1 Digital Capture Volume",
		TWL4030_REG_ATXL1PGA, TWL4030_REG_ATXR1PGA,
		0, 0x1f, 0, digital_capture_tlv),
	SOC_DOUBLE_R_TLV("TX2 Digital Capture Volume",
		TWL4030_REG_AVTXL2PGA, TWL4030_REG_AVTXR2PGA,
		0, 0x1f, 0, digital_capture_tlv),

	SOC_DOUBLE_TLV("Analog Capture Volume", TWL4030_REG_ANAMIC_GAIN,
		0, 3, 5, 0, input_gain_tlv),

	SOC_ENUM("AVADC Clock Priority", twl4030_avadc_clk_priority_enum),

	SOC_ENUM("HS ramp delay", twl4030_rampdelay_enum),

	SOC_ENUM("Vibra H-bridge mode", twl4030_vibradirmode_enum),
	SOC_ENUM("Vibra H-bridge direction", twl4030_vibradir_enum),
	
	SOC_SINGLE("Analog Capture Left Switch",
		TWL4030_REG_ANAMICL, 4, 1, 0),
};

static const struct snd_soc_dapm_widget twl4030_dapm_widgets[] = {
	/* Left channel inputs */
	SND_SOC_DAPM_INPUT("MAINMIC"),
	SND_SOC_DAPM_INPUT("HSMIC"),
	SND_SOC_DAPM_INPUT("AUXL"),
	SND_SOC_DAPM_INPUT("CARKITMIC"),
	/* Right channel inputs */
	SND_SOC_DAPM_INPUT("SUBMIC"),
	SND_SOC_DAPM_INPUT("AUXR"),
	/* Digital microphones (Stereo) */
	SND_SOC_DAPM_INPUT("DIGIMIC0"),
	SND_SOC_DAPM_INPUT("DIGIMIC1"),

	/* Outputs */
	SND_SOC_DAPM_OUTPUT("OUTL"),
	SND_SOC_DAPM_OUTPUT("OUTR"),
	SND_SOC_DAPM_OUTPUT("EARPIECE"),
	SND_SOC_DAPM_OUTPUT("PREDRIVEL"),
	SND_SOC_DAPM_OUTPUT("PREDRIVER"),
	SND_SOC_DAPM_OUTPUT("HSOL"),
	SND_SOC_DAPM_OUTPUT("HSOR"),
	SND_SOC_DAPM_OUTPUT("CARKITL"),
	SND_SOC_DAPM_OUTPUT("CARKITR"),
	SND_SOC_DAPM_OUTPUT("HFL"),
	SND_SOC_DAPM_OUTPUT("HFR"),
	SND_SOC_DAPM_OUTPUT("VIBRA"),

	/* DACs */
	SND_SOC_DAPM_DAC("DAC Right1", "Right Front HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Left1", "Left Front HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Right2", "Right Rear HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Left2", "Left Rear HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Voice", "Voice Playback",
			SND_SOC_NOPM, 0, 0),

	/* Analog bypasses */
	SND_SOC_DAPM_SWITCH_E("Right1 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassr1_control, bypass_event,
			SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Left1 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassl1_control,
			bypass_event, SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Right2 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassr2_control,
			bypass_event, SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Left2 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassl2_control,
			bypass_event, SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Voice Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassv_control,
			bypass_event, SND_SOC_DAPM_POST_REG),

	/* Digital bypasses */
	SND_SOC_DAPM_SWITCH_E("Left Digital Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_dbypassl_control, bypass_event,
			SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Right Digital Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_dbypassr_control, bypass_event,
			SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Voice Digital Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_dbypassv_control, bypass_event,
			SND_SOC_DAPM_POST_REG),

	/* Digital mixers, power control for the physical DACs */
	SND_SOC_DAPM_MIXER("Digital R1 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital L1 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 1, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital R2 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 2, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital L2 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 3, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital Voice Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 4, 0, NULL, 0),

	/* Analog mixers, power control for the physical PGAs */
	SND_SOC_DAPM_MIXER("Analog R1 Playback Mixer",
			TWL4030_REG_ARXR1_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog L1 Playback Mixer",
			TWL4030_REG_ARXL1_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog R2 Playback Mixer",
			TWL4030_REG_ARXR2_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog L2 Playback Mixer",
			TWL4030_REG_ARXL2_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog Voice Playback Mixer",
			TWL4030_REG_VDL_APGA_CTL, 0, 0, NULL, 0),

	/* Output MIXER controls */
	/* Earpiece */
	SND_SOC_DAPM_MIXER("Earpiece Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_earpiece_controls[0],
			ARRAY_SIZE(twl4030_dapm_earpiece_controls)),
	SND_SOC_DAPM_PGA_E("Earpiece PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, earpiecepga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	/* PreDrivL/R */
	SND_SOC_DAPM_MIXER("PredriveL Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_predrivel_controls[0],
			ARRAY_SIZE(twl4030_dapm_predrivel_controls)),
	SND_SOC_DAPM_PGA_E("PredriveL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, predrivelpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER("PredriveR Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_predriver_controls[0],
			ARRAY_SIZE(twl4030_dapm_predriver_controls)),
	SND_SOC_DAPM_PGA_E("PredriveR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, predriverpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	/* HeadsetL/R */
	SND_SOC_DAPM_MIXER("HeadsetL Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_hsol_controls[0],
			ARRAY_SIZE(twl4030_dapm_hsol_controls)),

//delete auto the pop noise cleared by guotao, 2010-11-16
#if 0
	SND_SOC_DAPM_PGA_E("HeadsetL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, headsetlpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
#else
	SND_SOC_DAPM_PGA_E("HeadsetL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, NULL,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
#endif

	SND_SOC_DAPM_MIXER("HeadsetR Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_hsor_controls[0],
			ARRAY_SIZE(twl4030_dapm_hsor_controls)),

//delete auto the pop noise cleared by guotao, 2010-11-16
#if 0
	SND_SOC_DAPM_PGA_E("HeadsetR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, headsetrpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
#else
	SND_SOC_DAPM_PGA_E("HeadsetR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, NULL,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
#endif

	/* CarkitL/R */
	SND_SOC_DAPM_MIXER("CarkitL Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_carkitl_controls[0],
			ARRAY_SIZE(twl4030_dapm_carkitl_controls)),
	SND_SOC_DAPM_PGA_E("CarkitL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, carkitlpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER("CarkitR Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_carkitr_controls[0],
			ARRAY_SIZE(twl4030_dapm_carkitr_controls)),
	SND_SOC_DAPM_PGA_E("CarkitR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, carkitrpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),

	/* Output MUX controls */
	/* HandsfreeL/R */
	SND_SOC_DAPM_MUX("HandsfreeL Mux", SND_SOC_NOPM, 0, 0,
		&twl4030_dapm_handsfreel_control),
	SND_SOC_DAPM_SWITCH("HandsfreeL", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_handsfreelmute_control),
	SND_SOC_DAPM_PGA_E("HandsfreeL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, handsfreelpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX("HandsfreeR Mux", SND_SOC_NOPM, 5, 0,
		&twl4030_dapm_handsfreer_control),
	SND_SOC_DAPM_SWITCH("HandsfreeR", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_handsfreermute_control),
	SND_SOC_DAPM_PGA_E("HandsfreeR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, handsfreerpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	/* Vibra */
	SND_SOC_DAPM_MUX("Vibra Mux", TWL4030_REG_VIBRA_CTL, 0, 0,
		&twl4030_dapm_vibra_control),
	SND_SOC_DAPM_MUX("Vibra Route", SND_SOC_NOPM, 0, 0,
		&twl4030_dapm_vibrapath_control),

	/* Introducing four virtual ADC, since TWL4030 have four channel for
	   capture */
	SND_SOC_DAPM_ADC("ADC Virtual Left1", "Left Front Capture",
		SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC Virtual Right1", "Right Front Capture",
		SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC Virtual Left2", "Left Rear Capture",
		SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC Virtual Right2", "Right Rear Capture",
		SND_SOC_NOPM, 0, 0),

	/* Analog/Digital mic path selection.
	   TX1 Left/Right: either analog Left/Right or Digimic0
	   TX2 Left/Right: either analog Left/Right or Digimic1 */
	SND_SOC_DAPM_MUX_E("TX1 Capture Route", SND_SOC_NOPM, 0, 0,
		&twl4030_dapm_micpathtx1_control, NULL,
		SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD|
		SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_MUX_E("TX2 Capture Route", SND_SOC_NOPM, 0, 0,
		&twl4030_dapm_micpathtx2_control, NULL,
		SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD|
		SND_SOC_DAPM_POST_REG),

	/* Analog input mixers for the capture amplifiers */
	SND_SOC_DAPM_MIXER("Analog Left",
		TWL4030_REG_ANAMICL, 4, 0,
		&twl4030_dapm_analoglmic_controls[0],
		ARRAY_SIZE(twl4030_dapm_analoglmic_controls)),
	SND_SOC_DAPM_MIXER("Analog Right",
		TWL4030_REG_ANAMICR, 4, 0,
		&twl4030_dapm_analogrmic_controls[0],
		ARRAY_SIZE(twl4030_dapm_analogrmic_controls)),

	SND_SOC_DAPM_PGA("ADC Physical Left",
		TWL4030_REG_AVADC_CTL, 3, 0, NULL, 0),
	SND_SOC_DAPM_PGA("ADC Physical Right",
		TWL4030_REG_AVADC_CTL, 1, 0, NULL, 0),

	SND_SOC_DAPM_PGA("Digimic0 Enable",
		TWL4030_REG_ADCMICSEL, 1, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Digimic1 Enable",
		TWL4030_REG_ADCMICSEL, 3, 0, NULL, 0),

	SND_SOC_DAPM_MICBIAS("Mic Bias 1", TWL4030_REG_MICBIAS_CTL, 0, 0),
	SND_SOC_DAPM_MICBIAS("Mic Bias 2", TWL4030_REG_MICBIAS_CTL, 1, 0),
	//SND_SOC_DAPM_MICBIAS("Headset Mic Bias", TWL4030_REG_MICBIAS_CTL, 2, 0),

};

static const struct snd_soc_dapm_route intercon[] = {
	{"Digital L1 Playback Mixer", NULL, "DAC Left1"},
	{"Digital R1 Playback Mixer", NULL, "DAC Right1"},
	{"Digital L2 Playback Mixer", NULL, "DAC Left2"},
	{"Digital R2 Playback Mixer", NULL, "DAC Right2"},
	{"Digital Voice Playback Mixer", NULL, "DAC Voice"},

	{"Analog L1 Playback Mixer", NULL, "Digital L1 Playback Mixer"},
	{"Analog R1 Playback Mixer", NULL, "Digital R1 Playback Mixer"},
	{"Analog L2 Playback Mixer", NULL, "Digital L2 Playback Mixer"},
	{"Analog R2 Playback Mixer", NULL, "Digital R2 Playback Mixer"},
	{"Analog Voice Playback Mixer", NULL, "Digital Voice Playback Mixer"},

	/* Internal playback routings */
	/* Earpiece */
	{"Earpiece Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"Earpiece Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"Earpiece Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"Earpiece Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"Earpiece PGA", NULL, "Earpiece Mixer"},
	/* PreDrivL */
	{"PredriveL Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"PredriveL Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"PredriveL Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"PredriveL Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"PredriveL PGA", NULL, "PredriveL Mixer"},
	/* PreDrivR */
	{"PredriveR Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"PredriveR Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"PredriveR Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"PredriveR Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"PredriveR PGA", NULL, "PredriveR Mixer"},
	/* HeadsetL */
	{"HeadsetL Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"HeadsetL Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"HeadsetL Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"HeadsetL PGA", NULL, "HeadsetL Mixer"},
	/* HeadsetR */
	{"HeadsetR Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"HeadsetR Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"HeadsetR Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"HeadsetR PGA", NULL, "HeadsetR Mixer"},
	/* CarkitL */
	{"CarkitL Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"CarkitL Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"CarkitL Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"CarkitL PGA", NULL, "CarkitL Mixer"},
	/* CarkitR */
	{"CarkitR Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"CarkitR Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"CarkitR Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"CarkitR PGA", NULL, "CarkitR Mixer"},
	/* HandsfreeL */
	{"HandsfreeL Mux", "Voice", "Analog Voice Playback Mixer"},
	{"HandsfreeL Mux", "AudioL1", "Analog L1 Playback Mixer"},
	{"HandsfreeL Mux", "AudioL2", "Analog L2 Playback Mixer"},
	{"HandsfreeL Mux", "AudioR2", "Analog R2 Playback Mixer"},
	{"HandsfreeL", "Switch", "HandsfreeL Mux"},
	{"HandsfreeL PGA", NULL, "HandsfreeL"},
	/* HandsfreeR */
	{"HandsfreeR Mux", "Voice", "Analog Voice Playback Mixer"},
	{"HandsfreeR Mux", "AudioR1", "Analog R1 Playback Mixer"},
	{"HandsfreeR Mux", "AudioR2", "Analog R2 Playback Mixer"},
	{"HandsfreeR Mux", "AudioL2", "Analog L2 Playback Mixer"},
	{"HandsfreeR", "Switch", "HandsfreeR Mux"},
	{"HandsfreeR PGA", NULL, "HandsfreeR"},
	/* Vibra */
	{"Vibra Mux", "AudioL1", "DAC Left1"},
	{"Vibra Mux", "AudioR1", "DAC Right1"},
	{"Vibra Mux", "AudioL2", "DAC Left2"},
	{"Vibra Mux", "AudioR2", "DAC Right2"},

	/* outputs */
	{"OUTL", NULL, "Analog L2 Playback Mixer"},
	{"OUTR", NULL, "Analog R2 Playback Mixer"},
	{"EARPIECE", NULL, "Earpiece PGA"},
	{"PREDRIVEL", NULL, "PredriveL PGA"},
	{"PREDRIVER", NULL, "PredriveR PGA"},
	{"HSOL", NULL, "HeadsetL PGA"},
	{"HSOR", NULL, "HeadsetR PGA"},
	{"CARKITL", NULL, "CarkitL PGA"},
	{"CARKITR", NULL, "CarkitR PGA"},
	{"HFL", NULL, "HandsfreeL PGA"},
	{"HFR", NULL, "HandsfreeR PGA"},
	{"Vibra Route", "Audio", "Vibra Mux"},
	{"VIBRA", NULL, "Vibra Route"},

	/* Capture path */
	{"Analog Left", "Main Mic Capture Switch", "MAINMIC"},
	{"Analog Left", "Headset Mic Capture Switch", "HSMIC"},
	{"Analog Left", "AUXL Capture Switch", "AUXL"},
	{"Analog Left", "Carkit Mic Capture Switch", "CARKITMIC"},

	{"Analog Right", "Sub Mic Capture Switch", "SUBMIC"},
	{"Analog Right", "AUXR Capture Switch", "AUXR"},

	{"ADC Physical Left", NULL, "Analog Left"},
	{"ADC Physical Right", NULL, "Analog Right"},

	{"Digimic0 Enable", NULL, "DIGIMIC0"},
	{"Digimic1 Enable", NULL, "DIGIMIC1"},

	/* TX1 Left capture path */
	{"TX1 Capture Route", "Analog", "ADC Physical Left"},
	{"TX1 Capture Route", "Digimic0", "Digimic0 Enable"},
	/* TX1 Right capture path */
	{"TX1 Capture Route", "Analog", "ADC Physical Right"},
	{"TX1 Capture Route", "Digimic0", "Digimic0 Enable"},
	/* TX2 Left capture path */
	{"TX2 Capture Route", "Analog", "ADC Physical Left"},
	{"TX2 Capture Route", "Digimic1", "Digimic1 Enable"},
	/* TX2 Right capture path */
	{"TX2 Capture Route", "Analog", "ADC Physical Right"},
	{"TX2 Capture Route", "Digimic1", "Digimic1 Enable"},

	{"ADC Virtual Left1", NULL, "TX1 Capture Route"},
	{"ADC Virtual Right1", NULL, "TX1 Capture Route"},
	{"ADC Virtual Left2", NULL, "TX2 Capture Route"},
	{"ADC Virtual Right2", NULL, "TX2 Capture Route"},

	/* Analog bypass routes */
	{"Right1 Analog Loopback", "Switch", "Analog Right"},
	{"Left1 Analog Loopback", "Switch", "Analog Left"},
	{"Right2 Analog Loopback", "Switch", "Analog Right"},
	{"Left2 Analog Loopback", "Switch", "Analog Left"},
	{"Voice Analog Loopback", "Switch", "Analog Left"},

	{"Analog R1 Playback Mixer", NULL, "Right1 Analog Loopback"},
	{"Analog L1 Playback Mixer", NULL, "Left1 Analog Loopback"},
	{"Analog R2 Playback Mixer", NULL, "Right2 Analog Loopback"},
	{"Analog L2 Playback Mixer", NULL, "Left2 Analog Loopback"},
	{"Analog Voice Playback Mixer", NULL, "Voice Analog Loopback"},

	/* Digital bypass routes */
	{"Right Digital Loopback", "Volume", "TX1 Capture Route"},
	{"Left Digital Loopback", "Volume", "TX1 Capture Route"},
	{"Voice Digital Loopback", "Volume", "TX2 Capture Route"},

	{"Digital R2 Playback Mixer", NULL, "Right Digital Loopback"},
	{"Digital L2 Playback Mixer", NULL, "Left Digital Loopback"},
	{"Digital Voice Playback Mixer", NULL, "Voice Digital Loopback"},

};

static int twl4030_add_widgets(struct snd_soc_codec *codec)
{
	snd_soc_dapm_new_controls(codec, twl4030_dapm_widgets,
				 ARRAY_SIZE(twl4030_dapm_widgets));

	snd_soc_dapm_add_routes(codec, intercon, ARRAY_SIZE(intercon));

	snd_soc_dapm_new_widgets(codec);
	return 0;
}

static int twl4030_set_bias_level(struct snd_soc_codec *codec,
				  enum snd_soc_bias_level level)
{
	struct twl4030_priv *twl4030 = codec->private_data;

	switch (level) {
	case SND_SOC_BIAS_ON:
		twl4030_codec_mute(codec, 0);
		break;
	case SND_SOC_BIAS_PREPARE:
		twl4030_power_up(codec);
		if (twl4030->bypass_state)
			twl4030_codec_mute(codec, 0);
		else
			twl4030_codec_mute(codec, 1);
		break;
	case SND_SOC_BIAS_STANDBY:
		twl4030_power_up(codec);
		if (twl4030->bypass_state)
			twl4030_codec_mute(codec, 0);
		else
			twl4030_codec_mute(codec, 1);
		break;
	case SND_SOC_BIAS_OFF:
		twl4030_power_down(codec);
		break;
	}
	codec->bias_level = level;

	return 0;
}

static unsigned int twl4030_rate_min(struct substream_item *item,
				unsigned int rate)
{
	static const unsigned int table[] = {
			8000, 11025, 12000, 16000, 22050,
			24000, 32000, 44100, 48000, 96000};
	unsigned int value = rate;

	if (item->use256FS) {
		int i;
		rate *= 256;
		for (i = 0; i < ARRAY_SIZE(table); i++)
			if (rate % table[i] == 0) {
				value = table[i];
				break;
			}
	}
	return value;
}

static unsigned int twl4030_rate_max(struct substream_item *item,
				unsigned int rate)
{
	static const unsigned int table[] = {
			96000, 48000, 44100, 32000, 24000,
			22050, 16000, 12000, 11025, 8000};
	unsigned int value = rate;

	if (item->use256FS) {
		int i;
		rate *= 256;
		for (i = 0; i < ARRAY_SIZE(table); i++)
			if (rate % table[i] == 0) {
				value = table[i];
				break;
			}
	}
	return value;
}

static void twl4030_constraints(struct twl4030_priv *twl4030)
{
	struct substream_item *item = NULL;
	unsigned int value = 0;

	list_for_each_entry(item, &twl4030->started_list, started) {

		/* Set the constraints according to
		 * the already configured stream
		 */
		value = params_rate(&twl4030->params);
		if (value)
			snd_pcm_hw_constraint_minmax(item->substream->runtime,
					SNDRV_PCM_HW_PARAM_RATE,
					twl4030_rate_min(item, value),
					twl4030_rate_max(item, value));

		value = hw_param_interval(&twl4030->params,
					SNDRV_PCM_HW_PARAM_SAMPLE_BITS)->min;
		if (value && !item->use256FS)
			snd_pcm_hw_constraint_minmax(item->substream->runtime,
					SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
					value, value);
	}
}

/* In case of 4 channel mode, the RX1 L/R for playback and the TX2 L/R for
 * capture has to be enabled/disabled. */
static void twl4030_tdm_enable(struct snd_soc_codec *codec, int direction,
				int enable)
{
	u8 reg, mask;

	reg = twl4030_read_reg_cache(codec, TWL4030_REG_OPTION);

	if (direction == SNDRV_PCM_STREAM_PLAYBACK)
		mask = TWL4030_ARXL1_VRX_EN | TWL4030_ARXR1_EN;
	else
		mask = TWL4030_ATXL2_VTXL_EN | TWL4030_ATXR2_VTXR_EN;

	if (enable)
		reg |= mask;
	else
		reg &= ~mask;

	twl4030_write(codec, TWL4030_REG_OPTION, reg);
}

static int twl4030_new_substream(struct twl4030_priv *twl4030,
		struct snd_pcm_substream *substream, int use256FS)
{
	struct substream_item *item;

	item = kzalloc(sizeof(struct snd_pcm_substream), GFP_KERNEL);
	if (!item)
		return -ENOMEM;
	
	if(substream && (substream->stream==0))
	{
		stream_in_playing=1;
	}
	else if(substream && (substream->stream==1))
	{
		stream_in_recording=1;
	}
	
	item->substream = substream;
	item->use256FS = use256FS;

	mutex_lock(&twl4030->mutex);
	list_add_tail(&item->started, &twl4030->started_list);
	twl4030->extClock += item->use256FS;
	mutex_unlock(&twl4030->mutex);

	return 0;
}

static void twl4030_del_substream(struct twl4030_priv *twl4030,
		struct snd_pcm_substream *substream)
{
	struct substream_item *item = NULL;

	mutex_lock(&twl4030->mutex);

	
	if(substream && (substream->stream==0))
	{
		stream_in_playing=0;
	}
	else if(substream && (substream->stream==1))
	{
		stream_in_recording=0;
	}
	
	list_for_each_entry(item, &twl4030->config_list, configured) {
		if (item->substream == substream) {
			printk(KERN_ERR "TWL4030 deleted substream "
				" still configured!\n");
			list_del(&item->configured);
			break;
		}
	}

	list_for_each_entry(item, &twl4030->started_list, started) {
		if (item->substream == substream) {
			list_del(&item->started);
			twl4030->extClock -= item->use256FS;
			kfree(item);
			break;
		}
	}

	mutex_unlock(&twl4030->mutex);
}

static int twl4030_startup(struct snd_pcm_substream *substream,
			   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;

	if (!(twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE) &
		TWL4030_OPTION_1)) {
		/* In option2 4 channel is not supported, set the
		 * constraint for the first stream for channels, the
		 * second stream will 'inherit' this cosntraint */
		snd_pcm_hw_constraint_minmax(substream->runtime,
					SNDRV_PCM_HW_PARAM_CHANNELS,
					2, 2);
	}

	return twl4030_new_substream(twl4030, substream, 0);
}

static void twl4030_shutdown(struct snd_pcm_substream *substream,
			     struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;

	twl4030_del_substream(twl4030, substream);

	 /* If the closing substream had 4 channel, do the necessary cleanup */
	if (substream->runtime->channels == 4)
		twl4030_tdm_enable(codec, substream->stream, 0);
}

int speaker_amp_en(int enable)
{
	if(enable)
	{
		//gpio_set_value(GPIO_22,1);
		if(cp5860e_speaker_ic == ALC108_IC)
		{	
			alc108_enable_speaker(1);
		}
		else
		{
			wm9093_enable_speaker(1);
		}
	}
	else
	{
		//gpio_set_value(GPIO_22,0);
		if(cp5860e_speaker_ic == ALC108_IC)
		{
			alc108_enable_speaker(0);
		}
		else
		{
			wm9093_enable_speaker(0);
		}
	}
	
	return 0;
}

int ear_speaker_amp_en(int flag)
{
	if(flag == 0)
	{
		//turn off ear and speaker amplifier
		headset_amp_en(0);
		speaker_amp_en(0);	
	}
	else if(flag == 1)
	{
		//turn on ear and speaker amplifier
		headset_amp_en(1);
		speaker_amp_en(1);			
	}

	return 0;
}

int ear_amp_en(int flag)
{
	struct snd_ctl_elem_value uctrl;

	memset(&uctrl,0,sizeof(struct snd_ctl_elem_value));

	if(flag == 0)
	{
		uctrl.value.integer.value[0] = 0;		
		uctrl.value.integer.value[1] = 0;
	}
	else if(flag == 1)
	{
		uctrl.value.integer.value[0] = 1;		
		uctrl.value.integer.value[1] = 1;
	}

	snd_soc_put_gpio(NULL,&uctrl);
	
	return 0;
}

int twl4030_set_rate(struct snd_soc_codec *codec,
		   struct snd_pcm_hw_params *params)
{
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 mode, old_mode;

	if (params_rate(&twl4030->params) &&
		params_rate(&twl4030->params) != params_rate(params)) {
		return -EBUSY;
	}

	/* bit rate */
	old_mode = twl4030_read_reg_cache(codec,
			TWL4030_REG_CODEC_MODE) & ~TWL4030_CODECPDZ;
	mode = old_mode & ~TWL4030_APLL_RATE;

	switch (params_rate(params)) {
	case 8000:
		mode |= TWL4030_APLL_RATE_8000;
		break;
	case 11025:
		mode |= TWL4030_APLL_RATE_11025;
		break;
	case 12000:
		mode |= TWL4030_APLL_RATE_12000;
		break;
	case 16000:
		mode |= TWL4030_APLL_RATE_16000;
		break;
	case 22050:
		mode |= TWL4030_APLL_RATE_22050;
		break;
	case 24000:
		mode |= TWL4030_APLL_RATE_24000;
		break;
	case 32000:
		mode |= TWL4030_APLL_RATE_32000;
		break;
	case 44100:
		mode |= TWL4030_APLL_RATE_44100;
		break;
	case 48000:
		mode |= TWL4030_APLL_RATE_48000;
		break;
	case 96000:
		mode |= TWL4030_APLL_RATE_96000;
		break;
	default:
		printk(KERN_ERR "TWL4030 hw params: unknown rate %d\n",
				params_rate(params));
		return -EINVAL;
	}

	params_rate(&twl4030->params) = params_rate(params);

	if (mode != old_mode) {

		/* change rate and set CODECPDZ */
		twl4030_codec_enable(codec, 0);
		twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
		twl4030_codec_enable(codec, 1);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(twl4030_set_rate);

int twl4030_get_clock_divisor(struct snd_soc_codec *codec,
		   struct snd_pcm_hw_params *params)
{
	struct twl4030_priv *twl4030 = codec->private_data;
	int clock, divisor;

	clock = params_rate(&twl4030->params) * 256;
	divisor = clock / params_rate(params);
	divisor /= params_channels(params);

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_U8:
	case SNDRV_PCM_FORMAT_S8:
		divisor /= 8;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		divisor /= 16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		divisor /= 24;
		break;
	default:
		printk(KERN_ERR "TWL4030 get_clock_divisor: unknown format %d\n",
				params_format(params));
		return -EINVAL;
	}

	return divisor;
}
EXPORT_SYMBOL_GPL(twl4030_get_clock_divisor);

static int twl4030_set_format(struct snd_soc_codec *codec,
		   struct snd_pcm_hw_params *params)
{
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 format, old_format;

	/* sample size */
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
	format = old_format & ~TWL4030_DATA_WIDTH;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		format |= TWL4030_DATA_WIDTH_16S_16W;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		format |= TWL4030_DATA_WIDTH_32S_24W;
		break;
	default:
		printk(KERN_ERR "TWL4030 hw params: unknown format %d\n",
				params_format(params));
		return -EINVAL;
	}

	if (format == old_format)
		return 0;

	if (params_format(&twl4030->params) &&
		params_format(&twl4030->params) != params_format(params))
		return -EBUSY;

	*hw_param_mask(&twl4030->params, SNDRV_PCM_HW_PARAM_FORMAT) =
		*hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT);

	/* clear CODECPDZ before changing format (codec requirement) */
	twl4030_codec_enable(codec, 0);

	/* change format */
	twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);

	/* set CODECPDZ afterwards */
	twl4030_codec_enable(codec, 1);

	return 0;
}

static int twl4030_hw_params(struct snd_pcm_substream *substream,
			   struct snd_pcm_hw_params *params,
			   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	int rval;

	mutex_lock(&twl4030->mutex);

	 /* If the substream has 4 channel, do the necessary setup */
	if (params_channels(params) == 4) {
		/* Safety check: are we in the correct operating mode? */
		if ((twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE) &
			TWL4030_OPTION_1)) {
			twl4030_tdm_enable(codec, substream->stream, 1);
		} else {
			mutex_unlock(&twl4030->mutex);
			return -EINVAL;
		}
	}

	rval = twl4030_set_rate(codec, params);
	if (rval < 0) {
		mutex_unlock(&twl4030->mutex);
		return rval;
	}

	rval = twl4030_set_format(codec, params);
	if (rval < 0) {
		mutex_unlock(&twl4030->mutex);
		return rval;
	}

	/* If any other streams are currently open, and one of them
	 * is setting the hw parameters right now (since we are here), set
	 * constraints to the other stream(s) to match the current one. */
	twl4030_constraints(twl4030);

	mutex_unlock(&twl4030->mutex);

	return 0;
}

static int twl4030_hw_prepare(struct snd_pcm_substream *substream,
			struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct twl4030_setup_data *setup = socdev->codec_data;
	/*
	 * Set headset EXTMUTE signal to ON to make sure we
	 * get correct headset status
	 */

	if (setup && setup->hs_extmute) {
		if (setup->set_hs_extmute)
			setup->set_hs_extmute(1);
	}

	return 0;
}

static int twl4030_hw_free(struct snd_pcm_substream *substream,
			struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	struct substream_item *item = NULL;

	mutex_lock(&twl4030->mutex);

	list_for_each_entry(item, &twl4030->config_list, configured) {
		if (item->substream == substream) {
			list_del (&item->configured);
			break;
		}
	}

	if (list_empty(&twl4030->config_list))
		memset(&twl4030->params, 0, sizeof(twl4030->params));

	mutex_unlock(&twl4030->mutex);

	return 0;
}

static int twl4030_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 infreq;

	switch (freq) {
	case 19200000:
		infreq = TWL4030_APLL_INFREQ_19200KHZ;
		twl4030->sysclk = 19200;
		break;
	case 26000000:
		infreq = TWL4030_APLL_INFREQ_26000KHZ;
		twl4030->sysclk = 26000;
		break;
	case 38400000:
		infreq = TWL4030_APLL_INFREQ_38400KHZ;
		twl4030->sysclk = 38400;
		break;
	default:
		printk(KERN_ERR "TWL4030 set sysclk: unknown rate %d\n",
			freq);
		return -EINVAL;
	}

	infreq |= TWL4030_APLL_EN;
	twl4030_write(codec, TWL4030_REG_APLL_CTL, infreq);

	return 0;
}

int twl4030_set_ext_clock(struct snd_soc_codec *codec, int enable)
{
	u8 old_format, format;

	/* get format */
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);

	if (enable || smartphone_calling_enable)
		format = old_format | TWL4030_CLK256FS_EN;
	else
		format = old_format & ~TWL4030_CLK256FS_EN;

	if (format != old_format) {

		/* clear CODECPDZ before changing format (codec requirement) */
		twl4030_codec_enable(codec, 0);

		/* change format */
		twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);

		/* set CODECPDZ afterwards */
		twl4030_codec_enable(codec, 1);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(twl4030_set_ext_clock);

static int twl4030_set_dai_fmt(struct snd_soc_dai *codec_dai,
			     unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	int use256FS = 0;
	u8 old_format, format;

	/* get format */
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
	format = old_format;

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		format &= ~(TWL4030_AIF_SLAVE_EN);
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		format |= TWL4030_AIF_SLAVE_EN;
		use256FS = 1;
		break;
	default:
		return -EINVAL;
	}

	/* interface format */
	format &= ~TWL4030_AIF_FORMAT;
	format |= TWL4030_CLK256FS_EN;
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		format |= TWL4030_AIF_FORMAT_CODEC;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		format |= TWL4030_AIF_FORMAT_TDM;
		break;
	default:
		return -EINVAL;
	}

	if (format != old_format) {

		/* clear CODECPDZ before changing format (codec requirement) */
		twl4030_codec_enable(codec, 0);

		/* change format */
		twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);

		/* set CODECPDZ afterwards */
		twl4030_codec_enable(codec, 1);
	}

	return twl4030_set_ext_clock(codec, use256FS | twl4030->extClock | smartphone_calling_enable);
}

static int twl4030_set_tristate(struct snd_soc_dai *dai, int tristate)
{
	struct snd_soc_codec *codec = dai->codec;
	u8 reg = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);

	if (tristate)
		reg |= TWL4030_AIF_TRI_EN;
	else
		reg &= ~TWL4030_AIF_TRI_EN;

	return twl4030_write(codec, TWL4030_REG_AUDIO_IF, reg);
}

/* In case of voice mode, the RX1 L(VRX) for downlink and the TX2 L/R
 * (VTXL, VTXR) for uplink has to be enabled/disabled. */
static void twl4030_voice_enable(struct snd_soc_codec *codec, int direction,
				int enable)
{
	u8 reg, mask;

	reg = twl4030_read_reg_cache(codec, TWL4030_REG_OPTION);

	if (direction == SNDRV_PCM_STREAM_PLAYBACK)
		mask = TWL4030_ARXL1_VRX_EN;
	else
		mask = TWL4030_ATXL2_VTXL_EN | TWL4030_ATXR2_VTXR_EN;

	if (enable)
		reg |= mask;
	else
		reg &= ~mask;

	twl4030_write(codec, TWL4030_REG_OPTION, reg);
}

static int twl4030_voice_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 infreq;
	u8 mode;

	/* If the system master clock is not 26MHz, the voice PCM interface is
	 * not avilable.
	 */
	infreq = twl4030_read_reg_cache(codec, TWL4030_REG_APLL_CTL)
		& TWL4030_APLL_INFREQ;

	if (infreq != TWL4030_APLL_INFREQ_26000KHZ) {
		printk(KERN_ERR "TWL4030 voice startup: "
			"MCLK is not 26MHz, call set_sysclk() on init\n");
		return -EINVAL;
	}

	/* If the codec mode is not option2, the voice PCM interface is not
	 * avilable.
	 */
	mode = twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE)
		& TWL4030_OPT_MODE;

	if (mode != TWL4030_OPTION_2) {
		printk(KERN_ERR "TWL4030 voice startup: "
			"the codec mode is not option2\n");
		return -EINVAL;
	}

	return twl4030_new_substream(twl4030, substream, 1);
}

static void twl4030_voice_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;

	twl4030_del_substream(twl4030, substream);

	/* Enable voice digital filters */
	twl4030_voice_enable(codec, substream->stream, 0);
}

static int twl4030_voice_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;

	u8 old_mode, mode;

	/* Enable voice digital filters */
	twl4030_voice_enable(codec, substream->stream, 1);

	mutex_lock(&twl4030->mutex);

	/* bit rate */
	old_mode = twl4030_read_reg_cache(codec,
			TWL4030_REG_CODEC_MODE) & ~TWL4030_CODECPDZ;
	mode = old_mode & ~TWL4030_APLL_RATE;

#if 1
	//Nina added to test arecord
	mode = old_mode &~TWL4030_OPT_MODE;
#endif

	switch (params_rate(params)) {
	case 8000:
		mode &= ~(TWL4030_SEL_16K);
		break;
	case 16000:
		mode |= TWL4030_SEL_16K;
		break;
	default:
		mutex_unlock(&twl4030->mutex);
		printk(KERN_ERR "TWL4030 voice hw params: unknown rate %d\n",
			params_rate(params));
		return -EINVAL;
	}

	if (mode != old_mode) {
		/* change rate and set CODECPDZ */
		twl4030_codec_enable(codec, 0);
		twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
		twl4030_codec_enable(codec, 1);
	}

	mutex_unlock(&twl4030->mutex);
	return 0;
}

static int twl4030_voice_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u8 infreq;

	switch (freq) {
	case 26000000:
		infreq = TWL4030_APLL_INFREQ_26000KHZ;
		break;
	default:
		printk(KERN_ERR "TWL4030 voice set sysclk: unknown rate %d\n",
			freq);
		return -EINVAL;
	}

	infreq |= TWL4030_APLL_EN;
	twl4030_write(codec, TWL4030_REG_APLL_CTL, infreq);

	return 0;
}

static int twl4030_voice_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	int use256FS = 0;
	u8 old_format, format;

	/* get format */
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_VOICE_IF);
	format = old_format & ~TWL4030_VIF_TRI_EN;

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFM:
		format &= ~(TWL4030_VIF_SLAVE_EN);
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		format |= TWL4030_VIF_SLAVE_EN;
		use256FS = 1;
		break;
	default:
		return -EINVAL;
	}

	/* clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_IB_NF:
		format &= ~(TWL4030_VIF_FORMAT);
		break;
	case SND_SOC_DAIFMT_NB_IF:
		format |= TWL4030_VIF_FORMAT;
		break;
	default:
		return -EINVAL;
	}

	if (format != old_format) {
		/* change format and set CODECPDZ */
		twl4030_codec_enable(codec, 0);
		twl4030_write(codec, TWL4030_REG_VOICE_IF, format);
		twl4030_codec_enable(codec, 1);
	}

	return twl4030_set_ext_clock(codec, use256FS | twl4030->extClock | smartphone_calling_enable);
}

static int twl4030_clock_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;

	return twl4030_new_substream(twl4030, substream, 1);
}

static int twl4030_clock_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	int rval;

	mutex_lock(&twl4030->mutex);

	rval = twl4030_set_rate(codec, params);

	/* See if we are a multiple of the current FS. If so, then still OK. */
	if (rval) {
		int divisor = twl4030_get_clock_divisor(codec, params);
		int clock = params_rate(&twl4030->params) * 256;
		int remainder = clock % params_rate(params);

		if (remainder == 0 && divisor <= 256)
			rval = 0;
	}

	/* If any other streams are currently open, and one of them
	 * is setting the hw parameters right now (since we are here), set
	 * constraints to the other stream(s) to match the current one. */
	twl4030_constraints(twl4030);

	mutex_unlock(&twl4030->mutex);

	return rval;
}

static int twl4030_clock_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;

	return twl4030_set_ext_clock(codec, 1);
}

static int twl4030_voice_set_tristate(struct snd_soc_dai *dai, int tristate)
{
	struct snd_soc_codec *codec = dai->codec;
	u8 reg = twl4030_read_reg_cache(codec, TWL4030_REG_VOICE_IF);

	if (tristate)
		reg |= TWL4030_VIF_TRI_EN;
	else
		reg &= ~TWL4030_VIF_TRI_EN;

	return twl4030_write(codec, TWL4030_REG_VOICE_IF, reg);
}

#define TWL4030_RATES	 (SNDRV_PCM_RATE_8000_48000)
#define TWL4030_FORMATS	 (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FORMAT_S24_LE)

static struct snd_soc_dai_ops twl4030_dai_ops = {
	.startup	= twl4030_startup,
	.shutdown	= twl4030_shutdown,
	.prepare	= twl4030_hw_prepare,
	.hw_params	= twl4030_hw_params,
	.hw_free	= twl4030_hw_free,
	.set_sysclk	= twl4030_set_dai_sysclk,
	.set_fmt	= twl4030_set_dai_fmt,
	.set_tristate	= twl4030_set_tristate,
};

static struct snd_soc_dai_ops twl4030_dai_voice_ops = {
	.startup	= twl4030_voice_startup,
	.shutdown	= twl4030_voice_shutdown,
	.hw_params	= twl4030_voice_hw_params,
	.hw_free	= twl4030_hw_free,
	.set_sysclk	= twl4030_voice_set_dai_sysclk,
	.set_fmt	= twl4030_voice_set_dai_fmt,
	.set_tristate	= twl4030_voice_set_tristate,
};

static struct snd_soc_dai_ops twl4030_dai_clock_ops = {
	.startup = twl4030_clock_startup,
	.shutdown = twl4030_shutdown,
	.hw_params = twl4030_clock_hw_params,
	.hw_free = twl4030_hw_free,
	.set_sysclk = twl4030_set_dai_sysclk,
	.set_fmt = twl4030_clock_set_dai_fmt,
};

struct snd_soc_dai twl4030_dai[] = {
{
	.name = "twl4030",
	.playback = {
		.stream_name = "HiFi Playback",
		.channels_min = 2,
		.channels_max = 4,
		/*.rates = TWL4030_RATES | SNDRV_PCM_RATE_96000, */
		.rates = SNDRV_PCM_RATE_44100,
		.formats = TWL4030_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 2,
		.channels_max = 4,
		/*.rates = TWL4030_RATES, */
		.rates = SNDRV_PCM_RATE_44100,
		.formats = TWL4030_FORMATS,},
	.ops = &twl4030_dai_ops,
},
{
	.name = "twl4030 Voice",
	.playback = {
		.stream_name = "Voice Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = &twl4030_dai_voice_ops,
},
{
	.name = "twl4030 Clock",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = TWL4030_RATES,
		.formats = SNDRV_PCM_FMTBIT_U8 | TWL4030_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = TWL4030_RATES,
		.formats = SNDRV_PCM_FMTBIT_U8 | TWL4030_FORMATS,},
	.ops = &twl4030_dai_clock_ops,
},
};
EXPORT_SYMBOL_GPL(twl4030_dai);

int get_twl4030_hsmic_status(void)
{	
	return hsmic_bias_opened;
}

void set_twl4030_hsmic_status(int status)
{
	struct snd_soc_codec *codec = twl4030_socdev->card->codec;
	unsigned char micbias_ctl = 0;

    twl_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &micbias_ctl,TWL4030_REG_MICBIAS_CTL);					    			    

	if (status == 1) {
		yl_debug("before micbias_ctl=%d, status=%d, in %s func\n", micbias_ctl, status, __FUNCTION__);
		micbias_ctl = twl4030_read_reg_cache(codec, TWL4030_REG_MICBIAS_CTL);
		micbias_ctl |= TWL4030_HSMICBIAS_EN;
		twl4030_write(codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);
		yl_debug("after micbias_ctl=%d, status=%d, in %s func\n", micbias_ctl, status, __FUNCTION__);
		hsmic_bias_opened=1;
	} else {
		yl_debug("before micbias_ctl=%d, status=%d, in %s func\n", micbias_ctl, status, __FUNCTION__);		
		micbias_ctl = twl4030_read_reg_cache(codec, TWL4030_REG_MICBIAS_CTL);
		micbias_ctl &= (~TWL4030_HSMICBIAS_EN);
		twl4030_write(codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);
		yl_debug("after micbias_ctl=%d, status=%d, in %s func\n", micbias_ctl, status, __FUNCTION__);
		hsmic_bias_opened=0;
	}
}

EXPORT_SYMBOL_GPL(get_twl4030_hsmic_status);
EXPORT_SYMBOL_GPL(set_twl4030_hsmic_status);

#ifdef CONFIG_KEYBOARD_HEADSET_KEY
extern int zoom2_headset_key_resume(void);
extern int zoom2_headset_key_suspend(void);
#endif

#ifdef CHECK_SUSPEND_REG
static void twl4030_dump_chip(u8 * buf)
{
	int i;

	for (i = 0; i < TWL4030_REG_SW_SHADOW; i++)
		twl_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE,&buf[i],i);

}
#endif
static int twl4030_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	if(smartphone_calling_enable == 0)
	{

        yl_debug("enter %s func, before execute twl4030_set_bias_level(), location 1\n",__FUNCTION__);
        ear_speaker_amp_en(0);

        #ifdef CONFIG_KEYBOARD_HEADSET_KEY
        zoom2_headset_key_suspend();
        #endif

		twl4030_set_bias_level(codec, SND_SOC_BIAS_OFF);
		
		hsmic_bias_opened = 0;
	}
	else 
	{

    #ifdef	CHECK_SUSPEND_REG
	 twl4030_dump_chip(twl4030_reg_suspend);
	#endif	

		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x92,TWL4030_REG_ARXL2_APGA_CTL);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, 0x92,TWL4030_REG_ARXR2_APGA_CTL);

	}


	return 0;
}

static int twl4030_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;
    
	unsigned char micbias_ctl = 0;
    u8 *cache = codec->reg_cache;
	if(smartphone_calling_enable == 0)	// required to avoid headset irqs
	{
		yl_debug("enter %s func, before execute twl4030_set_bias_level(), location 1\n",__FUNCTION__);

	    twl4030_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	    twl4030_set_bias_level(codec, codec->suspend_bias_level);
		
		twl4030_gpio2_config();
		twl4030_gpio1_config();
		//twl4030_gpio6_config();
		
		//it need always enable the TWL4030_HSMICBIAS_EN
		//modified by guotao, 2010-01-14
		if (get_headset_plug_status()==0)
		{
			printk("gpio_get_value(GPIO_HEAD_DET)=%d\n", gpio_get_value(GPIO_HEAD_DET));
			micbias_ctl = twl4030_read_reg_cache(codec, TWL4030_REG_MICBIAS_CTL);
			micbias_ctl |= TWL4030_HSMICBIAS_EN;
			twl4030_write(codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);
			//mdelay(50);
		
			hsmic_bias_opened = 1;
		}
		else
		{
			printk("gpio_get_value(GPIO_HEAD_DET)=%d\n", gpio_get_value(GPIO_HEAD_DET));
			hsmic_bias_opened = 0;
		}
		#ifdef CONFIG_KEYBOARD_HEADSET_KEY
		zoom2_headset_key_resume();
		#endif
		
	}
	
	else 
	{
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, cache[TWL4030_REG_ARXL2_APGA_CTL],TWL4030_REG_ARXL2_APGA_CTL);
		twl_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, cache[TWL4030_REG_ARXR2_APGA_CTL],TWL4030_REG_ARXR2_APGA_CTL);

	#ifdef CHECK_SUSPEND_REG
	    int temp;	    
   
	    twl4030_dump_chip(twl4030_reg_resume);
	    for(temp=0;temp<TWL4030_REG_SW_SHADOW;temp++)
	    {
            if(twl4030_reg_suspend[temp]!=twl4030_reg_resume[temp])
            {
                printk(KERN_ERR "%d:change [%d] --> [%d] \n",temp,twl4030_reg_suspend[temp],twl4030_reg_resume[temp]);
            }
            if(twl4030_reg_suspend[temp]!=cache[temp])
            {
                printk(KERN_ERR "cache:%d:change [%d] --> [%d] \n",temp,twl4030_reg_suspend[temp],cache[temp]);
            }
	    }
	#endif
	}

	
	return 0;
}


static int twl4030_gpio2_config(void)
{
	unsigned char rd_data = 0;

//set twl5030 gpio2 input mode, and pull it up
//modified by guotao , 2010-01-14
#define TWL5030_GPIO_ON_SHIFT 	(1 << 2)
#define GPIO2_DEBOUNCE_EN		(1 << 2)
#define GPIO2_INPUT_MODE		(1 << 2)
#define GPIO2_PULL_UP_EN		(1 << 5)
#define GPIO2_PULL_DOWN_EN		(1 << 4)

	/*enable twl5030 the GPIO module */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIO_CTRL))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 1 \n",__FUNCTION__);
		goto pcm_err;
	}
	rd_data |= TWL5030_GPIO_ON_SHIFT;
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIO_CTRL);
	
	
	/*set twl5030 GPIO2 input mode */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIODATADIR1))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 2 \n",__FUNCTION__);
		goto pcm_err;
	}

	rd_data &= (~GPIO2_INPUT_MODE);
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIODATADIR1);


	/*pullup twl5030 GPIO2 pin */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIOPUPDCTR1))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 3 \n",__FUNCTION__);
		goto pcm_err;
	}
	rd_data |= GPIO2_PULL_UP_EN;
	rd_data &= (~GPIO2_PULL_DOWN_EN);
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIOPUPDCTR1);
	

	/*enable twl5030 GPIO2 debounce */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIO_DEBEN1))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 4 \n",__FUNCTION__);
		goto pcm_err;
	}
	rd_data |= GPIO2_DEBOUNCE_EN;
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIO_DEBEN1);

	//hsmic_bias_opened = 1;
	return 0;

pcm_err:
	//hsmic_bias_opened = 0;
	return -1;

}


static int twl4030_gpio1_config(void)
{
	unsigned char rd_data = 0;

//set twl5030 gpio1 input mode, and pull it up
//modified by guotao , 2010-01-14
#define TWL5030_GPIO1_ON_SHIFT 	(1 << 2)
#define GPIO1_DEBOUNCE_EN		(1 << 1)
#define GPIO1_INPUT_MODE		(1 << 1)
#define GPIO1_PULL_UP_EN		(1 << 3)
#define GPIO1_PULL_DOWN_EN		(1 << 2)

	/*enable twl5030 the GPIO module */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIO_CTRL))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 1 \n",__FUNCTION__);
	}
	rd_data |= TWL5030_GPIO1_ON_SHIFT;
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIO_CTRL);
	
	
	/*set twl5030 GPIO2 input mode */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIODATADIR1))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 2 \n",__FUNCTION__);
	}

	rd_data &= (~GPIO1_INPUT_MODE);
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIODATADIR1);


	/*pullup twl5030 GPIO2 pin */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIOPUPDCTR1))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 3 \n",__FUNCTION__);
	}
	rd_data |= GPIO1_PULL_UP_EN;
	rd_data &= (~GPIO1_PULL_DOWN_EN);
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIOPUPDCTR1);
	

	/*enable twl5030 GPIO2 debounce */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIO_DEBEN1))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 4 \n",__FUNCTION__);
	}
	rd_data |= GPIO1_DEBOUNCE_EN;
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIO_DEBEN1);

	return 0;
}

static int twl4030_gpio6_config(void)
{
	unsigned char rd_data = 0;

//set twl5030 gpio2 input mode, and pull it up
//modified by guotao , 2010-01-14
#define TWL5030_GPIO6_ON_SHIFT 	(1 << 2)
#define GPIO6_DEBOUNCE_EN		(1 << 6)
#define GPIO6_INPUT_MODE		(1 << 6)
#define GPIO6_PULL_UP_EN		(1 << 5)
#define GPIO6_PULL_DOWN_EN		(1 << 4)

	/*enable twl5030 the GPIO module */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIO_CTRL))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 1 \n",__FUNCTION__);
	}
	rd_data |= TWL5030_GPIO6_ON_SHIFT;
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIO_CTRL);
	
	
	/*set twl5030 GPIO2 input mode */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIODATADIR1))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 2 \n",__FUNCTION__);
	}
	rd_data &= (~GPIO6_INPUT_MODE);
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIODATADIR1);


	/*pullup twl5030 GPIO2 pin */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIOPUPDCTR2))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 3 \n",__FUNCTION__);
	}
	rd_data |= GPIO6_PULL_UP_EN;
	rd_data &= (~GPIO6_PULL_DOWN_EN);
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIOPUPDCTR2);
	

	/*enable twl5030 GPIO2 debounce */
	if (0 != twl_i2c_read_u8(TWL4030_MODULE_GPIO, &rd_data, REG_GPIO_DEBEN1))
	{   /*error*/
		printk(KERN_ERR "read i2c error for in %s func, location 4 \n",__FUNCTION__);
	}
	rd_data |= GPIO2_DEBOUNCE_EN;
	twl_i2c_write_u8(TWL4030_MODULE_GPIO, rd_data, REG_GPIO_DEBEN1);

	return 0;
}

/*
 * initialize the driver
 * register the mixer and dsp interfaces with the kernel
 */

static int twl4030_init(struct snd_soc_device *socdev)
{
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_setup_data *setup = socdev->codec_data;
	struct twl4030_priv *twl4030 = codec->private_data;
	int ret = 0;

	printk(KERN_INFO "TWL4030 Audio Codec init \n");

	codec->name = "twl4030";
	codec->owner = THIS_MODULE;
	codec->read = twl4030_read_reg_cache;
	codec->write = twl4030_write;
	codec->set_bias_level = twl4030_set_bias_level;
	codec->dai = twl4030_dai;
	codec->num_dai = ARRAY_SIZE(twl4030_dai),
	codec->reg_cache_size = sizeof(twl4030_reg);
	codec->reg_cache = kmemdup(twl4030_reg, sizeof(twl4030_reg),
					GFP_KERNEL);
	if (codec->reg_cache == NULL)
		return -ENOMEM;

	/* Configuration for headset ramp delay from setup data */
	if (setup) {
		unsigned char hs_pop;

		if (setup->sysclk)
			twl4030->sysclk = setup->sysclk;
		else
			twl4030->sysclk = 26000;

		hs_pop = twl4030_read_reg_cache(codec, TWL4030_REG_HS_POPN_SET);
		hs_pop &= ~TWL4030_RAMP_DELAY;
		hs_pop |= (setup->ramp_delay_value << 2);
		twl4030_write_reg_cache(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
	} else {
		twl4030->sysclk = 26000;
	}

	/* register pcms */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		printk(KERN_ERR "twl4030: failed to create pcms\n");
		goto pcm_err;
	}

	//Nicho add 2009-11-30
	//GPIO_97 : Headset PA EN (High Active)
	//omap_cfg_reg(C23_34XX_CAM_FLD); 		//move this to board-zoom2.c ,guotao
	//gpio_request(GPIO_97, "Headset PA EN");
	//gpio_direction_output(GPIO_97, 0); //Set as Low, disable headset amplifier
	if(cp5860e_speaker_ic == ALC108_IC)
	{
		alc108_enable_earphone(0);
	}
	else
	{
		wm9093_enable_earphone(0);
	}
	
	
	//GPIO_22 : Speaker PA EN (High Active)
	//omap_cfg_reg(B26_34XX_GPIO111); 		//move this to board-zoom2.c ,guotao
	//gpio_request(GPIO_22, "Speaker PA EN");
	//gpio_direction_output(GPIO_22, 0);//Set as low,disable speaker amplifier
	if(cp5860e_speaker_ic == ALC108_IC)
	{
		alc108_enable_speaker(0);
	}
	else
	{
		wm9093_enable_speaker(0);
	}
	
	
	//GPIO_6 : MIC select (High: local mic, Low: headset mic)
	//gpio_request(GPIO_6, "MIC select");
	//gpio_direction_output(GPIO_6, 1);//Set as high,initial for local mic
	
#ifdef TEST_PCM_PHONE
	gpio_request(GPIO_2, "TD_PCM_EN");		//modified by guotao, 2010-01-26
	gpio_set_value(GPIO_2, 0);		//enable PCM channel
#endif
	
	twl4030_gpio2_config();
	twl4030_gpio1_config();
	//twl4030_gpio6_config();
	
#if 0 //def SUPPORT_UART_SEL
	//Larry add 2009-12-24
	//GPIO23, GPIO186: for MAX4782 select, init for UART connect	
	gpio_request(23, "uart_sel0");
	gpio_request(186, "uart_sel1");
	gpio_direction_output(23, 0);
	gpio_direction_output(186, 0);
#endif

	twl4030_init_chip(codec);

	/* power on device */
	twl4030_set_bias_level(codec, SND_SOC_BIAS_STANDBY);

	snd_soc_add_controls(codec, twl4030_snd_controls,
				ARRAY_SIZE(twl4030_snd_controls));
	twl4030_add_widgets(codec);

	ret = snd_soc_init_card(socdev);
	if (ret < 0) {
		printk(KERN_ERR "twl4030: failed to register card\n");
		goto card_err;
	}

	return ret;

card_err:
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
pcm_err:
	kfree(codec->reg_cache);
	return ret;
}

//static struct snd_soc_device *twl4030_socdev;

static int twl4030_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec;
	struct twl4030_priv *twl4030;

	codec = kzalloc(sizeof(struct snd_soc_codec), GFP_KERNEL);
	if (codec == NULL)
		return -ENOMEM;

	twl4030 = kzalloc(sizeof(struct twl4030_priv), GFP_KERNEL);
	if (twl4030 == NULL) {
		kfree(codec);
		return -ENOMEM;
	}

	mutex_init(&twl4030->mutex);
	INIT_LIST_HEAD(&twl4030->started_list);
	INIT_LIST_HEAD(&twl4030->config_list);
	codec->private_data = twl4030;
	socdev->card->codec = codec;
	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	twl4030_socdev = socdev;
	twl4030_init(socdev);

	return 0;
}

static int twl4030_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	printk(KERN_INFO "TWL4030 Audio Codec remove\n");
	twl4030_set_bias_level(codec, SND_SOC_BIAS_OFF);
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
	kfree(codec->private_data);
	kfree(codec);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_twl4030 = {
	.probe = twl4030_probe,
	.remove = twl4030_remove,
	.suspend = twl4030_suspend,
	.resume = twl4030_resume,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_twl4030);

static int __init twl4030_modinit(void)
{
	return snd_soc_register_dais(twl4030_dai, ARRAY_SIZE(twl4030_dai));
}
module_init(twl4030_modinit);

static void __exit twl4030_exit(void)
{
	snd_soc_unregister_dais(twl4030_dai, ARRAY_SIZE(twl4030_dai));
}
module_exit(twl4030_exit);

MODULE_DESCRIPTION("ASoC TWL4030 codec driver");
MODULE_AUTHOR("Steve Sakoman");
MODULE_LICENSE("GPL");

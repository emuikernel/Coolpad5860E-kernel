/*
 * zoom2.c  --  SoC audio for Zoom2
 *
 * Author: Misael Lopez Cruz <x0052729@ti.com>
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

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>

#include <plat/mcbsp.h>
#include <plat/mux.h>
#include <plat/omap-pm.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"
#include "../codecs/twl4030.h"

#include "../../../arch/arm/mach-omap2/mux.h"

#define OMAP_MCBSP_MASTER_MODE	0

#define ZOOM2_BT_MCBSP_GPIO		164
//#define ZOOM2_HEADSET_MUX_GPIO		(OMAP_MAX_GPIO_LINES + 15)
//#define ZOOM2_HEADSET_EXTMUTE_GPIO	153    //modified by yuanyufang, 2010-12-25
static int zoom2_twl4030_active;

/* McBSP mux config group */
enum {
	ZOOM2_ASOC_MUX_MCBSP2_SLAVE = 0,
	ZOOM2_ASOC_MUX_MCBSP2_MASTER,
	ZOOM2_ASOC_MUX_MCBSP3_SLAVE,
	ZOOM2_ASOC_MUX_MCBSP3_MASTER,
	ZOOM2_ASOC_MUX_MCBSP3_TRISTATE,
	ZOOM2_ASOC_MUX_MCBSP4_SLAVE,
	ZOOM2_ASOC_MUX_MCBSP4_MASTER,
};


static int zoom2_asoc_mux_config(int group)
{
	switch (group) {
	case ZOOM2_ASOC_MUX_MCBSP2_SLAVE:
		omap_mux_init_signal("mcbsp2_fsx.mcbsp2_fsx",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp2_clkx.mcbsp2_clkx",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp2_dr.mcbsp2_dr",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp2_dx.mcbsp2_dx",
						OMAP_PIN_OUTPUT);
		break;

	case ZOOM2_ASOC_MUX_MCBSP3_SLAVE:
		omap_mux_init_signal("mcbsp3_fsx.mcbsp3_fsx",
						OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("mcbsp3_clkx.mcbsp3_clkx",
						OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("mcbsp3_dr.mcbsp3_dr",
						OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("mcbsp3_dx.mcbsp3_dx",
						OMAP_PIN_OUTPUT);
		break;

	case ZOOM2_ASOC_MUX_MCBSP3_MASTER:
		omap_mux_init_signal("mcbsp_clks.mcbsp_clks",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp3_fsx.mcbsp3_fsx",
						OMAP_PIN_OUTPUT);
		omap_mux_init_signal("mcbsp3_clkx.mcbsp3_clkx",
						OMAP_PIN_OUTPUT);
		omap_mux_init_signal("mcbsp3_dr.mcbsp3_dr",
						OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("mcbsp3_dx.mcbsp3_dx",
						OMAP_PIN_OUTPUT);
		break;

	case ZOOM2_ASOC_MUX_MCBSP3_TRISTATE:
		omap_mux_init_signal("mcbsp3_fsx.safe_mode",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp3_clkx.safe_mode",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp3_dr.safe_mode",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp3_dx.safe_mode",
						OMAP_PIN_INPUT);
		break;

	case ZOOM2_ASOC_MUX_MCBSP4_MASTER:
		omap_mux_init_signal("mcbsp_clks.mcbsp_clks",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("mcbsp4_fsx.mcbsp4_fsx",
						OMAP_PIN_OUTPUT);
		omap_mux_init_signal("gpmc_ncs4.mcbsp4_clkx",
						OMAP_PIN_OUTPUT);
		omap_mux_init_signal("gpmc_ncs5.mcbsp4_dr",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("gpmc_ncs6.mcbsp4_dx",
						OMAP_PIN_OUTPUT);
		break;

	case ZOOM2_ASOC_MUX_MCBSP4_SLAVE:
		omap_mux_init_signal("mcbsp4_fsx.mcbsp4_fsx",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("gpmc_ncs4.mcbsp4_clkx",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("gpmc_ncs5.mcbsp4_dr",
						OMAP_PIN_INPUT);
		omap_mux_init_signal("gpmc_ncs6.mcbsp4_dx",
						OMAP_PIN_OUTPUT);
		break;

	default:
		return -ENODEV;
	}
	return 0;
}

static int zoom2_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
					SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	/* enable 256 FS clk for HDMI */
	if (zoom2_twl4030_active == 1) {
		ret = twl4030_set_ext_clock(codec_dai->codec, 1);
		if (ret < 0) {
			printk(KERN_ERR "can't set 256 FS clock\n");
			return ret;
		}
	}

	/* Use external clock for mcBSP2 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_EXT,
			0, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu_dai system clock\n");
		return ret;
	}

	/*
	 * Set headset EXTMUTE signal to ON to make sure we
	 * get correct headset status
	 */
	//gpio_direction_output(ZOOM2_HEADSET_EXTMUTE_GPIO, 1);     //modified by yuanyufang, 2010-12-25

	return 0;
}

static int zoom2_i2s_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	/* Use function clock for mcBSP2 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_FCLK,
			0, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu_dai system clock\n");
		return ret;
	}

	if (zoom2_twl4030_active == 1) {
		ret = twl4030_set_ext_clock(codec_dai->codec, 0);
		if (ret < 0) {
			printk(KERN_ERR "can't set 256 FS clock\n");
			return ret;
		}
	}

	return 0;
}

static struct pm_qos_request_list *qos_request;
static int snd_hw_latency;
extern void omap_dpll3_errat_wa(int disable);
static int zoom2_i2s_startup(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;

	zoom2_twl4030_active++;

	/*
	 * Hold C2 as min latency constraint. Deeper states
	 * MPU RET/OFF is overhead and consume more power than
	 * savings.
	 * snd_hw_latency check takes care of playback and capture
	 * usecase.
	 */
	if (!snd_hw_latency++) {
		omap_pm_set_max_mpu_wakeup_lat(&qos_request, 18);
		/*
		 * As of now for MP3 playback case need to enable dpll3
		 * autoidle part of dpll3 lock errata.
		 * REVISIT: Remove this, Once the dpll3 lock errata is
		 * updated with with a new workaround without impacting mp3 usecase.
		 */
		omap_dpll3_errat_wa(0);
	}

	return 0;
}

static void zoom2_i2s_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;

	zoom2_twl4030_active--;

	/* remove latency constraint */
	snd_hw_latency--;
	if (!snd_hw_latency) {
		omap_pm_set_max_mpu_wakeup_lat(&qos_request, -1);
		omap_dpll3_errat_wa(1);
	}
}

static struct snd_soc_ops zoom2_i2s_ops = {
	.startup = zoom2_i2s_startup,
	.hw_params = zoom2_i2s_hw_params,
	.hw_free = zoom2_i2s_hw_free,
	.shutdown = zoom2_i2s_shutdown,
};

static int zoom2_startup(struct snd_pcm_substream *substream)
{
	zoom2_twl4030_active++;
	return 0;
}

static void zoom2_shutdown(struct snd_pcm_substream *substream)
{
	zoom2_twl4030_active--;
}

static int zoom2_hw_voice_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	zoom2_asoc_mux_config(ZOOM2_ASOC_MUX_MCBSP3_SLAVE);

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
					SND_SOC_DAIFMT_DSP_A |
					SND_SOC_DAIFMT_IB_NF |
					SND_SOC_DAIFMT_CBS_CFM);

	if (ret) {
			printk(KERN_ERR "can't set codec DAI configuration\n");
			return ret;
	}

	/* set codec Voice IF to application mode*/
	ret = snd_soc_dai_set_tristate(codec_dai, 0);

	if (ret) {
			printk(KERN_ERR "can't disable codec VIF tristate\n");
			return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
					SND_SOC_DAIFMT_DSP_A |
					SND_SOC_DAIFMT_IB_NF |
					SND_SOC_DAIFMT_CBM_CFM);

	if (ret < 0) {
			printk(KERN_ERR "can't set cpu DAI configuration\n");
			return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
						SND_SOC_CLOCK_IN);
	if (ret < 0) {
			printk(KERN_ERR "can't set codec system clock\n");
			return ret;
	}

	if (zoom2_twl4030_active == 1) {
		ret = twl4030_set_ext_clock(codec_dai->codec, 1);
		if (ret < 0) {
			printk(KERN_ERR "can't set 256 FS clock\n");
			return ret;
		}
	}

	return 0;
}

static int zoom2_hw_voice_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	int ret;

	zoom2_asoc_mux_config(ZOOM2_ASOC_MUX_MCBSP3_TRISTATE);

	/* set codec Voice IF to tristate*/
	ret = snd_soc_dai_set_tristate(codec_dai, 1);

	if (ret) {
			printk(KERN_ERR "can't set codec VIF tristate\n");
			return ret;
	}

	if (zoom2_twl4030_active == 1) {
		ret = twl4030_set_ext_clock(codec_dai->codec, 0);
		if (ret < 0) {
			printk(KERN_ERR "can't set 256 FS clock\n");
			return ret;
		}
	}

	return 0;
}
static struct snd_soc_ops zoom2_voice_ops = {
	.startup = zoom2_startup,
	.hw_params = zoom2_hw_voice_params,
	.hw_free = zoom2_hw_voice_free,
	.shutdown = zoom2_shutdown,
};

static int zoom2_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	if (gpio_request(ZOOM2_BT_MCBSP_GPIO, "bt_mux") == 0) {
		gpio_direction_output(ZOOM2_BT_MCBSP_GPIO, 1);
		gpio_free(ZOOM2_BT_MCBSP_GPIO);
	}

#if OMAP_MCBSP_MASTER_MODE
	int divisor;

	zoom2_asoc_mux_config(ZOOM2_ASOC_MUX_MCBSP3_SLAVE);

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_DSP_B |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_B |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
					SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	ret = twl4030_set_rate(codec_dai->codec, params);

	/* Use external (CLK256FS) clock for mcBSP3 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_EXT,
			0, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk(KERN_ERR "can't set mcBSP3 to external clock\n");
		return ret;
	}

	divisor = twl4030_get_clock_divisor(codec_dai->codec, params);

	ret = snd_soc_dai_set_clkdiv(cpu_dai, OMAP_MCBSP_CLKGDV, divisor);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec clock divisor\n");
		return ret;
	}
#else
	zoom2_asoc_mux_config(ZOOM2_ASOC_MUX_MCBSP3_SLAVE);

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_B |
				  SND_SOC_DAIFMT_NB_IF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
				     SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}
#endif

	if (zoom2_twl4030_active == 1) {
		ret = twl4030_set_ext_clock(codec_dai->codec, 1);
		if (ret < 0) {
			printk(KERN_ERR "can't set 256 FS clock\n");
			return ret;
		}
	}

	return 0;
}

static int zoom2_pcm_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	int ret;

	zoom2_asoc_mux_config(ZOOM2_ASOC_MUX_MCBSP3_TRISTATE);

	if (zoom2_twl4030_active == 1) {
		ret = twl4030_set_ext_clock(codec_dai->codec, 0);
		if (ret < 0) {
			printk(KERN_ERR "can't set 256 FS clock\n");
			return ret;
		}
	}

	return 0;
}

static struct snd_soc_ops zoom2_pcm_ops = {
	.startup = zoom2_startup,
	.hw_params = zoom2_pcm_hw_params,
	.hw_free = zoom2_pcm_hw_free,
	.shutdown = zoom2_shutdown,
};

static int zoom2_fm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

#if OMAP_MCBSP_MASTER_MODE
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	int divisor;

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
			SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	ret = twl4030_set_rate(codec_dai->codec, params);

	/* Use external (CLK256FS) clock for mcBSP4 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_EXT,
			0, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		printk(KERN_ERR "can't set mcBSP4 to external clock\n");
		return ret;
	}

	divisor = twl4030_get_clock_divisor(codec_dai->codec, params);

	ret = snd_soc_dai_set_clkdiv(cpu_dai, OMAP_MCBSP_CLKGDV, divisor);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec clock divisor\n");
		return ret;
	}
#else
	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
				     SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}
#endif

	if (zoom2_twl4030_active == 1) {
		ret = twl4030_set_ext_clock(codec_dai->codec, 1);
		if (ret < 0) {
			printk(KERN_ERR "can't set 256 FS clock\n");
			return ret;
		}
	}

	return 0;
}

static int zoom2_fm_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	int ret = 0;

	if (zoom2_twl4030_active == 1) {
		ret = twl4030_set_ext_clock(codec_dai->codec, 0);
		if (ret < 0)
			printk(KERN_ERR "can't set 256 FS clock\n");
	}

	return ret;
}

static struct snd_soc_ops zoom2_fm_ops = {
	.startup = zoom2_startup,
	.hw_params = zoom2_fm_hw_params,
	.hw_free = zoom2_fm_hw_free,
	.shutdown = zoom2_shutdown,
};

/* Zoom2 machine DAPM */
static const struct snd_soc_dapm_widget zoom2_twl4030_dapm_widgets[] = {
	SND_SOC_DAPM_MIC("Ext Mic", NULL),
	SND_SOC_DAPM_SPK("Ext Spk", NULL),
	SND_SOC_DAPM_MIC("Headset Mic", NULL),
	SND_SOC_DAPM_HP("Headset Stereophone", NULL),
	SND_SOC_DAPM_LINE("Aux In", NULL),
};

static const struct snd_soc_dapm_route audio_map[] = {
	/* External Mics: MAINMIC, SUBMIC with bias*/
	{"MAINMIC", NULL, "Mic Bias 1"},
	{"SUBMIC", NULL, "Mic Bias 2"},
	{"Mic Bias 1", NULL, "Ext Mic"},
	{"Mic Bias 2", NULL, "Ext Mic"},

	/* External Speakers: HFL, HFR */
	{"Ext Spk", NULL, "HFL"},
	{"Ext Spk", NULL, "HFR"},

	/* Headset Stereophone:  HSOL, HSOR */
	{"Headset Stereophone", NULL, "HSOL"},
	{"Headset Stereophone", NULL, "HSOR"},

	/* Headset Mic: HSMIC with bias */
	{"HSMIC", NULL, "Headset Mic Bias"},
	{"Headset Mic Bias", NULL, "Headset Mic"},

	/* Aux In: AUXL, AUXR */
	{"Aux In", NULL, "AUXL"},
	{"Aux In", NULL, "AUXR"},
};

static int zoom2_twl4030_init(struct snd_soc_codec *codec)
{
	int ret;

	/* Add Zoom2 specific widgets */
	ret = snd_soc_dapm_new_controls(codec, zoom2_twl4030_dapm_widgets,
				ARRAY_SIZE(zoom2_twl4030_dapm_widgets));
	if (ret)
		return ret;

	/* Set up Zoom2 specific audio path audio_map */
	snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));

	/* Zoom2 connected pins */
	snd_soc_dapm_enable_pin(codec, "Ext Mic");
	snd_soc_dapm_enable_pin(codec, "Ext Spk");
	snd_soc_dapm_enable_pin(codec, "Headset Mic");
	snd_soc_dapm_enable_pin(codec, "Headset Stereophone");
	snd_soc_dapm_enable_pin(codec, "Aux In");

    /*added by yuanyufang ,2010-12-25*/
    snd_soc_dapm_enable_pin(codec, "EARPIECE");
	snd_soc_dapm_enable_pin(codec, "PREDRIVEL");
	snd_soc_dapm_enable_pin(codec, "PREDRIVER");
	snd_soc_dapm_enable_pin(codec, "HSOL");
	snd_soc_dapm_enable_pin(codec, "HSOR");

	/* TWL4030 not connected pins */
	snd_soc_dapm_nc_pin(codec, "CARKITMIC");
	snd_soc_dapm_nc_pin(codec, "DIGIMIC0");
	snd_soc_dapm_nc_pin(codec, "DIGIMIC1");

	snd_soc_dapm_nc_pin(codec, "OUTL");
	snd_soc_dapm_nc_pin(codec, "OUTR");
	//snd_soc_dapm_nc_pin(codec, "EARPIECE");    //deleted by yuanyufang, 2010-12-25
	//snd_soc_dapm_nc_pin(codec, "PREDRIVEL");    //deleted by yuanyufang, 2010-12-25
	//snd_soc_dapm_nc_pin(codec, "PREDRIVER");    //deleted by yuanyufang, 2010-12-25
	snd_soc_dapm_nc_pin(codec, "CARKITL");
	snd_soc_dapm_nc_pin(codec, "CARKITR");

	ret = snd_soc_dapm_sync(codec);

	return ret;
}

static int zoom2_twl4030_voice_init(struct snd_soc_codec *codec)
{
	unsigned short reg;

	/* Enable voice interface */
	reg = codec->read(codec, TWL4030_REG_VOICE_IF);
	reg |= TWL4030_VIF_DIN_EN | TWL4030_VIF_DOUT_EN | TWL4030_VIF_EN;
	codec->write(codec, TWL4030_REG_VOICE_IF, reg);

	return 0;
}

#if !OMAP_MCBSP_MASTER_MODE
struct snd_soc_dai null_dai = {
	.name = "null",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 4,
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FORMAT_S24_LE,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 4,
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FORMAT_S24_LE,},
};
#endif

/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link zoom2_dai[] = {
{
	.name = "TWL4030_I2S",
	.stream_name = "TWL4030_I2S",
	.cpu_dai = &omap_mcbsp_dai[0],
	.codec_dai = &twl4030_dai[TWL4030_DAI_HIFI],
	.init = zoom2_twl4030_init,
	.ops = &zoom2_i2s_ops,
},
{
	.name = "TWL4030 VOICE",
	.stream_name = "TWL4030 Voice",
	.cpu_dai = &omap_mcbsp_dai[1],
	.codec_dai = &twl4030_dai[TWL4030_DAI_VOICE],
	.init = zoom2_twl4030_voice_init,
	.ops = &zoom2_voice_ops,
},
{
	.name = "TWL4030_PCM",
	.stream_name = "TWL4030_PCM",
	.cpu_dai = &omap_mcbsp_dai[1],
#if OMAP_MCBSP_MASTER_MODE
	.codec_dai = &twl4030_dai[TWL4030_DAI_CLOCK],
#else
	.codec_dai = &null_dai,
#endif
	.init = zoom2_twl4030_voice_init,
	.ops = &zoom2_pcm_ops,
},
{
	.name = "TWL4030_FM",
	.stream_name = "TWL4030_FM",
	.cpu_dai = &omap_mcbsp_dai[2],
#if OMAP_MCBSP_MASTER_MODE
	.codec_dai = &twl4030_dai[TWL4030_DAI_CLOCK],
#else
	.codec_dai = &null_dai,
#endif
	.ops = &zoom2_fm_ops,
},
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_zoom2 = {
	.name = "Zoom2",
	.platform = &omap_soc_platform,
	.dai_link = zoom2_dai,
	.num_links = ARRAY_SIZE(zoom2_dai),
};

/* EXTMUTE callback function */
void zoom2_set_hs_extmute(int mute)
{
	//gpio_set_value(ZOOM2_HEADSET_EXTMUTE_GPIO, mute);    //deleted by yuanyufang, 2010-12-25
}

/* twl4030 setup */
static struct twl4030_setup_data twl4030_setup = {
	.ramp_delay_value = 2,	/* 161 ms */
	.sysclk = 26000,
	.hs_extmute = 0,
	.set_hs_extmute = zoom2_set_hs_extmute,
};

/* Audio subsystem */
static struct snd_soc_device zoom2_snd_devdata = {
	.card = &snd_soc_zoom2,
	.codec_dev = &soc_codec_dev_twl4030,
	.codec_data = &twl4030_setup,
};

static struct platform_device *zoom2_snd_device;

static int __init zoom2_soc_init(void)
{
	int ret;
	if (!(machine_is_omap_zoom2() ||
			machine_is_omap_zoom3())) {
		pr_debug("Not Zoom2/3!\n");
		return -ENODEV;
	}
	printk(KERN_INFO "Zoom2 SoC init\n");

	zoom2_asoc_mux_config(ZOOM2_ASOC_MUX_MCBSP2_SLAVE);
#if !OMAP_MCBSP_MASTER_MODE
	snd_soc_register_dais(&null_dai, 1);
	//omap3_mux_config("OMAP_MCBSP4_SLAVE");    //deleted by yuanyufang, 2010-12-25
#else
	//omap3_mux_config("OMAP_MCBSP4_MASTER");
#endif

	zoom2_snd_device = platform_device_alloc("soc-audio", -1);
	if (!zoom2_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(zoom2_snd_device, &zoom2_snd_devdata);
	zoom2_snd_devdata.dev = &zoom2_snd_device->dev;
	*(unsigned int *)zoom2_dai[0].cpu_dai->private_data = 1; /* McBSP2 */
	*(unsigned int *)zoom2_dai[1].cpu_dai->private_data = 2; /* McBSP3 */
	*(unsigned int *)zoom2_dai[2].cpu_dai->private_data = 2; /* McBSP3 */
	*(unsigned int *)zoom2_dai[3].cpu_dai->private_data = 3; /* McBSP4 */

	ret = platform_device_add(zoom2_snd_device);
	if (ret)
		goto err1;

	//BUG_ON(gpio_request(ZOOM2_HEADSET_MUX_GPIO, "hs_mux") < 0);
	//gpio_direction_output(ZOOM2_HEADSET_MUX_GPIO, 0);

	//BUG_ON(gpio_request(ZOOM2_HEADSET_EXTMUTE_GPIO, "ext_mute") < 0);    //deleted by yuanyufang, 2010-12-25
	/* set EXTMUTE on for initial headset detection */
	//gpio_direction_output(ZOOM2_HEADSET_EXTMUTE_GPIO, 1);    //deleted by yuanyufang, 2010-12-25
	zoom2_twl4030_active = 0;

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(zoom2_snd_device);

	return ret;
}
module_init(zoom2_soc_init);

static void __exit zoom2_soc_exit(void)
{
	//gpio_free(ZOOM2_HEADSET_MUX_GPIO);
	//gpio_free(ZOOM2_HEADSET_EXTMUTE_GPIO);    //deleted by yuanyufang, 2010-12-25

	platform_device_unregister(zoom2_snd_device);
}
module_exit(zoom2_soc_exit);

MODULE_AUTHOR("Misael Lopez Cruz <x0052729@ti.com>");
MODULE_DESCRIPTION("ALSA SoC Zoom2");
MODULE_LICENSE("GPL");


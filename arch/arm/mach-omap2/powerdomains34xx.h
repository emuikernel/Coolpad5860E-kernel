/*
 * OMAP3 powerdomain definitions
 *
 * Copyright (C) 2007-2008 Texas Instruments, Inc.
 * Copyright (C) 2007-2010 Nokia Corporation
 *
 * Written by Paul Walmsley
 * Debugging and integration fixes by Jouni Högander
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef ARCH_ARM_MACH_OMAP2_POWERDOMAINS34XX
#define ARCH_ARM_MACH_OMAP2_POWERDOMAINS34XX

/*
 * N.B. If powerdomains are added or removed from this file, update
 * the array in mach-omap2/powerdomains.h.
 */

#include <plat/powerdomain.h>

#include "prcm-common.h"
#include "prm.h"
#include "prm-regbits-34xx.h"
#include "cm.h"
#include "cm-regbits-34xx.h"

/*
 * 34XX-specific powerdomains, dependencies
 */

#ifdef CONFIG_ARCH_OMAP3

/*
 * Powerdomains
 */

static struct powerdomain iva2_pwrdm = {
	.name		  = "iva2_pwrdm",
	.prcm_offs	  = OMAP3430_IVA2_MOD,
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	.banks		  = 4,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_OFF_RET,
		[1] = PWRSTS_OFF_RET,
		[2] = PWRSTS_OFF_RET,
		[3] = PWRSTS_OFF_RET,
	},
	.pwrsts_mem_on	  = {
		[0] = PWRDM_POWER_ON,
		[1] = PWRDM_POWER_ON,
		[2] = PWRSTS_OFF_ON,
		[3] = PWRDM_POWER_ON,
	},
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 1100,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = 350,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

static struct powerdomain mpu_3xxx_pwrdm = {
	.name		  = "mpu_pwrdm",
	.prcm_offs	  = MPU_MOD,
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	.flags		  = PWRDM_HAS_MPU_QUIRK,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_OFF_RET,
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_OFF_ON,
	},
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 95,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = 45,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

static struct powerdomain core_3xxx_pre_es3_1_pwrdm = {
	.name		  = "core_pwrdm",
	.prcm_offs	  = CORE_MOD,
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_IS_OMAP3430ES1 |
					   CHIP_IS_OMAP3430ES2 |
					   CHIP_IS_OMAP3430ES3_0),
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	.banks		  = 2,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_OFF_RET,	 /* MEM1RETSTATE */
		[1] = PWRSTS_OFF_RET,	 /* MEM2RETSTATE */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_OFF_RET_ON, /* MEM1ONSTATE */
		[1] = PWRSTS_OFF_RET_ON, /* MEM2ONSTATE */
	},
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 100,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = 60,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

static struct powerdomain core_3xxx_es3_1_pwrdm = {
	.name		  = "core_pwrdm",
	.prcm_offs	  = CORE_MOD,
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_GE_OMAP3430ES3_1),
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	.flags		  = PWRDM_HAS_HDWR_SAR, /* for USBTLL only */
	.banks		  = 2,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_OFF_RET,	 /* MEM1RETSTATE */
		[1] = PWRSTS_OFF_RET,	 /* MEM2RETSTATE */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_OFF_RET_ON, /* MEM1ONSTATE */
		[1] = PWRSTS_OFF_RET_ON, /* MEM2ONSTATE */
	},
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 100,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = 60,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

static struct powerdomain dss_pwrdm = {
	.name		  = "dss_pwrdm",
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
	.prcm_offs	  = OMAP3430_DSS_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRDM_POWER_RET,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRDM_POWER_RET, /* MEMRETSTATE */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRDM_POWER_ON,  /* MEMONSTATE */
	},
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 70,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = 20,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

/*
 * Although the 34XX TRM Rev K Table 4-371 notes that retention is a
 * possible SGX powerstate, the SGX device itself does not support
 * retention.
 */
static struct powerdomain sgx_pwrdm = {
	.name		  = "sgx_pwrdm",
	.prcm_offs	  = OMAP3430ES2_SGX_MOD,
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_GE_OMAP3430ES2),
	/* XXX This is accurate for 3430 SGX, but what about GFX? */
	.pwrsts		  = PWRSTS_OFF_ON,
	.pwrsts_logic_ret = PWRDM_POWER_RET,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRDM_POWER_RET, /* MEMRETSTATE */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRDM_POWER_ON,  /* MEMONSTATE */
	},
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 1000,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

static struct powerdomain cam_pwrdm = {
	.name		  = "cam_pwrdm",
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
	.prcm_offs	  = OMAP3430_CAM_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRDM_POWER_RET,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRDM_POWER_RET, /* MEMRETSTATE */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRDM_POWER_ON,  /* MEMONSTATE */
	},
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 850,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = 35,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

static struct powerdomain per_pwrdm = {
	.name		  = "per_pwrdm",
	.prcm_offs	  = OMAP3430_PER_MOD,
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRDM_POWER_RET, /* MEMRETSTATE */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRDM_POWER_ON,  /* MEMONSTATE */
	},
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 200,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = 110,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

static struct powerdomain emu_pwrdm = {
	.name		= "emu_pwrdm",
	.prcm_offs	= OMAP3430_EMU_MOD,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
};

static struct powerdomain neon_pwrdm = {
	.name		  = "neon_pwrdm",
	.prcm_offs	  = OMAP3430_NEON_MOD,
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRDM_POWER_RET,
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 200,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = 35,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

static struct powerdomain usbhost_pwrdm = {
	.name		  = "usbhost_pwrdm",
	.prcm_offs	  = OMAP3430ES2_USBHOST_MOD,
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_GE_OMAP3430ES2),
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRDM_POWER_RET,
	/*
	 * REVISIT: Enabling usb host save and restore mechanism seems to
	 * leave the usb host domain permanently in ACTIVE mode after
	 * changing the usb host power domain state from OFF to active once.
	 * Disabling for now.
	 */
	.flags	  = PWRDM_HAS_HDWR_SAR, /* for USBHOST ctrlr only */
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRDM_POWER_RET, /* MEMRETSTATE */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRDM_POWER_ON,  /* MEMONSTATE */
	},
	.wakeup_lat = {
		[PWRDM_FUNC_PWRST_OFF] = 800,
		[PWRDM_FUNC_PWRST_OSWR] = UNSUP_STATE,
		[PWRDM_FUNC_PWRST_CSWR] = 150,
		[PWRDM_FUNC_PWRST_ON] = 0,
	},
};

static struct powerdomain dpll1_pwrdm = {
	.name		= "dpll1_pwrdm",
	.prcm_offs	= MPU_MOD,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
};

static struct powerdomain dpll2_pwrdm = {
	.name		= "dpll2_pwrdm",
	.prcm_offs	= OMAP3430_IVA2_MOD,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
};

static struct powerdomain dpll3_pwrdm = {
	.name		= "dpll3_pwrdm",
	.prcm_offs	= PLL_MOD,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
};

static struct powerdomain dpll4_pwrdm = {
	.name		= "dpll4_pwrdm",
	.prcm_offs	= PLL_MOD,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP3430),
};

static struct powerdomain dpll5_pwrdm = {
	.name		= "dpll5_pwrdm",
	.prcm_offs	= PLL_MOD,
	.omap_chip	= OMAP_CHIP_INIT(CHIP_GE_OMAP3430ES2),
};


#endif    /* CONFIG_ARCH_OMAP3 */


#endif

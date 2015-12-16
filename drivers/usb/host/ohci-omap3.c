/*
 * ohci-omap3.c - driver for OHCI on OMAP3 and later processors
 *
 * Bus Glue for OMAP3 USBHOST 3 port OHCI controller
 * This controller is also used in later OMAPs and AM35x chips
 *
 * Copyright (C) 2007-2010 Texas Instruments, Inc.
 * Author: Vikram Pandita <vikram.pandita@ti.com>
 * Author: Anand Gadiyar <gadiyar@ti.com>
 *
 * Based on ehci-omap.c and some other ohci glue layers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * TODO (last updated Mar 10th, 2010):
 *	- add kernel-doc
 *	- Factor out code common to EHCI to a separate file
 *	- Make EHCI and OHCI coexist together
 *	  - needs newer silicon versions to actually work
 *	  - the last one to be loaded currently steps on the other's toes
 *	- Add hooks for configuring transceivers, etc. at init/exit
 *	- Add aggressive clock-management code
 */

#include <linux/platform_device.h>
#include <linux/clk.h>

#include <plat/usb.h>

#ifdef CONFIG_AP2MODEM_VIATELECOM
extern void sleep_modem(void);
extern void wake_modem(void);
#endif
/*
 * OMAP USBHOST Register addresses: VIRTUAL ADDRESSES
 *	Use ohci_omap_readl()/ohci_omap_writel() functions
 */

/* TLL Register Set */
#define	OMAP_USBTLL_REVISION				(0x00)
#define	OMAP_USBTLL_SYSCONFIG				(0x10)
//#define	OMAP_USBTLL_SYSCONFIG_CACTIVITY			(1 << 8)
#define	OMAP_USBTLL_SYSCONFIG_CACTIVITY			(0 << 8)
//#define	OMAP_USBTLL_SYSCONFIG_SIDLEMODE			(1 << 3)
#define	OMAP_USBTLL_SYSCONFIG_SIDLEMODE			(2 << 3)
#define	OMAP_USBTLL_SYSCONFIG_ENAWAKEUP			(1 << 2)
#define	OMAP_USBTLL_SYSCONFIG_SOFTRESET			(1 << 1)
#define	OMAP_USBTLL_SYSCONFIG_AUTOIDLE			(1 << 0)

#define	OMAP_USBTLL_SYSSTATUS				(0x14)
#define	OMAP_USBTLL_SYSSTATUS_RESETDONE			(1 << 0)

#define	OMAP_USBTLL_IRQSTATUS				(0x18)
#define	OMAP_USBTLL_IRQENABLE				(0x1C)

#define	OMAP_TLL_SHARED_CONF				(0x30)
#define	OMAP_TLL_SHARED_CONF_USB_90D_DDR_EN		(1 << 6)
#define	OMAP_TLL_SHARED_CONF_USB_180D_SDR_EN		(1 << 5)
#define	OMAP_TLL_SHARED_CONF_USB_DIVRATION		(1 << 2)
#define	OMAP_TLL_SHARED_CONF_FCLK_REQ			(1 << 1)
#define	OMAP_TLL_SHARED_CONF_FCLK_IS_ON			(1 << 0)

#define	OMAP_TLL_CHANNEL_CONF(num)			(0x040 + 0x004 * num)
#define OMAP_TLL_CHANNEL_CONF_FSLSMODE_SHIFT		24
#define	OMAP_TLL_CHANNEL_CONF_ULPINOBITSTUFF		(1 << 11)
#define	OMAP_TLL_CHANNEL_CONF_ULPI_ULPIAUTOIDLE		(1 << 10)
#define	OMAP_TLL_CHANNEL_CONF_UTMIAUTOIDLE		(1 << 9)
#define	OMAP_TLL_CHANNEL_CONF_ULPIDDRMODE		(1 << 8)
#define OMAP_TLL_CHANNEL_CONF_CHANMODE_FSLS		(1 << 1)
#define	OMAP_TLL_CHANNEL_CONF_CHANEN			(1 << 0)

#define OMAP_TLL_CHANNEL_COUNT				3

/* UHH Register Set */
#define	OMAP_UHH_REVISION				(0x00)
#define	OMAP_UHH_SYSCONFIG				(0x10)
#define	OMAP_UHH_SYSCONFIG_MIDLEMODE			(2 << 12)//modify by tang 20110429 (1 << 12)
#define	OMAP_UHH_SYSCONFIG_CACTIVITY			(0 << 8) //modify by tang 20110510 (1 << 8)
#define	OMAP_UHH_SYSCONFIG_SIDLEMODE			(2 << 3)//modify by tang 20110429 (1 << 3)
#define	OMAP_UHH_SYSCONFIG_ENAWAKEUP			(1 << 2)
#define	OMAP_UHH_SYSCONFIG_SOFTRESET			(1 << 1)
#define	OMAP_UHH_SYSCONFIG_AUTOIDLE			(1 << 0)

#define	OMAP_UHH_SYSSTATUS				(0x14)
#define	OMAP_UHH_HOSTCONFIG				(0x40)
#define	OMAP_UHH_HOSTCONFIG_ULPI_BYPASS			(1 << 0)
#define	OMAP_UHH_HOSTCONFIG_ULPI_P1_BYPASS		(1 << 0)
#define	OMAP_UHH_HOSTCONFIG_ULPI_P2_BYPASS		(1 << 11)
#define	OMAP_UHH_HOSTCONFIG_ULPI_P3_BYPASS		(1 << 12)
#define OMAP_UHH_HOSTCONFIG_INCR4_BURST_EN		(1 << 2)
#define OMAP_UHH_HOSTCONFIG_INCR8_BURST_EN		(1 << 3)
#define OMAP_UHH_HOSTCONFIG_INCR16_BURST_EN		(1 << 4)
#define OMAP_UHH_HOSTCONFIG_INCRX_ALIGN_EN		(1 << 5)
#define OMAP_UHH_HOSTCONFIG_P1_CONNECT_STATUS		(1 << 8)
#define OMAP_UHH_HOSTCONFIG_P2_CONNECT_STATUS		(1 << 9)
#define OMAP_UHH_HOSTCONFIG_P3_CONNECT_STATUS		(1 << 10)

#define	OMAP_UHH_DEBUG_CSR				(0x44)

/* OMAP4 specific */
#define OMAP_UHH_SYSCONFIG_IDLEMODE_RESET		(~(0xC))
#define OMAP_UHH_SYSCONFIG_FIDLEMODE_SET		(0 << 2)
#define OMAP_UHH_SYSCONFIG_NIDLEMODE_SET		(1 << 2)
#define OMAP_UHH_SYSCONFIG_SIDLEMODE_SET		(2 << 2)
#define OMAP_UHH_SYSCONFIG_SWIDLMODE_SET		(3 << 2)

#define OMAP_UHH_SYSCONFIG_STDYMODE_RESET		(~(3 << 4))
#define OMAP_UHH_SYSCONFIG_FSTDYMODE_SET		(0 << 4)
#define OMAP_UHH_SYSCONFIG_NSTDYMODE_SET		(1 << 4)
#define OMAP_UHH_SYSCONFIG_SSTDYMODE_SET		(2 << 4)
#define OMAP_UHH_SYSCONFIG_SWSTDMODE_SET		(3 << 4)

#define OMAP_UHH_HOST_PORTS_RESET			(~(0xF << 16))
#define OMAP_UHH_HOST_P1_SET_ULPIPHY			(0 << 16)
#define OMAP_UHH_HOST_P1_SET_ULPITLL			(1 << 16)
#define OMAP_UHH_HOST_P1_SET_HSIC			(3 << 16)

#define OMAP_UHH_HOST_P2_SET_ULPIPHY			(0 << 18)
#define OMAP_UHH_HOST_P2_SET_ULPITLL			(1 << 18)
#define OMAP_UHH_HOST_P2_SET_HSIC			(3 << 18)
#define OMAP4_UHH_SYSCONFIG_SOFTRESET			(1 << 0)

/*add by pengriliang*/
#define	OMAP_UHH_SYSCONFIG_MIDLEMODE_SHIFT	12
#define	OMAP_UHH_SYSCONFIG_CACTIVITY_SHIFT	8
#define	OMAP_UHH_SYSCONFIG_SIDLEMODE_SHIFT	3
#define	OMAP_UHH_SYSCONFIG_ENAWAKEUP_SHIFT	2
#define	OMAP_UHH_SYSCONFIG_SOFTRESET_SHIFT	1
#define	OMAP_UHH_SYSCONFIG_AUTOIDLE_SHIFT	0

#define OMAP4_TLL_CHANNEL_COUNT				2
/*-------------------------------------------------------------------------*/

static inline void ohci_omap_writel(void __iomem *base, u32 reg, u32 val)
{
	__raw_writel(val, base + reg);
}

static inline u32 ohci_omap_readl(void __iomem *base, u32 reg)
{
	return __raw_readl(base + reg);
}

static inline void ohci_omap_writeb(void __iomem *base, u8 reg, u8 val)
{
	__raw_writeb(val, base + reg);
}

static inline u8 ohci_omap_readb(void __iomem *base, u8 reg)
{
	return __raw_readb(base + reg);
}

/*-------------------------------------------------------------------------*/

struct ohci_hcd_omap {
	struct ohci_hcd		*ohci;
	struct device		*dev;
	struct clk		*usbhost_ick;
	struct clk		*usbhost_fck;
	struct clk		*usbhost_fs_fck;
	struct clk		*usbtll_fck;
	struct clk		*usbtll_ick;
	struct clk		*xclk60mhsp1_ck;
	struct clk		*xclk60mhsp2_ck;
	struct clk		*utmi_p1_fck;
	struct clk		*utmi_p2_fck;

	/* port_mode: TLL/PHY, 2/3/4/6-PIN, DP-DM/DAT-SE0 */
	enum ohci_omap3_port_mode	port_mode[OMAP3_HS_USB_PORTS];
	void __iomem		*uhh_base;
	void __iomem		*tll_base;
	void __iomem		*ohci_base;

	unsigned suspended:1;
};

//#define OHCI_REGISTER_DUMP
#ifdef OHCI_REGISTER_DUMP
#define OHCI_REG_BASE (0x48064400)
char *reg_name[] =
{
    "HCREVISION",
    "HCCONTROL",
    "HCCOMMANDSTATUS",
    "HCINTERRUPTSTATUS",
    "HCINTERRUPTENABLE",
    "HCINTERRUPTDISABLE",
    "HCHCCA",
};
void sguan_dump_ohci(void)
{
    int i = 0;
    pr_debug("sguan--%s--enter\n", __func__);
    for(i = 0; i < ARRAY_SIZE(reg_name); i++)
    {
      pr_debug("sguan-%s[0x%x] value is 0x%x\n", reg_name[i], \
          OHCI_REG_BASE + (4*i), omap_readl(OHCI_REG_BASE + 4 * i));
    }
    printk("USB HOST domain:\n");
	printk("CM_IDLEST_USBHOST=0x%8x, CM_CLKSTST_USBHOST=0x%8x\n", omap_readl(0x48005420), omap_readl(0x4800544c));
	printk("CM_FCLKEN_USBHOST=0x%8x, CM_ICLKEN_USBHOST=0x%8x, CM_CLKSTCTRL_USBHOST=0x%8x\n\n", omap_readl(0x48005400), omap_readl(0x48005410), omap_readl(0x48005448));
    printk("USBHOST: 0x483074E0=%04x,0x483074E4=%04x,0x483074E8=%04x,0x4800544c=%04x\n", 
        omap_readl(0x483074E0),omap_readl(0x483074E4),
        omap_readl(0x483074E8),omap_readl(0x4800544c)
        );    
	
    pr_debug("sguan--%s--exit\n", __func__);
}
#else
void sguan_dump_ohci(void)
{
  return;
}
#endif

//#define OHCI_OMAP3_DEBUG

#ifdef OHCI_OMAP3_DEBUG
#undef dbg
#define dbg(format, arg...) printk(KERN_DEBUG format "\n", ## arg)
#else
#undef dbg
#define dbg(format, arg...) 
#endif
/*-------------------------------------------------------------------------*/
//added by coolpad tanzhongjun 2010-09-03 for CP2058
struct usb_hcd *g_usb_hcd = 0;
void ohci_resume_rh_autostop(void)
{
#if 1
    struct usb_hcd *hcd = g_usb_hcd;
	struct ohci_hcd *ohci = hcd_to_ohci (g_usb_hcd);
	if (ohci->autostop) {
		spin_lock (&ohci->lock);
		ohci_rh_resume (ohci);
		spin_unlock (&ohci->lock);
	}
#else
	usb_hcd_poll_rh_status(g_usb_hcd);
#endif
}
EXPORT_SYMBOL(ohci_resume_rh_autostop);
//ended

static void omap3_ohci_clock_power(struct ohci_hcd_omap *omap, int on)
{
	if (on) {
		if (omap->usbtll_ick != NULL)
			clk_enable(omap->usbtll_ick);
		if (omap->usbtll_fck != NULL)
			clk_enable(omap->usbtll_fck);
		if (omap->usbhost_ick != NULL)
			clk_enable(omap->usbhost_ick);
		if (omap->usbhost_fs_fck != NULL)
			clk_enable(omap->usbhost_fs_fck);
		if (omap->usbhost_fck != NULL)
			clk_enable(omap->usbhost_fck);
		if (omap->utmi_p1_fck != NULL)
			clk_enable(omap->utmi_p1_fck);
		if (omap->utmi_p2_fck != NULL)
			clk_enable(omap->utmi_p2_fck);

	} else {
		if (omap->usbtll_ick != NULL)
			clk_disable(omap->usbtll_ick);
		if (omap->usbtll_fck != NULL)
			clk_disable(omap->usbtll_fck);
		if (omap->usbhost_ick != NULL)
			clk_disable(omap->usbhost_ick);
		if (omap->usbhost_fs_fck != NULL)
			clk_disable(omap->usbhost_fs_fck);
		if (omap->usbhost_fck != NULL)
			clk_disable(omap->usbhost_fck);
		if (omap->utmi_p1_fck != NULL)
			clk_disable(omap->utmi_p1_fck);
		if (omap->utmi_p2_fck != NULL)
			clk_disable(omap->utmi_p2_fck);
	}
}

static int ohci_omap_init(struct usb_hcd *hcd)
{
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int ret;

	dev_dbg(hcd->self.controller, "starting OHCI controller\n");

	ret = ohci_init(ohci);

	return ret;
}

/*-------------------------------------------------------------------------*/

static int ohci_omap_start(struct usb_hcd *hcd)
{
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int ret;
    u32 t_sguan = 0;

	/*
	 * RemoteWakeupConnected has to be set explicitly before
	 * calling ohci_run. The reset value of RWC is 0.
	 */
	ohci->hc_control = OHCI_CTRL_RWC;
	writel(OHCI_CTRL_RWC, &ohci->regs->control);
    t_sguan = readl(&ohci->regs->control);

	ret = ohci_run(ohci);

	if (ret < 0) {
		dev_err(hcd->self.controller, "can't start\n");
		ohci_stop(hcd);
		return ret;
	}
	return 0;
}


/*-------------------------------------------------------------------------*/

static unsigned omap3_ohci_fslsmode(enum ohci_omap3_port_mode mode)
{
	switch (mode) {
	case OMAP_OHCI_PORT_MODE_UNUSED:
	case OMAP_OHCI_PORT_MODE_PHY_6PIN_DATSE0:
		return 0x0;

	case OMAP_OHCI_PORT_MODE_PHY_6PIN_DPDM:
		return 0x1;

	case OMAP_OHCI_PORT_MODE_PHY_3PIN_DATSE0:
		return 0x2;

	case OMAP_OHCI_PORT_MODE_PHY_4PIN_DPDM:
		return 0x3;

	case OMAP_OHCI_PORT_MODE_TLL_6PIN_DATSE0:
		return 0x4;

	case OMAP_OHCI_PORT_MODE_TLL_6PIN_DPDM:
		return 0x5;

	case OMAP_OHCI_PORT_MODE_TLL_3PIN_DATSE0:
		return 0x6;

	case OMAP_OHCI_PORT_MODE_TLL_4PIN_DPDM:
		return 0x7;

	case OMAP_OHCI_PORT_MODE_TLL_2PIN_DATSE0:
		return 0xA;

	case OMAP_OHCI_PORT_MODE_TLL_2PIN_DPDM:
		return 0xB;
	default:
		pr_warning("Invalid port mode, using default\n");
		return 0x0;
	}
}

static void omap_ohci_tll_config(struct ohci_hcd_omap *omap,
					u8 tll_channel_count)
{
	u32 reg;
	int i;

	/* Program TLL SHARED CONF */
	reg = ohci_omap_readl(omap->tll_base, OMAP_TLL_SHARED_CONF);
	reg &= ~OMAP_TLL_SHARED_CONF_USB_90D_DDR_EN;
	reg &= ~OMAP_TLL_SHARED_CONF_USB_180D_SDR_EN;
	reg |= OMAP_TLL_SHARED_CONF_USB_DIVRATION;
	reg |= OMAP_TLL_SHARED_CONF_FCLK_IS_ON;
	ohci_omap_writel(omap->tll_base, OMAP_TLL_SHARED_CONF, reg);

	/* Program each TLL channel */
	/*
	 * REVISIT: Only the 3-pin and 4-pin PHY modes have
	 * actually been tested.
	 */
	for (i = 0; i < tll_channel_count; i++) {

		/* Enable only those channels that are actually used */
		if (omap->port_mode[i] == OMAP_OHCI_PORT_MODE_UNUSED)
			continue;

		reg = ohci_omap_readl(omap->tll_base, OMAP_TLL_CHANNEL_CONF(i));
		reg |= omap3_ohci_fslsmode(omap->port_mode[i])
				<< OMAP_TLL_CHANNEL_CONF_FSLSMODE_SHIFT;
		reg |= OMAP_TLL_CHANNEL_CONF_CHANMODE_FSLS;
		reg |= OMAP_TLL_CHANNEL_CONF_CHANEN;
		ohci_omap_writel(omap->tll_base, OMAP_TLL_CHANNEL_CONF(i), reg);
	}
}

/* omap_start_ohc
 *	- Start the TI USBHOST controller
 */
static int omap_start_ohc(struct ohci_hcd_omap *omap, struct usb_hcd *hcd)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(1000);
	u32 reg = 0;
	int ret = 0;

	dev_dbg(omap->dev, "starting TI OHCI USB Controller\n");

	if (cpu_is_omap44xx()) {
#if 0
		/* Enable clocks for OMAP4 USBHOST */
		omap->usbhost_fck = clk_get(omap->dev, "usb_host_fck");
		if (IS_ERR(omap->usbhost_fck)) {
			ret = PTR_ERR(omap->usbhost_fck);
			return ret;
		}

		omap->usbhost_fs_fck = clk_get(omap->dev, "usb_host_fs_fck");
		if (IS_ERR(omap->usbhost_fs_fck)) {
			ret = PTR_ERR(omap->usbhost_fs_fck);
			goto err_44host_fs_fck;
		}

		omap->xclk60mhsp1_ck = clk_get(omap->dev, "xclk60mhsp1_ck");
		if (IS_ERR(omap->xclk60mhsp1_ck)) {
			ret = PTR_ERR(omap->xclk60mhsp1_ck);
			goto err_xclk60mhsp1_ck;
		}

		omap->utmi_p1_fck = clk_get(omap->dev, "utmi_p1_gfclk_ck");
		if (IS_ERR(omap->utmi_p1_fck)) {
			ret = PTR_ERR(omap->utmi_p1_fck);
			goto err_xclk60mhsp1_ck;
		}

		omap->utmi_p2_fck = clk_get(omap->dev, "utmi_p2_gfclk_ck");
		if (IS_ERR(omap->utmi_p2_fck)) {
			ret = PTR_ERR(omap->utmi_p2_fck);
			goto err_xclk60mhsp2_ck;
		}

		omap->usbtll_ick = clk_get(omap->dev, "usb_tll_ick");
		if (IS_ERR(omap->usbtll_ick)) {
			ret = PTR_ERR(omap->usbtll_ick);
			goto err_44tll_ick;
		}

		/* Now enable all the clocks in the correct order */
		omap3_ohci_clock_power(omap, 1);

		/* perform TLL soft reset, and wait
		 * until reset is complete */
		ohci_omap_writel(omap->tll_base, OMAP_USBTLL_SYSCONFIG,
				OMAP_USBTLL_SYSCONFIG_SOFTRESET);

		/* Wait for TLL reset to complete */
		while (!(ohci_omap_readl(omap->tll_base,
			OMAP_USBTLL_SYSSTATUS) &
			OMAP_USBTLL_SYSSTATUS_RESETDONE)) {
			cpu_relax();

			if (time_after(jiffies, timeout)) {
				dev_dbg(omap->dev, "operation timed out\n");
				ret = -EINVAL;
				goto err_44sys_status;
			}
		}

		omap->xclk60mhsp2_ck = NULL;
		omap->xclk60mhsp1_ck = NULL;
		omap->usbhost_ick = NULL;
		omap->usbtll_ick = NULL;
		dev_dbg(omap->dev, "TLL RESET DONE\n");

		/* Put UHH in NoIdle/NoStandby mode */
		reg = ohci_omap_readl(omap->uhh_base, OMAP_UHH_SYSCONFIG);
		reg &= OMAP_UHH_SYSCONFIG_IDLEMODE_RESET;
		reg |= OMAP_UHH_SYSCONFIG_NIDLEMODE_SET;

		reg &= OMAP_UHH_SYSCONFIG_STDYMODE_RESET;
		reg |= OMAP_UHH_SYSCONFIG_NSTDYMODE_SET;

		ohci_omap_writel(omap->uhh_base, OMAP_UHH_SYSCONFIG, reg);

		reg = ohci_omap_readl(omap->uhh_base, OMAP_UHH_HOSTCONFIG);

		/* setup ULPI bypass and burst configurations */
		reg |= (OMAP_UHH_HOSTCONFIG_INCR4_BURST_EN |
			OMAP_UHH_HOSTCONFIG_INCR8_BURST_EN |
			OMAP_UHH_HOSTCONFIG_INCR16_BURST_EN);
		reg &= ~OMAP_UHH_HOSTCONFIG_INCRX_ALIGN_EN;

		/* set p1 & p2 modes */
		/* OHCI has to go through USBTLL */
		reg &= OMAP_UHH_HOST_PORTS_RESET;
		if (omap->port_mode[0] != OMAP_OHCI_PORT_MODE_UNUSED)
			reg |= OMAP_UHH_HOST_P1_SET_ULPITLL;
		if (omap->port_mode[1] != OMAP_OHCI_PORT_MODE_UNUSED)
			reg |= OMAP_UHH_HOST_P2_SET_ULPITLL;

		ohci_omap_writel(omap->uhh_base, OMAP_UHH_HOSTCONFIG, reg);
		dev_dbg(omap->dev, "UHH setup done, uhh_hostconfig=%x\n", reg);

		/* (1<<3) = no idle mode only for initial debugging */
		ohci_omap_writel(omap->tll_base, OMAP_USBTLL_SYSCONFIG,
				OMAP_USBTLL_SYSCONFIG_ENAWAKEUP |
				OMAP_USBTLL_SYSCONFIG_SIDLEMODE |
				OMAP_USBTLL_SYSCONFIG_CACTIVITY);

		omap_ohci_tll_config(omap, OMAP4_TLL_CHANNEL_COUNT);

		return 0;

err_44sys_status:
		omap3_ohci_clock_power(omap, 0);
		clk_put(omap->usbtll_ick);

err_44tll_ick:
		clk_put(omap->utmi_p2_fck);

err_xclk60mhsp2_ck:
		clk_put(omap->utmi_p1_fck);

err_xclk60mhsp1_ck:
		clk_put(omap->usbhost_fs_fck);

err_44host_fs_fck:
		clk_put(omap->usbhost_fck);

		return ret;

#endif
	} 
	else {

		/* Get all the clock handles we need */
		omap->usbhost_ick = clk_get(omap->dev, "usbhost_ick");
		if (IS_ERR(omap->usbhost_ick)) {
			ret =  PTR_ERR(omap->usbhost_ick);
			goto err_host_ick;
		}

		omap->usbhost_fck = clk_get(omap->dev, "usbhost_120m_fck");
		if (IS_ERR(omap->usbhost_fck)) {
			ret = PTR_ERR(omap->usbhost_fck);
			goto err_host_fck;
		}

		omap->usbhost_fs_fck = clk_get(omap->dev, "usbhost_48m_fck");
		if (IS_ERR(omap->usbhost_fs_fck)) {
			ret = PTR_ERR(omap->usbhost_fs_fck);
			goto err_host_fs_fck;
		}

		/* Configure TLL for 60Mhz clk for ULPI */
		omap->usbtll_fck = clk_get(omap->dev, "usbtll_fck");
		if (IS_ERR(omap->usbtll_fck)) {
			ret = PTR_ERR(omap->usbtll_fck);
			goto err_tll_fck;
		}

		omap->usbtll_ick = clk_get(omap->dev, "usbtll_ick");
		if (IS_ERR(omap->usbtll_ick)) {
			ret = PTR_ERR(omap->usbtll_ick);
			goto err_tll_ick;
		}

		/* Now enable all the clocks in the correct order */
		omap3_ohci_clock_power(omap, 1);

		/* perform TLL soft reset, and wait until reset is complete */
		ohci_omap_writel(omap->tll_base, OMAP_USBTLL_SYSCONFIG,
					OMAP_USBTLL_SYSCONFIG_SOFTRESET);

		/* Wait for TLL reset to complete */
		while (!(ohci_omap_readl(omap->tll_base, OMAP_USBTLL_SYSSTATUS)
			& OMAP_USBTLL_SYSSTATUS_RESETDONE)) {
			cpu_relax();

			if (time_after(jiffies, timeout)) {
				dev_dbg(omap->dev, "operation timed out\n");
				ret = -EINVAL;
				goto err_sys_status;
			}
		}

		dev_dbg(omap->dev, "TLL reset done\n");

		/* (1<<3) = no idle mode only for initial debugging */
		ohci_omap_writel(omap->tll_base, OMAP_USBTLL_SYSCONFIG,
				OMAP_USBTLL_SYSCONFIG_ENAWAKEUP |
				OMAP_USBTLL_SYSCONFIG_SIDLEMODE |
				OMAP_USBTLL_SYSCONFIG_CACTIVITY |
				OMAP_USBTLL_SYSCONFIG_AUTOIDLE);


		/* Put UHH in NoIdle/NoStandby mode */
		reg = ohci_omap_readl(omap->uhh_base, OMAP_UHH_SYSCONFIG);
		reg |= (OMAP_UHH_SYSCONFIG_ENAWAKEUP
			| OMAP_UHH_SYSCONFIG_SIDLEMODE
			| OMAP_UHH_SYSCONFIG_CACTIVITY
			| OMAP_UHH_SYSCONFIG_MIDLEMODE
			| OMAP_UHH_SYSCONFIG_AUTOIDLE);
		//reg &= ~OMAP_UHH_SYSCONFIG_AUTOIDLE;
		//reg &= ~OMAP_UHH_SYSCONFIG_SOFTRESET;

		ohci_omap_writel(omap->uhh_base, OMAP_UHH_SYSCONFIG, reg);
		printk("%s:OMAP_UHH_SYSCONFIG=0x%x\n",__func__,ohci_omap_readl(omap->uhh_base, OMAP_UHH_SYSCONFIG));

		reg = ohci_omap_readl(omap->uhh_base, OMAP_UHH_HOSTCONFIG);

		/* setup ULPI bypass and burst configurations */
		reg |= (OMAP_UHH_HOSTCONFIG_INCR4_BURST_EN
			| OMAP_UHH_HOSTCONFIG_INCR8_BURST_EN
			| OMAP_UHH_HOSTCONFIG_INCR16_BURST_EN);
		reg &= ~OMAP_UHH_HOSTCONFIG_INCRX_ALIGN_EN;

		/*
		 * REVISIT: Pi_CONNECT_STATUS controls MStandby
		 * assertion and Swakeup generation - let us not
		 * worry about this for now. OMAP HWMOD framework
		 * might take care of this later. If not, we can
		 * update these registers when adding aggressive
		 * clock management code.
		 *
		 * For now, turn off all the Pi_CONNECT_STATUS bits
		 *
		if (omap->port_mode[0] == OMAP_OHCI_PORT_MODE_UNUSED)
			reg &= ~OMAP_UHH_HOSTCONFIG_P1_CONNECT_STATUS;
		if (omap->port_mode[1] == OMAP_OHCI_PORT_MODE_UNUSED)
			reg &= ~OMAP_UHH_HOSTCONFIG_P2_CONNECT_STATUS;
		if (omap->port_mode[2] == OMAP_OHCI_PORT_MODE_UNUSED)
			reg &= ~OMAP_UHH_HOSTCONFIG_P3_CONNECT_STATUS;
		*/
		reg &= ~OMAP_UHH_HOSTCONFIG_P1_CONNECT_STATUS;
		reg &= ~OMAP_UHH_HOSTCONFIG_P2_CONNECT_STATUS;
		reg &= ~OMAP_UHH_HOSTCONFIG_P3_CONNECT_STATUS;

		if (omap_rev() <= OMAP3430_REV_ES2_1) {
			/*
			 * All OHCI modes need to go through the TLL,
			 * unlike in the EHCI case. So use UTMI mode
			 * for all ports for OHCI, on ES2.x silicon
			 */
			dev_dbg(omap->dev, "OMAP3 ES version <= ES2.1\n");
			reg |= OMAP_UHH_HOSTCONFIG_ULPI_BYPASS;
		} else {
			dev_dbg(omap->dev, "OMAP3 ES version > ES2.1\n");
			if (omap->port_mode[0] == OMAP_OHCI_PORT_MODE_UNUSED)
				reg &= ~OMAP_UHH_HOSTCONFIG_ULPI_P1_BYPASS;
			else
				reg |= OMAP_UHH_HOSTCONFIG_ULPI_P1_BYPASS;

			if (omap->port_mode[1] == OMAP_OHCI_PORT_MODE_UNUSED)
				reg &= ~OMAP_UHH_HOSTCONFIG_ULPI_P2_BYPASS;
			else
				reg |= OMAP_UHH_HOSTCONFIG_ULPI_P2_BYPASS;

			if (omap->port_mode[2] == OMAP_OHCI_PORT_MODE_UNUSED)
				reg &= ~OMAP_UHH_HOSTCONFIG_ULPI_P3_BYPASS;
			else
				reg |= OMAP_UHH_HOSTCONFIG_ULPI_P3_BYPASS;

		}
		ohci_omap_writel(omap->uhh_base, OMAP_UHH_HOSTCONFIG, reg);
		dev_dbg(omap->dev, "UHH setup done, uhh_hostconfig=%x\n", reg);

		omap_ohci_tll_config(omap, OMAP_TLL_CHANNEL_COUNT);

		return 0;

err_sys_status:
		omap3_ohci_clock_power(omap, 0);
		clk_put(omap->usbtll_ick);

err_tll_ick:
		clk_put(omap->usbtll_fck);

err_tll_fck:
		clk_put(omap->usbhost_fs_fck);

err_host_fs_fck:
		clk_put(omap->usbhost_fck);

err_host_fck:
		clk_put(omap->usbhost_ick);

err_host_ick:
		return ret;
	}
}

static void omap_stop_ohc(struct ohci_hcd_omap *omap, struct usb_hcd *hcd)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(100);

	dev_dbg(omap->dev, "stopping TI OHCI USB Controller\n");

	/* Reset OMAP modules for insmod/rmmod to work */
	if (cpu_is_omap44xx()) {
		ohci_omap_writel(omap->uhh_base, OMAP_UHH_SYSCONFIG,
					OMAP4_UHH_SYSCONFIG_SOFTRESET);
	} else {
		ohci_omap_writel(omap->uhh_base, OMAP_UHH_SYSCONFIG,
			OMAP_UHH_SYSCONFIG_SOFTRESET);
		printk("%s:OMAP_UHH_SYSCONFIG=0x%x\n",__func__,ohci_omap_readl(omap->uhh_base, OMAP_UHH_SYSCONFIG));
	}

	while (!(ohci_omap_readl(omap->uhh_base, OMAP_UHH_SYSSTATUS)
				& (1 << 0))) {
		cpu_relax();

		if (time_after(jiffies, timeout))
			dev_dbg(omap->dev, "operation timed out\n");
	}

	while (!(ohci_omap_readl(omap->uhh_base, OMAP_UHH_SYSSTATUS)
				& (1 << 1))) {
		cpu_relax();

		if (time_after(jiffies, timeout))
			dev_dbg(omap->dev, "operation timed out\n");
	}

	while (!(ohci_omap_readl(omap->uhh_base, OMAP_UHH_SYSSTATUS)
				& (1 << 2))) {
		cpu_relax();

		if (time_after(jiffies, timeout))
			dev_dbg(omap->dev, "operation timed out\n");
	}

	ohci_omap_writel(omap->tll_base, OMAP_USBTLL_SYSCONFIG, (1 << 1));

	while (!(ohci_omap_readl(omap->tll_base, OMAP_USBTLL_SYSSTATUS)
				& (1 << 0))) {
		cpu_relax();

		if (time_after(jiffies, timeout))
			dev_dbg(omap->dev, "operation timed out\n");
	}

	omap3_ohci_clock_power(omap, 0);

	if (omap->usbtll_fck != NULL) {
		clk_put(omap->usbtll_fck);
		omap->usbtll_fck = NULL;
	}

	if (omap->usbhost_ick != NULL) {
		clk_put(omap->usbhost_ick);
		omap->usbhost_ick = NULL;
	}

	if (omap->usbhost_fs_fck != NULL) {
		clk_put(omap->usbhost_fs_fck);
		omap->usbhost_fs_fck = NULL;
	}

	if (omap->usbhost_fck != NULL) {
		clk_put(omap->usbhost_fck);
		omap->usbhost_fck = NULL;
	}

	if (omap->usbtll_ick != NULL) {
		clk_put(omap->usbtll_ick);
		omap->usbtll_ick = NULL;
	}

	dev_dbg(omap->dev, "Clock to USB host has been disabled\n");
}


#ifdef CONFIG_PM
static int omap_ohci_bus_suspend(struct usb_hcd *hcd)
{
	int rc;
	u32 uhh_sysconfig =0 ;
	struct ohci_hcd_omap *omap = platform_get_drvdata(
                container_of(hcd->self.controller, struct platform_device, dev));

    printk("\n===>%s() states=%s \n",__func__,omap->suspended?"SLEEPED":"WAKED");

    if(!omap->suspended)
    {
	    rc = ohci_bus_suspend(hcd);
	
        mdelay(8); /* MSTANDBY assertion delayed by ~8ms */

        /* Need to set ForceStandby,ForceIdle here
	     * else the domain may not be able to transition
	     * back during clk_enable if there was a pending event.
	     */

	    uhh_sysconfig = ohci_omap_readl(omap->uhh_base,OMAP_UHH_SYSCONFIG);
        printk("sguan-%s-UHH_SYSCONFIG value is 0x%x\n", __func__, uhh_sysconfig);
	    uhh_sysconfig &= ~(3 << OMAP_UHH_SYSCONFIG_MIDLEMODE_SHIFT);
        uhh_sysconfig &= ~(3 << OMAP_UHH_SYSCONFIG_SIDLEMODE_SHIFT);
        ohci_omap_writel(omap->uhh_base, OMAP_UHH_SYSCONFIG, uhh_sysconfig);

        //ohci_context_save();

	    //ohci_clocks->suspended = 1;
	    omap3_ohci_clock_power(omap, 0);
        omap->suspended =1;
	    clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	}

#ifdef CONFIG_AP2MODEM_VIATELECOM
    sleep_modem();
#endif
	return rc;
}

static int omap_ohci_bus_resume(struct usb_hcd *hcd)
{
	int rc;
	u32 uhh_sysconfig =0;
	struct ohci_hcd		*ohci = hcd_to_ohci (hcd);
	struct ohci_hcd_omap *omap = platform_get_drvdata(
                container_of(hcd->self.controller, struct platform_device, dev));

    printk("\n===>%s() states=%s \n",__func__,omap->suspended?"SLEEPED":"WAKED");

    if(omap->suspended)
    {
        spin_lock_irq (&ohci->lock);
	    omap3_ohci_clock_power(omap, 1);
	    set_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);

	    uhh_sysconfig = ohci_omap_readl(omap->uhh_base,OMAP_UHH_SYSCONFIG);
	    printk("sguan-%s-UHH_SYSCONFIG value is 0x%x\n", __func__, uhh_sysconfig);
	    uhh_sysconfig &= ~(3 << OMAP_UHH_SYSCONFIG_MIDLEMODE_SHIFT);
	    uhh_sysconfig &= ~(3 << OMAP_UHH_SYSCONFIG_SIDLEMODE_SHIFT);
	    uhh_sysconfig |= (1 << OMAP_UHH_SYSCONFIG_MIDLEMODE_SHIFT);
	    uhh_sysconfig |= (1 << OMAP_UHH_SYSCONFIG_SIDLEMODE_SHIFT);
	    ohci_omap_writel(omap->uhh_base, OMAP_UHH_SYSCONFIG, uhh_sysconfig);
	    spin_unlock_irq (&ohci->lock);

	    rc = ohci_bus_resume(hcd);

        omap->suspended =0;
    }
#ifdef CONFIG_AP2MODEM_VIATELECOM
	wake_modem();
#endif
	return rc;
}

#endif //#ifdef CONFIG_PM
/*-------------------------------------------------------------------------*/

static const struct hc_driver ohci_omap_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"OMAP3 OHCI Host Controller",
	.hcd_priv_size =	sizeof(struct ohci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			ohci_irq,
	.flags =		HCD_USB11 | HCD_MEMORY,

	/*
	 * basic lifecycle operations
	 */
	.reset =		ohci_omap_init,
	.start =		ohci_omap_start,
	.stop =			ohci_stop,
	.shutdown =		ohci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ohci_urb_enqueue,
	.urb_dequeue =		ohci_urb_dequeue,
	.endpoint_disable =	ohci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ohci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ohci_hub_status_data,
	.hub_control =		ohci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend =		omap_ohci_bus_suspend,
	.bus_resume =		omap_ohci_bus_resume,
#endif
	.start_port_reset =	ohci_start_port_reset,
};

/*-------------------------------------------------------------------------*/

/*
 * configure so an HC device and id are always provided
 * always called with process context; sleeping is OK
 */

/**
 * ohci_hcd_omap3_probe - initialize OMAP-based HCDs
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 */
static int ohci_hcd_omap_probe(struct platform_device *pdev)
{
	struct ohci_hcd_omap_platform_data *pdata = pdev->dev.platform_data;
	struct ohci_hcd_omap *omap;
	struct resource *res;
	struct usb_hcd *hcd;

	int irq = platform_get_irq(pdev, 0);
	int ret = -ENODEV;

	if (!pdata) {
		dev_dbg(&pdev->dev, "missing platform_data\n");
		goto err_pdata;
	}

	if (usb_disabled())
		goto err_disabled;

	omap = kzalloc(sizeof(*omap), GFP_KERNEL);
	if (!omap) {
		ret = -ENOMEM;
		goto err_disabled;
	}

	hcd = usb_create_hcd(&ohci_omap_hc_driver, &pdev->dev,
			dev_name(&pdev->dev));

    g_usb_hcd =hcd;
	
	if (!hcd) {
		ret = -ENOMEM;
		goto err_create_hcd;
	}

	platform_set_drvdata(pdev, omap);
	omap->dev		= &pdev->dev;
	omap->port_mode[0]		= pdata->port_mode[0];
	omap->port_mode[1]		= pdata->port_mode[1];
	omap->port_mode[2]		= pdata->port_mode[2];
	omap->ohci		= hcd_to_ohci(hcd);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_err(&pdev->dev, "OHCI ioremap failed\n");
		ret = -ENOMEM;
		goto err_ioremap;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	omap->uhh_base = ioremap(res->start, resource_size(res));
	if (!omap->uhh_base) {
		dev_err(&pdev->dev, "UHH ioremap failed\n");
		ret = -ENOMEM;
		goto err_uhh_ioremap;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	omap->tll_base = ioremap(res->start, resource_size(res));
	if (!omap->tll_base) {
		dev_err(&pdev->dev, "TLL ioremap failed\n");
		ret = -ENOMEM;
		goto err_tll_ioremap;
	}

	ret = omap_start_ohc(omap, hcd);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to start ohci\n");
		goto err_start;
	}

	ohci_hcd_init(omap->ohci);

	ret = usb_add_hcd(hcd, irq, IRQF_DISABLED);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to add hcd with err %d\n", ret);
		goto err_add_hcd;
	}
	omap->suspended = 0;

	return 0;

err_add_hcd:
	omap_stop_ohc(omap, hcd);

err_start:
	iounmap(omap->tll_base);

err_tll_ioremap:
	iounmap(omap->uhh_base);

err_uhh_ioremap:
	iounmap(hcd->regs);

err_ioremap:
	usb_put_hcd(hcd);

err_create_hcd:
	kfree(omap);
err_disabled:
err_pdata:
	return ret;
}

/*
 * may be called without controller electrically present
 * may be called with controller, bus, and devices active
 */

/**
 * ohci_hcd_omap3_remove - shutdown processing for OHCI HCDs
 * @pdev: USB Host Controller being removed
 *
 * Reverses the effect of ohci_hcd_omap3_probe(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 */
static int ohci_hcd_omap_remove(struct platform_device *pdev)
{
	struct ohci_hcd_omap *omap = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = ohci_to_hcd(omap->ohci);

    printk(KERN_ERR "%s() states=%s\n",__func__,omap->suspended?"SLEEP":"WAKE");
    if(omap->suspended)
	{
	    omap3_ohci_clock_power(omap, 1);
	    omap->suspended = 0;
	}

	usb_remove_hcd(hcd);
	omap_stop_ohc(omap, hcd);
	iounmap(hcd->regs);
	iounmap(omap->tll_base);
	iounmap(omap->uhh_base);
	usb_put_hcd(hcd);
	kfree(omap);

	return 0;
}
int usbhost_poweron = 1;
static void ohci_hcd_omap_shutdown(struct platform_device *pdev)
{
	struct ohci_hcd_omap *omap = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = ohci_to_hcd(omap->ohci);

    printk(KERN_ERR "%s() states=%s\n",__func__,omap->suspended?"SLEEP":"WAKE");
    if(omap->suspended)
	{
	    omap3_ohci_clock_power(omap, 1);
	    omap->suspended = 0;
	}

	if((omap_readl(0x48005420)==0x3)||(omap_readl(0x4800544c)==0x0)
		||(omap_readl(0x48005400) == 0x0)||(omap_readl(0x48005410) == 0x0)
		||(omap_readl(0x483074E4) == 0x0)||(omap_readl(0x4800544c) == 0x0))
		usbhost_poweron = 0;
	if (hcd->driver->shutdown)
		hcd->driver->shutdown(hcd);
	usbhost_poweron = 1;
}

static struct platform_driver ohci_hcd_omap3_driver = {
	.probe		= ohci_hcd_omap_probe,
	.remove		= ohci_hcd_omap_remove,
	.shutdown	= ohci_hcd_omap_shutdown,
	.driver		= {
		.name	= "ohci-omap3",
	},
};

MODULE_ALIAS("platform:omap3-ohci");
MODULE_AUTHOR("Anand Gadiyar <gadiyar@ti.com>");

/*
 * kernel/power/suspend.c - Suspend to RAM and standby functionality.
 *
 * Copyright (c) 2003 Patrick Mochel
 * Copyright (c) 2003 Open Source Development Lab
 * Copyright (c) 2009 Rafael J. Wysocki <rjw@sisk.pl>, Novell Inc.
 *
 * This file is released under the GPLv2.
 */

#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/gfp.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <plat/misc.h>
#include <plat/yl_debug.h>//define yl_debug();

#include "power.h"

const char *const pm_states[PM_SUSPEND_MAX] = {
#ifdef CONFIG_EARLYSUSPEND
	[PM_SUSPEND_ON]		= "on",
#endif
	[PM_SUSPEND_STANDBY]	= "standby",
	[PM_SUSPEND_MEM]	= "mem",
};

static struct platform_suspend_ops *suspend_ops;

/**
 *	suspend_set_ops - Set the global suspend method table.
 *	@ops:	Pointer to ops structure.
 */
void suspend_set_ops(struct platform_suspend_ops *ops)
{
	mutex_lock(&pm_mutex);
	suspend_ops = ops;
	mutex_unlock(&pm_mutex);
}

bool valid_state(suspend_state_t state)
{
	/*
	 * All states need lowlevel support and need to be valid to the lowlevel
	 * implementation, no valid callback implies that none are valid.
	 */
	return suspend_ops && suspend_ops->valid && suspend_ops->valid(state);
}

/**
 * suspend_valid_only_mem - generic memory-only valid callback
 *
 * Platform drivers that implement mem suspend only and only need
 * to check for that in their .valid callback can use this instead
 * of rolling their own .valid callback.
 */
int suspend_valid_only_mem(suspend_state_t state)
{
	return state == PM_SUSPEND_MEM;
}

static int suspend_test(int level)
{
#ifdef CONFIG_PM_DEBUG
	if (pm_test_level == level) {
		printk(KERN_INFO "suspend debug: Waiting for 5 seconds.\n");
		mdelay(5000);
		return 1;
	}
#endif /* !CONFIG_PM_DEBUG */
	return 0;
}

/**
 *	suspend_prepare - Do prep work before entering low-power state.
 *
 *	This is common code that is called for each state that we're entering.
 *	Run suspend notifiers, allocate a console and stop all processes.
 */
static int suspend_prepare(void)
{
	int error;

	if (!suspend_ops || !suspend_ops->enter)
		return -EPERM;

	yl_pm_debug(LOG_DEBUG, "%s: pm_prepare_console\n", __func__);
	pm_prepare_console();

	yl_pm_debug(LOG_DEBUG, "%s: pm_notifier_call_chain\n", __func__);
	error = pm_notifier_call_chain(PM_SUSPEND_PREPARE);
	if (error)
		goto Finish;

	yl_pm_debug(LOG_DEBUG, "%s: usermodehelper_disable\n", __func__);
	error = usermodehelper_disable();
	if (error)
		goto Finish;

	yl_pm_debug(LOG_DEBUG, "%s: suspend_freeze_processes\n", __func__);
	error = suspend_freeze_processes();
	if (!error)
		return 0;

	yl_pm_debug(LOG_DEBUG, "%s fail and traw_precesses\n", __func__);
	suspend_thaw_processes();
	usermodehelper_enable();
 Finish:
	yl_pm_debug(LOG_DEBUG, "%s fail\n", __func__);
	pm_notifier_call_chain(PM_POST_SUSPEND);
	pm_restore_console();
	return error;
}

/* default implementation */
void __attribute__ ((weak)) arch_suspend_disable_irqs(void)
{
	local_irq_disable();
}

/* default implementation */
void __attribute__ ((weak)) arch_suspend_enable_irqs(void)
{
	local_irq_enable();
}

/**
 *	suspend_enter - enter the desired system sleep state.
 *	@state:		state to enter
 *
 *	This function should be called after devices have been suspended.
 */
static int suspend_enter(suspend_state_t state)
{
	int error;

	if (suspend_ops->prepare) {
		error = suspend_ops->prepare();
		if (error)
			return error;
	}

	error = dpm_suspend_noirq(PMSG_SUSPEND);
	if (error) {
		printk(KERN_ERR "PM: Some devices failed to power down\n");
		goto Platfrom_finish;
	}

	if (suspend_ops->prepare_late) {
		error = suspend_ops->prepare_late();
		if (error)
			goto Power_up_devices;
	}

	if (suspend_test(TEST_PLATFORM))
		goto Platform_wake;

	error = disable_nonboot_cpus();
	if (error || suspend_test(TEST_CPUS))
		goto Enable_cpus;

	arch_suspend_disable_irqs();
	BUG_ON(!irqs_disabled());

	error = sysdev_suspend(PMSG_SUSPEND);
	if (!error) {
		if (!suspend_test(TEST_CORE))
			error = suspend_ops->enter(state);
		sysdev_resume();
	}

	arch_suspend_enable_irqs();
	BUG_ON(irqs_disabled());

 Enable_cpus:
	enable_nonboot_cpus();

 Platform_wake:
	if (suspend_ops->wake)
		suspend_ops->wake();

 Power_up_devices:
	dpm_resume_noirq(PMSG_RESUME);

 Platfrom_finish:
	if (suspend_ops->finish)
		suspend_ops->finish();

	return error;
}


extern void wake_modem(void);
extern void ohci_resume_rh_autostop_ex(void);
extern int g_usb_autosuspended;
extern void simulate_powerkey_press(void);
/**
 *	suspend_devices_and_enter - suspend devices and enter the desired system
 *				    sleep state.
 *	@state:		  state to enter
 */
int suspend_devices_and_enter(suspend_state_t state)
{
	int error;
	gfp_t saved_mask;
	static int test_count = 0;

	if (!suspend_ops)
		return -ENOSYS;

	if (suspend_ops->begin) {
		error = suspend_ops->begin(state);
		if (error)
			goto Close;
	}
	suspend_console();
	saved_mask = clear_gfp_allowed_mask(GFP_IOFS);
	suspend_test_start();
	yl_pm_debug(LOG_DEBUG, "%s: suspend device\n", __func__);
	error = dpm_suspend_start(PMSG_SUSPEND);
	if (error) {
		printk(KERN_ERR "PM: Some devices failed to suspend\n");
        
        if(g_usb_autosuspended ==1){
            wake_modem();
        
            printk(KERN_ERR "%s++ohci_resume_rh_autostop_ex\n",__func__); 
            ohci_resume_rh_autostop_ex();
        }
		goto Recover_platform;
	}
	suspend_test_finish("suspend devices");
	if (suspend_test(TEST_DEVICES))
		goto Recover_platform;

	yl_pm_debug(LOG_DEBUG, "%s: suspend dev end\n", __func__);
	suspend_enter(state);
	yl_pm_debug(LOG_DEBUG, "%s: exit suspend enter\n", __func__);

 Resume_devices:
	suspend_test_start();
	yl_pm_debug(LOG_DEBUG, "%s: resume device\n", __func__);
	dpm_resume_end(PMSG_RESUME);
	yl_pm_debug(LOG_DEBUG, "%s: resume dev end\n", __func__);

	//test suspend and wakeup, added by huangjiefeng 20110412
	if(STRESS_TEST_SUSPEND_WAKEUP == test_suspend_wakeup)
	{
		test_count++;
		printk(KERN_ERR"\n=========resume count=%4d=========\n\n", test_count);
		simulate_powerkey_press();
	}

	suspend_test_finish("resume devices");
	set_gfp_allowed_mask(saved_mask);
	
	/*
		sometimes system have some warning and will set loglevel to 15, this cause to be slowly in wakeup,
		customer will be unpleasant , so we set loglevel to 4 in release version.

		huangjiefeng in 20120105
	*/
	if(console_loglevel == 15)
		console_loglevel = 4;

	resume_console();
 Close:
	if (suspend_ops->end)
		suspend_ops->end();
	return error;

 Recover_platform:
	if (suspend_ops->recover)
		suspend_ops->recover();
	goto Resume_devices;
}

/**
 *	suspend_finish - Do final work before exiting suspend sequence.
 *
 *	Call platform code to clean up, restart processes, and free the
 *	console that we've allocated. This is not called for suspend-to-disk.
 */
static void suspend_finish(void)
{
	suspend_thaw_processes();
	usermodehelper_enable();
	pm_notifier_call_chain(PM_POST_SUSPEND);
	pm_restore_console();
}

/**
 *	enter_state - Do common work of entering low-power state.
 *	@state:		pm_state structure for state we're entering.
 *
 *	Make sure we're the only ones trying to enter a sleep state. Fail
 *	if someone has beat us to it, since we don't want anything weird to
 *	happen when we wake up.
 *	Then, do the setup for suspend, enter the state, and cleaup (after
 *	we've woken up).
 */
int enter_state(suspend_state_t state)
{
	int error;
	extern void stop_drawing_early_suspend(void);
	extern void start_drawing_late_resume(void);
	extern void lcd_panel_resume(void);//yangliang add 20120427
	if (!valid_state(state))
		return -ENODEV;

	if (!mutex_trylock(&pm_mutex))
		return -EBUSY;
	stop_drawing_early_suspend();

	printk(KERN_INFO "PM: Syncing filesystems ... ");
	suspend_sys_sync_queue();
	printk("done.\n");

	yl_pm_debug(LOG_DEBUG, "PM: Preparing system for %s sleep\n", pm_states[state]);
	error = suspend_prepare();
	if (error)
		goto Unlock;

	if (suspend_test(TEST_FREEZER))
		goto Finish;

	yl_pm_debug(LOG_DEBUG, "PM: Entering %s sleep\n", pm_states[state]);
	error = suspend_devices_and_enter(state);

 Finish:
	yl_pm_debug(LOG_DEBUG, "PM: Finishing wakeup and thaw processes\n");
	lcd_panel_resume();//yangliang add 20120427
	suspend_finish();
	yl_pm_debug(LOG_DEBUG, "PM: suspend_finish\n");
 Unlock:
 	start_drawing_late_resume();

	mutex_unlock(&pm_mutex);
	return error;
}

/**
 *	pm_suspend - Externally visible function for suspending system.
 *	@state:		Enumerated value of state to enter.
 *
 *	Determine whether or not value is within range, get state
 *	structure, and enter (above).
 */
int pm_suspend(suspend_state_t state)
{
	if (state > PM_SUSPEND_ON && state <= PM_SUSPEND_MAX)
		return enter_state(state);
	return -EINVAL;
}
EXPORT_SYMBOL(pm_suspend);

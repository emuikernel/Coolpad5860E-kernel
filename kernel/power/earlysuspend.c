/* kernel/power/earlysuspend.c
 *
 * Copyright (C) 2005-2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/earlysuspend.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/rtc.h>
#include <linux/syscalls.h> /* sys_sync */
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <plat/misc.h>

#include "power.h"

#define AC_CHARGE_DEVICE        1
#define USB_CHARGE_DEVICE       2
#define AC_CHARGE_CURRENT		809
#define USB_CHARGE_CURRENT		500

enum {
	DEBUG_USER_STATE = 1U << 0,
	DEBUG_SUSPEND = 1U << 2,
};
static int debug_mask = DEBUG_USER_STATE;
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

static DEFINE_MUTEX(early_suspend_lock);
static LIST_HEAD(early_suspend_handlers);
static void early_suspend(struct work_struct *work);
static void late_resume(struct work_struct *work);
static DECLARE_WORK(early_suspend_work, early_suspend);
static DECLARE_WORK(late_resume_work, late_resume);
static DEFINE_SPINLOCK(state_lock);
enum {
	SUSPEND_REQUESTED = 0x1,
	SUSPENDED = 0x2,
	SUSPEND_REQUESTED_AND_SUSPENDED = SUSPEND_REQUESTED | SUSPENDED,
};
static int state;
bool system_sts = 1;
extern int ac_charger_flag;

static atomic_t mmc_earlysuspend_status_err; //chuqifang add 110627
//added by chuqifang on 2011-06-27
int mmc_read_earlysuspend_status_err(void)
{
   return atomic_read(&mmc_earlysuspend_status_err);
}

void mmc_clr_earlysuspend_status_err(void)
{
   atomic_set(&mmc_earlysuspend_status_err,0);
}

void mmc_set_earlysuspend_status_err(void)
{
   atomic_set(&mmc_earlysuspend_status_err,1);
}

bool get_system_sts(void)
{
	return system_sts;
}

void register_early_suspend(struct early_suspend *handler)
{
	struct list_head *pos;

	mutex_lock(&early_suspend_lock);
	list_for_each(pos, &early_suspend_handlers) {
		struct early_suspend *e;
		e = list_entry(pos, struct early_suspend, link);
		if (e->level > handler->level)
			break;
	}
	list_add_tail(&handler->link, pos);
	if ((state & SUSPENDED) && handler->suspend)
		handler->suspend(handler);
	mutex_unlock(&early_suspend_lock);
}
EXPORT_SYMBOL(register_early_suspend);

void unregister_early_suspend(struct early_suspend *handler)
{
	mutex_lock(&early_suspend_lock);
	list_del(&handler->link);
	mutex_unlock(&early_suspend_lock);
}
EXPORT_SYMBOL(unregister_early_suspend);

static void early_suspend(struct work_struct *work)
{
	struct early_suspend *pos;
	unsigned long irqflags;
	int abort = 0;
	int i = 0;

	mutex_lock(&early_suspend_lock);
	spin_lock_irqsave(&state_lock, irqflags);
	if (state == SUSPEND_REQUESTED)
		state |= SUSPENDED;
	else
		abort = 1;
	spin_unlock_irqrestore(&state_lock, irqflags);

	if (abort) {
		if (debug_mask & DEBUG_SUSPEND)
			pr_info("early_suspend: abort, state %d\n", state);
		mutex_unlock(&early_suspend_lock);
		goto abort;
	}

	if (debug_mask & DEBUG_SUSPEND)
		pr_info("early_suspend: call handlers\n");

	sys_sync();//fengchunsong.2012-06-7.解决cpu忙的时候、大文件复制、数据库批量操作，按power键不亮屏的问题
	list_for_each_entry(pos, &early_suspend_handlers, link) {
		printk(KERN_DEBUG"%s: %d +", __func__, i);
		if (pos->suspend != NULL)
			pos->suspend(pos);
		printk(KERN_DEBUG"%s: %d -", __func__, i);
		i++;
	}
	mutex_unlock(&early_suspend_lock);

    #if 0//fengchunsong.2011-04-12.解决cpu忙的时候、大文件复制、数据库批量操作，按power键不亮屏的问题
	if (debug_mask & DEBUG_SUSPEND)
		pr_info("early_suspend: sync\n");

	sys_sync();
	#endif
    mmc_set_earlysuspend_status_err();

	system_sts = 0;
	if(ac_charger_flag)
	{
		setup_charge_current(AC_CHARGE_DEVICE, AC_CHARGE_CURRENT);
	}

abort:
	spin_lock_irqsave(&state_lock, irqflags);
	if (state == SUSPEND_REQUESTED_AND_SUSPENDED)
		wake_unlock(&main_wake_lock);
	spin_unlock_irqrestore(&state_lock, irqflags);
}

static void late_resume(struct work_struct *work)
{
	struct early_suspend *pos;
	unsigned long irqflags;
	int abort = 0;
	int i = 0;

    mmc_clr_earlysuspend_status_err();

	mutex_lock(&early_suspend_lock);
	spin_lock_irqsave(&state_lock, irqflags);
	if (state == SUSPENDED)
		state &= ~SUSPENDED;
	else
		abort = 1;
	spin_unlock_irqrestore(&state_lock, irqflags);

	if (abort) {
		if (debug_mask & DEBUG_SUSPEND)
			pr_info("late_resume: abort, state %d\n", state);
		goto abort;
	}
	if (debug_mask & DEBUG_SUSPEND)
		pr_info("late_resume: call handlers\n");
	list_for_each_entry_reverse(pos, &early_suspend_handlers, link)
	{
		printk(KERN_DEBUG"%s: %d +", __func__, i);
		if (pos->resume != NULL)
			pos->resume(pos);
		printk(KERN_DEBUG"%s: %d -", __func__, i);
		i++;
	}

	system_sts = 1;
	if(ac_charger_flag)
	{
		setup_charge_current(USB_CHARGE_DEVICE, USB_CHARGE_CURRENT);
	}

	if (debug_mask & DEBUG_SUSPEND)
		pr_info("late_resume: done\n");
abort:
	mutex_unlock(&early_suspend_lock);
}

void request_suspend_state(suspend_state_t new_state)
{
	unsigned long irqflags;
	int old_sleep;

	spin_lock_irqsave(&state_lock, irqflags);
	old_sleep = state & SUSPEND_REQUESTED;
	if (debug_mask & DEBUG_USER_STATE) {
		struct timespec ts;
		struct rtc_time tm;
		getnstimeofday(&ts);
		rtc_time_to_tm(ts.tv_sec, &tm);
		pr_info("request_suspend_state: %s (%d->%d) at %lld "
			"(%d-%02d-%02d %02d:%02d:%02d.%09lu UTC)\n",
			new_state != PM_SUSPEND_ON ? "sleep" : "wakeup",
			requested_suspend_state, new_state,
			ktime_to_ns(ktime_get()),
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
	}
	if (!old_sleep && new_state != PM_SUSPEND_ON) {
		state |= SUSPEND_REQUESTED;
		queue_work(suspend_work_queue, &early_suspend_work);
	} else if (old_sleep && new_state == PM_SUSPEND_ON) {
		state &= ~SUSPEND_REQUESTED;
		wake_lock(&main_wake_lock);
		queue_work(suspend_work_queue, &late_resume_work);
	}
	requested_suspend_state = new_state;
	spin_unlock_irqrestore(&state_lock, irqflags);
}

suspend_state_t get_suspend_state(void)
{
	return requested_suspend_state;
}

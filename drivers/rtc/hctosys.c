/*
 * RTC subsystem, initialize system time on startup
 *
 * Copyright (C) 2005 Tower Technologies
 * Author: Alessandro Zummo <a.zummo@towertech.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/rtc.h>

/* IMPORTANT: the RTC only stores whole seconds. It is arbitrary
 * whether it stores the most close value or the value with partial
 * seconds truncated. However, it is important that we use it to store
 * the truncated value. This is because otherwise it is necessary,
 * in an rtc sync function, to read both xtime.tv_sec and
 * xtime.tv_nsec. On some processors (i.e. ARM), an atomic read
 * of >32bits is not possible. So storing the most close value would
 * slow down the sync API. So here we have the truncated value and
 * the best guess is to add 0.5s.
 */

int rtc_hctosys_ret = -ENODEV;

static int __init rtc_hctosys(void)
{
	int err = -ENODEV;
	struct rtc_time tm;
	struct timespec tv = {
		.tv_nsec = NSEC_PER_SEC >> 1,
	};
	struct rtc_device *rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);

	if (rtc == NULL) {
		pr_err("%s: unable to open rtc device (%s)\n",
			__FILE__, CONFIG_RTC_HCTOSYS_DEVICE);
		goto err_open;
	}

	err = rtc_read_time(rtc, &tm);
	if (err) {
		dev_err(rtc->dev.parent,
			"hctosys: unable to read the hardware clock\n");
		goto err_read;

	}

	err = rtc_valid_tm(&tm);
	if(err == 0)
	{
		rtc_tm_to_time(&tm, &tv.tv_sec);

		do_settimeofday(&tv);

		dev_info(rtc->dev.parent,
			"setting system clock to "
			"%d-%02d-%02d %02d:%02d:%02d UTC (%u)\n",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			(unsigned int) tv.tv_sec);
	}
	else
	{
		printk(KERN_ERR"hctosys: invalid date/time, set to default time\n");
		tv.tv_nsec = NSEC_PER_SEC >> 1;

		//set default value to rtc when rtc time > 2037.12.31 huangjiefeng
		memset(&tm, 0, sizeof(struct rtc_time));
		tm.tm_year += 100;
		tm.tm_mday += 1;
		rtc_set_time(rtc, &tm);//set to 2000.1.1 0:0:0
			
		rtc_tm_to_time(&tm, &tv.tv_sec);

		do_settimeofday(&tv);

		printk(KERN_ERR
			"setting system clock to default value"
			"%d-%02d-%02d %02d:%02d:%02d UTC (%u)\n",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			(unsigned int) tv.tv_sec);
		rtc_hctosys_ret = 0;
	}

err_invalid:
err_read:
	rtc_class_close(rtc);

err_open:
	rtc_hctosys_ret = err;

	return err;
}

late_initcall(rtc_hctosys);

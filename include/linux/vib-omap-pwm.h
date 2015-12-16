/* include/linux/vib-omap-pwm.h
 *
 * Copyright (C) 2008 Motorola, Inc.
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

#ifndef __VIB_OMAP_PWM_H__
#define __VIB_OMAP_PWM_H__

#define PWM_VIBRATOR_NAME 				"vibrator" 

//yulong.fengchunsong.2011-2-28
#define VIBRATOR_VOLTAGE_GRADE_ZERO			(0)
#define VIBRATOR_VOLTAGE_GRADE_ONE			(1)
#define VIBRATOR_VOLTAGE_GRADE_TWO			(2)
#define VIBRATOR_VOLTAGE_GRADE_THREE		(3)
#define VIBRATOR_VOLTAGE_GRADE_FOURE		(4)

#define VIBRATOR_PWM_LEVEL_MIN			    (0)
#define VIBRATOR_PWM_LEVEL_MAX			    (100)


#define IOCTL_VIBRATOR_MAGIC 				'V'
#define IOCTL_VIBRATOR_ON					_IOWR(IOCTL_VIBRATOR_MAGIC, 1, unsigned long )
#define IOCTL_VIBRATOR_OFF					_IOWR(IOCTL_VIBRATOR_MAGIC, 2, unsigned long )
#define IOCTL_GET_VIBRATOR_SW				_IOWR(IOCTL_VIBRATOR_MAGIC, 3, unsigned long )

//for vibrator voltage grade 2.5v, 2.8v, 3.0v
#define IOCTL_SET_VIBRATOR_VOLTAGE_GRADE			_IOWR(IOCTL_VIBRATOR_MAGIC, 4, unsigned long )
#define IOCTL_GET_VIBRATOR_VOLTAGE_GRADE			_IOWR(IOCTL_VIBRATOR_MAGIC, 5, unsigned long )

//for PWM output level
#define IOCTL_SET_VIBRATOR_PWM_LEVEL			_IOWR(IOCTL_VIBRATOR_MAGIC, 6, unsigned long )
#define IOCTL_GET_VIBRATOR_PWM_LEVEL			_IOWR(IOCTL_VIBRATOR_MAGIC, 7, unsigned long )

void __init vibrator_omap_pwm_init(int initial_vibrate);
void vibrator_haptic_fire(int value);

#endif


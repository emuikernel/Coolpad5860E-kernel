#ifndef _TOUCH_DEBUG_H
#define _TOUCH_DEBUG_H

#include <plat/misc.h>

#define yl_touch_debug(fmt, ...)   \
         if(yl_get_debug_enalbe(YL_TOUCH_DEBUG_ENABLE))  \
		 	printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)

#endif

/*
 * linux/drivers/input/keyboard/omap-keypad.c
 *
 * OMAP Keypad Driver
 *
 * Copyright (C) 2003 Nokia Corporation
 * Written by Timo Teräs <ext-timo.teras@nokia.com>
 *
 * Added support for H2 & H3 Keypad
 * Copyright (C) 2004 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <plat/keypad.h>
#include <plat/menelaus.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <plat/mux.h>
#include <linux/input/matrix_keypad.h>
#include "../../../arch/arm/mach-omap2/mux.h"


#define OMAP_KPD_TAG  "OMAP keypad: "

//#define OMAP_KPD_DEBUG //调试时打开，传CC时关闭
#ifdef OMAP_KPD_DEBUG	    
#define OMAP_KPD_DBG(fmt, ...) \
	    printk(KERN_ERR OMAP_KPD_TAG pr_fmt(fmt), ##__VA_ARGS__)	    
#else
#define OMAP_KPD_DBG(fmt, ...) do{ \
	}while(0)
#endif

static void omap_kp_tasklet(unsigned long);
static void omap_kp_timer(unsigned long);

static unsigned char keypad_state[8];
static DEFINE_MUTEX(kp_enable_mutex);
static int kp_enable = 1;
static int kp_cur_group = -1;

struct omap_kp {
	struct input_dev *input;
	struct timer_list timer;
	int irq;
	unsigned int rows;
	unsigned int cols;
	unsigned long delay;
	unsigned int debounce;
};

static DECLARE_TASKLET_DISABLED(kp_tasklet, omap_kp_tasklet, 0);

static int *keymap;
static unsigned int *row_gpios;
static unsigned int *col_gpios;

#if defined(CONFIG_ARCH_OMAP2) || defined(CONFIG_ARCH_OMAP3)
static void set_col_gpio_val(struct omap_kp *omap_kp, u8 value)
{
	int col;

	for (col = 0; (col < omap_kp->cols)  && (col_gpios[col]<OMAP_MAX_GPIO_LINES); col++)
	{
		gpio_set_value(col_gpios[col], value & (1 << col));
	}
}

static u8 get_row_gpio_val(struct omap_kp *omap_kp)
{
	int row;
	u8 value = 0;

	for (row = 0; row < omap_kp->rows; row++) {
		if (gpio_get_value(row_gpios[row]))
			value |= (1 << row);
	}
	return value;
}
#else
#define		set_col_gpio_val(x, y)	do {} while (0)
#define		get_row_gpio_val(x)	0
#endif

static irqreturn_t omap_kp_interrupt(int irq, void *dev_id)
{
	struct omap_kp *omap_kp = dev_id;

	/* disable keyboard interrupt and schedule for handling */
	if (cpu_is_omap24xx() || cpu_is_omap34xx()) {
		unsigned int i;

		for (i = 0; i < omap_kp->rows; i++) {
			int gpio_irq = gpio_to_irq(row_gpios[i]);
			
			/*
			 * The interrupt which we're currently handling should
			 * be disabled _nosync() to avoid deadlocks waiting
			 * for this handler to complete.  All others should
			 * be disabled the regular way for SMP safety.
			 */
			if (gpio_irq == irq)
				disable_irq_nosync(gpio_irq);
			else
				disable_irq(gpio_irq);
		}
	} else
		/* disable keyboard interrupt and schedule for handling */
		omap_writew(1, OMAP1_MPUIO_BASE + OMAP_MPUIO_KBD_MASKIT);

	tasklet_schedule(&kp_tasklet);

	return IRQ_HANDLED;
}

static void omap_kp_timer(unsigned long data)
{
	tasklet_schedule(&kp_tasklet);
}

static void omap_kp_scan_keypad(struct omap_kp *omap_kp, unsigned char *state)
{
	unsigned int col = 0;

	/* read the keypad status */
	if (cpu_is_omap24xx() || cpu_is_omap34xx()) {
		/* read the keypad status */
		for (col = 0; col < omap_kp->cols; col++) {
			set_col_gpio_val(omap_kp, ~(1 << col));
			state[col] = (get_row_gpio_val(omap_kp)) & 0xff;//home键默认是低电平，翻转一下
		}
		set_col_gpio_val(omap_kp, 0);

	} else {
		/* disable keyboard interrupt and schedule for handling */
		omap_writew(1, OMAP1_MPUIO_BASE + OMAP_MPUIO_KBD_MASKIT);

		/* read the keypad status */
		omap_writew(0xff, OMAP1_MPUIO_BASE + OMAP_MPUIO_KBC);
		for (col = 0; col < omap_kp->cols; col++) {
			omap_writew(~(1 << col) & 0xff,
				    OMAP1_MPUIO_BASE + OMAP_MPUIO_KBC);

			udelay(omap_kp->delay);

			state[col] = ~omap_readw(OMAP1_MPUIO_BASE +
						 OMAP_MPUIO_KBR_LATCH) & 0xff;
		}
		omap_writew(0x00, OMAP1_MPUIO_BASE + OMAP_MPUIO_KBC);
		udelay(2);
	}
}

static inline int omap_kp_find_key(int col, int row)
{
	int i, key;

	key = KEY(row,col,0);
	for (i = 0; keymap[i] != 0; i++)
		if ((keymap[i] & 0xff000000) == key)
			return keymap[i] & 0x00ffffff;
	return -1;
}

static void omap_kp_tasklet(unsigned long data)
{
	struct omap_kp *omap_kp_data = (struct omap_kp *) data;
	unsigned char new_state[8], changed, key_down = 0;
	int col, row;
	int spurious = 0;

	/* check for any changes */
	omap_kp_scan_keypad(omap_kp_data, new_state);

	/* check for changes and print those */
	for (col = 0; col < omap_kp_data->cols; col++) {
		changed = new_state[col] ^ keypad_state[col];
		key_down |= new_state[col];
		if (changed == 0)
			continue;

		for (row = 0; row < omap_kp_data->rows; row++) {
			int key;
			if (!(changed & (1 << row)))
				continue;

			OMAP_KPD_DBG("key %d-%d %s\n", col,
			       row, (new_state[col] & (1 << row)) ?
			       "pressed" : "released");

			/* Keymappings have changed in omap4.*/
			if (cpu_is_omap44xx())
				key = omap_kp_find_key(row, col);
			else
				key = omap_kp_find_key(col, row);

			if (key < 0) {
				OMAP_KPD_DBG("Spurious key event %d-%d\n",
				       col, row);
				/* We scan again after a couple of seconds */
				spurious = 1;
				continue;
			}

			if (!(kp_cur_group == (key & GROUP_MASK) ||
			      kp_cur_group == -1))
				continue;

			kp_cur_group = key & GROUP_MASK;

			input_report_key(omap_kp_data->input,
				key, new_state[col]
					 & (1 << row));
					 
		    input_sync(omap_kp_data->input);
		}
	}
	memcpy(keypad_state, new_state, sizeof(keypad_state));

	if (key_down) {
                int delay = HZ / 20;
		/* some key is pressed - keep irq disabled and use timer
		 * to poll the keypad */
		if (spurious)
			delay = 2 * HZ;
		mod_timer(&omap_kp_data->timer, jiffies + delay);
	} else {
		/* enable interrupts */
		if (cpu_is_omap24xx() || cpu_is_omap34xx()) {
			int i;
			for (i = 0; i < omap_kp_data->rows; i++)
				enable_irq(gpio_to_irq(row_gpios[i]));
		} else {
			omap_writew(0, OMAP1_MPUIO_BASE + OMAP_MPUIO_KBD_MASKIT);
			kp_cur_group = -1;
		}
	}
}

static ssize_t omap_kp_enable_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", kp_enable);
}

static ssize_t omap_kp_enable_store(struct device *dev, struct device_attribute *attr,
				    const char *buf, size_t count)
{
	int state;

	if (sscanf(buf, "%u", &state) != 1)
		return -EINVAL;

	if ((state != 1) && (state != 0))
		return -EINVAL;

	mutex_lock(&kp_enable_mutex);
	if (state != kp_enable) {
		if (state)
			enable_irq(INT_KEYBOARD);
		else
			disable_irq(INT_KEYBOARD);
		kp_enable = state;
	}
	mutex_unlock(&kp_enable_mutex);

	return strnlen(buf, count);
}

static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, omap_kp_enable_show, omap_kp_enable_store);

#ifdef CONFIG_PM
static int omap_kp_suspend(struct platform_device *dev, pm_message_t state)
{
	/* Nothing yet */

	return 0;
}

static int omap_kp_resume(struct platform_device *dev)
{
	/* Nothing yet */

	return 0;
}
#else
#define omap_kp_suspend	NULL
#define omap_kp_resume	NULL
#endif

static int __devinit omap_kp_probe(struct platform_device *pdev)
{
	struct omap_kp *omap_kp;
	struct input_dev *input_dev;
	struct omap_kp_platform_data *pdata =  pdev->dev.platform_data;
	int i, col_idx, row_idx, irq_idx, ret;

	if (!pdata->rows || !pdata->cols || !pdata->keymap) {
		OMAP_KPD_DBG("No rows, cols or keymap from pdata\n");
		return -EINVAL;
	}

	omap_kp = kzalloc(sizeof(struct omap_kp), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!omap_kp || !input_dev) {
		kfree(omap_kp);
		input_free_device(input_dev);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, omap_kp);

	omap_kp->input = input_dev;

	/* Disable the interrupt for the MPUIO keyboard */
	if (!cpu_is_omap24xx() && !cpu_is_omap34xx())
		omap_writew(1, OMAP1_MPUIO_BASE + OMAP_MPUIO_KBD_MASKIT);

	keymap = pdata->keymap;

	if (pdata->rep)
		__set_bit(EV_REP, input_dev->evbit);

	if (pdata->delay)
		omap_kp->delay = pdata->delay;

	if (pdata->row_gpios && pdata->col_gpios) {
		row_gpios = pdata->row_gpios;
		col_gpios = pdata->col_gpios;
	}

	omap_kp->rows = pdata->rows;
	omap_kp->cols = pdata->cols;

	//配置GPIO
    omap_mux_init_signal("gpio_7", OMAP_PIN_INPUT);

	if (cpu_is_omap24xx() || cpu_is_omap34xx()) {
		/* Cols: outputs */
		for (col_idx = 0; (col_idx < omap_kp->cols) && (col_gpios[col_idx]<OMAP_MAX_GPIO_LINES); col_idx++) {		    
			if (gpio_request(col_gpios[col_idx], "omap_kp_col") < 0) {
				OMAP_KPD_DBG("Failed to request"
				       "GPIO%d for keypad\n",
				       col_gpios[col_idx]);
				goto err1;
			}
			gpio_direction_output(col_gpios[col_idx], 0);
		}
		/* Rows: inputs */
		for (row_idx = 0; row_idx < omap_kp->rows; row_idx++) {
			if (gpio_request(row_gpios[row_idx], "omap_kp_row") < 0) {
				OMAP_KPD_DBG("Failed to request"
				       "GPIO%d for keypad\n",
				       row_gpios[row_idx]);
				goto err2;
			}			
			gpio_direction_input(row_gpios[row_idx]);
		}
	} else {
		col_idx = 0;
		row_idx = 0;
	}

	setup_timer(&omap_kp->timer, omap_kp_timer, (unsigned long)omap_kp);

	/* get the irq and init timer*/
	tasklet_enable(&kp_tasklet);
	kp_tasklet.data = (unsigned long) omap_kp;

	ret = device_create_file(&pdev->dev, &dev_attr_enable);
	if (ret < 0)
		goto err2;

	/* setup input device */
	__set_bit(EV_KEY, input_dev->evbit);
	for (i = 0; keymap[i] != 0; i++)
		__set_bit( KEY_VAL(keymap[i]) & KEY_MAX, input_dev->keybit);
	input_dev->name = "omap-keypad";
	input_dev->phys = "omap-keypad/input0";
	input_dev->dev.parent = &pdev->dev;

	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0100;

	ret = input_register_device(omap_kp->input);
	if (ret < 0) {
		OMAP_KPD_DBG("Unable to register omap-keypad input device\n");
		goto err3;
	}

	if (pdata->dbounce)
		omap_writew(0xff, OMAP1_MPUIO_BASE + OMAP_MPUIO_GPIO_DEBOUNCING);

	/* scan current status and enable interrupt */
	omap_kp_scan_keypad(omap_kp, keypad_state);
	if (!(cpu_is_omap24xx() || cpu_is_omap34xx())) {
		omap_kp->irq = platform_get_irq(pdev, 0);
		if (omap_kp->irq >= 0) {
			if (request_irq(omap_kp->irq, omap_kp_interrupt, 0,
					"omap-keypad", omap_kp) < 0)
				goto err4;
		}
		omap_writew(0, OMAP1_MPUIO_BASE + OMAP_MPUIO_KBD_MASKIT);
	} else {
		for (irq_idx = 0; irq_idx < omap_kp->rows; irq_idx++) {
			if (request_irq(gpio_to_irq(row_gpios[irq_idx]),
					omap_kp_interrupt,
					(IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING),
					"omap-keypad", omap_kp) < 0)
				goto err5;
		}
	}
	return 0;
err5:
	for (i = irq_idx - 1; i >=0; i--)
		free_irq(row_gpios[i], 0);
err4:
	input_unregister_device(omap_kp->input);
	input_dev = NULL;
err3:
	device_remove_file(&pdev->dev, &dev_attr_enable);
err2:
	for (i = row_idx - 1; i >=0; i--)
		gpio_free(row_gpios[i]);
err1:
	for (i = col_idx - 1; i >=0; i--)
		gpio_free(col_gpios[i]);

	kfree(omap_kp);
	input_free_device(input_dev);

	return -EINVAL;
}

static int __devexit omap_kp_remove(struct platform_device *pdev)
{
	struct omap_kp *omap_kp = platform_get_drvdata(pdev);

	/* disable keypad interrupt handling */
	tasklet_disable(&kp_tasklet);
	if (cpu_is_omap34xx()) {
		int i;
		for (i = 0; (i < omap_kp->cols)  && (col_gpios[i]<OMAP_MAX_GPIO_LINES); i++)
			gpio_free(col_gpios[i]);
		for (i = 0; i < omap_kp->rows; i++) {
			gpio_free(row_gpios[i]);
			free_irq(gpio_to_irq(row_gpios[i]), 0);
		}
	} else {
		omap_writew(1, OMAP1_MPUIO_BASE + OMAP_MPUIO_KBD_MASKIT);
		free_irq(omap_kp->irq, 0);
	}

	del_timer_sync(&omap_kp->timer);
	tasklet_kill(&kp_tasklet);

	/* unregister everything */
	input_unregister_device(omap_kp->input);

	device_remove_file(&pdev->dev, &dev_attr_enable);

	kfree(omap_kp);

	return 0;
}

static struct platform_driver omap_kp_driver = {
	.probe		= omap_kp_probe,
	.remove		= __devexit_p(omap_kp_remove),
	.suspend	= omap_kp_suspend,
	.resume		= omap_kp_resume,
	.driver		= {
		.name	= "omap-keypad",
		.owner	= THIS_MODULE,
	},
};

static int __init omap_kp_init(void)
{
	return platform_driver_register(&omap_kp_driver);
}

static void __exit omap_kp_exit(void)
{
	platform_driver_unregister(&omap_kp_driver);
}

module_init(omap_kp_init);
module_exit(omap_kp_exit);

MODULE_AUTHOR("fengchunsong");
MODULE_DESCRIPTION("OMAP Keypad Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:omap-keypad");

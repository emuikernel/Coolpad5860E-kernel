#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/gpio.h>
#include <plat/mux.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

struct device *pcm_select_dev;

static ssize_t pcm_select_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	printk("pcm_select_show\n");
}

static ssize_t pcm_select_store(struct device *dev,
				 struct device_attribute *attr,
				 char *buf,size_t count)
{
	int error;
	unsigned long state;
	printk("pcm_select_store\n");
	error = strict_strtoul(buf, 0, &state);
	if (error)
		return error;
	if (state != 0 && state != 1)
		return -EINVAL;

	switch (state) {
	case 0:
		printk("set gpio127 value to 0\n");
		gpio_set_value(127, 0);
		break;
	case 1:
		printk("set gpio127 value to 1\n");
		gpio_set_value(127, 1);
		break;
	default:
		printk(KERN_ERR "invalid  state %d\n", state);
	}

	return count;

}

static struct device_attribute pcm_src_select_dev_attrs[] = {
	__ATTR(pcm_select, S_IRUGO|S_IWUSR, pcm_select_show, pcm_select_store),
	__ATTR_NULL
};

static struct class pcm_src_select_class = {
	.name		= "pcm_src_select",
	.dev_attrs	= pcm_src_select_dev_attrs,
};




static int __init pcm_select_init(void)
{
	int error;
	struct device *dev; 
	error = class_register(&pcm_src_select_class);
	if (error) {
		printk(KERN_ERR "rfkill: unable to register rfkill class\n");
		return error;
	}

	dev = kzalloc(sizeof(struct device), GFP_KERNEL);
	if (!dev)
		return NULL;

	pcm_select_dev = dev;
	dev->class = &pcm_src_select_class;
	dev->parent = NULL;
	device_initialize(dev);
	dev_set_name(dev, "pcm_src_sel");
	error = device_add(dev);
	if (error) {
		printk("device add error by xiao\n");
		return error;
	}
	return 0;
}

static void __exit pcm_select_exit(void)
{
	printk("device: '%s': %s\n", dev_name(pcm_select_dev), __func__);
	device_del(pcm_select_dev);
	put_device(pcm_select_dev);//if reference count is 0, then free its memory
	class_unregister(&pcm_src_select_class);
}

module_exit(pcm_select_exit);
subsys_initcall(pcm_select_init);

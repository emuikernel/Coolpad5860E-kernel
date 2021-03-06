/*
 * drivers/input/touchscreen/touchscreen.c
 * Copyright (C) 2010 YULONG Company.
 * Author: YULONG Company
 *
 * This program is free dispware; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free dispware Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include "touchscreen.h"


touchscreen_ops_tpye *touchscreen_ops[2];

#define TOUCH_IN_ACTIVE(num) (touchscreen_ops[num] && touchscreen_ops[num]->active && touchscreen_ops[num]->active())

static DEFINE_MUTEX(touchscreen_mutex);

/**********************************************************************
* 函数名称：touchscreen_set_ops

* 功能描述：设置触摸屏节点操作函数
				  
* 输入参数：touchscreen_ops_tpye

* 输出参数：NONE  

* 返回值      ：0---成功，-EBUSY---失败

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
int touchscreen_set_ops(touchscreen_ops_tpye *ops)
{
	if(ops==NULL || ops->touch_id>1 )
	{
		printk("ops error!\n");
		return -EBUSY;
	}
	
	mutex_lock(&touchscreen_mutex);
	if(touchscreen_ops[ops->touch_id]!=NULL)  
	{
		printk("ops has been used!\n");
		mutex_unlock(&touchscreen_mutex);
		return -EBUSY;
	}
       touchscreen_ops[ops->touch_id] = ops;
	mutex_unlock(&touchscreen_mutex);
	
	printk("ops add success!\n");
	return 0;
}

/**********************************************************************
* 函数名称：touchscreen_type_show

* 功能描述：读当前触摸屏类型
				  
* 输入参数：buf

* 输出参数：buf: 1---电容屏，2---电阻屏

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_type_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		ret = touchscreen_ops[0]->touch_type;
	}
	else if(TOUCH_IN_ACTIVE(1))
	{
		ret = touchscreen_ops[1]->touch_type;
	}	
	mutex_unlock(&touchscreen_mutex);
	
	return sprintf(buf, "%d\n",ret);
	
}

/**********************************************************************
* 函数名称：touchscreen_active_show

* 功能描述：读当前触摸屏状态
				  
* 输入参数：buf

* 输出参数：buf: 1--当前使用状态，0--待机状态    

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_active_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret=0;
	int ret1=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if (touchscreen_ops[0] && touchscreen_ops[0]->active) 
	{
		ret = touchscreen_ops[0]->active();
	}

	if (touchscreen_ops[1] && touchscreen_ops[1]->active) 
	{
		ret1 = touchscreen_ops[1]->active();
	}
	mutex_unlock(&touchscreen_mutex);
	
	printk("%d,%d in %s\n",ret,ret1,__FUNCTION__);
	return sprintf(buf, "%d,%d\n",ret,ret1);
}

/**********************************************************************
* 函数名称：touchscreen_firmware_update_show

* 功能描述：查询触摸屏固件是否需要更新
				  
* 输入参数：buf

* 输出参数：buf: 1--需要升级固件，0--固件已经是最新 

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t touchscreen_firmware_update_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0]->firmware_need_update)
	       	ret = touchscreen_ops[0]->firmware_need_update();
	}
	else if(TOUCH_IN_ACTIVE(1)) 
	{
		if(touchscreen_ops[1]->firmware_need_update)
	       	ret = touchscreen_ops[1]->firmware_need_update();
	}
	mutex_unlock(&touchscreen_mutex);
	
	return sprintf(buf, "%d\n",ret);
}


/**********************************************************************
* 函数名称：touchscreen_firmware_update_store

* 功能描述：更新固件
				  
* 输入参数：buf:系统写"update"  

* 输出参数：buf

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_firmware_update_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
	int ret=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	if(strncmp(buf, "update",count-1))
	{
		printk("string is %s,count=%d not update!\n",buf,count);
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
	     if(touchscreen_ops[0]->firmware_need_update && touchscreen_ops[0]->firmware_need_update() && touchscreen_ops[0]->firmware_do_update)
		{
                   ret = touchscreen_ops[0]->firmware_do_update();
            	}
	}
	else if(TOUCH_IN_ACTIVE(1))
	{
  	     if(touchscreen_ops[1]->firmware_need_update && touchscreen_ops[1]->firmware_need_update() && touchscreen_ops[1]->firmware_do_update)
		{
                   ret = touchscreen_ops[1]->firmware_do_update();
            	}
	}
	mutex_unlock(&touchscreen_mutex);
	
	return ret;
}


/**********************************************************************
* 函数名称：touchscreen_calibrate_store

* 功能描述：触摸屏校准
				  
* 输入参数：buf:系统写"calibrate"  

* 输出参数：buf

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t touchscreen_calibrate_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}	

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0]->need_calibrate)
	       	ret = touchscreen_ops[0]->need_calibrate();
	}
	else if(TOUCH_IN_ACTIVE(1)) 
	{
		if(touchscreen_ops[1]->need_calibrate)
	       	ret = touchscreen_ops[1]->need_calibrate();
	}
	mutex_unlock(&touchscreen_mutex);
	
	return sprintf(buf, "%d\n",ret);
}

/**********************************************************************
* 函数名称：touchscreen_calibrate_store

* 功能描述：触摸屏校准
				  
* 输入参数：buf:系统写"calibrate"  

* 输出参数：buf

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_calibrate_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
	int ret=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}	

	if(strncmp(buf, "calibrate",count-1))
	{
		printk("string is %s,not calibrate!\n",buf);
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0]->calibrate)
	       	ret = touchscreen_ops[0]->calibrate();
	}
	else if(TOUCH_IN_ACTIVE(1)) 
	{
		if(touchscreen_ops[1]->calibrate)
	       	ret = touchscreen_ops[1]->calibrate();
	}
	mutex_unlock(&touchscreen_mutex);
	
	return ret;

}


/**********************************************************************
* 函数名称：touchscreen_firmware_version_show

* 功能描述：设置手写模式  
				  
* 输入参数：buf

* 输出参数：buf 触摸屏厂家、ic型号、固件版本

* 返回值      ：size 返回长度

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_firmware_version_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	char version[32]={0};

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0]->get_firmware_version)
	       	touchscreen_ops[0]->get_firmware_version(version);		
	}
	else if(TOUCH_IN_ACTIVE(1))
	{
		if(touchscreen_ops[1]->get_firmware_version)
	       	touchscreen_ops[1]->get_firmware_version(version);		
	}
	mutex_unlock(&touchscreen_mutex);

	return sprintf(buf, "%s\n",version);
}


/**********************************************************************
* 函数名称：touchscreen_reset_store

* 功能描述：设置手写模式  
				  
* 输入参数：buf:通信写"reset"

* 输出参数：buf

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_reset_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
	int ret=0;


	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}
	

	if(strncmp(buf, "reset",count-1))
	{
		printk("string is %s,not reset!\n",buf);
		return -EINVAL;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0]->reset_touchscreen)
	       	ret = touchscreen_ops[0]->reset_touchscreen();		
	}
	else if(TOUCH_IN_ACTIVE(1))
	{
		if(touchscreen_ops[1]->reset_touchscreen)
	       	ret = touchscreen_ops[1]->reset_touchscreen();		
	}
	mutex_unlock(&touchscreen_mutex);

	return ret;

}

/**********************************************************************
* 函数名称：touchscreen_mode_show

* 功能描述：读当前触摸屏工作模式
				  
* 输入参数：buf

* 输出参数：buf: 1--手写，0--正常 

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_mode_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0)) 
	{
		if(touchscreen_ops[0]->get_mode)
		ret = touchscreen_ops[0]->get_mode();
	}
	else if(TOUCH_IN_ACTIVE(1)) 
	{
		if(touchscreen_ops[1]->get_mode)
		ret = touchscreen_ops[1]->get_mode();
	}
	mutex_unlock(&touchscreen_mutex);
	
	return sprintf(buf, "%d\n",ret);
}

/**********************************************************************
* 函数名称：touchscreen_work_mode_store

* 功能描述：设置工作模式
				  
* 输入参数：buf:输入法"handwrite" "normal"    

* 输出参数：buf

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_mode_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
	int ret=0;
	int mode=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	if(strncmp(buf, "normal",count-1)==0)
		mode=MODE_NORMAL;	
	else if(strncmp(buf, "handwrite",count-1)==0)
		mode=MODE_HANDWRITE;
	else
	{
		printk("Don't support %s mode!\n",buf);
		return -EINVAL;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0]->set_mode)
	       	ret = touchscreen_ops[0]->set_mode(mode);		
	}
	else if(TOUCH_IN_ACTIVE(1))
	{
		if(touchscreen_ops[1]->set_mode)
	       	ret = touchscreen_ops[1]->set_mode(mode);				
	}
	mutex_unlock(&touchscreen_mutex);

	return ret;
}

/**********************************************************************
* 函数名称：touchscreen_oreitation_show

* 功能描述：获取触摸屏当前方位
				  
* 输入参数：buf

* 输出参数：buf:传感器应用"X"    X:1,2,3,4--> 0,90,180,270

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_oreitation_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0)) 
	{
		if(touchscreen_ops[0]->get_oreitation)
			ret = touchscreen_ops[0]->get_oreitation();
	}
	else if(TOUCH_IN_ACTIVE(1)) 
	{
		if(touchscreen_ops[1]->get_oreitation)
			ret = touchscreen_ops[1]->get_oreitation();
	}
	mutex_unlock(&touchscreen_mutex);
	
	return sprintf(buf, "%d\n",ret);
}


/**********************************************************************
* 函数名称：touchscreen_oreitate_store

* 功能描述：设置触摸屏当前方位
				  
* 输入参数：buf:传感器应用"oreitation:X"    X:1,2,3,4--> 0,90,180,270

* 输出参数：buf

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_oreitation_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)  
{
	ssize_t ret=0;
	int oreitation=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}
	

	if(strncmp(buf, "oreitation",count-2))
	{
		printk("string is %s,not oreitation\n",buf);
		return -EINVAL;
	}
	
	oreitation=buf[count-2]-'0';
	printk("oreitation=%d",oreitation);
	if(oreitation<0 || oreitation>3)
	{
		printk("oreitation[%d] is invalid\n",oreitation);
		return -EINVAL;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0]->set_oreitation)
	       	ret = touchscreen_ops[0]->set_oreitation(oreitation);		
	}
	else if(TOUCH_IN_ACTIVE(1))
	{
		if(touchscreen_ops[1]->set_oreitation)
	       	ret = touchscreen_ops[1]->set_oreitation(oreitation);	
	}
	mutex_unlock(&touchscreen_mutex);

	return ret;

}


/**********************************************************************
* 函数名称：touchscreen_read_regs_show

* 功能描述：读触摸屏ic寄存器
				  
* 输入参数：NONE

* 输出参数：buf[256]: ef:ab [寄存器]：值  

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_regs_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret=0;
	
	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0] ->read_regs)
			ret=touchscreen_ops[0] ->read_regs(buf);
	}
	else if(TOUCH_IN_ACTIVE(1))
	{
		if(touchscreen_ops[1] ->read_regs)
			ret=touchscreen_ops[1] ->read_regs(buf);
	}
	mutex_unlock(&touchscreen_mutex);
	
	return ret;
}


/**********************************************************************
* 函数名称：touchscreen_write_regs_store

* 功能描述：写触摸屏寄存器值
				  
* 输入参数：buf[256]: ef:ab [寄存器]：值 

* 输出参数：buf

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_regs_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
	int ret=0;

	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0] ->write_regs)
			ret=touchscreen_ops[0] ->write_regs(buf);
	}
	else if(TOUCH_IN_ACTIVE(1))
	{
		if(touchscreen_ops[1] ->write_regs)
			ret=touchscreen_ops[1] ->write_regs(buf);
	}
	mutex_unlock(&touchscreen_mutex);
	
	return ret;
}


/**********************************************************************
* 函数名称：touchscreen_debug_store

* 功能描述：设置调试模式
				  
* 输入参数：buf:"on" "off"

* 输出参数：buf

* 返回值      ：size

* 其它说明：

* 修改日期         修改人	              修改内容
* --------------------------------------------------------------------
* 2011/11/19	   冯春松                  创 建
**********************************************************************/
static ssize_t  touchscreen_debug_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
	int ret=0;
	int on=0;
	
	if(buf==NULL)
	{
		printk("buf is NULL!\n");
		return -ENOMEM;
	}

	if(strncmp(buf, "on",count-1)==0)
		on=1;

	mutex_lock(&touchscreen_mutex);
	if(TOUCH_IN_ACTIVE(0))
	{
		if(touchscreen_ops[0] ->debug)
			ret= touchscreen_ops[0] ->debug(on);
	}
	else if(TOUCH_IN_ACTIVE(1))
	{
		if(touchscreen_ops[1] ->debug)
			ret= touchscreen_ops[1] ->debug(on);
	}
	mutex_unlock(&touchscreen_mutex);
	
	return ret;
}

static DEVICE_ATTR(type, 0444, touchscreen_type_show, NULL);
static DEVICE_ATTR(active, 0444, touchscreen_active_show, NULL);
static DEVICE_ATTR(firmware_update, 0644, touchscreen_firmware_update_show, touchscreen_firmware_update_store);
static DEVICE_ATTR(calibrate, 0644, touchscreen_calibrate_show, touchscreen_calibrate_store);
static DEVICE_ATTR(firmware_version, 0444, touchscreen_firmware_version_show,NULL);
static DEVICE_ATTR(reset, 0200, NULL, touchscreen_reset_store);
static DEVICE_ATTR(mode, 0644, touchscreen_mode_show, touchscreen_mode_store);
static DEVICE_ATTR(oreitation, 0644, touchscreen_oreitation_show, touchscreen_oreitation_store);
static DEVICE_ATTR(regs, 0644, touchscreen_regs_show, touchscreen_regs_store);
static DEVICE_ATTR(debug, 0200, NULL, touchscreen_debug_store);

static const struct attribute *touchscreen_attrs[] = {
	&dev_attr_type.attr,
	&dev_attr_active.attr,
	&dev_attr_firmware_update.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_firmware_version.attr,
	&dev_attr_reset.attr,
	&dev_attr_mode.attr,
	&dev_attr_oreitation.attr,
	&dev_attr_regs.attr,
	&dev_attr_debug.attr,
	NULL,
};

static const struct attribute_group touchscreen_attr_group = {
	.attrs = (struct attribute **) touchscreen_attrs,
};

static ssize_t export_store(struct class *class, struct class_attribute *attr, const char *buf, size_t len)
{
	return 1;
}

static ssize_t unexport_store(struct class *class, struct class_attribute *attr,const char *buf, size_t len)
{
	return 1;
}

static struct class_attribute uart_class_attrs[] = {
	__ATTR(export, 0200, NULL, export_store),
	__ATTR(unexport, 0200, NULL, unexport_store),
	__ATTR_NULL,
};

static struct class touchscreen_class = {
	.name =		"touchscreen",
	.owner =	THIS_MODULE,

	.class_attrs =	uart_class_attrs,
};

static int touchscreen_export(void)
{
	int	status;
	struct device	*dev;

	dev = device_create(&touchscreen_class, NULL, MKDEV(0, 0), NULL, "touchscreen_dev");
	if (dev) 
	{
		status = sysfs_create_group(&dev->kobj, &touchscreen_attr_group);
	} 
	else
	{
		printk(KERN_ERR"uart switch sysfs_create_group fail\r\n");
		status = -ENODEV;
	}

	return status;
}

static int __init touchscreen_sysfs_init(void)
{
	int	status;
	touchscreen_ops[0]=NULL;
	touchscreen_ops[1]=NULL;
	status = class_register(&touchscreen_class);
	if (status < 0)
	{
		printk(KERN_ERR"uart switch class_register fail\r\n");
		return status;
	}

	status = touchscreen_export();

	return status;
}

arch_initcall(touchscreen_sysfs_init);


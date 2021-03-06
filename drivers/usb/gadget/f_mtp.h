/*
 * Gadget Driver for Android ACM
 *
 * Copyright (C) 2009 Motorola, Inc.
 * Author:
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

#ifndef __F_MTP_H
#define __F_MTP_H

int mtp_bind_config(struct usb_composite_dev *cdev, struct usb_configuration *c);

struct usb_function *mtp_function_enable(int enable,int id);

#endif /* __F_ACM_H */

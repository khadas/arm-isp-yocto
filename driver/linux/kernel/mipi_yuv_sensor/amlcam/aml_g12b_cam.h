/*
*
* SPDX-License-Identifier: GPL-2.0
*
* Copyright (C) 2020 Amlogic or its affiliates
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 2.
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#ifndef __AML_G12B_CAM_H__
#define __AML_G12B_CAM_H__

#ifdef CONFIG_G12B
#define PLATFORM_G12B     1
#endif

#include "cam_csiphy/aml_g12b_csiphy.h"
#include "cam_adapter/aml_g12b_adapter.h"

#define AML_CAM_DRIVER_NAME	"g12b-cam-%u"
#define AML_CAM_BUS_INFO	"platform:" AML_CAM_DRIVER_NAME
#define AML_CAM_COUNT_MAX      8

#if PLATFORM_G12B
#define AO_RTI_GEN_PWR_SLEEP0 	(0xff800000 + 0x3a * 4)
#define AO_RTI_GEN_PWR_ISO0		(0xff800000 + 0x3b * 4)
#define HHI_ISP_MEM_PD_REG0		(0xff63c000 + 0x45 * 4)
#define HHI_ISP_MEM_PD_REG1		(0xff63c000 + 0x46 * 4)
#define HHI_CSI_PHY_CNTL0		(0xff630000 + 0xd3 * 4)
#define HHI_CSI_PHY_CNTL1		(0xff630000 + 0x114 * 4)
#define HWI_ISP_RESET           (0xffd01090)
#endif

enum {
	AML_CAM_0 = 0,
	AML_CAM_1,
};

struct cam_device {
	u32 index;
	char *bus_info;
	struct device *dev;
	struct v4l2_device v4l2_dev;
	struct v4l2_async_notifier notifier;
    struct v4l2_async_subdev cam_asd[1];
    struct v4l2_async_subdev* cam_asd_ptr[1];
	struct media_device media_dev;

	struct csiphy_dev_t csiphy_dev;
	struct adapter_dev_t adap_dev;

	void *priv;
};

struct cam_dev_info {
	u32 cnt;
	struct cam_device *dev_info[AML_CAM_COUNT_MAX];
};

#endif /* __AML_T7_CAM_H__ */

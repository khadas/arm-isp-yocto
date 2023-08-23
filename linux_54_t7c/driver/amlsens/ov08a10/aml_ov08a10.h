// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 */
#include <linux/version.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <media/media-entity.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-subdev.h>
#include <linux/of_platform.h>
#include <linux/of_graph.h>

#define OV08A10_GAIN		0x3508
#define OV08A10_GAIN_L		0x3509
#define OV08A10_EXPOSURE	0x3501
#define OV08A10_EXPOSURE_L	0x3502
#define OV08A10_ID			0x530841
#define OV08A10_SLAVE_ID	0x0036

#define OV08A10_HDR_30FPS_1440M
//#define OV08A10_SDR_60FPS_1440M

struct ov08a10_regval {
	u16 reg;
	u8 val;
};

struct ov08a10_mode {
	u32 width;
	u32 height;
	u32 hmax;
	u32 link_freq_index;

	const struct ov08a10_regval *data;
	u32 data_size;
};

struct ov08a10 {
	int index;
	struct device *dev;
	struct clk *xclk;
	struct regmap *regmap;
	u8 nlanes;
	u8 bpp;
	u32 enWDRMode;

	struct i2c_client *client;
	struct v4l2_subdev sd;
	struct v4l2_fwnode_endpoint ep;
	struct media_pad pad;
	struct v4l2_mbus_framefmt current_format;
	const struct ov08a10_mode *current_mode;

	struct sensor_gpio *gpio;

	struct v4l2_ctrl_handler ctrls;
	struct v4l2_ctrl *link_freq;
	struct v4l2_ctrl *pixel_rate;
	struct v4l2_ctrl *wdr;

	int status;
	struct mutex lock;
};

struct ov08a10_pixfmt {
	u32 code;
	u32 min_width;
	u32 max_width;
	u32 min_height;
	u32 max_height;
	u8 bpp;
};

static const struct ov08a10_pixfmt ov08a10_formats[] = {
	{ MEDIA_BUS_FMT_SBGGR10_1X10, 720, 3840, 720, 2160, 10 },
	{ MEDIA_BUS_FMT_SBGGR12_1X12, 1280, 3840, 720, 2160, 12 },
};

static const struct regmap_config ov08a10_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
};

#ifdef OV08A10_SDR_60FPS_1440M
static struct ov08a10_regval setting_3840_2160_4lane_1440m_60fps[] = {
	{0x0103, 0x01 },
	{0x0303, 0x01 },
	{0x0305, 0x5c },
	{0x0306, 0x00 },
	{0x0308, 0x03 },
	{0x0309, 0x04 },
	{0x032a, 0x00 },
	{0x300f, 0x11 },
	{0x3010, 0x01 },
	{0x3012, 0x41 },
	{0x3016, 0xf0 },
	{0x301e, 0x98 },
	{0x3031, 0xa9 },
	{0x3103, 0x92 },
	{0x3104, 0x01 },
	{0x3106, 0x10 },
	{0x3400, 0x04 },
	{0x3025, 0x03 },
	{0x3425, 0x51 },
	{0x3428, 0x01 },
	{0x3406, 0x08 },
	{0x3408, 0x03 },
	{0x340c, 0xff },
	{0x340d, 0xff },
	{0x031e, 0x09 },
	{0x3501, 0x08 },
	{0x3502, 0xe5 },
	{0x3505, 0x83 },
	{0x3508, 0x00 },
	{0x3509, 0x80 },
	{0x350a, 0x04 },
	{0x350b, 0x00 },
	{0x350c, 0x00 },
	{0x350d, 0x80 },
	{0x350e, 0x04 },
	{0x350f, 0x00 },
	{0x3600, 0x00 },
	{0x3605, 0x50 },
	{0x3609, 0xb5 },
	{0x3610, 0x69 },
	{0x360c, 0x01 },
	{0x3628, 0xa4 },
	{0x362d, 0x10 },
	{0x3660, 0x43 },
	{0x3661, 0x06 },
	{0x3662, 0x00 },
	{0x3663, 0x28 },
	{0x3664, 0x0d },
	{0x366a, 0x38 },
	{0x366b, 0xa0 },
	{0x366d, 0x00 },
	{0x366e, 0x00 },
	{0x3680, 0x00 },
	{0x3701, 0x02 },
	{0x373b, 0x02 },
	{0x373c, 0x02 },
	{0x3736, 0x02 },
	{0x3737, 0x02 },
	{0x3705, 0x00 },
	{0x3706, 0x35 },
	{0x370a, 0x00 },
	{0x370b, 0x98 },
	{0x3709, 0x49 },
	{0x3714, 0x21 },
	{0x371c, 0x00 },
	{0x371d, 0x08 },
	{0x375e, 0x0b },
	{0x3776, 0x10 },
	{0x3781, 0x02 },
	{0x3782, 0x04 },
	{0x3783, 0x02 },
	{0x3784, 0x08 },
	{0x3785, 0x08 },
	{0x3788, 0x01 },
	{0x3789, 0x01 },
	{0x3797, 0x04 },
	{0x3800, 0x00 },
	{0x3801, 0x00 },
	{0x3802, 0x00 },
	{0x3803, 0x0c },
	{0x3804, 0x0e },
	{0x3805, 0xff },
	{0x3806, 0x08 },
	{0x3807, 0x6f },
	{0x3808, 0x0f },
	{0x3809, 0x00 },
	{0x380a, 0x08 },
	{0x380b, 0x70 },
	{0x380c, 0x04 },
	{0x380d, 0x0c },
	{0x380e, 0x09 },
	{0x380f, 0x0a },
	{0x3813, 0x10 },
	{0x3814, 0x01 },
	{0x3815, 0x01 },
	{0x3816, 0x01 },
	{0x3817, 0x01 },
	{0x381c, 0x00 },
	{0x3820, 0x00 },
	{0x3821, 0x04 },
	{0x3823, 0x08 },
	{0x3826, 0x00 },
	{0x3827, 0x08 },
	{0x382d, 0x08 },
	{0x3832, 0x02 },
	{0x383c, 0x48 },
	{0x383d, 0xff },
	{0x3d85, 0x0b },
	{0x3d84, 0x40 },
	{0x3d8c, 0x63 },
	{0x3d8d, 0xd7 },
	{0x4000, 0xf8 },
	{0x4001, 0x2f },
	{0x400a, 0x01 },
	{0x400f, 0xa1 },
	{0x4010, 0x12 },
	{0x4018, 0x04 },
	{0x4008, 0x02 },
	{0x4009, 0x0d },
	{0x401a, 0x58 },
	{0x4050, 0x00 },
	{0x4051, 0x01 },
	{0x4028, 0x0f },
	{0x4052, 0x00 },
	{0x4053, 0x80 },
	{0x4054, 0x00 },
	{0x4055, 0x80 },
	{0x4056, 0x00 },
	{0x4057, 0x80 },
	{0x4058, 0x00 },
	{0x4059, 0x80 },
	{0x430b, 0xff },
	{0x430c, 0xff },
	{0x430d, 0x00 },
	{0x430e, 0x00 },
	{0x4501, 0x18 },
	{0x4502, 0x00 },
	{0x4643, 0x00 },
	{0x4640, 0x01 },
	{0x4641, 0x04 },
	{0x4800, 0x04 },
	{0x4809, 0x2b },
	{0x4813, 0x90 },
	{0x4817, 0x04 },
	{0x4833, 0x18 },
	{0x4837, 0x0a },
	{0x483b, 0x00 },
	{0x484b, 0x03 },
	{0x4850, 0x7c },
	{0x4852, 0x06 },
	{0x4856, 0x58 },
	{0x4857, 0xaa },
	{0x4862, 0x0a },
	{0x4869, 0x18 },
	{0x486a, 0xaa },
	{0x486e, 0x03 },
	{0x486f, 0x55 },
	{0x4875, 0xf0 },
	{0x5000, 0x89 },
	{0x5001, 0x40 },
	{0x5004, 0x40 },
	{0x5005, 0x00 },
	{0x5180, 0x00 },
	{0x5181, 0x10 },
	{0x580b, 0x03 },

	{0x4700, 0x2b },
	{0x4e00, 0x2b },

};

#else
static struct ov08a10_regval setting_3840_2160_4lane_800m_30fps[] = {
	{0x0103, 0x01},
	{0x0303, 0x01},
	{0x0305, 0x32},
	{0x0306, 0x00},
	{0x0308, 0x03},
	{0x0309, 0x04},
	{0x032a, 0x00},
	{0x300f, 0x11},
	{0x3010, 0x01},
	{0x3012, 0x41},
	{0x3016, 0xf0},
	{0x301e, 0x98},
	{0x3031, 0xa9},
	{0x3103, 0x92},
	{0x3104, 0x01},
	{0x3106, 0x10},
	{0x3400, 0x04},
	{0x3025, 0x03},
	{0x3425, 0x51},
	{0x3428, 0x01},
	{0x3406, 0x08},
	{0x3408, 0x03},
	{0x340c, 0xff},
	{0x340d, 0xff},
	{0x031e, 0x09},
	{0x3501, 0x08},
	{0x3502, 0xe5},
	{0x3505, 0x83},
	{0x3508, 0x00},
	{0x3509, 0x80},
	{0x350a, 0x04},
	{0x350b, 0x00},
	{0x350c, 0x00},
	{0x350d, 0x80},
	{0x350e, 0x04},
	{0x350f, 0x00},
	{0x3600, 0x00},
	{0x3605, 0x50},
	{0x3609, 0xb5},
	{0x3610, 0x69},
	{0x360c, 0x01},
	{0x3628, 0xa4},
	{0x362d, 0x10},
	{0x3660, 0x43},
	{0x3661, 0x06},
	{0x3662, 0x00},
	{0x3663, 0x28},
	{0x3664, 0x0d},
	{0x366a, 0x38},
	{0x366b, 0xa0},
	{0x366d, 0x00},
	{0x366e, 0x00},
	{0x3680, 0x00},
	{0x3701, 0x02},
	{0x373b, 0x02},
	{0x373c, 0x02},
	{0x3736, 0x02},
	{0x3737, 0x02},
	{0x3705, 0x00},
	{0x3706, 0x35},
	{0x370a, 0x00},
	{0x370b, 0x98},
	{0x3709, 0x49},
	{0x3714, 0x21},
	{0x371c, 0x00},
	{0x371d, 0x08},
	{0x375e, 0x0b},
	{0x3776, 0x10},
	{0x3781, 0x02},
	{0x3782, 0x04},
	{0x3783, 0x02},
	{0x3784, 0x08},
	{0x3785, 0x08},
	{0x3788, 0x01},
	{0x3789, 0x01},
	{0x3797, 0x04},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x0c},
	{0x3804, 0x0e},
	{0x3805, 0xff},
	{0x3806, 0x08},
	{0x3807, 0x6f},
	{0x3808, 0x0f},
	{0x3809, 0x00},
	{0x380a, 0x08},
	{0x380b, 0x70},
	{0x380c, 0x08},
	{0x380d, 0x18},
	{0x380e, 0x09},
	{0x380f, 0x0a},
	{0x3813, 0x10},
	{0x3814, 0x01},
	{0x3815, 0x01},
	{0x3816, 0x01},
	{0x3817, 0x01},
	{0x381c, 0x00},
	{0x3820, 0x00},
	{0x3821, 0x00},
	{0x3823, 0x08},
	{0x3826, 0x00},
	{0x3827, 0x08},
	{0x382d, 0x08},
	{0x3832, 0x02},
	{0x383c, 0x48},
	{0x383d, 0xff},
	{0x3d85, 0x0b},
	{0x3d84, 0x40},
	{0x3d8c, 0x63},
	{0x3d8d, 0xd7},
	{0x4000, 0xf8},
	{0x4001, 0x2f},
	{0x400a, 0x01},
	{0x400f, 0xa1},
	{0x4010, 0x12},
	{0x4018, 0x04},
	{0x4008, 0x02},
	{0x4009, 0x0d},
	{0x401a, 0x58},
	{0x4050, 0x00},
	{0x4051, 0x01},
	{0x4028, 0x0f},
	{0x4052, 0x00},
	{0x4053, 0x80},
	{0x4054, 0x00},
	{0x4055, 0x80},
	{0x4056, 0x00},
	{0x4057, 0x80},
	{0x4058, 0x00},
	{0x4059, 0x80},
	{0x430b, 0xff},
	{0x430c, 0xff},
	{0x430d, 0x00},
	{0x430e, 0x00},
	{0x4501, 0x18},
	{0x4502, 0x00},
	{0x4643, 0x00},
	{0x4640, 0x01},
	{0x4641, 0x04},
	{0x4800, 0x04},
	{0x4809, 0x2b},
	{0x4813, 0x90},
	{0x4817, 0x04},
	{0x4833, 0x18},
	{0x4837, 0x14},
	{0x483b, 0x00},
	{0x484b, 0x03},
	{0x4850, 0x7c},
	{0x4852, 0x06},
	{0x4856, 0x58},
	{0x4857, 0xaa},
	{0x4862, 0x0a},
	{0x4869, 0x18},
	{0x486a, 0xaa},
	{0x486e, 0x03},
	{0x486f, 0x55},
	{0x4875, 0xf0},
	{0x5000, 0x89},
	{0x5001, 0x40},
	{0x5004, 0x40},
	{0x5005, 0x00},
	{0x5180, 0x00},
	{0x5181, 0x10},
	{0x580b, 0x03},
	{0x4700, 0x2b},
	{0x4e00, 0x2b},
};
#endif

#ifdef OV08A10_HDR_30FPS_1440M
static struct ov08a10_regval setting_hdr_3840_2160_4lane_1440m_30fps[] = {
	{ 0x0100, 0x00},
	{ 0x0103, 0x01},
	{ 0x0303, 0x01},
	{ 0x0305, 0x5a},
	{ 0x0306, 0x00},
	{ 0x0308, 0x03},
	{ 0x0309, 0x04},
	{ 0x032a, 0x00},
	{ 0x300f, 0x11},
	{ 0x3010, 0x01},
	{ 0x3011, 0x04},
	{ 0x3012, 0x41},
	{ 0x3016, 0xf0},
	{ 0x301e, 0x98},
	{ 0x3031, 0xa9},
	{ 0x3103, 0x92},
	{ 0x3104, 0x01},
	{ 0x3106, 0x10},
	{ 0x340c, 0xff},
	{ 0x340d, 0xff},
	{ 0x031e, 0x09},
	{ 0x3501, 0x08},
	{ 0x3502, 0xe5},
	{ 0x3505, 0x83},
	{ 0x3508, 0x00},
	{ 0x3509, 0x80},
	{ 0x350a, 0x04},
	{ 0x350b, 0x00},
	{ 0x350c, 0x00},
	{ 0x350d, 0x80},
	{ 0x350e, 0x04},
	{ 0x350f, 0x00},
	{ 0x3600, 0x00},
	{ 0x3603, 0x2c},
	{ 0x3605, 0x50},
	{ 0x3609, 0xb5},
	{ 0x3610, 0x39},
	{ 0x360c, 0x01},
	{ 0x3628, 0xa4},
	{ 0x362d, 0x10},
	{ 0x3660, 0x42},
	{ 0x3661, 0x07},
	{ 0x3662, 0x00},
	{ 0x3663, 0x28},
	{ 0x3664, 0x0d},
	{ 0x366a, 0x38},
	{ 0x366b, 0xa0},
	{ 0x366d, 0x00},
	{ 0x366e, 0x00},
	{ 0x3680, 0x00},
	{ 0x36c0, 0x00},
	{ 0x3701, 0x02},
	{ 0x373b, 0x02},
	{ 0x373c, 0x02},
	{ 0x3736, 0x02},
	{ 0x3737, 0x02},
	{ 0x3705, 0x00},
	{ 0x3706, 0x39},
	{ 0x370a, 0x00},
	{ 0x370b, 0x98},
	{ 0x3709, 0x49},
	{ 0x3714, 0x21},
	{ 0x371c, 0x00},
	{ 0x371d, 0x08},
	{ 0x3740, 0x1b},
	{ 0x3741, 0x04},
	{ 0x375e, 0x0b},
	{ 0x3760, 0x10},
	{ 0x3776, 0x10},
	{ 0x3781, 0x02},
	{ 0x3782, 0x04},
	{ 0x3783, 0x02},
	{ 0x3784, 0x08},
	{ 0x3785, 0x08},
	{ 0x3788, 0x01},
	{ 0x3789, 0x01},
	{ 0x3797, 0x04},
	{ 0x3762, 0x11},
	{ 0x3800, 0x00},
	{ 0x3801, 0x00},
	{ 0x3802, 0x00},
	{ 0x3803, 0x0c},
	{ 0x3804, 0x0e},
	{ 0x3805, 0xff},
	{ 0x3806, 0x08},
	{ 0x3807, 0x6f},
	{ 0x3808, 0x0f},
	{ 0x3809, 0x00},
	{ 0x380a, 0x08},
	{ 0x380b, 0x70},
	{ 0x380c, 0x04},
	{ 0x380d, 0x0c},
	{ 0x380e, 0x09},
	{ 0x380f, 0x0a},
	{ 0x3813, 0x10},
	{ 0x3814, 0x01},
	{ 0x3815, 0x01},
	{ 0x3816, 0x01},
	{ 0x3817, 0x01},
	{ 0x381c, 0x08},
	{ 0x3820, 0x00},
	{ 0x3821, 0x24},
	{ 0x3823, 0x08},
	{ 0x3826, 0x00},
	{ 0x3827, 0x08},
	{ 0x382d, 0x08},
	{ 0x3832, 0x02},
	{ 0x3833, 0x01},
	{ 0x383c, 0x48},
	{ 0x383d, 0xff},
	{ 0x3d85, 0x0b},
	{ 0x3d84, 0x40},
	{ 0x3d8c, 0x63},
	{ 0x3d8d, 0xd7},
	{ 0x4000, 0xf8},
	{ 0x4001, 0x2b},
	{ 0x4004, 0x00},
	{ 0x4005, 0x40},
	{ 0x400a, 0x01},
	{ 0x400f, 0xa0},
	{ 0x4010, 0x12},
	{ 0x4018, 0x00},
	{ 0x4008, 0x02},
	{ 0x4009, 0x0d},
	{ 0x401a, 0x58},
	{ 0x4050, 0x00},
	{ 0x4051, 0x01},
	{ 0x4028, 0x2f},
	{ 0x4052, 0x00},
	{ 0x4053, 0x80},
	{ 0x4054, 0x00},
	{ 0x4055, 0x80},
	{ 0x4056, 0x00},
	{ 0x4057, 0x80},
	{ 0x4058, 0x00},
	{ 0x4059, 0x80},
	{ 0x430b, 0xff},
	{ 0x430c, 0xff},
	{ 0x430d, 0x00},
	{ 0x430e, 0x00},
	{ 0x4501, 0x18},
	{ 0x4502, 0x00},
	{ 0x4643, 0x00},
	{ 0x4640, 0x01},
	{ 0x4641, 0x04},
	{ 0x4800, 0x04},
	{ 0x4809, 0x2b},
	{ 0x4813, 0x98},
	{ 0x4817, 0x04},
	{ 0x4833, 0x18},
	{ 0x4837, 0x0b},
	{ 0x483b, 0x00},
	{ 0x484b, 0x03},
	{ 0x4850, 0x7c},
	{ 0x4852, 0x06},
	{ 0x4856, 0x58},
	{ 0x4857, 0xaa},
	{ 0x4862, 0x0a},
	{ 0x4869, 0x18},
	{ 0x486a, 0xaa},
	{ 0x486e, 0x07},
	{ 0x486f, 0x55},
	{ 0x4875, 0xf0},
	{ 0x5000, 0x89},
	{ 0x5001, 0x42},
	{ 0x5004, 0x40},
	{ 0x5005, 0x00},
	{ 0x5180, 0x00},
	{ 0x5181, 0x10},
	{ 0x580b, 0x03},
	{ 0x4d00, 0x03},
	{ 0x4d01, 0xc9},
	{ 0x4d02, 0xbc},
	{ 0x4d03, 0xc6},
	{ 0x4d04, 0x4a},
	{ 0x4d05, 0x25},
	{ 0x4700, 0x2b},
	{ 0x4e00, 0x2b},
	{ 0x3501, 0x08},
	{ 0x3502, 0xe1},
	{ 0x3511, 0x00},
	{ 0x3512, 0x20},
	{ 0x3833, 0x01},
};
#else
static struct ov08a10_regval setting_hdr_3840_2160_4lane_848m_15fps[] = {
	{0x0100, 0x00},
	{0x0103, 0x01},
	{0x0303, 0x01},
	{0x0305, 0x35},
	{0x0306, 0x00},
	{0x0308, 0x03},
	{0x0309, 0x04},
	{0x032a, 0x00},
	{0x300f, 0x11},
	{0x3010, 0x01},
	{0x3011, 0x04},
	{0x3012, 0x41},
	{0x3016, 0xf0},
	{0x301e, 0x98},
	{0x3031, 0xa9},
	{0x3103, 0x92},
	{0x3104, 0x01},
	{0x3106, 0x10},
	{0x340c, 0xff},
	{0x340d, 0xff},
	{0x031e, 0x09},
	{0x3501, 0x06},
	{0x3502, 0xe5},
	{0x3505, 0x83},
	{0x3508, 0x00},
	{0x3509, 0x80},
	{0x350a, 0x04},
	{0x350b, 0x00},
	{0x350c, 0x00},
	{0x350d, 0x80},
	{0x350e, 0x04},
	{0x350f, 0x00},
	{0x3600, 0x00},
	{0x3603, 0x2c},
	{0x3605, 0x50},
	{0x3609, 0xb5},
	{0x3610, 0x39},
	{0x360c, 0x01},
	{0x3628, 0xa4},
	{0x362d, 0x10},
	{0x3660, 0x42},
	{0x3661, 0x07},
	{0x3662, 0x00},
	{0x3663, 0x28},
	{0x3664, 0x0d},
	{0x366a, 0x38},
	{0x366b, 0xa0},
	{0x366d, 0x00},
	{0x366e, 0x00},
	{0x3680, 0x00},
	//{0x36c0, 0x 1},
	{0x36c0, 0x00},
	{0x3701, 0x02},
	{0x373b, 0x02},
	{0x373c, 0x02},
	{0x3736, 0x02},
	{0x3737, 0x02},
	{0x3705, 0x00},
	{0x3706, 0x39},
	{0x370a, 0x00},
	{0x370b, 0x98},
	{0x3709, 0x49},
	{0x3714, 0x21},
	{0x371c, 0x00},
	{0x371d, 0x08},
	{0x3740, 0x1b},
	{0x3741, 0x04},
	{0x375e, 0x0b},
	{0x3760, 0x10},
	{0x3776, 0x10},
	{0x3781, 0x02},
	{0x3782, 0x04},
	{0x3783, 0x02},
	{0x3784, 0x08},
	{0x3785, 0x08},
	{0x3788, 0x01},
	{0x3789, 0x01},
	{0x3797, 0x04},
	{0x3798, 0x00},
	{0x3799, 0x00},
	{0x3762, 0x11},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x0c},
	{0x3804, 0x0e},
	{0x3805, 0xff},
	{0x3806, 0x08},
	{0x3807, 0x6f},
	{0x3808, 0x0f},
	{0x3809, 0x00},
	{0x380a, 0x08},
	{0x380b, 0x70},
	{0x3813, 0x10},
	{0x3814, 0x01},
	{0x3815, 0x01},
	{0x3816, 0x01},
	{0x3817, 0x01},
	{0x381c, 0x08},
	{0x3820, 0x00},
	{0x3821, 0x24},
	{0x3822, 0x54},
	{0x3823, 0x08},
	{0x3826, 0x00},
	{0x3827, 0x08},
	{0x382d, 0x08},
	{0x3832, 0x02},
	{0x3833, 0x01},
	{0x383c, 0x48},
	{0x383d, 0xff},
	{0x3d85, 0x0b},
	{0x3d84, 0x40},
	{0x3d8c, 0x63},
	{0x3d8d, 0xd7},
	{0x4000, 0xf8},
	{0x4001, 0x2b},
	{0x4004, 0x00},
	{0x4005, 0x40},
	{0x400a, 0x01},
	{0x400f, 0xa0},
	{0x4010, 0x12},
	{0x4018, 0x00},
	{0x4008, 0x02},
	{0x4009, 0x0d},
	{0x401a, 0x58},
	{0x4050, 0x00},
	{0x4051, 0x01},
	{0x4028, 0x2f},
	{0x4052, 0x00},
	{0x4053, 0x80},
	{0x4054, 0x00},
	{0x4055, 0x80},
	{0x4056, 0x00},
	{0x4057, 0x80},
	{0x4058, 0x00},
	{0x4059, 0x80},
	{0x430b, 0xff},
	{0x430c, 0xff},
	{0x430d, 0x00},
	{0x430e, 0x00},
	{0x4501, 0x18},
	{0x4502, 0x00},
	{0x4643, 0x00},
	{0x4640, 0x01},
	{0x4641, 0x04},
	{0x4800, 0x04},
	{0x4809, 0x2b},
	{0x4813, 0x98},
	{0x4817, 0x04},
	{0x4833, 0x18},
	{0x4837, 0x12},
	{0x483b, 0x00},
	{0x484b, 0x03},
	{0x4850, 0x7c},
	{0x4852, 0x06},
	{0x4856, 0x58},
	{0x4857, 0xaa},
	{0x4862, 0x0a},
	{0x4869, 0x18},
	{0x486a, 0xaa},
	{0x486e, 0x07},
	{0x486f, 0x55},
	{0x4875, 0xf0},
	{0x5000, 0x89},
	{0x5001, 0x42},
	{0x5004, 0x40},
	{0x5005, 0x00},
	{0x5180, 0x00},
	{0x5181, 0x10},
	{0x580b, 0x03},
	{0x4d00, 0x03},
	{0x4d01, 0xc9},
	{0x4d02, 0xbc},
	{0x4d03, 0xc6},
	{0x4d04, 0x4a},
	{0x4d05, 0x25},
	{0x4700, 0x2b},
	{0x4e00, 0x2b},
	{0x0323, 0x02},
	{0x0325, 0x45},
	{0x0328, 0x05},
	{0x0329, 0x01},
	{0x032a, 0x00},
	{0x3106, 0x10},
	{0x380c, 0x07},
	{0x380d, 0xd0},
	{0x380e, 0x08},
	{0x380f, 0xca},
	{0x3810, 0x00},
	{0x3811, 0x00},
	{0x3812, 0x00},
	{0x3813, 0x10},
};
#endif

static const struct ov08a10_regval ov08a10_1080p_settings[] = {

};

static const struct ov08a10_regval ov08a10_720p_settings[] = {

};

static const struct ov08a10_regval ov08a10_10bit_settings[] = {

};

static const struct ov08a10_regval ov08a10_12bit_settings[] = {

};

extern int ov08a10_init(struct i2c_client *client, void *sdrv);
extern int ov08a10_deinit(struct i2c_client *client);
extern int ov08a10_sensor_id(struct i2c_client *client);
extern int ov08a10_power_on(struct device *dev, struct sensor_gpio *gpio);
extern int ov08a10_power_off(struct device *dev, struct sensor_gpio *gpio);
extern int ov08a10_power_suspend(struct device *dev);
extern int ov08a10_power_resume(struct device *dev);

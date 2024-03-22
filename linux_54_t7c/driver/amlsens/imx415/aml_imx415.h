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

#define IMX415_GAIN           0x3090
#define IMX415_EXPOSURE       0x3050
#define IMX415_EXPOSURE_SHR0  IMX415_EXPOSURE
#define IMX415_EXPOSURE_SHR1  0x3054
#define IMX415_ID             0x0602
#define IMX415_SLAVE_ID       0x001A

#define IMX415_HDR_30FPS_1440M
#define IMX415_SDR_30FPS_1440M

struct imx415_regval {
	u16 reg;
	u8 val;
};

struct imx415_mode {
	u32 width;
	u32 height;
	u32 hmax;
	u32 link_freq_index;

	const struct imx415_regval *data;
	u32 data_size;
};

struct imx415 {
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
	const struct imx415_mode *current_mode;

	struct sensor_gpio *gpio;

	struct v4l2_ctrl_handler ctrls;
	struct v4l2_ctrl *link_freq;
	struct v4l2_ctrl *pixel_rate;
	struct v4l2_ctrl *wdr;
	struct v4l2_ctrl *data_lanes;

	int status;
	struct mutex lock;
};

struct imx415_pixfmt {
	u32 code;
	u32 min_width;
	u32 max_width;
	u32 min_height;
	u32 max_height;
	u8 bpp;
};

static const struct imx415_pixfmt imx415_formats[] = {
	{ MEDIA_BUS_FMT_SGBRG10_1X10, 720, 3840, 720, 2160, 10 },
	{ MEDIA_BUS_FMT_SGBRG10_1X10, 1280, 3840, 720, 2160, 10 },
};

static const struct regmap_config imx415_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
};

static struct imx415_regval linear_4k_30fps_1440Mbps_4lane_10bits[] = {
	{0x3000, 0x01}, /* standby */

	{0x3002, 0x00}, /* XTMSTA */

	{0x3008, 0x54},
	{0x300A, 0x3B},
	{0x3024, 0xD0}, // VMAX 0x08d0
	{0x3025, 0x08},
	{0x3028, 0x2A},
	{0x3029, 0x04}, // HMAX 0x042A
	{0x3031, 0x00},
	{0x3032, 0x00},
	{0x3033, 0x08},
	{0x3050, 0x08},
	{0x30C1, 0x00},
	{0x3116, 0x23},
	{0x3118, 0xB4},
	{0x311A, 0xFC},
	{0x311E, 0x23},
	{0x32D4, 0x21},
	{0x32EC, 0xA1},
	{0x3452, 0x7F},
	{0x3453, 0x03},
	{0x358A, 0x04},
	{0x35A1, 0x02},
	{0x36BC, 0x0C},
	{0x36CC, 0x53},
	{0x36CD, 0x00},
	{0x36CE, 0x3C},
	{0x36D0, 0x8C},
	{0x36D1, 0x00},
	{0x36D2, 0x71},
	{0x36D4, 0x3C},
	{0x36D6, 0x53},
	{0x36D7, 0x00},
	{0x36D8, 0x71},
	{0x36DA, 0x8C},
	{0x36DB, 0x00},
	{0x3701, 0x00},
	{0x3724, 0x02},
	{0x3726, 0x02},
	{0x3732, 0x02},
	{0x3734, 0x03},
	{0x3736, 0x03},
	{0x3742, 0x03},
	{0x3862, 0xE0},
	{0x38CC, 0x30},
	{0x38CD, 0x2F},
	{0x395C, 0x0C},
	{0x3A42, 0xD1},
	{0x3A4C, 0x77},
	{0x3AE0, 0x02},
	{0x3AEC, 0x0C},
	{0x3B00, 0x2E},
	{0x3B06, 0x29},
	{0x3B98, 0x25},
	{0x3B99, 0x21},
	{0x3B9B, 0x13},
	{0x3B9C, 0x13},
	{0x3B9D, 0x13},
	{0x3B9E, 0x13},
	{0x3BA1, 0x00},
	{0x3BA2, 0x06},
	{0x3BA3, 0x0B},
	{0x3BA4, 0x10},
	{0x3BA5, 0x14},
	{0x3BA6, 0x18},
	{0x3BA7, 0x1A},
	{0x3BA8, 0x1A},
	{0x3BA9, 0x1A},
	{0x3BAC, 0xED},
	{0x3BAD, 0x01},
	{0x3BAE, 0xF6},
	{0x3BAF, 0x02},
	{0x3BB0, 0xA2},
	{0x3BB1, 0x03},
	{0x3BB2, 0xE0},
	{0x3BB3, 0x03},
	{0x3BB4, 0xE0},
	{0x3BB5, 0x03},
	{0x3BB6, 0xE0},
	{0x3BB7, 0x03},
	{0x3BB8, 0xE0},
	{0x3BBA, 0xE0},
	{0x3BBC, 0xDA},
	{0x3BBE, 0x88},
	{0x3BC0, 0x44},
	{0x3BC2, 0x7B},
	{0x3BC4, 0xA2},
	{0x3BC8, 0xBD},
	{0x3BCA, 0xBD},
	{0x4004, 0x00},
	{0x4005, 0x06},
	{0x4018, 0x9F},
	{0x401A, 0x57},
	{0x401C, 0x57},
	{0x401E, 0x87},
	{0x4020, 0x5F},
	{0x4022, 0xA7},
	{0x4024, 0x5F},
	{0x4026, 0x97},
	{0x4028, 0x4F},

    {0x344C, 0x2B}, //fix vertical stripe issue, start
    {0x344D, 0x01},
    {0x344E, 0xED},
    {0x344F, 0x01},
    {0x3450, 0xF6},
    {0x3451, 0x02},
    {0x3720, 0x00},
    {0x39A4, 0x07},
    {0x39A8, 0x32},
    {0x39AA, 0x32},
    {0x39AC, 0x32},
    {0x39AE, 0x32},
    {0x39B0, 0x32},
    {0x39B2, 0x2F},
    {0x39B4, 0x2D},
    {0x39B6, 0x28},
    {0x39B8, 0x30},
    {0x39BA, 0x30},
    {0x39BC, 0x30},
    {0x39BE, 0x30},
    {0x39C0, 0x30},
    {0x39C2, 0x2E},
    {0x39C4, 0x2B},
    {0x39C6, 0x25}, //end

	{0x3002, 0x00},
};

static struct imx415_regval dol_4k_30fps_1440Mbps_4lane_10bits[] = {
	{0x3000, 0x01}, /* standby */

	{0x3002, 0x00}, /* XTMSTA */

	{0x3008, 0x54},
	{0x300A, 0x3B},
	{0x3024, 0xD0}, // vmax 0x08d0
	{0x3028, 0x15}, // hmax 0x0215
	{0x302C, 0x01},
	{0x302D, 0x01},
	{0x3031, 0x00},
	{0x3032, 0x00},
	{0x3033, 0x08},
	{0x3050, 0x5C}, // 50 51 52 SHR0 for frame id 1; 0x105c
	{0x3051, 0x10},
	{0x3054, 0x09}, // 54 55 56 SHR1 for frame id 0; 0x9
	{0x3060, 0x11}, // RHS1 0x0111
	{0x3061, 0x01},
	{0x30C1, 0x00},
	{0x30CF, 0x01},
	{0x3116, 0x23},
	{0x3118, 0xB4},
	{0x311A, 0xFC},
	{0x311E, 0x23},
	{0x32D4, 0x21},
	{0x32EC, 0xA1},
	{0x3452, 0x7F},
	{0x3453, 0x03},
	{0x358A, 0x04},
	{0x35A1, 0x02},
	{0x36BC, 0x0C},
	{0x36CC, 0x53},
	{0x36CD, 0x00},
	{0x36CE, 0x3C},
	{0x36D0, 0x8C},
	{0x36D1, 0x00},
	{0x36D2, 0x71},
	{0x36D4, 0x3C},
	{0x36D6, 0x53},
	{0x36D7, 0x00},
	{0x36D8, 0x71},
	{0x36DA, 0x8C},
	{0x36DB, 0x00},
	{0x3701, 0x00},
	{0x3724, 0x02},
	{0x3726, 0x02},
	{0x3732, 0x02},
	{0x3734, 0x03},
	{0x3736, 0x03},
	{0x3742, 0x03},
	{0x3862, 0xE0},
	{0x38CC, 0x30},
	{0x38CD, 0x2F},
	{0x395C, 0x0C},
	{0x3A42, 0xD1},
	{0x3A4C, 0x77},
	{0x3AE0, 0x02},
	{0x3AEC, 0x0C},
	{0x3B00, 0x2E},
	{0x3B06, 0x29},
	{0x3B98, 0x25},
	{0x3B99, 0x21},
	{0x3B9B, 0x13},
	{0x3B9C, 0x13},
	{0x3B9D, 0x13},
	{0x3B9E, 0x13},
	{0x3BA1, 0x00},
	{0x3BA2, 0x06},
	{0x3BA3, 0x0B},
	{0x3BA4, 0x10},
	{0x3BA5, 0x14},
	{0x3BA6, 0x18},
	{0x3BA7, 0x1A},
	{0x3BA8, 0x1A},
	{0x3BA9, 0x1A},
	{0x3BAC, 0xED},
	{0x3BAD, 0x01},
	{0x3BAE, 0xF6},
	{0x3BAF, 0x02},
	{0x3BB0, 0xA2},
	{0x3BB1, 0x03},
	{0x3BB2, 0xE0},
	{0x3BB3, 0x03},
	{0x3BB4, 0xE0},
	{0x3BB5, 0x03},
	{0x3BB6, 0xE0},
	{0x3BB7, 0x03},
	{0x3BB8, 0xE0},
	{0x3BBA, 0xE0},
	{0x3BBC, 0xDA},
	{0x3BBE, 0x88},
	{0x3BC0, 0x44},
	{0x3BC2, 0x7B},
	{0x3BC4, 0xA2},
	{0x3BC8, 0xBD},
	{0x3BCA, 0xBD},
	{0x4004, 0x00},
	{0x4005, 0x06},
	{0x4018, 0x9F},
	{0x401A, 0x57},
	{0x401C, 0x57},
	{0x401E, 0x87},
	{0x4020, 0x5F},
	{0x4022, 0xA7},
	{0x4024, 0x5F},
	{0x4026, 0x97},
	{0x4028, 0x4F},

	{0x344C, 0x2B}, //fix vertical stripe issue, start
	{0x344D, 0x01},
	{0x344E, 0xED},
	{0x344F, 0x01},
	{0x3450, 0xF6},
	{0x3451, 0x02},
	{0x3720, 0x00},
	{0x39A4, 0x07},
	{0x39A8, 0x32},
	{0x39AA, 0x32},
	{0x39AC, 0x32},
	{0x39AE, 0x32},
	{0x39B0, 0x32},
	{0x39B2, 0x2F},
	{0x39B4, 0x2D},
	{0x39B6, 0x28},
	{0x39B8, 0x30},
	{0x39BA, 0x30},
	{0x39BC, 0x30},
	{0x39BE, 0x30},
	{0x39C0, 0x30},
	{0x39C2, 0x2E},
	{0x39C4, 0x2B},
	{0x39C6, 0x25}, //end

	{0x3002, 0x00},
};

extern int imx415_init(struct i2c_client *client, void *sdrv);
extern int imx415_deinit(struct i2c_client *client);
extern int imx415_sensor_id(struct i2c_client *client);
extern int imx415_power_on(struct device *dev, struct sensor_gpio *gpio);
extern int imx415_power_off(struct device *dev, struct sensor_gpio *gpio);
extern int imx415_power_suspend(struct device *dev);
extern int imx415_power_resume(struct device *dev);

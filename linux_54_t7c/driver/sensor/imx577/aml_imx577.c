// SPDX-License-Identifier: GPL-2.0-only
/*
 * Sony imx577 Camera Sensor Driver
 *
 * Copyright (C) 2021 Intel Corporation
 */

#define pr_fmt(fmt)  "[imx577]:%s:%d: " fmt, __func__, __LINE__

#include <asm/unaligned.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-subdev.h>

#include "../include/mclk_api.h"

static int setting_index = 0;
module_param(setting_index, int, 0644);
MODULE_PARM_DESC(setting_index, "Set setting index: 0 vendor setting;");

/* Streaming Mode */
#define IMX577_REG_MODE_SELECT   0x0100
#define IMX577_MODE_STANDBY      0x00
#define IMX577_MODE_STREAMING    0x01

/* Lines per frame */
#define IMX577_REG_LPFR          0x0340

/* Chip ID */
#define IMX577_REG_ID            0x0016
#define IMX577_ID                0x577

/* Exposure control */
#define IMX577_EXPOSURE          0x0202
#define IMX577_EXPOSURE_MIN      8
#define IMX577_EXPOSURE_OFFSET   22
#define IMX577_EXPOSURE_STEP     1
#define IMX577_EXPOSURE_DEFAULT  0x0c16

/* Analog gain control */
#define IMX577_AGAIN             0x0204
#define IMX577_AGAIN_MIN         0
#define IMX577_AGAIN_MAX         978
#define IMX577_AGAIN_STEP        1
#define IMX577_AGAIN_DEFAULT     0

/* Group hold register */
#define IMX577_REG_HOLD          0x0104

/* Input clock rate */
#define IMX577_INCLK_RATE        24000000

/* CSI2 HW configuration */
#define IMX577_LINK_FREQ         600000000
#define IMX577_NUM_DATA_LANES    4

#define IMX577_REG_MIN           0x00
#define IMX577_REG_MAX           0xffff

/**
 * struct imx577_regval - imx577 sensor register
 * @reg: Register address
 * @val: Register value
 */
struct imx577_regval {
	u16 reg;
	u8 val;
};

/**
 * struct imx577_mode - imx577 sensor mode structure
 * @width: Frame width
 * @height: Frame height
 * @code: Format code
 * @hblank: Horizontal blanking in lines
 * @vblank: Vertical blanking in lines
 * @vblank_min: Minimum vertical blanking in lines
 * @vblank_max: Maximum vertical blanking in lines
 * @pclk: Sensor pixel clock
 * @link_freq_index: Link frequency index
 * @reg_list: Register list for sensor mode
 */
struct imx577_mode {
	u32 width;
	u32 height;
	u32 code;

	u32 hblank;
	u32 vblank;
	u32 vblank_min;
	u32 vblank_max;

	u32 link_freq_index;
	u64 pclk;

	const struct imx577_regval *data;
	u32 data_size;
};


/**
 * struct imx577 - imx577 sensor device structure
 * @dev: Pointer to generic device
 * @client: Pointer to i2c client
 * @sd: V4L2 sub-device
 * @pad: Media pad. Only one pad supported
 * @reset_gpio: Sensor reset gpio
 * @inclk: Sensor input clock
 * @supplies: Regulator supplies
 * @ctrl_handler: V4L2 control handler
 * @link_freq_ctrl: Pointer to link frequency control
 * @pclk_ctrl: Pointer to pixel clock control
 * @hblank_ctrl: Pointer to horizontal blanking control
 * @vblank_ctrl: Pointer to vertical blanking control
 * @exp_ctrl: Pointer to exposure control
 * @again_ctrl: Pointer to analog gain control
 * @vblank: Vertical blanking in lines
 * @current_mode: Pointer to current selected sensor mode
 * @mutex: Mutex for serializing sensor controls
 * @streaming: Flag indicating streaming state
 */
struct imx577 {
	int index;
	struct device *dev;
	struct clk *xclk;
	struct regmap *regmap;

	u32 vblank;
	u32 enWDRMode;

	struct i2c_client *client;
	struct v4l2_subdev sd;
	struct v4l2_fwnode_endpoint ep;
	struct media_pad pad;
	struct v4l2_mbus_framefmt current_format;
	const struct imx577_mode *current_mode;

	int reset_gpio;
	int power_gpio;
	u32 xclk_freq;

	struct v4l2_ctrl_handler ctrl_handler;
	struct v4l2_ctrl *link_freq_ctrl;
	struct v4l2_ctrl *pclk_ctrl;

	struct v4l2_ctrl *hblank_ctrl;
	struct v4l2_ctrl *vblank_ctrl;

	struct v4l2_ctrl *wdr;

	struct v4l2_ctrl *exp_ctrl;
	struct v4l2_ctrl *again_ctrl;

	struct mutex mutex;

	bool streaming;
};

static const struct regmap_config imx577_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
};

static const s64 link_freq[] = {
	IMX577_LINK_FREQ
};

/* Sensor mode registers */
static const struct imx577_regval mode_4048x3040_raw10_30fps_from_vendor[] = {
	{ 0x0136, 0x18},
	{ 0x0137, 0x00},
	{ 0x3C7E, 0x01},
	{ 0x3C7F, 0x02},
	{ 0x38A8, 0x1F},
	{ 0x38A9, 0xFF},
	{ 0x38AA, 0x1F},
	{ 0x38AB, 0xFF},
	{ 0x55D4, 0x00},
	{ 0x55D5, 0x00},
	{ 0x55D6, 0x07},
	{ 0x55D7, 0xFF},
	{ 0x55E8, 0x07},
	{ 0x55E9, 0xFF},
	{ 0x55EA, 0x00},
	{ 0x55EB, 0x00},
	{ 0x575C, 0x07},
	{ 0x575D, 0xFF},
	{ 0x575E, 0x00},
	{ 0x575F, 0x00},
	{ 0x5764, 0x00},
	{ 0x5765, 0x00},
	{ 0x5766, 0x07},
	{ 0x5767, 0xFF},
	{ 0x5974, 0x04},
	{ 0x5975, 0x01},
	{ 0x5F10, 0x09},
	{ 0x5F11, 0x92},
	{ 0x5F12, 0x32},
	{ 0x5F13, 0x72},
	{ 0x5F14, 0x16},
	{ 0x5F15, 0xBA},
	{ 0x5F17, 0x13},
	{ 0x5F18, 0x24},
	{ 0x5F19, 0x60},
	{ 0x5F1A, 0xE3},
	{ 0x5F1B, 0xAD},
	{ 0x5F1C, 0x74},
	{ 0x5F2D, 0x25},
	{ 0x5F5C, 0xD0},
	{ 0x6A22, 0x00},
	{ 0x6A23, 0x1D},
	{ 0x7BA8, 0x00},
	{ 0x7BA9, 0x00},
	{ 0x886B, 0x00},
	{ 0x9002, 0x0A},
	{ 0x9004, 0x1A},
	{ 0x9214, 0x93},
	{ 0x9215, 0x69},
	{ 0x9216, 0x93},
	{ 0x9217, 0x6B},
	{ 0x9218, 0x93},
	{ 0x9219, 0x6D},
	{ 0x921A, 0x57},
	{ 0x921B, 0x58},
	{ 0x921C, 0x57},
	{ 0x921D, 0x59},
	{ 0x921E, 0x57},
	{ 0x921F, 0x5A},
	{ 0x9220, 0x57},
	{ 0x9221, 0x5B},
	{ 0x9222, 0x93},
	{ 0x9223, 0x02},
	{ 0x9224, 0x93},
	{ 0x9225, 0x03},
	{ 0x9226, 0x93},
	{ 0x9227, 0x04},
	{ 0x9228, 0x93},
	{ 0x9229, 0x05},
	{ 0x922A, 0x98},
	{ 0x922B, 0x21},
	{ 0x922C, 0xB2},
	{ 0x922D, 0xDB},
	{ 0x922E, 0xB2},
	{ 0x922F, 0xDC},
	{ 0x9230, 0xB2},
	{ 0x9231, 0xDD},
	{ 0x9232, 0xB2},
	{ 0x9233, 0xE1},
	{ 0x9234, 0xB2},
	{ 0x9235, 0xE2},
	{ 0x9236, 0xB2},
	{ 0x9237, 0xE3},
	{ 0x9238, 0xB7},
	{ 0x9239, 0xB9},
	{ 0x923A, 0xB7},
	{ 0x923B, 0xBB},
	{ 0x923C, 0xB7},
	{ 0x923D, 0xBC},
	{ 0x923E, 0xB7},
	{ 0x923F, 0xC5},
	{ 0x9240, 0xB7},
	{ 0x9241, 0xC7},
	{ 0x9242, 0xB7},
	{ 0x9243, 0xC9},
	{ 0x9244, 0x98},
	{ 0x9245, 0x56},
	{ 0x9246, 0x98},
	{ 0x9247, 0x55},
	{ 0x9380, 0x00},
	{ 0x9381, 0x62},
	{ 0x9382, 0x00},
	{ 0x9383, 0x56},
	{ 0x9384, 0x00},
	{ 0x9385, 0x52},
	{ 0x9388, 0x00},
	{ 0x9389, 0x55},
	{ 0x938A, 0x00},
	{ 0x938B, 0x55},
	{ 0x938C, 0x00},
	{ 0x938D, 0x41},
	{ 0x5078, 0x01},
	{ 0x0112, 0x0A},
	{ 0x0113, 0x0A},
	{ 0x0114, 0x03},
	{ 0x0342, 0x23},
	{ 0x0343, 0x18},
	{ 0x0340, 0x0C},
	{ 0x0341, 0x2C},
	{ 0x3210, 0x00},
	{ 0x0344, 0x00},
	{ 0x0345, 0x00},
	{ 0x0346, 0x00},
	{ 0x0347, 0x00},
	{ 0x0348, 0x0F},
	{ 0x0349, 0xD7},
	{ 0x034A, 0x0B},
	{ 0x034B, 0xDF},
	{ 0x00E3, 0x00},
	{ 0x00E4, 0x00},
	{ 0x00E5, 0x00},
	{ 0x00FC, 0x0A},
	{ 0x00FD, 0x0A},
	{ 0x00FE, 0x0A},
	{ 0x00FF, 0x0A},
	{ 0xE013, 0x00},
	{ 0x0220, 0x00},
	{ 0x0221, 0x11},
	{ 0x0381, 0x01},
	{ 0x0383, 0x01},
	{ 0x0385, 0x01},
	{ 0x0387, 0x01},
	{ 0x0900, 0x00},
	{ 0x0901, 0x11},
	{ 0x0902, 0x00},
	{ 0x3140, 0x02},
	{ 0x3241, 0x11},
	{ 0x3250, 0x03},
	{ 0x3E10, 0x00},
	{ 0x3E11, 0x00},
	{ 0x3F0D, 0x00},
	{ 0x3F42, 0x00},
	{ 0x3F43, 0x00},
	{ 0x0401, 0x00},
	{ 0x0404, 0x00},
	{ 0x0405, 0x10},
	{ 0x0408, 0x00},
	{ 0x0409, 0x00},
	{ 0x040A, 0x00},
	{ 0x040B, 0x00},
	{ 0x040C, 0x0F},
	{ 0x040D, 0xD0},
	{ 0x040E, 0x0B},
	{ 0x040F, 0xE0},
	{ 0x034C, 0x0F},
	{ 0x034D, 0xD0},
	{ 0x034E, 0x0B},
	{ 0x034F, 0xE0},
	{ 0x0301, 0x05},
	{ 0x0303, 0x02},
	{ 0x0305, 0x04},
	{ 0x0306, 0x01},
	{ 0x0307, 0x5E},
	{ 0x0309, 0x0A},
	{ 0x030B, 0x02},
	{ 0x030D, 0x02},
	{ 0x030E, 0x00},
	{ 0x030F, 0xA6},
	{ 0x0310, 0x01},
	{ 0x0820, 0x0F},
	{ 0x0821, 0x90},
	{ 0x0822, 0x00},
	{ 0x0823, 0x00},
	{ 0x3E20, 0x01},
	{ 0x3E37, 0x00},
	{ 0x3F50, 0x00},
	{ 0x3F56, 0x00},
	{ 0x3F57, 0x82},
	{ 0x3C0A, 0x5A},
	{ 0x3C0B, 0x55},
	{ 0x3C0C, 0x28},
	{ 0x3C0D, 0x07},
	{ 0x3C0E, 0xFF},
	{ 0x3C0F, 0x00},
	{ 0x3C10, 0x00},
	{ 0x3C11, 0x02},
	{ 0x3C12, 0x00},
	{ 0x3C13, 0x03},
	{ 0x3C14, 0x00},
	{ 0x3C15, 0x00},
	{ 0x3C16, 0x0C},
	{ 0x3C17, 0x0C},
	{ 0x3C18, 0x0C},
	{ 0x3C19, 0x0A},
	{ 0x3C1A, 0x0A},
	{ 0x3C1B, 0x0A},
	{ 0x3C1C, 0x00},
	{ 0x3C1D, 0x00},
	{ 0x3C1E, 0x00},
	{ 0x3C1F, 0x00},
	{ 0x3C20, 0x00},
	{ 0x3C21, 0x00},
	{ 0x3C22, 0x3F},
	{ 0x3C23, 0x0A},
	{ 0x3E35, 0x01},
	{ 0x3F4A, 0x03},
	{ 0x3F4B, 0xBF},
	{ 0x3F26, 0x00},
	{ 0x0202, 0x0C},
	{ 0x0203, 0x16},
	{ 0x0204, 0x00},
	{ 0x0205, 0x00},
	{ 0x020E, 0x01},
	{ 0x020F, 0x00},
	{ 0x0210, 0x01},
	{ 0x0211, 0x00},
	{ 0x0212, 0x01},
	{ 0x0213, 0x00},
	{ 0x0214, 0x01},
	{ 0x0215, 0x00}
};

/* Supported sensor mode configurations */
static const struct imx577_mode supported_mode[] =
{
	{
		.width = 4048,
		.height = 3040,
		.code = MEDIA_BUS_FMT_SRGGB10_1X10,

		.hblank = 456,
		.vblank = 76,
		.vblank_min = 0,
		.vblank_max = 32420,

		.link_freq_index = 0,
		.pclk = 480000000,

		.data = mode_4048x3040_raw10_30fps_from_vendor,
		.data_size = ARRAY_SIZE(mode_4048x3040_raw10_30fps_from_vendor),
	}
};

static int imx577_power_on(struct imx577 *imx577);
static int imx577_power_off(struct imx577 *imx577);


/**
 * to_imx577() - imx577 V4L2 sub-device to imx577 device.
 * @subdev: pointer to imx577 V4L2 sub-device
 *
 * Return: pointer to imx577 device
 */
static inline struct imx577 *to_imx577(struct v4l2_subdev *subdev)
{
	return container_of(subdev, struct imx577, sd);
}

static inline int imx577_read_reg(struct imx577 *imx577, u16 addr, u8 *value)
{
	unsigned int regval;

	int i, ret;

	for ( i = 0; i < 3; ++i ) {
		ret = regmap_read(imx577->regmap, addr, &regval);
		if ( 0 == ret ) {
			break;
		}
	}

	if (ret)
		dev_err(imx577->dev, "I2C read with i2c transfer failed for addr: %x, ret %d\n", addr, ret);

	*value = regval & 0xff;
	return 0;
}

static int imx577_write_reg(struct imx577 *imx577, u16 addr, u8 value)
{
	int i, ret;

	for (i = 0; i < 3; i++) {
		ret = regmap_write(imx577->regmap, addr, value);
		if (0 == ret) {
			break;
		}
	}

	if (ret)
		dev_err(imx577->dev, "I2C write failed for addr: %x, ret %d\n", addr, ret);

	return ret;
}

static int imx577_set_register_array(struct imx577 *imx577,
				     const struct imx577_regval *settings,
				     unsigned int num_settings)
{
	unsigned int i;
	int ret = 0;

	for (i = 0; i < num_settings; ++i, ++settings) {
		ret = imx577_write_reg(imx577, settings->reg, settings->val);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int imx577_write_reg_le_first(struct imx577 *imx577, u16 address_low,
				     u8 nr_regs, u32 value)
{
	unsigned int i;
	int ret;
	if (1 == nr_regs) {
		return imx577_write_reg(imx577, address_low, (u8)(value & 0xff) );
	}

	// little endian to low addr
	for (i = 0; i < nr_regs; i++) {
		ret = imx577_write_reg(imx577, address_low + i,
				       (u8)(value >> (i * 8)));
		if (ret) {
			pr_err("Error writing buffered registers\n");
			return ret;
		}
	}

	return ret;
}

static int imx577_write_reg_be_first(struct imx577 *imx577, u16 address_low,
				     u8 nr_regs, u32 value)
{
	unsigned int i;
	int ret;
	if (1 == nr_regs) {
		return imx577_write_reg(imx577, address_low, (u8)(value & 0xff) );
	}
	// big endian to low addr.
	for (i = 0; i < nr_regs; i++) {
		u8 reg_val = ((value >> ((nr_regs - 1 - i) * 8)) & 0xff) ;
		ret = imx577_write_reg(imx577, address_low + i,
				       reg_val);
		if (ret) {
			pr_err("Error writing buffered registers\n");
			return ret;
		}
	}

	return ret;
}

/**
 * imx577_get_id() - Detect imx577 sensor
 * @imx577: pointer to imx577 device
 *
 * Return: 0 if successful, -EIO if sensor id does not match
 */

static int imx577_get_id(struct imx577 *imx577)
{
	int rtn = -EINVAL;
	uint32_t sensor_id = 0;
	u8 val = 0;

	imx577_read_reg(imx577, IMX577_REG_ID, &val);
	sensor_id |= (val << 8);
    imx577_read_reg(imx577, IMX577_REG_ID + 1, &val);
	sensor_id |= val;

	if (sensor_id != IMX577_ID) {
		dev_err(imx577->dev, "Failed to get imx577 id: 0x%x\n", sensor_id);
		return rtn;
	} else {
		dev_err(imx577->dev, "success get imx577 id 0x%x", sensor_id);
	}

	return 0;
}

/**
 * imx577_update_controls() - Update control ranges based on streaming mode
 * @imx577: pointer to imx577 device
 * @mode: pointer to imx577_mode sensor mode
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_update_controls(struct imx577 *imx577,
				  const struct imx577_mode *mode)
{
	int ret;

	ret = __v4l2_ctrl_s_ctrl(imx577->link_freq_ctrl, mode->link_freq_index);
	if (ret)
		return ret;

	ret = __v4l2_ctrl_s_ctrl(imx577->hblank_ctrl, mode->hblank);
	if (ret)
		return ret;

	return __v4l2_ctrl_modify_range(imx577->vblank_ctrl, mode->vblank_min,
					mode->vblank_max, 1, mode->vblank);
}

/**
 * imx577_update_exp_gain() - Set updated exposure and gain
 * @imx577: pointer to imx577 device
 * @exposure: updated exposure value
 * @gain: updated analog gain value
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_update_exp_gain(struct imx577 *imx577, u32 exposure, u32 gain)
{
	u32 lpfr;
	int ret;

	lpfr = imx577->vblank + imx577->current_mode->height;

	//pr_info( "Set exp (reg) 0x%x, again(reg) 0x%x , lpfr(reg) 0x%x",
	//	exposure, gain, lpfr);

	ret = imx577_write_reg_be_first(imx577, IMX577_REG_HOLD, 1, 1);
	if (ret) {
		pr_err("error leave");
		return ret;
	}

	ret = imx577_write_reg_be_first(imx577, IMX577_REG_LPFR, 2, lpfr);
	if (ret) {
		pr_err("error leave");
		goto error_release_group_hold;
	}

	ret = imx577_write_reg_be_first(imx577, IMX577_EXPOSURE, 2, exposure);
	if (ret) {
		pr_err("error leave");
		goto error_release_group_hold;
	}

	ret = imx577_write_reg_be_first(imx577, IMX577_AGAIN, 2, gain);

error_release_group_hold:
	imx577_write_reg_be_first(imx577, IMX577_REG_HOLD, 1, 0);

	return ret;
}


static int imx577_set_gain(struct imx577 *imx577, u32 value)
{
	int ret;
	//pr_info( "new analog gain 0x%x", value);

	ret = imx577_update_exp_gain(imx577, imx577->exp_ctrl->val, value);
	if (ret)
		dev_err(imx577->dev, "Unable to write gain\n");

	return ret;
}

static int imx577_set_exposure(struct imx577 *imx577, u32 value)
{
	int ret;
	//pr_info( "new exp 0x%x", value);

	ret = imx577_update_exp_gain(imx577, value, imx577->again_ctrl->val);
	if (ret)
		dev_err(imx577->dev, "Unable to write gain\n");

	return ret;
}

/**
 * imx577_set_ctrl() - Set subdevice control
 * @ctrl: pointer to v4l2_ctrl structure
 *
 * Supported controls:
 * - V4L2_CID_VBLANK
 *   - V4L2_CID_GAIN
 *   - V4L2_CID_EXPOSURE
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_set_ctrl(struct v4l2_ctrl *ctrl)
{
	struct imx577 *imx577 =
		container_of(ctrl->handler, struct imx577, ctrl_handler);

	int ret = 0;

	/* V4L2 controls values will be applied only when power is already up */
	if (!pm_runtime_get_if_in_use(imx577->dev))
		return 0;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		ret = imx577_set_gain(imx577, ctrl->val);
		break;
	case V4L2_CID_EXPOSURE:
		ret = imx577_set_exposure(imx577, ctrl->val);
		break;
	case V4L2_CID_HBLANK:
		break;
	case V4L2_CID_VBLANK:
		imx577->vblank = imx577->vblank_ctrl->val;

		pr_info( "Received vblank %u, new lpfr %u",
			imx577->vblank,
			imx577->vblank + imx577->current_mode->height);

		ret = __v4l2_ctrl_modify_range(imx577->exp_ctrl,
					       IMX577_EXPOSURE_MIN,
					       imx577->vblank + imx577->current_mode->height - IMX577_EXPOSURE_OFFSET,
					       1, IMX577_EXPOSURE_DEFAULT);
		break;

	case V4L2_CID_AML_MODE:
		imx577->enWDRMode = ctrl->val;
		break;
	default:
		dev_err(imx577->dev, "Error ctrl->id %u, flag 0x%lx\n",
			ctrl->id, ctrl->flags);
		ret = -EINVAL;
		break;
	}

	pm_runtime_put(imx577->dev);

	return ret;

}

/* V4l2 subdevice control ops*/
static const struct v4l2_ctrl_ops imx577_ctrl_ops = {
	.s_ctrl = imx577_set_ctrl,
};

/**
 * imx577_enum_mbus_code() - Enumerate V4L2 sub-device mbus codes
 * @sd: pointer to imx577 V4L2 sub-device structure
 * @sd_state: V4L2 sub-device configuration
 * @code: V4L2 sub-device code enumeration need to be filled
 *
 * Return: 0 if successful, error code otherwise.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static int imx577_enum_mbus_code(struct v4l2_subdev *sd,
				  struct v4l2_subdev_state *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
#else
static int imx577_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
#endif
{
	if (code->index > 0)
		return -EINVAL;

	code->code = supported_mode[setting_index].code;

	return 0;
}

/**
 * imx577_enum_frame_size() - Enumerate V4L2 sub-device frame sizes
 * @sd: pointer to imx577 V4L2 sub-device structure
 * @sd_state: V4L2 sub-device configuration
 * @fsize: V4L2 sub-device size enumeration need to be filled
 *
 * Return: 0 if successful, error code otherwise.
 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static int imx577_enum_frame_size(struct v4l2_subdev *sd,
			        struct v4l2_subdev_state *cfg,
			       struct v4l2_subdev_frame_size_enum *fsize)
#else
static int imx577_enum_frame_size(struct v4l2_subdev *sd,
			       struct v4l2_subdev_pad_config *cfg,
			       struct v4l2_subdev_frame_size_enum *fsize)
#endif
{
	if (fsize->index > 0)
		return -EINVAL;

	if (fsize->code != supported_mode[setting_index].code)
		return -EINVAL;

	fsize->min_width = supported_mode[setting_index].width;
	fsize->max_width = fsize->min_width;
	fsize->min_height = supported_mode[setting_index].height;
	fsize->max_height = fsize->min_height;

	return 0;
}

/**
 * imx577_fill_pad_format() - Fill subdevice pad format
 *                            from selected sensor mode
 * @imx577: pointer to imx577 device
 * @mode: pointer to imx577_mode sensor mode
 * @fmt: V4L2 sub-device format need to be filled
 */
static void imx577_fill_pad_format(struct imx577 *imx577,
				   const struct imx577_mode *mode,
				   struct v4l2_subdev_format *fmt)
{
	fmt->format.width = mode->width;
	fmt->format.height = mode->height;
	fmt->format.code = mode->code;
	fmt->format.field = V4L2_FIELD_NONE;
	fmt->format.colorspace = V4L2_COLORSPACE_RAW;
	fmt->format.ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	fmt->format.quantization = V4L2_QUANTIZATION_DEFAULT;
	fmt->format.xfer_func = V4L2_XFER_FUNC_NONE;
}

/**
 * imx577_get_pad_format() - Get subdevice pad format
 * @sd: pointer to imx577 V4L2 sub-device structure
 * @sd_state: V4L2 sub-device configuration
 * @fmt: V4L2 sub-device format need to be set
 *
 * Return: 0 if successful, error code otherwise.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static int imx577_get_pad_format(struct v4l2_subdev *sd,
				 struct v4l2_subdev_state *cfg,
				 struct v4l2_subdev_format *fmt)
#else
static int imx577_get_pad_format(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)

#endif

{
	struct imx577 *imx577 = to_imx577(sd);

	mutex_lock(&imx577->mutex);

	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY) {
		struct v4l2_mbus_framefmt *framefmt;

		framefmt = v4l2_subdev_get_try_format(sd, cfg, fmt->pad);
		fmt->format = *framefmt;
	} else {
		imx577_fill_pad_format(imx577, imx577->current_mode, fmt);
	}

	mutex_unlock(&imx577->mutex);

	return 0;
}

/**
 * imx577_set_pad_format() - Set subdevice pad format
 * @sd: pointer to imx577 V4L2 sub-device structure
 * @sd_state: V4L2 sub-device configuration
 * @fmt: V4L2 sub-device format need to be set
 *
 * Return: 0 if successful, error code otherwise.
 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static int imx577_set_pad_format(struct v4l2_subdev *sd,
				 struct v4l2_subdev_state *cfg,
				 struct v4l2_subdev_format *fmt)
#else
static int imx577_set_pad_format(struct v4l2_subdev *sd,
			struct v4l2_subdev_pad_config *cfg,
			struct v4l2_subdev_format *fmt)
#endif

{
	struct imx577 *imx577 = to_imx577(sd);
	const struct imx577_mode *mode;
	int ret = 0;

	imx577_power_on(imx577);

	mutex_lock(&imx577->mutex);

	mode = &supported_mode[setting_index];
	imx577_fill_pad_format(imx577, mode, fmt);

	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY) {
		struct v4l2_mbus_framefmt *framefmt;

		framefmt = v4l2_subdev_get_try_format(sd, cfg, fmt->pad);
		*framefmt = fmt->format;
	} else {
		imx577->current_mode = mode;
		ret = imx577_update_controls(imx577, mode);
	}

	/* Set init register settings */
	ret = imx577_set_register_array(imx577, imx577->current_mode->data,
		imx577->current_mode->data_size );
	if (ret < 0) {
		pr_err( "Could not set initial setting\n");
	} else {
		pr_info("mode changed. setting ok \n");
	}

	mutex_unlock(&imx577->mutex);

	return ret;
}

/**
 * imx577_init_pad_cfg() - Initialize sub-device pad configuration
 * @sd: pointer to imx577 V4L2 sub-device structure
 * @sd_state: V4L2 sub-device configuration
 *
 * Return: 0 if successful, error code otherwise.
 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static int imx577_init_pad_cfg(struct v4l2_subdev *subdev,
				  struct v4l2_subdev_state *cfg)
#else
static int imx577_init_pad_cfg(struct v4l2_subdev *subdev,
				  struct v4l2_subdev_pad_config *cfg)
#endif

{
	struct imx577 *imx577 = to_imx577(subdev);
	struct v4l2_subdev_format fmt = { 0 };

	fmt.which = cfg ? V4L2_SUBDEV_FORMAT_TRY : V4L2_SUBDEV_FORMAT_ACTIVE;
	imx577_fill_pad_format(imx577, &supported_mode[setting_index], &fmt);

	return imx577_set_pad_format(subdev, cfg, &fmt);
}

/**
 * imx577_start_streaming() - Start sensor stream
 * @imx577: pointer to imx577 device
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_start_streaming(struct imx577 *imx577)
{
	int ret;

	/* Setup handler will write actual exposure and gain */
	ret =  __v4l2_ctrl_handler_setup(imx577->sd.ctrl_handler);
	if (ret) {
		dev_err(imx577->dev, "fail to setup handler");
		return ret;
	}

	/* Delay is required before streaming*/
	usleep_range(7400, 8000);

	/* Start streaming */
	ret = imx577_write_reg_be_first(imx577, IMX577_REG_MODE_SELECT,
			       1, IMX577_MODE_STREAMING);
	if (ret) {
		dev_err(imx577->dev, "fail to start streaming");
		return ret;
	}

	return 0;
}

/**
 * imx577_stop_streaming() - Stop sensor stream
 * @imx577: pointer to imx577 device
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_stop_streaming(struct imx577 *imx577)
{
	return imx577_write_reg_be_first(imx577, IMX577_REG_MODE_SELECT,
				1, IMX577_MODE_STANDBY);
}

/**
 * imx577_set_stream() - Enable sensor streaming
 * @sd: pointer to imx577 subdevice
 * @enable: set to enable sensor streaming
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_set_stream(struct v4l2_subdev *sd, int enable)
{
	struct imx577 *imx577 = to_imx577(sd);
	int ret;

	mutex_lock(&imx577->mutex);

	if (imx577->streaming == enable) {
		mutex_unlock(&imx577->mutex);
		return 0;
	}

	if (enable) {
		ret = imx577_start_streaming(imx577);
		if (ret)
			goto error_unlock;
	} else {
		imx577_stop_streaming(imx577);
	}

	imx577->streaming = enable;

	mutex_unlock(&imx577->mutex);

	return 0;

error_unlock:
	mutex_unlock(&imx577->mutex);

	return ret;
}

static int aml_gpio_set(struct device *dev, const char* propname, const char * requestname, int val)
{
	int gpio = of_get_named_gpio(dev->of_node, propname, 0);

	if (gpio >= 0) {
		devm_gpio_request(dev, gpio, requestname);
		if (gpio_is_valid(gpio)) {
			gpio_direction_output(gpio, val);
			pr_info("gpio %s init\n", propname);
		} else {
			pr_err("gpio %s is not valid\n", propname);
		}
	} else {
		pr_err("get_named_gpio %s fail\n", propname);
	}

	return gpio;
}


/**
 * imx577_parse_hw_config() - Parse HW configuration and check if supported
 * @imx577: pointer to imx577 device
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_parse_hw_config(struct imx577 *imx577)
{
	struct fwnode_handle *fwnode = dev_fwnode(imx577->dev);
	struct v4l2_fwnode_endpoint bus_cfg = {
		.bus_type = V4L2_MBUS_CSI2_DPHY
	};
	struct fwnode_handle *node;

	unsigned int i;
	int ret;

	if (!fwnode) {
		dev_err(imx577->dev, "Could not get fwnode\n");
		return -ENXIO;
	}

	/* Request optional reset pin */
	imx577->reset_gpio = aml_gpio_set(imx577->dev, "reset", "RESET", 0);
	imx577->power_gpio = aml_gpio_set(imx577->dev, "power", "POWER", 0);

	ret = of_property_read_u32(imx577->dev->of_node, "mclk-frequency",
				       &imx577->xclk_freq);
	if (ret) {
		dev_err(imx577->dev, "Could not get mclk-frequency\n");
		return -EINVAL;
	} else {
		dev_info(imx577->dev, "get dts clk frequency %d \n", imx577->xclk_freq);
	}

	node = fwnode_graph_get_next_endpoint(fwnode, NULL);
	if (!node) {
		dev_err(imx577->dev, "no ports node found");
		return -ENXIO;
	}

	ret = v4l2_fwnode_endpoint_alloc_parse(node, &bus_cfg);
	fwnode_handle_put(node);
	if (ret) {
		dev_err(imx577->dev, "parse info from endpoint fails");
		return ret;
	}

	if (bus_cfg.bus.mipi_csi2.num_data_lanes != IMX577_NUM_DATA_LANES) {
		dev_err(imx577->dev,
			"number of CSI2 data lanes %d is not supported",
			bus_cfg.bus.mipi_csi2.num_data_lanes);
		ret = -EINVAL;
		goto done_endpoint_free;
	}

	if (!bus_cfg.nr_of_link_frequencies) {
		dev_err(imx577->dev, "no link frequencies defined");
		ret = -EINVAL;
		goto done_endpoint_free;
	}

	for (i = 0; i < bus_cfg.nr_of_link_frequencies; i++) {
		//if (bus_cfg.link_frequencies[i] == IMX577_LINK_FREQ)
		{
			dev_info(imx577->dev, "got link frequencies %d", bus_cfg.link_frequencies[0]);
			goto done_endpoint_free;
		}
	}

	ret = -EINVAL;

done_endpoint_free:
	v4l2_fwnode_endpoint_free(&bus_cfg);

	return ret;
}


/**
 * imx577_power_on() - Sensor power on sequence
 * @dev: pointer to i2c device
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_power_on(struct imx577 *imx577)
{
	int ret;
	struct device *dev = imx577->dev;
	if (gpio_is_valid(imx577->power_gpio)) {
		gpio_direction_output(imx577->power_gpio, 1);
	} else {
		dev_err(dev, "invalid power gpio");
	}

	usleep_range(30000, 31000);

	ret = mclk_enable(imx577->dev, imx577->xclk_freq);
	if (ret < 0 ) {
		dev_err(imx577->dev, "set mclk fail\n");
	}

	if (gpio_is_valid(imx577->reset_gpio)) {
		gpio_direction_output(imx577->reset_gpio, 1);
	} else {
		dev_err(dev, "invalid reset gpio");
	}
	usleep_range(30000, 31000);
	return ret;

}

/**
 * imx577_power_off() - Sensor power off sequence
 * @dev: pointer to i2c device
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_power_off(struct imx577 *imx577)
{
	struct device *dev = imx577->dev;
	if (gpio_is_valid(imx577->reset_gpio)) {
		gpio_direction_output(imx577->reset_gpio, 0);
	} else {
		dev_err(dev, "invalid reset gpio");
	}

	mclk_disable(imx577->dev);

	if (gpio_is_valid(imx577->power_gpio)) {
		gpio_direction_output(imx577->power_gpio, 0);
	} else {
		dev_err(dev, "invalid power gpio");
	}

	return 0;
}


static int imx577_power_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct imx577 *imx577 = to_imx577(sd);

	reset_am_enable(imx577->dev,"reset", 0);

	return 0;
}

static int imx577_power_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct imx577 *imx577 = to_imx577(sd);

	reset_am_enable(imx577->dev,"reset", 1);

	return 0;
}

static int imx577_log_status(struct v4l2_subdev *sd)
{
	struct imx577 *imx577 = to_imx577(sd);

	dev_info(imx577->dev, "log status done\n");

	return 0;
}

int imx577_sbdev_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh) {
	struct imx577 *imx577 = to_imx577(sd);
	pr_info("in");
	imx577_power_on(imx577);
	return 0;
}

int imx577_sbdev_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh) {
	struct imx577 *imx577 = to_imx577(sd);
	pr_info("in");

	imx577_stop_streaming(imx577);
	imx577_power_off(imx577);
	return 0;
}

static const struct dev_pm_ops imx577_pm_ops = {
	SET_RUNTIME_PM_OPS(imx577_power_suspend, imx577_power_resume, NULL)
};

const struct v4l2_subdev_core_ops imx577_core_ops = {
	.log_status = imx577_log_status,
};

/* V4l2 subdevice ops */
static const struct v4l2_subdev_video_ops imx577_video_ops = {
	.s_stream = imx577_set_stream,
};

static const struct v4l2_subdev_pad_ops imx577_pad_ops = {
	.init_cfg = imx577_init_pad_cfg,
	.enum_mbus_code = imx577_enum_mbus_code,
	.enum_frame_size = imx577_enum_frame_size,
	.get_fmt = imx577_get_pad_format,
	.set_fmt = imx577_set_pad_format,
};

static struct v4l2_subdev_internal_ops imx577_internal_ops = {
	.open = imx577_sbdev_open,
	.close = imx577_sbdev_close,
};

static const struct v4l2_subdev_ops imx577_subdev_ops = {
	.core = &imx577_core_ops,
	.video = &imx577_video_ops,
	.pad = &imx577_pad_ops,
};

static const struct media_entity_operations imx577_subdev_entity_ops = {
	.link_validate = v4l2_subdev_link_validate,
};

static struct v4l2_ctrl_config wdr_cfg = {
	.ops = &imx577_ctrl_ops,
	.id = V4L2_CID_AML_MODE,
	.name = "wdr mode",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.flags = V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
	.min = 0,
	.max = 2,
	.step = 1,
	.def = 0,
};

/**
 * imx577_init_controls() - Initialize sensor subdevice controls
 * @imx577: pointer to imx577 device
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_init_controls(struct imx577 *imx577)
{
	struct v4l2_ctrl_handler *ctrl_hdlr = &imx577->ctrl_handler;
	const struct imx577_mode *mode = imx577->current_mode;
	u32 lpfr;
	int ret;

	ret = v4l2_ctrl_handler_init(ctrl_hdlr, 8);
	if (ret)
		return ret;

	/* Serialize controls with sensor device */
	ctrl_hdlr->lock = &imx577->mutex;

	/* Initialize exposure and gain */
	imx577->again_ctrl = v4l2_ctrl_new_std(ctrl_hdlr,
					       &imx577_ctrl_ops,
					       V4L2_CID_GAIN,
					       IMX577_AGAIN_MIN,
					       IMX577_AGAIN_MAX,
					       IMX577_AGAIN_STEP,
					       IMX577_AGAIN_DEFAULT);

	lpfr = mode->vblank + mode->height;
	imx577->exp_ctrl = v4l2_ctrl_new_std(ctrl_hdlr,
					     &imx577_ctrl_ops,
					     V4L2_CID_EXPOSURE,
					     IMX577_EXPOSURE_MIN,
					     lpfr - IMX577_EXPOSURE_OFFSET,
					     IMX577_EXPOSURE_STEP,
					     IMX577_EXPOSURE_DEFAULT);

	imx577->link_freq_ctrl = v4l2_ctrl_new_int_menu(ctrl_hdlr,
							&imx577_ctrl_ops,
							V4L2_CID_LINK_FREQ,
							ARRAY_SIZE(link_freq) -
							1,
							mode->link_freq_index,
							link_freq);
	if (imx577->link_freq_ctrl)
		imx577->link_freq_ctrl->flags |= V4L2_CTRL_FLAG_READ_ONLY;

	/* Read only controls */
	imx577->pclk_ctrl = v4l2_ctrl_new_std(ctrl_hdlr,
					      &imx577_ctrl_ops,
					      V4L2_CID_PIXEL_RATE,
					      mode->pclk, mode->pclk,
					      1, mode->pclk);

	imx577->wdr = v4l2_ctrl_new_custom(ctrl_hdlr, &wdr_cfg, NULL);

	imx577->vblank_ctrl = v4l2_ctrl_new_std(ctrl_hdlr,
						&imx577_ctrl_ops,
						V4L2_CID_VBLANK,
						mode->vblank_min,
						mode->vblank_max,
						1, mode->vblank);

	imx577->hblank_ctrl = v4l2_ctrl_new_std(ctrl_hdlr,
						&imx577_ctrl_ops,
						V4L2_CID_HBLANK,
						IMX577_REG_MIN,
						IMX577_REG_MAX,
						1, mode->hblank);
	if (imx577->hblank_ctrl)
		imx577->hblank_ctrl->flags |= V4L2_CTRL_FLAG_READ_ONLY;

	if (ctrl_hdlr->error) {
		dev_err(imx577->dev, "control init failed: %d",
			ctrl_hdlr->error);
		v4l2_ctrl_handler_free(ctrl_hdlr);
		return ctrl_hdlr->error;
	}

	imx577->sd.ctrl_handler = ctrl_hdlr;

	return 0;
}

/**
 * imx577_probe() - I2C client device binding
 * @client: pointer to i2c client device
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_probe(struct i2c_client *client)
{
	struct imx577 *imx577;
	int ret;

	imx577 = devm_kzalloc(&client->dev, sizeof(*imx577), GFP_KERNEL);
	if (!imx577) {
		dev_err(imx577->dev, "alloc mem fails");
		return -ENOMEM;
	}

	imx577->dev = &client->dev;
	imx577->client = client;

	imx577->regmap = devm_regmap_init_i2c(client, &imx577_regmap_config);
	if (IS_ERR(imx577->regmap)) {
		dev_err(imx577->dev, "Unable to initialize regmap \n");
		return -ENODEV;
	}

	if (of_property_read_u32(imx577->dev->of_node, "index", &imx577->index)) {
		dev_err(imx577->dev, "Failed to read sensor index. default to 0\n");
		imx577->index = 0;
	}

	/* Initialize subdev */
	v4l2_i2c_subdev_init(&imx577->sd, client, &imx577_subdev_ops);

	ret = imx577_parse_hw_config(imx577);
	if (ret) {
		dev_err(imx577->dev, "HW configuration is not supported");
		return ret;
	}

	mutex_init(&imx577->mutex);

	ret = imx577_power_on(imx577);
	if (ret) {
		dev_err(imx577->dev, "failed to power-on the sensor");
		goto error_mutex_destroy;
	}

	/* Check module identity */
	ret = imx577_get_id(imx577);
	if (ret) {
		dev_err(imx577->dev, "failed to find sensor: %d", ret);
		goto error_power_off;
	}

	/* Set default mode to max resolution */
	imx577->current_mode = &supported_mode[setting_index];

	ret = imx577_init_controls(imx577);
	if (ret) {
		dev_err(imx577->dev, "failed to init controls: %d", ret);
		goto error_power_off;
	}

	/* Initialize subdev */
	imx577->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	imx577->sd.dev = &imx577->client->dev;
	imx577->sd.internal_ops = &imx577_internal_ops;
	imx577->sd.entity.ops = &imx577_subdev_entity_ops;
	imx577->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	snprintf(imx577->sd.name, sizeof(imx577->sd.name), "imx577-%u", imx577->index);

	/* Initialize source pad */
	imx577->pad.flags = MEDIA_PAD_FL_SOURCE;
	ret = media_entity_pads_init(&imx577->sd.entity, 1, &imx577->pad);
	if (ret) {
		dev_err(imx577->dev, "failed to init entity pads: %d", ret);
		goto error_handler_free;
	}

	ret = v4l2_async_register_subdev(&imx577->sd);
	if (ret < 0) {
		dev_err(imx577->dev,
			"failed to register async subdev: %d", ret);
		goto error_media_entity;
	}

	return 0;

error_media_entity:
	media_entity_cleanup(&imx577->sd.entity);
error_handler_free:
	v4l2_ctrl_handler_free(imx577->sd.ctrl_handler);
error_power_off:
	imx577_power_off(imx577);
error_mutex_destroy:
	mutex_destroy(&imx577->mutex);

	return ret;
}

/**
 * imx577_remove() - I2C client device unbinding
 * @client: pointer to I2C client device
 *
 * Return: 0 if successful, error code otherwise.
 */
static int imx577_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct imx577 *imx577 = to_imx577(sd);

	v4l2_async_unregister_subdev(sd);
	media_entity_cleanup(&sd->entity);
	v4l2_ctrl_handler_free(sd->ctrl_handler);

	imx577_power_off(imx577);

	mutex_destroy(&imx577->mutex);

	return 0;
}

static const struct of_device_id imx577_of_match[] = {
	{ .compatible = "sony, imx577" },
	{ }
};

MODULE_DEVICE_TABLE(of, imx577_of_match);

static struct i2c_driver imx577_driver = {
	.probe_new = imx577_probe,
	.remove = imx577_remove,
	.driver = {
		.name = "imx577",
		.of_match_table = imx577_of_match,
	},
};

module_i2c_driver(imx577_driver);

MODULE_DESCRIPTION("Sony imx577 sensor driver");
MODULE_AUTHOR("zhiwei.zhang <zhiwei.zhang@amlogic.com>");
MODULE_LICENSE("GPL v2");

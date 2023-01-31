/*
 * Copyright (c) 2018 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef __AISP_H__
#define __AISP_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    AML_CMD_GET = 0x0,
    AML_CMD_SET,
} aisp_cmd_type_t;

struct aml_format {
    uint32_t xstart;
    uint32_t ystart;
    uint32_t width;
    uint32_t height;
    uint32_t code;
    uint32_t fourcc;
    uint32_t nplanes;
    uint32_t bpp;
};

typedef struct _image_resolution_t {
    uint32_t width;
    uint32_t height;
} image_resolution_t;

typedef struct
{
    uint32_t fps;                             // Actual sensor fps
    uint32_t pixels_per_line;                 // Actual pixels per line after scaling/binning
    int hcg_fixed_ratio;                 // HCG mode fixed gain ratio
    int again_log2;                      // analog gain value in log2 format
    int again_log2_max;                  // Maximum analog gain value in log2 format
    int again_high_log2;                 // HCG analog gain value in log2 format
    int again_high_log2_max;             // Maximum HCG analog gain value in log2 format
    int dgain_log2;                      // digital gain value in log2 format
    int dgain_log2_max;                  // Maximum digital gain value in log2 format
    int dgain_high_log2;                 // HCG analog gain value in log2 format
    int dgain_high_log2_max;             // Maximum HCG analog gain value in log2 format
    int again_accuracy_fmt;              // Precision of the gain format 0: linear 1: log2
    int again_accuracy;                  // Precision of the gain
    int again_high_accuracy_fmt;         // Precision of the HCG format 0: linear 1: log2
    int again_high_accuracy;             // Precision of the HCG
    int dgain_accuracy_fmt;              // Precision of the digital gain format 0: linear 1: log2
    int dgain_accuracy;                  // Precision of the digital gain
    int dgain_high_accuracy_fmt;         // Precision of the HCG digital gain format 0: linear 1: log2
    int dgain_high_accuracy;             // Precision of the HCG digital gain
    int expos_lines;                     // the exposure time lines
    int expos_accuracy;                  // Precision of the exposure time
    int sexpos_lines;                    // the short exposure time lines
    int sexpos_accuracy;                 // Precision of the short exposure time
    int vsexpos_lines;                   // the very short exposure time lines
    int vsexpos_accuracy;                // Precision of the very short exposure time
    int vvsexpos_lines;                  // the very very short exposure time lines
    int vvsexpos_accuracy;               // Precision of the very very short exposure time
    uint32_t integration_time_min;           // Minimum integration time for the sensor in lines
    uint32_t integration_time_max;           // Maximum integration time for the sensor in lines without dropping fps
    uint32_t integration_time_long_max;      // Maximum integration time for long
    uint32_t integration_time_limit;         // Maximum possible integration time for the sensor
    uint16_t day_light_integration_time_max; // Limit of integration time for non-flickering light source
    uint8_t integration_time_apply_delay;    // Delay to apply integration time in frames
    uint8_t isp_exposure_channel_delay;      // Select which WDR exposure channel gain is delayed 0-none, 1-long, 2-medium, 3-short (only 0 and 1 implemented)
    uint8_t gain_apply_delay;                // Delay to apply gain time in frames
    int xoffset;                        // Used for image stabilization
    int yoffset;                        // Used for image stabilization
    uint32_t lines_per_second;               // Number of lines per second used for antiflicker
    int sensor_exp_number;              // Number of different exposures supported by the sensor
    int sensor_gain_number;             // Number of different gain supported by the sensor
    uint8_t bits;                            // current raw bit
    uint8_t raw_mode;                        // current raw format
    uint8_t pattern;                         // current pattern
    image_resolution_t total;               // Total resolution of the image with blanking
    image_resolution_t active;              // Active resolution without blanking

    uint32_t      u32AGain[4];
    uint32_t      u32DGain[4];
    uint32_t      u32Inttime[2][4];

    uint16_t      u16GainCnt;
    uint16_t      u16IntTimeCnt;
} ALG_SENSOR_DEFAULT_S;

typedef struct
{
    void(*pfn_cmos_fps_set)(int ViPipe, float f32Fps, ALG_SENSOR_DEFAULT_S *pstAeSnsDft);
    int (*pfn_cmos_get_alg_default)(int ViPipe, ALG_SENSOR_DEFAULT_S *pstAlgSnsDft);

    /* w_le isp notify ae to update sensor regs, ae call these funcs. */
    void (*pfn_cmos_alg_update)(int ViPipe);

    void (*pfn_cmos_again_calc_table)(int ViPipe, uint32_t *pu32AgainLin, uint32_t *pu32AgainDb);
    void (*pfn_cmos_dgain_calc_table)(int ViPipe, uint32_t *pu32DgainLin, uint32_t *pu32DgainDb);
    void (*pfn_cmos_inttime_calc_table)(int ViPipe, uint32_t pu32ExpL, uint32_t pu32ExpS, uint32_t pu32ExpVS, uint32_t pu32ExpVVS);

} ALG_SENSOR_EXP_FUNC_S;

typedef struct
{
    ALG_SENSOR_DEFAULT_S   stSnsDft;
    ALG_SENSOR_EXP_FUNC_S  stSnsExp;
} AML_ALG_CTX_S;

void aisp_enable(uint32_t ctx_id, void *pstAlgCtx, void *calib);
void aisp_disable(uint32_t ctx_id);
void aisp_alg2user(uint32_t ctx_id, void *param);
void aisp_alg2kernel(uint32_t ctx_id, void *param);
void aisp_fw_interface(uint32_t ctx_id, void *param);

#define LOG2_GAIN_SHIFT 12
#define SHUTTER_TIME_SHIFT 12

#ifdef __cplusplus
}
#endif

#endif

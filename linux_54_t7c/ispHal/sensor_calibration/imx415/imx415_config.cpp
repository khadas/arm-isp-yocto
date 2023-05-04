/*
 * Copyright (c) 2018 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_TAG "imx415Cfg"

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>

#include "aml_isp_api.h"

#include "imx415_sdr_calibration.h"
#include "imx415_wdr_calibration.h"

#include "imx415_api.h"
#include "logs.h"


typedef struct
{
    int  enWDRMode = 0;
    ALG_SENSOR_DEFAULT_S snsAlgInfo;
    struct media_entity  * sensor_ent;
} ISP_SNS_STATE_S;

static ISP_SNS_STATE_S sensor;

void cmos_set_sensor_entity_imx415(struct media_entity * sensor_ent,int wdr)
{
    sensor.sensor_ent = sensor_ent;
    sensor.enWDRMode = wdr;
}

void cmos_get_sensor_calibration_imx415(struct media_entity *sensor_ent, aisp_calib_info_t *calib)
{
    if (sensor.enWDRMode == 1)
        Imx415WdrCalibration::dynamic_wdr_calibrations_init_imx415(calib);
    else
        Imx415SdrCalibration::dynamic_sdr_calibrations_init_imx415(calib);
}

int cmos_get_ae_default_imx415(int ViPipe, ALG_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    INFO("cmos_get_ae_default\n");

    sensor.snsAlgInfo.active.width = 1920;
    sensor.snsAlgInfo.active.height = 1080;
    sensor.snsAlgInfo.fps = 30;
    sensor.snsAlgInfo.sensor_exp_number = 1;
    sensor.snsAlgInfo.bits = 12;

    sensor.snsAlgInfo.sensor_gain_number = 1;
    sensor.snsAlgInfo.total.width = 545;
    sensor.snsAlgInfo.total.height = 4503;

    sensor.snsAlgInfo.lines_per_second = sensor.snsAlgInfo.total.height*30;
    sensor.snsAlgInfo.pixels_per_line = sensor.snsAlgInfo.total.width;

    if (sensor.enWDRMode == 1) {
        sensor.snsAlgInfo.integration_time_long_max = (sensor.snsAlgInfo.total.height*2 - (225+3)) <<SHUTTER_TIME_SHIFT;
        sensor.snsAlgInfo.integration_time_limit = (225-3)<<SHUTTER_TIME_SHIFT;

        sensor.snsAlgInfo.integration_time_min = 1<<SHUTTER_TIME_SHIFT;
        sensor.snsAlgInfo.integration_time_max = (225-3)<<SHUTTER_TIME_SHIFT;
        sensor.snsAlgInfo.integration_time_long_max = (sensor.snsAlgInfo.total.height*2 - (225+3)) <<SHUTTER_TIME_SHIFT;
        sensor.snsAlgInfo.integration_time_limit = (225-3)<<SHUTTER_TIME_SHIFT;
    } else {
        sensor.snsAlgInfo.integration_time_min = 4<<SHUTTER_TIME_SHIFT;
        sensor.snsAlgInfo.integration_time_max = (sensor.snsAlgInfo.total.height - 8)<<SHUTTER_TIME_SHIFT;
        sensor.snsAlgInfo.integration_time_long_max = (sensor.snsAlgInfo.total.height - 8)<<SHUTTER_TIME_SHIFT;
        sensor.snsAlgInfo.integration_time_limit = (sensor.snsAlgInfo.total.height - 8)<<SHUTTER_TIME_SHIFT;
    }

    sensor.snsAlgInfo.again_log2_max = (72/6)<<(LOG2_GAIN_SHIFT);
    sensor.snsAlgInfo.again_high_log2_max = (72/6)<<(LOG2_GAIN_SHIFT);
    sensor.snsAlgInfo.dgain_log2_max = 0;
    sensor.snsAlgInfo.dgain_high_log2_max = 0;
    sensor.snsAlgInfo.dgain_high_accuracy_fmt = 0;
    sensor.snsAlgInfo.dgain_high_accuracy = 1;
    sensor.snsAlgInfo.dgain_accuracy_fmt = 0;
    sensor.snsAlgInfo.dgain_accuracy = 1;
    sensor.snsAlgInfo.again_high_accuracy_fmt = 1;
    sensor.snsAlgInfo.again_high_accuracy = (1<<(LOG2_GAIN_SHIFT))/20;
    sensor.snsAlgInfo.again_accuracy_fmt = 1;
    sensor.snsAlgInfo.again_accuracy = (1<<(LOG2_GAIN_SHIFT))/20;
    sensor.snsAlgInfo.expos_accuracy = (1<<(SHUTTER_TIME_SHIFT));
    sensor.snsAlgInfo.sexpos_accuracy = (1<<(SHUTTER_TIME_SHIFT));
    sensor.snsAlgInfo.vsexpos_accuracy = (1<<(SHUTTER_TIME_SHIFT));
    sensor.snsAlgInfo.vvsexpos_accuracy = (1<<(SHUTTER_TIME_SHIFT));

    sensor.snsAlgInfo.gain_apply_delay = 0;
    sensor.snsAlgInfo.integration_time_apply_delay = 0;
    INFO("cmos_get_ae_default++++++\n");

    memcpy(pstAeSnsDft, &sensor.snsAlgInfo, sizeof(ALG_SENSOR_DEFAULT_S));

    return 0;
}

void cmos_again_calc_table_imx415(int ViPipe, uint32_t *pu32AgainLin, uint32_t *pu32AgainDb)
{
    INFO("cmos_again_calc_table: %d, %d\n", *pu32AgainLin, *pu32AgainDb);
    uint32_t again_reg;
    uint32_t u32AgainDb;

    u32AgainDb = *pu32AgainDb;
    u32AgainDb = ((u32AgainDb*20)>>LOG2_GAIN_SHIFT);

    again_reg = (uint32_t)(u32AgainDb);
    if (again_reg > 720/3) //72dB, 0.3dB step.
        again_reg = 720/3;

    if (sensor.snsAlgInfo.u32AGain[0] != again_reg) {
        sensor.snsAlgInfo.u16GainCnt = sensor.snsAlgInfo.gain_apply_delay + 1;
        sensor.snsAlgInfo.u32AGain[0] = again_reg;
    }

}

void cmos_dgain_calc_table_imx415(int ViPipe, uint32_t *pu32DgainLin, uint32_t *pu32DgainDb)
{
    //CAMHAL_LOGD("cmos_dgain_calc_table: %d, %d\n", *pu32DgainLin, *pu32DgainDb);
}

void cmos_inttime_calc_table_imx415(int ViPipe, uint32_t pu32ExpL, uint32_t pu32ExpS, uint32_t pu32ExpVS, uint32_t pu32ExpVVS)
{
    INFO("cmos_inttime_calc_table: %d, %d, %d, %d\n", pu32ExpL, pu32ExpS, pu32ExpVS, pu32ExpVVS);
    uint32_t shutter_time_lines = pu32ExpL >> SHUTTER_TIME_SHIFT;
    uint32_t shutter_time_line_each_frame = sensor.snsAlgInfo.total.height;

    uint32_t shutter_time_lines_short = pu32ExpS >> SHUTTER_TIME_SHIFT;

    //CAMHAL_LOGD("expo: %d, %d\n", shutter_time_lines, shutter_time_lines_short);
    if (sensor.enWDRMode == 0) {
        if (shutter_time_lines > shutter_time_line_each_frame)
            shutter_time_lines = shutter_time_line_each_frame;
        shutter_time_lines = shutter_time_line_each_frame - shutter_time_lines;
        if (shutter_time_lines)
            shutter_time_lines = shutter_time_lines - 1;
        if (shutter_time_lines < 8)
            shutter_time_lines = 8;
    } else {
        if (shutter_time_lines_short < 1)
            shutter_time_lines_short = 1;
        shutter_time_lines_short = 225 - shutter_time_lines_short - 1;
        shutter_time_lines = shutter_time_line_each_frame * 2  - shutter_time_lines - 1;
    }

    if (sensor.snsAlgInfo.u32Inttime[0][0] != shutter_time_lines || sensor.snsAlgInfo.u32Inttime[1][0] != shutter_time_lines_short) {
        sensor.snsAlgInfo.u16IntTimeCnt = sensor.snsAlgInfo.integration_time_apply_delay + 1;
        sensor.snsAlgInfo.u32Inttime[0][0] = shutter_time_lines;
        sensor.snsAlgInfo.u32Inttime[1][0] = shutter_time_lines_short;
    }
}

void cmos_fps_set_imx415(int ViPipe, float f32Fps, ALG_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    INFO("cmos_fps_set: %f\n", f32Fps);
}

void cmos_alg_update_imx415(int ViPipe)
{
    uint32_t shutter_time_lines = 0;//, shutter_time_lines_short = 0;
    uint32_t i = 0;

    if ( sensor.snsAlgInfo.u16GainCnt || sensor.snsAlgInfo.u16IntTimeCnt ) {
        if ( sensor.snsAlgInfo.u16GainCnt ) {
            sensor.snsAlgInfo.u16GainCnt--;
            struct v4l2_ext_control gain;
            gain.id = V4L2_CID_GAIN;
            gain.value = sensor.snsAlgInfo.u32AGain[sensor.snsAlgInfo.gain_apply_delay];
            v4l2_subdev_set_ctrls(sensor.sensor_ent, &gain, 1);
        }

        // -------- Integration Time ----------
        if ( sensor.snsAlgInfo.u16IntTimeCnt ) {
            sensor.snsAlgInfo.u16IntTimeCnt--;
            shutter_time_lines = sensor.snsAlgInfo.u32Inttime[0][sensor.snsAlgInfo.integration_time_apply_delay];
            if (sensor.enWDRMode == 0) {
                struct v4l2_ext_control expo;
                expo.id = V4L2_CID_EXPOSURE;
                expo.value = shutter_time_lines;
                v4l2_subdev_set_ctrls(sensor.sensor_ent, &expo, 1);
            }

            if (sensor.enWDRMode) {
                //shutter_time_lines_short = sensor.snsAlgInfo.u32Inttime[1][sensor.snsAlgInfo.integration_time_apply_delay];
                //imx415_write_register(ViPipe, 0x3020, shutter_time_lines_short & 0xff);
                //imx415_write_register(ViPipe, 0x3021, (shutter_time_lines_short>>8) & 0xff);
                //imx415_write_register(ViPipe, 0x3024, shutter_time_lines&0xff);
                //imx415_write_register(ViPipe, 0x3025, (shutter_time_lines>>8) & 0xff);
                //CAMHAL_LOGD("sensor expo: %d, %d\n", shutter_time_lines, shutter_time_lines_short);
            }
        }
    }

    for ( i = 3; i > 0; i --) {
        sensor.snsAlgInfo.u32AGain[i] = sensor.snsAlgInfo.u32AGain[i - 1];
        sensor.snsAlgInfo.u32Inttime[0][i] = sensor.snsAlgInfo.u32Inttime[0][i - 1];
        sensor.snsAlgInfo.u32Inttime[1][i] = sensor.snsAlgInfo.u32Inttime[1][i - 1];
    }

}

/*
 * Copyright (c) 2018 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

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

#include "../../media-v4l2/mediactl.h"
#include "../../media-v4l2/v4l2subdev.h"
#include "../../media-v4l2/v4l2videodev.h"

#include "aml_isp_api.h"

#include "sensor_calibration.h"
#include "imx290_api.h"

ISP_SNS_STATE_S g_pastImx290;

void cmos_sensor_control_cb(ALG_SENSOR_EXP_FUNC_S *stSnsExp)
{
    stSnsExp->pfn_cmos_alg_update = cmos_alg_update;
    stSnsExp->pfn_cmos_get_alg_default = cmos_get_ae_default;
    stSnsExp->pfn_cmos_again_calc_table = cmos_again_calc_table;
    stSnsExp->pfn_cmos_dgain_calc_table = cmos_dgain_calc_table;
    stSnsExp->pfn_cmos_inttime_calc_table = cmos_inttime_calc_table;
    stSnsExp->pfn_cmos_fps_set = cmos_fps_set;
}

void cmos_set_sensor_entity(struct media_entity * sensor_ent, int wdr)
{
    g_pastImx290.enWDRMode = wdr;
    g_pastImx290.sensor_ent = sensor_ent;
}

void cmos_get_sensor_calibration(aisp_calib_info_t * calib)
{
    dynamic_calibrations_init(calib);
}

int cmos_get_ae_default(int ViPipe, ALG_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    printf("cmos_get_ae_default\n");

    g_pastImx290.snsAlgInfo.active.width = 1920;
    g_pastImx290.snsAlgInfo.active.height = 1080;
    g_pastImx290.snsAlgInfo.fps = 30;
    g_pastImx290.snsAlgInfo.sensor_exp_number = 1;
    g_pastImx290.snsAlgInfo.bits = 12;

    g_pastImx290.snsAlgInfo.sensor_gain_number = 1;
    g_pastImx290.snsAlgInfo.total.width = 4400;
    g_pastImx290.snsAlgInfo.total.height = 1125;

    g_pastImx290.snsAlgInfo.lines_per_second = g_pastImx290.snsAlgInfo.total.height*30;
    g_pastImx290.snsAlgInfo.pixels_per_line = g_pastImx290.snsAlgInfo.total.width;

    if (g_pastImx290.enWDRMode == 1) {
        g_pastImx290.snsAlgInfo.integration_time_min = 1<<SHUTTER_TIME_SHIFT;
        g_pastImx290.snsAlgInfo.integration_time_max = (225 - 3) << SHUTTER_TIME_SHIFT;
        g_pastImx290.snsAlgInfo.integration_time_long_max = (g_pastImx290.snsAlgInfo.total.height*2 - (225 + 3)) << SHUTTER_TIME_SHIFT;
        g_pastImx290.snsAlgInfo.integration_time_limit = (225 - 3)<<SHUTTER_TIME_SHIFT;
    } else {
        g_pastImx290.snsAlgInfo.integration_time_min = 1<<SHUTTER_TIME_SHIFT;
        g_pastImx290.snsAlgInfo.integration_time_max = g_pastImx290.snsAlgInfo.total.height<<SHUTTER_TIME_SHIFT;
        g_pastImx290.snsAlgInfo.integration_time_long_max = g_pastImx290.snsAlgInfo.total.height<<SHUTTER_TIME_SHIFT;
        g_pastImx290.snsAlgInfo.integration_time_limit = g_pastImx290.snsAlgInfo.total.height<<SHUTTER_TIME_SHIFT;
    }

    g_pastImx290.snsAlgInfo.again_log2_max = (72/6)<<(LOG2_GAIN_SHIFT);
    g_pastImx290.snsAlgInfo.again_high_log2_max = (72/6)<<(LOG2_GAIN_SHIFT);
    g_pastImx290.snsAlgInfo.dgain_log2_max = 0;
    g_pastImx290.snsAlgInfo.dgain_high_log2_max = 0;
    g_pastImx290.snsAlgInfo.dgain_high_accuracy_fmt = 0;
    g_pastImx290.snsAlgInfo.dgain_high_accuracy = 1;
    g_pastImx290.snsAlgInfo.dgain_accuracy_fmt = 0;
    g_pastImx290.snsAlgInfo.dgain_accuracy = 1;
    g_pastImx290.snsAlgInfo.again_high_accuracy_fmt = 1;
    g_pastImx290.snsAlgInfo.again_high_accuracy = (1<<(LOG2_GAIN_SHIFT))/20;
    g_pastImx290.snsAlgInfo.again_accuracy_fmt = 1;
    g_pastImx290.snsAlgInfo.again_accuracy = (1<<(LOG2_GAIN_SHIFT))/20;
    g_pastImx290.snsAlgInfo.expos_lines = (0x2A2<<(SHUTTER_TIME_SHIFT));
    g_pastImx290.snsAlgInfo.expos_accuracy = (1<<(SHUTTER_TIME_SHIFT));
    g_pastImx290.snsAlgInfo.sexpos_lines = (1<<(SHUTTER_TIME_SHIFT));
    g_pastImx290.snsAlgInfo.sexpos_accuracy = (1<<(SHUTTER_TIME_SHIFT));
    g_pastImx290.snsAlgInfo.vsexpos_lines = (1<<(SHUTTER_TIME_SHIFT));
    g_pastImx290.snsAlgInfo.vsexpos_accuracy = (1<<(SHUTTER_TIME_SHIFT));
    g_pastImx290.snsAlgInfo.vvsexpos_lines = (1<<(SHUTTER_TIME_SHIFT));
    g_pastImx290.snsAlgInfo.vvsexpos_accuracy = (1<<(SHUTTER_TIME_SHIFT));

    g_pastImx290.snsAlgInfo.gain_apply_delay = 0;
    g_pastImx290.snsAlgInfo.integration_time_apply_delay = 0;

    memcpy(pstAeSnsDft, &g_pastImx290.snsAlgInfo, sizeof(ALG_SENSOR_DEFAULT_S));

    return 0;
}

void cmos_again_calc_table(int ViPipe, uint32_t *pu32AgainLin, uint32_t *pu32AgainDb)
{
    printf("cmos_again_calc_table: %d, %d\n", *pu32AgainLin, *pu32AgainDb);
    uint32_t again_reg;
    uint32_t u32AgainDb;

    u32AgainDb = *pu32AgainDb;
    u32AgainDb = ((u32AgainDb*20)>>LOG2_GAIN_SHIFT);

    again_reg = (uint32_t)(u32AgainDb);
    if (again_reg > 720/3) //72dB, 0.3dB step.
        again_reg = 720/3;

    if (g_pastImx290.snsAlgInfo.u32AGain[0] != again_reg) {
        g_pastImx290.snsAlgInfo.u16GainCnt = g_pastImx290.snsAlgInfo.gain_apply_delay + 1;
        g_pastImx290.snsAlgInfo.u32AGain[0] = again_reg;
    }

}

void cmos_dgain_calc_table(int ViPipe, uint32_t *pu32DgainLin, uint32_t *pu32DgainDb)
{
    //printf("cmos_dgain_calc_table: %d, %d\n", *pu32DgainLin, *pu32DgainDb);
}

void cmos_inttime_calc_table(int ViPipe, uint32_t pu32ExpL, uint32_t pu32ExpS, uint32_t pu32ExpVS, uint32_t pu32ExpVVS)
{
    printf("cmos_inttime_calc_table: %d, %d, %d, %d\n", pu32ExpL, pu32ExpS, pu32ExpVS, pu32ExpVVS);
    uint32_t shutter_time_lines = pu32ExpL >> SHUTTER_TIME_SHIFT;
    uint32_t shutter_time_line_each_frame = g_pastImx290.snsAlgInfo.total.height;

    uint32_t shutter_time_lines_short = pu32ExpS >> SHUTTER_TIME_SHIFT;

    //printf("expo: %d, %d\n", shutter_time_lines, shutter_time_lines_short);
    if (g_pastImx290.enWDRMode == 0) {
        if (shutter_time_lines > shutter_time_line_each_frame)
            shutter_time_lines = shutter_time_line_each_frame;
        shutter_time_lines = shutter_time_line_each_frame - shutter_time_lines;
        if (shutter_time_lines)
            shutter_time_lines = shutter_time_lines - 1;
        if (shutter_time_lines < 1)
            shutter_time_lines = 1;
    } else {
        if (shutter_time_lines_short < 1)
            shutter_time_lines_short = 1;
        shutter_time_lines_short = 225 - shutter_time_lines_short - 1;
        shutter_time_lines = shutter_time_line_each_frame * 2  - shutter_time_lines - 1;
    }

    if (g_pastImx290.snsAlgInfo.u32Inttime[0][0] != shutter_time_lines || g_pastImx290.snsAlgInfo.u32Inttime[1][0] != shutter_time_lines_short) {
        g_pastImx290.snsAlgInfo.u16IntTimeCnt = g_pastImx290.snsAlgInfo.integration_time_apply_delay + 1;
        g_pastImx290.snsAlgInfo.u32Inttime[0][0] = shutter_time_lines;
        g_pastImx290.snsAlgInfo.u32Inttime[1][0] = shutter_time_lines_short;
    }
}

void cmos_fps_set(int ViPipe, float f32Fps, ALG_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    printf("cmos_fps_set: %f\n", f32Fps);
}

void cmos_alg_update(int ViPipe)
{
    uint32_t shutter_time_lines = 0;//, shutter_time_lines_short = 0;
    uint32_t i = 0;

    if ( g_pastImx290.snsAlgInfo.u16GainCnt || g_pastImx290.snsAlgInfo.u16IntTimeCnt ) {
        if ( g_pastImx290.snsAlgInfo.u16GainCnt ) {
            g_pastImx290.snsAlgInfo.u16GainCnt--;
            struct v4l2_ext_control gain;
            gain.id = V4L2_CID_GAIN;
            gain.value = g_pastImx290.snsAlgInfo.u32AGain[g_pastImx290.snsAlgInfo.gain_apply_delay];
            v4l2_subdev_set_ctrls(g_pastImx290.sensor_ent, &gain, 1);
        }

        // -------- Integration Time ----------
        if ( g_pastImx290.snsAlgInfo.u16IntTimeCnt ) {
            g_pastImx290.snsAlgInfo.u16IntTimeCnt--;
            shutter_time_lines = g_pastImx290.snsAlgInfo.u32Inttime[0][g_pastImx290.snsAlgInfo.integration_time_apply_delay];
            if (g_pastImx290.enWDRMode == 0) {
                struct v4l2_ext_control expo;
                expo.id = V4L2_CID_EXPOSURE;
                expo.value = shutter_time_lines;
                v4l2_subdev_set_ctrls(g_pastImx290.sensor_ent, &expo, 1);
            }

            if (g_pastImx290.enWDRMode) {
                //shutter_time_lines_short = g_pastImx290.snsAlgInfo.u32Inttime[1][g_pastImx290.snsAlgInfo.integration_time_apply_delay];
                //imx290_write_register(ViPipe, 0x3020, shutter_time_lines_short & 0xff);
                //imx290_write_register(ViPipe, 0x3021, (shutter_time_lines_short>>8) & 0xff);
                //imx290_write_register(ViPipe, 0x3024, shutter_time_lines&0xff);
                //imx290_write_register(ViPipe, 0x3025, (shutter_time_lines>>8) & 0xff);
                //printf("sensor expo: %d, %d\n", shutter_time_lines, shutter_time_lines_short);
            }
        }
    }

    for ( i = 3; i > 0; i --) {
        g_pastImx290.snsAlgInfo.u32AGain[i] = g_pastImx290.snsAlgInfo.u32AGain[i - 1];
        g_pastImx290.snsAlgInfo.u32Inttime[0][i] = g_pastImx290.snsAlgInfo.u32Inttime[0][i - 1];
        g_pastImx290.snsAlgInfo.u32Inttime[1][i] = g_pastImx290.snsAlgInfo.u32Inttime[1][i - 1];
    }

}

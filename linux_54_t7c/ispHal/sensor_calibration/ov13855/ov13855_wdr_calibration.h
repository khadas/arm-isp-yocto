/**
 *  @file
 *
 *  @copyright Copyright (c) 2021 Amlogic, Inc.
 *
 *  This file and its contents ("Software") are protected by intellectual property rights including, without limitation,
 *  China. and/or foreign copyrights.  This Software is also the confidential and proprietary information of Amlogic, Inc.
 *  and its licensors.  You may not use, reproduce, disclose, distribute, modify, or otherwise prepare derivative works
 *  of this Software or any portion thereof except pursuant to a signed license agreement or nondisclosure agreement with
 *  Amlogic, Inc. or its authorized affiliates.  In the absence of such an agreement, you agree to promptly notify and
 *  return this Software to Amlogic, Inc.
 *
 **/
#include "aml_isp_tuning.h"

namespace Ov13855WdrCalibration {
//aisp_top_ctl_t
static int32_t _CALIBRATION_TOP_CTL[50] = {
    1, // ISP input channels n+1
    1, // wdr enable 0:off 1:on
    1, // WDR input channels n+1
    0, // decmp enable 0:off 1:on
    0, // ifmt enable 0:off 1:on
    0, // bac enable 0:off 1:on
    1, // fpnr enable 0:off 1:on
    1, // ge enable 0:off 1:on
    1, // dpc enable 0:off 1:on
    0, // pat enable 0:off 1:on
    1, // og enable 0:off 1:on
    1, // sqrt_eotf enable 0:off 1:on
    0, // lcge enable 0:off 1:on
    1, // pdpc enable 0:off 1:on
    1, // cac enable 0:off 1:on
    0, // rawcnr enable 0:off 1:on
    1, // snr1 enable 0:off 1:on
    1, // mc_tnr enable 0:off 1:on
    1, // tnr0 enable 0:off 1:on
    1, // cubic_cs enable 0:off 1:on
    1, // ltm enable 0:off 1:on
    0, // gtm enable 0:off 1:on
    1, // lns_mesh enable 0:off 1:on
    0, // lns_rad enable 0:off 1:on
    1, // wb enable 0:off 1:on
    1, // blc enable 0:off 1:on
    1, // nr enable 0:off 1:on
    1, // pk enable 0:off 1:on
    1, // dnlp enable 0:off 1:on
    0, // dhz enable 0:off 1:on
    1, // lc enable 0:off 1:on
    1, // bsc enable 0:off 1:on
    1, // cnr2 enable 0:off 1:on
    1, // gamma enable 0:off 1:on
    1, // ccm enable 0:off 1:on
    1, // dmsc enable 0:off 1:on
    1, // csc enable 0:off 1:on
    1, // ptnr enable 0:off 1:on
    1, // amcm enable 0:off 1:on
    1, // flkr stat enable 0:off 1:on
    1, // flkr stat switch 0:from FEO 1:from NR 2:from Post
    1, // awb stat enable 0:off 1:on
    2, // awb stat switch 0:from FE 1:from GE 2:before WB 3:after WB 4:from DRC 5 or else:from peak
    1, // ae stat enable 0:off 1:on
    1, // ae stat switch 0:from GE 1:from LSC 2:before DRC 3:after DRC
    0, // af stat enable 0:off 1:on
    0, // af stat switch 0:from SNR 1:from DMS 2or3:from peak
    1, // WDR stat enable 0:off 1:on
    0, // debug path output 0:off 1:on
    0, // debug path data select
};

//aisp_res_t
static uint32_t _CALIBRATION_RES_CTL[7] = {
    0, //crop_en
    0, //crop_ofs_x
    0, //crop_ofs_y
    1920, //crop_width
    1080,  //crop_height
    0, //bin_en
    0, //bin_mode
};

//aisp_awb_t
static int32_t _CALIBRATION_AWB_CTL[16] = {
    1,       // u8, AWB auto enable
    1,       // u8, AWB manual mode, 0: manual gain mode, 1: manual temperature mode
    32,      //u8, AWB convergence speed.
    0,       //u8, mixed color temperature mode option. 0:mix mode 1:outdoor mode 2:indoor mode 3:auto mode
    32,      //u16, a cover range around planck curve
    1,       //u1, color temperature dynamic cover range enable
    0,       //u1, color temperature luma weighted calculation enable by luma value of local block
    1,       //u1, color temperature luma weighted calculation enable
    0,       //u1, color temperature adjust enable
    1,       //u1, awb delay adjust enable
    10,      //u16, awb delay frame count
    200,     //u16, awb delay adjust tolerance by color temperature
    256,     //u16, manual awb mode red gain
    256,     //u16, manual awb mode blue gain
    5000,     //u16, manual awb mode temperature
    0,       //bit[0] color temperature hist, [1] weight table, [2] ct table, [3] log
};

//_CALIBRATION_AWB_CT_POS
static uint32_t _CALIBRATION_AWB_CT_POS[20] = {10000,7500,6500,5000,4050,3850,2800,2200};

//_CALIBRATION_AWB_CT_RG_COMPENSATION
static int32_t  _CALIBRATION_AWB_CT_RG_COMPENSATION[20] = {0,0,0,0,0,0,0,0};

//_CALIBRATION_AWB_CT_BG_COMPENSATION
static int32_t  _CALIBRATION_AWB_CT_BG_COMPENSATION[20] = {0,0,0,0,0,0,0,0};

//_CALIBRATION_AWB_CT_WGT
static int32_t _CALIBRATION_AWB_CT_WGT[20] = {1,1,2,3,2,1,1,1};

//_CALIBRATION_AWB_CT_DYN_CVRANGE
static int32_t _CALIBRATION_AWB_CT_DYN_CVRANGE[2][20] = {
    {-10,-8,-2,16,8,-12,-12,-16},
    {-10,-8,-2,16,8,-12,-12,-16},
};

//aisp_ae_t
static int32_t _CALIBRATION_AE_CTL[29] = {
    1,  //ae auto enable
    0,  //ae exposure mode, 0: none, 1: spot mode 2:center mode 3: upper part mode 4: lower part mode
    0,  //ae exposure strategy, 0: none mode, 1: outdoor mode, 2:indoor mode
    0,  // ae route strategy, 0: exposure priority, 1: gain priority 2: external ae route
    0,  //ae route deflicker mode, 0: none, 1: anti-50hz, 2: anti-60hz, 3: auto detected
    30,   //exposure convergence speed [0, 128]
    128,  //ae global luma target compensation
    150,  //ae luma target srgb curve
    40,   //ae luma wdr target
    0,    //low light enhancement mode, 0: adjust exposure 1: adjust curve
    256,  //[0,256] low light enhancement strength
    4096, //[1024, 1024*(1<<8)]low light gain maximum limit
    16,    // [0,1024] high light reduce trigger threshold
    128,    // [0,1024] high light reduce strength
    10,   //ae tolerance
    1, //ae delay adjust enable
    10, //ae delay frame count
    100, //ae delay adjust tolerance
    50,   //WDR mode only: Max percentage of clipped pixels for long exposure: WDR mode only: 256 = 100% clipped pixels
    15,   //WDR mode only: Time filter for exposure ratio
    0,   //reduce fps feature enable.
    15,        //target fps of reduce frame rates.
    (4<<12),   //trigger threshold of the reduce fps, write gain log2 value.
    (1<<10),   //lag threshold of the reduce fps, write gain log2 value.
    (2<<12),        // max isp gain limit, exp: x4 = log2(4)<<12 = 2<<12
    (1000<<12),     // max shutter time limit, exp:  1000ms = 1000<<12
    (30720),       // max total gain limit, exp:x1024 = log2(1024)<<12 = 10<<12, 54db = (54/6)<<12 = 9<<12
    (10<<6),       // max exposure ratio limit, exp: x128 = 128<<6
    0,       //ae debug:bit[0] target, [1] ratio, [2] exposure calculate
};

static int32_t _CALIBRATION_AE_CORR_LUT[64] =  {128, 128, 110, 100, 80, 80, 80, 80};

static int32_t _CALIBRATION_AE_CORR_POS_LUT[64] = {41516+(0<<12), 41516+(1<<12), 41516+(2<<12), 41516+(3<<12), 41516+(4<<12), 41516+(5<<12),41516+(6<<12),41516+(7<<12)};

static int32_t _CALIBRATION_AE_ROUTE[1+2*16] = {
/* shuttertime  | gain*/
/* x ms         | n gain */
    6,              /* total 8 joints */
    0, 10*(1<<12),  /*joint1: 0ms->10ms*/
    1, 2*(1<<12),   /*joint2: x1->x2 gain*/
    0, 20*(1<<12),  /*joint3: 10ms->20ms*/
    1, 4*(1<<12),   /*joint4: x2->x4 gain*/
    0, 40*(1<<12),  /*joint5: 20ms->40ms*/
    1, 64*(1<<12),  /*joint6: x4->x64 gain*/
};

static uint8_t _CALIBRATION_AE_WEIGHT_H[17] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
static uint8_t _CALIBRATION_AE_WEIGHT_V[15] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

//aisp_dn_det_t
static int32_t _CALIBRATION_DAYNIGHT_DETECT[14] = {
    0,    //light_control; 1:0n, 0: off
    0,    // hist_stat_mode; 0: average based AE, 1: weight
    120,  // predict_day_thr;  default is 50
    60,   // predict_night_thr; default is 50
    8,    // dn_det_tran_ratio; default 16/128
    240,  // dn_det_day_thr; default 60
    240,  // dn_det_night_thr;  default 240
    2000, // dn_det_light_ct_low;
    5000, // dn_det_light_ct_high;
    1023, //dn_wdr_mean_ratio
    300, // dn_rg_blk_sum_thr
    400, //dn_rg_thr
    400, //dn_bg_thr
    0, //print_debug 0:not print 1:print
};

//aisp_af_t
static uint32_t _CALIBRATION_AF_CTL[22] = {
    1, //af_en;
    1, //af_pos_min_down;
    1, //af_pos_min;
    1, //af_pos_min_up;
    1, //af_pos_inf_down;
    1, //af_pos_inf;
    1, //af_pos_inf_up;
    1, //af_pos_macro_down;
    1, //af_pos_macro;
    1, //af_pos_macro_up;
    1, //af_pos_max_down;
    1, //af_pos_max;
    1, //af_pos_max_up;
    1, //af_fast_search_positions;
    1, //af_skip_frames_init;
    1, //af_skip_frames_move;
    1, //af_dynamic_range_th;
    1, //af_spot_tolerance;
    1, //af_exit_th;
    1, //af_caf_trigger_th;
    1, //af_caf_stable_th;
    1, //af_print_debug;
};

static uint8_t _CALIBRATION_AF_WEIGHT_H[17] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
static uint8_t _CALIBRATION_AF_WEIGHT_V[15] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

//aisp_flkr_t
static uint32_t _CALIBRATION_FLICKER_CTL[13] = {
    1,      //u32, whether delete invalid flicker
    0,      //u32, 0: half (reg_flkr_stat_yed-reg_flkr_stat_yst) statistic, 1: the whole (reg_flkr_stat_yed-reg_flkr_stat_yst) statistic.
    1,      //u32, 0:no lpf,1: [1 2 1]/4, 2: [1 2 2 2 1]/8, 3: [1 1 1 2 1 1 1]/8, 4 or else: [1 2 2 2 2 2 2 2 1]/16, lpf of row avg for flicker detection
    10,      //u32, output flicker result after flkr_det_cnt
    64000,  //u32, peaks/valleys interval thrd for valid wave
    5,      //u32, peaks/valleys value for valid wave
    20,     //u32, peaks/valleys value difference for valid wave
    1,      //u32, enable fft valid flicker detection
    512,   //u32, fft nlen, default value is recommended
    1,      //u32, fft mlen, default value is recommended
    100,      //u32, fft norm, default value is recommended
    1000,   //u32, threshold for valid flicker of fft, default value is recommended
    1,   //u32, sensor exposure information adjust gain, default is 1, 2x2bin is 2
};

static uint16_t _CALIBRATION_GTM[129]= {
    0,  32,  64,  96,  128, 160,  192,  224,  256,  288,  320,  352,  384,  416,  448,  480,  512,
    544,  576,  608,  640,  672,  704,  736,  768,  800,  832,  864,  896,  928,  960,  992,  1024,
    1056,  1088,  1120,  1152,  1184,  1216,  1248,  1280,  1312,  1344,  1376,  1408,  1440,  1472,
    1504,  1536,  1568,  1600,  1632,  1664,  1696,  1728,  1760,  1792,  1824,  1856,  1888,  1920,
    1952,  1984,  2016,  2048,  2080,  2112,  2144,  2176,  2208,  2240,  2272,  2304,  2336,  2368,
    2400,  2432,  2464,  2496,  2528,  2560,  2592,  2624,  2656,  2688,  2720,  2752,  2784,  2816,
    2848,  2880,  2912,  2944,  2976,  3008,  3040,  3072,  3104,  3136,  3168,  3200,  3232,  3264,
    3296,  3328,  3360,  3392,  3424,  3456,  3488,  3520,  3552,  3584,  3616,  3648,  3680,  3712,
    3744,  3776,  3808,  3840,  3872,  3904,  3936,  3968,  4000,  4032,  4064,  4095,
};

//aisp_ge_adj_t
static uint8_t _CALIBRATION_GE_ADJ[ISO_NUM_MAX][8] = {
/*stat_edge_thd|ge_hv_thrd|ge_hv_wtlut[4]|reserve*/
    { 72,     48,    10,10,10,10,    0,0,},
    { 72,     48,    12,12,12,12,    0,0,},
    { 72,     48,    14,14,14,14,    0,0,},
    { 72,     48,    14,14,14,14,    0,0,},
    { 96,     64,    16,16,16,16,    0,0,},
    { 80,     56,    16,16,16,16,    0,0,},
    { 96,     64,    18,18,18,18,    0,0,},
    {106,     72,    22,22,22,22,    0,0,},
    {106,     72,    22,22,22,22,    0,0,},
    {106,     72,    22,22,22,22,    0,0,},
};

//aisp_ge_adj_t
static uint8_t _CALIBRATION_GE_S_ADJ[ISO_NUM_MAX][8] = {
/*stat_edge_thd|ge_hv_thrd|ge_hv_wtlut[4]|reserve*/
    { 72,     48,    10,10,10,10,    0,0,},
    { 72,     48,    12,12,12,12,    0,0,},
    { 72,     48,    14,14,14,14,    0,0,},
    { 72,     48,    14,14,14,14,    0,0,},
    { 96,     64,    16,16,16,16,    0,0,},
    { 80,     56,    16,16,16,16,    0,0,},
    { 96,     64,    18,18,18,18,    0,0,},
    {106,     72,    22,22,22,22,    0,0,},
    {106,     72,    22,22,22,22,    0,0,},
    {106,     72,    22,22,22,22,    0,0,},
};

//aisp_dpc_ctl_t
static uint8_t _CALIBRATION_DPC_CTL[8] = {
    1,         /**< cor_en */
    0,         /**< avg_dev_mode */
    3,         /**< avg_mode */
    0,         /**< avg_thd2_en */
    0,         /**< highlight_en */
    3,         /**< correct_mode */
    0,         /**< write_to_lut */
    0,         /**< reserve */
};

//aisp_dpc_ctl_t
static uint8_t _CALIBRATION_DPC_S_CTL[8] = {
    1,         /**< cor_en */
    0,         /**< avg_dev_mode */
    3,         /**< avg_mode */
    0,         /**< avg_thd2_en */
    0,         /**< highlight_en */
    3,         /**< correct_mode */
    0,         /**< write_to_lut */
    0,         /**< reserve */
};

//aisp_dpc_adj_t
static uint16_t _CALIBRATION_DPC_ADJ[ISO_NUM_MAX][12] = {
/* avg_gain_l0|avg_gain_h0|avg_gain_l1|avg_gain_h1|avg_gain_l2|avg_gain_h2|cond_en|max_min_bias_thd|std_diff_gain|std_gain|avg_dev_offset|reserve */
    {30,    750,    40,    650,    50,    550,    0,    3,     22,    12,    0,    0,},
    {30,    750,    40,    650,    50,    550,    0,    5,     22,    12,    0,    0,},
    {50,    750,    55,    600,    60,    500,    0,    10,    22,    12,    0,    0,},
    {50,    600,    55,    500,    60,    400,    0,    15,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    20,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    25,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    30,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    30,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    30,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    30,    22,    12,    0,    0,},
};

//aisp_dpc_adj_t
static uint16_t _CALIBRATION_DPC_S_ADJ[ISO_NUM_MAX][12] = {
/* avg_gain_l0|avg_gain_h0|avg_gain_l1|avg_gain_h1|avg_gain_l2|avg_gain_h2|cond_en|max_min_bias_thd|std_diff_gain|std_gain|avg_dev_offset|reserve */
    {30,    750,    40,    650,    50,    550,    0,    3,     22,    12,    0,    0,},
    {30,    750,    40,    650,    50,    550,    0,    5,     22,    12,    0,    0,},
    {50,    750,    55,    600,    60,    500,    0,    10,    22,    12,    0,    0,},
    {50,    600,    55,    500,    60,    400,    0,    15,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    20,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    25,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    30,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    30,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    30,    22,    12,    0,    0,},
    {50,    400,    55,    350,    60,    300,    0,    30,    22,    12,    0,    0,},
};

//aisp_wdr_t
static int32_t _CALIBRATION_WDR_CTL[37] = {
    // wdr hr regs
    1,                              // u1, WDR motion detection enable,0: disable, 1: enable,
    0,                              // u1, Pixel value wi/wo blc mode in MD,0: pixel value without blc for MD threshold calculation, 1: pixel value with blc for MD threshold calculation,
    0,                              // u2, Check saturation mode in MD,0:  check G & C with blc, 1: check G & C without blc, 2: check G & C with blc*, 3: check G & C without blc*,
    0,                              // u1, Motion map mode,0: final map determined by Gdiff, 1: final map determined by MAX3(Gmap, Rmap, Bmap),
    0,                              // u1, Check still in motion decision, 0: check G without blc, 1: check G & C without blc,
    0,                              // u8, Reduce motion map value in order to include more long exposure data,
    20,                             // u8, When motion map value is less than this threshold, this motion map is set as 0,
    1,                              // u2, WDR forcelong feature enable, 0: disable, 1:based on 32x32 long exposure data in previous frame, 2:based on theoretical model, increase short-exp data under specific condition to avoid discontinuity,
    0,                              // u1, WDR forcelong feature threshold calculation mode, 0: flong1 mode, 1:flong2 mode,
    1000,                           // u14, Low threshold for mask interpolation in flong1 mode,
    1600,                           // u14, High threshold for mask interpolation in flong1 mode,
    3,                              // u2, WDR exposure fusing mode, 0:original long and short data, 1:check G with BLC, 2:check G without BLC, 3:check G & C with BLC, 4:check G & C without BLC,
    4,                              // u4, Final index calculated by ratio of max and avg 0->using max, (short exp) 15-> using avg(long exp),
    1,                              // u1,  WDR stat lpf enable,, 0: disable, 1:enable,
    // fw regs for lo/hi_weight
    -44,                              // s8, Low weight offset[0] for MD,
    0,                              // s8, Low weight offset[1] for MD,
    0,                              // s8, Low weight offset[2] for MD,
    -60,                              // s8, Hi weight offset[0] for MD,
    0,                              // s8, Hi weight offset[0] for MD,
    0,                              // s8, Hi weight offset[0] for MD,
    // fw regs for motion detection
    1,                              // u1, auto enable
    0,                              // u1, MD saturation thd calc mode, 0: user defined; 1: firmware calculation,
    0,                              // u1, MD weight calculation mode, 0: user defined; 1: fw calculation
    0,                              // s8, user defined gr saturation margin for motion detection
    0,                              // s8, user defined gb saturation margin for motion detection
    0,                              // s8, user defined rg saturation margin for motion detection
    0,                              // s8, user defined bg saturation margin for motion detection
    0,                              // s8, user defined ir saturation margin for motion detection
    // fw regs for forcelong
    1,                              // u1,flong1 mode;0: used defined,1: firmware calculation
    800,                            // u14,threshold of day scene discrimination
    500,                            // u14,threshold of night scene discrimination
    1000,                           // u14,low threshold for day scene discrimination
    1600,                           // u14,high threshold for day scene discrimination
    2000,                           // u14,low threshold for night scene discrimination
    3800,                           // u14,high threshold for night scene discrimination
    // fw regs for force exp
    0,                              // u1, force long exp function,0: disable; 1:enable
    0,                              // u3, when force long exp is enabled, using reg_wdr_force_exp_mode to select the out exp, 0: long exp; 1: short1 exp; 2: short2 exp
};

//WDR cabliration parameters
static uint32_t _CALIBRATION_WDR_ADJUST[ISO_NUM_MAX][3] = {
/* mdetc ratio| noise gain | noise flor */
    { 128,     1,     1,},
    { 256,     2,     2,},
    { 512,     4,     3,},
    {1024,     8,     4,},
    {1536,    16,     6,},
    {2048,    32,    11,},
    {2560,    64,    17,},
    {3072,    64,    21,},
    {3584,    64,    21,},
    {4096,    64,    21,},
};

static uint8_t _CALIBRATION_WDR_MDETC_LOWEIGHT[ISO_NUM_MAX][16] = {
    {64,  64,  64,  72,  80,  96, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {64,  64,  64,  80,  96,  96, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {72,  72,  80,  96, 112, 128, 192, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {80,  80,  88, 104, 128, 160, 192, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {88,  88,  88, 128, 160, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {96,  96, 128, 160, 192, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {128, 128, 160, 192, 224, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {160, 160, 176, 192, 224, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {192, 192, 208, 224, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
};

static uint8_t _CALIBRATION_WDR_MDETC_HIWEIGHT[ISO_NUM_MAX][16] = {
    {88,  90,  90,  96,  96, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {90,  90,  96, 104, 112, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {94,  94, 104, 112, 120, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {96,  96, 112, 120, 128, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {104, 104, 112, 136, 168, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {128, 128, 160, 160, 192, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {160, 160, 176, 184, 192, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {192, 192, 200, 224, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,},
};

static int32_t _CALIBRATION_RAWCNR_CTL[3] = {
    1,  //rawcnr_totblk_higfrq_en
    1,  //rawcnr_curblk_higfrq_en
    0,  //rawcnr_ishigfreq_mode
};

//aisp_rawcnr_adj_t
static uint16_t _CALIBRATION_RAWCNR_ADJ[ISO_NUM_MAX][10] = {
/*sad_cor_np_gain | sublk_sum_dif_thd|curblk_sum_difnxn_thd|ya_min|ya_max|ca_min|ca_max|reserve*/
    {8,      50,80,    1800,1600,    0,    30,    0,    10,    0,},
    {8,    100,150,    1800,1600,    0,    40,    0,    20,    0,},
    {8,    100,200,    1800,1600,    0,    42,    0,    22,    0,},
    {8,    200,300,    1800,1600,    0,    44,    0,    24,    0,},
    {8,    200,300,    1800,1600,    0,    46,    0,    26,    0,},
    {8,    460,520,    1800,1600,    0,    50,    0,    30,    0,},
    {8,    460,520,    1800,1600,    0,    56,    0,    34,    0,},
    {8,    460,520,    1800,1600,    0,    60,    0,    36,    0,},
    {8,    460,520,    1800,1600,    5,    65,    5,    36,    0,},
    {8,    460,520,    1800,1600,    5,    70,    5,    38,    0,},
};

//aisp_rawcnr_t->rawcnr_meta_gain_lut
static uint8_t _CALIBRATION_RAWCNR_META_GAIN_LUT[ISO_NUM_MAX][8] = {
    {20,20,18,16,16,16,16,8,},
    {20,20,18,16,16,16,16,16,},
    {20,20,18,16,16,16,16,16,},
    {20,20,18,16,16,16,16,16,},
    {20,20,18,16,16,16,16,16,},
    {24,24,20,20,20,20,20,20,},
    {24,24,20,20,20,20,20,20,},
    {24,24,20,20,20,20,20,20,},
    {24,24,20,20,20,20,20,20,},
    {24,24,20,20,20,20,20,20,},
};

//aisp_rawcnr_t->rawcnr_sps_csig_weight5x5
static int8_t _CALIBRATION_RAWCNR_SPS_CSIG_WEIGHT5X5[ISO_NUM_MAX][25] = {
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
    {1,1,1,0,0,1,2,1,1,0,1,2,2,1,0,1,2,1,1,0,1,1,1,0,0,},
};

//aisp_snr_ctl_t
static int32_t _CALIBRATION_SNR_CTL[34] = {
    1,  //snr_luma_adj_en
    1,  //snr_sad_wt_adjust_en
    1,  //snr_mask_en
    1,  //snr_meta_en
    0,  //rad_snr1_en
    //snr_grad_gain[5]
    36, 36, 36, 32, 24,
    //snr_sad_th_mask_gain[4]
    32, 32, 32, 32,
    0, //snr_coring_mv_gain_x
    4, //snr_coring_mv_gain_xn
    //snr_coring_mv_gain_y[2]
    52, 52,
    1,  //snr_wt_var_adj_en
    4,  //snr_wt_var_th_x
    2,2,3,  //snr_wt_var_th_x
    255, 64, 64,  //snr_wt_var_th_y
    16,16,16,16,32,64,64,128,      //snr_mask_adj
};

//aisp_ snr_glb_adj_t
static uint16_t _CALIBRATION_SNR_GLB_ADJ[ISO_NUM_MAX][6] = {
/*snr_np_lut16_glb_adj|snr_meta2alp_glb_adj|snr_meta_gain_glb_adj|snr_wt_luma_gain_glb_adj|snr_grad_gain_glb_adj|snr_sad_th_mask_gain_glb_adj*/
    {256,    256,    256,    256,    256,    256,},
    {256,    256,    256,    256,    256,    256,},
    {256,    256,    256,    256,    256,    256,},
    {256,    256,    256,    256,    256,    256,},
    {256,    256,    256,    256,    256,    256,},
    {256,    256,    256,    256,    256,    256,},
    {256,    256,    256,    256,    256,    256,},
    {256,    256,    256,    256,    256,    256,},
    {256,    256,    256,    256,    256,    256,},
    {256,    256,    256,    256,    256,    256,},
};

//aisp_snr_adj_t
static int16_t _CALIBRATION_SNR_ADJ[ISO_NUM_MAX][16] = {
/*weight|NP adj|cor_profile_adj|cor_profile_ofst|sad_wt_sum_th[2]|th_x0 x1 x2|var_flat_th_y[3]|sad_meta_ratio[4]*/
    { 72,    256,    8,    0,    1600,1024,    64,4,5,    32,44,48,    12,14,16,20,},
    { 80,    256,    8,    0,    1600,1024,    64,4,5,    32,44,48,    12,14,16,20,},
    { 96,    256,    8,    0,    1800,1024,    64,4,5,    32,44,48,    12,14,16,20,},
    {100,    256,    8,    0,    1800,1024,    64,4,5,    32,44,48,    12,14,16,20,},
    {100,    256,    10,    0,    1800,1024,    64,4,5,    32,44,48,    12,14,16,16,},
    {100,    256,    10,    0,    2000,1024,    64,7,8,    32,44,63,    8,10,12,16,},
    {100,    256,    10,    0,    2000,1536,    64,7,8,    32,48,63,    8,10,12,16,},
    {100,    256,    10,    4,    2100,2048,    64,7,8,    32,48,63,    8,10,12,16,},
    {100,    256,    10,    4,    2100,2048,    64,7,8,    32,48,63,    8,10,12,16,},
    {100,    256,    10,    6,    2100,2048,    64,7,8,    32,48,63,    8,10,12,16,},
};

//aisp_snr_t->snr_cur_wt
static uint16_t _CALIBRATION_SNR_CUR_WT[ISO_NUM_MAX][8] = {
    {255,280,320,360,400,400,460,500,},
    {255,280,320,360,400,400,460,500,},
    {255,280,320,360,400,400,460,500,},
    {255,280,320,360,400,400,460,500,},
    {255,280,320,360,400,400,460,500,},
    {255,280,320,360,400,400,460,500,},
    {255,280,320,360,400,400,400,400,},
    {255,280,300,300,300,300,300,300,},
    {255,280,300,300,300,300,300,300,},
    {255,280,300,300,300,300,300,300,},
};

//aisp_snr_t->snr_wt_luma_gain
static uint8_t _CALIBRATION_SNR_WT_LUMA_GAIN[ISO_NUM_MAX][8] = {
    {12,12,10,10,10,16,20,30,},
    {12,12,10,10,18,20,20,30,},
    {16,16,10,10,22,22,20,30,},
    {16,16,10,10,22,24,26,32,},
    {16,16,16,16,24,26,32,32,},
    {16,16,16,16,28,30,32,32,},
    {28,28,24,24,30,32,32,32,},
    {32,32,32,32,32,32,32,32,},
    {32,32,32,32,32,32,32,32,},
    {32,32,32,32,32,32,32,32,},
};

//aisp_snr_t->snr_sad_meta2alp
static uint8_t _CALIBRATION_SNR_SAD_META2ALP[ISO_NUM_MAX][8] = {
    {120,64,56,32,24,12,6,6,},
    {160,120,96,48,24,16,10,10,},
    {160,120,96,48,24,20,10,10,},
    {160,120,96,48,28,24,10,10,},
    {180,160,120,96,70,46,12,12,},
    {180,160,120,100,84,60,36,12,},
    {220,210,180,160,150,90,46,20,},
    {230,210,185,160,150,100,80,25,},
    {230,220,190,170,160,110,80,25,},
    {230,220,190,170,160,110,80,30,},
};

//aisp_snr_t->snr_meta_adj
static uint8_t _CALIBRATION_SNR_META_ADJ[ISO_NUM_MAX][8] = {
    {32,32,32,32,28,12,6,6,},
    {32,32,32,32,28,16,10,10,},
    {32,32,32,32,32,22,11,11,},
    {32,32,32,32,32,28,12,12,},
    {28,28,30,30,32,30,16,16,},
    {26,26,26,28,28,30,20,20,},
    {45,26,26,28,28,30,26,22,},
    {60,55,40,32,30,30,32,22,},
    {60,55,40,32,30,30,32,22,},
    {60,55,40,32,30,30,32,32,},
};

static uint8_t _CALIBRATION_SNR_PHS[4][24] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {1, 2, 1, 2, 2, 1, 2, 1, 1, 2, 2, 1, 2, 1, 1, 2, 1, 2, 1, 1, 2, 1, 2, 2,},
    {2, 1, 2, 1, 1, 2, 1, 2, 2, 1, 1, 2, 1, 2, 2, 1, 2, 1, 2, 2, 1, 2, 1, 1,},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,},
};

static uint8_t _CALIBRATION_NR_RAD_LUT65[3][65] = {
    {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 128,},
    {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 128,},
    {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 128, 128, 128, 128, 128,},
};

//aisp_psnr_adj_t
static uint16_t _CALIBRATION_PST_SNR_ADJ[ISO_NUM_MAX][2] = {
/* post NR Y | Post NR chroma*/
    {16,    16,},
    {20,    20,},
    {24,    24,},
    {28,    28,},
    {32,    32,},
    {48,    48,},
    {52,    52,},
    {56,    56,},
    {60,    60,},
    {63,    63,},
};

//aisp_tnr_ctl_t
static int32_t _CALIBRATION_TNR_CTL[29] = {
    36,5,5,    //reg_ma_mix_th_x[3]
    1,    //reg_rad_tnr0_en
    2,6,6,    //reg_ma_sad_pdtl4_x[3]
    2,8,12,    //reg_ma_sad_pdtl4_y[3]
    0,    //reg_ma_adp_dtl_mix_th_nfl
    4,8,12,16,    //reg_ma_sad_th_mask_gain[4]
    4,8,12,16,    //reg_ma_mix_th_mask_gain[4]
    64,    //reg_ma_mix_ratio
    70,300,700,1400,//reg_ma_sad_luma_adj_x[4];
    32,24,18,16,16//reg_ma_sad_luma_adj_y[5];
};

//aisp_tnr_glb_adj_t
static uint16_t _CALIBRATION_TNR_GLB_ADJ[ISO_NUM_MAX][4] = {
/*tnr_ma_mix_h_th_glb_adj|tnr_ma_np_lut16_glb_adj|tnr_ma_sad2alp_glb_adj|tnr_mc_meta2alp_glb_adj*/
    {256,    256,    256,    256,},
    {256,    256,    256,    256,},
    {256,    256,    256,    256,},
    {256,    256,    256,    256,},
    {256,    256,    256,    256,},
    {256,    256,    256,    256,},
    {256,    256,    256,    256,},
    {256,    256,    256,    256,},
    {256,    256,    256,    256,},
    {256,    256,    256,    256,},
};

//aisp_tnr_adj_t
static uint16_t _CALIBRATION_TNR_ADJ[ISO_NUM_MAX][26] = {
/*tnr_np_gain|tnr_np_ofst|ma_mix_h_th_gain[4]|reg_ma_mix_h_th_y|reg_ma_mix_l_th_y|reg_ma_sad_var_th_x_xx|reg_ma_sad_var_th_y_xx|me_sad_cor_np_gain|me_sad_cor_np_ofst|me_meta_sad_th0|reg_me_meta_sad_th1*/
    { 4,     0,    16,20,24,24,    20,30,40,    20,30,40,    50,4,5,    32,32,32,     0,    0,    2,5,10,    2,5,10,},
    {10,     0,    16,20,24,24,    20,30,40,    20,30,40,    50,4,5,    32,32,32,     0,    0,    2,5,10,    2,5,10,},
    {12,     0,    16,20,24,24,    20,30,40,    20,30,40,    50,4,5,    32,32,32,     0,    0,    2,5,10,    2,5,10,},
    {14,     0,    16,20,24,24,    20,30,40,    20,30,40,    50,4,5,    32,32,32,     0,    0,    2,5,10,    2,5,10,},
    {18,     0,    16,20,24,24,    20,30,40,    20,30,40,    50,4,5,    32,32,32,     0,    0,    2,5,10,    2,5,10,},
    {18,     0,    16,20,22,24,    30,30,40,    30,30,40,    50,4,5,    32,32,32,     0,    0,    2,5,10,    2,5,10,},
    {20,     0,    16,20,22,24,    30,40,50,    25,35,45,    50,4,5,    32,32,32,     0,    0,    2,5,10,    2,5,10,},
    {25,     8,    25,35,40,50,    35,40,50,    25,35,45,    40,4,5,    22,25,32,     0,    0,    2,5,10,    2,5,10,},
    {30,    12,    30,40,45,55,    75,75,75,    50,50,50,    40,4,5,    22,25,32,     0,    0,    2,5,10,    2,5,10,},
    {30,    12,    30,40,45,55,    75,75,75,    50,50,50,    40,4,5,    22,25,32,     0,    0,    2,5,10,    2,5,10,},
};

//aisp_tnr_t->nr_ma_sad2alpha
static uint8_t _CALIBRATION_TNR_SAD2ALPHA[ISO_NUM_MAX][64] = {
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 50,50,50,49,49,48,48,47, 55,55,55,55,55,55,55,55,},
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 56,56,56,55,55,54,54,53, 60,60,60,60,60,60,60,60,},
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 56,56,56,55,55,54,54,53, 60,60,60,60,60,60,60,60,},
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 56,56,56,55,55,54,54,53, 60,60,60,60,60,60,60,60,},
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 56,56,56,55,55,54,54,53, 60,60,60,60,60,60,60,60,},
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 56,56,56,55,55,54,54,53, 60,60,60,60,60,60,60,60,},
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 56,56,56,55,55,54,54,53, 60,60,60,60,60,60,60,60,},
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 56,56,56,55,55,54,54,53, 62,62,62,60,60,60,60,60,},
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 56,56,56,55,55,54,54,53, 62,62,62,60,60,60,60,60,},
    {9,9,7,7,5,5,4,3, 16,16,16,16,16,15,15,14, 19,19,19,18,18,18,18,17, 21,21,21,20,20,20,20,19, 46,45,44,43,40,40,40,40, 50,50,50,49,49,48,48,47, 56,56,56,55,55,54,54,53, 62,62,62,60,60,60,60,60,},
};

//aisp_tnr_t->tnr_mc_meta2alpha
static uint8_t _CALIBRATION_MC_META2ALPHA[ISO_NUM_MAX][64] = {
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
    {0,0,0,0,0,0,0,0, 8,7,6,5,5,5,5,5, 10,9,8,7,6,6,6,6, 12,10,10,9,9,8,7,7, 14,12,11,10,9,8,8,8, 16,14,13,12,10,9,9,9, 16,16,14,13,12,11,10,10, 16,16,16,15,14,13,12,11,},
};

//aisp_tnr_t->ptnr_alp_lut
static uint8_t _CALIBRATION_PST_TNR_ALP_LUT[ISO_NUM_MAX][8] = {
    {0,2,8,16,24,32,36,46,},
    {0,2,8,16,24,32,36,48,},
    {0,2,8,16,24,32,36,50,},
    {0,2,8,16,24,32,36,50,},
    {0,2,8,16,24,32,36,50,},
    {0,2,8,16,24,32,36,50,},
    {0,2,8,16,24,32,36,60,},
    {0,2,8,16,24,32,36,60,},
    {0,2,8,16,24,32,36,60,},
    {0,2,8,16,24,32,36,60,},
};

//aisp_lsc_adj_t
static uint16_t _CALIBRATION_LENS_SHADING_ADJ[ISO_NUM_MAX][2] = {
/* radial shding strength  | mesh shding strength*/
    {128, 128,}, //x1 gain
    {128, 128,}, //x2 gain
    {128, 128,}, //x4 gain
    {128, 128,}, //x8 gain
    {128, 128,}, //x16 gain
    {64,   64,}, //x32 gain
    {64,   64,}, //x64 gain
    {64,   64,}, //x128 gain
    {64,   64,}, //x256 gain
    {64,   64,}, //x512 gain
};

static int32_t _CALIBRATION_LENS_SHADING_CT_CORRECT[2] = {
/*    TL40 diff           |   CWF color diff */
    0, 0
};

//aisp_dms_t
static uint16_t _CALIBRATION_DMS_ADJ[ISO_NUM_MAX][2] = {
/*plp_alp | detail_non_dir_str*/
    {0,  5,},
    {0,  5,},
    {0,  5,},
    {0,  5,},
    {0,  5,},
    {0,  5,},
    {0, 10,},
    {0, 16,},
    {0, 16,},
    {0, 16,},
};

//aisp_ccm_t->ccm_str
static uint32_t _CALIBRATION_CCM_ADJ[ISO_NUM_MAX][1] = {
/* Color Correct strength */
    {128,}, //x1 gain
    {128,}, //x2 gain
    {128,}, //x4 gain
    {128,}, //x8 gain
    {128,}, //x16 gain
    {128,}, //x32 gain
    {110,}, //x64 gain
    {80,}, //x128 gain
    {80,}, //x256 gain
    {80,}, //x512 gain
};

//aisp_cnr_ctl_t
static int32_t _CALIBRATION_CNR_CTL[6] = {
    0,  //cnr_map_xthd
    16, //cnr_map_kappa
    15, //cnr_map_ythd0
    15, //cnr_map_ythd1
    12, //cnr_map_norm
    16, //cnr_map_str
};

//aisp_cnr_adj_t
static uint16_t _CALIBRATION_CNR_ADJ[ISO_NUM_MAX][6] = {
/*cnr_weight|umargin_up|umargin_dw|vmargin_up|vmargin_dw| alp_mode*/
    {48,    16,    16,    16,    16,    2,},
    {48,    16,    16,    16,    16,    2,},
    {48,    16,    16,    16,    16,    2,},
    {48,    16,    16,    16,    16,    2,},
    {48,    16,    16,    16,    16,    2,},
    {48,    16,    16,    16,    16,    2,},
    {48,    16,    16,    16,    16,    2,},
    {48,    16,    16,    16,    16,    2,},
    {48,    16,    16,    16,    16,    2,},
    {48,    16,    16,    16,    16,    2,},
};

//aisp_purple_ctl_t
static int32_t _CALIBRATION_PURPLE_CTL[2] = {
    120,//purple_luma_osat_thd
    0,// pfr_mode
};

//aisp_purple_adj_t
static uint16_t _CALIBRATION_PURPLE_ADJ[ISO_NUM_MAX][7] = {
/*purple_weight|purple_umargin_up|purple_umargin_dw|purple_vmargin_up|purple_vmargin_dw| purple_cst_thd|purple_desat_en|*/
    {64,    16,    16,    16,    16,    5,    2,},
    {64,    16,    16,    16,    16,    5,    2,},
    {64,    16,    16,    16,    16,    5,    2,},
    {64,    16,    16,    16,    16,    5,    2,},
    {64,    16,    16,    16,    16,    5,    2,},
    {64,    16,    16,    16,    16,    5,    2,},
    {64,    16,    16,    16,    16,    5,    2,},
    {64,    16,    16,    16,    16,    5,    2,},
    {64,    16,    16,    16,    16,    5,    2,},
    {64,    16,    16,    16,    16,    5,    2,},
};

//aisp_ltm_t
static int32_t _CALIBRATION_LTM_CTL[13] = {
    1,       //ltm_auto_en
    8,       //ltm_damper64
    56,       //ltm_lmin_alpha
    32,       //ltm_lmax_alpha
    64,  //ltm_hi_gm_u7
    20,  //ltm_lo_gm_u6
    1,  //ltm_dtl_ehn_en
    63,  //ltm_vs_gtm_alpha
    0,  //ltm_cc_en
    1,  //ltm_lmin_med_en
    1,  //ltm_lmax_med_en
    1,  //ltm_bld_lvl_adp_en
    0,  //ltm_lo_hi_gm_auto
};

static int32_t _CALIBRATION_LTM_LO_HI_GM[ISO_NUM_MAX][2] = {
/* ltm_lo_gm_u6 | ltm_hi_gm_u7 */
    {15, 72,},
    {16, 72,},
    {17, 72,},
    {19, 72,},
    {19, 72,},
    {21, 72,},
    {21, 72,},
    {21, 72,},
    {21, 72,},
    {21, 72,},
};

static int32_t _CALIBRATION_LTM_CONTRAST[ISO_NUM_MAX] = {
/* u9, only for WDR mode*/
    344,
    400,
    400,
    400,
    400,
    428,
    428,
    456,
    456,
    484,
};

//aisp_sharpen_ltm_t
static int32_t _CALIBRATION_LTM_SHARP_ADJ[ISO_NUM_MAX][4] = {
/* alpha | shrp_r_u6 | shrp_s_u8 | shrp_smth_lvlsft */
    {32,    16,    128,     7,},
    {24,    16,    96,      7,},
    {24,    16,    64,      7,},
    {20,    16,    32,      7,},
    {16,    16,    32,      7,},
    {16,    16,    24,      7,},
    { 8,    16,     8,      7,},
    { 8,    16,     8,      7,},
    { 8,    16,     8,      7,},
    { 8,    16,     8,      7,},
};

static uint16_t _CALIBRATION_LTM_SATUR_LUT[63] = {
    11,   12,   13,   83,   90,   98,  106,  115,  125,  136,  148,  161,  175,  191,  208,  227,
    248,  271,  296,  322,  352,  383,  417,  454,  494,  536,  581,  630,  682,  737,  795,  858,
    923,  993, 1066, 1142, 1221, 1304, 1390, 1478, 1569, 1663, 1760, 1858, 1960, 2063, 2168, 2275,
    2384, 2495, 2607, 2721, 2836, 2952, 3069, 3187, 3306, 3426, 3546, 3667, 3788, 3910, 4018
};

//aisp_lc_t
static int32_t _CALIBRATION_LC_CTL[9] = {
    1,  //lc_auto_enable
    1, //lc_blkblend_mode
    6, //lc_lmtrat_minmax
    48, //lc_contrast_low u8
    32, //lc_contrast_hig u8
    0,  //lc_cc_en
    63,  //lc_ypkbv_slope_lmt[1]
    32,  //lc_ypkbv_slope_lmt[0]
    0,  //lc_str_fixed
};

static int32_t _CALIBRATION_LC_STRENGTH[ISO_NUM_MAX][2] = {
/* lc_ypkbv_slope_lmt_0, lc_ypkbv_slope_lmt_1 */
    {64, 96,},
    {64, 96,},
    {64, 96,},
    {64, 96,},
    {64, 96,},
    {48, 96,},
    {48, 96,},
    {48, 96,},
    {48, 96,},
    {48, 96,},
};

static uint16_t _CALIBRATION_LC_SATUR_LUT[63] = {
    12,   36,   55,   83,   90,   98,  106,  115,  125,  136,  148,  161,  175,  191,  208,  227,
    248,  271,  296,  322,  352,  383,  417,  454,  494,  536,  581,  630,  682,  737,  795,  858,
    923,  993, 1066, 1142, 1221, 1304, 1390, 1478, 1569, 1663, 1760, 1858, 1960, 2063, 2168, 2275,
    2384, 2495, 2607, 2721, 2836, 2952, 3069, 3187, 3306, 3426, 3546, 3667, 3788, 3910, 4018
};

//aisp_dnlp_t
static int32_t _CALIBRATION_DNLP_CTL[21] = {
    1,   // dnlp_auto_enable
    5,   // dnlp_cuvbld_min
    15,  // dnlp_cuvbld_max
    0,   // dnlp_clashBgn
    64,  // dnlp_clashEnd
    6,   // dnlp_blkext_ofst
    5,  // dnlp_whtext_ofst
    32,  // dnlp_blkext_rate
    64,  // dnlp_whtext_rate
    0,   // dnlp_dbg_map
    8,  // dnlp_final_gain
    20,  // dnlp_scurv_low_th
    70,  // dnlp_scurv_mid1_th
    100, // dnlp_scurv_mid2_th
    130, // dnlp_scurv_hgh1_th
    160, // dnlp_scurv_hgh2_th
    0,   // dnlp_mtdrate_adp_en
    1,   // dnlp_ble_en
    48,  // dnlp_scn_chg_th
    32, // dnlp_mtdbld_rate
    0,  //dnlp_str_fixed
};

//CALIBRATION_DNLP_STRENGTH, 8bit, normalization: 8 as 1, {ISO100,ISO200,ISO400,ISO800,ISO1600,ISO3200,ISO6400,ISO12800,ISO25600,ISO51200}
static int32_t _CALIBRATION_DNLP_STRENGTH[ISO_NUM_MAX] = {12, 12, 10, 8, 8, 8, 8, 8, 8, 8};

static uint16_t _CALIBRATION_DNLP_SCURV_LOW[65] = {
    0, 13, 27, 41, 55, 69, 84, 99, 114, 129, 145, 162, 179, 197, 215, 233, 252, 272, 291, 310, 329,
    348, 367, 386, 405, 424, 442, 461, 479, 497, 515, 533, 550, 567, 584, 601, 617, 633, 650, 665,
    681, 697, 712, 727, 742, 757, 772, 787, 801, 816, 830, 844, 858, 872, 886, 900, 914, 927, 941, 955, 968, 982, 995, 1009, 1023
};

static uint16_t _CALIBRATION_DNLP_SCURV_MID1[65] = {
    0, 13, 26, 39, 52, 65, 79, 93, 107, 121, 136, 150, 166, 181, 197, 214, 231, 249, 267, 286, 305,
    325, 346, 368, 390, 408, 426, 444, 462, 480, 498, 516, 534, 551, 569, 586, 602, 619, 636, 652,
    668, 684, 700, 716, 731, 747, 762, 777, 792, 807, 822, 837, 851, 866, 880, 895, 909, 923, 938, 952, 966, 980, 994, 1008, 1023
};

static uint16_t _CALIBRATION_DNLP_SCURV_MID2[65] = {
    0, 11, 23, 34, 46, 58, 70, 83, 95, 108, 121, 135, 149, 164, 178, 194, 210, 226, 244, 262, 280,
    300, 320, 341, 362, 385, 408, 431, 454, 472, 490, 508, 525, 543, 560, 578, 595, 611, 628, 645,
    661, 677, 693, 709, 725, 740, 756, 771, 787, 802, 817, 832, 847, 862, 877, 892, 906, 921, 935, 950, 965, 979, 994, 1008, 1023
};

static uint16_t _CALIBRATION_DNLP_SCURV_HGH1[65] = {
    0, 9, 18, 28, 37, 47, 57, 68, 78, 90, 102, 114, 127, 140, 155, 170, 185, 202, 220, 238, 256,
    275, 293, 312, 331, 349, 368, 387, 405, 423, 442, 460, 478, 496, 514, 532, 549, 567, 585, 602,
    619, 637, 654, 671, 688, 705, 723, 739, 756, 773, 790, 807, 824, 840, 857, 874, 890, 907, 923, 940, 956, 973, 989, 1006, 1023
};

static uint16_t _CALIBRATION_DNLP_SCURV_HGH2[65] = {
    0, 7, 14, 21, 29, 37, 45, 54, 64, 74, 85, 97, 110, 123, 138, 154, 172, 191, 210, 229, 245, 262,
    279, 297, 316, 334, 352, 370, 388, 406, 424, 442, 460, 478, 496, 514, 531, 549, 567, 585, 602,
    620, 638, 655, 673, 691, 708, 726, 743, 761, 778, 796, 813, 831, 848, 866, 883, 901, 918, 935, 953, 970, 988, 1005, 1023
};

//aisp_dhz_t
static int32_t _CALIBRATION_DHZ_CTL[10] = {
    1,   //dhz_auto_enable
    750,  //dhz_dlt_rat
    256,  //dhz_hig_dlt_rat
    256,  //dhz_low_dlt_rat
    8,  //dhz_lmtrat_lowc
    8,  //dhz_lmtrat_higc
    0,  //dhz_cc_en
    0,  //dhz_sky_prot_en
    614,  //dhz_sky_prot_stre
    0,   //dhz_str_fixed
};

//CALIBRATION_DHZ_STRENGTH, 16bit, normalization: 1024 as 1, {ISO100,ISO200,ISO400,ISO800,ISO1600,ISO3200,ISO6400,ISO12800,ISO25600,ISO51200}
static int32_t _CALIBRATION_DHZ_STRENGTH[ISO_NUM_MAX] = {1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024};

//aisp_sharpen_ctl_t
static int32_t _CALIBRATION_PEAKING_CTL[52] = {
    //pk_flt1_v1d[3]
    120, -60, 0,
    //pk_flt2_v1d[3]
    120, -60, 0,
    //pk_flt1_h1d[5]
    120, -60, 0, 0, 0,
    //pk_flt2_h1d[5]
    120, -60, 0, 0, 0,
    //pkosht_vsluma_lut[9]
    4, 4, 3, 3, 3, 3, 2, 1, 0,
    //pk_flt1_2d[3][4]
    124,    70,     -29,    0,
    70,     -37,    -16,    0,
    -29,    -16,    -3,     0,
    //pk_flt2_2d[3][4]
    124,    70,     -29,    0,
    70,     -37,    -16,    0,
    -29,    -16,    -3,     0,
    /*124,    -11,     -5,    0,
    -11,     -11,    -2,    0,
    -5,    -2,    0,     0,*/
    1, //pk_motion_adp_gain_en   0:disable  1:enable
    1,  // pk_dejaggy_en         0:disable  1:enable
    0,  // pk_debug_mode  0:disable debug  18:dir flt1 gain 19:dir flt2 gain 20: cir flt1 gain 21: cir flt2 gain 22:gradinfo
};

//aisp_sharpen_adj_t
static uint16_t _CALIBRATION_PEAKING_ADJUST[ISO_NUM_MAX][6] = {
/*flt1_final_gain|flt2_final_gain|pre_flt_str|os_up|os_dw|pre_flt_range*/
    {40,    40,    12,    80,    120,    64,},
    {40,    40,    16,    80,    120,    32,},
    {40,    40,    20,    80,    120,    24,},
    {32,    32,    24,    80,    120,    20,},
    {32,    32,    32,    80,    120,    12,},
    {32,    32,    40,    50,    100,    4,},
    {16,    16,    48,    40,    70,     0,},
    {16,    16,    54,    40,    60,     0,},
    {16,    16,    60,    10,    30,     0,},
    {16,    16,    60,    10,    30,     0,},
};

//aisp_sharpen_t->peaking_flt1_gain_adp_motion
static uint8_t _CALIBRATION_PEAKING_FLT1_MOTION_ADP_GAIN[ISO_NUM_MAX][8] = {
    {22, 42, 42, 42, 42, 42, 42, 42,},
    {22, 42, 42, 42, 42, 42, 42, 42,},
    {18, 36, 42, 42, 42, 42, 42, 42,},
    {14, 28, 36, 42, 42, 42, 42, 42,},
    {12, 26, 30, 36, 42, 42, 42, 42,},
    {10, 24, 26, 28, 32, 36, 36, 36,},
    {8, 22, 24, 26, 26, 26, 26, 32,},
    {8, 20, 22, 22, 22, 22, 22, 22,},
    {8, 15, 18, 18, 18, 18, 18, 18,},
    {8, 10, 10, 10, 10, 10, 10, 10,},
};

//aisp_sharpen_t->peaking_flt2_gain_adp_motion
static uint8_t _CALIBRATION_PEAKING_FLT2_MOTION_ADP_GAIN[ISO_NUM_MAX][8] = {
    {22, 42, 42, 42, 42, 42, 42, 42,},
    {22, 42, 42, 42, 42, 42, 42, 42,},
    {18, 36, 42, 42, 42, 42, 42, 42,},
    {14, 28, 36, 42, 42, 42, 42, 42,},
    {12, 26, 30, 36, 42, 42, 42, 42,},
    {10, 24, 26, 28, 32, 36, 36, 36,},
    {8, 22, 24, 26, 26, 26, 26, 32,},
    {8, 20, 22, 22, 22, 22, 22, 22,},
    {8, 15, 18, 18, 18, 18, 18, 18,},
    {8, 10, 10, 10, 10, 10, 10, 10,},
};

//aisp_sharpen_t->peaking_gain_adp_luma
static uint8_t _CALIBRATION_PEAKING_GAIN_VS_LUMA_LUT[ISO_NUM_MAX][9] = {
    {4, 4, 4, 5, 5, 5, 5, 4, 3,},
    {4, 4, 4, 4, 5, 5, 5, 4, 3,},
    {3, 3, 4, 4, 4, 5, 5, 4, 2,},
    {3, 3, 4, 4, 4, 4, 5, 4, 2,},
    {2, 3, 4, 4, 4, 4, 4, 4, 2,},
    {1, 3, 3, 4, 4, 4, 4, 3, 2,},
    {0, 2, 3, 4, 4, 4, 4, 2, 2,},
    {0, 1, 2, 4, 4, 4, 4, 2, 2,},
    {0, 1, 2, 4, 4, 4, 4, 2, 2,},
    {0, 1, 2, 4, 4, 4, 4, 2, 2,},
};

//aisp_sharpen_t->peaking_gain_adp_grad1
static uint8_t _CALIBRATION_PEAKING_CIR_FLT1_GAIN[ISO_NUM_MAX][5] = {
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
};

//aisp_sharpen_t->peaking_gain_adp_grad2
static uint8_t _CALIBRATION_PEAKING_CIR_FLT2_GAIN[ISO_NUM_MAX][5] = {
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
    {20, 65, 30, 18, 120,},
};

//aisp_sharpen_t->peaking_gain_adp_grad3
static uint8_t _CALIBRATION_PEAKING_DRT_FLT1_GAIN[ISO_NUM_MAX][5] = {
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {25, 100, 20, 15, 100,},
    {20, 100, 15, 15, 100,},
    {20, 100, 10, 15, 100,},
    {20, 100, 10, 15, 100,},
};

//aisp_sharpen_t->peaking_gain_adp_grad4
static uint8_t _CALIBRATION_PEAKING_DRT_FLT2_GAIN[ISO_NUM_MAX][5] = {
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {30, 100, 20, 15, 100,},
    {25, 100, 20, 15, 100,},
    {20, 100, 15, 15, 100,},
    {20, 100, 10, 15, 100,},
    {20, 100, 10, 15, 100,},
};

//aisp_cm_ctl_t
static int32_t _CALIBRATION_CM_CTL[4] = {
    //cm_sat
    512,
    //cm_hue
    0,
    //cm_contrast
    1024,
    //cm_brightness
    0,
};

//aisp_cm_t->cm_y_via_hue
static int8_t _CALIBRATION_CM_Y_VIA_HUE[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

//aisp_cm_t->cm_satglbgain_via_y
static int8_t _CALIBRATION_CM_SATGLBGAIN_VIA_Y[ISO_NUM_MAX][9] = {
    {0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,},
    {-60,-40,-20,0,0,0,0,0,0,},
    {-60,-40,-20,0,0,0,0,0,0,},
    {-60,-40,-20,0,0,0,0,0,0,},
    {-60,-40,-20,0,0,0,0,0,0,},
    {-60,-40,-20,0,0,0,0,0,0,},
};

//aisp_cm_t->cm_sat_via_hs
static int8_t _CALIBRATION_CM_SAT_VIA_HS[3][32] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
};

//aisp_cm_t->cm_satgain_via_y
static int8_t _CALIBRATION_CM_SATGAIN_VIA_Y[5][32] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
};

//aisp_cm_t->cm_hue_via_h
static int8_t _CALIBRATION_CM_HUE_VIA_H[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

//aisp_cm_t->cm_hue_via_s
static int8_t _CALIBRATION_CM_HUE_VIA_S[5][32] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
};

//aisp_cm_t->cm_hue_via_y
static int8_t _CALIBRATION_CM_HUE_VIA_Y[5][32] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
};

//aisp_hlc_t
static int32_t _CALIBRATION_HLC_CTL[3] = {
    0,      //hlc_en
    240,    //hlc_luma_thd
    240,    //hlc_luma_trgt
};

static int32_t _CALIBRATION_BLACK_LEVEL[9][5] =
{
    {61440,61440,61440,61440,61440,},
    {61440,61440,61440,61440,61440,},
    {61440,61440,61440,61440,61440,},
    {61440,61440,61440,61440,61440,},
    {61440,61440,61440,61440,61440,},
    {61440,61440,61440,61440,61440,},
    {61440,61440,61440,61440,61440,},
    {61440,61440,61440,61440,61440,},
    {61440,61440,61440,61440,61440,},
};

static uint16_t _CALIBRATION_SHADING_RADIAL_R[129]=
{
4096,4104,4096,4116,4111,4112,4111,4114,4126,4129,4126,4141,4142,4160,4158,
4170,4179,4187,4202,4210,4224,4241,4244,4258,4272,4284,4299,4313,4328,4341,
4361,4374,4389,4412,4426,4444,4466,4489,4510,4535,4559,4581,4606,4632,4655,
4683,4708,4738,4763,4796,4819,4850,4882,4915,4947,4978,5011,5048,5083,5118,
5157,5190,5233,5271,5307,5351,5387,5427,5473,5513,5554,5602,5642,5694,5743,
5791,5844,5891,5946,5998,6058,6113,6169,6235,6279,6348,6407,6472,6537,6602,
6675,6749,6835,6925,7010,7094,7177,7270,7367,7461,7547,7653,7750,7849,7949,
8051,8175,8274,8376,8500,8621,8755,8849,8988,9134,9241,9374,9539,9684,9841,
9971,10144,10335,10500,10690,10796,11041,11088,11311
};

static uint16_t _CALIBRATION_SHADING_RADIAL_G[129]=
{
4096,4096,4112,4101,4111,4112,4115,4119,4121,4131,4136,4140,4146,4153,4163,
4173,4179,4189,4196,4207,4220,4231,4239,4250,4262,4276,4288,4302,4315,4330,
4345,4360,4378,4390,4408,4426,4447,4467,4484,4506,4528,4546,4570,4593,4618,
4644,4667,4693,4718,4746,4773,4802,4829,4857,4885,4918,4948,4981,5012,5046,
5081,5116,5148,5185,5222,5258,5294,5335,5373,5410,5452,5488,5533,5574,5617,
5665,5712,5760,5808,5856,5907,5960,6008,6064,6117,6175,6230,6286,6346,6410,
6467,6536,6615,6691,6775,6855,6929,7008,7091,7180,7262,7344,7434,7523,7617,
7712,7807,7897,8003,8112,8211,8321,8421,8550,8659,8781,8901,9023,9162,9283,
9422,9571,9716,9851,10014,10162,10328,10530,10647
};

static uint16_t _CALIBRATION_SHADING_RADIAL_B[129]=
{
4096,4096,4119,4111,4098,4104,4104,4114,4122,4120,4133,4143,4144,4140,4151,
4157,4171,4180,4188,4207,4200,4218,4228,4236,4254,4258,4276,4287,4305,4321,
4330,4349,4358,4376,4395,4417,4440,4455,4474,4494,4516,4537,4559,4578,4600,
4623,4649,4679,4699,4734,4754,4788,4810,4832,4867,4897,4926,4956,4989,5018,
5051,5096,5123,5151,5185,5227,5260,5300,5334,5378,5409,5451,5491,5528,5577,
5616,5665,5710,5758,5799,5854,5906,5954,6010,6055,6115,6166,6220,6280,6336,
6393,6452,6529,6616,6686,6762,6832,6917,6994,7074,7153,7236,7317,7406,7493,
7584,7678,7778,7870,7972,8073,8167,8280,8381,8499,8603,8723,8824,8955,9082,
9215,9362,9513,9621,9828,9927,10056,10213,10340
};

static uint8_t _CALIBRATION_SHADING_LS_D65_R[1024]=
{
160,144,132,122,114,107,101,96,91,88,86,83,82,80,80,80,79,80,79,81,83,85,87,90,94,99,104,110,118,127,139,153,155,141,128,119,111,104,99,94,90,86,84,82,80,79,78,77,77,77,78,80,81,83,86,89,93,96,102,108,115,124,135,149,150,136,125,116,108,102,97,92,88,85,82,79,78,76,76,75,76,76,76,77,80,81,83,86,90,95,100,106,112,121,131,144,146,133,122,113,106,101,95,90,86,83,80,78,76,75,74,74,74,74,75,76,77,80,82,85,89,92,97,103,109,118,128,140,142,130,119,110,104,98,93,88,84,81,79,76,75,73,72,73,73,73,73,74,75,78,80,83,86,90,96,101,107,115,125,137,140,128,117,109,102,96,91,86,83,79,77,75,73,72,72,71,71,71,72,73,75,76,79,82,85,89,93,99,106,112,123,134,138,124,114,107,100,94,90,85,81,78,76,74,72,71,70,69,69,69,70,72,73,75,77,80,83,87,92,98,103,111,120,131,135,123,113,105,99,93,88,83,80,77,75,72,71,69,69,69,69,69,69,70,72,74,76,79,83,86,91,96,103,109,118,129,134,121,111,103,97,92,86,83,79,76,74,72,70,69,68,67,67,68,69,69,71,73,76,78,81,85,89,95,101,108,116,126,131,120,110,103,96,91,86,82,78,75,72,71,69,68,67,66,66,67,68,69,70,73,74,77,80,84,89,93,99,106,115,125,130,118,109,102,96,90,85,81,77,74,72,70,69,67,66,66,66,66,67,68,69,72,73,76,79,83,88,93,99,106,114,123,129,117,107,100,94,89,84,80,76,74,71,69,68,66,66,66,66,66,66,67,69,71,73,75,79,82,87,92,98,104,112,122,128,117,107,100,93,88,83,79,76,73,71,69,67,67,66,65,65,65,66,67,69,70,72,75,79,82,86,91,97,103,112,121,127,116,107,100,94,88,83,80,76,73,70,69,67,66,65,64,64,65,66,66,68,70,73,75,78,82,86,91,96,103,111,121,127,115,106,99,94,87,82,80,76,72,70,68,66,66,65,64,64,64,66,66,68,69,72,75,78,82,87,91,96,102,111,120,126,115,106,100,94,87,83,79,76,72,70,68,66,66,65,64,64,64,66,66,68,69,72,75,78,81,86,90,95,102,110,120,126,115,106,99,93,87,82,79,76,72,70,68,66,66,65,64,64,64,65,66,68,69,72,74,78,81,86,90,96,102,110,119,127,116,106,100,94,88,83,79,76,72,70,69,66,66,65,64,64,64,66,66,68,69,72,75,78,81,86,90,96,102,110,120,128,116,107,100,93,88,83,80,76,73,70,69,67,66,65,65,64,65,66,66,68,70,72,75,78,82,86,91,96,103,111,120,128,117,107,100,94,89,84,79,76,73,71,69,67,67,66,65,65,65,66,66,69,70,72,75,78,82,86,91,97,103,111,121,129,118,107,101,95,90,84,80,76,74,71,69,68,66,66,66,65,66,66,67,69,70,72,75,79,83,87,92,97,103,112,121,130,119,109,101,96,90,85,81,77,74,72,70,69,67,66,66,66,67,67,68,69,71,73,76,79,83,87,92,98,104,113,123,131,120,110,103,97,91,86,82,78,75,72,71,70,68,67,66,66,66,67,69,70,72,74,77,80,83,88,93,99,105,114,125,133,121,111,104,97,92,86,83,79,76,73,72,70,69,68,67,67,68,69,69,71,72,75,78,81,85,89,95,100,107,115,126,135,123,113,105,99,93,88,84,80,77,75,72,71,70,69,68,69,69,69,70,72,74,76,79,82,85,90,95,102,109,117,128,138,125,115,107,100,95,89,85,81,78,76,74,72,71,70,69,69,69,70,72,73,75,77,80,83,87,92,97,103,110,119,130,140,127,118,109,102,97,91,86,83,80,77,75,73,72,71,71,70,71,72,72,74,76,79,81,85,89,93,98,105,112,121,132,143,130,120,110,104,98,93,88,84,81,78,76,75,73,72,72,72,73,73,74,76,77,80,82,85,90,95,100,106,114,124,135,146,133,122,114,107,100,94,89,86,83,79,78,76,75,74,73,73,74,74,75,77,79,81,84,88,92,96,102,109,116,126,138,150,136,125,116,108,102,97,92,87,85,82,80,78,76,75,75,75,75,75,77,79,80,83,85,89,93,98,104,111,119,129,141,155,141,128,119,111,104,99,94,89,86,84,82,79,79,77,76,76,77,78,79,80,83,85,88,92,95,101,107,113,122,133,146,160,144,133,122,114,106,101,96,92,88,86,83,82,80,79,78,78,79,79,81,83,85,86,89,93,98,103,109,116,125,138,151
};

static uint8_t _CALIBRATION_SHADING_LS_D65_G[1024]=
{
154,140,128,119,111,105,99,95,91,87,85,82,81,79,79,79,78,79,79,80,82,84,86,89,93,97,102,108,115,124,134,147,149,136,125,116,109,102,97,92,89,86,83,81,79,78,77,76,76,77,78,79,80,82,85,88,91,95,100,105,112,121,131,143,145,132,122,113,106,100,95,90,87,84,81,79,78,76,76,75,75,75,76,77,79,80,83,85,89,93,98,103,110,118,128,140,141,129,119,110,104,98,93,89,85,82,79,77,76,75,74,73,73,73,74,76,77,79,81,84,87,92,96,102,108,115,124,136,138,126,116,108,102,96,91,87,83,80,78,76,74,73,72,72,72,72,72,74,75,77,79,82,85,90,94,99,105,112,122,133,135,123,114,106,100,95,89,86,82,79,76,75,73,72,71,70,70,71,72,72,74,75,79,81,84,88,92,98,104,111,119,130,132,122,112,105,99,93,88,84,81,78,76,73,72,70,69,69,69,69,70,71,72,75,77,79,83,86,91,96,102,108,118,128,130,119,110,103,97,92,87,83,79,76,74,72,71,69,69,68,68,69,69,70,72,73,75,79,82,85,90,95,101,107,115,126,129,118,109,102,96,91,86,82,79,76,73,71,69,69,68,67,67,67,68,69,71,72,75,78,81,85,88,94,99,105,114,124,128,116,107,101,95,89,85,81,77,75,72,70,69,68,67,66,66,66,67,69,70,72,74,77,80,83,88,92,98,105,113,123,126,116,106,99,94,89,84,80,77,74,72,69,68,67,66,66,66,66,66,68,69,71,73,76,79,83,87,92,98,104,111,121,126,115,106,99,93,88,83,79,76,73,71,69,67,66,66,65,65,66,66,67,69,70,72,75,79,82,86,91,96,102,111,120,125,113,105,98,92,87,83,79,76,73,71,69,67,66,65,65,65,65,66,66,68,70,72,75,78,82,85,91,96,102,110,119,124,113,104,98,92,87,82,79,76,72,70,68,67,66,65,64,64,65,66,66,68,69,72,75,78,81,85,90,95,101,109,118,123,112,104,97,92,86,82,79,76,72,70,68,66,66,65,64,64,64,65,66,68,69,72,74,77,81,85,90,95,101,109,118,123,112,104,97,92,86,82,79,75,72,70,68,66,66,64,64,64,64,65,66,67,69,72,74,77,81,85,89,95,101,109,118,123,112,104,97,92,86,82,79,75,72,70,68,66,66,64,64,64,64,65,66,67,69,72,74,77,81,85,89,95,101,108,117,123,112,104,98,92,86,82,79,76,72,70,68,66,66,65,64,64,64,65,66,67,69,72,74,77,81,85,89,95,101,108,117,123,113,104,98,92,87,82,79,76,72,70,68,66,66,65,64,64,65,66,66,68,69,72,75,77,81,85,90,95,101,109,118,124,114,105,99,92,87,83,79,76,73,71,69,67,66,65,65,65,65,66,66,68,70,72,75,78,82,85,90,95,101,109,119,125,114,106,99,93,88,83,79,76,73,71,69,67,66,66,65,65,66,66,67,69,70,72,75,79,82,86,91,96,102,110,120,126,116,106,99,94,89,84,80,77,74,72,69,68,67,66,66,66,66,66,68,69,71,73,75,79,82,87,92,97,103,111,120,127,116,108,101,95,89,85,81,77,75,72,70,69,68,66,66,66,66,67,69,69,72,74,76,79,83,88,92,98,104,112,121,129,118,109,102,96,90,86,82,79,76,73,71,69,69,68,67,67,67,68,69,71,72,75,77,81,84,88,93,99,105,114,124,131,119,110,103,97,92,87,83,79,76,74,72,71,69,69,68,68,69,69,70,72,73,75,79,82,85,89,95,101,107,115,125,133,122,112,105,99,93,89,84,81,78,76,73,72,70,69,69,69,69,70,71,72,75,77,79,82,86,91,96,101,108,117,127,136,124,114,106,100,95,89,85,82,79,76,75,73,72,71,70,70,71,72,72,74,75,78,81,84,88,92,98,103,110,119,129,138,126,117,109,102,96,91,87,83,80,78,76,74,73,72,72,72,72,72,74,75,77,79,82,85,89,94,99,105,112,121,131,142,129,119,111,104,98,93,89,85,82,79,77,75,75,73,73,73,73,74,75,76,79,81,84,87,91,95,101,107,114,123,134,145,132,122,113,106,100,95,90,87,84,81,79,77,76,75,75,75,75,75,77,78,80,82,85,88,93,98,103,109,117,127,138,149,136,126,116,109,102,97,92,89,85,83,81,79,78,77,76,76,76,77,79,80,82,85,87,91,95,99,105,112,120,130,142,154,140,129,119,111,105,99,95,91,87,85,82,81,79,79,78,78,78,79,80,82,84,86,89,92,96,101,107,114,123,133,146
};

static uint8_t _CALIBRATION_SHADING_LS_D65_B[1024]=
{
152,138,126,117,109,103,98,93,90,87,84,81,80,78,78,78,78,79,79,80,82,83,85,88,92,96,101,107,114,122,133,145,148,134,123,114,108,102,96,92,88,86,82,80,78,77,77,76,76,76,77,79,79,81,84,87,90,94,99,104,110,120,129,141,143,131,120,111,105,99,95,90,86,83,81,78,77,75,75,74,74,75,76,76,78,80,82,85,88,92,97,102,108,117,126,138,139,127,118,109,103,97,92,88,85,82,79,77,76,74,73,73,72,73,74,75,76,78,81,83,86,91,94,101,106,114,124,134,136,125,115,107,101,95,91,86,83,80,78,75,74,72,71,72,72,72,72,73,75,77,79,81,85,88,93,98,104,111,120,131,134,122,113,105,99,94,89,85,82,78,76,74,72,71,70,70,70,70,71,71,73,75,78,80,83,87,91,97,102,110,118,128,132,121,111,104,98,92,88,83,80,77,75,73,72,70,69,68,68,69,69,71,72,74,76,79,82,86,90,95,100,107,116,127,130,118,109,102,96,91,86,82,78,76,74,71,70,69,69,68,68,68,68,70,72,73,75,78,81,85,89,95,100,106,114,124,128,117,108,101,96,90,86,81,78,76,73,71,69,68,67,67,67,67,68,68,70,72,75,77,80,84,88,93,98,104,113,123,126,115,107,99,94,89,85,81,77,74,72,70,69,67,66,66,66,66,67,68,69,72,73,76,79,83,87,92,98,104,112,121,125,115,105,98,93,88,83,79,76,74,71,69,68,67,65,66,66,65,66,67,68,71,73,75,78,82,86,92,97,103,111,120,124,114,105,98,92,87,83,79,76,73,71,68,67,66,66,65,65,66,65,67,69,70,72,75,78,81,85,91,95,102,110,119,123,112,104,98,92,87,82,78,76,72,70,69,67,66,65,65,65,65,66,66,68,69,71,75,78,81,85,90,95,101,109,118,123,112,103,97,92,86,82,79,75,72,70,68,66,66,65,64,64,64,65,66,67,69,72,74,77,81,84,89,94,101,108,117,122,112,103,97,91,86,82,78,75,71,69,68,66,65,64,64,64,64,65,66,67,69,72,74,77,80,85,89,94,101,108,116,121,112,103,97,91,86,82,78,75,71,69,68,66,65,64,64,64,64,65,65,67,68,71,74,77,80,84,89,94,101,107,116,121,111,103,97,91,86,81,78,75,71,69,68,66,65,64,64,64,64,65,65,67,68,71,74,77,80,85,89,94,101,108,117,121,112,103,97,91,86,81,78,75,71,69,68,66,66,64,64,64,64,65,66,67,68,71,74,77,81,85,89,94,101,108,117,122,111,104,97,92,86,82,79,75,72,70,68,66,66,65,64,64,65,66,66,67,69,72,74,77,81,84,89,94,101,108,117,122,112,105,98,92,87,82,78,76,72,70,69,67,66,65,65,65,65,66,66,68,69,71,75,77,81,84,89,95,101,109,117,124,114,105,98,92,87,83,79,75,73,71,68,67,66,66,65,65,66,65,67,69,70,72,75,78,82,85,90,95,102,109,118,125,114,105,98,93,88,84,80,76,74,72,69,68,66,65,66,66,66,66,67,68,71,72,75,79,82,86,91,96,102,111,120,126,115,107,100,94,88,85,80,77,74,71,70,69,67,66,66,66,66,67,68,69,72,74,76,79,83,87,91,97,104,111,121,129,117,108,101,96,90,86,82,78,75,73,71,69,68,67,67,67,67,68,68,70,72,75,77,80,84,88,93,98,104,113,123,130,118,109,102,96,91,86,82,79,76,74,71,70,69,69,68,68,68,68,70,72,73,75,78,81,84,89,95,100,106,114,124,132,121,111,104,98,92,88,83,80,77,75,73,71,70,69,68,68,69,70,71,72,74,76,79,82,86,90,95,101,107,116,126,134,122,113,105,99,94,89,85,82,79,76,74,72,72,71,70,70,70,71,71,73,75,78,80,83,88,91,97,102,109,118,129,137,125,115,108,101,95,90,86,83,80,77,76,74,72,71,72,71,72,72,73,75,77,79,81,84,88,94,98,104,111,120,131,140,128,118,109,103,97,92,88,85,82,78,77,75,74,73,72,72,73,73,75,76,79,81,83,86,90,94,100,106,113,123,134,143,131,120,112,104,99,94,90,86,83,81,79,77,75,75,74,74,75,75,76,78,80,82,85,88,92,97,102,108,116,126,137,146,134,124,115,108,102,96,91,88,85,82,80,78,77,76,76,75,76,77,78,79,82,84,87,90,94,98,104,111,119,128,140,151,138,127,118,110,103,98,94,90,87,84,81,80,78,78,78,77,78,79,79,82,83,85,88,91,96,100,107,113,122,132,144
};

static uint8_t _CALIBRATION_SHADING_LS_CWF_R[1024]=
{
164,147,134,123,115,107,102,97,93,90,87,84,83,81,80,79,80,80,80,82,83,86,88,91,96,100,106,112,120,129,140,155,158,143,131,120,112,105,100,94,90,87,84,82,80,80,78,77,77,78,79,80,82,83,86,89,93,98,103,109,117,126,136,151,153,139,127,118,110,103,97,92,88,85,83,80,79,77,76,76,76,76,77,78,80,82,84,87,91,96,100,107,113,123,134,147,148,135,124,115,108,101,95,90,87,83,81,79,76,76,75,74,74,74,75,76,78,80,83,86,90,93,98,104,111,120,130,143,144,132,121,112,105,99,94,89,85,82,79,77,75,74,72,73,73,72,73,75,76,78,81,84,87,92,96,102,109,116,127,140,141,128,118,110,103,97,91,87,83,80,77,76,73,73,72,71,71,71,72,73,75,77,80,83,86,89,94,100,106,114,124,136,139,126,116,108,101,95,90,86,82,79,77,74,73,71,70,69,69,70,71,72,73,76,78,81,84,88,93,99,105,112,122,133,137,125,115,106,100,93,89,84,80,77,75,73,71,70,70,69,69,69,69,71,73,74,77,80,84,87,92,96,103,110,119,131,135,123,112,104,98,93,87,82,80,77,74,72,70,69,68,68,67,68,69,69,72,73,76,79,82,86,90,96,102,109,118,129,133,121,112,103,97,91,87,82,78,76,73,71,70,68,67,67,66,67,68,69,70,73,75,78,81,85,89,95,101,107,116,127,132,120,110,102,96,90,86,81,77,75,73,70,69,67,66,66,66,66,67,68,69,72,74,77,80,84,89,93,100,107,116,126,131,118,109,101,95,90,85,80,76,74,72,69,68,66,66,66,66,66,66,67,69,71,73,76,80,83,88,93,99,105,114,125,129,118,108,100,94,89,84,80,76,73,71,69,67,66,66,65,65,65,66,67,69,70,72,76,79,83,87,92,98,104,113,123,128,117,108,100,94,89,84,80,76,73,71,69,67,66,65,64,64,65,66,66,68,70,73,75,78,83,86,92,97,104,113,123,128,117,108,100,94,88,83,80,76,72,70,69,66,66,65,64,64,64,66,66,68,69,73,75,78,82,86,91,97,103,112,123,129,117,107,100,94,88,83,80,76,72,70,68,66,66,65,64,64,64,66,66,68,69,72,75,78,82,86,91,97,103,112,122,129,116,107,100,94,88,83,80,76,72,70,69,66,66,65,64,64,64,65,66,68,69,72,75,78,82,86,91,96,103,112,122,128,117,108,101,94,88,83,80,76,73,70,69,67,66,65,64,64,64,66,66,68,69,72,75,78,82,86,91,97,103,112,122,128,117,108,101,94,89,84,80,76,73,71,69,67,66,65,64,64,65,66,66,68,70,73,75,79,82,86,92,97,103,113,123,129,117,108,100,95,90,84,80,76,74,71,70,67,67,66,65,65,65,66,67,69,70,72,76,79,83,86,92,97,104,113,123,130,118,109,101,95,90,85,80,77,74,72,69,68,66,66,66,66,66,66,67,69,71,73,76,80,82,87,93,98,105,113,124,131,119,110,102,96,90,86,81,78,75,73,70,69,67,66,66,66,67,67,68,69,72,73,76,79,83,88,93,99,106,114,125,133,122,111,104,97,91,86,83,79,76,72,71,70,68,67,66,66,67,67,69,70,73,75,77,80,84,89,94,100,106,116,126,135,123,113,105,98,92,87,83,80,76,74,72,70,69,68,67,67,68,69,69,71,73,75,78,81,86,90,95,101,108,117,128,136,124,115,106,99,94,89,84,80,77,75,72,71,69,69,69,68,69,69,70,72,74,76,80,83,86,91,96,103,109,120,130,139,127,117,107,101,95,90,86,82,79,76,74,73,71,70,69,69,70,70,72,73,76,77,80,83,88,93,98,104,112,121,132,142,129,118,110,103,97,91,87,83,80,77,75,73,73,72,71,70,71,72,72,74,76,79,82,85,90,94,100,106,113,123,135,145,131,120,112,105,99,94,89,85,81,79,76,75,73,72,72,72,73,73,74,76,78,80,83,86,90,96,101,108,116,126,137,148,135,124,114,107,100,95,90,86,83,80,78,76,75,74,73,73,74,75,76,77,80,82,85,88,93,97,103,109,118,129,141,151,138,127,117,110,103,97,92,88,85,82,79,78,76,76,75,75,76,76,77,79,81,83,86,90,94,100,106,113,121,132,145,156,142,130,121,112,105,99,94,90,87,84,82,80,79,78,77,77,77,78,80,81,82,86,89,92,96,102,108,116,125,135,148,162,147,134,124,114,107,101,97,93,89,87,84,82,80,80,79,79,79,79,81,83,85,87,90,94,99,104,110,119,128,139,153
};

static uint8_t _CALIBRATION_SHADING_LS_CWF_G[1024]=
{
156,142,130,120,112,106,99,95,91,88,86,83,81,80,79,79,79,79,79,81,82,85,87,90,94,98,103,108,116,125,136,148,151,137,127,117,110,103,98,93,89,86,83,82,79,78,77,77,77,77,78,79,81,82,85,89,92,96,101,106,113,122,132,144,146,134,123,114,106,101,96,91,87,84,82,79,78,76,76,75,75,76,76,77,79,81,83,86,90,94,99,104,111,119,128,141,142,130,120,111,105,99,94,89,86,82,80,78,76,75,74,73,73,74,75,76,77,79,82,85,88,92,97,102,108,116,125,137,140,127,117,109,103,97,92,87,84,81,78,76,75,73,72,72,72,72,73,74,76,78,80,83,86,90,95,100,106,114,123,134,137,125,115,107,101,95,90,86,82,79,77,75,73,72,71,70,70,71,72,72,74,76,79,82,85,89,93,98,105,111,121,131,134,123,113,105,99,93,89,85,81,78,76,73,72,71,69,69,69,69,70,72,73,75,77,80,83,87,92,97,102,109,118,129,132,120,111,103,98,93,87,83,79,77,75,72,71,69,69,68,68,69,69,70,72,74,76,79,82,85,90,95,102,108,117,128,130,119,110,103,96,91,86,82,79,76,73,72,69,69,68,67,67,68,68,69,71,73,75,78,81,85,89,94,100,106,115,125,129,118,108,101,96,90,86,81,78,75,72,70,69,68,67,66,66,67,67,69,70,72,74,77,80,84,89,93,98,105,114,124,128,116,107,100,94,89,85,80,77,74,72,70,68,67,66,66,66,66,67,68,69,71,73,76,79,83,88,92,98,104,112,122,126,116,106,99,93,89,84,80,76,73,71,69,68,66,66,65,66,66,66,67,69,71,72,76,79,82,87,92,97,103,111,121,126,115,106,99,93,88,83,79,76,73,71,69,67,66,65,65,65,65,66,67,68,70,72,76,79,82,86,91,96,103,111,120,125,114,105,98,93,87,82,79,76,72,70,68,67,66,65,64,64,65,66,66,68,69,72,75,78,82,85,90,96,102,110,120,125,114,105,98,92,87,82,79,76,72,70,68,66,66,65,64,64,64,65,66,68,69,72,74,78,81,85,90,95,102,110,119,124,113,105,98,92,87,82,79,76,72,70,68,66,66,64,64,64,64,65,66,67,69,72,74,77,81,85,90,95,102,109,118,124,113,104,98,92,87,82,79,76,72,70,68,66,66,64,64,64,64,65,66,67,69,72,74,77,81,85,90,95,102,109,118,124,113,105,98,92,87,82,79,76,72,70,68,66,66,65,64,64,64,65,66,68,69,72,75,78,81,85,90,95,102,109,119,125,114,105,98,92,87,82,79,76,72,70,69,66,66,65,64,64,65,66,66,68,69,72,75,78,82,85,90,96,102,110,119,125,115,106,99,93,88,83,79,76,73,71,69,67,66,65,65,65,65,66,66,68,70,72,75,78,82,86,91,96,102,110,120,126,115,106,99,93,88,84,80,76,73,71,69,68,66,66,65,65,66,66,67,69,70,72,76,79,82,86,91,97,103,111,121,127,116,107,100,94,89,85,80,77,74,72,69,68,67,66,66,66,66,66,68,69,71,73,76,79,83,87,92,98,104,111,121,129,117,109,101,95,90,86,81,78,75,72,70,69,68,67,66,66,66,67,69,70,72,74,77,80,83,88,93,98,105,113,123,130,119,109,102,96,91,86,82,79,76,73,72,69,69,68,67,67,67,68,69,71,72,75,78,81,85,89,94,99,106,115,124,132,121,111,103,97,92,87,83,79,77,74,72,71,69,69,68,68,69,69,70,72,74,76,79,82,85,90,95,101,108,116,126,134,123,113,106,99,93,89,85,81,78,76,73,72,71,69,69,69,69,70,71,72,75,77,79,83,87,91,96,102,109,118,128,136,125,115,107,101,95,90,86,82,79,76,75,73,72,71,70,70,71,71,72,74,76,79,81,84,88,92,98,104,111,120,131,139,127,118,109,102,97,92,87,84,81,78,76,74,73,72,72,72,72,72,74,76,77,79,82,85,89,94,100,105,113,122,134,142,130,120,111,105,99,93,89,86,82,79,77,76,75,74,73,73,73,74,76,77,79,81,84,87,92,96,102,108,115,125,136,146,133,122,114,107,101,96,91,87,84,81,79,78,76,76,75,75,75,76,77,79,80,82,85,89,93,98,104,110,118,128,140,150,137,126,117,109,103,98,93,89,86,83,81,79,78,77,76,76,77,77,79,80,82,85,87,91,95,100,105,112,121,131,144,155,141,130,120,112,106,99,95,91,88,85,82,81,79,79,78,78,79,79,80,82,84,86,89,93,97,102,108,115,125,134,147
};

static uint8_t _CALIBRATION_SHADING_LS_CWF_B[1024]=
{
155,139,128,118,110,104,99,94,90,88,85,82,81,79,79,78,78,79,79,80,81,83,86,89,93,97,102,108,114,124,133,147,150,136,125,116,108,102,97,92,89,85,83,81,79,78,77,76,76,77,78,78,80,81,84,88,91,95,100,105,112,120,130,143,145,132,121,112,106,100,95,90,87,83,81,78,77,76,76,75,75,75,75,77,78,80,82,85,88,93,98,103,110,117,127,139,141,129,119,110,103,98,93,89,85,82,79,77,75,74,73,73,73,73,74,75,76,78,81,84,87,91,95,101,107,114,124,136,138,126,116,108,102,96,92,87,83,80,78,75,74,73,72,72,72,71,72,73,75,77,79,81,85,89,94,99,105,113,121,132,135,123,113,105,100,94,89,86,82,78,76,74,72,72,71,70,70,70,71,71,73,75,78,81,84,88,92,98,103,110,119,130,133,121,112,104,99,92,88,84,80,77,75,73,72,70,69,69,68,69,70,71,72,75,76,79,82,86,91,96,101,108,117,127,130,119,110,102,96,92,86,82,79,76,74,72,70,69,69,68,68,68,68,70,72,73,75,78,82,85,89,95,101,107,116,126,129,118,109,102,95,90,85,82,78,76,73,71,69,69,68,67,67,67,68,68,71,72,75,77,80,84,88,93,98,105,114,124,127,116,107,100,95,89,85,81,77,75,72,70,69,68,66,66,66,66,67,68,69,72,74,76,79,83,88,92,98,104,112,122,125,115,106,99,93,89,84,80,76,74,72,69,68,67,66,66,66,65,66,67,68,71,73,75,78,82,86,91,97,103,111,121,125,114,106,99,92,88,83,79,76,73,71,68,67,66,66,65,65,66,65,67,69,70,72,76,79,81,85,91,96,102,111,120,124,113,105,98,92,87,82,78,75,72,70,69,67,65,65,65,65,65,66,66,68,69,71,75,78,82,85,90,95,101,110,119,123,113,104,97,92,86,82,79,75,72,70,68,66,66,65,64,64,64,65,66,67,69,72,74,77,81,85,90,95,101,109,118,123,112,103,97,91,86,82,78,75,72,70,68,66,66,64,64,64,64,65,65,67,68,72,74,77,81,85,89,95,101,109,118,122,112,103,97,92,86,82,78,75,71,69,68,66,65,64,64,64,64,65,65,67,69,71,74,77,80,85,89,95,101,108,118,122,112,103,97,92,86,82,78,75,72,69,68,66,65,64,64,64,64,65,65,67,69,71,74,77,81,85,89,95,101,108,118,122,112,104,97,92,86,82,78,75,72,69,68,66,66,64,64,64,64,65,66,67,69,71,74,77,81,85,89,95,101,109,118,122,112,104,98,92,86,82,79,75,72,70,68,66,66,65,64,64,65,65,66,68,69,72,74,77,81,85,89,95,101,109,119,123,113,105,98,92,87,82,78,76,72,70,69,67,66,65,65,65,65,66,66,68,69,71,75,78,81,85,90,95,101,110,119,124,114,105,99,92,88,83,79,75,73,71,68,67,66,66,65,65,66,65,67,68,70,72,75,78,81,85,91,96,102,110,120,125,115,106,99,93,88,84,79,76,74,72,69,68,66,65,66,66,66,66,67,68,71,72,75,78,82,86,91,97,103,111,121,126,116,107,100,95,89,85,81,77,74,71,70,69,67,66,66,66,66,67,69,69,72,74,76,79,83,88,92,98,104,112,122,128,117,109,102,95,90,85,82,78,76,72,71,69,68,67,67,67,67,68,69,70,72,75,77,80,84,88,93,98,105,113,123,130,119,111,102,97,91,86,82,79,76,74,71,70,69,69,68,68,68,68,70,72,73,76,78,81,85,89,95,100,106,115,125,132,122,111,104,98,92,88,84,80,77,75,73,71,70,69,68,68,69,70,71,72,74,76,79,82,86,91,95,101,108,117,127,135,123,114,105,99,94,89,85,82,78,76,74,72,72,71,70,70,70,71,72,73,76,78,80,84,88,92,97,103,110,119,129,138,125,116,108,102,96,91,86,83,80,77,76,74,72,71,72,71,72,72,73,75,77,79,81,85,89,94,98,104,112,121,132,141,128,118,110,103,98,92,88,85,82,78,77,76,74,73,72,72,73,74,75,76,78,81,83,87,91,95,101,107,114,124,134,145,132,121,112,105,99,95,90,86,83,81,78,77,75,75,74,74,75,75,76,78,79,82,85,88,92,97,102,109,118,127,137,148,134,124,115,108,101,96,91,88,85,82,80,78,77,76,76,75,76,77,78,80,81,84,87,90,95,99,104,111,120,130,141,152,138,128,117,110,104,98,93,90,87,84,81,80,78,78,78,77,78,78,80,82,84,85,88,92,96,102,107,114,122,133,145
};

static uint8_t _CALIBRATION_SHADING_LS_TL84_R[1024]=
{
163,148,135,124,115,108,102,98,93,90,87,84,82,80,80,79,79,80,80,82,83,86,88,92,95,99,105,112,120,130,141,156,157,143,131,120,112,105,100,95,90,87,84,83,80,79,78,77,77,78,79,80,82,83,87,89,93,97,103,109,116,126,136,150,153,138,127,117,109,103,97,92,88,85,83,80,79,77,76,75,76,76,77,78,80,82,84,87,91,95,100,106,113,123,133,146,148,135,124,115,107,101,95,90,87,83,80,79,76,76,74,74,74,74,75,76,78,80,83,86,90,93,98,104,111,120,130,143,145,132,121,111,104,99,93,89,85,82,79,77,75,73,72,73,73,72,73,75,76,78,81,83,87,91,96,102,108,116,127,140,142,129,119,109,103,97,91,87,83,80,77,76,73,73,72,71,71,71,72,73,75,77,80,83,86,89,95,99,106,114,124,136,139,126,116,107,101,95,90,86,82,79,76,74,73,71,70,69,69,70,71,72,73,76,78,81,84,88,93,98,105,112,122,133,136,124,114,106,100,93,88,84,80,77,75,72,71,69,69,69,69,69,69,71,73,74,76,79,82,87,92,97,102,110,119,131,135,122,112,104,98,93,87,83,80,77,74,72,70,69,68,67,67,68,69,69,72,73,76,79,82,86,90,96,102,109,118,129,133,121,111,103,97,91,86,82,78,75,72,71,70,68,67,66,66,67,68,69,70,73,75,77,81,85,89,94,100,107,116,127,132,119,110,102,96,90,86,81,77,74,73,70,69,67,66,66,66,66,67,68,69,72,74,76,80,84,89,93,100,107,115,126,131,118,108,101,95,90,85,80,76,74,72,69,68,66,66,66,65,66,66,67,69,71,73,76,80,83,87,93,99,105,113,123,129,118,108,101,94,89,84,80,76,73,71,69,67,66,66,65,65,65,66,67,69,70,72,76,79,83,87,92,98,104,112,122,129,117,108,100,94,88,83,80,76,73,71,69,67,66,65,64,64,65,66,66,68,70,73,75,78,83,86,92,97,104,112,123,129,117,107,100,93,88,83,80,76,72,70,68,66,66,65,64,64,64,66,66,68,69,72,75,78,82,86,91,96,103,112,122,128,116,107,100,94,88,83,80,76,72,70,68,66,66,64,64,64,64,65,66,68,69,72,75,78,82,85,91,96,103,112,121,128,117,107,100,94,88,83,79,76,72,70,69,66,66,65,64,64,64,65,66,68,69,72,75,78,82,85,91,96,103,111,121,128,117,107,100,93,88,83,80,76,73,70,69,66,66,65,64,64,64,66,66,68,69,72,75,78,82,86,91,97,103,112,121,129,117,108,101,94,89,83,80,77,73,71,69,67,66,65,64,64,65,66,66,68,70,73,75,78,82,86,91,97,103,112,122,129,118,108,101,94,89,84,80,76,73,71,69,67,67,66,65,65,65,66,67,69,70,72,76,79,83,86,92,97,104,112,123,130,118,109,101,95,90,85,80,77,74,72,69,68,66,66,66,65,66,66,67,69,71,73,76,79,82,87,92,98,105,113,123,132,120,110,102,96,91,86,81,78,75,73,70,69,67,66,66,66,66,67,68,69,72,73,76,79,83,88,93,99,106,114,125,133,122,111,104,97,91,87,82,78,76,73,71,70,68,67,66,66,66,67,69,70,73,74,77,80,84,89,93,99,106,116,126,135,122,113,104,98,93,87,83,80,77,73,72,70,69,68,67,67,68,69,69,71,73,75,78,82,86,89,95,101,108,117,128,137,125,115,106,99,94,89,84,80,77,75,72,71,69,69,68,68,69,69,70,72,74,76,79,83,86,91,96,102,109,119,130,140,127,117,107,101,95,91,86,82,79,76,73,73,71,70,69,69,69,71,72,73,75,77,80,83,88,93,98,104,111,121,133,142,129,118,110,103,97,91,87,83,80,77,75,73,73,71,71,71,71,72,72,74,76,79,82,85,89,94,99,106,113,123,135,145,132,121,112,104,98,94,89,84,81,79,76,75,73,72,72,72,73,73,74,76,78,80,82,86,91,96,101,108,116,126,138,148,135,123,115,107,101,95,90,87,82,80,78,77,75,74,73,73,73,74,76,77,80,82,85,88,93,97,103,109,118,129,141,152,138,126,117,109,103,97,92,88,85,82,79,78,76,76,75,75,76,76,77,79,81,83,86,89,94,100,105,113,121,131,145,157,143,130,120,112,105,100,94,90,87,84,82,80,79,78,77,77,77,78,80,81,82,85,88,92,96,101,108,116,124,136,149,164,147,135,124,115,107,102,97,93,89,87,84,82,80,80,79,79,79,79,81,83,85,87,90,94,98,103,110,119,128,140,154
};

static uint8_t _CALIBRATION_SHADING_LS_TL84_G[1024]=
{
156,141,130,120,112,106,100,95,91,88,86,83,82,80,79,79,79,79,79,81,82,85,87,90,94,99,103,108,116,125,136,149,151,137,127,117,110,103,98,93,89,86,83,82,79,79,77,77,77,77,78,79,81,82,85,89,92,96,101,106,114,122,132,145,147,134,123,114,107,101,96,91,87,84,82,79,78,76,76,75,75,76,76,77,79,81,83,86,90,94,99,105,111,119,129,141,143,130,120,111,105,99,93,89,86,82,80,78,76,75,74,73,73,74,75,76,77,79,82,85,88,92,97,102,108,116,126,138,140,128,117,109,103,97,92,87,84,81,79,76,75,73,72,72,72,72,73,74,76,78,80,83,86,90,95,100,106,114,123,134,137,125,115,107,101,95,90,86,83,79,77,75,73,72,71,71,70,71,72,72,74,76,79,82,85,89,93,98,105,111,121,132,134,123,113,106,100,94,89,85,81,78,76,73,72,71,70,69,69,69,70,72,73,75,77,80,83,87,92,97,102,110,118,129,133,121,111,104,98,93,88,83,79,77,74,72,71,69,69,68,68,69,69,70,72,74,76,79,82,86,90,95,102,108,117,128,130,120,110,103,96,91,86,83,79,76,73,72,70,69,68,67,67,68,69,69,71,73,76,78,81,85,89,95,100,107,115,125,129,118,109,101,96,90,86,81,78,75,72,70,69,68,67,66,66,67,67,69,70,72,74,77,80,84,89,93,98,105,114,124,128,117,107,100,95,89,85,80,77,74,72,70,68,67,66,66,66,66,67,68,69,71,73,76,79,83,88,92,98,104,113,123,127,116,106,100,94,89,84,80,76,73,71,69,68,66,66,66,65,66,66,67,69,71,73,76,79,82,87,92,97,103,111,121,126,115,106,99,93,88,83,79,76,73,71,69,67,66,66,65,65,65,66,67,68,70,72,75,79,82,86,91,96,103,111,121,125,114,105,98,93,87,83,79,76,72,70,69,67,66,65,64,64,65,66,66,68,69,72,75,78,82,85,90,96,102,111,120,125,113,105,98,93,87,83,79,76,72,70,68,66,66,65,64,64,64,65,66,68,69,72,75,78,82,85,90,95,102,110,119,124,113,105,98,92,87,83,79,76,72,70,68,66,66,65,64,64,64,65,66,68,69,72,74,78,81,85,90,95,102,110,119,124,113,105,98,92,87,83,79,76,72,70,68,66,66,64,64,64,64,65,66,67,69,72,74,77,81,85,90,95,102,109,119,124,114,105,98,93,87,83,79,76,72,70,68,66,66,65,64,64,64,65,66,68,69,72,75,78,81,85,90,95,102,110,119,125,114,105,98,93,87,83,79,76,72,70,69,67,66,65,64,64,65,66,66,68,69,72,75,78,82,85,90,96,102,110,120,126,115,106,99,93,88,83,79,76,73,71,69,67,66,65,65,65,65,66,66,68,70,72,75,78,82,86,91,96,102,111,120,126,116,106,100,93,89,84,80,76,73,71,69,68,66,66,65,65,66,66,67,69,71,72,76,79,82,86,91,97,103,111,121,127,117,107,100,94,89,85,80,77,74,72,69,68,67,66,66,66,66,66,68,69,71,73,76,79,83,87,92,98,104,112,122,128,117,109,101,96,90,86,81,78,75,72,71,69,68,67,66,66,66,67,69,70,72,74,77,80,84,88,93,98,105,113,123,130,120,110,103,96,91,86,83,79,76,73,72,69,69,68,67,67,67,68,69,71,72,75,78,81,85,89,94,100,106,115,124,132,121,112,103,98,92,87,83,79,77,74,72,71,69,69,68,68,69,69,70,72,74,76,79,82,85,90,95,101,108,116,127,134,123,113,106,100,93,89,85,81,78,76,73,72,71,69,69,69,69,70,71,73,75,77,80,83,87,92,96,102,109,118,128,137,125,116,107,101,95,90,86,83,79,76,75,73,72,71,70,70,71,72,72,74,76,79,81,84,89,93,98,104,111,120,131,140,127,118,110,103,97,92,87,84,81,78,76,75,73,72,72,72,72,72,74,76,77,79,82,85,90,95,100,106,113,122,133,143,130,120,112,105,99,93,89,86,82,79,77,76,75,74,73,73,73,74,76,77,79,81,84,87,92,96,102,108,115,125,136,147,134,123,114,107,101,96,91,87,84,82,79,78,76,76,75,75,75,76,77,79,80,83,85,89,93,98,104,110,118,128,140,151,137,127,117,110,103,98,93,89,86,83,81,79,78,77,76,76,77,77,79,80,82,85,88,91,95,100,105,113,121,131,144,156,141,130,120,112,106,99,95,91,88,85,82,81,79,79,78,78,79,79,80,82,84,86,89,93,97,102,108,115,125,134,147
};

static uint8_t _CALIBRATION_SHADING_LS_TL84_B[1024]=
{
154,140,128,119,111,104,98,95,90,87,85,82,81,79,79,78,78,79,79,80,81,84,86,89,93,97,102,108,114,124,133,146,149,136,125,116,109,102,97,92,89,85,83,81,79,78,77,76,76,77,78,78,80,81,85,87,91,95,100,105,112,120,130,143,144,132,122,112,105,100,95,91,87,84,81,78,77,76,76,75,75,75,75,77,79,80,82,85,89,93,98,103,109,117,127,139,140,129,119,110,103,98,93,89,85,82,79,77,75,75,73,73,73,73,74,76,76,78,81,84,87,91,95,101,107,114,124,136,137,126,116,108,102,95,92,87,84,80,78,75,74,73,72,72,72,71,72,74,76,77,79,82,85,89,94,99,105,112,122,133,135,123,114,105,100,95,89,86,82,78,76,75,72,72,71,70,70,70,71,72,74,75,78,81,84,88,92,98,104,110,119,130,133,121,112,104,99,93,89,84,81,78,75,73,72,70,69,69,69,69,70,71,72,75,77,79,82,86,91,96,101,108,117,127,130,119,110,103,97,92,87,82,79,76,74,72,71,69,69,68,68,68,68,70,72,73,75,78,81,85,89,95,101,107,115,126,128,118,109,102,96,90,85,82,78,76,73,71,69,69,68,67,67,67,68,69,70,72,75,77,80,84,88,93,99,105,114,124,126,116,108,100,95,89,85,81,77,75,72,70,69,68,67,66,66,66,67,69,69,72,74,76,79,83,88,92,98,104,113,123,126,115,106,99,94,89,84,80,76,74,72,69,68,67,66,66,66,65,66,68,68,71,73,75,78,82,87,91,97,103,111,120,125,115,106,99,92,88,83,79,76,73,71,68,67,66,66,65,65,66,66,67,69,70,72,76,78,82,86,91,96,102,111,120,124,113,105,98,92,87,83,79,75,72,70,69,67,66,65,64,65,65,66,66,68,69,72,75,78,82,85,91,95,102,110,119,123,113,104,97,92,86,83,79,76,72,70,68,66,66,65,64,64,65,66,66,68,69,72,74,77,81,85,90,95,101,109,118,123,113,104,97,92,86,83,79,75,72,69,68,66,66,64,64,64,64,65,66,67,69,72,74,77,81,85,89,95,101,109,118,123,113,104,97,92,86,82,78,75,72,69,68,66,65,64,64,64,64,65,65,67,69,71,74,77,81,85,89,95,101,109,118,123,112,103,97,92,86,82,78,75,72,69,68,66,65,64,64,64,64,65,65,67,68,71,74,77,81,85,89,95,101,108,117,123,112,104,97,92,86,83,78,75,72,70,68,66,65,64,64,64,64,65,66,67,69,72,74,77,81,85,90,95,101,109,118,123,112,104,97,92,87,83,79,75,72,70,68,66,66,65,64,64,65,65,66,67,69,72,74,77,81,85,89,95,101,109,118,123,113,105,98,92,87,83,78,75,72,70,69,67,66,65,65,65,65,66,66,68,69,71,75,78,81,85,90,95,102,110,119,125,114,106,99,93,88,83,79,76,73,71,68,67,66,66,65,65,66,66,67,69,70,72,75,78,81,86,91,96,102,111,120,126,115,106,99,94,89,84,80,76,74,72,69,68,66,65,66,66,66,66,67,68,71,73,75,78,82,86,92,97,103,111,120,127,116,107,100,95,89,85,80,77,75,72,70,69,67,66,66,66,66,67,68,69,72,74,76,79,83,87,92,98,105,112,121,129,118,109,101,96,90,85,82,79,76,73,71,69,68,67,67,67,67,68,69,70,72,75,77,80,84,89,93,99,105,114,124,131,119,110,102,97,91,87,82,79,76,74,72,70,69,69,68,68,68,68,70,72,73,76,79,81,85,89,95,100,107,115,125,133,121,112,104,99,92,88,84,80,77,75,72,72,70,69,68,68,69,70,71,72,74,76,79,82,86,91,95,101,108,117,127,135,124,114,105,100,94,89,85,82,78,76,74,72,72,70,70,70,70,71,71,73,76,78,81,84,88,91,98,103,110,119,129,137,126,116,108,102,95,91,86,83,80,78,75,74,72,71,71,71,72,72,73,75,77,79,81,85,89,94,99,105,111,120,132,140,129,119,110,103,97,92,88,85,82,78,77,76,74,73,72,72,73,74,75,76,79,81,84,87,91,95,101,107,114,124,134,144,132,121,113,105,99,95,90,86,83,81,78,77,75,75,75,74,75,75,76,78,80,82,85,88,92,97,103,109,117,127,138,148,135,125,116,108,102,96,91,88,85,82,80,78,77,76,76,75,76,77,78,80,81,84,87,90,95,99,105,111,120,130,142,153,140,128,119,111,104,99,94,90,87,84,82,80,78,78,78,78,78,78,80,82,83,85,88,92,96,101,107,114,123,134,147
};

static uint8_t _CALIBRATION_SHADING_LS_A_R[1024]=
{
165,149,136,125,115,108,102,97,93,90,87,85,83,81,79,80,80,79,80,82,84,86,88,92,96,100,106,113,120,130,142,157,158,143,132,121,113,106,100,95,91,87,85,83,80,79,78,77,78,78,79,80,82,84,86,89,94,98,103,110,116,126,138,152,153,139,128,118,110,104,97,93,89,85,83,80,79,77,76,76,76,76,77,78,80,82,84,87,91,96,101,107,114,123,134,148,149,136,125,115,108,101,96,91,87,83,81,79,77,76,74,74,74,74,75,76,78,80,83,86,89,94,99,105,111,120,130,143,147,132,122,112,105,99,93,89,85,82,80,77,75,74,73,72,73,72,73,75,76,79,81,84,87,92,96,103,109,116,127,140,143,130,118,110,104,97,92,87,83,80,78,76,74,73,72,71,71,71,73,73,75,77,80,83,86,90,95,101,107,115,125,136,140,127,116,108,101,96,90,86,82,79,77,74,73,71,70,69,69,70,71,72,74,76,78,81,85,89,93,99,105,113,123,134,138,126,115,107,100,94,89,84,80,78,75,72,71,70,70,69,69,69,69,71,73,75,77,79,83,87,92,97,103,111,120,132,136,124,113,105,98,93,87,83,80,77,74,72,70,69,68,68,67,68,69,70,72,73,76,79,82,86,90,96,102,109,119,130,134,121,111,104,97,92,87,83,79,76,73,71,70,68,67,66,66,67,68,69,70,72,75,78,81,85,89,95,101,108,116,128,133,121,111,103,97,91,86,82,78,75,73,70,69,67,66,66,67,66,67,68,69,72,74,77,80,84,89,94,99,106,116,126,132,119,109,102,96,90,85,81,77,74,72,69,68,66,66,66,66,66,66,67,69,71,73,76,79,83,88,92,99,106,114,125,131,118,108,101,95,90,84,80,76,73,71,70,67,66,65,64,65,65,67,67,69,70,72,76,79,82,87,92,98,105,113,124,130,118,107,101,94,89,84,79,77,73,71,69,67,66,65,64,64,65,66,66,68,70,73,76,79,83,86,92,97,104,112,123,129,118,107,101,94,89,84,80,76,72,70,69,67,66,65,64,64,64,66,66,68,70,73,75,78,82,86,92,97,104,112,123,129,117,108,100,93,88,83,80,76,72,70,68,66,66,65,64,64,64,65,66,68,69,72,75,78,82,86,91,97,103,113,123,129,117,108,101,94,88,83,80,76,73,70,69,66,66,65,64,64,64,65,66,68,69,72,75,78,82,86,91,97,104,112,122,129,118,108,101,94,89,83,80,76,73,70,69,67,66,65,64,64,64,66,66,68,70,73,75,78,82,86,92,97,104,113,123,130,118,108,101,94,89,84,80,77,73,71,69,67,66,65,65,64,65,66,66,68,70,72,75,79,83,86,92,98,104,112,123,130,118,108,101,95,90,84,80,76,73,71,70,68,66,66,65,65,65,66,67,69,70,72,76,79,83,87,92,98,105,113,123,132,119,109,102,96,91,85,81,77,74,72,69,68,66,66,66,66,66,66,67,69,71,73,76,80,83,88,93,99,106,114,124,132,121,111,103,97,90,86,82,78,75,73,70,69,67,66,67,66,66,67,68,69,72,74,76,80,84,88,93,100,106,115,126,134,122,111,104,97,92,87,83,79,76,73,71,70,68,67,66,66,67,68,69,70,73,75,77,81,85,89,94,100,107,116,127,136,124,113,105,99,93,87,83,79,76,74,72,70,69,68,68,67,68,69,69,71,73,76,78,82,86,90,96,102,109,118,130,137,125,115,107,100,94,89,85,81,77,75,73,71,70,69,69,68,69,69,71,73,74,76,80,83,87,91,97,103,110,120,132,139,127,117,108,101,96,90,86,82,79,77,74,73,71,70,69,69,70,71,72,73,76,78,80,84,88,93,99,105,113,122,133,143,129,118,111,104,97,92,87,83,80,78,75,74,73,71,71,71,71,72,73,75,76,79,82,86,89,94,100,106,114,124,136,146,132,122,112,105,99,94,89,85,82,79,76,75,74,72,73,72,72,73,74,76,78,80,83,86,91,96,102,109,116,126,139,150,136,125,115,107,101,95,91,87,83,80,78,76,76,74,74,73,74,75,76,77,80,82,85,89,93,98,104,110,119,130,142,153,139,127,118,110,104,97,93,88,85,83,80,78,77,76,76,75,76,76,77,79,81,83,86,90,95,99,106,113,122,133,146,158,142,131,121,112,106,100,95,90,87,84,82,80,79,78,77,77,77,78,80,81,83,86,89,93,97,102,108,116,126,136,150,163,146,135,124,115,108,102,97,93,89,87,84,82,81,79,79,79,79,80,81,83,85,88,91,95,100,104,111,119,129,141,156
};

static uint8_t _CALIBRATION_SHADING_LS_A_G[1024]=
{
157,143,130,121,113,106,100,96,92,89,86,83,82,80,79,79,79,79,80,81,82,85,87,91,95,99,104,110,117,125,137,151,152,138,127,118,110,104,98,93,89,86,84,82,80,79,78,77,77,77,78,79,81,83,85,89,92,97,102,107,114,123,133,146,148,134,124,114,107,101,96,91,88,84,82,79,78,76,76,75,76,76,76,78,79,81,84,86,90,94,99,105,112,120,130,142,144,131,121,112,105,99,94,89,86,82,80,78,76,75,74,73,73,74,75,76,77,79,82,85,89,92,97,102,109,117,127,138,141,128,118,110,103,97,93,88,84,81,79,76,75,73,72,72,72,72,73,74,76,78,80,83,86,91,95,101,107,115,124,135,137,126,116,108,101,96,90,86,83,79,77,75,73,72,71,71,71,71,72,73,75,76,79,82,85,89,93,99,105,112,121,132,134,123,114,106,100,94,89,85,81,78,76,74,72,71,70,69,69,69,70,72,73,75,78,80,84,88,92,98,103,110,119,130,133,121,112,104,98,93,88,83,80,77,75,72,71,69,69,69,68,69,69,70,72,74,76,79,82,86,91,96,102,108,118,128,131,120,110,103,97,92,86,83,79,76,73,72,70,69,68,67,67,68,69,69,71,73,76,78,82,85,89,95,100,107,115,126,130,119,109,102,96,90,86,82,78,75,72,71,69,68,67,66,66,67,67,69,70,72,75,77,80,84,89,94,99,106,115,124,128,117,108,101,95,89,85,81,77,74,72,70,69,67,66,66,66,66,67,68,69,72,74,76,79,83,88,93,98,105,113,123,127,117,107,100,94,89,84,80,76,74,71,69,68,66,66,66,66,66,66,67,69,71,73,76,79,83,87,92,98,104,112,122,127,116,106,100,93,88,83,79,76,73,71,69,67,66,66,65,65,65,66,67,68,70,72,76,79,82,86,92,97,103,111,121,127,115,106,99,93,88,83,79,76,72,70,69,67,66,65,64,64,65,66,66,68,70,72,75,78,82,86,91,96,102,111,121,126,115,106,99,93,87,83,79,76,72,70,68,66,66,65,64,64,64,66,66,68,69,72,75,78,82,85,91,96,102,111,120,126,114,105,99,93,87,83,79,76,72,70,68,66,66,64,64,64,64,65,66,68,69,72,75,78,82,85,90,96,102,110,120,126,114,105,99,93,87,83,79,76,72,70,68,66,66,64,64,64,64,65,66,67,69,72,75,78,82,85,90,96,102,110,120,125,114,105,99,93,87,83,79,76,72,70,68,66,66,65,64,64,64,65,66,68,69,72,75,78,82,85,90,96,102,110,120,126,115,106,99,93,88,83,79,76,73,70,69,67,66,65,64,64,65,66,66,68,70,72,75,78,82,85,91,96,102,111,121,127,116,106,100,93,88,83,79,76,73,71,69,67,66,66,65,65,65,66,66,68,70,72,76,79,82,86,91,97,103,111,121,127,117,107,100,94,89,84,80,76,74,72,69,68,66,66,65,65,66,66,67,69,71,73,76,79,82,87,92,97,104,111,121,128,117,108,101,95,89,85,81,77,75,72,70,69,67,66,66,66,66,66,68,69,71,73,76,79,83,88,92,98,105,112,122,130,118,109,102,96,90,86,82,78,75,72,71,69,68,67,66,66,66,67,69,70,72,74,77,80,84,88,93,99,105,114,124,131,120,110,103,97,91,86,83,79,76,73,72,70,69,68,67,67,68,69,69,71,73,75,78,81,85,89,95,100,107,115,125,134,122,112,104,98,93,88,83,80,77,75,72,71,69,69,68,68,69,69,70,72,74,76,79,82,86,91,95,102,108,117,128,135,123,114,106,100,94,89,85,81,78,76,73,72,71,70,69,69,69,70,72,73,75,77,80,83,87,92,97,103,110,119,130,137,126,116,108,102,96,90,86,83,79,77,75,73,72,71,70,70,71,72,72,74,76,79,82,85,89,93,98,105,111,121,132,141,128,118,110,103,97,92,88,84,81,79,76,75,73,72,72,72,72,73,74,76,78,80,82,86,90,95,100,106,114,124,134,144,131,121,112,106,99,94,89,86,82,80,78,76,75,74,73,73,73,74,76,77,79,82,84,88,92,97,102,108,117,126,138,147,134,123,115,107,101,96,92,88,84,82,79,78,76,76,75,75,76,76,77,79,81,83,86,89,94,99,104,111,119,129,141,151,138,127,118,110,103,98,93,89,86,84,82,79,79,77,76,76,77,78,79,80,82,85,88,92,95,101,106,114,122,132,144,156,142,130,120,113,106,100,96,91,88,86,83,82,80,79,79,78,79,79,81,82,84,87,90,93,98,102,108,116,125,136,149
};

static uint8_t _CALIBRATION_SHADING_LS_A_B[1024]=
{
158,142,129,118,112,106,99,95,91,89,86,83,81,80,78,79,79,78,79,81,82,84,86,89,94,98,102,108,116,124,135,148,151,137,126,116,109,103,97,93,89,86,83,82,79,78,77,76,77,77,78,79,81,82,85,88,91,95,101,106,113,121,132,144,147,133,122,113,106,101,95,91,87,84,82,79,78,76,76,75,75,76,76,77,78,81,83,86,89,94,98,104,111,118,128,141,142,130,119,111,105,99,93,89,86,82,79,77,75,75,74,73,73,74,75,76,77,78,82,84,88,91,97,102,108,116,126,137,139,126,117,109,102,96,92,88,84,80,78,76,75,73,72,71,72,72,73,74,76,78,80,83,85,90,95,100,106,114,123,134,136,124,114,107,101,95,90,85,83,79,76,75,73,72,71,71,70,71,72,72,74,76,79,81,84,89,93,97,105,111,121,131,133,122,112,105,100,93,89,85,81,78,76,73,72,71,69,69,69,69,70,72,73,75,77,80,83,87,92,97,102,109,118,130,132,120,111,103,98,93,87,83,79,77,75,72,71,69,69,68,68,68,69,70,72,74,75,78,81,85,90,95,101,108,117,127,130,119,109,102,96,91,86,82,79,76,73,72,70,69,68,67,67,67,68,69,71,72,75,78,81,85,89,94,100,106,115,125,129,118,108,101,95,89,85,81,77,75,72,70,70,68,67,66,66,66,67,69,69,71,74,77,80,84,89,93,98,105,113,123,128,116,107,100,94,89,84,80,77,74,72,69,68,67,66,66,66,65,66,68,68,71,73,76,79,83,87,92,98,104,112,122,126,116,106,99,93,88,83,79,76,73,71,69,68,66,66,65,65,66,66,67,69,70,72,76,78,82,86,91,97,103,111,121,126,115,106,99,92,87,83,79,75,72,71,69,67,65,65,65,65,65,66,67,68,70,72,75,78,81,85,91,96,102,111,121,125,114,105,98,92,87,82,78,76,72,70,68,66,66,65,64,64,65,65,66,68,69,72,74,77,82,85,90,95,101,110,120,124,113,105,98,92,86,83,79,75,72,70,68,66,66,64,64,64,64,65,66,68,69,72,74,77,81,85,90,95,101,109,118,123,113,104,97,92,86,82,79,76,72,70,68,66,65,64,64,64,64,65,66,67,69,71,74,77,81,85,90,95,101,109,118,123,112,104,97,92,86,83,78,75,72,69,68,66,66,64,64,64,64,65,65,67,69,71,74,77,81,85,89,95,101,109,118,122,112,105,98,92,87,82,79,75,72,70,68,66,66,65,64,64,64,65,66,67,69,72,74,77,81,85,90,95,101,109,119,123,113,105,99,92,87,82,78,76,72,70,68,66,66,65,64,64,65,65,66,68,69,71,75,78,81,85,90,96,101,110,120,124,114,106,99,92,87,83,79,75,72,71,69,67,65,65,65,65,65,66,66,68,70,72,75,78,82,85,91,96,102,111,120,126,115,105,99,93,88,83,79,75,73,71,68,67,66,66,65,65,66,65,67,69,70,72,76,79,82,86,91,97,103,111,121,127,116,106,100,94,89,84,80,76,74,72,69,68,66,66,66,66,65,66,68,69,71,73,75,79,83,87,91,98,104,112,121,128,117,108,101,95,89,85,81,77,75,72,70,69,67,66,66,66,66,67,69,69,72,74,76,79,83,88,92,98,104,113,123,129,119,109,102,96,90,85,82,78,75,73,71,69,69,68,67,67,67,68,69,71,72,75,77,81,85,88,94,99,106,114,124,131,120,111,103,97,92,87,83,79,76,74,72,70,69,69,68,68,68,68,70,72,73,75,79,82,85,90,95,101,108,116,127,133,123,112,105,99,93,89,84,80,78,76,73,72,70,69,68,68,69,70,71,72,74,77,79,83,87,92,96,102,109,118,129,137,125,115,106,100,94,89,85,82,78,76,74,73,72,71,70,70,70,71,72,74,75,78,81,84,89,92,98,105,111,121,131,139,126,117,109,102,96,91,87,83,80,78,75,74,73,71,72,72,71,72,73,75,77,79,82,85,90,95,100,106,113,122,134,142,129,119,110,104,98,93,89,85,82,79,77,75,75,73,73,73,73,74,75,76,79,81,84,87,92,96,101,108,115,125,136,146,133,122,113,106,100,95,90,86,83,81,78,77,76,75,75,75,75,76,77,78,80,82,85,88,93,98,104,110,118,127,139,150,136,126,116,109,102,97,92,89,85,82,81,79,78,77,76,76,76,77,79,80,82,85,87,91,95,100,106,113,121,130,143,153,139,129,119,111,105,98,94,90,87,85,82,81,79,78,78,78,78,79,80,81,84,86,89,93,97,102,108,116,124,134,146
};

//aisp_lsc_ctl_t
static uint32_t _CALIBRATION_LENS_SHADING_CTL[4] =
{
    2, //mesh shading split mode 0:64x64 1: 32x64 2:32x32
    1, //mesh lut normalize select 0: 128 1:64 2:32 3:16
    32, //mesh hori-node numbers
    32, //mesh vert-node numbers
};

static uint16_t _CALIBRATION_GAMMA[129]=
{
0,86,134,169,198,223,245,265,283,300,316,331,346,359,372,385,397,409,420,431,441,451,461,471,481,490,499,508,516,525,533,541,549,557,565,572,580,587,594,601,608,615,622,628,635,641,648,654,660,667,673,679,685,691,697,702,708,714,719,725,730,736,741,747,752,757,762,767,773,778,783,788,793,797,802,807,812,817,821,826,831,835,840,844,849,853,858,862,867,871,875,880,884,888,892,897,901,905,909,913,917,921,925,929,933,937,941,945,949,953,957,960,964,968,972,976,979,983,987,990,994,998,1001,1005,1009,1012,1016,1019,1023
};

static int32_t _CALIBRATION_CCM[201]=
{
6,
2856,418,88,7942,8063,437,8140,22,7920,506,
4000,533,8146,7962,8061,445,8134,22,7960,465,
4100,534,8001,8106,8045,474,8121,23,7984,441,
5000,488,8087,8066,8089,459,8092,12,7967,469,
6500,489,8019,8133,8095,470,8076,11,7950,487,
7500,488,8011,8142,8099,469,8073,7,7969,472,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0
};

static int8_t _CALIBRATION_CAC_RX[1024]=
{
0,0,1,1,1,2,2,2,2,2,2,2,1,1,1,0,0,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-1,-1,-1,0,0,0,0,1,1,2,2,2,2,2,2,2,2,1,1,1,0,0,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,1,1,0,0,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,-1,0,0,1,1,2,2,2,2,2,2,2,2,2,2,1,1,0,0,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,0,0,1,1,2,2,2,3,3,3,3,2,2,2,1,1,0,0,-1,-1,-2,-2,-2,-3,-3,-3,-3,-2,-2,-2,-1,-1,0,0,1,2,2,2,3,3,3,3,3,2,2,2,1,1,0,0,-1,-1,-2,-2,-2,-3,-3,-3,-3,-3,-2,-2,-2,-1,0,1,1,2,2,2,3,3,3,3,3,3,2,2,1,1,0,0,-1,-1,-2,-2,-3,-3,-3,-3,-3,-3,-2,-2,-2,-1,-1,1,1,2,2,3,3,3,3,3,3,3,2,2,2,1,0,0,-1,-2,-2,-2,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,-1,1,1,2,2,3,3,3,3,3,3,3,3,2,2,1,0,0,-1,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,-1,1,2,2,2,3,3,3,3,3,3,3,3,2,2,1,0,0,-1,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-2,-1,1,2,2,3,3,3,3,3,3,3,3,3,2,2,1,0,0,-1,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,3,3,3,3,3,3,2,2,1,0,0,-1,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,3,3,3,3,3,3,3,2,1,1,-1,-1,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,3,4,4,3,3,3,3,2,2,1,-1,-2,-2,-3,-3,-3,-3,-4,-4,-3,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,4,4,4,3,3,3,3,2,2,1,-1,-2,-2,-3,-3,-3,-3,-4,-4,-3,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,4,4,4,4,3,3,3,2,2,1,-1,-2,-2,-3,-3,-3,-4,-4,-4,-4,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,4,4,4,4,3,3,3,2,2,1,-1,-2,-2,-3,-3,-3,-4,-4,-4,-4,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,3,4,4,3,3,3,3,2,2,1,-1,-2,-2,-3,-3,-3,-3,-4,-4,-3,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,3,4,4,3,3,3,3,2,2,1,-1,-2,-2,-3,-3,-3,-3,-4,-4,-3,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,3,3,3,3,3,3,3,2,1,0,-1,-1,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,3,3,3,3,3,3,2,2,1,0,0,-1,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,1,2,2,3,3,3,3,3,3,3,3,3,2,2,1,0,0,-1,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,1,2,2,2,3,3,3,3,3,3,3,3,2,2,1,0,0,-1,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,-1,1,1,2,2,3,3,3,3,3,3,3,3,2,2,1,0,0,-1,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,-1,1,1,2,2,3,3,3,3,3,3,3,2,2,2,1,0,0,-1,-2,-2,-2,-3,-3,-3,-3,-3,-3,-3,-2,-2,-1,-1,1,1,2,2,2,3,3,3,3,3,3,2,2,1,1,0,0,-1,-1,-2,-2,-3,-3,-3,-3,-3,-3,-2,-2,-2,-1,-1,0,1,2,2,2,3,3,3,3,3,2,2,2,1,1,0,0,-1,-1,-2,-2,-2,-3,-3,-3,-3,-3,-2,-2,-2,-1,0,0,1,1,2,2,2,3,3,3,3,2,2,2,1,1,0,0,-1,-1,-2,-2,-2,-3,-3,-3,-3,-2,-2,-2,-1,-1,0,0,1,1,2,2,2,2,2,2,2,2,2,2,1,1,0,0,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,0,0,1,1,1,2,2,2,2,2,2,2,2,1,1,1,0,0,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,-1,0,0,0,1,1,2,2,2,2,2,2,2,2,1,1,1,0,0,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,0,0,0,0,1,1,1,2,2,2,2,2,2,2,1,1,1,0,0,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-1,-1,-1,0,0
};

static int8_t _CALIBRATION_CAC_RY[1024]=
{
0,0,0,1,1,1,2,2,2,3,3,3,3,3,4,4,4,4,3,3,3,3,3,2,2,2,1,1,1,0,0,0,0,0,1,1,1,1,2,2,2,3,3,3,3,3,4,4,4,4,3,3,3,3,3,2,2,2,1,1,1,1,0,0,0,0,1,1,1,2,2,2,2,3,3,3,3,3,4,4,4,4,3,3,3,3,3,2,2,2,2,1,1,1,0,0,0,0,1,1,1,2,2,2,2,3,3,3,3,3,4,4,4,3,3,3,3,3,3,2,2,2,1,1,1,1,0,0,0,0,1,1,1,1,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,1,1,1,1,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,1,1,1,1,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,1,1,1,1,0,0,0,0,1,1,1,1,1,2,2,2,2,3,3,3,3,3,3,3,3,3,3,2,2,2,2,1,1,1,1,1,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,3,3,3,2,2,2,2,2,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,3,2,2,2,2,2,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,2,2,3,3,2,2,2,2,2,2,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-3,-3,-2,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,0,0,0,0,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-1,-1,-1,-1,-1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-1,-1,-1,-1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-1,-1,-1,-1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-3,-3,-3,-3,-3,-3,-4,-4,-3,-3,-3,-3,-3,-3,-2,-2,-2,-1,-1,-1,-1,0,0,0,0,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3,-3,-4,-4,-4,-4,-3,-3,-3,-3,-3,-2,-2,-2,-2,-1,-1,-1,0,0,0,0,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3,-3,-4,-4,-4,-4,-3,-3,-3,-3,-3,-2,-2,-2,-2,-1,-1,-1,0,0,0,0,-1,-1,-1,-1,-2,-2,-2,-3,-3,-3,-3,-3,-4,-4,-4,-4,-3,-3,-3,-3,-3,-2,-2,-2,-1,-1,-1,-1,0,0,0,0,0,-1,-1,-1,-2,-2,-2,-3,-3,-3,-3,-3,-4,-4,-4,-4,-3,-3,-3,-3,-3,-2,-2,-2,-1,-1,-1,0,0,0
};

static int8_t _CALIBRATION_CAC_BX[1024]=
{
-31,-27,-24,-21,-18,-15,-13,-11,-9,-7,-6,-4,-3,-2,-1,0,0,1,2,3,4,6,7,9,11,13,15,18,21,24,27,31,-30,-27,-23,-20,-17,-15,-12,-10,-8,-7,-5,-4,-3,-2,-1,0,0,1,2,3,4,5,7,8,10,12,15,17,20,23,27,30,-30,-26,-23,-20,-17,-14,-12,-10,-8,-6,-5,-4,-3,-2,-1,0,0,1,2,3,4,5,7,8,10,12,14,17,20,23,26,30,-29,-26,-22,-19,-16,-14,-11,-9,-8,-6,-5,-4,-3,-2,-1,0,0,1,2,3,4,5,6,8,10,12,14,16,19,22,26,29,-29,-25,-22,-19,-16,-13,-11,-9,-7,-6,-5,-3,-3,-2,-1,0,0,1,2,3,3,5,6,7,9,11,13,16,19,22,25,29,-28,-25,-21,-18,-16,-13,-11,-9,-7,-6,-4,-3,-2,-2,-1,0,0,1,2,2,3,4,6,7,9,11,13,16,18,21,25,28,-28,-24,-21,-18,-15,-13,-10,-8,-7,-5,-4,-3,-2,-2,-1,0,0,1,2,2,3,4,5,7,9,11,13,15,18,21,24,28,-28,-24,-21,-18,-15,-12,-10,-8,-7,-5,-4,-3,-2,-1,-1,0,0,1,1,2,3,4,5,7,8,10,12,15,18,21,24,27,-27,-24,-20,-17,-15,-12,-10,-8,-6,-5,-4,-3,-2,-1,-1,0,0,1,1,2,3,4,5,6,8,10,12,15,17,20,24,27,-27,-23,-20,-17,-14,-12,-10,-8,-6,-5,-4,-3,-2,-1,-1,0,0,1,1,2,3,4,5,6,8,10,12,14,17,20,23,27,-27,-23,-20,-17,-14,-12,-9,-8,-6,-5,-4,-3,-2,-1,-1,0,0,1,1,2,3,4,5,6,8,10,12,14,17,20,23,27,-26,-23,-20,-17,-14,-11,-9,-7,-6,-4,-3,-3,-2,-2,-1,0,0,1,2,2,3,3,5,6,7,9,11,14,17,20,23,26,-26,-23,-19,-16,-14,-11,-9,-7,-6,-4,-3,-3,-2,-2,-1,-1,1,1,2,2,3,3,4,6,7,9,11,14,17,20,23,26,-26,-23,-19,-16,-14,-11,-9,-7,-6,-4,-3,-3,-2,-2,-2,-1,1,2,2,2,3,3,4,6,7,9,11,14,16,19,23,26,-26,-23,-19,-16,-14,-11,-9,-7,-6,-4,-3,-3,-2,-2,-2,-1,1,2,2,2,3,3,4,6,7,9,11,14,16,19,23,26,-26,-22,-19,-16,-13,-11,-9,-7,-6,-4,-3,-3,-2,-2,-2,-2,2,2,2,2,3,3,4,6,7,9,11,14,16,19,23,26,-26,-22,-19,-16,-13,-11,-9,-7,-6,-4,-3,-3,-2,-2,-2,-2,2,2,2,2,3,3,4,6,7,9,11,14,16,19,23,26,-26,-23,-19,-16,-14,-11,-9,-7,-6,-4,-3,-3,-2,-2,-2,-1,1,2,2,2,3,3,4,6,7,9,11,14,16,19,23,26,-26,-23,-19,-16,-14,-11,-9,-7,-6,-4,-3,-3,-2,-2,-1,-1,1,2,2,2,3,3,4,6,7,9,11,14,16,19,23,26,-26,-23,-19,-16,-14,-11,-9,-7,-6,-4,-3,-3,-2,-2,-1,0,1,1,2,2,3,3,4,6,7,9,11,14,17,20,23,26,-26,-23,-20,-17,-14,-11,-9,-7,-6,-5,-3,-3,-2,-2,-1,0,0,1,2,2,3,3,5,6,7,9,12,14,17,20,23,26,-27,-23,-20,-17,-14,-12,-9,-8,-6,-5,-4,-3,-2,-1,-1,0,0,1,1,2,3,4,5,6,8,10,12,14,17,20,23,27,-27,-23,-20,-17,-14,-12,-10,-8,-6,-5,-4,-3,-2,-1,-1,0,0,1,1,2,3,4,5,6,8,10,12,14,17,20,23,27,-27,-24,-20,-17,-15,-12,-10,-8,-6,-5,-4,-3,-2,-1,-1,0,0,1,1,2,3,4,5,6,8,10,12,15,17,20,24,27,-28,-24,-21,-18,-15,-12,-10,-8,-7,-5,-4,-3,-2,-1,-1,0,0,1,1,2,3,4,5,7,8,10,12,15,18,21,24,27,-28,-24,-21,-18,-15,-13,-10,-9,-7,-5,-4,-3,-2,-2,-1,0,0,1,2,2,3,4,5,7,9,11,13,15,18,21,24,28,-28,-25,-21,-18,-16,-13,-11,-9,-7,-6,-4,-3,-2,-2,-1,0,0,1,2,2,3,4,6,7,9,11,13,16,18,21,25,28,-29,-25,-22,-19,-16,-13,-11,-9,-7,-6,-5,-3,-3,-2,-1,0,0,1,2,3,4,5,6,7,9,11,14,16,19,22,25,29,-29,-26,-22,-19,-16,-14,-12,-10,-8,-6,-5,-4,-3,-2,-1,0,0,1,2,3,4,5,6,8,10,12,14,16,19,22,26,29,-30,-26,-23,-20,-17,-14,-12,-10,-8,-7,-5,-4,-3,-2,-1,0,0,1,2,3,4,5,7,8,10,12,14,17,20,23,26,30,-30,-27,-23,-20,-17,-15,-12,-10,-8,-7,-5,-4,-3,-2,-1,0,0,1,2,3,4,5,7,9,10,12,15,17,20,23,27,30,-31,-27,-24,-21,-18,-15,-13,-11,-9,-7,-6,-4,-3,-2,-1,0,0,1,2,3,4,6,7,9,11,13,15,18,21,24,27,31
};

static int8_t _CALIBRATION_CAC_BY[1024]=
{
-17,-16,-15,-14,-13,-13,-12,-11,-10,-10,-9,-9,-8,-8,-8,-7,-7,-8,-8,-8,-9,-9,-10,-10,-11,-12,-13,-13,-14,-15,-16,-17,-16,-15,-14,-13,-12,-11,-11,-10,-9,-9,-8,-7,-7,-7,-7,-7,-7,-7,-7,-7,-8,-8,-9,-9,-10,-11,-11,-12,-13,-14,-15,-16,-15,-14,-13,-12,-11,-10,-10,-9,-8,-8,-7,-7,-6,-6,-6,-6,-6,-6,-6,-6,-7,-7,-8,-8,-9,-10,-10,-11,-12,-13,-14,-15,-13,-12,-12,-11,-10,-9,-9,-8,-7,-7,-6,-6,-5,-5,-5,-5,-5,-5,-5,-5,-6,-6,-7,-7,-8,-9,-9,-10,-11,-12,-12,-13,-12,-11,-10,-10,-9,-8,-8,-7,-6,-6,-5,-5,-5,-4,-4,-4,-4,-4,-4,-5,-5,-5,-6,-6,-7,-8,-8,-9,-10,-10,-11,-12,-11,-10,-9,-9,-8,-7,-7,-6,-6,-5,-5,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-5,-5,-6,-6,-7,-7,-8,-9,-9,-10,-11,-10,-9,-8,-8,-7,-6,-6,-5,-5,-4,-4,-4,-3,-3,-3,-3,-3,-3,-3,-3,-4,-4,-4,-5,-5,-6,-6,-7,-8,-8,-9,-10,-8,-8,-7,-7,-6,-6,-5,-5,-4,-4,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-4,-4,-5,-5,-6,-6,-7,-7,-8,-8,-7,-7,-6,-6,-5,-5,-4,-4,-4,-3,-3,-3,-2,-2,-2,-2,-2,-2,-2,-2,-3,-3,-3,-4,-4,-4,-5,-5,-6,-6,-7,-7,-6,-6,-5,-5,-5,-4,-4,-3,-3,-3,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-3,-3,-3,-4,-4,-5,-5,-5,-6,-6,-5,-5,-4,-4,-4,-3,-3,-3,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-3,-3,-3,-4,-4,-5,-5,-5,-4,-4,-4,-3,-3,-3,-2,-2,-2,-2,-2,-1,-1,-2,-2,-2,-2,-2,-2,-1,-1,-2,-2,-2,-2,-2,-3,-3,-3,-4,-4,-4,-3,-3,-3,-3,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0,0,-1,-1,-2,-2,-1,-1,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,2,2,2,2,2,1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,4,3,3,3,3,2,2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,2,2,3,3,3,3,4,4,4,5,5,5,4,4,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,4,4,4,5,5,5,6,6,6,5,5,4,4,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,4,4,5,5,6,6,6,8,7,6,6,5,5,4,4,4,3,3,3,3,2,2,2,2,2,2,3,3,3,3,4,4,4,5,5,6,6,7,8,9,8,7,7,6,6,5,5,4,4,3,3,3,3,3,3,3,3,3,3,3,4,4,4,5,5,6,6,7,7,8,9,10,9,8,8,7,7,6,5,5,4,4,4,4,3,3,3,3,3,3,4,4,4,4,5,5,6,7,7,8,8,9,10,11,10,9,9,8,7,7,6,6,5,5,4,4,4,4,4,4,4,4,4,4,5,5,6,6,7,7,8,9,9,10,11,12,11,11,10,9,8,8,7,6,6,5,5,5,5,4,4,4,4,5,5,5,5,6,6,7,8,8,9,10,11,11,12,13,13,12,11,10,9,9,8,7,7,6,6,6,5,5,5,5,5,5,6,6,6,7,7,8,9,9,10,11,12,13,13,15,14,13,12,11,10,10,9,8,8,7,7,6,6,6,6,6,6,6,6,7,7,8,8,9,10,10,11,12,13,14,15,16,15,14,13,12,12,11,10,9,9,8,8,7,7,7,7,7,7,7,7,8,8,9,9,10,11,12,12,13,14,15,16,17,16,15,14,13,13,12,11,10,10,9,8,8,8,8,7,7,8,8,8,8,9,10,10,11,12,13,13,14,15,16,17
};

static int16_t _CALIBRATION_AWB_RG_POS[15]=
{
1501,1672,1842,2013,2183,2354,2525,2695,2866,3037,3207,3378,3549,3719,3890
};
static int16_t _CALIBRATION_AWB_BG_POS[15]=
{
991,1154,1316,1479,1641,1804,1966,2129,2291,2454,2616,2779,2941,3104,3266
};

static int16_t _CALIBRATION_AWB_MESH_DIST_TAB[15][15] =
{
{-160,-145,-131,-117,-103,-90,-77,-65,-54,-43,-33,-24,-15,-6,0,},
{-146,-131,-116,-101,-87,-74,-61,-48,-37,-25,-15,-5,3,12,20,},
{-132,-116,-101,-86,-71,-57,-44,-31,-19,-8,2,12,22,31,39,},
{-118,-102,-86,-71,-56,-42,-28,-15,-2,9,20,30,40,49,58,},
{-105,-89,-72,-57,-41,-26,-12,1,14,26,38,48,59,68,77,},
{-93,-76,-59,-43,-27,-11,2,17,30,43,55,66,77,87,96,},
{-80,-63,-46,-29,-13,2,17,32,46,60,72,84,95,105,114,},
{-69,-51,-33,-16,1,16,32,47,62,76,89,101,113,123,133,},
{-58,-39,-21,-4,13,29,46,62,77,92,105,118,130,141,151,},
{-47,-28,-10,7,25,42,59,76,92,107,121,135,147,159,170,},
{-37,-18,0,18,36,54,72,89,105,121,137,151,164,177,188,},
{-27,-8,10,29,47,66,84,101,119,135,151,166,180,194,205,},
{-19,0,19,39,58,76,95,113,131,148,165,181,196,210,223,},
{-10,9,28,48,67,86,105,124,142,160,178,195,211,226,239,},
{-2,17,37,57,76,96,115,134,153,172,190,208,225,241,256,},
};

static int16_t _CALIBRATION_AWB_MESH_CT_TAB[15][15] =
{
{6326,5602,4954,4376,3874,3441,3082,2798,2585,2445,2379,2386,2465,2617,2843,},
{6443,5718,5071,4492,3990,3558,3199,2914,2702,2562,2496,2502,2582,2733,2959,},
{6586,5862,5214,4636,4133,3701,3342,3058,2845,2705,2639,2645,2725,2877,3102,},
{6758,6034,5386,4808,4305,3873,3514,3230,3017,2877,2811,2817,2897,3049,3274,},
{6956,6232,5584,5006,4504,4072,3713,3428,3215,3076,3009,3016,3095,3247,3473,},
{7184,6460,5812,5234,4731,4299,3940,3656,3443,3303,3237,3243,3323,3475,3700,},
{7438,6713,6066,5487,4985,4553,4194,3910,3697,3557,3491,3497,3577,3728,3954,},
{7721,6996,6349,5771,5268,4836,4477,4193,3980,3840,3774,3780,3860,4012,4237,},
{8030,7305,6658,6079,5577,5145,4786,4502,4289,4149,4083,4089,4169,4320,4546,},
{8368,7644,6996,6418,5916,5484,5125,4840,4627,4488,4421,4428,4507,4659,4885,},
{8733,8008,7361,6782,6280,5848,5489,5204,4991,4852,4786,4792,4872,5023,5249,},
{9127,8402,7755,7177,6674,6242,5883,5599,5386,5246,5180,5186,5266,5418,5643,},
{9546,8822,8174,7596,7094,6661,6302,6018,5805,5665,5599,5606,5685,5837,6063,},
{9996,9272,8624,8046,7543,7111,6752,6468,6255,6115,6049,6055,6135,6287,6512,},
{10471,9746,9099,8520,8018,7586,7227,6942,6730,6590,6524,6530,6610,6761,6987,},
};

//_CALIBRATION_AWB_CT_RG_CURVE
static int32_t _CALIBRATION_AWB_CT_RG_CURVE[4] = {5460,-943,57,0};

//_CALIBRATION_AWB_CT_BG_CURVE
static int32_t _CALIBRATION_AWB_CT_BG_CURVE[4] = {33,509,-19,0};

//CALIBRATION_AWB_WB_GOLDEN_D50
static int16_t _CALIBRATION_AWB_WB_GOLDEN_D50[] = {2168,2146};

//CALIBRATION_AWB_WB_OTP_D50
static int16_t _CALIBRATION_AWB_WB_OTP_D50[] = {2168,2146};

//Noise reduce cabliration parameters
static uint16_t _CALIBRATION_NOISE_PROFILE[9][16] =
{
{0,9,23,31,34,36,37,38,37,35,32,29,24,18,11,3,},
{0,12,32,43,46,49,50,51,50,47,44,39,33,26,18,8,},
{0,19,45,61,66,69,70,71,69,66,62,56,48,39,29,17,},
{0,28,66,86,91,96,98,99,97,93,87,79,69,57,43,26,},
{0,45,95,127,135,139,142,141,138,133,124,114,100,84,65,44,},
{0,74,139,177,187,195,199,200,196,189,178,163,144,121,95,65,},
{9,130,211,261,274,283,287,285,279,267,250,228,200,168,130,87,},
{73,218,322,372,384,404,415,417,409,392,365,330,285,231,167,95,},
{73,218,322,372,384,404,415,417,409,392,365,330,285,231,167,95,},
};

static uint8_t _CALIBRATION_FPNR[2048*2*5] = {0};

//aisp_awb_info_t
static uint32_t _CALIBRATION_AWB_PRESET[6] =
{
    0,
    457,    //awb_sys_r_gain;
    256,    //awb_sys_g_gain;
    436,    //awb_sys_b_gain;
    5563,   //awb_sys_ct;
    20,     //awb_sys_cdiff;
};

static LookupTable calibration_top_ctl = {.ptr = _CALIBRATION_TOP_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_TOP_CTL ) / sizeof( _CALIBRATION_TOP_CTL[0] ), .width = sizeof( _CALIBRATION_TOP_CTL[0] )};
static LookupTable calibration_awb_ctl = {.ptr = _CALIBRATION_AWB_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_AWB_CTL ) / sizeof( _CALIBRATION_AWB_CTL[0] ), .width = sizeof( _CALIBRATION_AWB_CTL[0] )};
static LookupTable calibration_res_ctl = {.ptr = _CALIBRATION_RES_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_RES_CTL ) / sizeof( _CALIBRATION_RES_CTL[0] ), .width = sizeof( _CALIBRATION_RES_CTL[0] )};
static LookupTable calibration_awb_ct_pos = { .ptr = _CALIBRATION_AWB_CT_POS, .rows = 1, .cols = sizeof(_CALIBRATION_AWB_CT_POS) / sizeof(_CALIBRATION_AWB_CT_POS[0]), .width = sizeof(_CALIBRATION_AWB_CT_POS[0] ) };
static LookupTable calibration_awb_ct_rg_compensation = { .ptr = _CALIBRATION_AWB_CT_RG_COMPENSATION, .rows = 1, .cols = sizeof( _CALIBRATION_AWB_CT_RG_COMPENSATION ) / sizeof( _CALIBRATION_AWB_CT_RG_COMPENSATION[0] ), .width = sizeof( _CALIBRATION_AWB_CT_RG_COMPENSATION[0] )};
static LookupTable calibration_awb_ct_bg_compensation = { .ptr = _CALIBRATION_AWB_CT_BG_COMPENSATION, .rows = 1, .cols = sizeof(_CALIBRATION_AWB_CT_BG_COMPENSATION) / sizeof(_CALIBRATION_AWB_CT_BG_COMPENSATION[0]), .width = sizeof(_CALIBRATION_AWB_CT_BG_COMPENSATION[0] ) };
static LookupTable calibration_awb_ct_wgt = { .ptr = _CALIBRATION_AWB_CT_WGT, .rows = 1, .cols = sizeof( _CALIBRATION_AWB_CT_WGT ) / sizeof( _CALIBRATION_AWB_CT_WGT[0] ), .width = sizeof( _CALIBRATION_AWB_CT_WGT[0] )};
static LookupTable calibration_awb_ct_dyn_cvrange = { .ptr = _CALIBRATION_AWB_CT_DYN_CVRANGE, .rows = sizeof(_CALIBRATION_AWB_CT_DYN_CVRANGE) / sizeof(_CALIBRATION_AWB_CT_DYN_CVRANGE[0]), .cols = sizeof(_CALIBRATION_AWB_CT_DYN_CVRANGE[0]) / sizeof(_CALIBRATION_AWB_CT_DYN_CVRANGE[0][0]), .width = sizeof(_CALIBRATION_AWB_CT_DYN_CVRANGE[0][0] ) };
static LookupTable calibration_ae_ctl = {.ptr = _CALIBRATION_AE_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_AE_CTL ) / sizeof( _CALIBRATION_AE_CTL[0] ), .width = sizeof( _CALIBRATION_AE_CTL[0] )};
static LookupTable calibration_ae_corr_lut = {.ptr = _CALIBRATION_AE_CORR_LUT, .rows = 1, .cols = sizeof( _CALIBRATION_AE_CORR_LUT ) / sizeof( _CALIBRATION_AE_CORR_LUT[0] ), .width = sizeof( _CALIBRATION_AE_CORR_LUT[0] )};
static LookupTable calibration_ae_corr_pos_lut = {.ptr = _CALIBRATION_AE_CORR_POS_LUT, .rows = 1, .cols = sizeof( _CALIBRATION_AE_CORR_POS_LUT ) / sizeof( _CALIBRATION_AE_CORR_POS_LUT[0] ), .width = sizeof( _CALIBRATION_AE_CORR_POS_LUT[0] )};
static LookupTable calibration_ae_route = {.ptr = _CALIBRATION_AE_ROUTE, .rows = 1, .cols = sizeof( _CALIBRATION_AE_ROUTE ) / sizeof( _CALIBRATION_AE_ROUTE[0] ), .width = sizeof( _CALIBRATION_AE_ROUTE[0] )};
static LookupTable calibration_ae_weight_h = {.ptr = _CALIBRATION_AE_WEIGHT_H, .rows = 1, .cols = sizeof( _CALIBRATION_AE_WEIGHT_H ) / sizeof( _CALIBRATION_AE_WEIGHT_H[0] ), .width = sizeof( _CALIBRATION_AE_WEIGHT_H[0] )};
static LookupTable calibration_ae_weight_v = {.ptr = _CALIBRATION_AE_WEIGHT_V, .rows = 1, .cols = sizeof( _CALIBRATION_AE_WEIGHT_V ) / sizeof( _CALIBRATION_AE_WEIGHT_V[0] ), .width = sizeof( _CALIBRATION_AE_WEIGHT_V[0] )};
static LookupTable calibration_daynight_detect = {.ptr = _CALIBRATION_DAYNIGHT_DETECT, .rows = 1, .cols = sizeof( _CALIBRATION_DAYNIGHT_DETECT ) / sizeof( _CALIBRATION_DAYNIGHT_DETECT[0] ), .width = sizeof( _CALIBRATION_DAYNIGHT_DETECT[0] )};
static LookupTable calibration_af_ctl = {.ptr = _CALIBRATION_AF_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_AF_CTL ) / sizeof( _CALIBRATION_AF_CTL[0] ), .width = sizeof( _CALIBRATION_AF_CTL[0] )};
static LookupTable calibration_af_weight_h = {.ptr = _CALIBRATION_AF_WEIGHT_H, .rows = 1, .cols = sizeof( _CALIBRATION_AF_WEIGHT_H ) / sizeof( _CALIBRATION_AF_WEIGHT_H[0] ), .width = sizeof( _CALIBRATION_AF_WEIGHT_H[0] )};
static LookupTable calibration_af_weight_v = {.ptr = _CALIBRATION_AF_WEIGHT_V, .rows = 1, .cols = sizeof( _CALIBRATION_AF_WEIGHT_V ) / sizeof( _CALIBRATION_AF_WEIGHT_V[0] ), .width = sizeof( _CALIBRATION_AF_WEIGHT_V[0] )};
static LookupTable calibration_flicker_ctl = {.ptr = _CALIBRATION_FLICKER_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_FLICKER_CTL ) / sizeof( _CALIBRATION_FLICKER_CTL[0] ), .width = sizeof( _CALIBRATION_FLICKER_CTL[0] )};
static LookupTable calibration_gtm = { .ptr = _CALIBRATION_GTM, .rows = 1, .cols = sizeof( _CALIBRATION_GTM ) / sizeof( _CALIBRATION_GTM[0] ), .width = sizeof( _CALIBRATION_GTM[0] )};
static LookupTable calibration_ge_adj = { .ptr = _CALIBRATION_GE_ADJ, .rows = sizeof( _CALIBRATION_DPC_ADJ ) / sizeof( _CALIBRATION_DPC_ADJ[0] ), .cols = sizeof( _CALIBRATION_GE_ADJ[0] ) / sizeof( _CALIBRATION_GE_ADJ[0][0] ), .width = sizeof( _CALIBRATION_GE_ADJ[0][0] )};
static LookupTable calibration_ge_s_adj = { .ptr = _CALIBRATION_GE_S_ADJ, .rows = sizeof( _CALIBRATION_GE_S_ADJ ) / sizeof( _CALIBRATION_GE_S_ADJ[0] ), .cols = sizeof( _CALIBRATION_GE_S_ADJ[0] ) / sizeof( _CALIBRATION_GE_S_ADJ[0][0] ), .width = sizeof( _CALIBRATION_GE_S_ADJ[0][0] )};
static LookupTable calibration_dpc_ctl = { .ptr = _CALIBRATION_DPC_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_DPC_CTL ) / sizeof( _CALIBRATION_DPC_CTL[0] ), .width = sizeof( _CALIBRATION_DPC_CTL[0] )};
static LookupTable calibration_dpc_s_ctl = { .ptr = _CALIBRATION_DPC_S_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_DPC_S_CTL ) / sizeof( _CALIBRATION_DPC_S_CTL[0] ), .width = sizeof( _CALIBRATION_DPC_S_CTL[0] )};
static LookupTable calibration_dpc_adj = { .ptr = _CALIBRATION_DPC_ADJ, .rows = sizeof( _CALIBRATION_DPC_ADJ ) / sizeof( _CALIBRATION_DPC_ADJ[0] ), .cols = sizeof( _CALIBRATION_DPC_ADJ[0] ) / sizeof( _CALIBRATION_DPC_ADJ[0][0] ), .width = sizeof( _CALIBRATION_DPC_ADJ[0][0] )};
static LookupTable calibration_dpc_s_adj = { .ptr = _CALIBRATION_DPC_S_ADJ, .rows = sizeof( _CALIBRATION_DPC_S_ADJ ) / sizeof( _CALIBRATION_DPC_S_ADJ[0] ), .cols = sizeof( _CALIBRATION_DPC_S_ADJ[0] ) / sizeof( _CALIBRATION_DPC_S_ADJ[0][0] ), .width = sizeof( _CALIBRATION_DPC_S_ADJ[0][0] )};
static LookupTable calibration_wdr_ctl = {.ptr = _CALIBRATION_WDR_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_WDR_CTL ) / sizeof( _CALIBRATION_WDR_CTL[0] ), .width = sizeof( _CALIBRATION_WDR_CTL[0] )};
static LookupTable calibration_wdr_adjust = {.ptr = _CALIBRATION_WDR_ADJUST, .rows = sizeof( _CALIBRATION_WDR_ADJUST ) / sizeof( _CALIBRATION_WDR_ADJUST[0] ), .cols = sizeof( _CALIBRATION_WDR_ADJUST[0] ) / sizeof( _CALIBRATION_WDR_ADJUST[0][0] ), .width = sizeof( _CALIBRATION_WDR_ADJUST[0][0] )};
static LookupTable calibration_wdr_mdetc_loweight = { .ptr = _CALIBRATION_WDR_MDETC_LOWEIGHT, .rows = sizeof( _CALIBRATION_WDR_MDETC_LOWEIGHT ) / sizeof( _CALIBRATION_WDR_MDETC_LOWEIGHT[0]), .cols = sizeof( _CALIBRATION_WDR_MDETC_LOWEIGHT[0] ) / sizeof( _CALIBRATION_WDR_MDETC_LOWEIGHT[0][0] ), .width = sizeof( _CALIBRATION_WDR_MDETC_LOWEIGHT[0][0] )};
static LookupTable calibration_wdr_mdetc_hiweight = { .ptr = _CALIBRATION_WDR_MDETC_HIWEIGHT, .rows = sizeof( _CALIBRATION_WDR_MDETC_HIWEIGHT ) / sizeof( _CALIBRATION_WDR_MDETC_HIWEIGHT[0]), .cols = sizeof( _CALIBRATION_WDR_MDETC_HIWEIGHT[0] ) / sizeof( _CALIBRATION_WDR_MDETC_HIWEIGHT[0][0] ), .width = sizeof( _CALIBRATION_WDR_MDETC_HIWEIGHT[0][0] )};
static LookupTable calibration_rawcnr_ctl = { .ptr = _CALIBRATION_RAWCNR_CTL, .rows = 1, .cols = sizeof(_CALIBRATION_RAWCNR_CTL) / sizeof(_CALIBRATION_RAWCNR_CTL[0]), .width = sizeof(_CALIBRATION_RAWCNR_CTL[0] ) };
static LookupTable calibration_rawcnr_adj = { .ptr = _CALIBRATION_RAWCNR_ADJ, .rows = sizeof(_CALIBRATION_RAWCNR_ADJ) / sizeof(_CALIBRATION_RAWCNR_ADJ[0]), .cols = sizeof(_CALIBRATION_RAWCNR_ADJ[0]) / sizeof(_CALIBRATION_RAWCNR_ADJ[0][0]), .width = sizeof(_CALIBRATION_RAWCNR_ADJ[0][0] ) };
static LookupTable calibration_rawcnr_meta_gain_lut = { .ptr = _CALIBRATION_RAWCNR_META_GAIN_LUT, .rows = sizeof( _CALIBRATION_RAWCNR_META_GAIN_LUT ) / sizeof( _CALIBRATION_RAWCNR_META_GAIN_LUT[0] ), .cols = sizeof( _CALIBRATION_RAWCNR_META_GAIN_LUT[0] ) / sizeof( _CALIBRATION_RAWCNR_META_GAIN_LUT[0][0] ), .width = sizeof( _CALIBRATION_RAWCNR_META_GAIN_LUT[0][0] )};
static LookupTable calibration_rawcnr_sps_csig_weight5x5 = { .ptr = _CALIBRATION_RAWCNR_SPS_CSIG_WEIGHT5X5, .rows = sizeof( _CALIBRATION_RAWCNR_SPS_CSIG_WEIGHT5X5 ) / sizeof( _CALIBRATION_RAWCNR_SPS_CSIG_WEIGHT5X5[0] ), .cols = sizeof( _CALIBRATION_RAWCNR_SPS_CSIG_WEIGHT5X5[0] ) / sizeof( _CALIBRATION_RAWCNR_SPS_CSIG_WEIGHT5X5[0][0] ), .width = sizeof( _CALIBRATION_RAWCNR_SPS_CSIG_WEIGHT5X5[0][0] )};
static LookupTable calibration_snr_ctl = { .ptr = _CALIBRATION_SNR_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_SNR_CTL ) / sizeof( _CALIBRATION_SNR_CTL[0] ), .width = sizeof( _CALIBRATION_SNR_CTL[0] )};
static LookupTable calibration_snr_glb_adj = { .ptr = _CALIBRATION_SNR_GLB_ADJ, .rows = sizeof( _CALIBRATION_SNR_GLB_ADJ ) / sizeof( _CALIBRATION_SNR_GLB_ADJ[0] ), .cols = sizeof( _CALIBRATION_SNR_GLB_ADJ[0] ) / sizeof( _CALIBRATION_SNR_GLB_ADJ[0][0] ), .width = sizeof( _CALIBRATION_SNR_GLB_ADJ[0][0] )};
static LookupTable calibration_snr_adj = { .ptr = _CALIBRATION_SNR_ADJ, .rows = sizeof( _CALIBRATION_SNR_ADJ ) / sizeof( _CALIBRATION_SNR_ADJ[0] ), .cols = sizeof( _CALIBRATION_SNR_ADJ[0] ) / sizeof( _CALIBRATION_SNR_ADJ[0][0] ), .width = sizeof( _CALIBRATION_SNR_ADJ[0][0] )};
static LookupTable calibration_snr_cur_wt = { .ptr = _CALIBRATION_SNR_CUR_WT, .rows = sizeof( _CALIBRATION_SNR_CUR_WT ) / sizeof( _CALIBRATION_SNR_CUR_WT[0] ), .cols = sizeof( _CALIBRATION_SNR_CUR_WT[0] ) / sizeof( _CALIBRATION_SNR_CUR_WT[0][0] ), .width = sizeof( _CALIBRATION_SNR_CUR_WT[0][0] )};
static LookupTable calibration_snr_wt_luma_gain = { .ptr = _CALIBRATION_SNR_WT_LUMA_GAIN, .rows = sizeof( _CALIBRATION_SNR_WT_LUMA_GAIN ) / sizeof( _CALIBRATION_SNR_WT_LUMA_GAIN[0] ), .cols = sizeof( _CALIBRATION_SNR_WT_LUMA_GAIN[0] ) / sizeof( _CALIBRATION_SNR_WT_LUMA_GAIN[0][0] ), .width = sizeof( _CALIBRATION_SNR_WT_LUMA_GAIN[0][0] )};
static LookupTable calibration_snr_sad_meta2alp = { .ptr = _CALIBRATION_SNR_SAD_META2ALP, .rows = sizeof( _CALIBRATION_SNR_SAD_META2ALP ) / sizeof( _CALIBRATION_SNR_SAD_META2ALP[0] ), .cols = sizeof( _CALIBRATION_SNR_SAD_META2ALP[0] ) / sizeof( _CALIBRATION_SNR_SAD_META2ALP[0][0] ), .width = sizeof( _CALIBRATION_SNR_SAD_META2ALP[0][0] )};
static LookupTable calibration_snr_meta_adj = { .ptr = _CALIBRATION_SNR_META_ADJ, .rows = sizeof( _CALIBRATION_SNR_META_ADJ ) / sizeof( _CALIBRATION_SNR_META_ADJ[0] ), .cols = sizeof( _CALIBRATION_SNR_META_ADJ[0] ) / sizeof( _CALIBRATION_SNR_META_ADJ[0][0] ), .width = sizeof( _CALIBRATION_SNR_META_ADJ[0][0] )};
static LookupTable calibration_snr_phs = { .ptr = _CALIBRATION_SNR_PHS, .rows = sizeof(_CALIBRATION_SNR_PHS) / sizeof(_CALIBRATION_SNR_PHS[0]), .cols = sizeof(_CALIBRATION_SNR_PHS[0]) / sizeof(_CALIBRATION_SNR_PHS[0][0]), .width = sizeof(_CALIBRATION_SNR_PHS[0][0] ) };
static LookupTable calibration_nr_rad_lut65 = { .ptr = _CALIBRATION_NR_RAD_LUT65, .rows = sizeof(_CALIBRATION_NR_RAD_LUT65) / sizeof(_CALIBRATION_NR_RAD_LUT65[0]), .cols = sizeof(_CALIBRATION_NR_RAD_LUT65[0]) / sizeof(_CALIBRATION_NR_RAD_LUT65[0][0]), .width = sizeof(_CALIBRATION_NR_RAD_LUT65[0][0] ) };
static LookupTable calibration_pst_snr_adj = { .ptr = _CALIBRATION_PST_SNR_ADJ, .rows = sizeof(_CALIBRATION_PST_SNR_ADJ) / sizeof(_CALIBRATION_PST_SNR_ADJ[0]), .cols = sizeof(_CALIBRATION_PST_SNR_ADJ[0]) / sizeof(_CALIBRATION_PST_SNR_ADJ[0][0]), .width = sizeof(_CALIBRATION_PST_SNR_ADJ[0][0] ) };
static LookupTable calibration_tnr_ctl = { .ptr = _CALIBRATION_TNR_CTL, .rows = 1, .cols = sizeof(_CALIBRATION_TNR_CTL) / sizeof(_CALIBRATION_TNR_CTL[0]), .width = sizeof(_CALIBRATION_TNR_CTL[0] ) };
static LookupTable calibration_tnr_glb_adj = { .ptr = _CALIBRATION_TNR_GLB_ADJ, .rows = sizeof(_CALIBRATION_TNR_GLB_ADJ) / sizeof(_CALIBRATION_TNR_GLB_ADJ[0]), .cols = sizeof(_CALIBRATION_TNR_GLB_ADJ[0]) / sizeof(_CALIBRATION_TNR_GLB_ADJ[0][0]), .width = sizeof(_CALIBRATION_TNR_GLB_ADJ[0][0] ) };
static LookupTable calibration_tnr_adj = { .ptr = _CALIBRATION_TNR_ADJ, .rows = sizeof(_CALIBRATION_TNR_ADJ) / sizeof(_CALIBRATION_TNR_ADJ[0]), .cols = sizeof(_CALIBRATION_TNR_ADJ[0]) / sizeof(_CALIBRATION_TNR_ADJ[0][0]), .width = sizeof(_CALIBRATION_TNR_ADJ[0][0] ) };
static LookupTable calibration_tnr_sad2alpha = { .ptr = _CALIBRATION_TNR_SAD2ALPHA, .rows = sizeof(_CALIBRATION_TNR_SAD2ALPHA) / sizeof(_CALIBRATION_TNR_SAD2ALPHA[0]), .cols = sizeof(_CALIBRATION_TNR_SAD2ALPHA[0]) / sizeof(_CALIBRATION_TNR_SAD2ALPHA[0][0]), .width = sizeof(_CALIBRATION_TNR_SAD2ALPHA[0][0] ) };
static LookupTable calibration_mc_meta2alpha = { .ptr = _CALIBRATION_MC_META2ALPHA, .rows = sizeof(_CALIBRATION_MC_META2ALPHA) / sizeof(_CALIBRATION_MC_META2ALPHA[0]), .cols = sizeof(_CALIBRATION_MC_META2ALPHA[0]) / sizeof(_CALIBRATION_MC_META2ALPHA[0][0]), .width = sizeof(_CALIBRATION_MC_META2ALPHA[0][0] ) };
static LookupTable calibration_pst_tnr_alp_lut = { .ptr = _CALIBRATION_PST_TNR_ALP_LUT, .rows = sizeof(_CALIBRATION_PST_TNR_ALP_LUT) / sizeof(_CALIBRATION_PST_TNR_ALP_LUT[0]), .cols = sizeof(_CALIBRATION_PST_TNR_ALP_LUT[0]) / sizeof(_CALIBRATION_PST_TNR_ALP_LUT[0][0]), .width = sizeof(_CALIBRATION_PST_TNR_ALP_LUT[0][0] ) };
static LookupTable calibration_lens_shading_ct_correct = { .ptr = _CALIBRATION_LENS_SHADING_CT_CORRECT, .rows = 1, .cols = sizeof( _CALIBRATION_LENS_SHADING_CT_CORRECT ) / sizeof( _CALIBRATION_LENS_SHADING_CT_CORRECT[0] ), .width = sizeof( _CALIBRATION_LENS_SHADING_CT_CORRECT[0] )};
static LookupTable calibration_lens_shading_adj = {.ptr = _CALIBRATION_LENS_SHADING_ADJ, .rows = sizeof( _CALIBRATION_LENS_SHADING_ADJ ) / sizeof( _CALIBRATION_LENS_SHADING_ADJ[0] ), .cols = sizeof( _CALIBRATION_LENS_SHADING_ADJ[0] ) / sizeof( _CALIBRATION_LENS_SHADING_ADJ[0][0] ), .width = sizeof( _CALIBRATION_LENS_SHADING_ADJ[0][0] )};
static LookupTable calibration_dms_adj = {.ptr = _CALIBRATION_DMS_ADJ, .rows = sizeof( _CALIBRATION_DMS_ADJ ) / sizeof( _CALIBRATION_DMS_ADJ[0] ), .cols = sizeof( _CALIBRATION_DMS_ADJ[0] ) / sizeof( _CALIBRATION_DMS_ADJ[0][0] ), .width = sizeof( _CALIBRATION_DMS_ADJ[0][0] )};
static LookupTable calibration_ccm_adj = {.ptr = _CALIBRATION_CCM_ADJ, .rows = sizeof( _CALIBRATION_CCM_ADJ ) / sizeof( _CALIBRATION_CCM_ADJ[0] ), .cols = sizeof( _CALIBRATION_CCM_ADJ[0] ) / sizeof( _CALIBRATION_CCM_ADJ[0][0] ), .width = sizeof( _CALIBRATION_CCM_ADJ[0][0] )};
static LookupTable calibration_cnr_ctl = {.ptr = _CALIBRATION_CNR_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_CNR_CTL ) / sizeof( _CALIBRATION_CNR_CTL[0] ), .width = sizeof( _CALIBRATION_CNR_CTL[0] )};
static LookupTable calibration_cnr_adj = {.ptr = _CALIBRATION_CNR_ADJ, .rows = sizeof( _CALIBRATION_CNR_ADJ ) / sizeof( _CALIBRATION_CNR_ADJ[0] ), .cols = sizeof( _CALIBRATION_CNR_ADJ[0] ) / sizeof( _CALIBRATION_CNR_ADJ[0][0] ), .width = sizeof( _CALIBRATION_CNR_ADJ[0][0] )};
static LookupTable calibration_purple_ctl = {.ptr = _CALIBRATION_PURPLE_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_PURPLE_CTL ) / sizeof( _CALIBRATION_PURPLE_CTL[0] ), .width = sizeof( _CALIBRATION_PURPLE_CTL[0] )};
static LookupTable calibration_purple_adj = {.ptr = _CALIBRATION_PURPLE_ADJ, .rows = sizeof( _CALIBRATION_PURPLE_ADJ ) / sizeof( _CALIBRATION_PURPLE_ADJ[0] ), .cols = sizeof( _CALIBRATION_PURPLE_ADJ[0] ) / sizeof( _CALIBRATION_PURPLE_ADJ[0][0] ), .width = sizeof( _CALIBRATION_PURPLE_ADJ[0][0] )};
static LookupTable calibration_ltm_ctl = {.ptr = _CALIBRATION_LTM_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_LTM_CTL ) / sizeof( _CALIBRATION_LTM_CTL[0] ), .width = sizeof( _CALIBRATION_LTM_CTL[0] )};
static LookupTable calibration_ltm_contrast = {.ptr = _CALIBRATION_LTM_CONTRAST, .rows = 1, .cols = sizeof( _CALIBRATION_LTM_CONTRAST ) / sizeof( _CALIBRATION_LTM_CONTRAST[0] ), .width = sizeof( _CALIBRATION_LTM_CONTRAST[0] )};
static LookupTable calibration_ltm_lo_hi_gm = {.ptr = _CALIBRATION_LTM_LO_HI_GM, .rows = sizeof( _CALIBRATION_LTM_LO_HI_GM ) / sizeof( _CALIBRATION_LTM_LO_HI_GM[0] ), .cols = sizeof( _CALIBRATION_LTM_LO_HI_GM[0] ) / sizeof( _CALIBRATION_LTM_LO_HI_GM[0][0] ), .width = sizeof( _CALIBRATION_LTM_LO_HI_GM[0][0] )};
static LookupTable calibration_lc_strength = {.ptr = _CALIBRATION_LC_STRENGTH, .rows = sizeof( _CALIBRATION_LC_STRENGTH ) / sizeof( _CALIBRATION_LC_STRENGTH[0] ), .cols = sizeof( _CALIBRATION_LC_STRENGTH[0] ) / sizeof( _CALIBRATION_LC_STRENGTH[0][0] ), .width = sizeof( _CALIBRATION_LC_STRENGTH[0][0] )};
static LookupTable calibration_dnlp_strength = {.ptr = _CALIBRATION_DNLP_STRENGTH, .rows = 1, .cols = sizeof( _CALIBRATION_DNLP_STRENGTH ) / sizeof( _CALIBRATION_DNLP_STRENGTH[0] ), .width = sizeof( _CALIBRATION_DNLP_STRENGTH[0] )};
static LookupTable calibration_dhz_strength = {.ptr = _CALIBRATION_DHZ_STRENGTH, .rows = 1, .cols = sizeof( _CALIBRATION_DHZ_STRENGTH ) / sizeof( _CALIBRATION_DHZ_STRENGTH[0] ), .width = sizeof( _CALIBRATION_DHZ_STRENGTH[0] )};
static LookupTable calibration_dnlp_scurv_low = {.ptr = _CALIBRATION_DNLP_SCURV_LOW, .rows = 1, .cols = sizeof( _CALIBRATION_DNLP_SCURV_LOW ) / sizeof( _CALIBRATION_DNLP_SCURV_LOW[0] ), .width = sizeof( _CALIBRATION_DNLP_SCURV_LOW[0] )};
static LookupTable calibration_dnlp_scurv_mid1 = {.ptr = _CALIBRATION_DNLP_SCURV_MID1, .rows = 1, .cols = sizeof( _CALIBRATION_DNLP_SCURV_MID1 ) / sizeof( _CALIBRATION_DNLP_SCURV_MID1[0] ), .width = sizeof( _CALIBRATION_DNLP_SCURV_MID1[0] )};
static LookupTable calibration_dnlp_scurv_mid2 = {.ptr = _CALIBRATION_DNLP_SCURV_MID2, .rows = 1, .cols = sizeof( _CALIBRATION_DNLP_SCURV_MID2 ) / sizeof( _CALIBRATION_DNLP_SCURV_MID2[0] ), .width = sizeof( _CALIBRATION_DNLP_SCURV_MID2[0] )};
static LookupTable calibration_dnlp_scurv_hgh1 = {.ptr = _CALIBRATION_DNLP_SCURV_HGH1, .rows = 1, .cols = sizeof( _CALIBRATION_DNLP_SCURV_HGH1 ) / sizeof( _CALIBRATION_DNLP_SCURV_HGH1[0] ), .width = sizeof( _CALIBRATION_DNLP_SCURV_HGH1[0] )};
static LookupTable calibration_dnlp_scurv_hgh2 = {.ptr = _CALIBRATION_DNLP_SCURV_HGH2, .rows = 1, .cols = sizeof( _CALIBRATION_DNLP_SCURV_HGH2 ) / sizeof( _CALIBRATION_DNLP_SCURV_HGH2[0] ), .width = sizeof( _CALIBRATION_DNLP_SCURV_HGH2[0] )};
static LookupTable calibration_ltm_sharp_adj = {.ptr = _CALIBRATION_LTM_SHARP_ADJ, .rows = sizeof( _CALIBRATION_LTM_SHARP_ADJ ) / sizeof( _CALIBRATION_LTM_SHARP_ADJ[0] ), .cols = sizeof( _CALIBRATION_LTM_SHARP_ADJ[0] ) / sizeof( _CALIBRATION_LTM_SHARP_ADJ[0][0] ), .width = sizeof( _CALIBRATION_LTM_SHARP_ADJ[0][0] )};
static LookupTable calibration_ltm_satur_lut = { .ptr = _CALIBRATION_LTM_SATUR_LUT, .rows = 1, .cols = sizeof( _CALIBRATION_LTM_SATUR_LUT ) / sizeof( _CALIBRATION_LTM_SATUR_LUT[0] ), .width = sizeof( _CALIBRATION_LTM_SATUR_LUT[0] )};
static LookupTable calibration_lc_ctl = {.ptr = _CALIBRATION_LC_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_LC_CTL ) / sizeof( _CALIBRATION_LC_CTL[0] ), .width = sizeof( _CALIBRATION_LC_CTL[0] )};
static LookupTable calibration_lc_satur_lut = { .ptr = _CALIBRATION_LC_SATUR_LUT, .rows = 1, .cols = sizeof( _CALIBRATION_LC_SATUR_LUT ) / sizeof( _CALIBRATION_LC_SATUR_LUT[0] ), .width = sizeof( _CALIBRATION_LC_SATUR_LUT[0] )};
static LookupTable calibration_dnlp_ctl = {.ptr = _CALIBRATION_DNLP_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_DNLP_CTL ) / sizeof( _CALIBRATION_DNLP_CTL[0] ), .width = sizeof( _CALIBRATION_DNLP_CTL[0] )};
static LookupTable calibration_dhz_ctl = {.ptr = _CALIBRATION_DHZ_CTL, .rows = 1, .cols = sizeof( _CALIBRATION_DHZ_CTL ) / sizeof( _CALIBRATION_DHZ_CTL[0] ), .width = sizeof( _CALIBRATION_DHZ_CTL[0] )};
static LookupTable calibration_peaking_ctl = { .ptr = _CALIBRATION_PEAKING_CTL, .rows = 1, .cols = sizeof(_CALIBRATION_PEAKING_CTL) / sizeof(_CALIBRATION_PEAKING_CTL[0]), .width = sizeof(_CALIBRATION_PEAKING_CTL[0] ) };
static LookupTable calibration_peaking_adjust = { .ptr = _CALIBRATION_PEAKING_ADJUST, .rows = sizeof(_CALIBRATION_PEAKING_ADJUST) / sizeof(_CALIBRATION_PEAKING_ADJUST[0]), .cols = sizeof(_CALIBRATION_PEAKING_ADJUST[0]) / sizeof(_CALIBRATION_PEAKING_ADJUST[0][0]), .width = sizeof(_CALIBRATION_PEAKING_ADJUST[0][0] ) };
static LookupTable calibration_peaking_flt1_motion_adp_gain = { .ptr = _CALIBRATION_PEAKING_FLT1_MOTION_ADP_GAIN, .rows = sizeof( _CALIBRATION_PEAKING_FLT1_MOTION_ADP_GAIN ) / sizeof( _CALIBRATION_PEAKING_FLT1_MOTION_ADP_GAIN[0] ), .cols = sizeof( _CALIBRATION_PEAKING_FLT1_MOTION_ADP_GAIN[0] ) / sizeof( _CALIBRATION_PEAKING_FLT1_MOTION_ADP_GAIN[0][0] ), .width = sizeof( _CALIBRATION_PEAKING_FLT1_MOTION_ADP_GAIN[0][0] )};
static LookupTable calibration_peaking_flt2_motion_adp_gain = { .ptr = _CALIBRATION_PEAKING_FLT2_MOTION_ADP_GAIN, .rows = sizeof( _CALIBRATION_PEAKING_FLT2_MOTION_ADP_GAIN ) / sizeof( _CALIBRATION_PEAKING_FLT2_MOTION_ADP_GAIN[0] ), .cols = sizeof( _CALIBRATION_PEAKING_FLT2_MOTION_ADP_GAIN[0] ) / sizeof( _CALIBRATION_PEAKING_FLT2_MOTION_ADP_GAIN[0][0] ), .width = sizeof( _CALIBRATION_PEAKING_FLT2_MOTION_ADP_GAIN[0][0] )};
static LookupTable calibration_peaking_gain_vs_luma_lut = { .ptr = _CALIBRATION_PEAKING_GAIN_VS_LUMA_LUT, .rows = sizeof(_CALIBRATION_PEAKING_GAIN_VS_LUMA_LUT) / sizeof(_CALIBRATION_PEAKING_GAIN_VS_LUMA_LUT[0]), .cols = sizeof(_CALIBRATION_PEAKING_GAIN_VS_LUMA_LUT[0]) / sizeof(_CALIBRATION_PEAKING_GAIN_VS_LUMA_LUT[0][0]), .width = sizeof(_CALIBRATION_PEAKING_GAIN_VS_LUMA_LUT[0][0] ) };
static LookupTable calibration_peaking_cir_flt1_gain = { .ptr = _CALIBRATION_PEAKING_CIR_FLT1_GAIN, .rows = sizeof( _CALIBRATION_PEAKING_CIR_FLT1_GAIN ) / sizeof( _CALIBRATION_PEAKING_CIR_FLT1_GAIN[0] ), .cols = sizeof( _CALIBRATION_PEAKING_CIR_FLT1_GAIN[0] ) / sizeof( _CALIBRATION_PEAKING_CIR_FLT1_GAIN[0][0] ), .width = sizeof( _CALIBRATION_PEAKING_CIR_FLT1_GAIN[0][0] )};
static LookupTable calibration_peaking_cir_flt2_gain = { .ptr = _CALIBRATION_PEAKING_CIR_FLT2_GAIN, .rows = sizeof( _CALIBRATION_PEAKING_CIR_FLT2_GAIN ) / sizeof( _CALIBRATION_PEAKING_CIR_FLT2_GAIN[0] ), .cols = sizeof( _CALIBRATION_PEAKING_CIR_FLT2_GAIN[0] ) / sizeof( _CALIBRATION_PEAKING_CIR_FLT2_GAIN[0][0] ), .width = sizeof( _CALIBRATION_PEAKING_CIR_FLT2_GAIN[0][0] )};
static LookupTable calibration_peaking_drt_flt2_gain = { .ptr = _CALIBRATION_PEAKING_DRT_FLT2_GAIN, .rows = sizeof( _CALIBRATION_PEAKING_DRT_FLT2_GAIN ) / sizeof( _CALIBRATION_PEAKING_DRT_FLT2_GAIN[0] ), .cols = sizeof( _CALIBRATION_PEAKING_DRT_FLT2_GAIN[0] ) / sizeof( _CALIBRATION_PEAKING_DRT_FLT2_GAIN[0][0] ), .width = sizeof( _CALIBRATION_PEAKING_DRT_FLT2_GAIN[0][0] )};
static LookupTable calibration_peaking_drt_flt1_gain = { .ptr = _CALIBRATION_PEAKING_DRT_FLT1_GAIN, .rows = sizeof( _CALIBRATION_PEAKING_DRT_FLT1_GAIN ) / sizeof( _CALIBRATION_PEAKING_DRT_FLT1_GAIN[0] ), .cols = sizeof( _CALIBRATION_PEAKING_DRT_FLT1_GAIN[0] ) / sizeof( _CALIBRATION_PEAKING_DRT_FLT1_GAIN[0][0] ), .width = sizeof( _CALIBRATION_PEAKING_DRT_FLT1_GAIN[0][0] )};
static LookupTable calibration_cm_ctl = { .ptr = _CALIBRATION_CM_CTL, .rows = 1, .cols = sizeof(_CALIBRATION_CM_CTL) / sizeof(_CALIBRATION_CM_CTL[0]), .width = sizeof(_CALIBRATION_CM_CTL[0] ) };
static LookupTable calibration_cm_y_via_hue = { .ptr = _CALIBRATION_CM_Y_VIA_HUE, .rows = 1, .cols = sizeof(_CALIBRATION_CM_Y_VIA_HUE) / sizeof(_CALIBRATION_CM_Y_VIA_HUE[0]), .width = sizeof(_CALIBRATION_CM_Y_VIA_HUE[0] ) };
static LookupTable calibration_cm_satglbgain_via_y = { .ptr = _CALIBRATION_CM_SATGLBGAIN_VIA_Y, .rows = sizeof(_CALIBRATION_CM_SATGLBGAIN_VIA_Y) / sizeof(_CALIBRATION_CM_SATGLBGAIN_VIA_Y[0]), .cols = sizeof(_CALIBRATION_CM_SATGLBGAIN_VIA_Y[0]) / sizeof(_CALIBRATION_CM_SATGLBGAIN_VIA_Y[0][0]), .width = sizeof(_CALIBRATION_CM_SATGLBGAIN_VIA_Y[0][0] ) };
static LookupTable calibration_cm_sat_via_hs = { .ptr = _CALIBRATION_CM_SAT_VIA_HS, .rows = sizeof(_CALIBRATION_CM_SAT_VIA_HS) / sizeof(_CALIBRATION_CM_SAT_VIA_HS[0]), .cols = sizeof(_CALIBRATION_CM_SAT_VIA_HS[0]) / sizeof(_CALIBRATION_CM_SAT_VIA_HS[0][0]), .width = sizeof(_CALIBRATION_CM_SAT_VIA_HS[0][0] ) };
static LookupTable calibration_cm_satgain_via_y = { .ptr = _CALIBRATION_CM_SATGAIN_VIA_Y, .rows = sizeof(_CALIBRATION_CM_SATGAIN_VIA_Y) / sizeof(_CALIBRATION_CM_SATGAIN_VIA_Y[0]), .cols = sizeof(_CALIBRATION_CM_SATGAIN_VIA_Y[0]) / sizeof(_CALIBRATION_CM_SATGAIN_VIA_Y[0][0]), .width = sizeof(_CALIBRATION_CM_SATGAIN_VIA_Y[0][0] ) };
static LookupTable calibration_cm_hue_via_h = { .ptr = _CALIBRATION_CM_HUE_VIA_H, .rows = 1, .cols = sizeof(_CALIBRATION_CM_HUE_VIA_H) / sizeof(_CALIBRATION_CM_HUE_VIA_H[0]), .width = sizeof(_CALIBRATION_CM_HUE_VIA_H[0] ) };
static LookupTable calibration_cm_hue_via_s = { .ptr = _CALIBRATION_CM_HUE_VIA_S, .rows = sizeof(_CALIBRATION_CM_HUE_VIA_S) / sizeof(_CALIBRATION_CM_HUE_VIA_S[0]), .cols = sizeof(_CALIBRATION_CM_HUE_VIA_S[0]) / sizeof(_CALIBRATION_CM_HUE_VIA_S[0][0]), .width = sizeof(_CALIBRATION_CM_HUE_VIA_S[0][0] ) };
static LookupTable calibration_cm_hue_via_y = { .ptr = _CALIBRATION_CM_HUE_VIA_Y, .rows = sizeof(_CALIBRATION_CM_HUE_VIA_Y) / sizeof(_CALIBRATION_CM_HUE_VIA_Y[0]), .cols = sizeof(_CALIBRATION_CM_HUE_VIA_Y[0]) / sizeof(_CALIBRATION_CM_HUE_VIA_Y[0][0]), .width = sizeof(_CALIBRATION_CM_HUE_VIA_Y[0][0] ) };
static LookupTable calibration_hlc_ctl = { .ptr = _CALIBRATION_HLC_CTL, .rows = 1, .cols = sizeof(_CALIBRATION_HLC_CTL) / sizeof(_CALIBRATION_HLC_CTL[0]), .width = sizeof(_CALIBRATION_HLC_CTL[0] ) };

static LookupTable calibration_black_level = { .ptr = _CALIBRATION_BLACK_LEVEL, .rows = sizeof( _CALIBRATION_BLACK_LEVEL ) / sizeof( _CALIBRATION_BLACK_LEVEL[0] ), .cols = sizeof( _CALIBRATION_BLACK_LEVEL[0] ) / sizeof( _CALIBRATION_BLACK_LEVEL[0][0] ), .width = sizeof( _CALIBRATION_BLACK_LEVEL[0][0] )};
static LookupTable calibration_noise_profile = { .ptr = _CALIBRATION_NOISE_PROFILE, .rows = sizeof(_CALIBRATION_NOISE_PROFILE) / sizeof(_CALIBRATION_NOISE_PROFILE[0]), .cols = sizeof(_CALIBRATION_NOISE_PROFILE[0]) / sizeof(_CALIBRATION_NOISE_PROFILE[0][0]), .width = sizeof(_CALIBRATION_NOISE_PROFILE[0][0] ) };
static LookupTable calibration_awb_mesh_dist_tab = { .ptr = _CALIBRATION_AWB_MESH_DIST_TAB, .rows = sizeof(_CALIBRATION_AWB_MESH_DIST_TAB) / sizeof(_CALIBRATION_AWB_MESH_DIST_TAB[0]), .cols = sizeof(_CALIBRATION_AWB_MESH_DIST_TAB[0]) / sizeof(_CALIBRATION_AWB_MESH_DIST_TAB[0][0]), .width = sizeof(_CALIBRATION_AWB_MESH_DIST_TAB[0][0] ) };
static LookupTable calibration_awb_mesh_ct_tab = { .ptr = _CALIBRATION_AWB_MESH_CT_TAB, .rows = sizeof( _CALIBRATION_AWB_MESH_CT_TAB ) / sizeof( _CALIBRATION_AWB_MESH_CT_TAB[0] ), .cols = sizeof( _CALIBRATION_AWB_MESH_CT_TAB[0] ) / sizeof( _CALIBRATION_AWB_MESH_CT_TAB[0][0] ), .width = sizeof( _CALIBRATION_AWB_MESH_CT_TAB[0][0] )};
static LookupTable calibration_awb_rg_pos = { .ptr = _CALIBRATION_AWB_RG_POS, .rows = 1, .cols = sizeof( _CALIBRATION_AWB_RG_POS ) / sizeof( _CALIBRATION_AWB_RG_POS[0] ), .width = sizeof( _CALIBRATION_AWB_RG_POS[0] )};
static LookupTable calibration_awb_bg_pos = { .ptr = _CALIBRATION_AWB_BG_POS, .rows = 1, .cols = sizeof(_CALIBRATION_AWB_BG_POS) / sizeof(_CALIBRATION_AWB_BG_POS[0]), .width = sizeof(_CALIBRATION_AWB_BG_POS[0] ) };
static LookupTable calibration_awb_ct_rg_curve = { .ptr = _CALIBRATION_AWB_CT_RG_CURVE, .rows = 1, .cols = sizeof( _CALIBRATION_AWB_CT_RG_CURVE ) / sizeof( _CALIBRATION_AWB_CT_RG_CURVE[0] ), .width = sizeof( _CALIBRATION_AWB_CT_RG_CURVE[0] )};
static LookupTable calibration_awb_ct_bg_curve = { .ptr = _CALIBRATION_AWB_CT_BG_CURVE, .rows = 1, .cols = sizeof( _CALIBRATION_AWB_CT_BG_CURVE ) / sizeof( _CALIBRATION_AWB_CT_BG_CURVE[0] ), .width = sizeof( _CALIBRATION_AWB_CT_BG_CURVE[0] )};
static LookupTable calibration_awb_wb_golden_d50 = { .ptr = _CALIBRATION_AWB_WB_GOLDEN_D50, .rows = 1, .cols = sizeof( _CALIBRATION_AWB_WB_GOLDEN_D50 ) / sizeof( _CALIBRATION_AWB_WB_GOLDEN_D50[0] ), .width = sizeof( _CALIBRATION_AWB_WB_GOLDEN_D50[0] )};
static LookupTable calibration_awb_wb_otp_d50 = { .ptr = _CALIBRATION_AWB_WB_OTP_D50, .rows = 1, .cols = sizeof( _CALIBRATION_AWB_WB_OTP_D50 ) / sizeof( _CALIBRATION_AWB_WB_OTP_D50[0] ), .width = sizeof( _CALIBRATION_AWB_WB_OTP_D50[0] )};
static LookupTable calibration_ccm = { .ptr = _CALIBRATION_CCM, .rows = 1, .cols = sizeof( _CALIBRATION_CCM ) / sizeof( _CALIBRATION_CCM[0] ), .width = sizeof( _CALIBRATION_CCM[0] )};
static LookupTable calibration_gamma = { .ptr = _CALIBRATION_GAMMA, .rows = 1, .cols = sizeof( _CALIBRATION_GAMMA ) / sizeof( _CALIBRATION_GAMMA[0] ), .width = sizeof( _CALIBRATION_GAMMA[0] )};
static LookupTable calibration_cac_rx = { .ptr = _CALIBRATION_CAC_RX, .rows = 1, .cols = sizeof( _CALIBRATION_CAC_RX ) / sizeof( _CALIBRATION_CAC_RX[0] ), .width = sizeof( _CALIBRATION_CAC_RX[0] )};
static LookupTable calibration_cac_ry = { .ptr = _CALIBRATION_CAC_RY, .rows = 1, .cols = sizeof( _CALIBRATION_CAC_RY ) / sizeof( _CALIBRATION_CAC_RY[0] ), .width = sizeof( _CALIBRATION_CAC_RY[0] )};
static LookupTable calibration_cac_bx = { .ptr = _CALIBRATION_CAC_BX, .rows = 1, .cols = sizeof( _CALIBRATION_CAC_BX ) / sizeof( _CALIBRATION_CAC_BX[0] ), .width = sizeof( _CALIBRATION_CAC_BX[0] )};
static LookupTable calibration_cac_by = { .ptr = _CALIBRATION_CAC_BY, .rows = 1, .cols = sizeof( _CALIBRATION_CAC_BY ) / sizeof( _CALIBRATION_CAC_BY[0] ), .width = sizeof( _CALIBRATION_CAC_BY[0] )};
static LookupTable calibration_shading_radial_r = { .ptr = _CALIBRATION_SHADING_RADIAL_R, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_RADIAL_R ) / sizeof( _CALIBRATION_SHADING_RADIAL_R[0] ), .width = sizeof( _CALIBRATION_SHADING_RADIAL_R[0] )};
static LookupTable calibration_shading_radial_g = { .ptr = _CALIBRATION_SHADING_RADIAL_G, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_RADIAL_G ) / sizeof( _CALIBRATION_SHADING_RADIAL_G[0] ), .width = sizeof( _CALIBRATION_SHADING_RADIAL_G[0] )};
static LookupTable calibration_shading_radial_b = { .ptr = _CALIBRATION_SHADING_RADIAL_B, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_RADIAL_B ) / sizeof( _CALIBRATION_SHADING_RADIAL_B[0] ), .width = sizeof( _CALIBRATION_SHADING_RADIAL_B[0] )};
static LookupTable calibration_shading_ls_d65_r = { .ptr = _CALIBRATION_SHADING_LS_D65_R, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_D65_R ) / sizeof( _CALIBRATION_SHADING_LS_D65_R[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_D65_R[0] )};
static LookupTable calibration_shading_ls_d65_g = { .ptr = _CALIBRATION_SHADING_LS_D65_G, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_D65_G ) / sizeof( _CALIBRATION_SHADING_LS_D65_G[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_D65_G[0] )};
static LookupTable calibration_shading_ls_d65_b = { .ptr = _CALIBRATION_SHADING_LS_D65_B, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_D65_B ) / sizeof( _CALIBRATION_SHADING_LS_D65_B[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_D65_B[0] )};
static LookupTable calibration_shading_ls_cwf_r = { .ptr = _CALIBRATION_SHADING_LS_CWF_R, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_CWF_R ) / sizeof( _CALIBRATION_SHADING_LS_CWF_R[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_CWF_R[0] )};
static LookupTable calibration_shading_ls_cwf_g = { .ptr = _CALIBRATION_SHADING_LS_CWF_G, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_CWF_G ) / sizeof( _CALIBRATION_SHADING_LS_CWF_G[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_CWF_G[0] )};
static LookupTable calibration_shading_ls_cwf_b = { .ptr = _CALIBRATION_SHADING_LS_CWF_B, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_CWF_B ) / sizeof( _CALIBRATION_SHADING_LS_CWF_B[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_CWF_B[0] )};
static LookupTable calibration_shading_ls_tl84_r = { .ptr = _CALIBRATION_SHADING_LS_TL84_R, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_TL84_R ) / sizeof( _CALIBRATION_SHADING_LS_TL84_R[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_TL84_R[0] )};
static LookupTable calibration_shading_ls_tl84_g = { .ptr = _CALIBRATION_SHADING_LS_TL84_G, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_TL84_G ) / sizeof( _CALIBRATION_SHADING_LS_TL84_G[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_TL84_G[0] )};
static LookupTable calibration_shading_ls_tl84_b = { .ptr = _CALIBRATION_SHADING_LS_TL84_B, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_TL84_B ) / sizeof( _CALIBRATION_SHADING_LS_TL84_B[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_TL84_B[0] )};
static LookupTable calibration_shading_ls_a_r = { .ptr = _CALIBRATION_SHADING_LS_A_R, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_A_R ) / sizeof( _CALIBRATION_SHADING_LS_A_R[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_A_R[0] )};
static LookupTable calibration_shading_ls_a_g = { .ptr = _CALIBRATION_SHADING_LS_A_G, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_A_G ) / sizeof( _CALIBRATION_SHADING_LS_A_G[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_A_G[0] )};
static LookupTable calibration_shading_ls_a_b = { .ptr = _CALIBRATION_SHADING_LS_A_B, .rows = 1, .cols = sizeof( _CALIBRATION_SHADING_LS_A_B ) / sizeof( _CALIBRATION_SHADING_LS_A_B[0] ), .width = sizeof( _CALIBRATION_SHADING_LS_A_B[0] )};
static LookupTable calibration_lens_shading_ctl = { .ptr = _CALIBRATION_LENS_SHADING_CTL, .rows = 1, .cols = sizeof(_CALIBRATION_LENS_SHADING_CTL) / sizeof(_CALIBRATION_LENS_SHADING_CTL[0]), .width = sizeof(_CALIBRATION_LENS_SHADING_CTL[0] ) };
static LookupTable calibration_fpnr = { .ptr = _CALIBRATION_FPNR, .rows = 1, .cols = sizeof(_CALIBRATION_FPNR) / sizeof(_CALIBRATION_FPNR[0]), .width = sizeof(_CALIBRATION_FPNR[0] ) };
static LookupTable calibration_awb_preset = { .ptr = _CALIBRATION_AWB_PRESET, .rows = 1, .cols = sizeof(_CALIBRATION_AWB_PRESET) / sizeof(_CALIBRATION_AWB_PRESET[0]), .width = sizeof(_CALIBRATION_AWB_PRESET[0] ) };

int dynamic_wdr_calibrations_init_ov13855(aisp_calib_info_t *calib)
{
	calib->calibrations[CALIBRATION_TOP_CTL] = &calibration_top_ctl;
	calib->calibrations[CALIBRATION_RES_CTL] = &calibration_res_ctl;
	calib->calibrations[CALIBRATION_AWB_CTL] = &calibration_awb_ctl;
	calib->calibrations[CALIBRATION_AWB_CT_POS] = &calibration_awb_ct_pos;
	calib->calibrations[CALIBRATION_AWB_CT_RG_COMPENSATION] = &calibration_awb_ct_rg_compensation;
	calib->calibrations[CALIBRATION_AWB_CT_BG_COMPENSATION] = &calibration_awb_ct_bg_compensation;
	calib->calibrations[CALIBRATION_AWB_CT_WGT] = &calibration_awb_ct_wgt;
	calib->calibrations[CALIBRATION_AWB_CT_DYN_CVRANGE] = &calibration_awb_ct_dyn_cvrange;
	calib->calibrations[CALIBRATION_AE_CTL] = &calibration_ae_ctl;
	calib->calibrations[CALIBRATION_AE_CORR_POS_LUT] = &calibration_ae_corr_pos_lut;
	calib->calibrations[CALIBRATION_AE_CORR_LUT] = &calibration_ae_corr_lut;
	calib->calibrations[CALIBRATION_AE_ROUTE] = &calibration_ae_route;
	calib->calibrations[CALIBRATION_AE_WEIGHT_H] = &calibration_ae_weight_h;
	calib->calibrations[CALIBRATION_AE_WEIGHT_V] = &calibration_ae_weight_v;
	calib->calibrations[CALIBRATION_DAYNIGHT_DETECT] = &calibration_daynight_detect;
	calib->calibrations[CALIBRATION_AF_CTL] = &calibration_af_ctl;
	calib->calibrations[CALIBRATION_AF_WEIGHT_H] = &calibration_af_weight_h;
	calib->calibrations[CALIBRATION_AF_WEIGHT_V] = &calibration_af_weight_v;
	calib->calibrations[CALIBRATION_FLICKER_CTL] = &calibration_flicker_ctl;
	calib->calibrations[CALIBRATION_GTM] = &calibration_gtm;
	calib->calibrations[CALIBRATION_GE_ADJ] = &calibration_ge_adj;
	calib->calibrations[CALIBRATION_GE_S_ADJ] = &calibration_ge_s_adj;
	calib->calibrations[CALIBRATION_DPC_CTL] = &calibration_dpc_ctl;
	calib->calibrations[CALIBRATION_DPC_S_CTL] = &calibration_dpc_s_ctl;
	calib->calibrations[CALIBRATION_DPC_ADJ] = &calibration_dpc_adj;
	calib->calibrations[CALIBRATION_DPC_S_ADJ] = &calibration_dpc_s_adj;
	calib->calibrations[CALIBRATION_WDR_CTL] = &calibration_wdr_ctl;
	calib->calibrations[CALIBRATION_WDR_ADJUST] = &calibration_wdr_adjust;
	calib->calibrations[CALIBRATION_WDR_MDETC_LOWEIGHT] = &calibration_wdr_mdetc_loweight;
	calib->calibrations[CALIBRATION_WDR_MDETC_HIWEIGHT] = &calibration_wdr_mdetc_hiweight;
	calib->calibrations[CALIBRATION_RAWCNR_CTL] = &calibration_rawcnr_ctl;
	calib->calibrations[CALIBRATION_RAWCNR_ADJ] = &calibration_rawcnr_adj;
	calib->calibrations[CALIBRATION_RAWCNR_META_GAIN_LUT] = &calibration_rawcnr_meta_gain_lut;
	calib->calibrations[CALIBRATION_RAWCNR_SPS_CSIG_WEIGHT5X5] = &calibration_rawcnr_sps_csig_weight5x5;
	calib->calibrations[CALIBRATION_SNR_CTL] = &calibration_snr_ctl;
	calib->calibrations[CALIBRATION_SNR_GLB_ADJ] = &calibration_snr_glb_adj;
	calib->calibrations[CALIBRATION_SNR_ADJ] = &calibration_snr_adj;
	calib->calibrations[CALIBRATION_SNR_CUR_WT] = &calibration_snr_cur_wt;
	calib->calibrations[CALIBRATION_SNR_WT_LUMA_GAIN] = &calibration_snr_wt_luma_gain;
	calib->calibrations[CALIBRATION_SNR_SAD_META2ALP] = &calibration_snr_sad_meta2alp;
	calib->calibrations[CALIBRATION_SNR_META_ADJ] = &calibration_snr_meta_adj;
	calib->calibrations[CALIBRATION_SNR_PHS] = &calibration_snr_phs;
	calib->calibrations[CALIBRATION_NR_RAD_LUT65] = &calibration_nr_rad_lut65;
	calib->calibrations[CALIBRATION_PST_SNR_ADJ] = &calibration_pst_snr_adj;
	calib->calibrations[CALIBRATION_TNR_CTL] = &calibration_tnr_ctl;
	calib->calibrations[CALIBRATION_TNR_GLB_ADJ] = &calibration_tnr_glb_adj;
	calib->calibrations[CALIBRATION_TNR_ADJ] = &calibration_tnr_adj;
	calib->calibrations[CALIBRATION_TNR_SAD2ALPHA] = &calibration_tnr_sad2alpha;
	calib->calibrations[CALIBRATION_MC_META2ALPHA] = &calibration_mc_meta2alpha;
	calib->calibrations[CALIBRATION_PST_TNR_ALP_LUT] = &calibration_pst_tnr_alp_lut;
	calib->calibrations[CALIBRATION_LENS_SHADING_CT_CORRECT] = &calibration_lens_shading_ct_correct;
	calib->calibrations[CALIBRATION_LENS_SHADING_ADJ] = &calibration_lens_shading_adj;
	calib->calibrations[CALIBRATION_DMS_ADJ] = &calibration_dms_adj;
	calib->calibrations[CALIBRATION_CCM_ADJ] = &calibration_ccm_adj;
	calib->calibrations[CALIBRATION_CNR_CTL] = &calibration_cnr_ctl;
	calib->calibrations[CALIBRATION_CNR_ADJ] = &calibration_cnr_adj;
	calib->calibrations[CALIBRATION_PURPLE_CTL] = &calibration_purple_ctl;
	calib->calibrations[CALIBRATION_PURPLE_ADJ] = &calibration_purple_adj;
	calib->calibrations[CALIBRATION_LTM_CTL] = &calibration_ltm_ctl;
	calib->calibrations[CALIBRATION_LTM_LO_HI_GM] = &calibration_ltm_lo_hi_gm;
	calib->calibrations[CALIBRATION_LTM_CONTRAST] = &calibration_ltm_contrast;
	calib->calibrations[CALIBRATION_LTM_SHARP_ADJ] = &calibration_ltm_sharp_adj;
	calib->calibrations[CALIBRATION_LTM_SATUR_LUT] = &calibration_ltm_satur_lut;
	calib->calibrations[CALIBRATION_LC_CTL] = &calibration_lc_ctl;
	calib->calibrations[CALIBRATION_LC_SATUR_LUT] = &calibration_lc_satur_lut;
	calib->calibrations[CALIBRATION_LC_STRENGTH] = &calibration_lc_strength;
	calib->calibrations[CALIBRATION_DNLP_CTL] = &calibration_dnlp_ctl;
	calib->calibrations[CALIBRATION_DNLP_STRENGTH] = &calibration_dnlp_strength;
	calib->calibrations[CALIBRATION_DNLP_SCURV_LOW] = &calibration_dnlp_scurv_low;
	calib->calibrations[CALIBRATION_DNLP_SCURV_MID1] = &calibration_dnlp_scurv_mid1;
	calib->calibrations[CALIBRATION_DNLP_SCURV_MID2] = &calibration_dnlp_scurv_mid2;
	calib->calibrations[CALIBRATION_DNLP_SCURV_HGH1] = &calibration_dnlp_scurv_hgh1;
	calib->calibrations[CALIBRATION_DNLP_SCURV_HGH2] = &calibration_dnlp_scurv_hgh2;
	calib->calibrations[CALIBRATION_DHZ_CTL] = &calibration_dhz_ctl;
	calib->calibrations[CALIBRATION_DHZ_STRENGTH] = &calibration_dhz_strength;
	calib->calibrations[CALIBRATION_PEAKING_CTL] = &calibration_peaking_ctl;
	calib->calibrations[CALIBRATION_PEAKING_ADJUST] = &calibration_peaking_adjust;
	calib->calibrations[CALIBRATION_PEAKING_FLT1_MOTION_ADP_GAIN] = &calibration_peaking_flt1_motion_adp_gain;
	calib->calibrations[CALIBRATION_PEAKING_FLT2_MOTION_ADP_GAIN] = &calibration_peaking_flt2_motion_adp_gain;
	calib->calibrations[CALIBRATION_PEAKING_GAIN_VS_LUMA_LUT] = &calibration_peaking_gain_vs_luma_lut;
	calib->calibrations[CALIBRATION_PEAKING_CIR_FLT1_GAIN] = &calibration_peaking_cir_flt1_gain;
	calib->calibrations[CALIBRATION_PEAKING_CIR_FLT2_GAIN] = &calibration_peaking_cir_flt2_gain;
	calib->calibrations[CALIBRATION_PEAKING_DRT_FLT2_GAIN] = &calibration_peaking_drt_flt2_gain;
	calib->calibrations[CALIBRATION_PEAKING_DRT_FLT1_GAIN] = &calibration_peaking_drt_flt1_gain;
	calib->calibrations[CALIBRATION_CM_CTL] = &calibration_cm_ctl;
	calib->calibrations[CALIBRATION_CM_Y_VIA_HUE] = &calibration_cm_y_via_hue;
	calib->calibrations[CALIBRATION_CM_SATGLBGAIN_VIA_Y] = &calibration_cm_satglbgain_via_y;
	calib->calibrations[CALIBRATION_CM_SAT_VIA_HS] = &calibration_cm_sat_via_hs;
	calib->calibrations[CALIBRATION_CM_SATGAIN_VIA_Y] = &calibration_cm_satgain_via_y;
	calib->calibrations[CALIBRATION_CM_HUE_VIA_H] = &calibration_cm_hue_via_h;
	calib->calibrations[CALIBRATION_CM_HUE_VIA_S] = &calibration_cm_hue_via_s;
	calib->calibrations[CALIBRATION_CM_HUE_VIA_Y] = &calibration_cm_hue_via_y;
	calib->calibrations[CALIBRATION_HLC_CTL] = &calibration_hlc_ctl;

	calib->calibrations[CALIBRATION_BLACK_LEVEL] = &calibration_black_level;
	calib->calibrations[CALIBRATION_CAC_RX] = &calibration_cac_rx;
	calib->calibrations[CALIBRATION_CAC_RY] = &calibration_cac_ry;
	calib->calibrations[CALIBRATION_CAC_BX] = &calibration_cac_bx;
	calib->calibrations[CALIBRATION_CAC_BY] = &calibration_cac_by;
	calib->calibrations[CALIBRATION_SHADING_RADIAL_R] = &calibration_shading_radial_r;
	calib->calibrations[CALIBRATION_SHADING_RADIAL_G] = &calibration_shading_radial_g;
	calib->calibrations[CALIBRATION_SHADING_RADIAL_B] = &calibration_shading_radial_b;
	calib->calibrations[CALIBRATION_SHADING_LS_D65_R] = &calibration_shading_ls_d65_r;
	calib->calibrations[CALIBRATION_SHADING_LS_D65_G] = &calibration_shading_ls_d65_g;
	calib->calibrations[CALIBRATION_SHADING_LS_D65_B] = &calibration_shading_ls_d65_b;
	calib->calibrations[CALIBRATION_SHADING_LS_CWF_R] = &calibration_shading_ls_cwf_r;
	calib->calibrations[CALIBRATION_SHADING_LS_CWF_G] = &calibration_shading_ls_cwf_g;
	calib->calibrations[CALIBRATION_SHADING_LS_CWF_B] = &calibration_shading_ls_cwf_b;
	calib->calibrations[CALIBRATION_SHADING_LS_TL84_R] = &calibration_shading_ls_tl84_r;
	calib->calibrations[CALIBRATION_SHADING_LS_TL84_G] = &calibration_shading_ls_tl84_g;
	calib->calibrations[CALIBRATION_SHADING_LS_TL84_B] = &calibration_shading_ls_tl84_b;
	calib->calibrations[CALIBRATION_SHADING_LS_A_R] = &calibration_shading_ls_a_r;
	calib->calibrations[CALIBRATION_SHADING_LS_A_G] = &calibration_shading_ls_a_g;
	calib->calibrations[CALIBRATION_SHADING_LS_A_B] = &calibration_shading_ls_a_b;
	calib->calibrations[CALIBRATION_LENS_SHADING_CTL] = &calibration_lens_shading_ctl;
	calib->calibrations[CALIBRATION_GAMMA] = &calibration_gamma;
	calib->calibrations[CALIBRATION_CCM] = &calibration_ccm;
	calib->calibrations[CALIBRATION_AWB_RG_POS] = &calibration_awb_rg_pos;
	calib->calibrations[CALIBRATION_AWB_BG_POS] = &calibration_awb_bg_pos;
	calib->calibrations[CALIBRATION_AWB_MESH_DIST_TAB] = &calibration_awb_mesh_dist_tab;
	calib->calibrations[CALIBRATION_AWB_MESH_CT_TAB] = &calibration_awb_mesh_ct_tab;
	calib->calibrations[CALIBRATION_AWB_CT_RG_CURVE] = &calibration_awb_ct_rg_curve;
	calib->calibrations[CALIBRATION_AWB_CT_BG_CURVE] = &calibration_awb_ct_bg_curve;
	calib->calibrations[CALIBRATION_AWB_WB_GOLDEN_D50] = &calibration_awb_wb_golden_d50;
	calib->calibrations[CALIBRATION_AWB_WB_OTP_D50] = &calibration_awb_wb_otp_d50;
	calib->calibrations[CALIBRATION_NOISE_PROFILE] = &calibration_noise_profile;
	calib->calibrations[CALIBRATION_FPNR] = &calibration_fpnr;
	calib->calibrations[CALIBRATION_AWB_PRESET] = &calibration_awb_preset;

    return 0;
}
}


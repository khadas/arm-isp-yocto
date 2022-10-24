/*
 * Copyright (c) 2021 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef __AML_ISP_TUNING_H__
#define __AML_ISP_TUNING_H__

#include <stdint.h>

#define ISO_NUM_MAX     (10)

typedef enum cali_type {
    CALIBRATION_TOP_CTL                   ,
    CALIBRATION_RES_CTL                   ,
    CALIBRATION_AWB_CTL                   ,
    CALIBRATION_AWB_CT_POS                ,
    CALIBRATION_AWB_CT_RG_COMPENSAT       ,
    CALIBRATION_AWB_CT_BG_COMPENSAT       ,
    CALIBRATION_AWB_CT_WGT                ,
    CALIBRATION_AWB_CT_DYN_CVRANGE        ,
    CALIBRATION_AWB_GRAY_ZONE             ,
    CALIBRATION_AE_CTL                    ,
    CALIBRATION_AE_CORR_LUT               ,
    CALIBRATION_AE_CORR_POS_LUT           ,
    CALIBRATION_AE_ROUTE                  ,
    CALIBRATION_AE_WEIGHT_H               ,
    CALIBRATION_AE_WEIGHT_V               ,
    CALIBRATION_AF_CTL                    ,
    CALIBRATION_AF_WEIGHT_H               ,
    CALIBRATION_AF_WEIGHT_V               ,
    CALIBRATION_FLICKER_CTL               ,
    CALIBRATION_GTM                       ,
    CALIBRATION_GE_ADJ                    ,
    CALIBRATION_GE_S_ADJ                  ,
    CALIBRATION_DPC_ADJ                   ,
    CALIBRATION_DPC_S_ADJ                 ,
    CALIBRATION_WDR_CTL                   ,
    CALIBRATION_WDR_ADJUST                ,
    CALIBRATION_WDR_MDETC_LOWEIGHT        ,
    CALIBRATION_WDR_MDETC_HIWEIGHT        ,
    CALIBRATION_RAWCNR_CTL                ,
    CALIBRATION_RAWCNR_ADJ                ,
    CALIBRATION_RAWCNR_META_GAIN_LUT      ,
    CALIBRATION_RAWCNR_SPS_CSIG_WEIGHT5X5 ,
    CALIBRATION_SNR_CTL                   ,
    CALIBRATION_SNR_GLB_ADJ               ,
    CALIBRATION_SNR_ADJ                   ,
    CALIBRATION_SNR_CUR_WT                ,
    CALIBRATION_SNR_WT_LUMA_GAIN          ,
    CALIBRATION_SNR_SAD_META2ALP          ,
    CALIBRATION_SNR_META_ADJ              ,
    CALIBRATION_SNR_PHS                   ,
    CALIBRATION_NR_RAD_LUT65              ,
    CALIBRATION_PST_SNR_ADJ               ,
    CALIBRATION_TNR_CTL                   ,
    CALIBRATION_TNR_GLB_ADJ               ,
    CALIBRATION_TNR_ADJ                   ,
    CALIBRATION_TNR_SAD2ALPHA             ,
    CALIBRATION_MC_META2ALPHA             ,
    CALIBRATION_PST_TNR_ALP_LUT           ,
    CALIBRATION_LENS_SHADING_ADJ          ,
    CALIBRATION_LENS_SHADING_CT_CORRECT   ,
    CALIBRATION_DMS_ADJ                   ,
    CALIBRATION_CCM_ADJ                   ,
    CALIBRATION_CNR_CTL                   ,
    CALIBRATION_CNR_ADJ                   ,
    CALIBRATION_LTM_CTL                   ,
    CALIBRATION_LTM_LO_HI_GM              ,
    CALIBRATION_LTM_SHARP_ADJ             ,
    CALIBRATION_LTM_SATUR_LUT             ,
    CALIBRATION_LC_CTL                    ,
    CALIBRATION_LC_SATUR_LUT              ,
    CALIBRATION_DNLP_CTL                  ,
    CALIBRATION_DHZ_CTL                   ,
    CALIBRATION_PEAKING_CTL               ,
    CALIBRATION_PEAKING_ADJUST            ,
    CALIBRATION_PEAKING_FLT1_MOTION_ADP_GAIN,
    CALIBRATION_PEAKING_FLT2_MOTION_ADP_GAIN,
    CALIBRATION_PEAKING_GAIN_VS_LUMA_LUT  ,
    CALIBRATION_PEAKING_CIR_FLT1_GAIN     ,
    CALIBRATION_PEAKING_CIR_FLT2_GAIN     ,
    CALIBRATION_PEAKING_DRT_FLT1_GAIN     ,
    CALIBRATION_PEAKING_DRT_FLT2_GAIN     ,
    CALIBRATION_CM_CTL                    ,
    CALIBRATION_CM_Y_VIA_HUE              ,
    CALIBRATION_CM_SATGLBGAIN_VIA_Y       ,
    CALIBRATION_CM_SAT_VIA_HS             ,
    CALIBRATION_CM_SATGAIN_VIA_Y          ,
    CALIBRATION_CM_HUE_VIA_H              ,
    CALIBRATION_CM_HUE_VIA_S              ,
    CALIBRATION_CM_HUE_VIA_Y              ,
    CALIBRATION_HLC_CTL                   ,
    CALIBRATION_BLACK_LEVEL               ,
    CALIBRATION_SHADING_RADIAL_R          ,
    CALIBRATION_SHADING_RADIAL_G          ,
    CALIBRATION_SHADING_RADIAL_B          ,
    CALIBRATION_SHADING_LS_D65_R          ,
    CALIBRATION_SHADING_LS_D65_B          ,
    CALIBRATION_SHADING_LS_D65_G          ,
    CALIBRATION_SHADING_LS_CWF_R          ,
    CALIBRATION_SHADING_LS_CWF_B          ,
    CALIBRATION_SHADING_LS_CWF_G          ,
    CALIBRATION_SHADING_LS_TL84_R         ,
    CALIBRATION_SHADING_LS_TL84_G         ,
    CALIBRATION_SHADING_LS_TL84_B         ,
    CALIBRATION_SHADING_LS_A_R            ,
    CALIBRATION_SHADING_LS_A_G            ,
    CALIBRATION_SHADING_LS_A_B            ,
    CALIBRATION_LENS_SHADING_CTL          ,
    CALIBRATION_GAMMA                     ,
    CALIBRATION_CCM                       ,
    CALIBRATION_CAC_RX                    ,
    CALIBRATION_CAC_RY                    ,
    CALIBRATION_CAC_BX                    ,
    CALIBRATION_CAC_BY                    ,
    CALIBRATION_AWB_RG_POS                ,
    CALIBRATION_AWB_BG_POS                ,
    CALIBRATION_AWB_MESH_DIST_TAB         ,
    CALIBRATION_AWB_MESH_CT_TAB           ,
    CALIBRATION_AWB_CT_RG_CURVE           ,
    CALIBRATION_AWB_CT_BG_CURVE           ,
    CALIBRATION_AWB_WB_GOLDEN_D50         ,
    CALIBRATION_AWB_WB_OTP_D50            ,
    CALIBRATION_NOISE_PROFILE             ,
    CALIBRATION_FPNR                      ,
    CALIBRATION_SQRT0                     ,
    CALIBRATION_SQRT1                     ,
    CALIBRATION_EOTF0                     ,
    CALIBRATION_EOTF1                     ,
    CALIBRATION_LTM_HIST_BLK65            ,
    CALIBRATION_DNLP_SCURV_LOW            ,
    CALIBRATION_DNLP_SCURV_MID1           ,
    CALIBRATION_DNLP_SCURV_MID2           ,
    CALIBRATION_DNLP_SCURV_HGH1           ,
    CALIBRATION_DNLP_SCURV_HGH2           ,
    CALIBRATION_DNLP_ADP_THRD             ,
    CALIBRATION_DNLP_BLK_BOOST            ,
    CALIBRATION_DNLP_GAIN_VAR_LUT49       ,
    CALIBRATION_DNLP_WEXT_GAIN            ,
    CALIBRATION_DNLP_ADP_OFSET            ,
    CALIBRATION_DNLP_MONO_PROTECT         ,
    CALIBRATION_DNLP_TREND_WHT_EXPAND_LUT8,
    CALIBRATION_DHZ_SKY_PROT_LUT          ,
    CALIBRATION_PPS_SCALE_H               ,
    CALIBRATION_PPS_SCALE_V               ,
    CALIBRATION_DECMP0                    ,
    CALIBRATION_DECMP1                    ,
    CALIBRATION_AWB_PRESET                ,
    CALIBRATION_TOTAL_SIZE                ,
} cali_type_t;

typedef enum
{
    AML_MBI_ISP_BaseAttr= 0x0,
    AML_MBI_ISP_CSCAttr,
    AML_MBI_ISP_ModuleCtrlAttr,
    AML_MBI_ISP_HLCAttr,
    AML_MBI_ISP_PstGammaAttr,
    AML_MBI_ISP_RgbGammaAttr,
    AML_MBI_ISP_ShadingLutAttr,
    AML_MBI_ISP_RadialShadingLut,
    AML_MBI_ISP_FPNRLut,
    AML_MBI_ISP_DrcAttr,
    AISP_BASE_MAX,

    AML_MBI_ISP_ExposureAttr,
    AML_MBI_ISP_WDRExposureAttr,
    AML_MBI_ISP_WBAttr,
    AML_MBI_ISP_AePstGammaUpdate,
    AML_MBI_ISP_CaControl,
    AML_MBI_ISP_AERouterAttr,
    AML_MBI_ISP_CalcFlickerAttr,
    AML_MBI_ISP_CalcFlickerOutput,
    AML_MBI_ISP_WBExAttr,
    AML_MBI_ISP_QueryWBinfo,
    AML_MBI_ISP_QueryEXPinfo,
    AISP_3A_MAX,

    AML_MBI_ISP_SaturationAttr,
    AML_MBI_ISP_ShadingAttr,
    AML_MBI_ISP_RadialShadingAttr,
    AML_MBI_ISP_BlackLevelAttr,
    AML_MBI_ISP_ColorMatrixAttr,
    AISP_CORRECTION_MAX,

    AML_MBI_ISP_IspSharpenAttr,
    AML_MBI_ISP_FSWDRAttr,
    AML_MBI_ISP_LtmAlgAttr,
    AML_MBI_ISP_LtmGammaAttr,
    AML_MBI_ISP_DhzAttr,
    AML_MBI_ISP_LdcAttr,
    AML_MBI_ISP_LdcLutAttr,
    AML_MBI_ISP_DNLPAttr,
    AML_MBI_ISP_CM2Attr,
    AISP_ENHANCE_MAX,

    AML_MBI_ISP_RAWCNRAttr,
    AML_MBI_ISP_SNRAttr,
    AML_MBI_ISP_TNRAttr,
    AML_MBI_ISP_CNRAttr,
    AML_MBI_ISP_DMSAttr,
    AML_MBI_ISP_NPAttr,
    AML_MBI_ISP_CrAttr,
    AML_MBI_ISP_FPNRAttr,
    AML_MBI_ISP_DPCAttr,
    AISP_RESTORATION_MAX,

    AML_MBI_ISP_FMWAttr,
    AML_MBI_ISP_RotAttr,
    AML_MBI_ISP_FpsAttr,

} aisp_cmd_id_t;

typedef struct LookupTable {
    void *ptr;
    uint16_t rows;
    uint16_t cols;
    uint16_t width;
} LookupTable;

typedef struct _AIspCalibrations {
    LookupTable *calibrations[CALIBRATION_TOTAL_SIZE];
} AIspCalibrations;

typedef struct aisp_calib_info_s {
    LookupTable *calibrations[CALIBRATION_TOTAL_SIZE];
} aisp_calib_info_t;

uint32_t calibrations_static_init(AIspCalibrations *c);
uint32_t calibrations_dynamic_init(AIspCalibrations *c);

#endif //__AML_ISP_TUNING_H__

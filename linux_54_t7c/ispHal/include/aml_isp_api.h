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

typedef enum
{
    AML_CMD_GET = 0x0,
    AML_CMD_SET,
} aisp_cmd_type_t;

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

typedef struct _LENS_PARAM_T {
    uint16_t lens_type; //lens type which assigns one of the enum type after probing
    uint16_t min_step;  //lens step resolution
    uint16_t next_zoom; //next assigned zoom if zoom if available
    uint16_t curr_zoom; //current zoom positon if zoom if available
    uint16_t next_pos;  //lens position

    uint32_t minfocus_distance;     // lens minimum focus distance (diopters x 10000)
    uint32_t hyperfocal_distance;   // lens hyperfocal focus distance (diopters x 10000)
    uint32_t focal_length;          // focal length of the lens (mm x 10000)
    uint32_t aperture;              // lens aperture (f-number x 10000)
} LENS_PARAM_T;

typedef struct {
    void( *pfn_lens_move )( uint32_t ctx, uint16_t position );
    void( *pfn_lens_stop )( uint32_t ctx );
    uint8_t( *pfn_lens_is_moving )( uint32_t ctx );
    uint16_t( *pfn_lens_get_pos )( uint32_t ctx );
    int32_t( *pfn_lens_write_register )( void *ctx, uint8_t address, uint16_t data );
    void( *pfn_lens_read_register )( void *ctx, uint8_t address, uint16_t *data );
    const LENS_PARAM_T *( *pfn_lens_get_parameters )( uint32_t ctx );
    void( *pfn_lens_move_zoom )( uint32_t ctx, uint16_t next_zoom );
    uint8_t( *pfn_lens_is_zooming )( uint32_t ctx );
} ALG_LENS_FUNC_S;

typedef struct
{
    ALG_SENSOR_DEFAULT_S   stSnsDft;
    ALG_SENSOR_EXP_FUNC_S  stSnsExp;
    ALG_LENS_FUNC_S        stLensFunc;
} AML_ALG_CTX_S;

void aisp_enable(uint32_t ctx_id, void *pstAlgCtx, void *calib);
void aisp_disable(uint32_t ctx_id);
void aisp_alg2user(uint32_t ctx_id, void *param);
void aisp_alg2kernel(uint32_t ctx_id, void *param);
void aisp_fw_interface(uint32_t ctx_id, void *param);

#define LOG2_GAIN_SHIFT 12
#define SHUTTER_TIME_SHIFT 12

/** @brief read raw data form memory and normalize raw data
 *
 * @param pVin      Raw data bit stream
 * @param pData     Output normalize raw data value
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param endian    raw data endian format
 * @param src_bit_depth raw data source bit depth
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_read_raw_data(unsigned char *pVin, int *pData, int xsize, int ysize, int endian, int src_bit_depth);

/** @brief optical center point calibration
 *
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param phase_ofst    phase offset of raw color components
 * @param pData     normalize raw data
 * @param pLSC      calibration data of the radial shading.
 * @param pVout     raw data after radial shading correction.
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_optical_center_point_calibration(int xsize, int ysize, int *phase_ofst, int *pData, int *center_x, int *center_y);

/** @brief radial shading calibrate and output radial shading correcte table.
 *
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param phase_ofst    phase offset of raw color components
 * @param pData     normalize raw data
 * @param chroma_s  radial shading chroma correction strength percentage
 * @param luma_s    radial shading luma correction strength percentage
 * @param mesh_s    normalize value of the radial shading calibration value
 * @return error code
 *   @retval 0  - success
 */

int aml_radial_shading_calibration(int xsize, int ysize, int *phase_ofst, int center_ofs_x, int center_ofs_y, int *pData, int chroma_s, int luma_s, int *pVout);

/** @brief radial shading correct process
 *
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param phase_ofst    phase offset of raw color components
 * @param pData     normalize raw data
 * @param pLSC      calibration data of the radial shading.
 * @param pVout     raw data after radial shading correction.
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_radial_shading_correct(int xsize, int ysize, int *phase_ofst, int center_ofs_x, int center_ofs_y, int *pData, int *pRadLSC, int *pVout);

/** @brief mesh shading calibrate and output mesh shading correcte table.
 *
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param X_node    horizontal nodes of the mesh shading table
 * @param Y_node    vertical nodes of the mesh shading table
 * @param phase_ofst    phase offset of raw color components
 * @param pData     normalize raw data
 * @param chroma_s  mesh shading chroma correction strength percentage
 * @param luma_s    mesh shading luma correction strength percentage
 * @param mesh_s    normalize value of the mesh shading calibration value
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 *   @retval -2 - mesh scale parameter error
 */
int aml_mesh_shading_calibration(int xsize, int ysize, int X_node, int Y_node, int *phase_ofst, int *pData, int chroma_s, int luma_s, int mesh_s, int *pVout);

/** @brief mesh shading correct process
 *
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param X_node    horizontal nodes of the mesh shading table
 * @param Y_node    vertical nodes of the mesh shading table
 * @param phase_ofst    phase offset of raw color components
 * @param pData     normalize raw data
 * @param pLSC      calibration data of the mesh shading.
 * @param meshscale normalize value of the mesh shading calibration value
 * @param pVout     raw data after mesh shading correction.
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_mesh_shading_correct(int xsize, int ysize, int X_node, int Y_node, int *phase_ofst, int *pData, int *pLSC, int meshscale, int *pVout);


/** @brief mesh shading calibration data compression processing
 *
 * @param X_node    horizontal nodes of the mesh shading table
 * @param Y_node    vertical nodes of the mesh shading table
 * @param pLSC      calibration data of the mesh shading.
 * @param pLSC_enc  calibration data after compression.
 * @param size      max size of the compression buffer
 * @param lose_level compression lose level, 0:lossless
 * @return          valid compression byte numbers
 */
int aml_mesh_shading_compress(int X_node, int Y_node, int *pLSC, unsigned char *pLSC_enc, int size, int lose_level);

/** @brief mesh shading calibration data decompression processing
 *
 * @param X_node    horizontal nodes of the mesh shading table
 * @param Y_node    vertical nodes of the mesh shading table
 * @param pLSC      calibration data of the mesh shading.
 * @param pLSC_enc  calibration data after compression.
 * @param size      max size of the compression buffer
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_mesh_shading_decompress(int X_node, int Y_node, int *pLSC, unsigned char *pLSC_enc, int size);

/** @brief radial shading calibration data compression processing
 *
 * @param node     nodes of the mesh shading table
 * @param pLSC      calibration data of the mesh shading.
 * @param pLSC_enc  calibration data after compression.
 * @param size      max size of the compression buffer
 * @param lose_level compression lose level, 0:lossless
 * @return          valid compression byte numbers
 */
int aml_rad_shading_compress(int node, int *pLSC, unsigned char *pLSC_enc, int size, int lose_level);

/** @brief radial shading calibration data decompression processing
 *
 * @param node      nodes of the mesh shading table
 * @param pLSC      calibration data of the mesh shading.
 * @param pLSC_enc  calibration data after compression.
 * @param size      max size of the compression buffer
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_rad_shading_decompress(int node, int *pLSC, unsigned char *pLSC_enc, int size);

/** @brief white balance OTP calibration processing
 *
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param pData     normalize raw data
 * @param pVout     white balance OTP data.
 * @param phase_ofst    phase offset of raw color components
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_white_balance_otp_calibration(int xsize, int ysize, int *pData, int *pVout, int *phase_ofst);

/** @brief black level correct process
 *
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param phase_ofst    phase offset of raw color components
 * @param blc       black level value
 * @param pVout     raw data after mesh shading correction.
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_black_level_correct(int xsize, int ysize, int *phase_ofst, int *blc, int *pVout);

/** @brief  white balance correct process
 *
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param in_bw     white balance OTP data.
 * @param phase_ofst    phase offset of raw color components
 * @param pVout     raw data after mesh shading correction.
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_white_balance_correct(int xsize, int ysize, int *in_bw, int *phase_ofst, int *pVout);

/** @brief Convert Raw data format to RGB data format
 *
 * @param xsize     width of the raw data resolution
 * @param ysize     height of the raw data resolution
 * @param phase_ofst    phase offset of raw color components
 * @param pData     normalize raw data
 * @param pVout     RGB data
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_demosaic(int xsize, int ysize, int *phase_ofst, int *pData, int *pVout);
/** @brief write RGB data into file, use BMP file format
 *
 * @param filename  BMP file name
 * @param cpBufr    RGB data stream
 * @param xsize     width of the RGB data resolution
 * @param ysize     height of the RGB data resolution
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
int aml_write_bmp(char *filename, int *cpBufr, int xsize, int ysize);
/** @brief whether print debug info.
 *
 * @param debug     0:disable 1: enable
 * @return error code
 *   @retval 0  - success
 *   @retval -1 - failure
 */
void aml_debug_mode_set(int debug);

#endif

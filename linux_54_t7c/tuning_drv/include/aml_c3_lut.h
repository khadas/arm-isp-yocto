#include <asm/types.h>
#include "aml_c3_connection.h"

#define LUT_GTM_ID                  0
#define LUT_GTM_SHD_ID              1
#define LUT_LNS_RAD_ID              2
#define LUT_LNS_MESH_ID             3
#define LUT_DFE_PRE_SQRT0           4
#define LUT_DFE_PRE_SQRT1           5
#define LUT_DFE_NR_CUBIC            6
#define LUT_PSTY_ID                 7
#define LUT_PSTY_SHD_ID             8
#define LUT_GAMMA0_ID               9
#define LUT_GAMMA0_SHD_ID           10
#define LUT_GAMMA1_ID               11
#define LUT_GAMMA1_SHD_ID           12
#define LUT_GAMMA2_ID               13
#define LUT_GAMMA2_SHD_ID           14
#define LUT_GAMMA3_ID               15
#define LUT_GAMMA3_SHD_ID           16
#define LUT_AF_STAT_IDX_ID          17
#define LUT_AE_STAT_IDX_ID          18
#define LUT_AE_STAT_BLK_WET_ID      19
#define LUT_AWB_STAT_IDX_ID         20
#define LUT_AWB_STAT_BLK_WET_ID     21
#define LUT_CAC_TABLE_ID            22
#define LUT_DPC_1024_ID             23

//hardware lookup table abstraction layer
typedef struct _ral_hw_lut_t
{
	u32 gtm_valid;
	u32 gtm_shd_valid;
	u32 lns_rad_valid;
	u32 lns_mesh_valid;
	u32 dfe_pre_sqrt0_valid;
	u32 dfe_pre_sqrt1_valid;
	u32 dfe_nr_cubit_valid;
	u32 psty_valid;
	u32 psty_shd_valid;
	u32 gamma0_valid;
	u32 gamma0_shd_valid;
	u32 gamma1_valid;
	u32 gamma1_shd_valid;
	u32 gamma2_valid;
	u32 gamma2_shd_valid;
	u32 gamma3_valid;
	u32 gamma3_shd_valid;
	u32 ae_stat_inx_valid;
	u32 ae_stat_blk_wet_valid;
	u32 awb_stat_inx_valid;
	u32 awb_stat_blk_wet_valid;
	u32 cac_table_valid;
	u32 dpc_1024_valid;

	u32 gtm_checksum;
	u32 gtm_shd_checksum;
	u32 lns_rad_checksum;
	u32 lns_mesh_checksum;
	u32 dfe_pre_sqrt0_checksum;
	u32 dfe_pre_sqrt1_checksum;
	u32 dfe_nr_cubit_checksum;
	u32 psty_checksum;
	u32 psty_shd_checksum;
	u32 gamma0_checksum;
	u32 gamma0_shd_checksum;
	u32 gamma1_checksum;
	u32 gamma1_shd_checksum;
	u32 gamma2_checksum;
	u32 gamma2_shd_checksum;
	u32 gamma3_checksum;
	u32 gamma3_shd_checksum;
	u32 ae_stat_inx_checksum;
	u32 ae_stat_blk_wet_checksum;
	u32 awb_stat_inx_checksum;
	u32 awb_stat_blk_wet_checksum;
	u32 cac_table_checksum;
	u32 dpc_1024_checksum;
} drv_lut_t;

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

void aisp_read_lut(u32 addr, u32 *buf, u32 size);
void aisp_write_lut(u32 addr, u32 *buf, u32 size);



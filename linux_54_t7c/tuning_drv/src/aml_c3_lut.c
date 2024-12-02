/**
 *  @file aml_p1_lut.c
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
 *  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 *  Amlogic, INC. OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVawbISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  @details implementation ISP driver Lookup-Table
 *  @history  2021.09.28 create
 **/

#include <linux/device.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/vmalloc.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>

#include "aml_c3_lut.h"
#include "aml_c3_connection.h"

struct lut_list {
	u32 addr_addr;
	u32 data_addr;
	u32 lut_len;
	u32 reg_len;
	void (*read_lut)(u32 addr, u32 data_addr, u32 *data, u32 len);
	void (*write_lut)(u32 addr, u32 data_addr, u32 *data, u32 len);
};

static void read_rad_lut65(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, sum = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 16; i++) {
		hw_data = system_hw_read_32(data_addr);
		data[4*i + 0] = hw_data & 0xff;
		data[4*i + 1] = (hw_data >> 8) & 0xff;
		data[4*i + 2] = (hw_data >> 16) & 0xff;
		data[4*i + 3] = (hw_data >> 24) & 0xff;
		sum += hw_data;
	}

	hw_data = system_hw_read_32(data_addr);
	data[64] = hw_data & 0xff;
	sum += hw_data;
}

static void write_rad_lut65(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, sum = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 16; i++) {
		hw_data = (data[4 * i] & 0xff) |
			((data[4 * i + 1] & 0xff)  << 8)|
			((data[4 * i + 2] & 0xff)  << 16) |
			((data[4 * i + 3] & 0xff)  << 24);
		system_hw_write_32(data_addr, hw_data);
		sum += hw_data;
	}

	hw_data = data[64] & 0xff;
	system_hw_write_32(data_addr, hw_data);
	sum += hw_data;
}

static void read_gtm_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, sum = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++) {
		hw_data = system_hw_read_32(data_addr);
		data[2*i+0] = hw_data & 0xfff;
		data[2*i+1] = (hw_data >> 12) & 0xfff;
		sum += hw_data;
	}

	hw_data = system_hw_read_32(data_addr);
	data[128] = hw_data & 0xfff;
	sum += hw_data;
}


static void write_gtm_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, sum = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++) {
		hw_data = (data[2 * i + 0] & 0xfff) | ((data[2 * i + 1] & 0xfff) << 12);
		system_hw_write_32(data_addr, hw_data);
		sum += hw_data;
	}

	hw_data = data[128] & 0xfff;
	system_hw_write_32(data_addr, hw_data);
	sum += hw_data;
}

static void read_gtm_shd_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, sum = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++) {
		hw_data = system_hw_read_32(data_addr);
		data[2*i+0] = hw_data & 0xfff;
		data[2*i+1] = (hw_data >> 12) & 0xfff;
		sum += hw_data;
	}

	hw_data = system_hw_read_32(data_addr);
	data[128] = hw_data & 0xfff;
	sum += hw_data;
}


static void write_gtm_shd_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, sum = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++) {
		hw_data = (data[2 * i + 0] & 0xfff) | ((data[2 * i + 1] & 0xfff)  << 12);
		system_hw_write_32(data_addr, hw_data);
		sum += hw_data;
	}

	hw_data = data[128] & 0xfff;
	system_hw_write_32(data_addr, hw_data);
	sum += hw_data;
}

static void read_lns_rad_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, sum = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		sum += hw_data;
		data[i * 2] = hw_data & 0xffff;
		data[i * 2 + 1] = (hw_data >> 16) & 0xffff;
	}
	hw_data = system_hw_read_32(data_addr);
	sum += hw_data;
	data[128] = hw_data & 0xffff;

	for (i = 0; i < 64; i++) {
		hw_data = system_hw_read_32(data_addr);
		sum += hw_data;
		data[i * 2 + 129] = hw_data & 0xffff;
		data[i * 2 + 1 + 129] = (hw_data >> 16) & 0xffff;
	}
	hw_data = system_hw_read_32(data_addr);
	sum += hw_data;
	data[128 + 129] = hw_data & 0xffff;

	for (i = 0; i < 64; i++) {
		hw_data = system_hw_read_32(data_addr);
		sum += hw_data;
		data[i * 2 + 2 * 129] = hw_data & 0xffff;
		data[i * 2 + 1 + 2 * 129] = (hw_data >> 16) & 0xffff;
	}
	hw_data = system_hw_read_32(data_addr);
	sum += hw_data;
	data[128 + 2 * 129] = hw_data & 0xffff;

	for (i = 0; i < 64; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		sum += hw_data;
		data[i * 2 + 3 * 129] = hw_data & 0xffff;
		data[i * 2 + 1 + 3 * 129] = (hw_data >> 16) &  0xffff;
	}
	hw_data = system_hw_read_32(data_addr);
	sum += hw_data;
	data[128 + 3 * 129] = hw_data & 0xffff;
}

static void write_lns_rad_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, sum = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++) {
		hw_data = (data[i * 2] & 0xffff) |
				((data[i * 2 + 1] & 0xffff) << 16);
		system_hw_write_32(data_addr, hw_data);
		sum += hw_data;
	}
	hw_data = data[128]  & 0xffff;
	system_hw_write_32(data_addr, hw_data);
	sum += hw_data;

	for (i = 0; i < 64; i++) {
		hw_data = (data[i * 2 + 129] & 0xffff) |
				((data[i * 2 + 1 + 129] & 0xffff) << 16);
		system_hw_write_32(data_addr, hw_data);
		sum += hw_data;
	}
	hw_data = data[128 + 129] & 0xffff;
	system_hw_write_32(data_addr, hw_data);
	sum += hw_data;

	for (i = 0; i < 64; i++)
	{
		hw_data = (data[i * 2 + 2 * 129] & 0xffff) |
				((data[i * 2 + 1 + 2 * 129] & 0xffff) << 16);
		system_hw_write_32(data_addr, hw_data);
		sum += hw_data;
	}
	hw_data = data[128 + 2 * 129] & 0xffff;
	system_hw_write_32(data_addr, hw_data);
	sum += hw_data;

	for (i = 0; i < 64; i++)
	{
		hw_data =  (data[i * 2 + 3 * 129] & 0xffff) |
				((data[i * 2 + 1 + 3 * 129] & 0xffff) << 16);
		system_hw_write_32(data_addr, hw_data);
		sum += hw_data;
	}
	hw_data = data[128 + 3 * 129] & 0xffff;
	system_hw_write_32(data_addr, hw_data);
	sum += hw_data;
}

static void read_lns_mesh_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 4096; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xff;
		data[i + 4096] = (hw_data >> 8) & 0xff;
		data[i + 4096 * 2] = (hw_data >> 16) & 0xff;
		data[i + 4096 * 3] = (hw_data >> 24) & 0xff;
	}
}

static void write_lns_mesh_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 4096; i++) {
		hw_data = (data[i] & 0xff) |
			((data[i + 4096] & 0xff) << 8) |
			((data[i + 4096 * 2] & 0xff) << 16) |
			((data[i + 4096 * 3] & 0xff) << 24);
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_dfe_pre_sqrt0_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xfffff;
	}
}

static void write_dfe_pre_sqrt0_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 33; i++) {
		hw_data = data[i] & 0xfffff;
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_dfe_pre_sqrt1_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 129; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xfffff;
	}
}

static void write_dfe_pre_sqrt1_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 129; i++) {
		hw_data = data[i] & 0xfffff;
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_dfe_nr_cubic_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data;
	}
}

static void write_dfe_nr_cubic_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 33; i++) {
		hw_data = data[i];
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_gamma0_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[2 * i] = hw_data & 0xfff;
		data[2 * i + 1] = ( hw_data >> 12) & 0xfff;
	}
	hw_data = system_hw_read_32(data_addr);
	data[128] = hw_data & 0xfff;
}

static void write_gamma0_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++) {
		hw_data = (data[2 * i] & 0xfff) |
				((data[2 * i + 1] & 0xfff) << 12);
		system_hw_write_32(data_addr, hw_data);
	}

	hw_data = (data[128] & 0xfff);
	system_hw_write_32(data_addr, hw_data);
}

static void read_af_stat_idx_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		if (addr & 0xf0000)
			data[i] = hw_data >> 16;
		else
			data[i] = hw_data & 0xffff;
	}
}

static void write_af_stat_idx_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data[33];

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 33; i++) {
		hw_data[i] = system_hw_read_32(data_addr);
	}

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 33; i++) {
		if (addr & 0xf0000)
			hw_data[i] = (hw_data[i] & 0xffff) |((data[i] & 0xffff) << 16);
		else
			hw_data[i] = (data[i] & 0xffff) | ((hw_data[i] & 0xffff) << 16);
		system_hw_write_32(data_addr, hw_data[i]);
	}
}

static void read_ae_stat_idx_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 65; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		if (addr & 0xf0000)
			data[i] = hw_data >> 16;
		else
			data[i] = hw_data & 0xffff;
	}
}

static void write_ae_stat_idx_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data[65];

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 65; i++)
	{
		hw_data[i] = system_hw_read_32(data_addr);
	}

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 65; i++) {
		if (addr & 0xf0000)
			hw_data[i] = (hw_data[i] & 0xffff) | ((data[i] & 0xffff) << 16);
		else
			hw_data[i] = (data[i] & 0xffff) | ((hw_data[i] & 0xffff) << 16);
		system_hw_write_32(data_addr, hw_data[i]);
	}
}

static void read_ae_stat_blk_wet_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 31; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[8 * i] = hw_data & 0xf;
		data[8 * i + 1] = (hw_data >> 4) & 0xf;
		data[8 * i + 2] = (hw_data >> 8) & 0xf;
		data[8 * i + 3] = (hw_data >> 12) & 0xf;
		data[8 * i + 4] = (hw_data >> 16) & 0xf;
		data[8 * i + 5] = (hw_data >> 20) & 0xf;
		data[8 * i + 6] = (hw_data >> 24) & 0xf;
		data[8 * i + 7] = (hw_data >> 28) & 0xf;
	}

	hw_data = system_hw_read_32(data_addr);
	data[8 * i] = hw_data & 0xf;
	data[8 * i + 1] = (hw_data >> 4) & 0xf;
	data[8 * i + 2] = (hw_data >> 8) & 0xf;
	data[8 * i + 3] = (hw_data >> 12) & 0xf;
	data[8 * i + 4] = (hw_data >> 16) & 0xf;
	data[8 * i + 5] = (hw_data >> 20) & 0xf;
	data[8 * i + 6] = (hw_data >> 24) & 0xf;
}

static void write_ae_stat_blk_wet_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 31; i++) {
		hw_data = (data[8 * i] & 0xf) |
			((data[8 * i + 1] & 0xf) << 4) |
			((data[8 * i + 2] & 0xf) << 8) |
			((data[8 * i + 3] & 0xf) << 12) |
			((data[8 * i + 4] & 0xf) << 16) |
			((data[8 * i + 5] & 0xf) << 20) |
			((data[8 * i + 6] & 0xf) << 24) |
			((data[8 * i + 7] & 0xf) << 28);
		system_hw_write_32(data_addr, hw_data);
	}
	hw_data = (data[8 * i] & 0xf) |
	((data[8 * i + 1] & 0xf) << 4) |
	((data[8 * i + 2] & 0xf) << 8) |
	((data[8 * i + 3] & 0xf) << 12) |
	((data[8 * i + 4] & 0xf) << 16) |
	((data[8 * i + 5] & 0xf) << 20) |
	((data[8 * i + 6] & 0xf) << 24);
	system_hw_write_32(data_addr, hw_data);
}

static void read_awb_stat_idx_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		if (addr & 0xf0000)
			data[i] = hw_data >> 16;
		else
			data[i] = hw_data & 0xffff;
	}
}

static void write_awb_stat_idx_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data[33];

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 33; i++) {
		hw_data[i] = system_hw_read_32(data_addr);
	}

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 33; i++) {
		if (addr & 0xf0000)
			hw_data[i] = (hw_data[i] & 0xffff) |((data[i] & 0xffff) << 16);
		else
			hw_data[i] = (data[i] & 0xffff) | ((hw_data[i] & 0xffff) << 16);
		system_hw_write_32(data_addr, hw_data[i]);
	}
}

static void read_awb_stat_blk_wet_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 96; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[8 * i] = hw_data & 0xf;
		data[8 * i + 1] = (hw_data >> 4) & 0xf;
		data[8 * i + 2] = (hw_data >> 8) & 0xf;
		data[8 * i + 3] = (hw_data >> 12) & 0xf;
		data[8 * i + 4] = (hw_data >> 16) & 0xf;
		data[8 * i + 5] = (hw_data >> 20) & 0xf;
		data[8 * i + 6] = (hw_data >> 24) & 0xf;
		data[8 * i + 7] = (hw_data >> 28) & 0xf;
	}
}

static void write_awb_stat_blk_wet_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 96; i++) {
		hw_data = (data[8 * i] & 0xf) |
			((data[8 * i + 1] & 0xf) << 4) |
			((data[8 * i + 2] & 0xf) << 8) |
			((data[8 * i + 3] & 0xf) << 12) |
			((data[8 * i + 4] & 0xf) << 16) |
			((data[8 * i + 5] & 0xf) << 20) |
			((data[8 * i + 6] & 0xf) << 24) |
			((data[8 * i + 7] & 0xf) << 28);
		system_hw_write_32(data_addr, hw_data);
	}
}

static void write_cac_table_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr + 4, 1);
	system_hw_write_32(addr, 0);

	for (i = 0; i < 512; i++) {
		hw_data  = (data[2 * i] & 0xff) |
			(((data[1024 + 2 * i]) << 8) & 0xFF00) |
			(((data[2 * i + 1]) << 16) & 0xFF0000) |
			(((data[1024 + 2 * i + 1]) << 24) & 0xFF000000);
		system_hw_write_32(data_addr, hw_data);
	}

	for (i = 0; i < 512; i++) {
		hw_data  = (data[2048 + 2 * i] & 0xff) |
			(((data[3072 + 2 * i]) << 8) & 0xFF00) |
			(((data[2048 + 2 * i + 1]) << 16) & 0xFF0000) |
			(((data[3072 + 2 * i + 1]) << 24) & 0xFF000000);
		system_hw_write_32(data_addr, hw_data);
	}
}

#define CAC_SELECT(chn, vidx, hidx)    (((chn)<<16)|((vidx)<<8)|((hidx)<<0))
static void read_cac_table_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data;

	system_hw_write_32(addr + 4, 1);
	system_hw_write_32(addr, 0);
	for (i = 0; i < 32; i++) {
		for (j = 0; j < 32; j += 2) {
			system_hw_write_32(addr, CAC_SELECT(0, i, j));

			hw_data = system_hw_read_32(data_addr);
			data[32 * i + j] = hw_data & 0xff;
			data[1024 + 32 * i + j] = (hw_data >> 8) & 0xff;
			data[32 * i + j + 1] = (hw_data >> 16) & 0xff;
			data[1024 + 32 * i + j + 1] = (hw_data >> 24) & 0xff;
		}
	}

	for (i = 0; i < 32; i++) {
		for (j = 0; j < 32; j += 2) {
			system_hw_write_32(addr, CAC_SELECT(1, i, j));

			hw_data = system_hw_read_32(data_addr);
			data[2048 + 32 * i + j] = hw_data & 0xff;
			data[3072 + 32 * i + j] = (hw_data >> 8) & 0xff;
			data[2048 + 32 * i + j + 1] = (hw_data >> 16) & 0xff;
			data[3072 + 32 * i + j + 1] = (hw_data >> 24) & 0xff;
		}
	}
}

static void read_decmp0_eotf0_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xfffff;
	}
}

static void write_decmp0_eotf0_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 33; i++) {
		hw_data = data[i] & 0xfffff;
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_decmp1_eotf1_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 129; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xfffff;
	}
}

static void write_decmp1_eotf1_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 129; i++) {
		hw_data = data[i] & 0xfffff;
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_ltm_histxptsbuf_blk(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 79; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xfffff;
	}
}

static void write_ltm_histxptsbuf_blk(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 79; i++) {
		hw_data = data[i] & 0xfffff;
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_ltm_lm_blk(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 96; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		if (addr & 0xf0000)
			data[i] = hw_data & 0xfffff;
		else
			data[i] = hw_data & 0xfffff;
	}
}

static void write_ltm_lm_blk(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data[96];

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 96; i++) {
		hw_data[i] = system_hw_read_32(data_addr);
	}

	system_hw_write_32(addr & 0xffff, 0);
	for (i = 0; i < 96; i++) {
		if (addr & 0xf0000) {
			hw_data[i] = data[i] & 0xfffff;
			system_hw_write_32(data_addr, hw_data[i]);
		}else{
			hw_data[i] = (data[i] & 0xfffff);
			system_hw_write_32(data_addr, hw_data[i]);
		}
	}
}

static void read_ltm_ccrat_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 32; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[2*i] = hw_data & 0xfff;
		data[2*i + 1] = (hw_data >>12) & 0xfff;
	}
	hw_data = system_hw_read_32(data_addr);
	data[62] = hw_data&0xfff;
}

static void write_ltm_ccrat_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 32; i++)
	{
		hw_data = (data[2 * i] & 0xfff) |
			((data[2 * i + 1] & 0xfff) << 12);
		system_hw_write_32(data_addr, hw_data);
	}
	hw_data = data[62]&0xfff;
	system_hw_write_32(data_addr, hw_data);
}

static void read_chroma_coef(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 32; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		for (j = 0; j < 8; j++)
		{
			data[i*8+j] = (hw_data>>(4*j)) & 0xf;
		}
	}
}

static void write_chroma_coef(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 32; i++) {
		for (j = 0; j < 8; j++)
		{
			hw_data |= (data[i * 8 + j]<<(4*j));
		}
	system_hw_write_32(data_addr, hw_data);
	}
}

static void read_ro_ltm(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 96; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xfffff;
	}
}

static void read_ro_ltm_sta_hst_blk_sum(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 192; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		if ( addr & 0xf0000 ) {
			if (i % 2 == 1)
				data[i / 2] = hw_data & 0x1ffff;  //ro_ltm_sta_hst_blk_sum_h
		} else {
			if (i % 2 == 0)
				data[i / 2] = hw_data;    //ro_ltm_sta_hst_blk_sum_l
		}
	}
}

static void read_ro_ltm_histbuf(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 513; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xffffffff;
	}
}

static void read_ro_wdr_stat_yblk_flt(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 1024; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0x3fff;
	}
}

static void read_cnr2_satur_blk(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 128; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		for (j = 0; j < 8; j++)
		{
			data[i*8+j] = (hw_data>>(j*4))&0xf;
		}
	}
}

static void write_cnr2_satur_blk(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 128; i++) {
		for (j = 0; j < 8; j++)
		{
			hw_data |= (data[i*8+j]&0xf)<<(j*4);
		}
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_pk_clr_prct_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 32; i++)
	{
		hw_data = system_hw_read_32(data_addr + i * 4);
		for (j = 0; j < 8; j++)
		{
			data[i * 8 + j] = (hw_data >> (4 * j)) & 0xf;
		}
	}
}

static void write_pk_clr_prct_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 32; i++) {
		for (j = 0; j < 8; j++)
		{
			hw_data |= ((data[i] & 0xf) << (4 * j));
		}
		system_hw_write_32(data_addr + i * 4, hw_data);
	}
}

static void read_lut_meta_sad_2alpha(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 16; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[4 * i] = hw_data & 0xff;
		data[4 * i + 1] = (hw_data >> 8) & 0xff;
		data[4 * i + 2] = (hw_data >> 16) & 0xff;
		data[4 * i + 3] = (hw_data >> 24) & 0xff;
	}
}

static void write_lut_meta_sad_2alpha(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data = 0;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 16; i++) {
		hw_data = (data[4 * i] & 0xff) |
			((data[4 * i + 1] & 0xff) << 8) |
			((data[4 * i + 2] & 0xff) << 16) |
			((data[4 * i + 3] & 0xff) << 24);
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_dnlp_ygrid(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 32; i++)
	{
		hw_data = system_hw_read_32(data_addr + i * 4);
		data[2 * i] = hw_data & 0x3ff;
		data[2 * i + 1] = (hw_data >> 16) & 0x3ff;
	}
}

static void write_dnlp_ygrid(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 32; i++) {
		hw_data = (data[2 * i] & 0x3ff) |
			((data[2 * i + 1] & 0x3ff) << 16);
		system_hw_write_32(data_addr + i * 4, hw_data);
	}
}

static void read_lc_satur_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 31; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[2 * i] = hw_data & 0xfff;
		data[2 * i + 1] = (hw_data >> 16) & 0xfff;
	}
	hw_data = system_hw_read_32(data_addr);
	data[62] = hw_data & 0xfff;
}

static void write_lc_satur_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data[62];

	system_hw_write_32(addr, 0);
	for (i = 0; i < 31; i++) {
		hw_data[i] = (data[2 * i] & 0xfff) |
			((data[2 * i + 1] & 0xfff) << 16);
		system_hw_write_32(data_addr, hw_data[i]);
	}
	hw_data[62] = (data[62] & 0xfff);
	system_hw_write_32(data_addr, hw_data[62]);
}

static void read_dhz_sky_prot_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[2 * i] = hw_data & 0x3ff;
		data[2 * i + 1] = (hw_data >> 16) & 0x3ff;
	}
}

static void write_dhz_sky_prot_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 33; i++) {
		hw_data = (data[2 * i] & 0x3ff) |
			((data[2 * i + 1] & 0x3ff) << 16);
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_lut3d_ram(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0|(1<<31));
	for (i = 0; i < 9*9*9; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i*3 + 2] = (hw_data & 0xffff);
		data[i*3 + 1] = ((hw_data>>16) & 0xffff);

		hw_data = system_hw_read_32(data_addr);
		data[i*3] = (hw_data & 0xffff);
	}
}

static void write_lut3d_ram(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 9*9*9; i++) {
		hw_data = (data[i*3 + 1] << 16 & 0xffff)|
			(data[i*3+2]);
		system_hw_write_32(data_addr, hw_data);

		hw_data = data[i*3+0];
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_pst_gamma_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, cnt;

	for (cnt = 0; cnt < 4; cnt++) {
		system_hw_write_32(addr, cnt<<7);
		for (i = 0; i < 64; i++)
		{
			hw_data = system_hw_read_32(data_addr);
			data[2 * i] = hw_data & 0x3ff;
			data[2 * i + 1] = (hw_data >> 16) & 0x3ff;
		}
		hw_data = system_hw_read_32(data_addr);
		data[128] = hw_data & 0x3ff;
	}
}

static void write_pst_gamma_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data[129] , cnt;

	for (cnt = 0; cnt < 4; cnt++) {
		system_hw_write_32(addr, cnt << 7);
		for (i = 0; i < 64; i++) {
			hw_data[i] = (data[2 * i] & 0x3ff) |
				((data[2 * i+1] & 0x3ff) << 16);
			system_hw_write_32(data_addr, hw_data[i]);
		}
		hw_data[128] = data[128]&0x3ff;
		system_hw_write_32(data_addr, hw_data[128]);
	}
}

static void read_rgb_gamma_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++)
	{
		hw_data= system_hw_read_32(data_addr);
		data[2 * i] = hw_data & 0xfff;
		data[2 * i + 1] = (hw_data >> 12) & 0xfff;
	}
	hw_data = system_hw_read_32(data_addr);
	data[128] = hw_data & 0xfff;
}

static void write_rgb_gamma_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data[128];

	system_hw_write_32(addr, 0);
	for (i = 0; i < 64; i++)
	{
		hw_data[i] = (data[2 * i] & 0xfff)|
			((data[2*i + 1] & 0xfff) << 12);
		system_hw_write_32(data_addr, hw_data[i]);
	}
	hw_data[128] = data[128] & 0xfff;
	system_hw_write_32(data_addr, hw_data[128]);
}

static void read_snr_lpf_phs_sel(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 4*2; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		for (j = 0; j < 12; j++)
		{
			data[i*12 + j] = (hw_data & 0x3);
			hw_data = hw_data >> 2;
		}
	}
}

static void write_snr_lpf_phs_sel(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	int i, j, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 4*2; i++)
	{
		hw_data = 0;
		for (j = 0; j < 12; j++)
		{
			hw_data |= (data[i * 12 + j] & 3) << (j * 2);
		}
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_lc_lcsta_blk_hidx(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr,0);
	for (i = 0; i < 6; i++)
	{
		hw_data = system_hw_read_32(data_addr + i * 4);
		data[2 * i] = hw_data & 0xffff;
		data[2 * i + 1] = (hw_data >> 16) & 0xffff;
	}
	hw_data = system_hw_read_32(data_addr + 24);
	data[12] = hw_data & 0xffff;
}

static void write_lc_lcsta_blk_hidx(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data[12];

	system_hw_write_32(addr, 0);
	for (i = 0; i < 6; i++)
	{
		hw_data[2 * i] = (data[2 * i] & 0xffff)|
			((data[2*i + 1] & 0xffff) << 16);
		system_hw_write_32(data_addr + i * 4, hw_data[i]);
	}
	hw_data[12] = data[12] & 0xffff;
	system_hw_write_32(data_addr + 24, hw_data[12]);
}

static void read_lc_lcsta_blk_vidx(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr,0);
	for (i = 0; i < 4; i++)
	{
		hw_data = system_hw_read_32(data_addr + i * 4);
		data[2 * i] = hw_data & 0xffff;
		data[2 * i + 1] = (hw_data >> 16) & 0xffff;
	}
	hw_data = system_hw_read_32(data_addr + 16);
	data[8] = hw_data & 0xffff;
}

static void write_lc_lcsta_blk_vidx(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data[8];

	system_hw_write_32(addr, 0);
	for (i = 0; i < 4; i++)
	{
		hw_data[2 * i] = (data[2 * i] & 0xffff)|
			((data[2 * i + 1] & 0xffff) << 16);
		system_hw_write_32(data_addr + i * 4, hw_data[i]);
	}
	hw_data[8] = data[8] & 0xffff;
	system_hw_write_32(data_addr + 16, hw_data[8]);
}

static void read_ram_lcmap_nodes(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr,1<<31);
	for (i = 0; i < 96; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[6*i] = hw_data & 0x3ff;
		data[6*i + 1] = (hw_data >> 10) & 0x3ff;
		data[6*i + 2] = (hw_data >> 20) & 0x3ff;

		hw_data = system_hw_read_32(data_addr);

		data[6*i + 3] = (hw_data) & 0x3ff;
		data[6*i + 4] = (hw_data >> 10) & 0x3ff;
		data[6*i + 5] = (hw_data >> 20) & 0x3ff;
	}
}

static void write_ram_lcmap_nodes(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 96; i++)
	{
		hw_data = ((data[i*6+0]&0x3ff)<<0)|((data[i*6+1]&0x3ff)<<10)|
			((data[i*6+2]&0x3ff)<<20);
		system_hw_write_32(data_addr, hw_data);

		hw_data = ((data[i*6+3]&0x3ff)<<0)|((data[i*6+4]&0x3ff)<<10)|
			((data[i*6+5]&0x3ff)<<20);
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_ram_dhzmap_nodes(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr,(1<<31)|(1<<7));
	for (i = 0; i < 96; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[5*i] = hw_data & 0x3ff;
		data[5*i + 1] = (hw_data >> 10) & 0x3ff;
		data[5*i + 2] = (hw_data >> 20) & 0x3ff;

		hw_data = system_hw_read_32(data_addr);

		data[5*i + 3] = (hw_data) & 0x3ff;
		data[5*i + 4] = (hw_data >> 10) & 0x7ff;
	}
}

static void write_ram_dhzmap_nodes(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, (1<<7));
	for (i = 0; i < 96; i++)
	{
		hw_data = ((data[i*5+0]&0x3ff)<<0)|((data[i*5+1]&0x3ff)<<10)|
			((data[i*5+2]&0x3ff)<<20);
		system_hw_write_32(data_addr, hw_data);

		hw_data = ((data[i*5+3]&0x3ff)<<0)|((data[i*5+4]&0x7ff)<<10);
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_dpc1024_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr,0);
	for (i = 0; i < 1024; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data;
	}
}

static void write_dpc1024_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 1024; i++)
	{
		hw_data = data[i];
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_fpnr_corr_val(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data, val;

	val = system_hw_read_32(0x40d << 2);//trigger
	system_hw_write_32(0x40d << 2, val & (1 << 18));//trigger

	int cnt = 0;
	system_hw_write_32(addr, 0);
	for (i = 0; i < 2048; i++) {
		val = system_hw_read_32(data_addr);
		data[cnt++] = val & 0x3ff;
	}

	system_hw_write_32(addr, 0);
	for (i = 0; i < 2048; i++) {
		val = system_hw_read_32(data_addr);
		data[cnt++] = val & 0x3ff;
	}

	system_hw_write_32(addr, 0);
	for (i = 0; i < 2048; i++) {
		val = system_hw_read_32(data_addr);
		data[cnt++] = val & 0x3ff;
	}

	//system_hw_write_32(0x40d << 2, val & (~(1 << 18)));//trigger
}

static void write_fpnr_corr_val(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 2048; i++)
	{
		hw_data = (data[i] & 0x3fff) | ((data[i + 2048] & 0x3fff) << 16)|
			(data[i + 2048 * 2] & 0x3fff) | ((data[i + 2048 * 3] & 0x3fff) << 16);
		system_hw_write_32(data_addr, hw_data);
	}
}

#define SELECT_H_0123(data)    (((data)&(~(3<<10))&(~((1<<7)-1)))|(2<<10)|(1<<9))
#define SELECT_H_4567(data)    (((data)&(~(3<<10))&(~((1<<7)-1)))|(3<<10)|(1<<9))
#define SELECT_V_0123(data)    (((data)&(~(3<<10))&(~((1<<7)-1)))|(0<<10)|(1<<9))
#define SELECT_V_4567(data)    (((data)&(~(3<<10))&(~((1<<7)-1)))|(1<<10)|(1<<9))

static void read_pps_coef(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i,h_tap, v_tap, hsc_tap_data,hw_indx_data, hw_data;
	u32 hsc_tap_data_addr = 0x2080;

	hsc_tap_data = system_hw_read_32(hsc_tap_data_addr);
	h_tap = hsc_tap_data & 0xf;
	v_tap = (hsc_tap_data << 4) & 0xf;

	//h 0123
	hw_indx_data = system_hw_read_32(addr);
	hw_indx_data = SELECT_H_0123(hw_indx_data);
	system_hw_write_32(addr, hw_indx_data);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);

		if (0 < h_tap) data[i*h_tap+0] = ((hw_data>>16) & 0x7ff);
		if (1 < h_tap) data[i*h_tap+1] = ((hw_data) & 0x7ff);

		hw_data = system_hw_read_32(data_addr);

		if (2 < h_tap) data[i*h_tap+2] = ((hw_data>>16) & 0x7ff);
		if (3 < h_tap) data[i*h_tap+3] = ((hw_data) & 0x7ff);
	}

	//h 4567
	hw_indx_data = system_hw_read_32(addr);
	hw_indx_data = SELECT_H_4567(hw_indx_data);
	system_hw_write_32(addr, hw_indx_data);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		if (4 < h_tap) data[i*h_tap+4] = ((hw_data>>16)&0x7ff);
		if (5 < h_tap) data[i*h_tap+5] = ((hw_data)&0x7ff);

		hw_data = system_hw_read_32(data_addr);
		if (6 < h_tap) data[i*h_tap+6] = ((hw_data>>16)&0x7ff);
		if (7 < h_tap) data[i*h_tap+7] = ((hw_data)&0x7ff);
	}

	//v 0123
	hw_indx_data = system_hw_read_32(addr);
	hw_indx_data = SELECT_V_0123(hw_indx_data);
	system_hw_write_32(addr, hw_indx_data);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		if (0 < v_tap) data[i*v_tap + 0 + 33*8] = ((hw_data>>16)&0x7ff);
		if (1 < v_tap) data[i*v_tap + 1 + 33*8] = ((hw_data)&0x7ff);

		hw_data = system_hw_read_32(data_addr);
		if (2 < v_tap) data[i*v_tap + 2 + 33*8] = ((hw_data>>16)&0x7ff);
		if (3 < v_tap) data[i*v_tap + 3 + 33*8] = ((hw_data)&0x7ff);
	}

	// v 4567
	hw_indx_data = system_hw_read_32(addr);
	hw_indx_data = SELECT_V_4567(hw_indx_data);
	system_hw_write_32(addr, hw_indx_data);
	for (i = 0; i < 33; i++)
	{
		hw_data = system_hw_read_32(data_addr);

		if (4 < v_tap) data[i*v_tap+4 + 33*8] = ((hw_data>>16)&0x7ff);
		if (5 < v_tap) data[i*v_tap+5 + 33*8] = ((hw_data)&0x7ff);

		hw_data = system_hw_read_32(data_addr);
		if (6 < v_tap) data[i*v_tap+6 + 33*8] = ((hw_data>>16)&0x7ff);
		if (7 < v_tap) data[i*v_tap+7 + 33*8] = ((hw_data)&0x7ff);
	}
}

static void write_pps_coef(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i,h_tap, v_tap, hsc_tap_data, hw_indx_data, hw_data;
	u32 hsc_tap_data_addr = 0x2080;

	hsc_tap_data = system_hw_read_32(hsc_tap_data_addr);
	h_tap = hsc_tap_data & 0xf;
	v_tap = (hsc_tap_data << 4) & 0xf;

	//h 0123
	hw_indx_data = system_hw_read_32(addr);
	hw_indx_data = SELECT_H_0123(hw_indx_data);
	system_hw_write_32(addr, hw_indx_data);
	for (i = 0; i < 33; i++)
	{
		uint32_t coef[4];

		if (0 < h_tap)     coef[0] = data[i*h_tap+0];
		else            coef[0] = 0;
		if (1 < h_tap)     coef[1] = data[i*h_tap+1];
		else            coef[1] = 0;
		if (2 < h_tap)     coef[2] = data[i*h_tap+2];
		else            coef[2] = 0;
		if (3 < h_tap)     coef[3] = data[i*h_tap+3];
		else            coef[3] = 0;

		hw_data = ((coef[0]&0x7ff)<<16)|((coef[1]&0x7ff));
		system_hw_write_32(data_addr, hw_data);

		hw_data = ((coef[2]&0x7ff)<<16)|((coef[3]&0x7ff));
		system_hw_write_32(data_addr, hw_data);
	}

    //h 4567
    hw_indx_data = system_hw_read_32(addr);
    hw_indx_data = SELECT_H_4567(hw_indx_data);
    system_hw_write_32(addr, hw_indx_data);
    for (i = 0; i < 33; i++)
    {
		uint32_t coef[4];

		if (4 < h_tap)     coef[0] = data[i*h_tap+4];
		else            coef[0] = 0;
		if (5 < h_tap)     coef[1] = data[i*h_tap+5];
		else            coef[1] = 0;
		if (6 < h_tap)     coef[2] = data[i*h_tap+6];
		else            coef[2] = 0;
		if (7 < h_tap)     coef[3] = data[i*h_tap+7];
		else            coef[3] = 0;

		hw_data = ((coef[0]&0x7ff)<<16)|((coef[1]&0x7ff));
		system_hw_write_32(data_addr, hw_data);

		hw_data = ((coef[2]&0x7ff)<<16)|((coef[3]&0x7ff));
		system_hw_write_32(data_addr, hw_data);
	}

	//v 0123
	hw_indx_data = system_hw_read_32(addr);
	hw_indx_data = SELECT_V_0123(hw_indx_data);
	system_hw_write_32(addr, hw_indx_data);
	for (i = 0; i < 33; i++)
	{
		uint32_t coef[4];

		if (0 < v_tap)     coef[0] = data[i*v_tap+0 + 33*8];
		else            coef[0] = 0;
		if (1 < v_tap)     coef[1] = data[i*v_tap+1 + 33*8];
		else            coef[1] = 0;
		if (2 < v_tap)     coef[2] = data[i*v_tap+2 + 33*8];
		else            coef[2] = 0;
		if (3 < v_tap)     coef[3] = data[i*v_tap+3 + 33*8];
		else            coef[3] = 0;

		hw_data = ((coef[0]&0x7ff)<<16)|((coef[1]&0x7ff));
		//hw_data_sum += reg_data;
		system_hw_write_32(data_addr, hw_data);

		hw_data = ((coef[2]&0x7ff)<<16)|((coef[3]&0x7ff));
		//hw_data_sum += reg_data;
		system_hw_write_32(data_addr, hw_data);
	}

	//v 4567
	hw_indx_data = system_hw_read_32(addr);
	hw_indx_data = SELECT_V_4567(hw_indx_data);
	system_hw_write_32(addr, hw_indx_data);
	for (i = 0; i < 33; i++)
	{
		uint32_t coef[4];

		if (4 < v_tap)     coef[0] = data[i*v_tap+4 + 33*8];
		else            coef[0] = 0;
		if (5 < v_tap)     coef[1] = data[i*v_tap+5 + 33*8];
		else            coef[1] = 0;
		if (6 < v_tap)     coef[2] = data[i*v_tap+6 + 33*8];
		else            coef[2] = 0;
		if (7 < v_tap)     coef[3] = data[i*v_tap+7 + 33*8];
		else            coef[3] = 0;

		hw_data = ((coef[0]&0x7ff)<<16)|((coef[1]&0x7ff));
		//hw_data_sum += hw_data;
		system_hw_write_32(data_addr, hw_data);

		hw_data = ((coef[2]&0x7ff)<<16)|((coef[3]&0x7ff));
		//hw_data_sum += reg_data;
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_lc_region_out(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr,0);
	for (i = 0; i < 1632; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0x7fffffff;
	}
}

static void write_lc_region_out(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 1632; i++) {
		hw_data = data[i] & 0x7fffffff;
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_luma_stats(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data;

	system_hw_write_32(addr,0);
	for (j = 0 ;j < 4;j++) {
		for (i = 0;i < 16; i++) {
			hw_data = system_hw_read_32(data_addr);
		if (i % 2 == 0) {
			data[i / 2 + j * 8] = hw_data & 0xffff;
			} else if (i % 2 == 1) {
					data[i / 2 + j * 8] = (hw_data >> 16) & 0xffff;
			}
		}
	}
}

static void write_luma_stats(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j ,hw_data;

	system_hw_write_32(addr, 0);
	for (j = 0 ;j < 4;j++) {
		for (i = 0;i < 16; i++) {
		if (i % 2 == 0) {
			hw_data = data[i / 2 + j * 8] & 0xffff;
			system_hw_write_32(data_addr, hw_data);
			} else if (i % 2 == 1) {
				hw_data = (data[i / 2 + j * 8] & 0xffff) << 16;
				system_hw_write_32(data_addr, hw_data);
			}
		}
	}
}

static void read_ro_luma_stats_sum(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 16; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xffffffff;
	}
}

static void read_post_sta_glb_hist_out(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 256; i++)
	{
		hw_data = system_hw_read_32(data_addr);
		data[i] = hw_data & 0xfffffff;
	}
}

static void write_post_sta_glb_hist_out(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;

	system_hw_write_32(addr, 0);
	for (i = 0; i < 256; i++) {
		hw_data = data[i] & 0xfffffff;
		system_hw_write_32(data_addr, hw_data);
	}
}

static void read_draw_fram(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data;
#if 0
	system_hw_write_32(addr,0);
	for (j = 0 ;j < 4;j++) {
		for (i = 0;i < 16; i++) {
			hw_data = system_hw_read_32(data_addr);
		if (i % 2 == 0) {
			data[i % 2 + j * 8] = hw_data & 0xffff;
			} else if (i % 2 == 1) {
					data[i % 2 + j * 8] = (hw_data >> 16) & 0xffff;
			}
		}
	}
	for (i = 0;i < 16;i++) {
		hw_data = system_hw_read_32(data_addr);
		if (i % 4 == 0 ) {
			data[32 + i] = hw_data & 0xff;
			}else if (i % 4 == 1){
			data[32 + i] = (hw_data << 8) & 0xff;
			}else if (i % 4 == 2){
			data[32 + i] = (hw_data << 16) & 0xff;
			}else if (i % 4 == 3){
			data[32 + i] = (hw_data << 24) & 0xff;
			}
	}
	for (i = 0;i < 32;i++) {
		hw_data = system_hw_read_32(data_addr);
		data[48] = (hw_data << i) & 0x1;
		hw_data = system_hw_read_32(data_addr);
		data[49] = (hw_data << i) & 0x1;
	}
	#endif
}

static void write_draw_fram(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, hw_data;
	#if 0
	system_hw_write_32(addr, 0);
	#endif
}

static void read_disp_dth_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j, hw_data;
	system_hw_write_32(addr & 0xffff, 0);
	if (addr & 0xf0000) {
		for (j = 0; j < 16; j++)
		{
			for ( i = 0; i < 32; i++) {
				hw_data = system_hw_read_32(data_addr);
				data[i + j * 32] = (hw_data << i) & 0x1;
			}
		}
	}else{
		for (j = 0; j < 16; j++)
		{
			for( i = 0; i < 32; i++){
				hw_data = system_hw_read_32(data_addr);
				data[i + j * 32] = (hw_data << i) & 0x1;
			}
		}
	}
}

static void write_disp_dth_lut(u32 addr, u32 data_addr, u32 *data, u32 len)
{
	u32 i, j,hw_data;

	system_hw_write_32(addr & 0xffff, 0);

	if (addr & 0xf0000) {
		for (j = 0; j < 16; j++)
		{
			for ( i = 0; i < 32; i++) {
				hw_data = (data[i + j * 32] & 0x1) >> i;
				system_hw_write_32(data_addr, hw_data);
			}
		}
	}else{
		for (j = 0; j < 16; j++)
		{
			for ( i = 0; i < 32; i++) {
				hw_data = (data[i + j * 32] & 0x1) >> i;
				system_hw_write_32(data_addr, hw_data);
			}
		}
	}
}

static void read_lc_yminval_lmt(u32 addr, u32 data_addr, u32 *data, u32 len){
	u32 i, j,hw_data;
	system_hw_write_32(addr,0);
		for (i = 0;i < 8; i++) {
			hw_data = system_hw_read_32(data_addr + 2 + i);
			data[2 * i] = hw_data & 0x1ff;
			data[2*i + 1] = (hw_data >> 16) & 0x1ff;
		}
}

static void write_lc_yminval_lmt(u32 addr, u32 data_addr, u32 *data, u32 len){
	u32 i, j,hw_data;
	system_hw_write_32(addr,0);
	for (i = 0;i < 8; i++) {
		hw_data = (data[2*i] & 0x1ff) |((data[2*i+1]<<16) & 0x1ff);
		system_hw_write_32(data_addr + 2 + i, hw_data);
	}
}

static void read_lc_ypkbv_lmt(u32 addr, u32 data_addr, u32 *data, u32 len){
	u32 i, j,hw_data;
	system_hw_write_32(addr,0);
		for (i = 0;i < 8; i++) {
			hw_data = system_hw_read_32(data_addr + 2 + 2 * i);
			data[2 * i] = hw_data & 0x1ff;
			data[2*i + 1] = (hw_data >> 16) & 0x1ff;
		}
}

static void write_lc_ypkbv_lmt(u32 addr, u32 data_addr, u32 *data, u32 len){
	u32 i,hw_data;
	system_hw_write_32(addr,0);
	for (i = 0;i < 8; i++) {
		hw_data = (data[2*i] & 0x1ff) |((data[2*i+1]<<16) & 0x1ff);
		system_hw_write_32(data_addr + 2 + 2 * i, hw_data);
	}
}

static void read_lc_ymaxval_lmt(u32 addr, u32 data_addr, u32 *data, u32 len){
	u32 i, j,hw_data;
	system_hw_write_32(addr,0);
		for (i = 0;i < 8; i++) {
			hw_data = system_hw_read_32(data_addr + 2 + 3 * i);
			data[2 * i] = hw_data & 0x1ff;
			data[2*i + 1] = (hw_data >> 16) & 0x1ff;
		}
}

static void write_lc_ymaxval_lmt(u32 addr, u32 data_addr, u32 *data, u32 len){
	u32 i,hw_data;
	system_hw_write_32(addr,0);
	for (i = 0;i < 8; i++) {
		hw_data = (data[2*i] & 0x1ff) |((data[2*i+1]<<16) & 0x1ff);
		system_hw_write_32(data_addr + 2 + 3 * i, hw_data);
	}
}

static void read_lc_ypkbv_ratio(u32 addr, u32 data_addr, u32 *data, u32 len){

	u32 hw_data;
	hw_data = system_hw_read_32(data_addr + 22);
	data[0] = hw_data & 0xff;
	data[1] = (hw_data >> 8) & 0xff;
	data[2] = (hw_data >> 16) & 0xff;
	data[3] = (hw_data >> 24) & 0xff;
	}

static void write_lc_ypkbv_ratio(u32 addr, u32 data_addr, u32 *data, u32 len){
	u32 hw_data;

	system_hw_write_32(addr, 0);
		hw_data = (data[0] & 0xff)|((data[1] >> 8) & 0xff)|((data[2] >> 16) & 0xff)
				|((data[3] >> 24) & 0xff);
		system_hw_write_32(data_addr + 22, hw_data);
}

static void read_lc_cbus2ram_data(u32 addr, u32 data_addr, u32 *data, u32 len){

	u32 i,hw_data,hw_ram_en;
	system_hw_write_32(addr - 1, 1);
	system_hw_write_32(addr, 1<<31);

	for (i = 0; i < 12*8; i++)
	{
		hw_data = system_hw_read_32(addr);

		data[i*6+0] = (hw_data>>(0))&0x3ff;
		data[i*6+1] = (hw_data>>(10))&0x3ff;
		data[i*6+2] = (hw_data>>(20))&0x3ff;

		hw_data = system_hw_read_32(addr);
		data[i*6+3] = (hw_data>>(0))&0x3ff;
		data[i*6+4] = (hw_data>>(10))&0x3ff;
		data[i*6+5] = (hw_data>>(20))&0x3ff;
	}
	system_hw_write_32(addr - 1, 0);
}

static void write_lc_cbus2ram_data(u32 addr, u32 data_addr, u32 *data, u32 len){
	u32 i,hw_data;

	system_hw_write_32(addr - 1, 1);
	system_hw_write_32(addr, 0);

	for (i = 0; i < 12*8; i++)
	{
		hw_data = ((data[i*6+0]&0x3ff)<<0)|((data[i*6+1]&0x3ff)<<10)|((data[i*6+2]&0x3ff)<<20);
		system_hw_write_32(data_addr, hw_data);

		hw_data = ((data[i*6+3]&0x3ff)<<0)|((data[i*6+4]&0x3ff)<<10)|((data[i*6+5]&0x3ff)<<20);
			system_hw_write_32(data_addr, hw_data);
	}
	system_hw_write_32(addr - 1, 0);
}

static struct lut_list lut_table[] = {
	{0x0790 << 2, 0x0791 << 2,    65,   17, read_rad_lut65, write_rad_lut65},//rad_lut65
	{0x0e10 << 2, 0x0e11 << 2,   129,   65, read_gtm_lut, write_gtm_lut},//gtm_lut129
	{0x0df0 << 2, 0x0df1 << 2,   516,  260, read_lns_rad_lut, write_lns_rad_lut},//lns_rad_lut1024
	{0x0df2 << 2, 0x0df3 << 2, 16384, 4096, read_lns_mesh_lut, write_lns_mesh_lut},  //cant read correctly
	{0x071a << 2, 0x071b << 2,    33,   33, read_dfe_pre_sqrt0_lut, write_dfe_pre_sqrt0_lut},
	{0x071c << 2, 0x071d << 2,   129,  129, read_dfe_pre_sqrt1_lut, write_dfe_pre_sqrt1_lut},
	{0x0790 << 2, 0x0791 << 2,    32,   32, read_dfe_nr_cubic_lut, write_dfe_nr_cubic_lut},
	{0x1430 << 2, 0x1431 << 2,    33,   33, read_af_stat_idx_lut, write_af_stat_idx_lut},
	{0x1580 << 2, 0x1581 << 2,    65,   65, read_ae_stat_idx_lut, write_ae_stat_idx_lut},
	{0x1582 << 2, 0x1583 << 2, 255, 32, read_ae_stat_blk_wet_lut, write_ae_stat_blk_wet_lut},
	{0x1680 << 2, 0x1681 << 2,    33,   33, read_awb_stat_idx_lut, write_awb_stat_idx_lut},
	{0x1682 << 2, 0x1683 << 2,  768,  768, read_awb_stat_blk_wet_lut, read_awb_stat_blk_wet_lut},
	{0x0881 << 2, 0x0880 << 2, 4096, 1024, read_cac_table_lut, write_cac_table_lut},
	{0x4f0  << 2, 0x4f1  << 2,     33,  33, read_decmp0_eotf0_lut, write_decmp0_eotf0_lut},
	{0x4f2  << 2, 0x4f3  << 2,    129, 129, read_decmp1_eotf1_lut, write_decmp1_eotf1_lut},
	{0xdf4 << 2, 0xdf5 << 2,     33,  33, read_decmp0_eotf0_lut, write_decmp0_eotf0_lut},
	{0xdf6 << 2, 0xdf7 << 2,    129, 129, read_decmp1_eotf1_lut, write_decmp1_eotf1_lut},
	{0x0280 << 2, 0x0281 << 2,   1025,  32, read_dpc1024_lut, write_dpc1024_lut},
	{0xf30 << 2, 0xf31 << 2,     79,  79, read_ltm_histxptsbuf_blk, write_ltm_histxptsbuf_blk},
	{0xf32 << 2, 0xf33 << 2,    96, 96, read_ltm_lm_blk, write_ltm_lm_blk},
	{0xf34 << 2, 0xf35 << 2,     63,  33, read_ltm_ccrat_lut, write_ltm_ccrat_lut},
	{0xf36 << 2, 0xf37 << 2,    256,  32, read_chroma_coef, write_chroma_coef},
	{0xf38 << 2, 0xf39 << 2,     96,  32, read_ro_ltm, NULL},
	{0xf3a << 2, 0xf3b << 2,     96,  32, read_ro_ltm, NULL},
	{0xf3c << 2, 0xf3d << 2,     96,  32, read_ro_ltm_sta_hst_blk_sum, NULL},
	{0xf3e << 2, 0xf3f << 2,    513,  32, read_ro_ltm_histbuf, NULL},
	{0x5b0  << 2, 0x5b1  << 2,   1024,1024, read_ro_wdr_stat_yblk_flt, NULL},
	{0x1205 << 2, 0x1206 << 2,   1024, 128, read_cnr2_satur_blk, write_cnr2_satur_blk},
	{0x132b << 2, 0x132b << 2,    257,  32, read_pk_clr_prct_lut, write_pk_clr_prct_lut},
	{0x0b6d << 2, 0x0b6e <<  2,    64,  32, read_lut_meta_sad_2alpha, write_lut_meta_sad_2alpha},
	{0x1305 << 2, 0x1305 <<  2,    65,  32, read_dnlp_ygrid, write_dnlp_ygrid},
	{0x1365 << 2, 0x1366 <<  2,    64,  32, read_lc_satur_lut, write_lc_satur_lut},
	{0x1367 << 2, 0x1368 <<  2,    65,  33, read_dhz_sky_prot_lut, write_dhz_sky_prot_lut},
	{0x1137 << 2, 0x1138 <<  2,  2187,1458, read_lut3d_ram, write_lut3d_ram},
	{0x1133 << 2, 0x1134 <<  2,   129,  32, read_pst_gamma_lut , write_pst_gamma_lut},
	{0x0950 << 2, 0x0951 <<  2,    96,   8, read_snr_lpf_phs_sel, write_snr_lpf_phs_sel},
	{0x11c7 << 2, 0x11c7 <<  2,    13,   7, read_lc_lcsta_blk_hidx, write_lc_lcsta_blk_hidx},
	{0x11ce << 2, 0x11ce <<  2,     9,   5, read_lc_lcsta_blk_vidx, write_lc_lcsta_blk_vidx},
	{0x1369 << 2, 0x1369 <<  2,    14,  13, read_lc_lcsta_blk_hidx, write_lc_lcsta_blk_hidx},
	{0x1370 << 2, 0x1370 <<  2,    10,   9, read_lc_lcsta_blk_vidx, write_lc_lcsta_blk_vidx},
	{0x11f4 << 2, 0x11f5 <<  2,  1632,1632, read_lc_region_out, write_lc_region_out},
	{0x1384 << 2, 0x1385 <<  2,   576, 576, read_ram_lcmap_nodes, write_ram_lcmap_nodes},
	{0x1385 << 2, 0x1370 <<  2,   481, 481, read_ram_dhzmap_nodes, write_ram_dhzmap_nodes},
	{0x021e << 2, 0x021f <<  2, 2048*3,2048, read_fpnr_corr_val, write_fpnr_corr_val},
	{0x1790 << 2, 0x1791 <<  2,   528, 528, read_pps_coef, write_pps_coef},
	{0x1792 << 2, 0x1793 <<  2,   528, 528, read_pps_coef, write_pps_coef},
	{0x17a1 << 2, 0x17a2 <<  2,    64,  32, read_luma_stats, write_luma_stats},
	{0x17a3 << 2, 0x17a4 <<  2,    16,  16, read_ro_luma_stats_sum, NULL},
	{0x11f7 << 2, 0x11f8 <<  2,   256, 256, read_post_sta_glb_hist_out, write_post_sta_glb_hist_out},
	{0x17e1 << 2, 0x17e2 <<  2,    50,  50, read_draw_fram, write_draw_fram},
	{0x1711 << 2, 0x1710 <<  2,   512, 512, read_disp_dth_lut, write_disp_dth_lut},
	{0x13a1 << 2, 0x13a2 <<  2,    16,  16, read_lc_yminval_lmt, write_lc_yminval_lmt},
	{0x13a1 << 2, 0x13a2 <<  2,    16,  16, read_lc_ypkbv_lmt, write_lc_ypkbv_lmt},
	{0x13a1 << 2, 0x13a2 <<  2,    16,  16, read_lc_ymaxval_lmt, write_lc_ymaxval_lmt},
	{0x13a1 << 2, 0x13a2 <<  2,    16,  16, read_lc_ypkbv_ratio, write_lc_ypkbv_ratio},
	{0x1384 << 2, 0x1385 <<  2,    16,  16, read_lc_cbus2ram_data, write_lc_cbus2ram_data},
};

int aisp_get_lut_id(u32 addr)
{
	int ret = 0;
	u32 i;
	for (i = 0; i < NELEM(lut_table); i++) {
		if (lut_table[i].addr_addr == (addr & 0xffff)) {
			ret = i;
			break;
		}
	}

	if (i >= NELEM(lut_table))
		ret = -1;

	return ret;
}

void aisp_read_lut(u32 addr, u32 *buf, u32 size)
{
	int idx;

	idx = aisp_get_lut_id(addr);
	if (idx < 0) {
		pr_err("wrong lut addr 0x%x", addr / 4);
		return;
	}

	if (lut_table[idx].lut_len != size) {
		pr_err("wrong lut size %d", size);
		return;
	}

	if (lut_table[idx].read_lut)
		lut_table[idx].read_lut(lut_table[idx].addr_addr, lut_table[idx].data_addr, buf, size);

}
void aisp_write_lut(u32 addr, u32 *buf, u32 size)
{
	int idx;

	idx = aisp_get_lut_id(addr);
	if (idx < 0) {
		pr_err("wrong lut addr 0x%x", addr / 4);
		return;
	}

	if (lut_table[idx].lut_len != size) {
		pr_err("wrong lut size %d", size);
		return;
	}

	if (lut_table[idx].write_lut)
		lut_table[idx].write_lut(lut_table[idx].addr_addr, lut_table[idx].data_addr, buf, size);
}


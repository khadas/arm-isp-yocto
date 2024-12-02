/*
*
* SPDX-License-Identifier: GPL-2.0
*
* Copyright (C) 2020 Amlogic or its affiliates
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 2.
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#if !defined( __AISP_CONNECTION_H__ )
#define __AISP_CONNECTION_H__

#define BUS_ERROR_RESET -1
#define BUS_ERROR_FATAL -2
#define ISP_CONFIG_LUT_OFFSET    0x10000
#define ISP_CONFIG_PING_OFFSET    0x18E88
#define ISP_CONFIG_PING_SIZE    0x17FC0

void aisp_connection_init( void );
void aisp_connection_process( void );
void aisp_connection_destroy( void );
int connection_thread( void *foo );

struct isp_info_t {
    struct clk *isp_clk;
    struct clk *vapb_clk;
    struct device *dev;
    void __iomem *base;
};

uint32_t system_hw_read_32( u32 addr );

void system_hw_write_32( u32 addr, u32 data);

extern struct task_struct *isp_fw_connections_thread;

#endif /* __AISP_CONNECTION_H__ */

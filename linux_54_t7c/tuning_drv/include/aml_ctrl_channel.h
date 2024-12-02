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

#ifndef AISP_CTRL_CHANNEL_H
#define AISP_CTRL_CHANNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "linux/types.h"

#define CTRL_CHANNEL_DEV_NAME "ac_isp4uf0"
#define CTRL_CHANNEL_DEV_NODE_NAME "/dev/" CTRL_CHANNEL_DEV_NAME

/*#define CTRL_CHANNEL1_DEV_NAME "ac_isp4uf1"
#define CTRL_CHANNEL1_DEV_NODE_NAME "/dev/" CTRL_CHANNEL1_DEV_NAME

#define CTRL_CHANNEL2_DEV_NAME "ac_isp4uf2"
#define CTRL_CHANNEL2_DEV_NODE_NAME "/dev/" CTRL_CHANNEL2_DEV_NAME

#define CTRL_CHANNEL3_DEV_NAME "ac_isp4uf3"
#define CTRL_CHANNEL3_DEV_NODE_NAME "/dev/" CTRL_CHANNEL3_DEV_NAME

#define CTRL_CHANNEL4_DEV_NAME "ac_isp4uf4"
#define CTRL_CHANNEL4_DEV_NODE_NAME "/dev/" CTRL_CHANNEL4_DEV_NAME

#define CTRL_CHANNEL5_DEV_NAME "ac_isp4uf5"
#define CTRL_CHANNEL5_DEV_NODE_NAME "/dev/" CTRL_CHANNEL5_DEV_NAME*/


#define CTRL_CHANNEL_MAX_CMD_SIZE ( 8 * 1024 )


enum ctrl_cmd_category {
    CTRL_CMD_CATEGORY_API_COMMAND = 1,
    CTRL_CMD_CATEGORY_API_CALIBRATION,
};

struct ctrl_cmd_item {
    /* command metadata */
    uint32_t cmd_len;
    uint8_t cmd_category;

    /* command content */
    uint8_t cmd_type;
    uint8_t cmd_id;
    uint8_t cmd_direction;
    uint32_t cmd_value;
};

int ctrl_channel_init( void );
//void ctrl_channel_process( void );
void ctrl_channel_deinit( void );

/*int ctrl_channel1_init( void );
void ctrl_channel1_deinit( void );

int ctrl_channel2_init( void );
void ctrl_channel2_deinit( void );

int ctrl_channel3_init( void );
void ctrl_channel3_deinit( void );

int ctrl_channel4_init( void );
void ctrl_channel4_deinit( void );

int ctrl_channel5_init( void );
void ctrl_channel5_deinit( void );*/

//void ctrl_channel_handle_command( uint8_t command_type, uint8_t command, uint32_t value, uint8_t direction, uint32_t *ret_value );
//void ctrl_channel_handle_api_calibration( uint8_t type, uint8_t id, uint8_t direction, void *data, uint32_t data_size, uint32_t *ret_value );

uint8_t application_command0( uint8_t command_type, uint8_t command, uint32_t value, uint8_t direction, uint32_t *ret_value );
uint8_t application_api_calibration0( uint8_t type, uint8_t id, uint8_t direction, void *data, uint32_t data_size, uint32_t *ret_value );

/*uint8_t application_command1( uint8_t command_type, uint8_t command, uint32_t value, uint8_t direction, uint32_t *ret_value );
uint8_t application_api_calibration1( uint8_t type, uint8_t id, uint8_t direction, void *data, uint32_t data_size, uint32_t *ret_value );

uint8_t application_command2( uint8_t command_type, uint8_t command, uint32_t value, uint8_t direction, uint32_t *ret_value );
uint8_t application_api_calibration2( uint8_t type, uint8_t id, uint8_t direction, void *data, uint32_t data_size, uint32_t *ret_value );

uint8_t application_command3( uint8_t command_type, uint8_t command, uint32_t value, uint8_t direction, uint32_t *ret_value );
uint8_t application_api_calibration3( uint8_t type, uint8_t id, uint8_t direction, void *data, uint32_t data_size, uint32_t *ret_value );

uint8_t application_command4( uint8_t command_type, uint8_t command, uint32_t value, uint8_t direction, uint32_t *ret_value );
uint8_t application_api_calibration4( uint8_t type, uint8_t id, uint8_t direction, void *data, uint32_t data_size, uint32_t *ret_value );

uint8_t application_command5( uint8_t command_type, uint8_t command, uint32_t value, uint8_t direction, uint32_t *ret_value );
uint8_t application_api_calibration5( uint8_t type, uint8_t id, uint8_t direction, void *data, uint32_t data_size, uint32_t *ret_value );*/


#ifdef __cplusplus
}
#endif

#endif /* AISP_CTRL_CHANNEL_H */

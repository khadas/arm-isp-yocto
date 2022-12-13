/*
*
* SPDX-License-Identifier: GPL-2.0
*
* Copyright (C) 2022 Amlogic or its affiliates
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


#ifndef __CHIP_TYPE_H__
#define __CHIP_TYPE_H__

#define   CHIP_TYPE_T7        0
#define   CHIP_TYPE_T7C       1

int init_chip_type(void);
int get_chip_type(void);


#endif


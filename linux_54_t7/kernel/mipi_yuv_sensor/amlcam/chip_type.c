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

#include "linux/printk.h"

#include "linux/amlogic/cpu_info.h"

#include "chip_type.h"

static volatile int chip_type = CHIP_TYPE_T7;

int init_chip_type(void)
{
    unsigned char chip_id[CHIPID_LEN+1];

    cpuinfo_get_chipid(&chip_id[0],CHIPID_LEN );

    //t7 :  36 0b 010100000000040d00c911105690
    // t7c: 36 0c 0103000000002d09e53930563754
    int chip_rev = chip_id [1];

    printk( "chip type rev 0x%x", chip_rev);

    if (0x0c == chip_rev ) {
        printk( "chip type t7c", chip_rev);
        chip_type = CHIP_TYPE_T7C;
    } else {
        printk( "chip type t7", chip_rev);
        chip_type = CHIP_TYPE_T7;
    }
    return chip_type;
}


int get_chip_type(void)
{
    return chip_type;
}



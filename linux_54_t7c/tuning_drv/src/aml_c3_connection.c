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

#include <linux/device.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/pm_domain.h>

#include <linux/vmalloc.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
//#include "aml_c3_cam.h"

#define ALIGNMENT_MASK 0
#include "aml_chardev.h"

#include "aml_command_api.h"
#include "aml_ctrl_channel.h"
#include "aml_c3_connection.h"
#include "aml_c3_lut.h"

//#include "system_hw_io.h"
//#include "system_sw_io.h"

#define AISP_CONNECTION_TRACE( ... )

#define HEADER_SIZE 12

enum {
    API_RESET = 0,
    API_READ,
    API_WRITE
};

enum TransactionType {
    TransactionTypeRegRead = 1,
    TransactionTypeRegWrite,
    TransactionTypeRegMaskWrite,
    TransactionTypeLUTRead,
    TransactionTypeLUTWrite,

    TransactionTypeAPIRead = 10,
    TransactionTypeAPIWrite,

    TransactionTypeBUFRead = 20,
    TransactionTypeBUFWrite
};

typedef int ( *data_read_f )( void *p_ctrl, uint8_t *data, int size );
typedef int ( *data_write_f )( void *p_ctrl, const uint8_t *data, int size );

enum {
    STATE_IDLE,
    STATE_RX_DATA,
    STATE_SKIP_DATA,
    STATE_TX_PACKET
};

typedef struct {
    void *param;
    data_read_f data_read;
    data_write_f data_write;
    int state;
    uint8_t *buffer;
    uint32_t rx_buffer_inx;
    uint32_t rx_buffer_size;
    uint32_t tx_buffer_inx;
    uint32_t tx_buffer_size;
} connection_t;

#define CONNECTION_BUFFER_SIZE (80*1024)

#define ISP_HAS_CONNECTION_CHARDEV 1
#define ISP_HAS_STREAM_CONNECTION  1

#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )

#if ISP_HAS_STREAM_CONNECTION
static connection_t con;
static int camera_id = 0;
static void *p_hw_base = NULL;
static struct isp_info_t dev_info[4];
module_param(camera_id, uint, 0664);
MODULE_PARM_DESC(camera_id, "\n choose path\n");

static void reset_connection( void )
{
    con.state = STATE_IDLE;
    con.rx_buffer_inx = 0;
    con.tx_buffer_inx = 0;
}

extern void *aisp_get_api_ctx_ptr( void );
#endif //ISP_HAS_STREAM_CONNECTION

void aisp_connection_init( void )
{
    if ( !aisp_chardev_init() ) {
        con.data_read = (data_read_f)aisp_chardev_read;
        con.data_write = (data_write_f)aisp_chardev_write;
        con.buffer = vmalloc(CONNECTION_BUFFER_SIZE);
    }
}

uint8_t application_command( int idx, uint8_t command_type, uint8_t command, uint32_t value, uint8_t direction, uint32_t *ret_value )
{
    uint8_t ret = SUCCESS;
    switch (idx) {
        case 0:
            ret = application_command0( command_type, command, value, direction, ret_value );
            break;
        /*case 1:
            ret = application_command1( command_type, command, value, direction, ret_value );
            break;
        case 2:
            ret = application_command2( command_type, command, value, direction, ret_value );
            break;
        case 3:
            ret = application_command3( command_type, command, value, direction, ret_value );
            break;*/
        default:
            ret = application_command0( command_type, command, value, direction, ret_value );
            printk("wrong camera idx for cmd.\n");
            break;
    }
    return ret;
}

uint8_t application_api_calibration( int idx, uint8_t type, uint8_t id, uint8_t direction, void *data, uint32_t data_size, uint32_t *ret_value )
{
    uint8_t ret = SUCCESS;
    switch (idx) {
        case 0:
            ret = application_api_calibration0( type, id, direction, data, data_size, ret_value );
            break;
        /*case 1:
            ret = application_api_calibration1( type, id, direction, data, data_size, ret_value );
            break;
        case 2:
            ret = application_api_calibration2( type, id, direction, data, data_size, ret_value );
            break;
        case 3:
            ret = application_api_calibration3( type, id, direction, data, data_size, ret_value );
            break;*/
        default:
            ret = application_api_calibration0( type, id, direction, data, data_size, ret_value );
            printk("wrong camera idx for api_calibration.\n");
            break;
    }

    return ret;
}

void aisp_connection_destroy( void )
{
#if ISP_HAS_STREAM_CONNECTION
#if ISP_HAS_CONNECTION_CHARDEV
    int rc = aisp_chardev_destroy();
    if ( rc )
        printk("Unable to destroy character device with error %d", rc );
#endif /* connection type */
    con.param = NULL;
    con.data_read = NULL;
    con.data_write = NULL;
    if (con.buffer)
        vfree(con.buffer);
#endif /* ISP_HAS_STREAM_CONNECTION */
}

uint32_t system_hw_read_32( u32 addr )
{
    uint32_t result = 0;
    if ( p_hw_base != NULL ) {
        result = readl( p_hw_base + addr );
    }

    return result;
}

void system_hw_write_32( u32 addr, uint32_t data )
{
    if ( p_hw_base != NULL ) {
        void *ptr = (void *)( p_hw_base + addr );
        writel( data, ptr );
    }
}

#if ISP_HAS_STREAM_CONNECTION
static void write_32( uint32_t addr, uint8_t value, uint8_t msk )
{
    int shift = ( addr & 3 ) << 3;
    uint32_t mask = msk << shift;
    uint32_t addr_align = addr & ~3;
    uint32_t rc = 0;
    // Use SW registers for ping/pong memory, otherwise, use HW registers.
    if ( addr < ISP_CONFIG_LUT_OFFSET ) {
        uint32_t data = system_hw_read_32( addr_align );
        data = ( data & ~mask ) | ( ( uint32_t )( value & msk ) << shift );
        system_hw_write_32( addr_align, data );
    } else {
        // read from software context
        uintptr_t sw_addr = addr_align;
        uint32_t data = 0;
        application_command( camera_id, TREGISTERS, REGISTERS_SOURCE_ID, ISP, COMMAND_SET, &rc );
        application_command( camera_id, TREGISTERS, REGISTERS_SIZE_ID, 32, COMMAND_SET, &rc );
        application_command( camera_id, TREGISTERS, REGISTERS_ADDRESS_ID, sw_addr, COMMAND_SET, &rc );
        application_command( camera_id, TREGISTERS, REGISTERS_VALUE_ID, 0, COMMAND_GET, &data );
        data = ( data & ~mask ) | ( ( uint32_t )( value & msk ) << shift );
        application_command( camera_id, TREGISTERS, REGISTERS_VALUE_ID, data, COMMAND_SET, &rc );
    }
}

static uint32_t read_32( uint32_t addr )
{
    uint32_t addr_align = addr & ~3;
    uint32_t data = 0;

    // Use SW registers for ping/pong memory, otherwise, use HW registers.
    if ( addr < ISP_CONFIG_LUT_OFFSET ) {
        data = system_hw_read_32( addr_align );
    } else {
        uintptr_t sw_addr = addr_align;
        data = system_hw_read_32( sw_addr );
    }

    return data;
}

static void process_request( void )
{
    uint32_t *rx_buf = (uint32_t *)&con.buffer[8];
    uint32_t *tx_buf = (uint32_t *)con.buffer;
    uint16_t type = ( *rx_buf++ ) & 0xFFFF;
    switch ( type ) {
    case TransactionTypeRegRead:
        if ( con.rx_buffer_size == HEADER_SIZE + 8 ) {
            uint32_t addr = *rx_buf++;
            uint32_t size = *rx_buf++;
            uint32_t value;
            uint32_t tmp_size = size / 4;
            if ( size <= CONNECTION_BUFFER_SIZE - HEADER_SIZE - 4 ) {
                uint8_t *b = &con.buffer[HEADER_SIZE + 4];
                con.tx_buffer_size = HEADER_SIZE + 4 + size;
                tx_buf[3] = SUCCESS;
                if (addr == 0x40000) {
                    *b = camera_id;
                    printk("camera id to user setting : %d", camera_id);
                } else {
                    while ( tmp_size-- ) {
                        value = read_32(addr); //system_sw_read_8(addr++);
                        *b++ = (value >> 0) & 0xff;
                        *b++ = (value >> 8) & 0xff;
                        *b++ = (value >> 16) & 0xff;
                        *b++ = (value >> 24) & 0xff;
                        addr += 4;
                    }
                }
            } else {
                con.tx_buffer_size = HEADER_SIZE + 4;
                tx_buf[3] = FAIL;
            }
        } else {
            con.tx_buffer_size = HEADER_SIZE;
        }
        break;
    case TransactionTypeRegWrite:
        if ( con.rx_buffer_size >= HEADER_SIZE + 8 ) {
            uint32_t addr = *rx_buf++;
            uint32_t size = *rx_buf++;
            con.tx_buffer_size = HEADER_SIZE + 4;
            if ( size <= con.rx_buffer_size - HEADER_SIZE - 8 ) {
                uint8_t *b = &con.buffer[HEADER_SIZE + 8];
                tx_buf[3] = SUCCESS;
                if (addr == 0x40000) {
                    camera_id = *b;
                    if (dev_info[camera_id].base)
                        p_hw_base = dev_info[camera_id].base;
                    else
                        p_hw_base = NULL;
                    //printk("camera id from user setting : 0x%x, 0x%x, 0x%x, 0x%x", *b, *(b+1), *(b+2), *(b+3));
                } else {
                    while ( size-- ) {
                        write_32( addr++, *b++, 0xFF );
                    }
                }
            } else {
                tx_buf[3] = FAIL;
            }

        } else {
            con.tx_buffer_size = HEADER_SIZE;
        }
        break;
    case TransactionTypeRegMaskWrite:
        if ( con.rx_buffer_size >= HEADER_SIZE + 8 ) {
            uint32_t addr = *rx_buf++;
            uint32_t size = *rx_buf++;
            con.tx_buffer_size = HEADER_SIZE + 4;
            if ( 2 * size <= con.rx_buffer_size - HEADER_SIZE - 8 ) {
                uint8_t *b = &con.buffer[HEADER_SIZE + 8];
                uint8_t *m = &con.buffer[HEADER_SIZE + 8 + size];
                tx_buf[3] = SUCCESS;
                if (addr == 0x40000) {
                    camera_id = *b;
                    if (dev_info[camera_id].base)
                        p_hw_base = dev_info[camera_id].base;
                    else
                        p_hw_base = NULL;
                    //printk("camera id from user setting : 0x%x, 0x%x, 0x%x, 0x%x", *b, *(b+1), *(b+2), *(b+3));
                } else {
                    while ( size-- ) {
                        uint8_t mask = *m++;
                        uint8_t val = *b++;
                        write_32( addr, val, mask );
                        addr++;
                    }
                }
            } else {
                tx_buf[3] = FAIL;
            }
        } else {
            con.tx_buffer_size = HEADER_SIZE;
        }
        break;
    case TransactionTypeLUTRead:
        if ( con.rx_buffer_size == HEADER_SIZE + 8 ) {
            uint32_t addr = *rx_buf++;
            uint32_t size = *rx_buf++;
            if ( size <= CONNECTION_BUFFER_SIZE - HEADER_SIZE - 4 && !( addr & 3 ) ) {
                uint32_t *b = &tx_buf[4];
                con.tx_buffer_size = HEADER_SIZE + 4 + size;
                tx_buf[3] = SUCCESS;
                aisp_read_lut(addr, b, size / 4);
            } else {
                con.tx_buffer_size = HEADER_SIZE + 4;
                tx_buf[3] = FAIL;
            }
        } else {
            con.tx_buffer_size = HEADER_SIZE;
        }
        break;
    case TransactionTypeLUTWrite:
        if ( con.rx_buffer_size >= HEADER_SIZE + 8 ) {
            uint32_t addr = *rx_buf++;
            uint32_t size = *rx_buf++;
            con.tx_buffer_size = HEADER_SIZE + 4;
            if ( size <= con.rx_buffer_size - HEADER_SIZE - 8 && !( addr & 3 ) ) {
                tx_buf[3] = SUCCESS;
                aisp_write_lut(addr, rx_buf, size / 4);
            } else {
                tx_buf[3] = FAIL;
            }
        } else {
            con.tx_buffer_size = HEADER_SIZE;
        }
        break;
    case TransactionTypeAPIRead:
        if ( con.rx_buffer_size == HEADER_SIZE + 8 ) {
            uint8_t t = rx_buf[0] & 0xFF;
            uint8_t c = ( rx_buf[0] >> 8 ) & 0xFF;
            con.tx_buffer_size = HEADER_SIZE + 8;
            tx_buf[3] = application_command( camera_id, t, c, rx_buf[1], COMMAND_GET, &tx_buf[4] );
        } else {
            con.tx_buffer_size = HEADER_SIZE;
        }
        break;
    case TransactionTypeAPIWrite:
        if ( con.rx_buffer_size == HEADER_SIZE + 8 ) {
            uint8_t t = rx_buf[0] & 0xFF;
            uint8_t c = ( rx_buf[0] >> 8 ) & 0xFF;
            con.tx_buffer_size = HEADER_SIZE + 8;
            tx_buf[3] = application_command( camera_id, t, c, rx_buf[1], COMMAND_SET, &tx_buf[4] );
        } else {
            con.tx_buffer_size = HEADER_SIZE;
        }
        break;
    case TransactionTypeBUFRead:
        if ( con.rx_buffer_size == HEADER_SIZE + 8 ) {
            uint8_t id = rx_buf[0] & 0xFF;
            uint8_t buf_class = ( rx_buf[0] >> 8 ) & 0xFF;
            uint32_t size = rx_buf[1];
            uint32_t value;
            if ( size <= CONNECTION_BUFFER_SIZE - HEADER_SIZE - 4 ) {
                switch ( buf_class ) {
                case STATIC_CALIBRATIONS_ID:
                case DYNAMIC_CALIBRATIONS_ID:
                    con.tx_buffer_size = HEADER_SIZE + 4 + size;
                    tx_buf[3] = application_api_calibration( camera_id, buf_class, id, COMMAND_GET, &tx_buf[4], size, &value );
                    if ( tx_buf[3] != SUCCESS ) {
                        con.tx_buffer_size = HEADER_SIZE + 4;
                    }
                    break;

                default:
                    con.tx_buffer_size = HEADER_SIZE + 4;
                    tx_buf[3] = FAIL;
                }
            } else {
                con.tx_buffer_size = HEADER_SIZE + 4;
                tx_buf[3] = FAIL;
            }
        } else {
            con.tx_buffer_size = HEADER_SIZE;
        }
        break;
    case TransactionTypeBUFWrite:
        if ( con.rx_buffer_size >= HEADER_SIZE + 8 ) {
            uint8_t id = rx_buf[0] & 0xFF;
            uint8_t buf_class = ( rx_buf[0] >> 8 ) & 0xFF;
            uint32_t size = rx_buf[1];
            uint32_t value;
            con.tx_buffer_size = HEADER_SIZE + 4;
            if ( size <= con.rx_buffer_size - HEADER_SIZE - 8 ) {
                switch ( buf_class ) {
                case STATIC_CALIBRATIONS_ID:
                case DYNAMIC_CALIBRATIONS_ID:
                    tx_buf[3] = application_api_calibration( camera_id, buf_class, id, COMMAND_SET, &rx_buf[2], size, &value );
                    break;

                default:
                    tx_buf[3] = FAIL;
                }
            } else {
                tx_buf[3] = FAIL;
            }
        } else {
            con.tx_buffer_size = HEADER_SIZE;
        }
        break;
    default:
        con.tx_buffer_size = HEADER_SIZE;
    }
    *tx_buf = con.tx_buffer_size;
    con.tx_buffer_size = ( con.tx_buffer_size + ALIGNMENT_MASK ) & ~ALIGNMENT_MASK;
    con.tx_buffer_inx = 0;
    con.state = STATE_TX_PACKET;
}
#endif //ISP_HAS_STREAM_CONNECTION

void aisp_connection_process( void )
{
#if ISP_HAS_STREAM_CONNECTION
    int res = 0;
    int cnt = 20;
    uint32_t *const buf = (uint32_t *)con.buffer;

    if ( !con.data_read || !con.data_write ) {
        return;
    }

    do {
        switch ( con.state ) {
        case STATE_IDLE:
            res = con.data_read( con.param, &con.buffer[con.rx_buffer_inx], HEADER_SIZE - con.rx_buffer_inx );
            if ( res != 0 )
                AISP_CONNECTION_TRACE( "state %d res %d\n", con.state, res );
            if ( res < 0 ) {
                reset_connection();
                return;
            }
            con.rx_buffer_inx += res;
            if ( con.rx_buffer_inx >= HEADER_SIZE ) {
                con.rx_buffer_size = buf[0];
                if ( con.rx_buffer_size > CONNECTION_BUFFER_SIZE ) {
                    reset_connection();
                    con.state = STATE_SKIP_DATA;
                } else {
                    con.state = STATE_RX_DATA;
                }
            }
            break;
        case STATE_SKIP_DATA:
            res = con.data_read( con.param, &con.buffer[HEADER_SIZE], MIN( CONNECTION_BUFFER_SIZE - HEADER_SIZE, con.rx_buffer_size - con.rx_buffer_inx ) );
            if ( res != 0 )
                AISP_CONNECTION_TRACE( "state %d res %d\n", con.state, res );
            if ( res < 0 ) {
                reset_connection();
                return;
            }
            con.rx_buffer_inx += res;
            if ( con.rx_buffer_inx >= con.rx_buffer_size ) {
                con.tx_buffer_size = HEADER_SIZE;
                *buf = con.tx_buffer_size;
                con.tx_buffer_inx = 0;
                con.state = STATE_TX_PACKET;
            }
            break;
        case STATE_RX_DATA:
            res = con.data_read( con.param, &con.buffer[con.rx_buffer_inx], con.rx_buffer_size - con.rx_buffer_inx );
            if ( res != 0 )
                AISP_CONNECTION_TRACE( "state %d res %d\n", con.state, res );
            if ( res < 0 ) {
                reset_connection();
                return;
            }
            con.rx_buffer_inx += res;
            if ( con.rx_buffer_inx >= con.rx_buffer_size ) {
                process_request();
            }
            break;
        case STATE_TX_PACKET:
            res = con.data_write( con.param, &con.buffer[con.tx_buffer_inx], con.tx_buffer_size - con.tx_buffer_inx );
            if ( res != 0 )
                AISP_CONNECTION_TRACE( "state %d res %d\n", con.state, res );
            if ( res < 0 ) {
                reset_connection();
                return;
            }
            con.tx_buffer_inx += res;
            if ( con.tx_buffer_inx >= con.tx_buffer_size ) {
                AISP_CONNECTION_TRACE( "packet size %ld is transferred\n", con.tx_buffer_size );
                reset_connection();
                return; // this will make sure that FW itself will work as required
            }
            break;
        default:
            res = -1;
            reset_connection();
            return;
        }
    } while ( res > 0 && --cnt );
#endif //ISP_HAS_STREAM_CONNECTION
}

struct task_struct *isp_fw_connections_thread = NULL;

int connection_thread( void *foo )
{
    printk( "connection_thread start" );

    aisp_connection_init();

    ctrl_channel_init();
    //ctrl_channel1_init();
    //ctrl_channel2_init();
    //ctrl_channel3_init();

    while ( !kthread_should_stop() ) {
        aisp_connection_process();
    }

    aisp_connection_destroy();

    ctrl_channel_deinit();
    //ctrl_channel1_deinit();
    //ctrl_channel2_deinit();
    //ctrl_channel3_deinit();

    printk( "connection_thread stop" );

    return 0;
}

static int isp_power_on(int index, struct isp_info_t *isp_dev)
{
    int rtn = 0;
#if 0
    dev_pm_domain_attach(isp_dev->dev, true);
    pm_runtime_enable(isp_dev->dev);
    pm_runtime_get_sync(isp_dev->dev);
#endif

    switch (index) {
    case 0:
    case 1:
        clk_set_rate(isp_dev->isp_clk, 666666666);
        break;
    case 2:
    case 3:
        clk_set_rate(isp_dev->isp_clk, 200000000);
        break;
    default:
        dev_err(isp_dev->dev, "Error to set ISP-%d clk rate\n", index);
        break;
    }

    rtn = clk_prepare_enable(isp_dev->isp_clk);
    if (rtn)
        dev_err(isp_dev->dev, "Error to enable isp_clk\n");

    return rtn;
}

static void isp_power_off(struct isp_info_t *isp_dev)
{
    clk_disable_unprepare(isp_dev->isp_clk);

#if 0
    pm_runtime_put_sync(isp_dev->dev);
    pm_runtime_disable(isp_dev->dev);
    dev_pm_domain_detach(isp_dev->dev, true);
#endif
}

void __iomem * get_pipeline_info(int index, char * name)
{
    struct device_node * node;
    struct platform_device *pdev;
    struct clk *isp_clk;
    struct device *dev;
    struct resource *res = NULL;
    void __iomem *base = NULL;

    node = of_find_node_by_name(NULL, name);
    pdev = of_find_device_by_node(node);
    dev = &pdev->dev;

    dev_info[index].dev = dev;
#if 1
    isp_clk = devm_clk_get(dev, "mipi_isp_clk");
    if (IS_ERR(isp_clk)) {
        dev_err(dev, "Error to get isp_clk\n");
        return NULL;
    }
    dev_info[index].isp_clk = isp_clk;
#endif
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        dev_err(dev, "Error to get mem\n");
        //return NULL;
    }

    base = ioremap(0xfe3b4000, 0x10000);
    if (!base) {
        dev_err(dev, "Error to ioremap mem\n");
        return NULL;
    }

    //pr_err("pipeline start addr:%x", (uint32_t)res->start);

    return base;
}

static int __init aml_init_connect(void) {
    int i = 0;
    char name[5];

    for (i = 0; i < 1; i ++) {
        sprintf(name, "isp%d", i);
        dev_info[i].base = get_pipeline_info(i, name);
        isp_power_on(i, dev_info + i);
    }

    if ( dev_info[camera_id].base )
        p_hw_base = dev_info[camera_id].base;
    else
        return -1;

    isp_fw_connections_thread = kthread_run( connection_thread,
                                             NULL, "isp_connection" );
    return 0;
}

static void __exit aml_deinit_connect(void) {
    int i = 0;
    for ( i = 0; i < 1; i++ ) {
        iounmap(dev_info[i].base);
        isp_power_off(dev_info + i);
    }

    if ( isp_fw_connections_thread )
        kthread_stop( isp_fw_connections_thread );

    if (p_hw_base)
        p_hw_base = NULL;
}

module_init(aml_init_connect);
module_exit(aml_deinit_connect);

MODULE_AUTHOR("amlogic inc");
MODULE_DESCRIPTION("Amlogic Tunning Driver");
MODULE_LICENSE("Dual BSD/GPL");


/*
 * isp.c
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/cacheflush.h>

#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/omap-iommu.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/of_reserved_mem.h>

#include <media/v4l2-common.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mc.h>
#include "system_chardev.h"

int aisp_chardev_init( void )
{
    return system_chardev_init();
}

int aisp_chardev_read( void *unused, uint8_t *data, int size )
{
    return system_chardev_read( (char *)data, size );
}

int aisp_chardev_write( void *unused, const uint8_t *data, int size )
{
    return system_chardev_write( (char *)data, size );
}

int aisp_chardev_destroy( void )
{
    return system_chardev_destroy();
}


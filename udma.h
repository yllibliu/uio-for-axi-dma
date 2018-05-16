/*
 * udma module -- Simple zero-copy DMA to/from userspace for
 * dmaengine-compatible hardware.
 * 
 * Copyright (C) 2015 Jeremy Trimble
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/param.h>  /* HZ */
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/mm.h>

#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/wait.h>

#define UDMA_DEV_NAME_MAX_CHARS (16)

// Assume that reads/writes have to be multiples of this.
#define UDMA_ALIGN_BYTES (1)

#define SEM_TAKE_TIMEOUT (5)

enum udma_dir {
    UDMA_DEV_TO_CPU = 1,   // RX
    UDMA_CPU_TO_DEV = 2,   // TX
};

/* Right now the I/O concept is very simple -- all reads and writes
 * are blocking, and concurrent reads and writes are not allowed.
 * Concurrent open is also disallowed.
 */
enum dma_fsm_state {
    DMA_IDLE = 0,
    DMA_IN_FLIGHT = 1,
    DMA_COMPLETING = 3,
};

// These fields should only be valid during an ongoing read/write call.
struct udma_inflight_info {
    struct page **  pinned_pages;
    struct sg_table table;
    unsigned int    num_pages;
    bool            table_allocated;
    bool            pages_pinned;
    bool            dma_mapped;
    bool            dma_started;
};

struct udma_drvdata {
    struct platform_device *pdev;

    char name[udma_DEV_NAME_MAX_CHARS];
    uint32_t dir;   // udma_dir

    struct semaphore sem;   /* protects mutable data below */

    bool        in_use;
    atomic_t    accepting;

    spinlock_t state_lock;  // protects state below, may be taken from interrupt (tasklet) context
    enum dma_fsm_state state;
    struct udma_inflight_info inflight;

    wait_queue_head_t    wq;

    /* dmaengine */
    struct dma_chan *chan;

    /* device accounting */
    dev_t           udma_devt;
    struct cdev     udma_cdev;
    struct device * udma_dev;

    /* Statistics */
    atomic_t    packets_sent;
    atomic_t    packets_rcvd;

    struct list_head node;
    bool init_done;
};

/* LOCK ORDERING:  if taking both sem and state_lock, must always take sem first */

struct udma_pdev_drvdata {
    struct list_head udma_list;    // list of udma_drvdata instances created in
                                    // relation to this platform device
};


#define NUM_DEVICE_NUMBERS_TO_ALLOCATE (8)
static dev_t base_devno;
static int devno_in_use[NUM_DEVICE_NUMBERS_TO_ALLOCATE];
static struct class *udma_class;
static DEFINE_SEMAPHORE(devno_lock);

static inline int check_udma(struct platform_device *pdev);
static ssize_t udma_read(struct file *filp, char __user *userbuf, size_t count, loff_t *f_pos)
static ssize_t udma_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *f_pos)
static void teardown_udma( struct platform_device *pdev)



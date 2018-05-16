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

#include "udma.h"


static struct udma_drvdata *udma_rx_drvdata, *udma_tx_drvdata;


static inline int udma_init(struct platform_device *pdev)
{
	//udma_tx_drvdata = devm_kzalloc( &pdev->dev, sizeof(*udma_tx_drvdata), GFP_KERNEL );
	//udma_rx_drvdata = devm_kzalloc( &pdev->dev, sizeof(*udma_tx_drvdata), GFP_KERNEL );
	
	// tx channel init
	udma_tx_drvdata->pdev = pdev;
	udma_tx_drvdata->in_use = 0;
	udma_tx_drvdata->state = DMA_IDLE;
    spin_lock_init( &udma_tx_drvdata->state_lock );
    //list_add_tail( &udma_tx_drvdata->node, &p_pdev_info->udma_list );   dont know wut r doing
    sema_init( &udma_tx_drvdata->sem, 1 );
    init_waitqueue_head( &udma_tx_drvdata->wq );
    atomic_set( &udma_tx_drvdata->packets_sent, 0 );
    atomic_set( &udma_tx_drvdata->packets_rcvd, 0 );
    udma_tx_drvdata->name = "loop_tx"
    udma_tx_drvdata->dir = 2;	
    udma_tx_drvdata->chan = dma_request_slave_channel(&pdev->dev, udma_tx_drvdata->name);

	if ( !udma_tx_drvdata->chan )
	{
		printk( KERN_WARNING KBUILD_MODNAME 
		": couldn't find dma channel: %s, deferring...\n",
		udma_tx_drvdata->name);

		outer_rv = -EPROBE_DEFER;
	}

	udma_tx_drvdata->init_done = true;
	printk( KERN_ALERT KBUILD_MODNAME ": %s (%s) available\n", 
							udma_tx_drvdata->name,
							udma_tx_drvdata->dir == UDMA_DEV_TO_CPU ? "RX" : "TX");

    
    // rx channel init
    udma_rx_drvdata->pdev = pdev;
	udma_rx_drvdata->in_use = 0;
	udma_rx_drvdata->state = DMA_IDLE;
    spin_lock_init( &udma_rx_drvdata->state_lock );
    //list_add_tail( &udma_rx_drvdata->node, &p_pdev_info->udma_list );   dont know wut r doing
    sema_init( &udma_rx_drvdata->sem, 1 );
    init_waitqueue_head( &udma_rx_drvdata->wq );
    atomic_set( &udma_rx_drvdata->packets_sent, 0 );
    atomic_set( &udma_rx_drvdata->packets_rcvd, 0 );
    udma_rx_drvdata->name = "loop_rx";	
    udma_rx_drvdata->dir = 1;

    udma_rx_drvdata->chan = dma_request_slave_channel(&pdev->dev, "loop_rx");

	if ( !p_info->chan )
	{
		printk( KERN_WARNING KBUILD_MODNAME 
		": couldn't find dma channel: %s, deferring...\n",
		udma_rx_drvdata->name);

		outer_rv = -EPROBE_DEFER;
	}
	udma_rx_drvdata->init_done = true;

	printk( KERN_ALERT KBUILD_MODNAME ": %s (%s) available\n", 
							udma_rx_drvdata->name,
							udma_rx_drvdata->dir == UDMA_DEV_TO_CPU ? "RX" : "TX");


	return 2;


	/*
	int num_dma_names = of_property_count_strings(pdev->dev.of_node, "dma-names"); // should be > 0

	for (dma_name_idx = 0; dma_name_idx < num_dma_names; dma_name_idx++)
    {
    	struct udma_drvdata * p_info;
    	const char * p_dma_name;
    	int rv;

    	p_info = devm_kzalloc( &pdev->dev, sizeof(*p_info), GFP_KERNEL );
    	
    	if ( !p_info )
    	{
        	printk( KERN_ERR KBUILD_MODNAME ": failed to allocate udma_drvdata\n");
        	outer_rv = -ENOMEM;
        	break;
    	}  

    	p_info->pdev = pdev;
    	p_info->in_use = 0;
    	p_info->state = DMA_IDLE;
        spin_lock_init( &p_info->state_lock );
        list_add_tail( &p_info->node, &p_pdev_info->udma_list );
        sema_init( &p_info->sem, 1 );
        init_waitqueue_head( &p_info->wq );
        atomic_set( &p_info->packets_sent, 0 );
        atomic_set( &p_info->packets_rcvd, 0 );
	*/
        /* Read the dma name for the current index */
    /*
        rv = of_property_read_string_index(
                pdev->dev.of_node, "dma-names",
                dma_name_idx, &p_dma_name);

        if ( rv )
        {
            printk( KERN_ERR KBUILD_MODNAME
                    ": of_property_read_string_index() returned %d\n", rv);

            outer_rv = rv;
            break;
        }
        else
        {
            strncpy( p_info->name, p_dma_name, udma_DEV_NAME_MAX_CHARS-1 );
            p_info->name[udma_DEV_NAME_MAX_CHARS-1] = '\0';

            //printk( KERN_DEBUG KBUILD_MODNAME ": setting up %s\n", p_info->name);
        }
	*/

        /* Read the direction for the current index */
    /*
        rv = of_property_read_u32_index(
                pdev->dev.of_node, "udma,dirs",
                dma_name_idx, &p_info->dir);

        if ( rv )
        {
            printk( KERN_ERR KBUILD_MODNAME
                    ": couldn't read \"udma,dirs\" property for %s\n",
                    p_info->name );

            outer_rv = rv;
            break;
        }
        else if ( p_info->dir != udma_CPU_TO_DEV && 
                  p_info->dir != udma_DEV_TO_CPU )
        {
            printk( KERN_ERR KBUILD_MODNAME
                    ": %s specifies unsupported value of \"udma,dirs\": %d\n", 
                    p_info->name,
                    p_info->dir);

            outer_rv = -EINVAL;
            break;
        }
    */      
        /* Get the named DMA channel */
	/*
    	p_info->chan = dma_request_slave_channel(&pdev->dev, p_dma_name);

    	if ( !p_info->chan )
   	 	{
        	printk( KERN_WARNING KBUILD_MODNAME 
            	    ": couldn't find dma channel: %s, deferring...\n",
                	p_info->name);

        	outer_rv = -EPROBE_DEFER;
    	}

    	printk( KERN_ALERT KBUILD_MODNAME ": %s (%s) available\n", 
            	p_info->name,
            	p_info->dir == udma_DEV_TO_CPU ? "RX" : "TX"
            	);
	}
	*/


	//if ( outer_rv )
	//{
    		// Unroll what we've done here
    		//teardown_devices( p_pdev_info, pdev );
	//}

    //return outer_rv;    	
}



static inline int check_udma(struct platform_device *pdev)
{
	int num_dma_names = of_property_count_strings(pdev->dev.of_node, "dma-names");

    if ( 0 == num_dma_names )  // no udma
    {
        printk( KERN_ERR KBUILD_MODNAME ": no DMAs specified in udma \"dma-names\" property\n");
        return num_dma_names;
    }
    else if ( num_dma_names < 0 )  // udma information error
    {
        printk( KERN_ERR KBUILD_MODNAME ": got %d when trying to count the elements of \"dma-names\" property\n", num_dma_names);
        return num_dma_names;   // contains error code
    }


    return udma_init(pdev);
}

static void udma_unprepare_after_dma( struct udma_drvdata * p_info );


static void udma_dmaengine_callback_func(void *data)
{
    struct udma_drvdata * p_info = (struct udma_drvdata*)data;
    unsigned long iflags;

    //printk( KERN_ERR KBUILD_MODNAME ": %s: callback fired for %s\n",
    //        p_info->name, p_info->dir == EZDMA_DEV_TO_CPU ? "RX" : "TX" );

    spin_lock_irqsave(&p_info->state_lock, iflags);

    if ( DMA_IN_FLIGHT == p_info->state )
    {
        p_info->state = DMA_COMPLETING;
        wake_up_interruptible( &p_info->wq );
    }
    // else: well, nevermind then...
    
    spin_unlock_irqrestore(&p_info->state_lock, iflags);
}



static int udma_prepare_for_dma(
        struct udma_drvdata * p_info, 
        char __user *userbuf,
        size_t count
)
{
    int rv;

    BUG_ON( p_info->inflight.pinned_pages ); // should be NULL
    memset( &p_info->inflight, 0, sizeof( struct udma_inflight_info ) );
    
    p_info->inflight.num_pages = (offset_in_page(userbuf) + count + PAGE_SIZE-1) / PAGE_SIZE;
    p_info->inflight.pinned_pages = kmalloc( 
        p_info->inflight.num_pages * sizeof(struct page*),
        GFP_KERNEL);

    if ( !p_info->inflight.pinned_pages )
    {
        rv = -ENOMEM;
        goto err_out;
    }

    if ( (rv = sg_alloc_table(
                    &p_info->inflight.table, 
                    p_info->inflight.num_pages,
                    GFP_KERNEL )) )
    {
        printk( KERN_ERR KBUILD_MODNAME ": %s: sg_alloc_table() returned %d\n", 
                p_info->name, rv);
        goto err_out;
    }
    else
    {
        p_info->inflight.table_allocated = 1;
    }

    rv = get_user_pages_fast(
            (unsigned long)userbuf,             // start
            p_info->inflight.num_pages,
            p_info->dir == UDMA_DEV_TO_CPU,    // write
            p_info->inflight.pinned_pages);

    if ( rv != p_info->inflight.num_pages )
    {
        printk( KERN_ERR KBUILD_MODNAME ": %s: get_user_pages_fast() returned %d, expected %d\n",
                p_info->name, rv, p_info->inflight.num_pages);
        goto err_out;
    }
    else
    {
        p_info->inflight.pages_pinned = 1;
    }

    // Build scatterlist.
    {
        int i;
        struct scatterlist * sg;
        struct scatterlist * const sgl = p_info->inflight.table.sgl;
        const unsigned int num_pages = p_info->inflight.num_pages;

        size_t left_to_map = count;

        for_each_sg( sgl, sg, num_pages, i )
        {
            unsigned int len;
            unsigned int offset;

            len = left_to_map > PAGE_SIZE ? PAGE_SIZE : left_to_map;

            if ( 0 == i )
            {
                offset = offset_in_page(userbuf);
                if ( (offset + len) > PAGE_SIZE )
                    len = PAGE_SIZE - offset;
            }
            else
            {
                offset = 0;
            }

            //printk( KERN_DEBUG KBUILD_MODNAME ": %s: sgl[%d]: page: %p, len: %d, offset: %d\n",
            //        p_info->name, i, p_info->inflight.pinned_pages[i], len, offset );

            //sg_set_page( sgl, p_info->inflight.pinned_pages[i], len, offset );
            sg_set_page( sg, p_info->inflight.pinned_pages[i], len, offset );
            left_to_map -= len;
        }
    }

    // Map the scatterlist 

    rv = dma_map_sg(&p_info->pdev->dev,
                p_info->inflight.table.sgl,
                p_info->inflight.num_pages,
                p_info->dir == UDMA_DEV_TO_CPU ? DMA_FROM_DEVICE : DMA_TO_DEVICE);

    if ( rv != p_info->inflight.num_pages )
    {
        printk( KERN_ERR KBUILD_MODNAME ": %s: dma_map_sg() returned %d, expected %d\n", 
                p_info->name, rv, p_info->inflight.num_pages);
        goto err_out;
    }
    else
    {
        p_info->inflight.dma_mapped = 1;
    }

    // Issue DMA request here
    {
        struct dma_async_tx_descriptor * txn_desc;
        struct scatterlist * const sgl = p_info->inflight.table.sgl;
        dma_cookie_t cookie;

        txn_desc = dmaengine_prep_slave_sg(
                p_info->chan,
                sgl,
                p_info->inflight.num_pages,
                p_info->dir == UDMA_DEV_TO_CPU ? DMA_FROM_DEVICE : DMA_TO_DEVICE,
                DMA_PREP_INTERRUPT);    // run callback after this one

        if ( !txn_desc )
        {
            printk( KERN_ERR KBUILD_MODNAME ": %s: dmaengine_prep_slave_sg() failed\n", p_info->name);
            rv = -ENOMEM;
            goto err_out;
        }

        txn_desc->callback = udma_dmaengine_callback_func;
        txn_desc->callback_param = p_info;

        spin_lock_irq( &p_info->state_lock );

        p_info->state = DMA_IN_FLIGHT;

        cookie = dmaengine_submit(txn_desc);

        if ( cookie < DMA_MIN_COOKIE )
        {
            printk( KERN_ERR KBUILD_MODNAME ": %s: dmaengine_submit() returned %d\n", p_info->name, cookie);
            rv = cookie;
            p_info->state = DMA_IDLE;
        }
        else
        {
            p_info->inflight.dma_started = 1;
            dma_async_issue_pending( p_info->chan );    // Bam!
            //printk( KERN_ERR KBUILD_MODNAME ": %s: issued pending for %s\n",
            //        p_info->name,
            //        p_info->dir == EZDMA_DEV_TO_CPU ? "RX" : "TX" );
        }

        spin_unlock_irq( &p_info->state_lock );

        if ( cookie < DMA_MIN_COOKIE )
            goto err_out;
    }

    return 0;

    err_out:

    spin_lock_irq( &p_info->state_lock );
    udma_unprepare_after_dma( p_info );
    spin_unlock_irq( &p_info->state_lock );

    return rv;
}

// should be called with p_info->sem held, and with p_info_state_lock
static void udma_unprepare_after_dma( struct udma_drvdata * p_info )
{
    p_info->state = DMA_IDLE;

    if ( p_info->inflight.dma_mapped )
    {
        dma_unmap_sg(p_info->udma_dev,
                p_info->inflight.table.sgl,
                p_info->inflight.num_pages,
                p_info->dir == UDMA_DEV_TO_CPU ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
    }
    p_info->inflight.dma_mapped = 0;

    if ( p_info->inflight.pages_pinned )
    {
        if ( p_info->inflight.dma_started && p_info->dir == UDMA_DEV_TO_CPU )
        {
            /* Mark all pages dirty for now (not sure how to do this more
             * efficiently yet -- dmaengine API doesn't seem to return any
             * notion of how much data was actually transferred).
             */

            int i;

            for (i = 0; i < p_info->inflight.num_pages; ++i)
            {
                struct page * const page = p_info->inflight.pinned_pages[i];

                set_page_dirty( page );
                put_page( page );
            }
        }
    }
    p_info->inflight.pages_pinned = 0;

    if ( p_info->inflight.table_allocated )
        sg_free_table( &p_info->inflight.table );
    p_info->inflight.table_allocated = 0;

    if ( p_info->inflight.pinned_pages )
    {
        kfree(p_info->inflight.pinned_pages);
        p_info->inflight.pinned_pages = NULL;
    }

}

static int check_not_in_flight( struct udma_drvdata * p_info )
{
    int rv;
    spin_lock_irq(&p_info->state_lock);
    
    rv = (p_info->state != DMA_IN_FLIGHT);
    
    spin_unlock_irq(&p_info->state_lock);

    return rv;
}


// 
static ssize_t udma_read(struct file *filp, char __user *userbuf, size_t count, loff_t *f_pos)
{
    if ( 0 != (count % UDMA_ALIGN_BYTES) )
    {
        printk( KERN_WARNING KBUILD_MODNAME ": %s: unaligned read of %u bytes requested\n", udma_rx_drvdata->name, count);
        return -EINVAL;
    }
    if ( down_interruptible( &udma_rx_drvdata->sem ) )
        return -ERESTARTSYS;

    if ( !atomic_read(&udma_rx_drvdata->accepting ) )
    {
        rv = -EBADF;
    } 
    else
    {
        int prep_rv;
        int wait_rv;

        prep_rv = udma_prepare_for_dma( udma_rx_drvdata , userbuf, count );

        if (prep_rv)
        {
            rv = prep_rv;
            goto out;
        }

        up( &udma_rx_drvdata->sem );

        wait_rv = wait_event_interruptible( udma_rx_drvdata->wq, check_not_in_flight(udma_rx_drvdata) );

        if ( down_timeout( &udma_rx_drvdata->sem, SEM_TAKE_TIMEOUT ) )
        {
            printk( KERN_ALERT KBUILD_MODNAME 
                    ": %s: read sem take stalled for %d seconds -- probably broken\n",
                    udma_rx_drvdata->name, 
                    SEM_TAKE_TIMEOUT);
            goto noup_out;
        }

        spin_lock_irq(&udma_rx_drvdata->state_lock);
        if ( udma_rx_drvdata->state == DMA_IN_FLIGHT && -ERESTARTSYS == wait_rv )
        {
            dmaengine_terminate_all( udma_rx_drvdata->chan );
            rv = wait_rv;
        }

        udma_unprepare_after_dma( udma_rx_drvdata );    // sets us back to DMA_IDLE
        spin_unlock_irq(&udma_rx_drvdata->state_lock);
    }

    out:
    up( &udma_rx_drvdata->sem );

    noup_out:
    return rv;   
    /*
    struct udma_drvdata * p_info = (struct ezdma_drvdata*)filp->private_data;
    ssize_t rv = count;

    // Ensure this is a readable device.
    if ( UDMA_DEV_TO_CPU != p_info->dir )
    {
        printk( KERN_WARNING KBUILD_MODNAME ": %s: can't read, is a TX device\n", p_info->name);
        return -EINVAL;
    }

    if ( 0 != (count % EZDMA_ALIGN_BYTES) )
    {
        printk( KERN_WARNING KBUILD_MODNAME ": %s: unaligned read of %u bytes requested\n", p_info->name, count);
        return -EINVAL;
    }

    //TODO: verify size of count?

    if ( down_interruptible( &p_info->sem ) )
        return -ERESTARTSYS;

    if ( !atomic_read(&p_info->accepting ) )
    {
        rv = -EBADF;
        goto out;
    }
    else
    {
        int prep_rv;
        int wait_rv;

        prep_rv = ezdma_prepare_for_dma( p_info, userbuf, count );

        if (prep_rv)
        {
            rv = prep_rv;
            goto out;
        }

        up( &p_info->sem );

        wait_rv = wait_event_interruptible( p_info->wq, check_not_in_flight(p_info) );

        if ( down_timeout( &p_info->sem, SEM_TAKE_TIMEOUT ) )
        {
            printk( KERN_ALERT KBUILD_MODNAME 
                    ": %s: read sem take stalled for %d seconds -- probably broken\n",
                    p_info->name, 
                    SEM_TAKE_TIMEOUT);
            goto noup_out;
        }

        spin_lock_irq(&p_info->state_lock);
        if ( p_info->state == DMA_IN_FLIGHT && -ERESTARTSYS == wait_rv )
        {
            dmaengine_terminate_all( p_info->chan );
            rv = wait_rv;
        }

        ezdma_unprepare_after_dma( p_info );    // sets us back to DMA_IDLE
        spin_unlock_irq(&p_info->state_lock);
    }

    out:
    up( &p_info->sem );

    noup_out:
    return rv;
    */
}

static ssize_t udma_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *f_pos)
{
    
    //struct udma_drvdata * p_info = (struct ezdma_drvdata*)filp->private_data;
    ssize_t rv = count;

    // Ensure this is a writable device.
    //if ( EZDMA_CPU_TO_DEV != p_info->dir )
    //{
    //    printk( KERN_WARNING KBUILD_MODNAME ": %s: can't write, is an RX device\n", p_info->name);
    //    return -EINVAL;
    //}
    if ( 0 != (count % UDMA_ALIGN_BYTES) )
    {
        printk( KERN_WARNING KBUILD_MODNAME ": %s: unaligned write of %u bytes requested\n", udma_tx_drvdata->name, count);
        return -EINVAL;
    }

    // Ensure this is a writable device.

    if ( down_interruptible( &udma_tx_drvdata->sem ) )
        return -ERESTARTSYS;

    if ( !atomic_read(&udma_tx_drvdata->accepting ) )
    {
        rv = -EBADF;
        goto out;
    }
    else
    {
        int prep_rv;
        int wait_rv;

        prep_rv = udma_prepare_for_dma( udma_tx_drvdata, (char __user*)userbuf, count );

        if (prep_rv)
        {
            rv = prep_rv;
            goto out;
        }

        up( &udma_tx_drvdata->sem );

        wait_rv = wait_event_interruptible( udma_tx_drvdata->wq, check_not_in_flight(udma_tx_drvdata) );

        if ( down_timeout( &udma_tx_drvdata->sem, SEM_TAKE_TIMEOUT ) )
        {
            printk( KERN_ALERT KBUILD_MODNAME 
                    ": %s: write sem take stalled for %d seconds -- probably broken\n",
                    udma_tx_drvdata->name,
                    SEM_TAKE_TIMEOUT);
            goto noup_out;
        }

        spin_lock_irq(&udma_tx_drvdata->state_lock);
        if ( udma_tx_drvdata->state == DMA_IN_FLIGHT && -ERESTARTSYS == wait_rv )
        {
            dmaengine_terminate_all( udma_tx_drvdata->chan );
            rv = wait_rv;
        }

        udma_unprepare_after_dma( udma_tx_drvdata );    // sets us back to DMA_IDLE
        spin_unlock_irq(&udma_tx_drvdata->state_lock);
    }

    out:
    up( &udma_tx_drvdata->sem );

    noup_out:
    return rv;
    
}

//static void teardown_devices( struct udma_pdev_drvdata * p_pdev_info, struct platform_device *pdev)
static void teardown_udma( struct platform_device *pdev)
{
	if (udma_tx_drvdata->init_done){
		// teardown udma_tx_drvdata channel
	    printk( KERN_DEBUG KBUILD_MODNAME ": tearing down %s\n",
	                udma_tx_drvdata->name );    // name can only be all null-bytes or a valid string

	    if ( udma_tx_drvdata->chan )
	    {
	        dmaengine_terminate_all(udma_tx_drvdata->chan);
	        dma_release_channel(udma_tx_drvdata->chan);
	    }
	    udma_tx_drvdata->init_done = false;
	}

	if (udma_rx_drvdata->init_done) {
		// teardown udma_rx_drvdata channel
    	printk( KERN_DEBUG KBUILD_MODNAME ": tearing down %s\n",
                udma_rx_drvdata->name );    // name can only be all null-bytes or a valid string

    	if ( udma_rx_drvdata->chan )
    	{
        	dmaengine_terminate_all(udma_rx_drvdata->chan);
        	dma_release_channel(udma_rx_drvdata->chan);
    	}  
    	udma_rx_drvdata->init_done = false;
	}

  

}


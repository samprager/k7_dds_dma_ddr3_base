/*******************************************************************************
** © Copyright 2011 - 2012 Xilinx, Inc. All rights reserved.
** This file contains confidential and proprietary information of Xilinx, Inc. and
** is protected under U.S. and international copyright and other intellectual property laws.
*******************************************************************************
**   ____  ____
**  /   /\/   /
** /___/  \  /   Vendor: Xilinx
** \   \   \/
**  \   \
**  /   /
** \   \  /  \   Kintex-7 PCIe-DMA-DDR3 Base Targeted Reference Design
**  \___\/\___\
**
**  Device: xc7k325t
**  Reference: UG882
*******************************************************************************
**
**  Disclaimer:
**
**    This disclaimer is not a license and does not grant any rights to the materials
**    distributed herewith. Except as otherwise provided in a valid license issued to you
**    by Xilinx, and to the maximum extent permitted by applicable law:
**    (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS,
**    AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
**    INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
**    FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether in contract
**    or tort, including negligence, or under any other theory of liability) for any loss or damage
**    of any kind or nature related to, arising under or in connection with these materials,
**    including for any direct, or any indirect, special, incidental, or consequential loss
**    or damage (including loss of data, profits, goodwill, or any type of loss or damage suffered
**    as a result of any action brought by a third party) even if such damage or loss was
**    reasonably foreseeable or Xilinx had been advised of the possibility of the same.


**  Critical Applications:
**
**    Xilinx products are not designed or intended to be fail-safe, or for use in any application
**    requiring fail-safe performance, such as life-support or safety devices or systems,
**    Class III medical devices, nuclear facilities, applications related to the deployment of airbags,
**    or any other applications that could lead to death, personal injury, or severe property or
**    environmental damage (individually and collectively, "Critical Applications"). Customer assumes
**    the sole risk and liability of any use of Xilinx products in Critical Applications, subject only
**    to applicable laws and regulations governing limitations on product liability.

**  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT ALL TIMES.

*******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xdma_base.c
 *
 * This is the Linux base driver for the DMA engine core. It provides
 * multi-channel DMA capability with the help of the Northwest Logic
 * DMA engine.
 *
 * Author: Xilinx, Inc.
 *
 * 2007-2010 (c) Xilinx, Inc. This file is licensed uner the terms of the GNU
 * General Public License version 2.1. This program is licensed "as is" without
 * any warranty of any kind, whether express or implied.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Date     Changes
 * ----- -------- -------------------------------------------------------
 * 1.0   12/07/09 First release
 * 1.2   09/01/10 Updates to read version register and convey to GUI.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/version.h>

#include <linux/delay.h>

#include <xpmon_be.h>
#include "xdebug.h"
#include "xbasic_types.h"
#include "xstatus.h"
#include "xdma.h"
#include "xdma_hw.h"
#include "xdma_bdring.h"
#include "xdma_user.h"


/************************** Constant Definitions *****************************/

/** @name Macros for PCI probing
 * @{
 */
#define PCI_VENDOR_ID_DMA   0x10EE      /**< Vendor ID - Xilinx */

#define PCI_DEVICE_ID_DMA   0x7042      /**< Xilinx's Device ID */

/** Driver information */
#define DRIVER_NAME         "xdma_driver"
#define DRIVER_DESCRIPTION  "Xilinx DMA Linux driver"
#define DRIVER_VERSION      "1.0"

/** Driver Module information */
MODULE_AUTHOR("Xilinx, Inc.");
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

/** PCI device structure which probes for targeted design */
static struct pci_device_id ids[] = {
        { PCI_VENDOR_ID_DMA,    PCI_DEVICE_ID_DMA,
          PCI_ANY_ID,               PCI_ANY_ID,
          0,            0,          0UL },
          { }     /* terminate list with empty entry */
};

/**
 * Macro to export pci_device_id to user space to allow hot plug and
 * module loading system to know what module works with which hardware device
 */
MODULE_DEVICE_TABLE(pci, ids);

/*@}*/

/** Engine bitmask is 64-bit because there are 64 engines */
#define DMA_ENGINE_PER_SIZE     0x100   /**< Separation between engine regs */
#define DMA_OFFSET              0       /**< Starting register offset */
                                        /**< Size of DMA engine reg space */
#define DMA_SIZE                (MAX_DMA_ENGINES * DMA_ENGINE_PER_SIZE)

/**
 * Default S2C and C2S descriptor ring size.
 * BD Space needed is (DMA_BD_CNT*sizeof(Dma_Bd)).
 */

#define DMA_BD_CNT 3999

/* Size of packet pool */
#define MAX_POOL    10

/* Structures to store statistics - the latest 100 */
#define MAX_STATS   100

/************************** Variable Names ***********************************/
/** Pool of packet arrays to use while processing packets */
struct PktPool
{
    PktBuf * pbuf;
    struct PktPool * next;
};

PktBuf pktArray[MAX_POOL][DMA_BD_CNT]; // used for passing pkts between drivers.
struct PktPool pktPool[MAX_POOL];
struct PktPool * pktPoolHead=NULL;
struct PktPool * pktPoolTail=NULL;

struct timer_list stats_timer;
struct timer_list poll_timer;

struct cdev * xdmaCdev=NULL;

/** DMA driver state-related variables */
struct privData * dmaData = NULL;
u32 DriverState = UNINITIALIZED;

/* for exclusion of all program flows (processes, ISRs and BHs) */
static DEFINE_SPINLOCK(DmaStatsLock);
DEFINE_SPINLOCK(DmaLock);
static DEFINE_SPINLOCK(IntrLock);
static DEFINE_SPINLOCK(PktPoolLock);

/* Statistics-related variables */
int UserOpen=0;
DMAStatistics DStats[MAX_DMA_ENGINES][MAX_STATS];
SWStatistics SStats[MAX_DMA_ENGINES][MAX_STATS];
TRNStatistics TStats[MAX_STATS];
int dstatsRead[MAX_DMA_ENGINES], dstatsWrite[MAX_DMA_ENGINES];
int dstatsNum[MAX_DMA_ENGINES], sstatsRead[MAX_DMA_ENGINES];
int sstatsWrite[MAX_DMA_ENGINES], sstatsNum[MAX_DMA_ENGINES];
int tstatsRead, tstatsWrite, tstatsNum;
u32 SWrate[MAX_DMA_ENGINES];


/************************** Function Prototypes ******************************/
static int __devinit xdma_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void __devexit  xdma_remove(struct pci_dev *pdev);
static int xdma_dev_open(struct inode * in, struct file * filp);
static int xdma_dev_release(struct inode * in, struct file * filp);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int xdma_dev_ioctl(struct inode * in, struct file * filp,
                          unsigned int cmd, unsigned long arg);
#else
static long xdma_dev_ioctl(struct file * filp,
                          unsigned int cmd, unsigned long arg);
#endif
static int ReadPCIState(struct pci_dev * pdev, PCIState * pcistate);

#ifdef DEBUG_VERBOSE
void disp_bd_ring(Dma_BdRing *);
#endif

static void PutUnusedPkts(Dma_Engine * eptr, PktBuf * pbuf, int numpkts);
static void DmaSetupRecvBuffers(struct pci_dev *, Dma_Engine *);

#ifdef DEBUG_VERBOSE
void disp_frag(unsigned char *, u32);
//static void ReadRoot(struct pci_dev *);
#endif

#if defined(DEBUG_VERBOSE) || defined(DEBUG_NORMAL)
static void ReadConfig(struct pci_dev *);
#endif

/** xdma Driver information */
static struct pci_driver xdma_driver = {
    .name = DRIVER_NAME,
    .id_table = ids,
    .probe = xdma_probe,
    .remove = __devexit_p(xdma_remove)
};

/* The callback function for completed frames sent in SGDMA mode.
 * In the interrupt-mode, these functions are scheduled as bottom-halves.
 * In the polled-mode, these functions are invoked as functions.
 */
static void PktHandler(int eng, Dma_Engine * eptr);

static void poll_routine(unsigned long __opaque);

#ifdef TH_BH_ISR
unsigned long long PendingMask = 0x0LL;
int LastIntr[MAX_DMA_ENGINES]={ 0, };
int MSIEnabled=0;
static void IntrBH(unsigned long unused);
DECLARE_TASKLET(DmaBH, IntrBH, 0);
#endif

static void ReadDMAEngineConfiguration(struct pci_dev *, struct privData *);
static void poll_stats(unsigned long __opaque);

/* Functions to enqueue and dequeue packet arrays in packet pools */
struct PktPool * DQPool(void)
{
    struct PktPool * ppool;
    unsigned long flags;

    spin_lock_irqsave(&PktPoolLock, flags);
    ppool = pktPoolHead;
    pktPoolHead = ppool->next;
    if(pktPoolHead == NULL)
        printk(KERN_ERR "pktPoolHead is NULL. This should never happen\n");
    spin_unlock_irqrestore(&PktPoolLock, flags);

    return ppool;
}

void EQPool(struct PktPool * pp)
{
    unsigned long flags;

    spin_lock_irqsave(&PktPoolLock, flags);
    pktPoolTail->next = pp;
    pp->next = NULL;
    pktPoolTail = pp;
    spin_unlock_irqrestore(&PktPoolLock, flags);
}

#ifdef TH_BH_ISR

static void IntrBH(unsigned long unused)
{
    struct pci_dev *pdev;
    struct privData *lp;
    Dma_Engine * eptr;
    unsigned long flags;
    int i;

    pdev = dmaData->pdev;
    lp = pci_get_drvdata(pdev);

    log_verbose("IntrBH with PendingMask %llx\n", PendingMask);

    //while(PendingMask)
    for(i=0; PendingMask && i<MAX_DMA_ENGINES; i++)
    {
        if(!(PendingMask & (1LL << i))) continue;
        spin_lock_irqsave(&IntrLock, flags);

        /* At this point, we have engine identified. */

        /* First, reset mask bit */
        PendingMask &= ~(1LL << i);

        spin_unlock_irqrestore(&IntrLock, flags);

        eptr = &(lp->Dma[i]);
        if(eptr->EngineState != USER_ASSIGNED)      // Should not happen
            continue;

        /* The spinlocks need to be handled within this function, so
         * don't do them here.
         */

        PktHandler(i, eptr);

        spin_lock_irqsave(&IntrLock, flags);
        Dma_mEngIntEnable(eptr);

        /* Update flag to synchronise between ISR and poll_routine */
        LastIntr[i] = jiffies;
        spin_unlock_irqrestore(&IntrLock, flags);
    }
}

u32 Acks(u32 dirqval)
{
    u32 retval=0;

    retval |= (dirqval & DMA_ENG_ENABLE_MASK) ? DMA_ENG_ENABLE : 0;
    retval |= (dirqval & DMA_ENG_INT_ACTIVE_MASK) ? DMA_ENG_INT_ACK : 0;
    retval |= (dirqval & DMA_ENG_INT_BDCOMP) ? DMA_ENG_INT_BDCOMP_ACK : 0;
    retval |= (dirqval & DMA_ENG_INT_ALERR) ? DMA_ENG_INT_ALERR_ACK : 0;
    retval |= (dirqval & DMA_ENG_INT_FETERR) ? DMA_ENG_INT_FETERR_ACK : 0;
    retval |= (dirqval & DMA_ENG_INT_ABORTERR) ? DMA_ENG_INT_ABORTERR_ACK : 0;
    retval |= (dirqval & DMA_ENG_INT_CHAINEND) ? DMA_ENG_INT_CHAINEND_ACK : 0;

    if(dirqval & DMA_ENG_INT_ACTIVE_MASK)
        retval &= ~(DMA_ENG_INT_ENABLE);        \

    log_verbose(KERN_INFO "Acking %x with %x\n", dirqval, retval);
    return retval;
}

/* This function serves to handle the initial interrupt, as well as to
 * check again on pending interrupts, from the BH. If this is not done,
 * interrupts can stall.
 */
int IntrCheck(struct pci_dev * dev)
{
    u32 girqval, dirqval;
    struct privData *lp;
    u32 base, imask;
    Dma_Engine * eptr;
    int i, retval=XST_FAILURE;
    static int count0=0, count1=0, count2=0, count3=0;

    lp = pci_get_drvdata(dev);
    log_verbose(KERN_INFO "IntrCheck: device %x\n", (u32) dev);

    base = (u32)(lp->barInfo[0].baseVAddr);
    girqval = Dma_mReadReg(base, REG_DMA_CTRL_STATUS);
    //if(!(girqval & (DMA_INT_ACTIVE_MASK|DMA_INT_PENDING_MASK|DMA_USER_INT_ACTIVE_MASK)))
    //if(!(girqval & (DMA_INT_ACTIVE_MASK|DMA_USER_INT_ACTIVE_MASK)))
    //    return;

    /* Now, check each S2C DMA engine (0 to 7) */
    imask = (girqval & DMA_S2C_ENG_INT_VAL) >> 16;
    for(i=0; i<7; i++)
    {
        if(!imask) break;
        if(!(imask & (0x01<<i))) continue;

        if(!((lp->engineMask) & (1LL << i)))
            continue;

        eptr = &(lp->Dma[i]);

        dirqval = Dma_mGetCrSr(eptr);
        log_verbose("Eng %d: dirqval is %x\n", i, dirqval);

        /* Check whether interrupt is enabled, otherwise it could be a
         * re-check of the last checked engine before its bottom half has run.
         */
        if((dirqval & DMA_ENG_INT_ACTIVE_MASK) &&
           (dirqval & DMA_ENG_INT_ENABLE))
        {
            spin_lock(&IntrLock);
            //Dma_mEngIntAck(eptr, Acks(dirqval));
            Dma_mSetCrSr(eptr, Acks(dirqval));         \

            if(dirqval & (DMA_ENG_INT_ALERR|DMA_ENG_INT_FETERR|DMA_ENG_INT_ABORTERR))
                printk("Eng %d: Came with error dirqval %x\n", i, dirqval);
            if(dirqval & DMA_ENG_INT_BDCOMP)
            {
                //Dma_mEngIntDisable(eptr); // Already disabled
                PendingMask |= (1LL << i);
            }
            spin_unlock(&IntrLock);

            if(i==0) count0++;
            else if(i==1) count1++;
            retval = XST_SUCCESS;
        }
else if(dirqval & DMA_ENG_INT_ACTIVE_MASK) log_normal("1");
    }

    /* Now, check each C2S DMA engine (32 to 39) */
    imask = (girqval & DMA_C2S_ENG_INT_VAL) >> 24;
    for(i=32; i<39; i++)
    {
        if(!imask) break;
        if(!(imask & (0x01<<(i-32)))) continue;

        if(!((lp->engineMask) & (1LL << i)))
            continue;

        eptr = &(lp->Dma[i]);

        dirqval = Dma_mGetCrSr(eptr);
        log_verbose("Eng %d: dirqval is %x\n", i, dirqval);

        /* Check whether interrupt is enabled, otherwise it could be a
         * re-check of the last checked engine before its bottom half has run.
         */
        if((dirqval & DMA_ENG_INT_ACTIVE_MASK) &&
           (dirqval & DMA_ENG_INT_ENABLE))
        {
            spin_lock(&IntrLock);
            Dma_mEngIntAck(eptr, Acks(dirqval));

            if(dirqval & (DMA_ENG_INT_ALERR|DMA_ENG_INT_FETERR|DMA_ENG_INT_ABORTERR))
                printk("Eng %d: Came with error dirqval %x\n", i, dirqval);
            if(dirqval & DMA_ENG_INT_BDCOMP)
            {
                Dma_mEngIntDisable(eptr);
                PendingMask |= (1LL << i);
            }
            spin_unlock(&IntrLock);

            if(i==32) count2++;
            else if(i==33) count3++;
            retval = XST_SUCCESS;
        }
        else if(dirqval & DMA_ENG_INT_ACTIVE_MASK) log_normal("~");
    }

    spin_lock(&IntrLock);
    if(PendingMask && (retval == XST_SUCCESS))
    {
        tasklet_schedule(&DmaBH);
    }
    spin_unlock(&IntrLock);

    return retval;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t DmaInterrupt(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t DmaInterrupt(int irq, void *dev_id)
#endif
{
    struct pci_dev *dev = dev_id;

  /* Handle DMA and any user interrupts */
  if(IntrCheck(dev) == XST_SUCCESS)
        return IRQ_HANDLED;
    else
        return IRQ_NONE;
}

#endif

static void PktHandler(int eng, Dma_Engine * eptr)
{
    struct pci_dev * pdev;
    Dma_BdRing * rptr;
    UserPtrs * uptr;
    Dma_Bd *BdPtr, *BdCurPtr;
    int result = XST_SUCCESS;
    unsigned int bd_processed, bd_processed_save;
    dma_addr_t bufPA;
    int j;
    static int txcount = 0;
    static int rxcount = 0;
    PktBuf * pbuf;
    struct PktPool * ppool;
    u32 flag;

    rptr = &(eptr->BdRing);
    uptr = &(eptr->user);
    pdev = eptr->pdev;

    //log_verbose("[PH%d] ", eng);

    if(rptr->IsRxChannel) flag = PCI_DMA_FROMDEVICE;
    else flag = PCI_DMA_TODEVICE;

    spin_lock_bh(&DmaLock);

    /* Handle engine operations */
    bd_processed_save = 0;
    if ((bd_processed = Dma_BdRingFromHw(rptr, DMA_BD_CNT, &BdPtr)) > 0)
    {
        log_verbose(KERN_INFO "PktHandler: Processed %d BDs\n", bd_processed);

        /* First, get a pool of packets to work with */
        ppool = DQPool();

        if(rptr->IsRxChannel) rxcount += bd_processed;
        else txcount += bd_processed;
        log_verbose("PktHandler: txcount %d rxcount %d\n", txcount, rxcount);

        bd_processed_save = bd_processed;
        BdCurPtr = BdPtr;
        j = 0;
        do
        {
            pbuf = &((ppool->pbuf)[j]);

            bufPA = (dma_addr_t) Dma_mBdGetBufAddr(BdCurPtr);
            pbuf->size = Dma_mBdGetStatLength(BdCurPtr);
            pbuf->bufInfo = (unsigned char *)Dma_mBdGetId(BdCurPtr);

            /* For now, do this. Temac driver does not actually look
             * at pbuf->pktBuf, but eventually, this is not the right
             * thing to do.
             */
            pbuf->pktBuf = pbuf->bufInfo;
            pbuf->flags = Dma_mBdGetStatus(BdCurPtr);
            pbuf->userInfo = Dma_mBdGetUserData(BdCurPtr);

            log_verbose(KERN_INFO "Length %d Buf %x\n", pbuf->size, (u32) bufPA);
            pci_unmap_single(pdev, bufPA, pbuf->size, flag);

            /* reset BD id */
            Dma_mBdSetId(BdCurPtr, NULL);

            BdCurPtr = Dma_mBdRingNext(rptr, BdCurPtr);
            bd_processed--;
            j++;

            /* Add to SW payload stats counters */
            spin_lock_bh(&DmaStatsLock);
            SWrate[eng] += pbuf->size;
            spin_unlock_bh(&DmaStatsLock);
        } while (bd_processed > 0);

        result = Dma_BdRingFree(rptr, bd_processed_save, BdPtr);
        if (result != XST_SUCCESS) {
            printk(KERN_ERR "PktHandler: BdRingFree() error %d.\n", result);
            EQPool(ppool);
            spin_unlock_bh(&DmaLock);
            return;
        }
        spin_unlock_bh(&DmaLock);

        if (bd_processed_save)
        {
            log_verbose(KERN_INFO "PktHandler processed %d BDs\n", bd_processed_save);
            (uptr->UserPutPkt)(eptr, ppool->pbuf, bd_processed_save, uptr->privData);
        }

        /* Now return packet pool to list */
        EQPool(ppool);

        spin_lock_bh(&DmaLock);
    }

    /* Handle any RX-specific engine operations */
    if(rptr->IsRxChannel)
    {
        /* Replenish BDs in the RX ring */
        DmaSetupRecvBuffers(pdev, eptr);
    }
    spin_unlock_bh(&DmaLock);
}

static void poll_routine(unsigned long __opaque)
{
    struct pci_dev *pdev = (struct pci_dev *)__opaque;
    Dma_Engine * eptr;
    struct privData *lp;
    int i, offset;

    if(DriverState == UNREGISTERING)
        return;

    lp = pci_get_drvdata(pdev);

    //printk("p%d ", get_cpu());
    for(i=0; i<MAX_DMA_ENGINES; i++)
    {
#ifdef TH_BH_ISR
        /* Do housekeeping only if adequate time has elapsed since
         * last ISR.
         */
        if(jiffies < (LastIntr[i] + (HZ/50))) continue;
#endif

        if(!((lp->engineMask) & (1LL << i)))
            continue;

        eptr = &(lp->Dma[i]);
        if(eptr->EngineState != USER_ASSIGNED)
            continue;

        /* The spinlocks need to be handled within this function, so
         * don't do them here.
         */
        PktHandler(i, eptr);
    }

    /* Reschedule poll routine. Incase interrupts are enabled, the
     * bulk of processing should happen in the ISR.
     */
#ifdef TH_BH_ISR
    offset = HZ / 50;
#else
    offset = 0;
#endif
    poll_timer.expires = jiffies + offset;
    add_timer(&poll_timer);
}

static void poll_stats(unsigned long __opaque)
{
    struct pci_dev *pdev = (struct pci_dev *)__opaque;
    struct privData *lp;
    Dma_Engine * eptr;
    Dma_BdRing * rptr;
    int i, offset = 0;
    u32 at, wt, cb, t1, t2;
    u32 base;

    if(DriverState == UNREGISTERING)
        return;

    lp = pci_get_drvdata(pdev);
    log_verbose("s%d ", get_cpu());

    /* First, get DMA payload statistics */
    for(i=0; i<MAX_DMA_ENGINES; i++)
    {
        if(!((lp->engineMask) & (1LL << i)))
            continue;

        eptr = &(lp->Dma[i]);
        rptr = &(eptr->BdRing);

        spin_lock(&DmaStatsLock);

        /* First, read the DMA engine payload registers */
        at = Dma_mReadReg(rptr->ChanBase, REG_DMA_ENG_ACTIVE_TIME);
        wt = Dma_mReadReg(rptr->ChanBase, REG_DMA_ENG_WAIT_TIME);
        cb = Dma_mReadReg(rptr->ChanBase, REG_DMA_ENG_COMP_BYTES);

        /* Want to store the latest set of statistics. If the GUI is not
         * running, the statistics will build up. So, read pointer should
         * move forward alongwith the write pointer.
         */
        DStats[i][dstatsWrite[i]].LAT = 4*(at>>2);
        DStats[i][dstatsWrite[i]].LWT = 4*(wt>>2);
        DStats[i][dstatsWrite[i]].LBR = 4*(cb>>2);
        dstatsWrite[i] += 1;
        if(dstatsWrite[i] >= MAX_STATS) dstatsWrite[i] = 0;

        if(dstatsNum[i] < MAX_STATS)
            dstatsNum[i] += 1;
        /* else move the read pointer forward */
        else
        {
            dstatsRead[i] += 1;
            if(dstatsRead[i] >= MAX_STATS) dstatsRead[i] = 0;
        }

        /* Next, read the SW statistics counters */
        t1 = SWrate[i];
        SStats[i][sstatsWrite[i]].LBR = t1;
        SWrate[i] = 0;
        sstatsWrite[i] += 1;
        if(sstatsWrite[i] >= MAX_STATS) sstatsWrite[i] = 0;

        if(sstatsNum[i] < MAX_STATS)
            sstatsNum[i] += 1;
        /* else move the read pointer forward */
        else
        {
            sstatsRead[i] += 1;
            if(sstatsRead[i] >= MAX_STATS) sstatsRead[i] = 0;
        }

        log_normal(KERN_INFO "[%d]: active=[%d]%u, wait=[%d]%u, comp bytes=[%d]%u, sw=%u\n",
                i, (at&0x3), 4*(at>>2), (wt&0x3), 4*(wt>>2),
                (cb&0x3), 4*(cb>>2), t1);

        spin_unlock(&DmaStatsLock);
    }

    /* Now, get the TRN statistics */

    /* Registers to be read for TRN stats */
    base = (u32)(dmaData->barInfo[0].baseVAddr);

    /* This counts all TLPs including header */
   // t1 = XIo_In32(base+0x8200);
   // t2 = XIo_In32(base+0x8204);
    t1 = XIo_In32(base+0x900c);
    t2 = XIo_In32(base+0x9010);
    //printk(KERN_INFO "TRN util: TX [%d]%u, RX [%d]%u\n",
    //      (t1&0x3), 4*(t1>>2), (t2&0x3), 4*(t2>>2));
    TStats[tstatsWrite].LTX = 4*(t1>>2);
    TStats[tstatsWrite].LRX = 4*(t2>>2);
    tstatsWrite += 1;
    if(tstatsWrite >= MAX_STATS) tstatsWrite = 0;

    if(tstatsNum < MAX_STATS)
        tstatsNum += 1;
    /* else move the read pointer forward */
    else
    {
        tstatsRead += 1;
        if(tstatsRead >= MAX_STATS) tstatsRead = 0;
    }

    /* This counts only payload of TLPs - buffer and BDs */
    t1 = XIo_In32(base+0x9014);
    t2 = XIo_In32(base+0x9018);
    //printk(KERN_INFO "TRN payload: TX [%d]%u, RX [%d]%u\n",
    //      (t1&0x3), 4*(t1>>2), (t2&0x3), 4*(t2>>2));

    /* Reschedule poll routine */
    offset = -3;
    stats_timer.expires = jiffies + HZ + offset;
    add_timer(&stats_timer);
}

#ifdef DEBUG_VERBOSE
void disp_frag(unsigned char * addr, u32 len)
{
  int i;

  for(i=0; i<len; i++)
  {
    printk("%02x ", addr[i]);
    if(!((i+1)%4))
      printk(", ");
    if(!((i+1)%16))
      printk("\n");
  }
  printk("\n");
}
#endif

/* This function returns all unused packets to the app driver. Will either
 * happen because packets got for reception could not be queued for DMA,
 * or while the app driver is unregistering itself, just prior to unloading.
 */
static void PutUnusedPkts(Dma_Engine * eptr, PktBuf * pbuf, int numpkts)
{
    int i;
    UserPtrs * uptr;

    uptr = &(eptr->user);

    for(i=0; i < numpkts; i++)
        pbuf[i].flags = PKT_UNUSED;

    (uptr->UserPutPkt)(eptr, pbuf, numpkts, uptr->privData);
}

/*
 * DmaSetupRecvBuffers allocates as many packet buffers as it can up to
 * the number of free C2S buffer descriptors, and sets up the C2S
 * buffer descriptors to DMA into the buffers.
 */
static void DmaSetupRecvBuffers(struct pci_dev *pdev, Dma_Engine * eptr)
{
  struct privData *lp = NULL;
    Dma_BdRing * rptr;
    UserPtrs * uptr;
  int free_bd_count ;
    int numbds;
  dma_addr_t bufPA;
  Dma_Bd *BdPtr, *BdCurPtr;
  int result, num, numgot;
    int i, len;
    struct PktPool * ppool;
#ifdef TH_BH_ISR
    u32 mask;
#endif

#if defined DEBUG_NORMAL || defined DEBUG_VERBOSE
    //static int recv_count=1;
    //log_normal(KERN_INFO "DmaSetupRecvBuffers: #%d\n", recv_count++);
#endif

    lp = pci_get_drvdata(pdev);
    rptr = &(eptr->BdRing);
    uptr = &(eptr->user);
    free_bd_count = Dma_mBdRingGetFreeCnt(rptr);

    /* Maintain a separation between start and end of BD ring. This is
     * required because DMA will stall if the two pointers coincide -
     * this will happen whether ring is full or empty.
     */
    if(free_bd_count > 2) free_bd_count -= 2;
    else return;

    log_verbose(KERN_INFO "SetupRecv: Free BD count is %d\n", free_bd_count);

    /* First, get a pool of packets to work with */
    ppool = DQPool();

    numbds = 0;
    do {
        /* Get buffers from user */
        num = free_bd_count;
        log_verbose(KERN_INFO "Trying to get %d buffers from user driver\n", num);
    numgot = (uptr->UserGetPkt)(eptr, ppool->pbuf, eptr->pktSize, num, uptr->privData);
    if (!numgot) {
            log_verbose(KERN_ERR "Could not get any packet for RX from user\n");
      break;
    }

        /* Allocate BDs from ring */
        result = Dma_BdRingAlloc(rptr, numgot, &BdPtr);
        if (result != XST_SUCCESS) {
            /* We really shouldn't get this. Return unused buffers to app */
            printk(KERN_ERR "DmaSetupRecvBuffers: BdRingAlloc unsuccessful (%d)\n",
                   result);
            PutUnusedPkts(eptr, ppool->pbuf, numgot);
            break;
        }

        log_verbose(KERN_INFO "User returned %d RX buffers\n", numgot);
        BdCurPtr = BdPtr;
        for(i = 0; i < numgot; i++)
        {
            PktBuf * pbuf;

            pbuf = &((ppool->pbuf)[i]);
            bufPA = pci_map_single(pdev, (u32 *)(pbuf->pktBuf), pbuf->size, PCI_DMA_FROMDEVICE);
            log_verbose(KERN_INFO "The buffer after alloc is at VA %x PA %x size %d\n",
                            (u32) pbuf->pktBuf, bufPA, pbuf->size);

            Dma_mBdSetBufAddr(BdCurPtr, bufPA);
            Dma_mBdSetCtrlLength(BdCurPtr, pbuf->size);
            Dma_mBdSetId(BdCurPtr, pbuf->bufInfo);
            Dma_mBdSetCtrl(BdCurPtr, 0);        // Disable interrupts also.
            Dma_mBdSetUserData(BdCurPtr, 0LL);

#ifdef TH_BH_ISR
            /* Enable interrupts for errors and completion based on
             * coalesce count.
             */
            mask = DMA_BD_INT_ERROR_MASK;
            if(!(eptr->intrCount % INT_COAL_CNT))
                mask |= DMA_BD_INT_COMP_MASK;
            eptr->intrCount += 1;
            Dma_mBdSetCtrl(BdCurPtr, mask);
#endif

            BdCurPtr = Dma_mBdRingNext(rptr, BdCurPtr);
        }

        /* Enqueue all Rx BDs with attached buffers such that they are
         * ready for frame reception.
         */
        result = Dma_BdRingToHw(rptr, numgot, BdPtr);
        if (result != XST_SUCCESS) {
            /* Should not come here. Incase of error, unmap buffers,
             * unallocate BDs, and return buffers to app driver.
             */
            printk(KERN_ERR "DmaSetupRecvBuffers: BdRingToHw unsuccessful (%d)\n",
                   result);
            BdCurPtr = BdPtr;
            for(i=0; i < numgot; i++)
            {
                bufPA = Dma_mBdGetBufAddr(BdCurPtr);
                len = Dma_mBdGetCtrlLength(BdCurPtr);
                pci_unmap_single(pdev, bufPA, len, PCI_DMA_FROMDEVICE);
                Dma_mBdSetId(BdCurPtr, NULL);
                BdCurPtr = Dma_mBdRingNext(rptr, BdCurPtr);
            }
            Dma_BdRingUnAlloc(rptr, numgot, BdPtr);
            PutUnusedPkts(eptr, ppool->pbuf, numgot);
            break;
        }

        free_bd_count -= numgot;
        numbds += numgot;
        log_verbose(KERN_INFO "free_bd_count %d, numbds %d, numgot %d\n",
                            free_bd_count, numbds, numgot);
    } while (free_bd_count > 0);

    /* Return packet pool to list */
    EQPool(ppool);

#ifdef DEBUG_VERBOSE
    if(numbds)
        log_verbose(KERN_INFO "DmaSetupRecvBuffers: %d new RX BDs queued up\n",
                                        numbds);
#endif
}

/*****************************************************************************/
/**
 * This function initializes the DMA BD ring as follows -
 * - Calculates the space required by the DMA BD ring
 * - Allocates the space, and aligns it as per DMA engine requirement
 * - Creates the BD ring structure in the allocated space
 * - If it is a RX DMA engine, allocates buffers from the user driver, and
 *   associates each BD in the RX BD ring with a buffer
 *
 * @param  pdev is the PCI/PCIe device instance
 * @param  eptr is a pointer to the DMA engine instance to be worked on.
 *
 * @return 0 if successful
 * @return negative value if unsuccessful
 *
 *****************************************************************************/
int descriptor_init(struct pci_dev *pdev, Dma_Engine * eptr)
{
  int dftsize, numbds;
    u32 * BdPtr;
    dma_addr_t BdPhyAddr ;
  int result;
    u32 delta = 0;

  /* Calculate size of descriptor space pool - extra to allow for
     * alignment adjustment.
     */
  dftsize = sizeof(u32) * DMA_BD_SW_NUM_WORDS * (DMA_BD_CNT + 1);
  log_normal(KERN_INFO "XDMA: BD space: %d (0x%0x)\n", dftsize, dftsize);

    if((BdPtr = pci_alloc_consistent(pdev, dftsize, &BdPhyAddr)) == NULL)
    {
        printk(KERN_ERR "BD ring pci_alloc_consistent() failed\n");
        return -1;
    }

    log_normal(KERN_INFO "BD ring space allocated from %p, PA 0x%x\n",
                                                BdPtr, BdPhyAddr);
    numbds = Dma_BdRingAlign((u32)BdPtr, dftsize, DMA_BD_MINIMUM_ALIGNMENT, &delta);
    if(numbds <= 0) {
        log_normal(KERN_ERR "Unable to align allocated BD space\n");
        /* Free allocated space !!!! */
        return -1;
    }

    eptr->descSpacePA = BdPhyAddr + delta;
    eptr->descSpaceVA = BdPtr + delta;
  eptr->descSpaceSize = dftsize - delta;
    eptr->delta = delta;

  if (eptr->descSpaceVA == 0) {
    return -1;
  }

  log_normal(KERN_INFO
        "XDMA: (descriptor_init) PA: 0x%x, VA: 0x%x, Size: %d, Delta: %d\n",
         eptr->descSpacePA, (unsigned int) eptr->descSpaceVA,
         eptr->descSpaceSize, eptr->delta);

  result = Dma_BdRingCreate(&(eptr->BdRing), (u32) eptr->descSpacePA,
             (u32) eptr->descSpaceVA, DMA_BD_MINIMUM_ALIGNMENT, numbds);
  if (result != XST_SUCCESS)
  {
    printk(KERN_ERR "XDMA: DMA Ring Create. Error: %d\n", result);
    return -EIO;
  }

    if((eptr->Type & DMA_ENG_DIRECTION_MASK) == DMA_ENG_C2S)
        DmaSetupRecvBuffers(pdev, eptr);

#ifdef DEBUG_VERBOSE
  log_verbose(KERN_INFO "BD Ring buffers:\n");
  disp_bd_ring(&eptr->BdRing);
#endif

  return 0;
}

/*****************************************************************************/
/**
 * In order to free allocated space (and avoid memory leaks), this function
 * is called when the user driver unregisters itself from the DMA base driver.
 * It does the following -
 * - Forcibly retrieves the buffers which have been queued up for DMA with
 *   the DMA engine hardware
 * - Unmaps these buffers from the PCI/PCIe space
 * - Returns these buffers to the user driver, which will free them
 * - Frees the BD ring
 * - De-allocates the space used for the BD ring, and unmaps it from
 *   the PCI/PCIe space
 *
 * @param  pdev is the PCI/PCIe device instance
 * @param  eptr is a pointer to the DMA engine instance to be worked on.
 *
 * @return None.
 *
 *****************************************************************************/
void descriptor_free(struct pci_dev *pdev, Dma_Engine * eptr)
{
    Dma_Bd *BdPtr, *BdCurPtr;
    unsigned int bd_processed, bd_processed_save;
    Dma_BdRing * rptr;
    UserPtrs * uptr;
    PktBuf * pbuf;
    dma_addr_t bufPA;
    int j, result;
    struct PktPool * ppool;
u32 flag;

    log_verbose(KERN_INFO "descriptor_free: \n");

    rptr = &(eptr->BdRing);
    uptr = &(eptr->user);

    if(rptr->IsRxChannel) flag = PCI_DMA_FROMDEVICE;
    else flag = PCI_DMA_TODEVICE;

    spin_lock_bh(&DmaLock);

    /* First recover buffers and BDs queued up for DMA, then pass to user */
    bd_processed_save = 0;
    if ((bd_processed = Dma_BdRingForceFromHw(rptr, DMA_BD_CNT, &BdPtr)) > 0)
    {
        log_normal(KERN_INFO "descriptor_free: Forced %d BDs from hw\n", bd_processed);
        /* First, get a pool of packets to work with */
        ppool = DQPool();

        bd_processed_save = bd_processed;
        BdCurPtr = BdPtr;
        j = 0;
        do
        {
            pbuf = &((ppool->pbuf)[j]);

            bufPA = (dma_addr_t) Dma_mBdGetBufAddr(BdCurPtr);
            pbuf->size = Dma_mBdGetCtrlLength(BdCurPtr);
            pbuf->bufInfo = (unsigned char *)Dma_mBdGetId(BdCurPtr);
            /* For now, do this. Temac driver does not actually look
             * at pbuf->pktBuf, but eventually, this is not the right
             * thing to do.
             */
            pbuf->pktBuf = pbuf->bufInfo;
            pbuf->flags = PKT_UNUSED;
            pbuf->userInfo = Dma_mBdGetUserData(BdCurPtr);

            /* Now unmap this buffer */
#ifdef DEBUG_VERBOSE
            log_verbose(KERN_INFO "Length %d Buf %x\n", pbuf->size, (u32) bufPA);
#endif
            pci_unmap_single(pdev, bufPA, pbuf->size, flag);

            /* reset BD id */
            Dma_mBdSetId(BdCurPtr, NULL);

            BdCurPtr = Dma_mBdRingNext(rptr, BdCurPtr);
            bd_processed--;
            j++;
        } while (bd_processed > 0);

        spin_unlock_bh(&DmaLock);

        if (bd_processed_save)
        {
            log_normal("DmaUnregister pushing %d buffers to user\n", bd_processed_save);
            (uptr->UserPutPkt)(eptr, ppool->pbuf, bd_processed_save, uptr->privData);
        }

        spin_lock_bh(&DmaLock);

        result = Dma_BdRingFree(rptr, bd_processed_save, BdPtr);
        if (result != XST_SUCCESS) {
            /* Will be freeing the ring below. */
            printk(KERN_ERR "DmaUnregister: BdRingFree() error %d.\n", result);
            //return;
        }

        EQPool(ppool);
    }
    spin_unlock_bh(&DmaLock);

    /* Now free BD ring itself */
  if (eptr->descSpaceVA == 0) {
        printk(KERN_ERR "Unable to free BD ring NULL\n");
    return;
  }
    //spin_lock_bh(&DmaLock);
  log_verbose(KERN_INFO
        "XDMA: (descriptor_free) BD ring PA: 0x%x, VA: 0x%x, Size: %d, Delta: %d\n",
         (eptr->descSpacePA - eptr->delta),
           (unsigned int) (eptr->descSpaceVA - eptr->delta),
         (eptr->descSpaceSize + eptr->delta), eptr->delta);
    pci_free_consistent(pdev, (eptr->descSpaceSize + eptr->delta),
                       (eptr->descSpaceVA - eptr->delta),
                       (eptr->descSpacePA - eptr->delta));
    //spin_unlock_bh(&DmaLock);
}

#ifdef DEBUG_VERBOSE

void disp_bd_ring(Dma_BdRing *bd_ring)
{
  int num_bds = bd_ring->AllCnt;
  u32 *dptr ;
  int idx;

/*
  printk("ChanBase: %p\n", (void *) bd_ring->ChanBase);
  printk("FirstBdPhysAddr: %p\n", (void *) bd_ring->FirstBdPhysAddr);
  printk("FirstBdAddr: %p\n", (void *) bd_ring->FirstBdAddr);
  printk("LastBdAddr: %p\n", (void *) bd_ring->LastBdAddr);
  printk("Length: %d (0x%0x)\n", bd_ring->Length, bd_ring->Length);
  printk("RunState: %d (0x%0x)\n", bd_ring->RunState, bd_ring->RunState);
  printk("Separation: %d (0x%0x)\n", bd_ring->Separation,
         bd_ring->Separation);
  printk("BD Count: %d\n", bd_ring->AllCnt);

  printk("\n");

  printk("FreeHead: %p\n", (void *) bd_ring->FreeHead);
  printk("PreHead: %p\n", (void *) bd_ring->PreHead);
  printk("HwHead: %p\n", (void *) bd_ring->HwHead);
  printk("HwTail: %p\n", (void *) bd_ring->HwTail);
  printk("PostHead: %p\n", (void *) bd_ring->PostHead);
  printk("BdaRestart: %p\n", (void *) bd_ring->BdaRestart);
*/
  printk("Ring %p Contents:\n", bd_ring);
  printk("Idx Status / UStatusL UStatusH  CAddrL  Control/ SysAddrL SysAddrH NextBD\n");
  printk("      BC                               CAddrH/BC \n");
  printk("--- -------- -------- -------- -------- -------- -------- -------- --------\n");

  dptr = (u32 *)bd_ring->FirstBdAddr;
  for (idx = 0; idx < num_bds; idx++)
  {
    int i;
    printk("%3d ", idx);
    for(i=0; i<8; i++)
    {
            printk("%08x ", *dptr);
            dptr++;
    }
        printk("\n");
    printk("    ");
    for(i=0; i<8; i++)
    {
            printk("%08x ", *dptr);
            dptr++;
    }
        printk("\n");
  }

  printk("--------------------------------------- Done ---------------------------------------\n");
}

#endif

#if defined(DEBUG_VERBOSE) || defined(DEBUG_NORMAL)
static void ReadConfig(struct pci_dev * pdev)
{
  int i;
  u8 valb;
  u16 valw;
  u32 valdw;
  unsigned long reg_base, reg_len;

  /* Read PCI configuration space */
  printk(KERN_INFO "PCI Configuration Space:\n");
  for(i=0; i<0x40; i++)
  {
    pci_read_config_byte(pdev, i, &valb);
    printk("0x%x ", valb);
    if((i % 0x10) == 0xf)
      printk("\n");
  }
  printk("\n");

  /* Now read each element - one at a time */

  /* Read Vendor ID */
  pci_read_config_word(pdev, PCI_VENDOR_ID, &valw);
  printk("Vendor ID: 0x%x, ", valw);

  /* Read Device ID */
  pci_read_config_word(pdev, PCI_DEVICE_ID, &valw);
  printk("Device ID: 0x%x, ", valw);

  /* Read Command Register */
  pci_read_config_word(pdev, PCI_COMMAND, &valw);
  printk("Cmd Reg: 0x%x, ", valw);

  /* Read Status Register */
  pci_read_config_word(pdev, PCI_STATUS, &valw);
  printk("Stat Reg: 0x%x, ", valw);

  /* Read Revision ID */
  pci_read_config_byte(pdev, PCI_REVISION_ID, &valb);
  printk("Revision ID: 0x%x, ", valb);

  /* Read Class Code */
/*
  pci_read_config_dword(pdev, PCI_CLASS_PROG, &valdw);
  printk("Class Code: 0x%lx, ", valdw);
  valdw &= 0x00ffffff;
  printk("Class Code: 0x%lx, ", valdw);
*/
  /* Read Reg-level Programming Interface */
  pci_read_config_byte(pdev, PCI_CLASS_PROG, &valb);
  printk("Class Prog: 0x%x, ", valb);

  /* Read Device Class */
  pci_read_config_word(pdev, PCI_CLASS_DEVICE, &valw);
  printk("Device Class: 0x%x, ", valw);

  /* Read Cache Line */
  pci_read_config_byte(pdev, PCI_CACHE_LINE_SIZE, &valb);
  printk("Cache Line Size: 0x%x, ", valb);

  /* Read Latency Timer */
  pci_read_config_byte(pdev, PCI_LATENCY_TIMER, &valb);
  printk("Latency Timer: 0x%x, ", valb);

  /* Read Header Type */
  pci_read_config_byte(pdev, PCI_HEADER_TYPE, &valb);
  printk("Header Type: 0x%x, ", valb);

  /* Read BIST */
  pci_read_config_byte(pdev, PCI_BIST, &valb);
  printk("BIST: 0x%x\n", valb);

  /* Read all 6 BAR registers */
  for(i=0; i<=5; i++)
  {
    /* Physical address & length */
    reg_base = pci_resource_start(pdev, i);
    reg_len = pci_resource_len(pdev, i);
    printk("BAR%d: Addr:0x%lx Len:0x%lx,  ", i, reg_base, reg_len);

    /* Flags */
    if((pci_resource_flags(pdev, i) & IORESOURCE_MEM))
      printk("Region is for memory\n");
    else if((pci_resource_flags(pdev, i) & IORESOURCE_IO))
      printk("Region is for I/O\n");
  }
    printk("\n");

  /* Read CIS Pointer */
  pci_read_config_dword(pdev, PCI_CARDBUS_CIS, &valdw);
  printk("CardBus CIS Pointer: 0x%x, ", valdw);

  /* Read Subsystem Vendor ID */
  pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, &valw);
  printk("Subsystem Vendor ID: 0x%x, ", valw);

  /* Read Subsystem Device ID */
  pci_read_config_word(pdev, PCI_SUBSYSTEM_ID, &valw);
  printk("Subsystem Device ID: 0x%x\n", valw);

  /* Read Expansion ROM Base Address */
  pci_read_config_dword(pdev, PCI_ROM_ADDRESS, &valdw);
  printk("Expansion ROM Base Address: 0x%x\n", valdw);

  /* Read IRQ Line */
  pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &valb);
  printk("IRQ Line: 0x%x, ", valb);

  /* Read IRQ Pin */
  pci_read_config_byte(pdev, PCI_INTERRUPT_PIN, &valb);
  printk("IRQ Pin: 0x%x, ", valb);

  /* Read Min Gnt */
  pci_read_config_byte(pdev, PCI_MIN_GNT, &valb);
  printk("Min Gnt: 0x%x, ", valb);

  /* Read Max Lat */
  pci_read_config_byte(pdev, PCI_MAX_LAT, &valb);
  printk("Max Lat: 0x%x\n", valb);
}
#endif

#ifdef DEBUG_VERBOSE
static void ReadRoot(struct pci_dev * pdev)
{
  int i;
  u8 valb;
  struct pci_bus * parent;
  struct pci_bus * me;

  /* Read PCI configuration space for all devices on this bus */
  parent = pdev->bus->parent;
  for(i=0; i<256; i++)
  {
    pci_bus_read_config_byte(parent, 8, i, &valb);
    printk("%02x ", valb);
    if(!((i+1) % 16)) printk("\n");
  }

  printk("Device %p details:\n", pdev);
  printk("Bus_list %p\n", &(pdev->bus_list));
  printk("Bus %p\n", pdev->bus);
  printk("Subordinate %p\n", pdev->subordinate);
  printk("Sysdata %p\n", pdev->sysdata);
  printk("Procent %p\n", pdev->procent);
  printk("Devfn %d\n", pdev->devfn);
  printk("Vendor %x\n", pdev->vendor);
  printk("Device %x\n", pdev->device);
  printk("Subsystem_vendor %x\n", pdev->subsystem_vendor);
  printk("Subsystem_device %x\n", pdev->subsystem_device);
  printk("Class %d\n", pdev->class);
  printk("Hdr_type %d\n", pdev->hdr_type);
  printk("Rom_base_reg %d\n", pdev->rom_base_reg);
  printk("Pin %d\n", pdev->pin);
  printk("Driver %p\n", pdev->driver);
  printk("Dma_mask %lx\n", (unsigned long)(pdev->dma_mask));
  printk("Vendor_compatible: ");
  //for(i=0; i<DEVICE_COUNT_COMPATIBLE; i++)
  //  printk("%x ", pdev->vendor_compatible[i]);
  //printk("\n");
  //printk("Device_compatible: ");
  //for(i=0; i<DEVICE_COUNT_COMPATIBLE; i++)
  //  printk("%x ", pdev->device_compatible[i]);
  //printk("\n");
  printk("Cfg_size %d\n", pdev->cfg_size);
  printk("Irq %d\n", pdev->irq);
  printk("Transparent %d\n", pdev->transparent);
  printk("Multifunction %d\n", pdev->multifunction);
  //printk("Is_enabled %d\n", pdev->is_enabled);
  printk("Is_busmaster %d\n", pdev->is_busmaster);
  printk("No_msi %d\n", pdev->no_msi);
  printk("No_dld2 %d\n", pdev->no_d1d2);
  printk("Block_ucfg_access %d\n", pdev->block_ucfg_access);
  printk("Broken_parity_status %d\n", pdev->broken_parity_status);
  printk("Msi_enabled %d\n", pdev->msi_enabled);
  printk("Msix_enabled %d\n", pdev->msix_enabled);
  printk("Rom_attr_enabled %d\n", pdev->rom_attr_enabled);

  me = pdev->bus;
  printk("Bus details:\n");
  printk("Parent %p\n", me->parent);
  printk("Children %p\n", &(me->children));
  printk("Devices %p\n", &(me->devices));
  printk("Self %p\n", me->self);
  printk("Sysdata %p\n", me->sysdata);
  printk("Procdir %p\n", me->procdir);
  printk("Number %d\n", me->number);
  printk("Primary %d\n", me->primary);
  printk("Secondary %d\n", me->secondary);
  printk("Subordinate %d\n", me->subordinate);
  printk("Name %s\n", me->name);
  printk("Bridge_ctl %d\n", me->bridge_ctl);
  printk("Bridge %p\n", me->bridge);
}
#endif

static void ReadDMAEngineConfiguration(struct pci_dev * pdev, struct privData * dmaInfo)
{
    u32 base, offset;
    u32 val, type, dirn, num, bc;
    int i;
    Dma_Engine * eptr;

    /* DMA registers are in BAR0 */
    base = (u32)(dmaInfo->barInfo[0].baseVAddr);

    printk(KERN_INFO "Hardware design version %x\n", XIo_In32(base+0x8000));

    /* Walk through the capability register of all DMA engines */
    for(offset = DMA_OFFSET, i=0; offset < DMA_SIZE; offset += DMA_ENGINE_PER_SIZE, i++)
    {
        log_verbose(KERN_INFO "Reading engine capability from %x\n",
                                            (base+offset+REG_DMA_ENG_CAP));
        val = Dma_mReadReg((base+offset), REG_DMA_ENG_CAP);
        log_verbose(KERN_INFO "REG_DMA_ENG_CAP returned %x\n", val);

        if(val & DMA_ENG_PRESENT_MASK)
        {
            log_verbose(KERN_INFO "Engine capability is %x\n", val);
            eptr = &(dmaInfo->Dma[i]);

            log_verbose(KERN_INFO "DMA Engine present at offset %x: ", offset);

            dirn = (val & DMA_ENG_DIRECTION_MASK);
            if(dirn == DMA_ENG_C2S)
                printk("C2S, ");
            else
                printk("S2C, ");

            type = (val & DMA_ENG_TYPE_MASK);
            if(type == DMA_ENG_BLOCK)
                printk("Block DMA, ");
            else if(type == DMA_ENG_PACKET)
                printk("Packet DMA, ");
            else
                printk("Unknown DMA %x, ", type);

            num = (val & DMA_ENG_NUMBER) >> DMA_ENG_NUMBER_SHIFT;
            printk("Eng. Number %d, ", num);

            bc = (val & DMA_ENG_BD_MAX_BC) >> DMA_ENG_BD_MAX_BC_SHIFT;
            printk("Max Byte Count 2^%d\n", bc);

            if(type != DMA_ENG_PACKET) {
                log_normal(KERN_ERR "This driver is capable of only Packet DMA\n");
                continue;
            }

            /* Initialise this engine's data structure. This will also
             * reset the DMA engine.
             */
            Dma_Initialize(eptr, (base + offset), dirn);
            eptr->pdev = pdev;

            dmaInfo->engineMask |= (1LL << i);
        }
    }
    log_verbose(KERN_INFO "Engine mask is 0x%llx\n", dmaInfo->engineMask);
}

/* Character device file operations */
static int xdma_dev_open(struct inode * in, struct file * filp)
{
    if(DriverState != INITIALIZED)
    {
        printk("Driver not yet ready!\n");
        return -1;
    }

    if(UserOpen)
    {
        printk("Device already in use\n");
        return -EBUSY;
    }

    spin_lock_bh(&DmaStatsLock);
    UserOpen++;                 /* To prevent more than one GUI */
    spin_unlock_bh(&DmaStatsLock);

    return 0;
}

static int xdma_dev_release(struct inode * in, struct file * filp)
{
    if(!UserOpen)
    {
        /* Should not come here */
        printk("Device not in use\n");
        return -EFAULT;
    }

    spin_lock_bh(&DmaStatsLock);
    UserOpen-- ;
    spin_unlock_bh(&DmaStatsLock);

    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int xdma_dev_ioctl(struct inode * in, struct file * filp,
                          unsigned int cmd, unsigned long arg)
#else
static long xdma_dev_ioctl(struct file * filp,
                          unsigned int cmd, unsigned long arg)
#endif
{
    int retval=0;
    EngState eng;
    EngStatsArray es;
    TRNStatsArray tsa;
    SWStatsArray ssa;
    DMAStatistics * ds;
    TRNStatistics * ts;
    SWStatistics * ss;
    TestCmd tc;
    UserState ustate;
    PCIState pcistate;
    int len, i;
    Dma_Engine * eptr;
    Dma_BdRing * rptr;
    UserPtrs * uptr;

    if(DriverState != INITIALIZED)
    {
        /* Should not come here */
        printk("Driver not yet ready!\n");
        return -1;
    }

    /* Check cmd type and value */
    if(_IOC_TYPE(cmd) != XPMON_MAGIC) return -ENOTTY;
    if(_IOC_NR(cmd) > XPMON_MAX_CMD) return -ENOTTY;

    /* Check read/write and corresponding argument */
    if(_IOC_DIR(cmd) & _IOC_READ)
        if(!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd)))
            return -EFAULT;
    if(_IOC_DIR(cmd) & _IOC_WRITE)
        if(!access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd)))
            return -EFAULT;

    /* Looks ok, let us continue */
    switch(cmd)
    {
    case IGET_TEST_STATE:
        if(copy_from_user(&tc, (TestCmd *)arg, sizeof(TestCmd)))
        {
            printk("copy_from_user failed\n");
            retval = -EFAULT;
            break;
        }

        i = tc.Engine;
        //printk("For engine %d\n", i);

        /* First, check if requested engine is valid */
        if((i >= MAX_DMA_ENGINES) ||
          (!((dmaData->engineMask) & (1LL << i))))
        {
            printk("Invalid engine %d\n", i);
            retval = -EFAULT;
            break;
        }
        eptr = &(dmaData->Dma[i]);
        uptr = &(eptr->user);

        /* Then check if the user function registered */
        if((eptr->EngineState != USER_ASSIGNED) ||
           (uptr->UserGetState == NULL))
        {
            log_normal(KERN_ERR "UserGetState function does not exist\n");
            retval = -EFAULT;
            break;
        }

    if(!(uptr->UserGetState)(eptr, &ustate, uptr->privData))
        {
            tc.TestMode = ustate.TestMode;
            tc.MinPktSize = ustate.MinPktSize;
            tc.MaxPktSize = ustate.MaxPktSize;
            if(copy_to_user((TestCmd *)arg, &tc, sizeof(TestCmd)))
            {
                printk("copy_to_user failed\n");
                retval = -EFAULT;
                break;
            }
        }
        else
        {
            printk("UserGetState returned failure\n");
            retval = -EFAULT;
            break;
        }
        break;

    case ISTART_TEST:
    case ISTOP_TEST:
        if(copy_from_user(&tc, (TestCmd *)arg, sizeof(TestCmd)))
        {
            printk("copy_from_user failed\n");
            retval = -EFAULT;
            break;
        }

        i = tc.Engine;
        //printk("For engine %d\n", i);

        /* First, check if requested engine is valid */
        if((i >= MAX_DMA_ENGINES) ||
          (!((dmaData->engineMask) & (1LL << i))))
        {
            printk("Invalid engine %d\n", i);
            retval = -EFAULT;
            break;
        }
        eptr = &(dmaData->Dma[i]);
        uptr = &(eptr->user);

        /* Then check if the user function registered */
        if((eptr->EngineState != USER_ASSIGNED) ||
           (uptr->UserGetState == NULL))
        {
            log_normal(KERN_ERR "UserSetState function does not exist\n");
            retval = -EFAULT;
            break;
        }

        /* Use the whole bitmask, because we should not disturb any
         * other running test.
         */
        printk("For engine %d, TestMode %x\n", i, tc.TestMode);
        ustate.TestMode = tc.TestMode;
        ustate.MinPktSize = tc.MinPktSize;
        ustate.MaxPktSize = tc.MaxPktSize;
    retval = (uptr->UserSetState)(eptr, &ustate, uptr->privData);
        if(retval)
            printk("UserSetState returned failure %d\n", retval);
        break;

    case IGET_PCI_STATE:
        ReadPCIState(dmaData->pdev, &pcistate);
        if(copy_to_user((PCIState *)arg, &pcistate, sizeof(PCIState)))
        {
            printk("copy_to_user failed\n");
            retval = -EFAULT;
            break;
        }
        break;

    case IGET_ENG_STATE:
        if(copy_from_user(&eng, (EngState *)arg, sizeof(EngState)))
        {
            printk("\ncopy_from_user failed\n");
            retval = -EFAULT;
            break;
        }

        i = eng.Engine;
        //printk("For engine %d\n", i);

        /* First, check if requested engine is valid */
        if((i >= MAX_DMA_ENGINES) ||
          (!((dmaData->engineMask) & (1LL << i))))
        {
            printk("Invalid engine %d\n", i);
            retval = -EFAULT;
            break;
        }
        eptr = &(dmaData->Dma[i]);
        rptr = &(eptr->BdRing);
        uptr = &(eptr->user);

        /* Then check if the user function registered */
        if((eptr->EngineState != USER_ASSIGNED) ||
           (uptr->UserGetState == NULL))
        {
            log_normal(KERN_ERR "UserGetState function does not exist\n");
            retval = -EFAULT;
            break;
        }

        /* First, get the user state */
    if(!(uptr->UserGetState)(eptr, &ustate, uptr->privData))
        {
            eng.Buffers = ustate.Buffers;
            eng.MinPktSize = ustate.MinPktSize;
            eng.MaxPktSize = ustate.MaxPktSize;
            eng.TestMode = ustate.TestMode;
        }
        else
        {
            printk("UserGetState returned failure\n");
            retval = -EFAULT;
            break;
        }

        /* Now add the DMA state */
        eng.BDs = DMA_BD_CNT;
        eng.BDerrs = rptr->BDerrs;
        eng.BDSerrs = rptr->BDSerrs;
#ifdef TH_BH_ISR
        eng.IntEnab = 1;
#else
        eng.IntEnab = 0;
#endif
        if(copy_to_user((EngState *)arg, &eng, sizeof(EngState)))
        {
            printk("copy_to_user failed\n");
            retval = -EFAULT;
            break;
        }
        break;

    case IGET_DMA_STATISTICS:
        if(copy_from_user(&es, (EngStatsArray *)arg, sizeof(EngStatsArray)))
        {
            printk("copy_from_user failed\n");
            retval = -1;
            break;
        }

        ds = es.engptr;
        len = 0;
        for(i=0; i<es.Count; i++)
        {
            DMAStatistics from;
            int j;

            /* Must copy in a round-robin manner so that reporting is fair */
            for(j=0; j<MAX_DMA_ENGINES; j++)
            {
                if(!dstatsNum[j]) continue;

                spin_lock_bh(&DmaStatsLock);
                from = DStats[j][dstatsRead[j]];
                from.Engine = j;
                dstatsNum[j] -= 1;
                dstatsRead[j] += 1;
                if(dstatsRead[j] == MAX_STATS)
                    dstatsRead[j] = 0;
                spin_unlock_bh(&DmaStatsLock);

                if(copy_to_user(ds, &from, sizeof(DMAStatistics)))
                {
                    printk("copy_to_user failed\n");
                    retval = -EFAULT;
                    break;
                }

                len++;
                i++;
                if(i >= es.Count) break;
                ds++;
            }
            if(retval < 0) break;
        }
        es.Count = len;
        if(copy_to_user((EngStatsArray *)arg, &es, sizeof(EngStatsArray)))
        {
            printk("copy_to_user failed\n");
            retval = -EFAULT;
            break;
        }
        break;

    case IGET_TRN_STATISTICS:
        if(copy_from_user(&tsa, (TRNStatsArray *)arg, sizeof(TRNStatsArray)))
        {
            printk("copy_from_user failed\n");
            retval = -1;
            break;
        }

        ts = tsa.trnptr;
        len = 0;
        for(i=0; i<tsa.Count; i++)
        {
            TRNStatistics from;

            if(!tstatsNum) break;

            spin_lock_bh(&DmaStatsLock);
            from = TStats[tstatsRead];
            tstatsNum -= 1;
            tstatsRead += 1;
            if(tstatsRead == MAX_STATS)
                tstatsRead = 0;
            spin_unlock_bh(&DmaStatsLock);

            if(copy_to_user(ts, &from, sizeof(TRNStatistics)))
            {
                printk("copy_to_user failed\n");
                retval = -EFAULT;
                break;
            }

            len++;
            ts++;
        }
        tsa.Count = len;
        if(copy_to_user((TRNStatsArray *)arg, &tsa, sizeof(TRNStatsArray)))
        {
            printk("copy_to_user failed\n");
            retval = -EFAULT;
            break;
        }
        break;

    case IGET_SW_STATISTICS:
        if(copy_from_user(&ssa, (SWStatsArray *)arg, sizeof(SWStatsArray)))
        {
            printk("copy_from_user failed\n");
            retval = -1;
            break;
        }

        ss = ssa.swptr;
        len = 0;
        for(i=0; i<ssa.Count; i++)
        {
            SWStatistics from;
            int j;

            /* Must copy in a round-robin manner so that reporting is fair */
            for(j=0; j<MAX_DMA_ENGINES; j++)
            {
                if(!sstatsNum[j]) continue;

                spin_lock_bh(&DmaStatsLock);
                from = SStats[j][sstatsRead[j]];
                from.Engine = j;
                sstatsNum[j] -= 1;
                sstatsRead[j] += 1;
                if(sstatsRead[j] == MAX_STATS)
                    sstatsRead[j] = 0;
                spin_unlock_bh(&DmaStatsLock);

                if(copy_to_user(ss, &from, sizeof(SWStatistics)))
                {
                    printk("copy_to_user failed\n");
                    retval = -EFAULT;
                    break;
                }

                len++;
                i++;
                if(i >= ssa.Count) break;
                ss++;
            }
            if(retval < 0) break;
        }
        ssa.Count = len;
        if(copy_to_user((SWStatsArray *)arg, &ssa, sizeof(SWStatsArray)))
        {
            printk("copy_to_user failed\n");
            retval = -EFAULT;
            break;
        }
        break;

    default:
        printk("Invalid command %d\n", cmd);
        retval = -1;
        break;
    }

    return retval;
}

static int ReadPCIState(struct pci_dev * pdev, PCIState * pcistate)
{
  int pos;
  u16 valw;
  u8 valb;
#if defined(K7_TRD)
    u32 base;
#endif

    /* Since probe has succeeded, indicates that link is up. */
    pcistate->LinkState = LINK_UP;
    pcistate->VendorId = PCI_VENDOR_ID_DMA;
    pcistate->DeviceId = PCI_DEVICE_ID_DMA;

    /* Read Interrupt setting - Legacy or MSI/MSI-X */
    pci_read_config_byte(pdev, PCI_INTERRUPT_PIN, &valb);
    if(!valb)
    {
        if(pci_find_capability(pdev, PCI_CAP_ID_MSIX))
            pcistate->IntMode = INT_MSIX;
        else if(pci_find_capability(pdev, PCI_CAP_ID_MSI))
            pcistate->IntMode = INT_MSI;
        else
            pcistate->IntMode = INT_NONE;
    }
    else if((valb >= 1) && (valb <= 4))
        pcistate->IntMode = INT_LEGACY;
    else
        pcistate->IntMode = INT_NONE;

    if((pos = pci_find_capability(pdev, PCI_CAP_ID_EXP)))
    {
        /* Read Link Status */
        pci_read_config_word(pdev, pos+PCI_EXP_LNKSTA, &valw);
        pcistate->LinkSpeed = (valw & 0x0003);
        pcistate->LinkWidth = (valw & 0x03f0) >> 4;

        /* Read MPS & MRRS */
        pci_read_config_word(pdev, pos+PCI_EXP_DEVCTL, &valw);
        pcistate->MPS = 128 << ((valw & PCI_EXP_DEVCTL_PAYLOAD) >> 5);
        pcistate->MRRS = 128 << ((valw & PCI_EXP_DEVCTL_READRQ) >> 12);
    }
    else
    {
        printk("Cannot find PCI Express Capabilities\n");
        pcistate->LinkSpeed = pcistate->LinkWidth = 0;
        pcistate->MPS = pcistate->MRRS = 0;
    }

#if defined(K7_TRD)
    /* Read Initial Flow Control Credits information */
    base = (u32)(dmaData->barInfo[0].baseVAddr);
#if 0
    pcistate->InitFCCplD = XIo_In32(base+0x8210) & 0x00000FFF;
    pcistate->InitFCCplH = XIo_In32(base+0x8214) & 0x000000FF;
    pcistate->InitFCNPD  = XIo_In32(base+0x8218) & 0x00000FFF;
    pcistate->InitFCNPH  = XIo_In32(base+0x821C) & 0x000000FF;
    pcistate->InitFCPD   = XIo_In32(base+0x8220) & 0x00000FFF;
    pcistate->InitFCPH   = XIo_In32(base+0x8224) & 0x000000FF;
#endif

    pcistate->InitFCCplD = XIo_In32(base+0x901c) & 0x00000FFF;
    pcistate->InitFCCplH = XIo_In32(base+0x9020) & 0x000000FF;
    pcistate->InitFCNPD  = XIo_In32(base+0x9024) & 0x00000FFF;
    pcistate->InitFCNPH  = XIo_In32(base+0x9028) & 0x000000FF;
    pcistate->InitFCPD   = XIo_In32(base+0x902c) & 0x00000FFF;
    pcistate->InitFCPH   = XIo_In32(base+0x9030) & 0x000000FF;
   // printk("#####FCD %d FCH %d FCNPD %d FCNPH %d FCPD %d FCPH %d#####\n",pcistate->InitFCCplD,pcistate->InitFCCplH,pcistate->InitFCNPD,pcistate->InitFCNPH,pcistate->InitFCPD ,pcistate->InitFCPH);
    pcistate->Version    = XIo_In32(base+0x9000);
#endif

    return 0;
}

/********************************************************************/
/*  PCI probing function */
/********************************************************************/
static int __devinit xdma_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    int pciRet, chrRet;
    int i;
    dev_t xdmaDev;
    static struct file_operations xdmaDevFileOps;
    struct timer_list * timer = &poll_timer;

    /* Initialize device before it is used by driver. Ask low-level
     * code to enable I/O and memory. Wake up the device if it was
     * suspended. Beware, this function can fail.
     */
  pciRet = pci_enable_device(pdev);
  if (pciRet < 0)
    {
        printk(KERN_ERR "PCI device enable failed.\n");
        return pciRet;
    }

    /* Initialise packet pools for passing of packet arrays between this
     * and user drivers.
     */
    for(i=0; i<MAX_POOL; i++)
    {
        pktPool[i].pbuf = pktArray[i];      // Associate array with pool.

        if(i == (MAX_POOL-1))
            pktPool[i].next = NULL;
        else
            pktPool[i].next = &pktPool[i+1];
    }
    pktPoolTail = &pktPool[MAX_POOL-1];
    pktPoolHead = &pktPool[0];
#ifdef DEBUG_VERBOSE
    for(i=0; i<MAX_POOL; i++)
        printk("pktPool[%d] %p pktarray %p\n", i, &pktPool[i], pktPool[i].pbuf);
    printk("pktPoolHead %p pktPoolTail %p\n", pktPoolHead, pktPoolTail);
#endif
    /* Allocate space for holding driver-private data - for storing driver
     * context.
     */
    dmaData = kmalloc(sizeof(struct privData), GFP_KERNEL);
    if(dmaData == NULL)
    {
        printk(KERN_ERR "Unable to allocate DMA private data.\n");
        pci_disable_device(pdev);
        return XST_FAILURE;
    }
//printk("dmaData at %p\n", dmaData);
    dmaData->barMask = 0;
    dmaData->engineMask = 0;
    dmaData->userCount = 0;

#if defined(DEBUG_NORMAL) || defined(DEBUG_VERBOSE)
  /* Display PCI configuration space of device. */
  ReadConfig(pdev);
#endif

#ifdef DEBUG_VERBOSE
  /* Display PCI information on parent. */
  ReadRoot(pdev);
#endif

    /*
     * Enable bus-mastering on device. Calls pcibios_set_master() to do
     * the needed architecture-specific settings.
     */
  pci_set_master(pdev);

    /* Reserve PCI I/O and memory resources. Mark all PCI regions
     * associated with PCI device as being reserved by owner. Do not
     * access any address inside the PCI regions unless this call returns
     * successfully.
     */
    pciRet = pci_request_regions(pdev, DRIVER_NAME);
    if (pciRet < 0) {
        printk(KERN_ERR "Could not request PCI regions.\n");
        kfree(dmaData);
        pci_disable_device(pdev);
        return pciRet;
    }

    /* Returns success if PCI is capable of 32-bit DMA */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
    pciRet = pci_set_dma_mask(pdev, DMA_32BIT_MASK);
#else
    pciRet = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
#endif
    if (pciRet < 0) {
        printk(KERN_ERR "pci_set_dma_mask failed\n");
        pci_release_regions(pdev);
        kfree(dmaData);
        pci_disable_device(pdev);
        return pciRet;
    }

    /* First read all the BAR-related information. Then read all the
     * DMA engine information. Map the BAR region to the system only
     * when it is needed, for example, when a user requests it.
     */
    for(i=0; i<MAX_BARS; i++) {
        u32 size;

        /* Atleast BAR0 must be there. */
        if ((size = pci_resource_len(pdev, i)) == 0) {
            if (i == 0) {
                printk(KERN_ERR "BAR 0 not valid, aborting.\n");
                pci_release_regions(pdev);
                kfree(dmaData);
                pci_disable_device(pdev);
                return XST_FAILURE;
            }
            else
                continue;
        }
        /* Set a bitmask for all the BARs that are present. */
        else
            (dmaData->barMask) |= ( 1 << i );

        /* Check all BARs for memory-mapped or I/O-mapped. The driver is
         * intended to be memory-mapped.
         */
        if (!(pci_resource_flags(pdev, i) & IORESOURCE_MEM)) {
            printk(KERN_ERR "BAR %d is of wrong type, aborting.\n", i);
            pci_release_regions(pdev);
            kfree(dmaData);
            pci_disable_device(pdev);
            return XST_FAILURE;
        }

        /* Get base address of device memory and length for all BARs */
        dmaData->barInfo[i].basePAddr = pci_resource_start(pdev, i);
        dmaData->barInfo[i].baseLen = size;

        /* Map bus memory to CPU space. The ioremap may fail if size
         * requested is too long for kernel to provide as a single chunk
         * of memory, especially if users are sharing a BAR region. In
         * such a case, call ioremap for more number of smaller chunks
         * of memory. Or mapping should be done based on user request
         * with user size. Neither is being done now - maybe later.
         */
        if((dmaData->barInfo[i].baseVAddr =
            ioremap((dmaData->barInfo[i].basePAddr), size)) == 0UL)
        {
            printk(KERN_ERR "Cannot map BAR %d space, invalidating.\n", i);
            (dmaData->barMask) &= ~( 1 << i );
        }
        else
            log_verbose(KERN_INFO "[BAR %d] Base PA %x Len %d VA %x\n", i,
                (u32) (dmaData->barInfo[i].basePAddr),
                (u32) (dmaData->barInfo[i].baseLen),
                (u32) (dmaData->barInfo[i].baseVAddr));
    }
    log_verbose(KERN_INFO "Bar mask is 0x%x\n", (dmaData->barMask));
  log_normal(KERN_INFO "DMA Base VA %x\n",
                                (u32)(dmaData->barInfo[0].baseVAddr));

    /* Disable global interrupts */
    Dma_mIntDisable(dmaData->barInfo[0].baseVAddr);

    dmaData->pdev=pdev;
    dmaData->index = pdev->device;

    /* Initialize DMA common registers? !!!! */

    /* Read DMA engine configuration and initialise data structures */
    ReadDMAEngineConfiguration(pdev, dmaData);

    /* Save private data pointer in device structure */
  pci_set_drvdata(pdev, dmaData);

    /* The following code is for registering as a character device driver.
     * The GUI will use /dev/xdma_state file to read state & statistics.
     * Incase of any failure, the driver will come up without device
     * file support, but statistics will still be visible in the system log.
     */
    /* First allocate a major/minor number. */
    chrRet = alloc_chrdev_region(&xdmaDev, 0, 1, "xdma_stat");
    if(IS_ERR((int *)chrRet))
        log_normal(KERN_ERR "Error allocating char device region\n");
    else
    {
        /* Register our character device */
        xdmaCdev = cdev_alloc();
        if(IS_ERR(xdmaCdev))
        {
            log_normal(KERN_ERR "Alloc error registering device driver\n");
            unregister_chrdev_region(xdmaDev, 1);
            chrRet = -1;
        }
        else
        {
            xdmaDevFileOps.owner = THIS_MODULE;
            xdmaDevFileOps.open = xdma_dev_open;
            xdmaDevFileOps.release = xdma_dev_release;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
            xdmaDevFileOps.ioctl = xdma_dev_ioctl;
#else
            xdmaDevFileOps.unlocked_ioctl = xdma_dev_ioctl;
#endif
            xdmaCdev->owner = THIS_MODULE;
            xdmaCdev->ops = &xdmaDevFileOps;
            xdmaCdev->dev = xdmaDev;
            chrRet = cdev_add(xdmaCdev, xdmaDev, 1);
            if(chrRet < 0)
            {
                log_normal(KERN_ERR "Add error registering device driver\n");
                unregister_chrdev_region(xdmaDev, 1);
            }
        }
    }

    if(!IS_ERR((int *)chrRet))
    {
        printk(KERN_INFO "Device registered with major number %d\n",
                                            MAJOR(xdmaDev));
        /* Initialise all stats pointers */
        for(i=0; i<MAX_DMA_ENGINES; i++)
        {
            dstatsRead[i] = dstatsWrite[i] = dstatsNum[i] = 0;
            sstatsRead[i] = sstatsWrite[i] = sstatsNum[i] = 0;
            SWrate[i] = 0;
        }
        tstatsRead = tstatsWrite = tstatsNum = 0;

        /* Start stats polling routine */
        log_normal(KERN_INFO "probe: Starting stats poll routine with %x\n",
                                            (u32)pdev);
        /* Now start timer */
        init_timer(&stats_timer);
        stats_timer.expires=jiffies + HZ;
        stats_timer.data=(unsigned long) pdev;
        stats_timer.function = poll_stats;
        add_timer(&stats_timer);
    }

    DriverState = INITIALIZED;

    /* Start polling routine */
    log_normal(KERN_INFO "probe: Starting poll routine with %x\n", (u32)pdev);
    init_timer(timer);
    timer->expires=jiffies+(HZ/500);
    timer->data=(unsigned long) pdev;
    timer->function = poll_routine;
    add_timer(timer);

#ifdef TH_BH_ISR
    /* Now enable interrupts using MSI mode */
    if(!pci_enable_msi(pdev))
    {
        log_normal(KERN_INFO "MSI enabled\n");
        MSIEnabled = 1;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
    pciRet = request_irq(pdev->irq, DmaInterrupt, SA_SHIRQ, "xdma", pdev);
#else
    pciRet = request_irq(pdev->irq, DmaInterrupt, IRQF_SHARED, "xdma", pdev);
#endif

    if(pciRet)
    {
        printk(KERN_ERR "xdma could not allocate interrupt %d\n", pdev->irq);
        printk(KERN_ERR "Unload driver and try running with polled mode instead\n");
    }

    /* Set flag to synchronise between ISR and poll_routine */
    for(i=0; i<MAX_DMA_ENGINES; i++)
        LastIntr[i] = jiffies;

    /* Now, enable global interrupts. Engine interrupts will be enabled
     * only when they are used.
     */
    Dma_mIntEnable(dmaData->barInfo[0].baseVAddr);

#endif

    printk("Value of HZ is %d\n", HZ);
    printk("End of probe\n");

  return 0;
}

static void __devexit  xdma_remove(struct pci_dev *pdev)
{
  struct privData *lp;
    int i;
#ifdef TH_BH_ISR
    u32 girqval, base;
#endif

#ifdef FIFO_EMPTY_CHECK
    u32 barBase = (u32)(dmaData->barInfo[0].baseVAddr);  //todo: need to confirm that this will never change
    //printk("### value of 0x9008 ==> 0x%x", XIo_In32(barBase+0x9008));
#endif

printk("Came to xdma_remove\n");
    lp = pci_get_drvdata(pdev);

    /* The driver state flag has already been changed */

    mdelay(1000);

    /* Stop the polling routines */
    spin_lock_bh(&DmaStatsLock);
    del_timer_sync(&stats_timer);
    spin_unlock_bh(&DmaStatsLock);

    spin_lock_bh(&DmaLock);
    del_timer_sync(&poll_timer);
    spin_unlock_bh(&DmaLock);

#ifdef TH_BH_ISR
    base = (u32)(dmaData->barInfo[0].baseVAddr);
    Dma_mIntDisable(base);

    /* Disable MSI and interrupts */
    free_irq(pdev->irq, pdev);
    if(MSIEnabled) pci_disable_msi(pdev);
    girqval = Dma_mReadReg(base, REG_DMA_CTRL_STATUS);
    //if(girqval & (DMA_INT_ACTIVE_MASK|DMA_INT_PENDING_MASK|DMA_USER_INT_ACTIVE_MASK))
        //Dma_mWriteReg(base, REG_DMA_CTRL_STATUS, girqval);
    printk("While disabling interrupts, got %x\n", girqval);
#endif

#ifdef FIFO_EMPTY_CHECK
// should be done in a loop so that when more channels are added, code doesn't change.
    DmaFifoEmptyWait(HANDLE_0,DIR_TYPE_S2C);
    DmaFifoEmptyWait(HANDLE_1,DIR_TYPE_S2C);
    DmaFifoEmptyWait(HANDLE_0,DIR_TYPE_C2S);
    DmaFifoEmptyWait(HANDLE_1,DIR_TYPE_C2S);

    // wait for appropriate time to stabalize
    mdelay(STABILITY_WAIT_TIME);

#endif

    spin_lock_bh(&DmaLock);

#ifdef FIFO_EMPTY_CHECK
    // do axi-mig reset here.
    XIo_Out32(barBase + STATUS_REG_OFFSET, 1 << AXI_MIG_RST_SHIFT); //only this bit is writtable. so no need to read and mask.

#endif

    /* Reset DMA - this includes disabling interrupts and DMA. */
  log_normal(KERN_INFO "Doing DMA reset.\n");
    for(i=0; i<MAX_DMA_ENGINES; i++)
    {
        if((lp->engineMask) & (1LL << i))
            Dma_Reset(&(lp->Dma[i]));
    }

    for(i=0; i<MAX_BARS; i++)
    {
        if((dmaData->barMask) & ( 1 << i ))
            iounmap(dmaData->barInfo[i].baseVAddr);
    }
    spin_unlock_bh(&DmaLock);

    if(xdmaCdev != NULL)
    {
        printk("Unregistering char device driver\n");
        cdev_del(xdmaCdev);
        unregister_chrdev_region(xdmaCdev->dev, 1);
    }

  log_normal(KERN_INFO "PCI release regions and disable device.\n");
    pci_release_regions(pdev);
    pci_disable_device(pdev);
  pci_set_drvdata(pdev, NULL);
}

static int __init xdma_init(void)
{
  /* Initialize the locks */
  spin_lock_init(&DmaLock);
  spin_lock_init(&IntrLock);
  spin_lock_init(&DmaStatsLock);
  spin_lock_init(&PktPoolLock);

  /* Just register the driver. No kernel boot options used. */
  printk(KERN_INFO "XDMA: Inserting Xilinx base DMA driver in kernel.\n");
    return pci_register_driver(&xdma_driver);
}

static void __exit xdma_cleanup(void)
{
    struct PktPool * ppool;
    int oldstate;

    printk("Came to xdma_cleanup\n");

    /* First, change the driver state - so that other entry points
     * will not make a difference from this point on.
     */
    oldstate = DriverState;
    spin_lock_bh(&DmaLock);
    DriverState = UNREGISTERING;
    spin_unlock_bh(&DmaLock);

    /* Then, unregister driver with PCI in order to free up resources */
    pci_unregister_driver(&xdma_driver);

    if(dmaData != NULL)
    {
        printk("User count %d\n", dmaData->userCount);
        printk("GUI user open? %d\n", UserOpen);
        kfree(dmaData);
    }
    else
        printk("DriverState still %d\n", oldstate);

    /* Check whether pools are good */
    ppool = pktPoolHead;
    while (ppool != NULL)
    {
        log_verbose("pktPool %p pktarray %p\n", ppool, ppool->pbuf);
        ppool = ppool->next;
    }
    log_verbose("pktPoolHead %p pktPoolTail %p\n", pktPoolHead, pktPoolTail);

    printk(KERN_INFO "XDMA: Unregistering Xilinx base DMA driver from kernel.\n");
}

module_init(xdma_init);
module_exit(xdma_cleanup);

EXPORT_SYMBOL(DmaRegister);
EXPORT_SYMBOL(DmaUnregister);
#ifdef FIFO_EMPTY_CHECK
EXPORT_SYMBOL(DmaFifoEmptyWait);
#endif
EXPORT_SYMBOL(DmaSendPkt);

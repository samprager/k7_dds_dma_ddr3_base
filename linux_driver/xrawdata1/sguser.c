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
 * </pre>
 *
 *****************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#include <xdma_user.h>
#include "xdebug.h"
#include "xio.h"

/* Driver states */
#define UNINITIALIZED   0       /* Not yet come up */
#define INITIALIZED     1       /* But not yet set up for polling */
#define POLLING         2       /* But not yet registered with DMA */
#define REGISTERED      3       /* Registered with DMA */
#define CLOSED          4       /* Driver is being brought down */

/* DMA characteristics */
#define MYBAR           0

#ifdef XRAWDATA0
#define MYNAME   "Raw Data 0"
#define MYHANDLE  HANDLE_0
#else
#define MYNAME   "Raw Data 1"
#define MYHANDLE  HANDLE_1
#endif

#ifdef XRAWDATA0

#define TX_CONFIG_ADDRESS       0x9108
#define RX_CONFIG_ADDRESS       0x9100
#define PKT_SIZE_ADDRESS        0x9104
#define STATUS_ADDRESS          0x910C


/* Test start / stop conditions */
#define LOOPBACK            0x00000002  /* Enable TX data loopback onto RX */
//#define LOOPBACK            0x0001

/* Link status conditions */
//#define RX_LINK_UP          0x00000080  /**< RX link up / down */
//#define RX_ALIGNED          0x00000040  /**< RX link aligned */

#else

#define TX_CONFIG_ADDRESS   0x9208  /* Reg for controlling TX data */
#define RX_CONFIG_ADDRESS   0x9200  /* Reg for controlling RX pkt generator */
#define PKT_SIZE_ADDRESS    0x9204  /* Reg for programming packet size */
#define STATUS_ADDRESS      0x920C  /* Reg for checking TX pkt checker status */

/* Test start / stop conditions */
#define LOOPBACK            0x00000002  /* Enable TX data loopback onto RX */
#endif

/* Test start / stop conditions */
#define PKTCHKR             0x00000001  /* Enable TX packet checker */
#define PKTGENR             0x00000001  /* Enable RX packet generator */
#define CHKR_MISMATCH       0x00000001  /* TX checker reported data mismatch */

#ifdef XRAWDATA0
#define ENGINE_TX       0
#define ENGINE_RX       32
#else
#define ENGINE_TX       1
#define ENGINE_RX       33
#endif

/* Packet characteristics */
#define BUFSIZE         (PAGE_SIZE)

#ifdef XRAWDATA0
#define MAXPKTSIZE      (8*PAGE_SIZE)
#else
#define MAXPKTSIZE      (8*PAGE_SIZE)
#endif

#define MINPKTSIZE      (64)
#define NUM_BUFS        4000
#define BUFALIGN        8
#define BYTEMULTIPLE    8   /**< Lowest sub-multiple of memory path */


#define START_ADRS_0  0x9300
#define START_ADRS_1  0x9310
#define START_ADRS_2  0x9320
#define START_ADRS_3  0x9330

#define END_ADRS_0  0x9304
#define END_ADRS_1  0x9314
#define END_ADRS_2  0x9324
#define END_ADRS_3  0x9334

#define WRBURST_0 0x9308
#define WRBURST_1 0x9318
#define WRBURST_2 0x9318
#define WRBURST_3 0x9318

#define RDBURST_0 0x930C
#define RDBURST_1 0x931C
#define RDBURST_2 0x931C
#define RDBURST_3 0x931C


#define BURST_SIZE  64


int DriverState = UNINITIALIZED;
struct timer_list poll_timer;
void * handle[4] = {NULL, NULL, NULL, NULL};
u32 TXbarbase, RXbarbase;
u32 polldata = 0xaa55;
u32 RawTestMode = TEST_STOP;
u32 RawMinPktSize=MINPKTSIZE, RawMaxPktSize=MAXPKTSIZE;

typedef struct {
    int TotalNum;
    int AllocNum;
    int FirstBuf;
    int LastBuf;
    int FreePtr;
    int AllocPtr;
    unsigned char * origVA[NUM_BUFS];
} Buffer;

Buffer TxBufs;
Buffer RxBufs;
PktBuf pkts[NUM_BUFS];

/* For exclusion */
spinlock_t RawLock;

#ifdef XRAWDATA0
#define DRIVER_NAME         "xrawdata0_driver"
#define DRIVER_DESCRIPTION  "Xilinx Raw Data0 Driver "
#else
#define DRIVER_NAME         "xrawdata1_driver"
#define DRIVER_DESCRIPTION  "Xilinx Raw Data1 Driver"
#endif

static void poll_routine(unsigned long __opaque);
static void InitBuffers(Buffer * bptr);

static void FormatBuffer(unsigned char * buf, int pktsize, int bufsize, int fragment);
#ifdef DATA_VERIFY
static void VerifyBuffer(unsigned char * buf, int size, unsigned long long uinfo);
#endif

int myInit(unsigned int, unsigned int);
int myFreePkt(void *, unsigned int *, int, unsigned int);
static int DmaSetupTransmit(void *, int);
int myGetRxPkt(void *, PktBuf *, unsigned int, int, unsigned int);
int myPutTxPkt(void *, PktBuf *, int, unsigned int);
int myPutRxPkt(void *, PktBuf *, int, unsigned int);
int mySetState(void * hndl, UserState * ustate, unsigned int privdata);
int myGetState(void * hndl, UserState * ustate, unsigned int privdata);

extern unsigned int CRC(unsigned int * buf, int len);

/* For checking data integrity */
unsigned int TxBufCnt=0;
unsigned int RxBufCnt=0;
unsigned int ErrCnt=0;

unsigned short TxSeqNo = 0;
unsigned short RxSeqNo = 0;

/* Simplistic buffer management algorithm. Buffers must be freed in the
 * order in which they have been allocated. Out of order buffer frees will
 * result in the wrong buffer being freed and may cause a system hang during
 * the next buffer/DMA access.
 */
static void InitBuffers(Buffer * bptr)
{
    unsigned char * bufVA;
    int i;

    /* Initialise */
    bptr->TotalNum = bptr->AllocNum = 0;
    bptr->FirstBuf = 0;
    bptr->LastBuf = 0;
    bptr->FreePtr = 0;
    bptr->AllocPtr = 0;

    /* Allocate for TX buffer pool - have not taken care of alignment */
    for(i = 0; i < NUM_BUFS; i++)
    {
        if((bufVA = (unsigned char *)__get_free_pages(GFP_KERNEL, get_order(BUFSIZE))) == NULL)
        {
            printk("InitBuffers: Unable to allocate [%d] TX buffer for data\n", i);
            break;
        }
        bptr->origVA[i] = bufVA;
    }
    printk("Allocated %d buffers from %p\n", i, (void *)bufVA);

#if 0
    /* Do the buffer alignment adjustment, if required */
    if((u32)bufVA % BUFALIGN)
        bufVA += (BUFALIGN - ((u32)bufVA & 0xFF));
#endif

    if(i)
    {
        bptr->FirstBuf = 0;
        bptr->LastBuf = i;      // One more than last valid buffer.
        bptr->FreePtr = 0;
        bptr->AllocPtr = 0;
        bptr->TotalNum = i;
        bptr->AllocNum = 0;
    }
    //printk("Buffers allocated first %x last %x, number %d\n",
    //                (u32)(bptr->origVA[0]), (u32)(bptr->origVA[i-1]), i);
}

static inline unsigned char * AllocBuf(Buffer * bptr)
{
    unsigned char * cptr;
    int freeptr;

    if(bptr->AllocNum == bptr->TotalNum)
    {
        //printk("No buffers available to allocate\n");
        return NULL;
    }

    freeptr = bptr->FreePtr;

    //printk("Before allocating:\n");
    //printk("Allocated %d buffers\n", bptr->AllocNum);
    //printk("FreePtr %d AllocPtr %d\n", bptr->FreePtr, bptr->AllocPtr);

    cptr = bptr->origVA[freeptr];
    bptr->FreePtr ++;
    if(bptr->FreePtr == bptr->LastBuf)
        bptr->FreePtr = 0;
    bptr->AllocNum++;

    //printk("After allocating:\n");
    //printk("Allocated %d buffers\n", bptr->AllocNum);
    //printk("FreePtr %d AllocPtr %d\n", bptr->FreePtr, bptr->AllocPtr);

    return cptr;
}

static inline void FreeUsedBuf(Buffer * bptr, int num)
{
    /* This version differs from FreeUnusedBuf() in that this frees used
     * buffers by moving the AllocPtr, while the other frees unused buffers
     * by moving the FreePtr.
     */
    int i;

    //printk("Trying to free %d buffers\n", num);
    //printk("Allocated %d buffers\n", bptr->AllocNum);
    //printk("FreePtr %d AllocPtr %d\n", bptr->FreePtr, bptr->AllocPtr);

    for(i=0; i<num; i++)
    {
        if(bptr->AllocNum == 0)
        {
            printk("No more buffers allocated, cannot free anything\n");
            break;
        }

        bptr->AllocPtr ++;
        if(bptr->AllocPtr == bptr->LastBuf)
            bptr->AllocPtr = 0;
        bptr->AllocNum--;
    }
    //printk("After freeing:\n");
    //printk("Allocated %d buffers\n", bptr->AllocNum);
    //printk("FreePtr %d AllocPtr %d\n", bptr->FreePtr, bptr->AllocPtr);
}

static inline void FreeUnusedBuf(Buffer * bptr, int num)
{
    /* This version differs from FreeUsedBuf() in that this frees unused buffers
     * by moving the FreePtr, while the other frees used buffers by moving
     * the AllocPtr.
     */
    int i;

    //printk("Trying to free %d unused buffers\n", num);
    //printk("Allocated %d buffers\n", bptr->AllocNum);
    //printk("FreePtr %d AllocPtr %d\n", bptr->FreePtr, bptr->AllocPtr);

    for(i=0; i<num; i++)
    {
        if(bptr->AllocNum == 0)
        {
            printk("No more buffers allocated, cannot free anything\n");
            break;
        }

        if(bptr->FreePtr == 0)
            bptr->FreePtr = bptr->LastBuf;
        bptr->FreePtr --;
        bptr->AllocNum--;
    }
    //printk("After freeing:\n");
    //printk("Allocated %d buffers\n", bptr->AllocNum);
    //printk("FreePtr %d AllocPtr %d\n", bptr->FreePtr, bptr->AllocPtr);
}

static inline void PrintSummary(void)
{
    u32 val;

    printk("---------------------------------------------------\n");
    printk("%s Driver results Summary:-\n", MYNAME);
    printk("Current Run Min Packet Size = %d, Max Packet Size = %d\n",
                            RawMinPktSize, RawMaxPktSize);
    printk("Buffers Transmitted = %u, Buffers Received = %u, Error Count = %u\n", TxBufCnt, RxBufCnt, ErrCnt);
    printk("TxSeqNo = %u, RxSeqNo = %u\n", TxSeqNo, RxSeqNo);

    val = XIo_In32(TXbarbase+STATUS_ADDRESS);
    printk("Data Mismatch Status = %x\n", val);

    printk("---------------------------------------------------\n");
}

static void FormatBuffer(unsigned char * buf, int pktsize, int bufsize, int fragment)
{
    int i;

    /* Apply data pattern in the buffer */
    for(i = 0; i < bufsize - 1; i = i+2)
        *(unsigned short *)(buf + i) = TxSeqNo;

    /* Update header information for the first fragment in packet */
    if(!fragment)
    {
        /* Apply packet length and sequence number */
        *(unsigned short *)(buf + 0) = pktsize;
        *(unsigned short *)(buf + 2) = TxSeqNo;
    }
  // else
  //  printk("TX Buffer has:Size %d  \n",bufsize);

   if(bufsize % 2)
    {
        *(unsigned short *)(buf + bufsize - 1) = TxSeqNo;
    }

#ifdef DEBUG_VERBOSE
    printk("TX Buffer has:\n");
    for(i=0; i<bufsize; i++)
    {
        if(!(i % 32)) printk("\n");
        printk("%02x ", buf[i]);
    }
    printk("\n");
#endif
}

#ifdef DATA_VERIFY
static void VerifyBuffer(unsigned char * buf, int size, unsigned long long uinfo)
{
    unsigned short check2;
    unsigned char * bptr;

  int check_bit = 2;

  if (size < 2)
    return;

  if(size%2)
    check_bit += 1;

#ifdef DEBUG_VERBOSE
    int i;

    printk("%s: RX packet len %d uinfo %llx\n", MYNAME, size, uinfo);
    for(i=0; i<size; i++)
    {
        if(i && !(i % 16)) printk("\n");
        printk("%02x ", buf[i]);
    }
    printk("\n");
#endif

    bptr = buf+size;

    check2 = *(unsigned short *)(bptr-check_bit);

    if(check2 != RxSeqNo)
    {
        ErrCnt++;
        printk("Mismatch: Size %x SeqNo %x uinfo %x, buf check2 %x  bit %x\n",
                        size, (*(unsigned short *)(buf+2)),
                        (unsigned int)uinfo, check2,check_bit);
        printk("RxSeqNo %x\n", RxSeqNo);

        /* Update RxSeqNo */
        RxSeqNo = check2;
    }
}
#endif

static void poll_routine(unsigned long __opaque)
{
    struct timer_list *timer = &poll_timer;
    UserPtrs ufuncs;
    int offset = (2*HZ);

    //printk("Came to poll_routine with %x\n", (u32)(__opaque));

    /* Register with DMA incase not already done so */
    if(DriverState < POLLING)
    {
        spin_lock_bh(&RawLock);
        printk("Calling DmaRegister on engine %d and %d\n",
                            ENGINE_TX, ENGINE_RX);
        DriverState = REGISTERED;

        ufuncs.UserInit = myInit;
        ufuncs.UserPutPkt = myPutTxPkt;
        ufuncs.UserSetState = mySetState;
        ufuncs.UserGetState = myGetState;
        ufuncs.privData = 0x54545454;
        spin_unlock_bh(&RawLock);

#ifdef FIFO_EMPTY_CHECK
        // no need, just if h/w is not in good shape, we will see messages in dmesg
        DmaFifoEmptyWait(MYHANDLE,DIR_TYPE_S2C);
#endif

        if((handle[0]=DmaRegister(ENGINE_TX, MYBAR, &ufuncs, BUFSIZE)) == NULL)
        {
            printk("Register for engine %d failed. Stopping.\n", ENGINE_TX);
            spin_lock_bh(&RawLock);
            DriverState = UNINITIALIZED;
            spin_unlock_bh(&RawLock);
            return;     // This polling will not happen again.
        }
        printk("Handle for engine %d is %p\n", ENGINE_TX, handle[0]);

        spin_lock_bh(&RawLock);
        ufuncs.UserInit = myInit;
        ufuncs.UserPutPkt = myPutRxPkt;
        ufuncs.UserGetPkt = myGetRxPkt;
        ufuncs.UserSetState = mySetState;
        ufuncs.UserGetState = myGetState;
        ufuncs.privData = 0x54545456;
        spin_unlock_bh(&RawLock);

#ifdef FIFO_EMPTY_CHECK
        // no need, just if h/w is not in good shape, we will see messages in dmesg
        DmaFifoEmptyWait(MYHANDLE,DIR_TYPE_C2S);
#endif
        if((handle[2]=DmaRegister(ENGINE_RX, MYBAR, &ufuncs, BUFSIZE)) == NULL)
        {
            printk("Register for engine %d failed. Stopping.\n", ENGINE_RX);
            spin_lock_bh(&RawLock);
            DriverState = UNINITIALIZED;
            spin_unlock_bh(&RawLock);
            return;     // This polling will not happen again.
        }
        printk("Handle for engine %d is %p\n", ENGINE_RX, handle[2]);

        /* Reschedule poll routine */
        timer->expires = jiffies + offset;
        add_timer(timer);
    }
    else if(DriverState == REGISTERED)
    {
        /* Only if the test mode is set, and only for TX direction */
        if((RawTestMode & TEST_START) &&
           (RawTestMode & (ENABLE_PKTCHK|ENABLE_LOOPBACK)))
        {
            spin_lock_bh(&RawLock);
            /* First change the state */
            RawTestMode &= ~TEST_START;
            RawTestMode |= TEST_IN_PROGRESS;
            spin_unlock_bh(&RawLock);

            /* Now, queue up one packet to start the transmission */
            DmaSetupTransmit(handle[0], 1);

            /* For the first packet, give some gap before the next TX */
            offset = HZ;
        }
        else if(RawTestMode & TEST_IN_PROGRESS)
        {
            int avail;
            int times;

            for(times = 0; times < 9; times++)
            {
                avail = (TxBufs.TotalNum - TxBufs.AllocNum);
                if(avail <= 0) break;

                /* Queue up many packets to continue the transmission */
                if(DmaSetupTransmit(handle[0], 100) == 0) break;
            }

            /* Do the next TX as soon as possible */
            offset = 0;
        }

        /* Reschedule poll routine */
        timer->expires = jiffies + offset;
        add_timer(timer);
    }
}

int myInit(unsigned int barbase, unsigned int privdata)
{
    log_normal("Reached myInit with barbase %x and privdata %x\n",
                barbase, privdata);

    spin_lock_bh(&RawLock);
    if(privdata == 0x54545454)  // So that this is done only once
    {
        TXbarbase = barbase;
    }
    else if(privdata == 0x54545456)  // So that this is done only once
    {
        RXbarbase = barbase;
    }
    TxBufCnt = 0; RxBufCnt = 0;
    ErrCnt = 0;
    TxSeqNo = RxSeqNo = 0;

    /* Stop any running tests. The driver could have been unloaded without
     * stopping running tests the last time. Hence, good to reset everything.
     */
    XIo_Out32(TXbarbase+TX_CONFIG_ADDRESS, 0);
    XIo_Out32(TXbarbase+RX_CONFIG_ADDRESS, 0);

    spin_unlock_bh(&RawLock);

    return 0;
}

int myPutRxPkt(void * hndl, PktBuf * vaddr, int numpkts, unsigned int privdata)
{
    int i, unused=0;
    unsigned int flags;

    //printk("Reached myPutRxPkt with handle %p, VA %x, size %d, privdata %x\n",
    //            hndl, (u32)vaddr, size, privdata);

    /* Check driver state */
    if(DriverState != REGISTERED)
    {
        printk("Driver does not seem to be ready\n");
        return -1;
    }

    /* Check handle value */
    if(hndl != handle[2])
    {
        log_normal("Came with wrong handle %x\n", (u32)hndl);
        return -1;
    }

    spin_lock_bh(&RawLock);

    for(i=0; i<numpkts; i++)
    {
        flags = vaddr->flags;
        //printk("RX pkt flags %x\n", flags);
        if(flags & PKT_UNUSED)
        {
            unused = 1;
            break;
        }
        if(flags & PKT_EOP)
        {
#ifdef DATA_VERIFY
            VerifyBuffer((unsigned char *)(vaddr->bufInfo), (vaddr->size), (vaddr->userInfo));
#endif
            RxSeqNo++;
        }
        RxBufCnt++;
        vaddr++;
    }

    /* Return packet buffers to free pool */

    //printk("PutRxPkt: Freeing %d packets unused %d\n", numpkts, unused);
    if(unused)
        FreeUnusedBuf(&RxBufs, numpkts);
    else
        FreeUsedBuf(&RxBufs, numpkts);

    spin_unlock_bh(&RawLock);

    return 0;
}

int myGetRxPkt(void * hndl, PktBuf * vaddr, unsigned int size, int numpkts, unsigned int privdata)
{
    unsigned char * bufVA;
    PktBuf * pbuf;
    int i;

    //printk(KERN_INFO "myGetRxPkt: Came with handle %p size %d privdata %x\n",
    //                        hndl, size, privdata);

    /* Check driver state */
    if(DriverState != REGISTERED)
    {
        printk("Driver does not seem to be ready\n");
        return 0;
    }

    /* Check handle value */
    if(hndl != handle[2])
    {
        printk("Came with wrong handle\n");
        return 0;
    }

    /* Check size value */
    if(size != BUFSIZE)
        printk("myGetRxPkt: Requested size %d does not match mine %d\n",
                                size, (u32)BUFSIZE);

    spin_lock_bh(&RawLock);

    for(i=0; i<numpkts; i++)
    {
        pbuf = &(vaddr[i]);
        /* Allocate a buffer. DMA driver will map to PCI space. */
        bufVA = AllocBuf(&RxBufs);
        log_verbose(KERN_INFO "myGetRxPkt: The buffer after alloc is at address %x size %d\n",
                            (u32) bufVA, (u32) BUFSIZE);
        if (bufVA == NULL)
        {
            log_normal(KERN_ERR "RX: AllocBuf failed\n");
            break;
        }

        pbuf->pktBuf = bufVA;
        pbuf->bufInfo = bufVA;
        pbuf->size = BUFSIZE;
    }
    spin_unlock_bh(&RawLock);

    log_verbose(KERN_INFO "Requested %d, allocated %d buffers\n", numpkts, i);
    return i;
}

int myPutTxPkt(void * hndl, PktBuf * vaddr, int numpkts, unsigned int privdata)
{
    int nomore=0;
    int i;
    unsigned int flags;

    log_verbose(KERN_INFO "Reached myPutTxPkt with handle %p, numpkts %d, privdata %x\n",
                hndl, numpkts, privdata);

    /* Check driver state */
    if(DriverState != REGISTERED)
    {
        printk("Driver does not seem to be ready\n");
        return -1;
    }

    /* Check handle value */
    if(hndl != handle[0])
    {
        printk("Came with wrong handle\n");
        return -1;
    }

    /* Just check if we are on the way out */
    for(i=0; i<numpkts; i++)
    {
        flags = vaddr->flags;
        //printk("TX pkt flags %x\n", flags);
        if(flags & PKT_UNUSED)
        {
            nomore = 1;
            break;
        }
        vaddr++;
    }

    spin_lock_bh(&RawLock);

    /* Return packet buffer to free pool */
    //printk("PutTxPkt: Freed %d packets nomore %d\n", numpkts, nomore);
    if(nomore)
        FreeUnusedBuf(&TxBufs, numpkts);
    else
        FreeUsedBuf(&TxBufs, numpkts);
    spin_unlock_bh(&RawLock);

    return 0;
}

int mySetState(void * hndl, UserState * ustate, unsigned int privdata)
{
    int val;
    static unsigned int testmode;

    log_verbose(KERN_INFO "Reached mySetState with privdata %x\n", privdata);

    /* Check driver state */
    if(DriverState != REGISTERED)
    {
        printk("Driver does not seem to be ready\n");
        return EFAULT;
    }

    /* Check handle value */
    if((hndl != handle[0]) && (hndl != handle[2]))
    {
        printk("Came with wrong handle\n");
        return EBADF;
    }

    /* Valid only for TX engine */
    if(privdata == 0x54545454)
    {
        spin_lock_bh(&RawLock);

        /* Set up the value to be written into the register */
        RawTestMode = ustate->TestMode;

        if(RawTestMode & TEST_START)
        {
            testmode = 0;
            if(RawTestMode & ENABLE_LOOPBACK) testmode |= LOOPBACK;
            if(RawTestMode & ENABLE_PKTCHK) testmode |= PKTCHKR;
            if(RawTestMode & ENABLE_PKTGEN) testmode |= PKTGENR;
        }
        else
        {
            /* Deliberately not clearing the loopback bit, incase a
             * loopback test was going on - allows the loopback path
             * to drain off packets. Just stopping the source of packets.
             */
            if(RawTestMode & ENABLE_PKTCHK) testmode &= ~PKTCHKR;
            if(RawTestMode & ENABLE_PKTGEN) testmode &= ~PKTGENR;
        }

        printk("SetState TX with RawTestMode %x, reg value %x\n",
                                                    RawTestMode, testmode);

        /* Now write the registers */
        if(RawTestMode & TEST_START)
        {
            if(!(RawTestMode & (ENABLE_PKTCHK|ENABLE_PKTGEN|ENABLE_LOOPBACK)))
            {
                printk("%s Driver: TX Test Start with wrong mode %x\n",
                                                MYNAME, testmode);
                RawTestMode = 0;
                spin_unlock_bh(&RawLock);
                return EBADRQC;
            }

            printk("%s Driver: Starting the test - mode %x, reg %x\n",
                                            MYNAME, RawTestMode, testmode);

            /* Next, set packet sizes. Ensure they don't exceed PKTSIZEs */
            RawMinPktSize = ustate->MinPktSize;
            RawMaxPktSize = ustate->MaxPktSize;

            /* Set RX packet size for memory path */
            val = RawMaxPktSize;
            if(val % BYTEMULTIPLE)
      {
        printk("********** ODD PACKET SIZE **********\n");
              //val -= (val % BYTEMULTIPLE);
      }
            printk("Reg %x = %x\n", PKT_SIZE_ADDRESS, val);
            RawMinPktSize = RawMaxPktSize = val;

            /* Now ensure the sizes remain within bounds */
            if(RawMaxPktSize > MAXPKTSIZE)
                RawMinPktSize = RawMaxPktSize = MAXPKTSIZE;
            if(RawMinPktSize < MINPKTSIZE)
                RawMinPktSize = RawMaxPktSize = MINPKTSIZE;
            if(RawMinPktSize > RawMaxPktSize)
                RawMinPktSize = RawMaxPktSize;
            val = RawMaxPktSize;

            printk("========Reg %x = %d\n", PKT_SIZE_ADDRESS, val);
            XIo_Out32(TXbarbase+PKT_SIZE_ADDRESS, val);
            printk("RxPktSize %d\n", val);

/*
      #ifdef XRAWDATA0
        XIo_Out32(TXbarbase+START_ADRS_0, 0x00000000);
        XIo_Out32(TXbarbase+START_ADRS_1, 0x04000000);
        XIo_Out32(TXbarbase+END_ADRS_0, 0x03000000);
        XIo_Out32(TXbarbase+END_ADRS_1, 0x07000000);
        XIo_Out32(TXbarbase+WRBURST_0, BURST_SIZE );
        XIo_Out32(TXbarbase+RDBURST_0, BURST_SIZE );
        XIo_Out32(TXbarbase+WRBURST_1, BURST_SIZE );
        XIo_Out32(TXbarbase+RDBURST_1, BURST_SIZE );
      #else
        XIo_Out32(TXbarbase+START_ADRS_2, 0x08000000);
        XIo_Out32(TXbarbase+START_ADRS_3, 0x0C000000);
        XIo_Out32(TXbarbase+END_ADRS_2, 0x0B000000);
        XIo_Out32(TXbarbase+END_ADRS_3, 0x0F000000);
        XIo_Out32(TXbarbase+WRBURST_2, BURST_SIZE );
        XIo_Out32(TXbarbase+RDBURST_2, BURST_SIZE );
        XIo_Out32(TXbarbase+WRBURST_3, BURST_SIZE );
        XIo_Out32(TXbarbase+RDBURST_3, BURST_SIZE );
      #endif
*/

/* Incase the last test was a loopback test, that bit may not be cleared. */
            XIo_Out32(TXbarbase+TX_CONFIG_ADDRESS, 0);
            if(RawTestMode & (ENABLE_PKTCHK|ENABLE_LOOPBACK))
            {
                TxSeqNo = 0;
                if(RawTestMode & ENABLE_LOOPBACK)
                    RxSeqNo = 0;
                printk("========Reg %x = %x\n", TX_CONFIG_ADDRESS, testmode);
                XIo_Out32(TXbarbase+TX_CONFIG_ADDRESS, testmode);
            }
            if(RawTestMode & ENABLE_PKTGEN)
            {
                RxSeqNo = 0;
                printk("========Reg %x = %x\n", RX_CONFIG_ADDRESS, testmode);
                XIo_Out32(TXbarbase+RX_CONFIG_ADDRESS, testmode);
            }

        }
        /* Else, stop the test. Do not remove any loopback here because
         * the DMA queues and hardware FIFOs must drain first.
         */
        else
        {
            printk("%s Driver: Stopping the test, mode %x\n", MYNAME, testmode);
            printk("========Reg %x = %x\n", TX_CONFIG_ADDRESS, testmode);
            XIo_Out32(TXbarbase+TX_CONFIG_ADDRESS, testmode);
            printk("========Reg %x = %x\n", RX_CONFIG_ADDRESS, testmode);
            XIo_Out32(TXbarbase+RX_CONFIG_ADDRESS, testmode);

            /* Not resetting sequence numbers here - causes problems
             * in debugging. Instead, reset the sequence numbers when
             * starting a test.
             */
        }

        PrintSummary();
        spin_unlock_bh(&RawLock);
    }

    return 0;
}

int myGetState(void * hndl, UserState * ustate, unsigned int privdata)
{
    static int iter=0;

    log_verbose("Reached myGetState with privdata %x\n", privdata);

    /* Same state is being returned for both engines */

    ustate->LinkState = LINK_UP;
    ustate->Errors = 0;
    ustate->MinPktSize = RawMinPktSize;
    ustate->MaxPktSize = RawMaxPktSize;
    ustate->TestMode = RawTestMode;
    if(privdata == 0x54545454)
        ustate->Buffers = TxBufs.TotalNum;
    else
        ustate->Buffers = RxBufs.TotalNum;

    if(iter++ >= 4)
    {
        PrintSummary();

        iter = 0;
    }

    return 0;
}

static int DmaSetupTransmit(void * hndl, int num)
{
    int i, result;
    static int pktsize=0;
    int total, bufsize, fragment;
    int bufindex;
    unsigned char * bufVA;
    PktBuf * pbuf;
    int origseqno;
    //static unsigned short lastno=0;

    log_verbose("Reached DmaSetupTransmit with handle %p, num %d\n", hndl, num);

    /* Check driver state */
    if(DriverState != REGISTERED)
    {
        printk("Driver does not seem to be ready\n");
        return 0;
    }

    /* Check handle value */
    if(hndl != handle[0])
    {
        printk("Came with wrong handle\n");
        return 0;
    }

    /* Check number of packets */
    if(!num)
    {
        printk("Came with 0 packets for sending\n");
        return 0;
    }

    /* Hold the spinlock only when calling the buffer management APIs. */
    spin_lock_bh(&RawLock);
    origseqno = TxSeqNo;
    for(i=0, bufindex=0; i<num; i++)            /* Total packets loop */
    {
        //printk("i %d bufindex %d\n", i, bufindex);

            /* Fix the packet size to be the maximum entered in GUI */
            pktsize = RawMaxPktSize;

        //printk("pktsize is %d\n", pktsize);

        total = 0;
        fragment = 0;
        while(total < pktsize)      /* Packet fragments loop */
        {
            //printk("Buf loop total %d bufindex %d\n", total, bufindex);

            pbuf = &(pkts[bufindex]);

            /* Allocate a buffer. DMA driver will map to PCI space. */
            bufVA = AllocBuf(&TxBufs);

            log_verbose(KERN_INFO "TX: The buffer after alloc is at address %x size %d\n",
                                (u32) bufVA, (u32) BUFSIZE);
            if (bufVA == NULL)
            {
                //printk("TX: AllocBuf failed\n");
                //printk("[%d]", (num-i-1));
                break;
            }
            pbuf->pktBuf = bufVA;
            pbuf->bufInfo = bufVA;
            bufsize = ((total + BUFSIZE) > pktsize) ?
                                        (pktsize - total) : BUFSIZE ;
            total += bufsize;

            //printk("bufsize %d total %d\n", bufsize, total);

            log_verbose(KERN_INFO "Calling FormatBuffer pktsize %d bufsize %d fragment %d\n",
                                pktsize, bufsize, fragment);
            FormatBuffer(bufVA, pktsize, bufsize, fragment);

            pbuf->size = bufsize;
            pbuf->userInfo = TxSeqNo;
            pbuf->flags = PKT_ALL;
            if(!fragment)
                pbuf->flags |= PKT_SOP;
            if(total == pktsize)
            {
                pbuf->flags |= PKT_EOP;
                pbuf->size = bufsize;
            }

            //printk("flags %x\n", pbuf->flags);
            //printk("TxSeqNo %u\n", TxSeqNo);

            bufindex++;
            fragment++;
        }
        if(total < pktsize)
        {
            /* There must have been some error in the middle of the packet */

            if(fragment)
            {
                /* First, adjust the number of buffers to queue up or else
                 * partial packets will get transmitted, which will cause a
                 * problem later.
                 */
                bufindex -= fragment;

                /* Now, free any unused buffers from the partial packet, so
                 * that buffers are not lost.
                 */
                log_normal(KERN_ERR "Tried to send pkt of size %d, only %d fragments possible\n",
                                                pktsize, fragment);
                FreeUnusedBuf(&TxBufs, fragment);
            }
            break;
        }


        /* Increment packet sequence number */
        //if(lastno != TxSeqNo) printk(" %u-%u.", lastno, TxSeqNo);
        TxSeqNo++;
        //lastno = TxSeqNo;
    }
    spin_unlock_bh(&RawLock);

    //printk("[p%d-%d-%d] ", num, i, bufindex);

    if(i == 0)
        /* No buffers available */
        return 0;

    log_verbose("%s: Sending packet length %d seqno %d\n",
                                        MYNAME, pktsize, TxSeqNo);
    result = DmaSendPkt(hndl, pkts, bufindex);
    TxBufCnt += result;
    if(result != bufindex)
    {
        log_normal(KERN_ERR "Tried to send %d pkts in %d buffers, sent only %d\n",
                                    num, bufindex, result);
        //printk("[s%d-%d,%d-%d]", bufindex, result, TxSeqNo, origseqno);
        if(result) TxSeqNo = pkts[result].userInfo;
        else TxSeqNo = origseqno;
        //printk("-%u-", TxSeqNo);
        //lastno = TxSeqNo;

        spin_lock_bh(&RawLock);
        FreeUnusedBuf(&TxBufs, (bufindex-result));
        spin_unlock_bh(&RawLock);
        return 0;
    }
    else return 1;
}

static int __init rawdata_init(void)
{
  /* Just register the driver. No kernel boot options used. */
  printk(KERN_INFO "%s Init: Inserting Xilinx driver in kernel.\n",
                                        MYNAME);

    DriverState = INITIALIZED;
    spin_lock_init(&RawLock);

    /* First allocate the buffer pool and set the driver state
     * because GetPkt routine can potentially be called immediately
     * after Register is done.
     */
    //printk("PAGE_SIZE is %ld\n", PAGE_SIZE);
    msleep(5);

    InitBuffers(&TxBufs);
    InitBuffers(&RxBufs);

    /* Start polling routine - change later !!!! */
    printk(KERN_INFO "%s Init: Starting poll routine with %x\n",
                                        MYNAME, polldata);
    init_timer(&poll_timer);
    poll_timer.expires=jiffies+(HZ/5);
    poll_timer.data=(unsigned long) polldata;
    poll_timer.function = poll_routine;
    add_timer(&poll_timer);

    return 0;
}

static void __exit rawdata_cleanup(void)
{
    int i;

    /* Stop the polling routine */
    del_timer_sync(&poll_timer);
    //DriverState = CLOSED;

    /* Stop any running tests, else the hardware's packet checker &
     * generator will continue to run.
     */
    XIo_Out32(TXbarbase+TX_CONFIG_ADDRESS, 0);

    XIo_Out32(TXbarbase+RX_CONFIG_ADDRESS, 0);

    printk(KERN_INFO "%s: Unregistering Xilinx driver from kernel.\n", MYNAME);
    if (TxBufCnt != RxBufCnt)
    {
        printk("%s: Buffers Transmitted %u Received %u\n", MYNAME, TxBufCnt, RxBufCnt);
        printk("TxSeqNo = %u, RxSeqNo = %u\n", TxSeqNo, RxSeqNo);
        mdelay(1);
    }
#ifdef FIFO_EMPTY_CHECK
    DmaFifoEmptyWait(MYHANDLE,DIR_TYPE_S2C);
    // wait for appropriate time to stabalize
    mdelay(STABILITY_WAIT_TIME);
#endif

    DmaUnregister(handle[0]);

#ifdef FIFO_EMPTY_CHECK
    DmaFifoEmptyWait(MYHANDLE,DIR_TYPE_C2S);
    // wait for appropriate time to stabalize
    mdelay(STABILITY_WAIT_TIME);
#endif
    DmaUnregister(handle[2]);

    PrintSummary();

    mdelay(1000);

    /* Not sure if free_page() sleeps or not. */
    spin_lock_bh(&RawLock);
    printk("Freeing user buffers\n");
    for(i=0; i<TxBufs.TotalNum; i++)
        //kfree(TxBufs.origVA[i]);
        free_page((unsigned long)(TxBufs.origVA[i]));
    for(i=0; i<RxBufs.TotalNum; i++)
        //kfree(RxBufs.origVA[i]);
        free_page((unsigned long)(RxBufs.origVA[i]));
    spin_unlock_bh(&RawLock);
}

module_init(rawdata_init);
module_exit(rawdata_cleanup);

MODULE_AUTHOR("Xilinx, Inc.");
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_LICENSE("GPL");


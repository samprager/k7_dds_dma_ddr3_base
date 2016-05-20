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
 * @file xdma_user.c
 *
 * Contains the APIs used by the application-specific drivers.
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

/***************************** Include Files *********************************/
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/pci.h>

#include "xbasic_types.h"
#include "xdebug.h"
#include "xstatus.h"
#include "xio.h"

#include "xdma.h"
#include "xdma_bd.h"
#include "xdma_user.h"
#include "xdma_hw.h"

/************************** Function Prototypes ******************************/
#ifdef DEBUG_VERBOSE
extern void disp_frag(unsigned char * addr, u32 len);
#endif

/*****************************************************************************/
/**
 * This function must be called by the user driver to register itself with
 * the base DMA driver. After doing required checks to verify the choice
 * of engine and BAR register, it initializes the engine and the BD ring
 * associated with the engine, and enables interrupts if required.
 *
 * Only one user is supported per engine at any given time. Incase the
 * engine has already been registered with another user driver, an error
 * will be returned.
 *
 * @param engine is the DMA engine the user driver wants to use.
 * @param bar is the BAR register the user driver wants to use.
 * @param uptr is a pointer to the function callbacks in the user driver.
 * @param pktsize is the size of packets that the user driver will normally
 *        use.
 *
 * @return NULL incase of any error in completing the registration.
 * @return Handle with which the user driver is registered.
 *
 * @note This function should not be called in an interrupt context
 *
 *****************************************************************************/
void * DmaRegister(int engine, int bar, UserPtrs * uptr, int pktsize)
{
    Dma_Engine * eptr;
    u32 barbase;
    int result;

    log_verbose(KERN_INFO "User register for engine %d, BAR %d, pktsize %d\n",
                engine, bar, pktsize);

    if(DriverState != INITIALIZED)
    {
        printk(KERN_ERR "DMA driver state %d - not ready\n", DriverState);
        return NULL;
    }

    if((bar < 0) || (bar > 5)) {
        printk(KERN_ERR "Requested BAR %d is not valid\n", bar);
        return NULL;
    }

    if(!((dmaData->engineMask) & (1LL << engine))) {
        printk(KERN_ERR "Requested engine %d does not exist\n", engine);
        return NULL;
    }
    eptr = &(dmaData->Dma[engine]);
    barbase = (u32)(dmaData->barInfo[bar].baseVAddr);

    if(eptr->EngineState != INITIALIZED) {
        printk(KERN_ERR "Requested engine %d is not free\n", engine);
        return NULL;
    }

    /* Later, add check for reasonable packet size !!!! */

    /* Later, add check for mandatory user function pointers. For optional
     * ones, assign a stub function pointer. This is better than doing
     * a NULL value check in the performance path. !!!!
     */

    /* Copy user-supplied parameters */
    eptr->user = *uptr;
    eptr->pktSize = pktsize;

    /* Should check for errors returned here !!!! */
    (uptr->UserInit)(barbase, uptr->privData);

    spin_lock_bh(&DmaLock);

    /* Should inform the user of the errors !!!! */
  result = descriptor_init(eptr->pdev, eptr);
  if (result) {
        /* At this point, handle has not been returned to the user.
         * So, user refuses to prepare buffers. Will be trying again in
         * the poll_routine. So, do not abort here.
         */
        printk(KERN_ERR "Cannot create BD ring, will try again later.\n");
        //return NULL;
    }

    /* Change the state of the engine, and increment the user count */
    eptr->EngineState = USER_ASSIGNED;
    dmaData->userCount ++ ;

    /* Start the DMA engine */
  if (Dma_BdRingStart(&(eptr->BdRing)) == XST_FAILURE) {
    log_normal(KERN_ERR "DmaRegister: Could not start Dma channel\n");
    return NULL;
  }

#ifdef TH_BH_ISR
    printk("Now enabling interrupts\n");
    Dma_mEngIntEnable(eptr);
#endif

    spin_unlock_bh(&DmaLock);

    log_verbose(KERN_INFO "Returning user handle %p\n", eptr);

    return eptr;
}

/*****************************************************************************/
/**
 * This function must be called by the user driver to unregister itself from
 * the base DMA driver. After doing required checks to verify the handle
 * and engine state, the DMA engine is reset, interrupts are disabled if
 * required, and the BD ring is freed, while returning all the packet buffers
 * to the user driver.
 *
 * @param handle is the handle which was assigned during the registration
 * process.
 *
 * @return XST_FAILURE incase of any error
 * @return 0 incase of success
 *
 * @note This function should not be called in an interrupt context
 *
 *****************************************************************************/
int DmaUnregister(void * handle)
{
    Dma_Engine * eptr;

    // todo: make sure that caller has ensure that the fifo's are empty
    // caller should have cleared the fifo before calling this function.

    printk(KERN_INFO "User unregister for handle %p\n", handle);
    if(DriverState != INITIALIZED)
    {
        printk(KERN_ERR "DMA driver state %d - not ready\n", DriverState);
        return XST_FAILURE;
    }

    eptr = (Dma_Engine *)handle;

    /* Check if this engine's pointer is valid */
    if(eptr == NULL)
    {
        printk(KERN_ERR "Handle is a NULL value\n");
        return XST_FAILURE;
    }

    /* Is the engine assigned to any user? */
    if(eptr->EngineState != USER_ASSIGNED) {
        printk(KERN_ERR "Engine is not assigned to any user\n");
        return XST_FAILURE;
    }

    spin_lock_bh(&DmaLock);

    /* Change DMA engine state */
    eptr->EngineState = UNREGISTERING;

    /* First, reset DMA engine, so that buffers can be removed. */
    printk(KERN_INFO "Resetting DMA engine\n");
    Dma_Reset(eptr);

    /* Next, return all the buffers in the BD ring to the user.
     * And free the BD ring.
     */

    printk("Now checking all descriptors\n");
    spin_unlock_bh(&DmaLock);
    descriptor_free(eptr->pdev, eptr);
    spin_lock_bh(&DmaLock);

    /* Change DMA engine state */
    eptr->EngineState = INITIALIZED;
    dmaData->userCount --;

    printk("DMA driver user count is %d\n", dmaData->userCount);

    spin_unlock_bh(&DmaLock);

    return 0;
}


#ifdef FIFO_EMPTY_CHECK

void DmaFifoEmptyWait(int handleId, u32 type)
{
  u32 barBase = (u32)(dmaData->barInfo[0].baseVAddr);  //todo: need to confirm that this will never change
  u32 statusReg = barBase + STATUS_REG_OFFSET;
  u32 data = 0;
  //todo: need to callibrate the timeout value
  u32 timeout = FIFO_EMPTY_TIMEOUT;  // effectively its 100 miliseconds

  int check = 1 << (handleId + type);

  check <<= EMPTY_MASK_SHIFT;

  do
  {
    data = XIo_In32(statusReg);
    printk ("\n########read reg: 0x%x + 0x%x ==> 0x%x & check:0x%x\n",barBase, STATUS_REG_OFFSET, data,check);
    if (data & check)
    {
      break;
    }
    else
    {
      printk ("\nDDR FIFO Not empty");
            mdelay(1);
    }
  }while(timeout--);

  if(timeout == -1)
  {
    printk("************** Timeout DDR FIFO not empty **************");
  }

}

#endif
/*****************************************************************************/
/**
 * This function must be called by the user driver to unregister itself from
 * the base DMA driver. After doing required checks to verify the handle
 * and engine state, the DMA engine is reset, interrupts are disabled if
 * required, and the BD ring is freed, while returning all the packet buffers
 * to the user driver.
 *
 * @param handle is the handle which was assigned during the registration
 * process.
 * @param pkts is a PktBuf array, with the array of packets to be sent.
 * @param numpkts is the number of packets in the PktBuf array.
 *
 * @return 0 incase of any error
 * @return Number of packets successfully queued for DMA send, incase of
 * success.
 *
 * @note There may not be enough BDs to accomodate the number of packets
 * requested by the user driver, so the returned value may not match with
 * the requested numpkts. The user driver must be written to manage this
 * appropriately.
 *
 *
 *****************************************************************************/
int DmaSendPkt(void * handle, PktBuf * pkts, int numpkts)
{
#if defined DEBUG_NORMAL || defined DEBUG_VERBOSE
    static int send_count=1;
#endif
  int free_bd_count ;
    Dma_Engine * eptr;
    Dma_BdRing * rptr;
    struct pci_dev *pdev;
  struct privData *lp = NULL;
  Dma_Bd *BdPtr, *BdCurPtr, *PartialBDPtr=NULL;
    dma_addr_t bufPA;
  int result;
    PktBuf * pbuf;
    u32 flags, uflags;
    int i, len;
    int partialBDcount = 0, partialOK = 0;

    log_verbose(KERN_INFO "User send pkt for engine %p, numpkts %d\n",
                            handle, numpkts);
    if(DriverState != INITIALIZED)
    {
        printk(KERN_ERR "DMA driver state %d - not ready\n", DriverState);
        return 0;
    }

    eptr = (Dma_Engine *)handle;
    rptr = &(eptr->BdRing);

    /* Check if this engine's pointer is valid */
    if(eptr == NULL)
    {
        printk(KERN_ERR "Handle is a NULL value\n");
        return 0;
    }

    /* Is the number valid? */
    if(numpkts <= 0) {
        log_normal(KERN_ERR "Packet count should be non-zero\n");
        return 0;
    }

    /* Is the engine assigned to any user? */
    if(eptr->EngineState != USER_ASSIGNED) {
        log_normal(KERN_ERR "Engine is not assigned to any user\n");
        return 0;
    }

    /* Is the engine an S2C one? */
    if(rptr->IsRxChannel)
    {
        log_normal(KERN_ERR "The requested engine cannot send packets\n");
        return 0;
    }

    pdev = eptr->pdev;
    lp = pci_get_drvdata(pdev);

    /* Protect this entry point from the handling of sent packets */
    spin_lock_bh(&DmaLock);

    /* Ensure that requested number of packets can be queued up */
    free_bd_count = Dma_mBdRingGetFreeCnt(rptr);

#if defined DEBUG_NORMAL || defined DEBUG_VERBOSE
    log_normal(KERN_INFO "DmaSendPkt: #%d \n", send_count);
    send_count += numpkts;
    log_verbose(KERN_INFO "BD ring %x Free BD count is %d\n",
                                (u32)rptr, free_bd_count);
    //disp_frag((unsigned char *)bufVA, pktsize);
#endif

    /* Maintain a separation between start and end of BD ring. This is
     * required because DMA will stall if the two pointers coincide -
     * this will happen whether ring is full or empty.
     */
    if(free_bd_count > 2) free_bd_count -= 2;
    else
    {
        log_verbose(KERN_ERR "Not enough BDs to handle %d pkts\n", numpkts);
        spin_unlock_bh(&DmaLock);
        return 0;
    }

    log_normal("DmaSendPkt: numpkts %d free_bd_count %d\n", numpkts, free_bd_count);

    /* Fewer BDs are available than required */
    if(numpkts > free_bd_count)
        numpkts = free_bd_count;

    /* Allocate BDs from ring */
    result = Dma_BdRingAlloc(rptr, numpkts, &BdPtr);
    if (result != XST_SUCCESS) {
        /* we really shouldn't get this */
        printk(KERN_ERR "DmaSendPkt: BdRingAlloc unsuccessful (%d)\n", result);
        spin_unlock_bh(&DmaLock);
        return 0;
    }

    BdCurPtr = BdPtr;
    partialBDcount = 0;
    for(i=0; i<numpkts; i++)
    {
        pbuf = &(pkts[i]);
        bufPA = pci_map_single(pdev, pbuf->pktBuf, pbuf->size, PCI_DMA_TODEVICE);
        log_verbose(KERN_INFO "DmaSendPkt: BD %x buf PA %x VA %x size %d\n",
                   (u32)BdCurPtr, bufPA, (u32) (pbuf->pktBuf), pbuf->size);

        Dma_mBdSetBufAddr(BdCurPtr, bufPA);
        Dma_mBdSetCtrlLength(BdCurPtr, pbuf->size);
        Dma_mBdSetStatLength(BdCurPtr, pbuf->size);      // Required for TX BDs
        Dma_mBdSetId(BdCurPtr, pbuf->bufInfo);

        uflags = pbuf->flags;
        flags = 0;
        if(uflags & DMA_BD_SOP_MASK)
        {
            flags |= DMA_BD_SOP_MASK;

            partialOK = (uflags & PKT_ALL) ? 0 : 1;
            if(!partialOK) PartialBDPtr = BdCurPtr;
        }

        /* Keep track of whether the buffer is the last one in a packet */
        if(uflags & DMA_BD_EOP_MASK)
        {
            flags |= DMA_BD_EOP_MASK;
            partialBDcount = 0;
        }
        else
            partialBDcount++;
        //printk("partialBDcount = %d partialOK = %d PartialBDPtr = %x\n",
        //                        partialBDcount, partialOK, (u32)PartialBDPtr);

        Dma_mBdSetCtrl(BdCurPtr, flags);                 // No ints also.
        Dma_mBdSetUserData(BdCurPtr, pbuf->userInfo);

#ifdef TH_BH_ISR
        /* Enable interrupts for errors and completion based on
         * coalesce count.
         */
        flags |= DMA_BD_INT_ERROR_MASK;
        if(!(eptr->intrCount % INT_COAL_CNT))
            flags |= DMA_BD_INT_COMP_MASK;
        eptr->intrCount += 1;
        Dma_mBdSetCtrl(BdCurPtr, flags);
#endif

        log_verbose("DmaSendPkt: free %d BD %x buf PA %x VA %x size %d flags %x\n",
                 free_bd_count, (u32)BdCurPtr, bufPA, (u32) (pbuf->pktBuf),
                 pbuf->size, flags);

        BdCurPtr = Dma_mBdRingNext(rptr, BdCurPtr);
    }

    /* Ensure that the user does not require all or no fragments of a packet
     * to be handled. For example, incase of a SG-list of multi-BD packets,
     * the user might require all fragments to be handled together.
     */
    if(partialBDcount && !partialOK)
        log_normal(KERN_ERR "Cannot accomodate %d buffers. Discarding %d.\n",
                                           numpkts, partialBDcount);

    /* enqueue TxBD with the attached buffer such that it is
     * ready for frame transmission.
     */
    result = Dma_BdRingToHw(rptr, (numpkts-partialBDcount), BdPtr);
    if((result != XST_SUCCESS) || partialBDcount)
    {
        int count=0;
        if(result != XST_SUCCESS)
        {
            /* We should not come here. Incase of error, unmap buffers,
             * unallocated BDs, and return zero count so that app driver
             * can recover unused buffers.
             */
            printk(KERN_ERR "DmaSendPkt: BdRingToHw unsuccessful (%d)\n",
                                                               result);
            BdCurPtr = BdPtr;
            count = numpkts;
        }
        else if(partialBDcount)
        {
            /* Don't allow partial packets to be queued for DMA incase user
             * does not wish it. Unmap buffers, unallocate BDs, return the
             * count so that app driver can recover unused buffers.
             */
            log_verbose(KERN_ERR "DmaSendPkt: Recovering partial buffers\n");

            BdPtr = BdCurPtr = PartialBDPtr;
            count = partialBDcount;
        }

        for(i=0; i<count; i++)
        {
            bufPA = Dma_mBdGetBufAddr(BdCurPtr);
            len = Dma_mBdGetCtrlLength(BdCurPtr);
            pci_unmap_single(pdev, bufPA, len, PCI_DMA_TODEVICE);
            Dma_mBdSetId(BdCurPtr, NULL);
            BdCurPtr = Dma_mBdRingNext(rptr, BdCurPtr);
        }
        Dma_BdRingUnAlloc(rptr, count, BdPtr);
        numpkts -= count;
    }

    spin_unlock_bh(&DmaLock);

    log_verbose("DmaSendPkt: Successfully transmitted %d buffers\n", numpkts);
    return numpkts;
}

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
* @file xdma.c
*
* This file implements initialization and control related functions. For more
* information on this driver, see xdma.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   ps   12/07/09 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include <linux/string.h>
#include <linux/kernel.h>

#include "xbasic_types.h"
#include "xdebug.h"
#include "xstatus.h"
#include "xio.h"
#include "xdma.h"
#include "xdma_hw.h"


/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
 * This function initializes a DMA engine.  This function must be called
 * prior to using the DMA engine. Initialization of an engine includes setting
 * up the register base address, setting up the instance data, and ensuring the
 * hardware is in a quiescent state.
 *
 * @param  InstancePtr is a pointer to the DMA engine instance to be worked on.
 * @param  BaseAddress is where the registers for this engine can be found.
 *
 * @return None.
 *
 *****************************************************************************/
void Dma_Initialize(Dma_Engine * InstancePtr, u32 BaseAddress, u32 Type)
{
    log_verbose(KERN_INFO "Initializing DMA()\n");

  /* Set up the instance */
    log_verbose(KERN_INFO "Clearing DMA instance %p\n", InstancePtr);
  memset(InstancePtr, 0, sizeof(Dma_Engine));

    log_verbose(KERN_INFO "DMA base address is 0x%x\n", BaseAddress);
  InstancePtr->RegBase = BaseAddress;
    InstancePtr->Type = Type;

    /* Initialize the engine and ring states. */
    InstancePtr->BdRing.RunState = XST_DMA_SG_IS_STOPPED;
    InstancePtr->EngineState = INITIALIZED;

  /* Initialize the ring structure */
  InstancePtr->BdRing.ChanBase = BaseAddress;
    if(Type == DMA_ENG_C2S)
        InstancePtr->BdRing.IsRxChannel = 1;
    else
        InstancePtr->BdRing.IsRxChannel = 0;

  /* Reset the device and return */
  Dma_Reset(InstancePtr);
}

/*****************************************************************************/
/**
* Reset the DMA engine.
*
* Should not be invoked during initialization stage because hardware has
* just come out of a system reset. Should be invoked during shutdown stage.
*
* New BD fetches will stop immediately. Reset will be completed once the
* user logic completes its reset. DMA disable will be completed when the
* BDs already being processed are completed.
*
* @param  InstancePtr is a pointer to the DMA engine instance to be worked on.
*
* @return None.
*
* @note
*   - If the hardware is not working properly, and the self-clearing reset
*     bits do not clear, this function will be terminated after a timeout.
*
******************************************************************************/
void Dma_Reset(Dma_Engine * InstancePtr)
{
  Dma_BdRing *RingPtr;
    int i=0;
    u32 dirqval;

  log_verbose(KERN_INFO "Resetting DMA instance %p\n", InstancePtr);

  RingPtr = &Dma_mGetRing(InstancePtr);

  /* Disable engine interrupts before issuing software reset */
  Dma_mEngIntDisable(InstancePtr);

  /* Start reset process then wait for completion. Disable DMA and
     * assert reset request at the same time. This causes user logic to
     * be reset.
     */
    log_verbose(KERN_INFO "Disabling DMA. User reset request.\n");
    i=0;
  Dma_mSetCrSr(InstancePtr, (DMA_ENG_DISABLE|DMA_ENG_USER_RESET));

  /* Loop until the reset is done. The bits will self-clear. */
  while (Dma_mGetCrSr(InstancePtr) &
                        (DMA_ENG_STATE_MASK|DMA_ENG_USER_RESET)) {
        i++;
        if(i >= 100000)
        {
            printk(KERN_INFO "CR is now 0x%x\n", Dma_mGetCrSr(InstancePtr));
            break;
        }
  }

  /* Now reset the DMA engine, and wait for its completion. */
    log_verbose(KERN_INFO "DMA reset request.\n");
    i=0;
  Dma_mSetCrSr(InstancePtr, (DMA_ENG_RESET));

  /* Loop until the reset is done. The bit will self-clear. */
  while (Dma_mGetCrSr(InstancePtr) & DMA_ENG_RESET) {
        i++;
        if(i >= 100000)
        {
            printk(KERN_INFO "CR is now 0x%x\n", Dma_mGetCrSr(InstancePtr));
            break;
        }
  }

  /* Clear Interrupt register. Not doing so may cause interrupts
     * to be asserted after the engine reset if there is any
   * interrupt left over from before.
   */
    dirqval = Dma_mGetCrSr(InstancePtr);
    printk("While resetting engine, got %x in eng status reg\n", dirqval);
    if(dirqval & DMA_ENG_INT_ACTIVE_MASK)
        Dma_mEngIntAck(InstancePtr, (dirqval & DMA_ENG_ALLINT_MASK));

  RingPtr->RunState = XST_DMA_SG_IS_STOPPED;
}


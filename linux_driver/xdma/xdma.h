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
* @file xdma.h
*
* The Xilinx Scatter Gather DMA driver for the Northwest Logic DMA Engine.
* Each DMA engine may either be for TX (System-to-Card or S2C) or RX (Card-to-
* System or C2S) operation. Each driver instance supports one S2C and one C2S
* DMA engines, allowing full-duplex operation per instance.
*
* Please note that the Xilinx DMA driver uses only the Packet DMA engine
* of the Northwest Logic DMA Engine, and not the Block DMA engine type. As
* such, the description below pertains to the Packet DMA flow.
*
* This component is designed to be used as a basic building block for
* designing a device driver. It provides register accesses such that all
* DMA processing can be maintained easier, but the device driver designer
* must still understand all the details of the DMA channel.
*
* For a full description of DMA features, please see the hardware specification.
* This driver supports the following features:
*
*   - Scatter-Gather DMA (SGDMA)
*   - Interrupts
*   - Interrupts are coalesced by the driver to improve performance
*   - 32-bit addressing for Buffer Descriptors (BDs) and data buffers
*   - APIs to manage BD movement to and from the SGDMA engine
*   - Virtual memory support
*   - PCI Express support
*   - Performance measurement statistics
*   - APIs to enable DMA driver to be used by other application drivers
*
* <b>Packet DMA Transactions</b>
*
* To describe a DMA transaction in its simplest form, you need source address,
* destination address, and the number of bytes to transfer. When using a
* packet DMA receive channel, the source address is within some piece of IP
* hardware and doesn't require the software to explicitly set it. And the
* destination address points to a system buffer into which received data
* must be copied.
*
* Likewise, with a packet DMA transmit channel, the destination address is
* within the hardware and need not be specified by the software. And the
* source address points to a system buffer that must be transmitted.
*
* Therefore, some of the packet DMA transaction attributes include:
*
*   - An application buffer address
*   - The number bytes to transfer
*   - Whether this transaction represents the start of a packet, or end of a
*     packet, or neither (middle of a packet)
*
* The object used to describe a transaction is referred to as a Buffer
* Descriptor (BD). See xdma_bd.h for a detailed description of and the APIs
* for manipulation of these objects.
*
* <b>Scatter-Gather DMA (SGDMA)</b>
*
* SGDMA allows the application to define a list of transactions in memory which
* the hardware will process without further application intervention. During
* this time, the application is free to continue adding more work to keep the
* hardware busy.
*
* Notification of completed transactions can be done either by polling the
* hardware, or using interrupts that signal a transaction has completed, or
* by examining BDs for a completion status.
*
* SGDMA processes whole packets. A packet is defined as a series of
* data bytes that represent a message. SGDMA allows a packet of data to be
* broken up into one or more transactions. For example, take an Ethernet IP
* packet which consists of a 14-byte header followed by a data payload of one
* or more bytes. With SGDMA, the application may point a BD to the header
* and another BD to the payload, then transfer them as a single message.
* This strategy can make a TCP/IP stack more efficient by allowing it to
* keep packet headers and data in different memory regions instead of
* assembling packets into contiguous blocks of memory.
*
* <b>SGDMA Ring Management</b>
*
* The hardware expects BDs to be setup as a singly linked list. As a BD is
* completed, the DMA engine will dereference BD.Next and load the next BD
* to process. These BDs may be arranged in a large chain or in a large ring.
* This driver uses a fixed buffer ring where each BD is linked to the
* next BD in adjacent memory. The last BD in the ring is linked to the first.
*
* Within the ring, the driver maintains four groups of BDs. Each group consists
* of 0 or more adjacent BDs:
*
*   - Free: Those BDs that can be allocated for DMA with Dma_BdRingAlloc().
*
*   - Pre-process: Those BDs that have been allocated with Dma_BdRingAlloc().
*     These BDs can now be modified in preparation for future DMA
*     transactions.
*
*   - Hardware: Those BDs that have been enqueued to hardware with
*     Dma_BdRingToHw(). These BDs are under hardware control and may be in a
*     state of awaiting hardware processing, in process, or processed by
*     hardware. It is considered an error for the driver to change BDs
*     while they are in this group. Doing so can cause data corruption and
*     lead to system instability.
*
*   - Post-process: Those BDs that have been processed by hardware and have
*     been recovered by the driver with Dma_BdRingFromHw(). These BDs
*     are now under driver control. The driver may access these BDs to
*     determine the result of DMA transactions. When the driver is
*     finished, Dma_BdRingFree() should be called to place them back into
*     the Free group.
*
*
* Normally BDs are moved in the following way:
* <pre>
*
*         Dma_BdRingAlloc()                      Dma_BdRingToHw()
*   Free ------------------------> Pre-process ----------------------> Hardware
*                                                                      |
*    /|\                                                               |
*     |   Dma_BdRingFree()                       Dma_BdRingFromHw()    |
*     +--------------------------- Post-process <----------------------+
*
* </pre>
*
* There are a few exceptions to the flow above. One is that after BDs are
* moved from the Free group to the Pre-process group, the driver decides for
* whatever reason that these BDs are not ready and could not be given to
* hardware. In this case, these BDs could be moved back to Free group using
* Dma_BdRingUnAlloc() function to help keep the BD ring in great shape and
* recover the error. See comments of the function for details.
*
* <pre>
*
*           Dma_BdRingUnAlloc()
*   Free <----------------------- Pre-process
*
* </pre>
*
* The second exception to the flow above is when a registered
* application-specific driver unregisters from the DMA driver, then the
* buffers queued for DMA must be retrieved from the BD ring and returned to
* the driver for freeing. This is the only time that this flow is used.
*
* <pre>
*
*         Dma_BdRingAlloc()                      Dma_BdRingToHw()
*   Free ------------------------> Pre-process ----------------------> Hardware
*                                                                        |
*    /|\                                                                 |
*     |   Dma_BdRingFree()                       Dma_BdRingForceFromHw() |
*     +--------------------------- Post-process <------------------------+
*
* </pre>
*
* The API provides macros that allow BD list traversal. These macros should be
* used with care as they do not understand where one group ends and another
* begins.
*
* The driver does not cache or keep copies of any BD. When it modifies BDs
* returned by Dma_BdRingAlloc() or Dma_BdRingFromHw(), it is modifying the
* same BD that hardware accesses.
*
* Certain pairs of list modification functions have usage restrictions. See
* the function headers for Dma_BdRingAlloc() and Dma_BdRingFromHw() for
* more information.
*
* <b>SGDMA Descriptor Ring Creation</b>
*
* During initialization, the function Dma_BdRingCreate() is used to set up
* a memory block to contain all BDs for the DMA channel. This function takes
* as an argument the number of BDs to place in the list.
*
* The caller allocates a memory block large enough to contain the desired
* number of BDs plus an extra one to take care of any shifting in order to
* meet the 32-byte alignment needs of the Northwest Logic DMA hardware.
* This has been covered in more detail below.
*
* Once the list has been created, it can be used right away to perform DMA
* transactions.
*
* <b>Interrupts</b>
*
* The driver has a compile-time flag to enable either interrupts or polled
* mode of operation. In some configurations, the polled mode is seen to have
* better performance than the interrupt mode driver.
*
* When interrupts are enabled, the interrupt handler !!!MUST!!! clear
* pending interrupts before handling the BDs processed by the DMA engine.
* Otherwise the following corner case could raise some issue:
*
* - A packet is transmitted(/received) and asserts a TX(/RX) interrupt, and if
*   this interrupt handler deals with the BDs finished by the DMA before it
*   clears the interrupt, another packet could get transmitted(/received)
*   and assert the interrupt between when the BDs are taken care and when
*   the interrupt clearing operation begins, and the interrupt clearing
*   operation will clear the interrupt raised by the second packet and will
*   never process it according BDs until a new interrupt occurs.
*
* Changing the sequence to "Clear interrupts before handling BDs" solves this
* issue:
*
* - If the interrupt raised by the second packet is before the interrupt
*   clearing operation, the descriptors associated with the second packet must
*   have been finished by hardware and ready for the handler to deal with,
*   and those descriptors will processed with those BDs of the first packet
*   during the handling of the interrupt asserted by the first packet.
*
* - if the interrupt of the second packet is asserted after the interrupt
*   clearing operation but its BDs are finished before the handler starts to
*   deal with BDs, the packet's buffer descriptors will be handled with
*   those of the first packet during the handling of the interrupt asserted
*   by the first packet.
*
* - Otherwise, the BDs of the second packet is not ready when the interrupt
*   handler starts to deal with the BDs of the first packet. Those BDs will
*   be handled next time the interrupt handled gets invoked as the interrupt
*   of the second packet is not cleared in current pass and thereby will
*   cause the handler to get invoked again
*
* Please note if the second case above occurs, the handler will find
* NO buffer descriptor is finished by the hardware (i.e., Dma_BdRingFromHw()
* returns 0) during the handling of the interrupt asserted by the second
* packet. This is valid and the driver should NOT consider this is a
* hardware error and have no need to reset the hardware.
*
* <b>Interrupt Coalescing</b>
*
* On a high-speed link, significant processor overhead may be used in
* servicing interrupts. Interrupt coalescing can be achieved by the driver
* enabling BD completion interrupts, not on every BD, but on a set of BDs,
* thus reducing the frequency of interrupts. The macro INT_COAL_CNT in
* xdma_hw.h can be used for this purpose.
*
* In addition, the driver processes outstanding BDs to be processed after an
* idle timeout with no transactions elapses.
*
* <b> Software Initialization </b>
*
* The driver does the following steps in order to prepare the DMA engine
* to be ready to process DMA transactions:
*
* - DMA Initialization using Dma_Initialize() function. This step
*   initializes a driver instance for the given DMA engine and resets the
*   engine. One driver instance exists for a pair of (S2C and C2S) engines.
* - BD Ring creation. A BD ring is needed per engine and can be built by
*   calling Dma_BdRingCreate(). A parameter passed to this function is the
*   number of BDs fit in a given memory range, and Dma_mBdRingCntCalc() helps
*   calculate the value.
* - (RX channel only) Prepare BDs with attached data buffers and give them to
*   the RX channel. First, allocate BDs using Dma_BdRingAlloc(), then populate
*   data buffer address, data buffer size and the control word fields of each
*   allocated BD with valid values. Last call Dma_BdRingToHw() to give the
*   BDs to the channel.
* - Enable interrupts if interrupt mode is chosen. The application is
*   responsible for setting up the interrupt system, which includes providing
*   and connecting interrupt handlers and call back functions, before
*   the interrupts are enabled.
* - Start DMA channels: Call Dma_BdRingStart() to start a channel
*
* <b> How to start DMA transactions </b>
*
* RX channel is ready to start RX transactions once the initialization (see
* Initialization section above) is finished. The DMA transactions are triggered
* by the user IP (like Local Link TEMAC).
*
* Starting TX transactions needs some work. The application calls
* Dma_BdRingAlloc() to allocate a BD list, then populates necessary
* attributes of each allocated BD including data buffer address, data size,
* and control word, and last passes those BDs to the TX channel
* (see Dma_BdRingToHw()). The added BDs will be processed as soon as the
* TX channel reaches them.
*
* <b> Software Post-Processing on completed DMA transactions </b>
*
* Some software post-processing is needed after DMA transactions are finished.
*
* If interrupts are set up and enabled, DMA channels notify the software
* the finishing of DMA transactions using interrupts,  Otherwise the
* application could poll the channels (see Dma_BdRingFromHw()).
*
* - Once BDs are finished by a channel, the application first needs to fetch
*   them from the channel (see Dma_BdRingFromHw()).
* - On TX side, the application now could free the data buffers attached to
*   those BDs as the data in the buffers has been transmitted.
* - On RX side, the application now could use the received data in the buffers
*   attached to those BDs
* - For both channels, those BDs need to be freed back to the Free group (see
*   Dma_BdRingFree()) so they are allocatable for future transactions.
* - On RX side, it is the application's responsibility for having BDs ready
*   to receive data at any time. Otherwise the RX channel will refuse to
*   accept any data once it runs out of RX BDs. As we just freed those hardware
*   completed BDs in the previous step, it is good timing to allocate them
*   back (see Dma_BdRingAlloc()), prepare them, and feed them to the RX
*   channel again (see Dma_BdRingToHw())
*
* <b>Address Translation</b>
*
* When the BD list is setup with Dma_BdRingCreate(), a physical and
* virtual address is supplied for the segment of memory containing the
* descriptors. The driver will handle any translations internally. Subsequent
* access of descriptors by the application is done in terms of their virtual
* address.
*
* Any application data buffer address attached to a BD must be physical
* address. The application is responsible for calculating the physical address
* before assigns it to the buffer address field in the BD.
*
* <b> Memory Barriers </b>
*
* The DMA hardware expects the update to its Next BD pointer register to be
* the event which initiates DMA processing. Hence, memory barrier wmb() calls
* have been used to ensure this.
*
* <b>Alignment</b>
*
* <b> For BDs: </b>
* The Northwest Logic DMA hardware requires BDs to be aligned at 32-byte
* boundaries. In addition to the this, the driver has its own alignment
* requirements. It needs to store per-packet information in each BD, for
* example, the buffer virtual address. In order to do this, the software
* view of the BD may be larger than the hardware view of the BD. For example,
* DMA_BD_SW_NUM_WORDS can be set to 16 words (64 bytes), even though
* DMA_BD_HW_NUM_WORDS is 8 words (32 bytes). Due to this, the driver
* gets additional space in which to store per-BD private information.
*
* Minimum alignment is defined by the constant DMA_BD_MINIMUM_ALIGNMENT.
* This is the smallest alignment allowed by both hardware and software for them
* to properly work. Other than DMA_BD_MINIMUM_ALIGNMENT, multiples of the
* constant are the only valid alignments for BDs.
*
* If the descriptor ring is to be placed in cached memory, alignment also MUST
* be at least the processor's cache-line size. If this requirement is not met
* then system instability will result. This is also true if the length of a BD
* is longer than one cache-line, in which case multiple cache-lines are needed
* to accommodate each BD.
*
* Aside from the initial creation of the descriptor ring (see
* Dma_BdRingCreate()), there are no other run-time checks for proper
* alignment.
*
* <b>For application data buffers:</b>
* Application data buffer alignment is taken care of by the
* application-specific drivers.
*
* <b>Reset After Stopping</b>
*
* This driver is designed to allow for stop-reset-start cycles of the DMA
* hardware while keeping the BD list intact. When restarted after a reset, this
* driver will point the DMA engine to where it left off after stopping it.
*
* It is possible to load an application-specific driver, run it for some
* time, and then unload it. Without unloading the DMA driver as well, it
* should be possible to load another instance of the application-specific
* driver and it should work fine.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   ps   06/30/09 First version
* 1.0   ps   12/07/09 Release version
* </pre>
*
******************************************************************************/

#ifndef XDMA_H    /* prevent circular inclusions */
#define XDMA_H    /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include <linux/timer.h>

#include "xbasic_types.h"
#include "xdma_bdring.h"
#include "xdma_user.h"

/************************** Constant Definitions *****************************/

#define MAX_BARS                6       /**< Maximum number of BARs */
#define MAX_DMA_ENGINES         64      /**< Maximum number of DMA engines */

#define DMA_ALL_BDS         0xFFFFFFFF  /**< Indicates all valid BDs */

/**************************** Type Definitions *******************************/

/** @name DMA engine instance data
 * @{
 */
typedef struct {
    struct pci_dev * pdev;  /**< PCIe Device handle */
    u32 RegBase;            /**< Virtual base address of DMA engine */

    u32 EngineState;        /**< State of the DMA engine */
  Dma_BdRing BdRing;      /**< BD container for DMA engine */
    u32 Type;               /**< Type of DMA engine - C2S or S2C */
    UserPtrs user;          /**< User callback functions */
    int pktSize;            /**< User-specified usual size of packets */

#ifdef TH_BH_ISR
    int intrCount;          /**< Counter to control interrupt coalescing */
#endif

    dma_addr_t descSpacePA; /**< Physical address of BD space */
    u32 descSpaceSize;      /**< Size of BD space in bytes */
    u32 * descSpaceVA;      /**< Virtual address of BD space */
    u32 delta;              /**< Shift from original start for alignment */
} Dma_Engine;
/*@}*/

/** @name Private per-device data
 * The PCI device entry points to this as driver-private data. In some
 * cases, pointer back to PCI device entry is also required.
 * @{
 */
struct privData {
    struct pci_dev * pdev;          /**< PCI device entry */

    /** BAR information discovered on probe. BAR0 is understood by this driver.
     * Other BARs will be used as app. drivers register with this driver.
     */
    u32 barMask;                    /**< Bitmask for BAR information */
    struct {
        unsigned long basePAddr;    /**< Base address of device memory */
        unsigned long baseLen;      /**< Length of device memory */
        void __iomem * baseVAddr;   /**< VA - mapped address */
    } barInfo[MAX_BARS];

  //struct list_head rcv;
  //struct list_head xmit;

  u32 index;                    /**< Which interface is this */

    /**
     * The user driver instance data. An instance must be allocated
     * for each user request. The user driver request will request separately
     * for one C2S and one S2C DMA engine instances, if required. The DMA
     * driver will not allocate a pair of engine instances on its own.
     */
    long long engineMask;           /**< For storing a 64-bit mask */
  Dma_Engine Dma[MAX_DMA_ENGINES];/**< Per-engine information */

    int userCount;                  /**< Number of registered users */
};
extern struct privData * dmaData;
/*@}*/

extern u32 DriverState;

/* for exclusion of all program flows (processes, ISRs and BHs) */
extern spinlock_t DmaLock;
extern spinlock_t DmaTXLock;
extern spinlock_t DmaRXLock;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/** @name Initialization and control functions in xdma.c
 * @{
 */
int descriptor_init(struct pci_dev *pdev, Dma_Engine * eptr);
void descriptor_free(struct pci_dev *pdev, Dma_Engine * eptr);
void Dma_Initialize(Dma_Engine * InstancePtr, u32 BaseAddress, u32 Type);
void Dma_Reset(Dma_Engine * InstancePtr);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

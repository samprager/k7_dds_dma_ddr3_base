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
* @file xdebug.h
*
* This file contains logging tools for Xilinx software IP.
*
* @note
*
* This file contains items which are architecture dependent.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#ifdef __cplusplus
extern "C" {
#endif


/************************** Constant Definitions *****************************/

#ifndef XDEBUG  /* prevent circular inclusions */
#define XDEBUG  /* by using protection macros */

//#define MYDEBUG
#if defined(MYDEBUG) && !defined(NDEBUG)

#ifndef XDEBUG_WARNING
#define XDEBUG_WARNING
//#warning DEBUG is enabled
#endif

#define XDBG_DEBUG_ERROR             0x00000001 /**< error condition messages */
#define XDBG_DEBUG_GENERAL           0x00000002 /**< general debug  messages */
#define XDBG_DEBUG_ALL               0xFFFFFFFF /**< all debugging data */

#define XDBG_DEBUG_FIFO_REG          0x00000100 /**< display register reads/writes */
#define XDBG_DEBUG_FIFO_RX           0x00000101 /**< receive debug messages */
#define XDBG_DEBUG_FIFO_TX           0x00000102 /**< transmit debug messages */
#define XDBG_DEBUG_FIFO_ALL          0x0000010F /**< all fifo debug messages */

#define XDBG_DEBUG_TEMAC_REG         0x00000400 /**< display register reads/writes */
#define XDBG_DEBUG_TEMAC_RX          0x00000401 /**< receive debug messages */
#define XDBG_DEBUG_TEMAC_TX          0x00000402 /**< transmit debug messages */
#define XDBG_DEBUG_TEMAC_ALL         0x0000040F /**< all temac  debug messages */

#define XDBG_DEBUG_TEMAC_ADPT_RX     0x00000800 /**< receive debug messages */
#define XDBG_DEBUG_TEMAC_ADPT_TX     0x00000801 /**< transmit debug messages */
#define XDBG_DEBUG_TEMAC_ADPT_IOCTL  0x00000802 /**< ioctl debug messages */
#define XDBG_DEBUG_TEMAC_ADPT_MISC   0x00000803 /**< debug msg for other routines */
#define XDBG_DEBUG_TEMAC_ADPT_ALL    0x0000080F /**< all temac adapter debug messages */

#define xdbg_current_types (XDBG_DEBUG_ERROR | XDBG_DEBUG_GENERAL | XDBG_DEBUG_TEMAC_REG)

#define xdbg_stmnt(x)  x
#define xdbg_printf(type, ...) (((type) & xdbg_current_types) ? printk (__VA_ARGS__) : 0)

#else
#define xdbg_stmnt(x)
#define xdbg_printf(...)
#endif


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/* Macros for handling normal and verbose logging in adapter and DMA code */

#ifdef DEBUG_VERBOSE /* Enable both normal and verbose logging */

#define log_verbose(args...)    printk(args)
#define log_normal(args...)     printk(args)

#elif defined DEBUG_NORMAL /* Enable only normal logging */

#define log_verbose(x...)
#define log_normal(args...)     printk(args)

#else

#define log_normal(x...)
#define log_verbose(x...)

#endif /* DEBUG_VERBOSE and DEBUG_NORMAL */


/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XDEBUG */ /* end of protection macro */

/*******************************************************************************
** � Copyright 2011 - 2012 Xilinx, Inc. All rights reserved.
** This file contains confidential and proprietary information of Xilinx, Inc. and
** is protected under U.S. and international copyright and other intellectual property laws.
*******************************************************************************
**   ____  ____
**  /   /\/   /
** /___/  \  /   Vendor: Xilinx
** \   \   \/
**  \   \
**  /   /
** /___/   /\
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

#ifndef TYPES_H
#define TYPES_H

#include <gtk/gtk.h>
#include <sys/types.h>

/* Types */
typedef unsigned char  uchar;   /* 8-bit value on Windows */
//typedef unsigned short ushort;  /* 16-bit value on Windows */
//typedef unsigned int   uint;    /* 32-bit value on Windows */

/* Various printfs
 * The following should ideally be properly enclosed in curly braces as
 * a good programming practice. But, due to the necessity for handling
 * variable argument lists, this has not been done.
 * Also, comma is being used as the separator, incase the printf is from
 * an if-else branch.
 */
#define msg_error       display=MSG_ERROR, g_print
#define msg_warning     display=MSG_WARNING, g_print
#define msg_info        display=MSG_INFO, g_print
#define msg_debug       display=MSG_DEBUG, g_print

enum print
{
  MSG_ERROR = 0,
  MSG_WARNING,
  MSG_INFO,
  MSG_DEBUG
};
extern enum print display;

#endif

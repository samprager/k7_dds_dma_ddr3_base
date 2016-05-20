// -- (c) Copyright 2010 - 2013 Xilinx, Inc. All rights reserved.
// --
// -- This file contains confidential and proprietary information
// -- of Xilinx, Inc. and is protected under U.S. and 
// -- international copyright and other intellectual property
// -- laws.
// --
// -- DISCLAIMER
// -- This disclaimer is not a license and does not grant any
// -- rights to the materials distributed herewith. Except as
// -- otherwise provided in a valid license issued to you by
// -- Xilinx, and to the maximum extent permitted by applicable
// -- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// -- WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// -- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// -- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// -- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// -- (2) Xilinx shall not be liable (whether in contract or tort,
// -- including negligence, or under any other theory of
// -- liability) for any loss or damage of any kind or nature
// -- related to, arising under or in connection with these
// -- materials, including for any direct, or any indirect,
// -- special, incidental, or consequential loss or damage
// -- (including loss of data, profits, goodwill, or any type of
// -- loss or damage suffered as a result of any action brought
// -- by a third party) even if such damage or loss was
// -- reasonably foreseeable or Xilinx had been advised of the
// -- possibility of the same.
// --
// -- CRITICAL APPLICATIONS
// -- Xilinx products are not designed or intended to be fail-
// -- safe, or for use in any application requiring fail-safe
// -- performance, such as life-support or safety devices or
// -- systems, Class III medical devices, nuclear facilities,
// -- applications related to the deployment of airbags, or any
// -- other applications that could lead to death, personal
// -- injury, or severe property or environmental damage
// -- (individually and collectively, "Critical
// -- Applications"). Customer assumes the sole risk and
// -- liability of any use of Xilinx products in Critical
// -- Applications, subject only to applicable laws and
// -- regulations governing limitations on product liability.
// --
// -- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// -- PART OF THIS FILE AT ALL TIMES.
//-----------------------------------------------------------------------------
//
// AXI data fifo module:    
//   5-channel memory-mapped AXI4 interfaces.
//   SRL or BRAM based FIFO on AXI W and/or R channels.
//   FIFO to accommodate various data flow rates through the AXI interconnect
//
// Verilog-standard:  Verilog 2001
//-----------------------------------------------------------------------------
//
// Structure:
//   axi_data_fifo
//     fifo_generator
//
//-----------------------------------------------------------------------------

`timescale 1ps/1ps

(* DowngradeIPIdentifiedWarnings="yes" *) 
module axi_interconnect_v1_7_axi_data_fifo #
  (
   parameter         C_FAMILY                    = "virtex7",
   parameter integer C_AXI_ID_WIDTH              = 4,
   parameter integer C_AXI_ADDR_WIDTH            = 32,
   parameter integer C_AXI_DATA_WIDTH            = 32,
   parameter integer C_AXI_SUPPORTS_USER_SIGNALS = 0,
   parameter integer C_AXI_AWUSER_WIDTH          = 1,
   parameter integer C_AXI_ARUSER_WIDTH          = 1,
   parameter integer C_AXI_WUSER_WIDTH           = 1,
   parameter integer C_AXI_RUSER_WIDTH           = 1,
   parameter integer C_AXI_BUSER_WIDTH           = 1,
   parameter integer C_AXI_WRITE_FIFO_DEPTH      = 0,      // Range: (0, 32, 512)
   parameter         C_AXI_WRITE_FIFO_TYPE       = "lut",  // "lut" = LUT (SRL) based,
                                                           // "bram" = BRAM based
   parameter integer C_AXI_WRITE_FIFO_DELAY      = 0,      // 0 = No, 1 = Yes (FUTURE FEATURE)
                       // Indicates whether AWVALID and WVALID assertion is delayed until:
                       //   a. the corresponding WLAST is stored in the FIFO, or
                       //   b. no WLAST is stored and the FIFO is full.
                       // 0 means AW channel is pass-through and 
                       //   WVALID is asserted whenever FIFO is not empty.
   parameter integer C_AXI_READ_FIFO_DEPTH       = 0,      // Range: (0, 32, 512)
   parameter         C_AXI_READ_FIFO_TYPE        = "lut",  // "lut" = LUT (SRL) based,
                                                           // "bram" = BRAM based
   parameter integer C_AXI_READ_FIFO_DELAY       = 0)      // 0 = No, 1 = Yes (FUTURE FEATURE)
                       // Indicates whether ARVALID assertion is delayed until the 
                       //   the remaining vacancy of the FIFO is at least the burst length
                       //   as indicated by ARLEN.
                       // 0 means AR channel is pass-through.
   // System Signals
  (input wire ACLK,
   input wire ARESETN,

   // Slave Interface Write Address Ports
   input  wire [C_AXI_ID_WIDTH-1:0]     S_AXI_AWID,
   input  wire [C_AXI_ADDR_WIDTH-1:0]   S_AXI_AWADDR,
   input  wire [8-1:0]                  S_AXI_AWLEN,
   input  wire [3-1:0]                  S_AXI_AWSIZE,
   input  wire [2-1:0]                  S_AXI_AWBURST,
   input  wire [2-1:0]                  S_AXI_AWLOCK,
   input  wire [4-1:0]                  S_AXI_AWCACHE,
   input  wire [3-1:0]                  S_AXI_AWPROT,
   input  wire [4-1:0]                  S_AXI_AWREGION,
   input  wire [4-1:0]                  S_AXI_AWQOS,
   input  wire [C_AXI_AWUSER_WIDTH-1:0] S_AXI_AWUSER,
   input  wire                          S_AXI_AWVALID,
   output wire                          S_AXI_AWREADY,

   // Slave Interface Write Data Ports
   input  wire [C_AXI_DATA_WIDTH-1:0]   S_AXI_WDATA,
   input  wire [C_AXI_DATA_WIDTH/8-1:0] S_AXI_WSTRB,
   input  wire                          S_AXI_WLAST,
   input  wire [C_AXI_WUSER_WIDTH-1:0]  S_AXI_WUSER,
   input  wire                          S_AXI_WVALID,
   output wire                          S_AXI_WREADY,

   // Slave Interface Write Response Ports
   output wire [C_AXI_ID_WIDTH-1:0]     S_AXI_BID,
   output wire [2-1:0]                  S_AXI_BRESP,
   output wire [C_AXI_BUSER_WIDTH-1:0]  S_AXI_BUSER,
   output wire                          S_AXI_BVALID,
   input  wire                          S_AXI_BREADY,

   // Slave Interface Read Address Ports
   input  wire [C_AXI_ID_WIDTH-1:0]     S_AXI_ARID,
   input  wire [C_AXI_ADDR_WIDTH-1:0]   S_AXI_ARADDR,
   input  wire [8-1:0]                  S_AXI_ARLEN,
   input  wire [3-1:0]                  S_AXI_ARSIZE,
   input  wire [2-1:0]                  S_AXI_ARBURST,
   input  wire [2-1:0]                  S_AXI_ARLOCK,
   input  wire [4-1:0]                  S_AXI_ARCACHE,
   input  wire [3-1:0]                  S_AXI_ARPROT,
   input  wire [4-1:0]                  S_AXI_ARREGION,
   input  wire [4-1:0]                  S_AXI_ARQOS,
   input  wire [C_AXI_ARUSER_WIDTH-1:0] S_AXI_ARUSER,
   input  wire                          S_AXI_ARVALID,
   output wire                          S_AXI_ARREADY,

   // Slave Interface Read Data Ports
   output wire [C_AXI_ID_WIDTH-1:0]     S_AXI_RID,
   output wire [C_AXI_DATA_WIDTH-1:0]   S_AXI_RDATA,
   output wire [2-1:0]                  S_AXI_RRESP,
   output wire                          S_AXI_RLAST,
   output wire [C_AXI_RUSER_WIDTH-1:0]  S_AXI_RUSER,
   output wire                          S_AXI_RVALID,
   input  wire                          S_AXI_RREADY,
   
   // Master Interface Write Address Port
   output wire [C_AXI_ID_WIDTH-1:0]     M_AXI_AWID,
   output wire [C_AXI_ADDR_WIDTH-1:0]   M_AXI_AWADDR,
   output wire [8-1:0]                  M_AXI_AWLEN,
   output wire [3-1:0]                  M_AXI_AWSIZE,
   output wire [2-1:0]                  M_AXI_AWBURST,
   output wire [2-1:0]                  M_AXI_AWLOCK,
   output wire [4-1:0]                  M_AXI_AWCACHE,
   output wire [3-1:0]                  M_AXI_AWPROT,
   output wire [4-1:0]                  M_AXI_AWREGION,
   output wire [4-1:0]                  M_AXI_AWQOS,
   output wire [C_AXI_AWUSER_WIDTH-1:0] M_AXI_AWUSER,
   output wire                          M_AXI_AWVALID,
   input  wire                          M_AXI_AWREADY,
   
   // Master Interface Write Data Ports
   output wire [C_AXI_DATA_WIDTH-1:0]   M_AXI_WDATA,
   output wire [C_AXI_DATA_WIDTH/8-1:0] M_AXI_WSTRB,
   output wire                          M_AXI_WLAST,
   output wire [C_AXI_WUSER_WIDTH-1:0]  M_AXI_WUSER,
   output wire                          M_AXI_WVALID,
   input  wire                          M_AXI_WREADY,
   
   // Master Interface Write Response Ports
   input  wire [C_AXI_ID_WIDTH-1:0]     M_AXI_BID,
   input  wire [2-1:0]                  M_AXI_BRESP,
   input  wire [C_AXI_BUSER_WIDTH-1:0]  M_AXI_BUSER,
   input  wire                          M_AXI_BVALID,
   output wire                          M_AXI_BREADY,
   
   // Master Interface Read Address Port
   output wire [C_AXI_ID_WIDTH-1:0]     M_AXI_ARID,
   output wire [C_AXI_ADDR_WIDTH-1:0]   M_AXI_ARADDR,
   output wire [8-1:0]                  M_AXI_ARLEN,
   output wire [3-1:0]                  M_AXI_ARSIZE,
   output wire [2-1:0]                  M_AXI_ARBURST,
   output wire [2-1:0]                  M_AXI_ARLOCK,
   output wire [4-1:0]                  M_AXI_ARCACHE,
   output wire [3-1:0]                  M_AXI_ARPROT,
   output wire [4-1:0]                  M_AXI_ARREGION,
   output wire [4-1:0]                  M_AXI_ARQOS,
   output wire [C_AXI_ARUSER_WIDTH-1:0] M_AXI_ARUSER,
   output wire                          M_AXI_ARVALID,
   input  wire                          M_AXI_ARREADY,
   
   // Master Interface Read Data Ports
   input  wire [C_AXI_ID_WIDTH-1:0]     M_AXI_RID,
   input  wire [C_AXI_DATA_WIDTH-1:0]   M_AXI_RDATA,
   input  wire [2-1:0]                  M_AXI_RRESP,
   input  wire                          M_AXI_RLAST,
   input  wire [C_AXI_RUSER_WIDTH-1:0]  M_AXI_RUSER,
   input  wire                          M_AXI_RVALID,
   output wire                          M_AXI_RREADY);
  
  localparam integer P_WIDTH_RACH = 4 + 4 + 3 + 4 + 2 + 2 + 3 + 8 + C_AXI_ADDR_WIDTH + C_AXI_ID_WIDTH + (C_AXI_SUPPORTS_USER_SIGNALS ? C_AXI_ARUSER_WIDTH : 0);
  localparam integer P_WIDTH_RDCH = 1 + 2 + C_AXI_DATA_WIDTH + C_AXI_ID_WIDTH + (C_AXI_SUPPORTS_USER_SIGNALS ? C_AXI_RUSER_WIDTH : 0);
  localparam integer P_WIDTH_WACH = 4 + 4 + 3 + 4 + 2 + 2 + 3 + 8 + C_AXI_ADDR_WIDTH + C_AXI_ID_WIDTH + (C_AXI_SUPPORTS_USER_SIGNALS ? C_AXI_AWUSER_WIDTH : 0);
  localparam integer P_WIDTH_WDCH = 1 + C_AXI_DATA_WIDTH + C_AXI_DATA_WIDTH/8 + C_AXI_ID_WIDTH + (C_AXI_SUPPORTS_USER_SIGNALS ? C_AXI_WUSER_WIDTH : 0);
  
  localparam P_PRIM_FIFO_TYPE = "512x72" ;
  localparam integer P_WRITE_FIFO_DEPTH_LOG = (C_AXI_WRITE_FIFO_DEPTH > 1) ? f_ceil_log2(C_AXI_WRITE_FIFO_DEPTH) : 1;
  localparam integer P_READ_FIFO_DEPTH_LOG = (C_AXI_READ_FIFO_DEPTH > 1) ? f_ceil_log2(C_AXI_READ_FIFO_DEPTH) : 1;
  
  // Ceiling of log2(x)
  function integer f_ceil_log2
    (
     input integer x
     );
    integer acc;
    begin
      acc=0;
      while ((2**acc) < x)
        acc = acc + 1;
      f_ceil_log2 = acc;
    end
  endfunction

  generate
    if ((C_AXI_WRITE_FIFO_DEPTH == 0) && (C_AXI_READ_FIFO_DEPTH == 0)) begin : gen_bypass
      assign M_AXI_AWID     = S_AXI_AWID;
      assign M_AXI_AWADDR   = S_AXI_AWADDR;
      assign M_AXI_AWLEN    = S_AXI_AWLEN;
      assign M_AXI_AWSIZE   = S_AXI_AWSIZE;
      assign M_AXI_AWBURST  = S_AXI_AWBURST;
      assign M_AXI_AWLOCK   = S_AXI_AWLOCK;
      assign M_AXI_AWCACHE  = S_AXI_AWCACHE;
      assign M_AXI_AWPROT   = S_AXI_AWPROT;
      assign M_AXI_AWREGION = S_AXI_AWREGION;
      assign M_AXI_AWQOS    = S_AXI_AWQOS;
      assign M_AXI_AWUSER   = S_AXI_AWUSER;
      assign M_AXI_AWVALID  = S_AXI_AWVALID;
      assign S_AXI_AWREADY  = M_AXI_AWREADY;
      
      assign M_AXI_WDATA    = S_AXI_WDATA;
      assign M_AXI_WSTRB    = S_AXI_WSTRB;
      assign M_AXI_WLAST    = S_AXI_WLAST;
      assign M_AXI_WUSER    = S_AXI_WUSER;
      assign M_AXI_WVALID   = S_AXI_WVALID;
      assign S_AXI_WREADY   = M_AXI_WREADY;
    
      assign S_AXI_BID      = M_AXI_BID;
      assign S_AXI_BRESP    = M_AXI_BRESP;
      assign S_AXI_BUSER    = M_AXI_BUSER;
      assign S_AXI_BVALID   = M_AXI_BVALID;
      assign M_AXI_BREADY   = S_AXI_BREADY;
    
      assign M_AXI_ARID     = S_AXI_ARID;
      assign M_AXI_ARADDR   = S_AXI_ARADDR;
      assign M_AXI_ARLEN    = S_AXI_ARLEN;
      assign M_AXI_ARSIZE   = S_AXI_ARSIZE;
      assign M_AXI_ARBURST  = S_AXI_ARBURST;
      assign M_AXI_ARLOCK   = S_AXI_ARLOCK;
      assign M_AXI_ARCACHE  = S_AXI_ARCACHE;
      assign M_AXI_ARPROT   = S_AXI_ARPROT;
      assign M_AXI_ARREGION = S_AXI_ARREGION;
      assign M_AXI_ARQOS    = S_AXI_ARQOS;
      assign M_AXI_ARUSER   = S_AXI_ARUSER;
      assign M_AXI_ARVALID  = S_AXI_ARVALID;
      assign S_AXI_ARREADY  = M_AXI_ARREADY;
  
      assign S_AXI_RID      = M_AXI_RID;
      assign S_AXI_RDATA    = M_AXI_RDATA;
      assign S_AXI_RRESP    = M_AXI_RRESP;
      assign S_AXI_RLAST    = M_AXI_RLAST;
      assign S_AXI_RUSER    = M_AXI_RUSER;
      assign S_AXI_RVALID   = M_AXI_RVALID;
      assign M_AXI_RREADY   = S_AXI_RREADY;
    end else begin : gen_fifo

  fifo_generator_v10_0 #(
          .C_ADD_NGC_CONSTRAINT(0),
          .C_APPLICATION_TYPE_AXIS(0),
          .C_APPLICATION_TYPE_RACH(C_AXI_READ_FIFO_DELAY ? 1 : 0),
          .C_APPLICATION_TYPE_RDCH(0),
          .C_APPLICATION_TYPE_WACH(C_AXI_WRITE_FIFO_DELAY ? 1 : 0),
          .C_APPLICATION_TYPE_WDCH(0),
          .C_APPLICATION_TYPE_WRCH(0),
          .C_AXI_ADDR_WIDTH(C_AXI_ADDR_WIDTH),
          .C_AXI_ARUSER_WIDTH(C_AXI_ARUSER_WIDTH),
          .C_AXI_AWUSER_WIDTH(C_AXI_AWUSER_WIDTH),
          .C_AXI_BUSER_WIDTH(C_AXI_BUSER_WIDTH),
          .C_AXI_DATA_WIDTH(C_AXI_DATA_WIDTH),
          .C_AXI_ID_WIDTH(C_AXI_ID_WIDTH),
          .C_AXI_RUSER_WIDTH(C_AXI_RUSER_WIDTH),
          .C_AXI_TYPE(1),
          .C_AXI_WUSER_WIDTH(C_AXI_WUSER_WIDTH),
          .C_AXIS_TDATA_WIDTH(64),
          .C_AXIS_TDEST_WIDTH(4),
          .C_AXIS_TID_WIDTH(8),
          .C_AXIS_TKEEP_WIDTH(4),
          .C_AXIS_TSTRB_WIDTH(4),
          .C_AXIS_TUSER_WIDTH(4),
          .C_AXIS_TYPE(0),
          .C_COMMON_CLOCK(1),
          .C_COUNT_TYPE(0),
          .C_DATA_COUNT_WIDTH(10),
          .C_DEFAULT_VALUE("BlankString"),
          .C_DIN_WIDTH(18),
          .C_DIN_WIDTH_AXIS(1),
          .C_DIN_WIDTH_RACH(P_WIDTH_RACH),
          .C_DIN_WIDTH_RDCH(P_WIDTH_RDCH),
          .C_DIN_WIDTH_WACH(P_WIDTH_WACH),
          .C_DIN_WIDTH_WDCH(P_WIDTH_WDCH),
          .C_DIN_WIDTH_WRCH(6),
          .C_DOUT_RST_VAL("0"),
          .C_DOUT_WIDTH(18),
          .C_ENABLE_RLOCS(0),
          .C_ENABLE_RST_SYNC(1),
          .C_ERROR_INJECTION_TYPE(0),
          .C_ERROR_INJECTION_TYPE_AXIS(0),
          .C_ERROR_INJECTION_TYPE_RACH(0),
          .C_ERROR_INJECTION_TYPE_RDCH(0),
          .C_ERROR_INJECTION_TYPE_WACH(0),
          .C_ERROR_INJECTION_TYPE_WDCH(0),
          .C_ERROR_INJECTION_TYPE_WRCH(0),
          .C_FAMILY(C_FAMILY),
          .C_FULL_FLAGS_RST_VAL(1),
          .C_HAS_ALMOST_EMPTY(0),
          .C_HAS_ALMOST_FULL(0),
          .C_HAS_AXI_ARUSER(C_AXI_SUPPORTS_USER_SIGNALS),
          .C_HAS_AXI_AWUSER(C_AXI_SUPPORTS_USER_SIGNALS),
          .C_HAS_AXI_BUSER(C_AXI_SUPPORTS_USER_SIGNALS),
          .C_HAS_AXI_RD_CHANNEL(1),
          .C_HAS_AXI_RUSER(C_AXI_SUPPORTS_USER_SIGNALS),
          .C_HAS_AXI_WR_CHANNEL(1),
          .C_HAS_AXI_WUSER(C_AXI_SUPPORTS_USER_SIGNALS),
          .C_HAS_AXIS_TDATA(0),
          .C_HAS_AXIS_TDEST(0),
          .C_HAS_AXIS_TID(0),
          .C_HAS_AXIS_TKEEP(0),
          .C_HAS_AXIS_TLAST(0),
          .C_HAS_AXIS_TREADY(1),
          .C_HAS_AXIS_TSTRB(0),
          .C_HAS_AXIS_TUSER(0),
          .C_HAS_BACKUP(0),
          .C_HAS_DATA_COUNT(0),
          .C_HAS_DATA_COUNTS_AXIS(0),
          .C_HAS_DATA_COUNTS_RACH(0),
          .C_HAS_DATA_COUNTS_RDCH(0),
          .C_HAS_DATA_COUNTS_WACH(0),
          .C_HAS_DATA_COUNTS_WDCH(0),
          .C_HAS_DATA_COUNTS_WRCH(0),
          .C_HAS_INT_CLK(0),
          .C_HAS_MASTER_CE(0),
          .C_HAS_MEMINIT_FILE(0),
          .C_HAS_OVERFLOW(0),
          .C_HAS_PROG_FLAGS_AXIS(0),
          .C_HAS_PROG_FLAGS_RACH(0),
          .C_HAS_PROG_FLAGS_RDCH(0),
          .C_HAS_PROG_FLAGS_WACH(0),
          .C_HAS_PROG_FLAGS_WDCH(0),
          .C_HAS_PROG_FLAGS_WRCH(0),
          .C_HAS_RD_DATA_COUNT(0),
          .C_HAS_RD_RST(0),
          .C_HAS_RST(1),
          .C_HAS_SLAVE_CE(0),
          .C_HAS_SRST(0),
          .C_HAS_UNDERFLOW(0),
          .C_HAS_VALID(0),
          .C_HAS_WR_ACK(0),
          .C_HAS_WR_DATA_COUNT(0),
          .C_HAS_WR_RST(0),
          .C_IMPLEMENTATION_TYPE(0),
          .C_IMPLEMENTATION_TYPE_AXIS(1),
          .C_IMPLEMENTATION_TYPE_RACH(2),
          .C_IMPLEMENTATION_TYPE_RDCH((C_AXI_READ_FIFO_TYPE == "bram") ? 1 : 2),
          .C_IMPLEMENTATION_TYPE_WACH(2),
          .C_IMPLEMENTATION_TYPE_WDCH((C_AXI_WRITE_FIFO_TYPE == "bram") ? 1 : 2),
          .C_IMPLEMENTATION_TYPE_WRCH(2),
          .C_INIT_WR_PNTR_VAL(0),
          .C_INTERFACE_TYPE(1),
          .C_MEMORY_TYPE(1),
          .C_MIF_FILE_NAME("BlankString"),
          .C_MSGON_VAL(1),
          .C_OPTIMIZATION_MODE(0),
          .C_OVERFLOW_LOW(0),
          .C_PRELOAD_LATENCY(1),
          .C_PRELOAD_REGS(0),
          .C_PRIM_FIFO_TYPE(P_PRIM_FIFO_TYPE),
          .C_PROG_EMPTY_THRESH_ASSERT_VAL(2),
          .C_PROG_EMPTY_THRESH_ASSERT_VAL_AXIS(1022),
          .C_PROG_EMPTY_THRESH_ASSERT_VAL_RACH(30),
          .C_PROG_EMPTY_THRESH_ASSERT_VAL_RDCH(510),
          .C_PROG_EMPTY_THRESH_ASSERT_VAL_WACH(30),
          .C_PROG_EMPTY_THRESH_ASSERT_VAL_WDCH(510),
          .C_PROG_EMPTY_THRESH_ASSERT_VAL_WRCH(14),
          .C_PROG_EMPTY_THRESH_NEGATE_VAL(3),
          .C_PROG_EMPTY_TYPE(0),
          .C_PROG_EMPTY_TYPE_AXIS(5),
          .C_PROG_EMPTY_TYPE_RACH(5),
          .C_PROG_EMPTY_TYPE_RDCH(5),
          .C_PROG_EMPTY_TYPE_WACH(5),
          .C_PROG_EMPTY_TYPE_WDCH(5),
          .C_PROG_EMPTY_TYPE_WRCH(5),
          .C_PROG_FULL_THRESH_ASSERT_VAL(1022),
          .C_PROG_FULL_THRESH_ASSERT_VAL_AXIS(1023),
          .C_PROG_FULL_THRESH_ASSERT_VAL_RACH(31),
          .C_PROG_FULL_THRESH_ASSERT_VAL_RDCH(511),
          .C_PROG_FULL_THRESH_ASSERT_VAL_WACH(31),
          .C_PROG_FULL_THRESH_ASSERT_VAL_WDCH(511),
          .C_PROG_FULL_THRESH_ASSERT_VAL_WRCH(15),
          .C_PROG_FULL_THRESH_NEGATE_VAL(1021),
          .C_PROG_FULL_TYPE(0),
          .C_PROG_FULL_TYPE_AXIS(5),
          .C_PROG_FULL_TYPE_RACH(5),
          .C_PROG_FULL_TYPE_RDCH(5),
          .C_PROG_FULL_TYPE_WACH(5),
          .C_PROG_FULL_TYPE_WDCH(5),
          .C_PROG_FULL_TYPE_WRCH(5),
          .C_RACH_TYPE(((C_AXI_READ_FIFO_DEPTH != 0) && C_AXI_READ_FIFO_DELAY) ? 0 : 2),
          .C_RD_DATA_COUNT_WIDTH(10),
          .C_RD_DEPTH(1024),
          .C_RD_FREQ(1),
          .C_RD_PNTR_WIDTH(10),
          .C_RDCH_TYPE((C_AXI_READ_FIFO_DEPTH != 0) ? 0 : 2),
          .C_REG_SLICE_MODE_AXIS(0),
          .C_REG_SLICE_MODE_RACH(0),
          .C_REG_SLICE_MODE_RDCH(0),
          .C_REG_SLICE_MODE_WACH(0),
          .C_REG_SLICE_MODE_WDCH(0),
          .C_REG_SLICE_MODE_WRCH(0),
          .C_UNDERFLOW_LOW(0),
          .C_USE_COMMON_OVERFLOW(0),
          .C_USE_COMMON_UNDERFLOW(0),
          .C_USE_DEFAULT_SETTINGS(0),
          .C_USE_DOUT_RST(1),
          .C_USE_ECC(0),
          .C_USE_ECC_AXIS(0),
          .C_USE_ECC_RACH(0),
          .C_USE_ECC_RDCH(0),
          .C_USE_ECC_WACH(0),
          .C_USE_ECC_WDCH(0),
          .C_USE_ECC_WRCH(0),
          .C_USE_EMBEDDED_REG(0),
          .C_USE_FIFO16_FLAGS(0),
          .C_USE_FWFT_DATA_COUNT(0),
          .C_VALID_LOW(0),
          .C_WACH_TYPE(((C_AXI_WRITE_FIFO_DEPTH != 0) && C_AXI_WRITE_FIFO_DELAY) ? 0 : 2),
          .C_WDCH_TYPE((C_AXI_WRITE_FIFO_DEPTH != 0) ? 0 : 2),
          .C_WR_ACK_LOW(0),
          .C_WR_DATA_COUNT_WIDTH(10),
          .C_WR_DEPTH(1024),
          .C_WR_DEPTH_AXIS(1024),
          .C_WR_DEPTH_RACH(32),
          .C_WR_DEPTH_RDCH(C_AXI_READ_FIFO_DEPTH),
          .C_WR_DEPTH_WACH(32),
          .C_WR_DEPTH_WDCH(C_AXI_WRITE_FIFO_DEPTH),
          .C_WR_DEPTH_WRCH(16),
          .C_WR_FREQ(1),
          .C_WR_PNTR_WIDTH(10),
          .C_WR_PNTR_WIDTH_AXIS(10),
          .C_WR_PNTR_WIDTH_RACH(5),
          .C_WR_PNTR_WIDTH_RDCH((C_AXI_READ_FIFO_DEPTH> 1) ? f_ceil_log2(C_AXI_READ_FIFO_DEPTH) : 1),
          .C_WR_PNTR_WIDTH_WACH(5),
          .C_WR_PNTR_WIDTH_WDCH((C_AXI_WRITE_FIFO_DEPTH > 1) ? f_ceil_log2(C_AXI_WRITE_FIFO_DEPTH) : 1),
          .C_WR_PNTR_WIDTH_WRCH(4),
          .C_WR_RESPONSE_LATENCY(1),
          .C_WRCH_TYPE(2)
        )
        fifo_gen_inst (
          .s_aclk(ACLK),
          .s_aresetn(ARESETN),
          .s_axi_awid(S_AXI_AWID),
          .s_axi_awaddr(S_AXI_AWADDR),
          .s_axi_awlen(S_AXI_AWLEN),
          .s_axi_awsize(S_AXI_AWSIZE),
          .s_axi_awburst(S_AXI_AWBURST),
          .s_axi_awlock(S_AXI_AWLOCK),
          .s_axi_awcache(S_AXI_AWCACHE),
          .s_axi_awprot(S_AXI_AWPROT),
          .s_axi_awqos(S_AXI_AWQOS),
          .s_axi_awregion(S_AXI_AWREGION),
          .s_axi_awuser(S_AXI_AWUSER),
          .s_axi_awvalid(S_AXI_AWVALID),
          .s_axi_awready(S_AXI_AWREADY),
          .s_axi_wid({C_AXI_ID_WIDTH{1'b0}}),
          .s_axi_wdata(S_AXI_WDATA),
          .s_axi_wstrb(S_AXI_WSTRB),
          .s_axi_wlast(S_AXI_WLAST),
          .s_axi_wvalid(S_AXI_WVALID),
          .s_axi_wready(S_AXI_WREADY),
          .s_axi_bid(S_AXI_BID),
          .s_axi_bresp(S_AXI_BRESP),
          .s_axi_bvalid(S_AXI_BVALID),
          .s_axi_bready(S_AXI_BREADY),
          .m_axi_awid(M_AXI_AWID),
          .m_axi_awaddr(M_AXI_AWADDR),
          .m_axi_awlen(M_AXI_AWLEN),
          .m_axi_awsize(M_AXI_AWSIZE),
          .m_axi_awburst(M_AXI_AWBURST),
          .m_axi_awlock(M_AXI_AWLOCK),
          .m_axi_awcache(M_AXI_AWCACHE),
          .m_axi_awprot(M_AXI_AWPROT),
          .m_axi_awqos(M_AXI_AWQOS),
          .m_axi_awregion(M_AXI_AWREGION),
          .m_axi_awuser(M_AXI_AWUSER),
          .m_axi_awvalid(M_AXI_AWVALID),
          .m_axi_awready(M_AXI_AWREADY),
          .m_axi_wid(),
          .m_axi_wdata(M_AXI_WDATA),
          .m_axi_wstrb(M_AXI_WSTRB),
          .m_axi_wlast(M_AXI_WLAST),
          .m_axi_wvalid(M_AXI_WVALID),
          .m_axi_wready(M_AXI_WREADY),
          .m_axi_bid(M_AXI_BID),
          .m_axi_bresp(M_AXI_BRESP),
          .m_axi_bvalid(M_AXI_BVALID),
          .m_axi_bready(M_AXI_BREADY),
          .s_axi_arid(S_AXI_ARID),
          .s_axi_araddr(S_AXI_ARADDR),
          .s_axi_arlen(S_AXI_ARLEN),
          .s_axi_arsize(S_AXI_ARSIZE),
          .s_axi_arburst(S_AXI_ARBURST),
          .s_axi_arlock(S_AXI_ARLOCK),
          .s_axi_arcache(S_AXI_ARCACHE),
          .s_axi_arprot(S_AXI_ARPROT),
          .s_axi_arqos(S_AXI_ARQOS),
          .s_axi_arregion(S_AXI_ARREGION),
          .s_axi_arvalid(S_AXI_ARVALID),
          .s_axi_arready(S_AXI_ARREADY),
          .s_axi_rid(S_AXI_RID),
          .s_axi_rdata(S_AXI_RDATA),
          .s_axi_rresp(S_AXI_RRESP),
          .s_axi_rlast(S_AXI_RLAST),
          .s_axi_rvalid(S_AXI_RVALID),
          .s_axi_rready(S_AXI_RREADY),
          .m_axi_arid(M_AXI_ARID),
          .m_axi_araddr(M_AXI_ARADDR),
          .m_axi_arlen(M_AXI_ARLEN),
          .m_axi_arsize(M_AXI_ARSIZE),
          .m_axi_arburst(M_AXI_ARBURST),
          .m_axi_arlock(M_AXI_ARLOCK),
          .m_axi_arcache(M_AXI_ARCACHE),
          .m_axi_arprot(M_AXI_ARPROT),
          .m_axi_arqos(M_AXI_ARQOS),
          .m_axi_arregion(M_AXI_ARREGION),
          .m_axi_arvalid(M_AXI_ARVALID),
          .m_axi_arready(M_AXI_ARREADY),
          .m_axi_rid(M_AXI_RID),
          .m_axi_rdata(M_AXI_RDATA),
          .m_axi_rresp(M_AXI_RRESP),
          .m_axi_rlast(M_AXI_RLAST),
          .m_axi_rvalid(M_AXI_RVALID),
          .m_axi_rready(M_AXI_RREADY),
          .m_aclk(ACLK),
          .m_aclk_en(1'b1),
          .s_aclk_en(1'b1),
          .s_axi_wuser(S_AXI_WUSER),
          .s_axi_buser(S_AXI_BUSER),
          .m_axi_wuser(M_AXI_WUSER),
          .m_axi_buser(M_AXI_BUSER),
          .s_axi_aruser(S_AXI_ARUSER),
          .s_axi_ruser(S_AXI_RUSER),
          .m_axi_aruser(M_AXI_ARUSER),
          .m_axi_ruser(M_AXI_RUSER),
          .almost_empty(),
          .almost_full(),
          .axis_data_count(),
          .axis_dbiterr(),
          .axis_injectdbiterr(1'b0),
          .axis_injectsbiterr(1'b0),
          .axis_overflow(),
          .axis_prog_empty(),
          .axis_prog_empty_thresh(10'b0),
          .axis_prog_full(),
          .axis_prog_full_thresh(10'b0),
          .axis_rd_data_count(),
          .axis_sbiterr(),
          .axis_underflow(),
          .axis_wr_data_count(),
          .axi_ar_data_count(),
          .axi_ar_dbiterr(),
          .axi_ar_injectdbiterr(1'b0),
          .axi_ar_injectsbiterr(1'b0),
          .axi_ar_overflow(),
          .axi_ar_prog_empty(),
          .axi_ar_prog_empty_thresh(5'b0),
          .axi_ar_prog_full(),
          .axi_ar_prog_full_thresh(5'b0),
          .axi_ar_rd_data_count(),
          .axi_ar_sbiterr(),
          .axi_ar_underflow(),
          .axi_ar_wr_data_count(),
          .axi_aw_data_count(),
          .axi_aw_dbiterr(),
          .axi_aw_injectdbiterr(1'b0),
          .axi_aw_injectsbiterr(1'b0),
          .axi_aw_overflow(),
          .axi_aw_prog_empty(),
          .axi_aw_prog_empty_thresh(5'b0),
          .axi_aw_prog_full(),
          .axi_aw_prog_full_thresh(5'b0),
          .axi_aw_rd_data_count(),
          .axi_aw_sbiterr(),
          .axi_aw_underflow(),
          .axi_aw_wr_data_count(),
          .axi_b_data_count(),
          .axi_b_dbiterr(),
          .axi_b_injectdbiterr(1'b0),
          .axi_b_injectsbiterr(1'b0),
          .axi_b_overflow(),
          .axi_b_prog_empty(),
          .axi_b_prog_empty_thresh(4'b0),
          .axi_b_prog_full(),
          .axi_b_prog_full_thresh(4'b0),
          .axi_b_rd_data_count(),
          .axi_b_sbiterr(),
          .axi_b_underflow(),
          .axi_b_wr_data_count(),
          .axi_r_data_count(),
          .axi_r_dbiterr(),
          .axi_r_injectdbiterr(1'b0),
          .axi_r_injectsbiterr(1'b0),
          .axi_r_overflow(),
          .axi_r_prog_empty(),
          .axi_r_prog_empty_thresh({P_READ_FIFO_DEPTH_LOG{1'b0}}),
          .axi_r_prog_full(),
          .axi_r_prog_full_thresh({P_READ_FIFO_DEPTH_LOG{1'b0}}),
          .axi_r_rd_data_count(),
          .axi_r_sbiterr(),
          .axi_r_underflow(),
          .axi_r_wr_data_count(),
          .axi_w_data_count(),
          .axi_w_dbiterr(),
          .axi_w_injectdbiterr(1'b0),
          .axi_w_injectsbiterr(1'b0),
          .axi_w_overflow(),
          .axi_w_prog_empty(),
          .axi_w_prog_empty_thresh({P_WRITE_FIFO_DEPTH_LOG{1'b0}}),
          .axi_w_prog_full(),
          .axi_w_prog_full_thresh({P_WRITE_FIFO_DEPTH_LOG{1'b0}}),
          .axi_w_rd_data_count(),
          .axi_w_sbiterr(),
          .axi_w_underflow(),
          .axi_w_wr_data_count(),
          .backup(1'b0),
          .backup_marker(1'b0),
          .clk(1'b0),
          .data_count(),
          .dbiterr(),
          .din(18'b0),
          .dout(),
          .empty(),
          .full(),
          .injectdbiterr(1'b0),
          .injectsbiterr(1'b0),
          .int_clk(1'b0),
          .m_axis_tdata(),
          .m_axis_tdest(),
          .m_axis_tid(),
          .m_axis_tkeep(),
          .m_axis_tlast(),
          .m_axis_tready(1'b0),
          .m_axis_tstrb(),
          .m_axis_tuser(),
          .m_axis_tvalid(),
          .overflow(),
          .prog_empty(),
          .prog_empty_thresh(10'b0),
          .prog_empty_thresh_assert(10'b0),
          .prog_empty_thresh_negate(10'b0),
          .prog_full(),
          .prog_full_thresh(10'b0),
          .prog_full_thresh_assert(10'b0),
          .prog_full_thresh_negate(10'b0),
          .rd_clk(1'b0),
          .rd_data_count(),
          .rd_en(1'b0),
          .rd_rst(1'b0),
          .rst(1'b0),
          .sbiterr(),
          .srst(1'b0),
          .s_axis_tdata(64'b0),
          .s_axis_tdest(4'b0),
          .s_axis_tid(8'b0),
          .s_axis_tkeep(4'b0),
          .s_axis_tlast(1'b0),
          .s_axis_tready(),
          .s_axis_tstrb(4'b0),
          .s_axis_tuser(4'b0),
          .s_axis_tvalid(1'b0),
          .underflow(),
          .valid(),
          .wr_ack(),
          .wr_clk(1'b0),
          .wr_data_count(),
          .wr_en(1'b0),
          .wr_rst(1'b0)
      );
    end
  endgenerate
endmodule


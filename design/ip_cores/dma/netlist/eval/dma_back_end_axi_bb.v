// -------------------------------------------------------------------------
//
//  PROJECT: PCI Express Core
//  COMPANY: Northwest Logic, Inc.
//
// ------------------------- CONFIDENTIAL ----------------------------------
//
//                 Copyright 2010 by Northwest Logic, Inc.
//
//  All rights reserved.  No part of this source code may be reproduced or
//  transmitted in any form or by any means, electronic or mechanical,
//  including photocopying, recording, or any information storage and
//  retrieval system, without permission in writing from Northest Logic, Inc.
//
//  Further, no use of this source code is permitted in any form or means
//  without a valid, written license agreement with Northwest Logic, Inc.
//
//                         Northwest Logic, Inc.
//                  1100 NW Compton Drive, Suite 100
//                      Beaverton, OR 97006, USA
//
//                       Ph.  +1 503 533 5800
//                       Fax. +1 503 533 5900
//                          www.nwlogic.com
//
// -------------------------------------------------------------------------

`timescale 1ps / 1ps



// -----------------------
// -- Module Definition --
// -----------------------

module dma_back_end_axi (

    // PCI Express Interface
    rst_n,
    clk,
    testmode,

    // PCI Express Core Tx & Rx Interface
    tx_buf_av,
    tx_err_drop,
    s_axis_tx_tready,
    s_axis_tx_tdata,
    s_axis_tx_tkeep,
    s_axis_tx_tuser,
    s_axis_tx_tlast,
    s_axis_tx_tvalid,

    m_axis_rx_tdata,
    m_axis_rx_tkeep,
    m_axis_rx_tlast,
    m_axis_rx_tvalid,
    m_axis_rx_tready,
    m_axis_rx_tuser,
    rx_np_ok,
    // Management Interface
    mgmt_mst_en,
    mgmt_msi_en,
    mgmt_msix_en,
    mgmt_msix_table_offset,
    mgmt_msix_pba_offset,
    mgmt_msix_function_mask,
    mgmt_max_payload_size,
    mgmt_max_rd_req_size,
    mgmt_clk_period_in_ns,
    mgmt_version,
    mgmt_pcie_version,
    mgmt_user_version,
    mgmt_cfg_id,
    mgmt_interrupt,
    user_interrupt,
    cfg_interrupt_rdy,
    cfg_interrupt_assert,
    cfg_interrupt,
    cfg_interrupt_di,
    cfg_interrupt_do,

    mgmt_ch_infinite,
    mgmt_cd_infinite,
    mgmt_ch_credits,
    mgmt_cd_credits,

    mgmt_adv_cpl_timeout_disable,
    mgmt_adv_cpl_timeout_value,
    mgmt_cpl_timeout_disable,
    mgmt_cpl_timeout_value,

    err_cpl_to_closed_tag,
    err_cpl_timeout,
    cpl_tag_active,

    err_pkt_poison,
    err_pkt_ur,
    err_pkt_header,

    // DMA System to Card Engine Interface
    s2c_cfg_constants,
    s2c_areset_n,
    s2c_aclk,
    s2c_fifo_addr_n,
    s2c_awvalid,
    s2c_awready,
    s2c_awaddr,
    s2c_awlen,
    s2c_awusereop,
    s2c_awsize,
    s2c_wvalid,
    s2c_wready,
    s2c_wdata,
    s2c_wstrb,
    s2c_wlast,
    s2c_wusereop,
    s2c_wusercontrol,
    s2c_bvalid,
    s2c_bready,
    s2c_bresp,

    // DMA Card to System Engine Interface
    c2s_cfg_constants,
    c2s_areset_n,
    c2s_aclk,
    c2s_fifo_addr_n,
    c2s_arvalid,
    c2s_arready,
    c2s_araddr,
    c2s_arlen,
    c2s_arsize,
    c2s_rvalid,
    c2s_rready,
    c2s_rdata,
    c2s_rresp,
    c2s_rlast,
    c2s_ruserstatus,
    c2s_ruserstrb,

    // AXI Master Interface
    m_areset_n,
    m_aclk,
    m_awvalid,
    m_awready,
    m_awaddr,
    m_wvalid,
    m_wready,
    m_wdata,
    m_wstrb,
    m_bvalid,
    m_bready,
    m_bresp,
    m_arvalid,
    m_arready,
    m_araddr,
    m_rvalid,
    m_rready,
    m_rdata,
    m_rresp,
    m_interrupt,

    // AXI Target Interface
    t_areset_n,
    t_aclk,
    t_awvalid,
    t_awready,
    t_awregion,
    t_awaddr,
    t_awlen,
    t_awsize,
    t_wvalid,
    t_wready,
    t_wdata,
    t_wstrb,
    t_wlast,
    t_bvalid,
    t_bready,
    t_bresp,
    t_arvalid,
    t_arready,
    t_arregion,
    t_araddr,
    t_arlen,
    t_arsize,
    t_rvalid,
    t_rready,
    t_rdata,
    t_rresp,
    t_rlast

);



// ----------------
// -- Parameters --
// ----------------

// Note: localparam values are constants and must not be modfied

localparam  EXPRESSO_DMA_VERSION        = 32'h00_02_00_00;

parameter   NUM_S2C                     = 2;    // Number of S2C DMA Engines to Implement
parameter   NUM_C2S                     = 2;    // Number of C2S DMA Engines to Implement

localparam  MAX_S2C                     = 8;    // Maximum number of S2C Engines Supported - Do not modify
localparam  MAX_C2S                     = 8;    // Maximum number of C2S Engines Supported - Do not modify

localparam  INT_VECTORS                 = NUM_S2C + NUM_C2S + 1;

parameter   USER_CONTROL_WIDTH          = 64;
parameter   USER_STATUS_WIDTH           = 64;

// PCI Express Parameters
parameter   REG_ADDR_WIDTH              = 13;   // Register BAR is 64KBytes
localparam  CORE_DATA_WIDTH             = 64;   // Width of input and output data
localparam  CORE_BE_WIDTH               = CORE_DATA_WIDTH / 8;

localparam  XIL_DATA_WIDTH              = 64;
localparam  XIL_STRB_WIDTH              = 8;

localparam  RQ_TAG_WIDTH                = 3;    // Number of tag bits implemented by the S2C DMA Engine Reorder Queues

localparam  NUM_TAGS                    = (1 << RQ_TAG_WIDTH) + 2;  // Number of tags implemented by Completion Monitor; must be 2^RQ_TAG_WIDTH+2
localparam  TAG_WIDTH                   = RQ_TAG_WIDTH + 1;         // Width of tags implemented by Completion Monitor

localparam  CARD_ADDR_WIDTH             = 64;   // Maximum DMA Card address width
localparam  BYTE_COUNT_WIDTH            = 13;
localparam  DESC_ADDR_WIDTH             = 64;   // Maximum Descriptor Pointer address width
localparam  DESC_STATUS_WIDTH           = 160;
localparam  DESC_WIDTH                  = 256;

// AXI Parameters
localparam  AXI_LEN_WIDTH               = 4;    // Sets maximum AXI burst size; supported values 4 (AXI3/AXI4) or 8 (AXI4); For AXI_DATA_WIDTH==256 must be 4 so a 4 KB boundary is not crossed
localparam  AXI_MAX_SIMUL_WIDTH         = 4;    // Maximum number of simultaneously pending AXI transactions == 2^AXI_MAX_SIMUL_WIDTH
localparam  AXI_ADDR_WIDTH              = 36;   // Width of AXI DMA address ports
localparam  T_AXI_ADDR_WIDTH            = 32;   // Width of AXI Target address ports
localparam  AXI_DATA_WIDTH              = 64;   // AXI Data Width
localparam  AXI_BE_WIDTH                = AXI_DATA_WIDTH / 8;

// Register byte addresses 0x1FFF-0x0000 are reserved for up to 32 System to Card DMA Register Blocks;
//   Each Register Block is 256 bytes; the first Register Block must be placed at 0x0000; subsequent
//   Register Blocks are placed every 256 bytes; software can determine the number of present
//   Register Blocks by reading the Capabilities register at all of the possible locations;
//   define the Register Block offsets in terms of CORE_DATA_WIDTH addresses
localparam  REG_BASE_ADDR_S2C           = 32'h00;
localparam  REG_BASE_ADDR_S2C_INC       = 32'h20;

// Register byte addresses 0x3FFF-0x2000 are reserved for up to 32 Card to System DMA Register Blocks;
//   Each Register Block is 256 bytes; the first Register Block must be placed at 0x2000; subsequent
//   Register Blocks are placed every 256 bytes; software can determine the number of present
//   Register Blocks by reading the Capabilities register at all of the possible locations
// reg_wr_addr and reg_rd_addr are CORE_DATA_WIDTH addresses rather than byte addresses;
//   define the Register Block offsets in terms of CORE_DATA_WIDTH
localparam  REG_BASE_ADDR_C2S           = 32'h400;
localparam  REG_BASE_ADDR_C2S_INC       = 32'h20;

// The DMA Common Register Block is at 0x4000 offset into BAR0
localparam  REG_BASE_ADDR_COMMON        = 32'h800;



// ----------------------
// -- Port Definitions --
// ----------------------

input                                       rst_n;
input                                       clk;
input                                       testmode;

// PCI Express Core Tx & Rx Interface
input   [5:0]                               tx_buf_av;
input                                       tx_err_drop;
input                                       s_axis_tx_tready;
output  [XIL_DATA_WIDTH-1:0]                s_axis_tx_tdata;
output  [XIL_STRB_WIDTH-1:0]                s_axis_tx_tkeep;
output  [3:0]                               s_axis_tx_tuser;
output                                      s_axis_tx_tlast;
output                                      s_axis_tx_tvalid;

input   [XIL_DATA_WIDTH-1:0]                m_axis_rx_tdata;
input   [XIL_STRB_WIDTH-1:0]                m_axis_rx_tkeep;
input                                       m_axis_rx_tlast;
input                                       m_axis_rx_tvalid;
output                                      m_axis_rx_tready;
input   [21:0]                              m_axis_rx_tuser;
output                                      rx_np_ok;
// Management Interface
input                                       mgmt_mst_en;
input                                       mgmt_msi_en;
input                                       mgmt_msix_en;
input   [31:0]                              mgmt_msix_table_offset;
input   [31:0]                              mgmt_msix_pba_offset;
input                                       mgmt_msix_function_mask;
input   [2:0]                               mgmt_max_payload_size;
input   [2:0]                               mgmt_max_rd_req_size;
input   [7:0]                               mgmt_clk_period_in_ns;
output  [31:0]                              mgmt_version;
input   [31:0]                              mgmt_pcie_version;
input   [31:0]                              mgmt_user_version;
input   [15:0]                              mgmt_cfg_id;
input   [31:0]                              mgmt_interrupt;
input                                       user_interrupt;
input                                       cfg_interrupt_rdy;
output                                      cfg_interrupt_assert;
output                                      cfg_interrupt;
output  [7:0]                               cfg_interrupt_di;
input   [7:0]                               cfg_interrupt_do;

input                                       mgmt_ch_infinite;
input                                       mgmt_cd_infinite;
input   [7:0]                               mgmt_ch_credits;
input   [11:0]                              mgmt_cd_credits;

output                                      mgmt_adv_cpl_timeout_disable;
output  [3:0]                               mgmt_adv_cpl_timeout_value;
input                                       mgmt_cpl_timeout_disable;
input   [3:0]                               mgmt_cpl_timeout_value;

output                                      err_cpl_to_closed_tag;
output                                      err_cpl_timeout;
output                                      cpl_tag_active;

output                                      err_pkt_poison;
output                                      err_pkt_ur;
output  [127:0]                             err_pkt_header;

// DMA System to Card Engine Interface
input   [(NUM_S2C*64)                -1:0]  s2c_cfg_constants;
output  [ NUM_S2C                    -1:0]  s2c_areset_n;
input   [ NUM_S2C                    -1:0]  s2c_aclk;
input   [ NUM_S2C                    -1:0]  s2c_fifo_addr_n;
output  [ NUM_S2C                    -1:0]  s2c_awvalid;
input   [ NUM_S2C                    -1:0]  s2c_awready;
output  [(NUM_S2C*AXI_ADDR_WIDTH)    -1:0]  s2c_awaddr;
output  [(NUM_S2C*AXI_LEN_WIDTH)     -1:0]  s2c_awlen;
output  [ NUM_S2C                    -1:0]  s2c_awusereop;
output  [(NUM_S2C*3)                 -1:0]  s2c_awsize;
output  [ NUM_S2C                    -1:0]  s2c_wvalid;
input   [ NUM_S2C                    -1:0]  s2c_wready;
output  [(NUM_S2C*AXI_DATA_WIDTH)    -1:0]  s2c_wdata;
output  [(NUM_S2C*AXI_BE_WIDTH)      -1:0]  s2c_wstrb;
output  [ NUM_S2C                    -1:0]  s2c_wlast;
output  [ NUM_S2C                    -1:0]  s2c_wusereop;
output  [(NUM_S2C*USER_CONTROL_WIDTH)-1:0]  s2c_wusercontrol;
input   [ NUM_S2C                    -1:0]  s2c_bvalid;
output  [ NUM_S2C                    -1:0]  s2c_bready;
input   [(NUM_S2C*2)                 -1:0]  s2c_bresp;

// DMA Card to System Engine Interface
input   [(NUM_C2S*64)                -1:0]  c2s_cfg_constants;
output  [ NUM_C2S                    -1:0]  c2s_areset_n;
input   [ NUM_C2S                    -1:0]  c2s_aclk;
input   [ NUM_C2S                    -1:0]  c2s_fifo_addr_n;
output  [ NUM_C2S                    -1:0]  c2s_arvalid;
input   [ NUM_C2S                    -1:0]  c2s_arready;
output  [(NUM_C2S*AXI_ADDR_WIDTH)    -1:0]  c2s_araddr;
output  [(NUM_C2S*AXI_LEN_WIDTH)     -1:0]  c2s_arlen;
output  [(NUM_C2S*3)                 -1:0]  c2s_arsize;
input   [ NUM_C2S                    -1:0]  c2s_rvalid;
output  [ NUM_C2S                    -1:0]  c2s_rready;
input   [(NUM_C2S*AXI_DATA_WIDTH)    -1:0]  c2s_rdata;
input   [(NUM_C2S*2)                 -1:0]  c2s_rresp;
input   [ NUM_C2S                    -1:0]  c2s_rlast;
input   [(NUM_C2S*USER_STATUS_WIDTH) -1:0]  c2s_ruserstatus;
input   [(NUM_C2S*AXI_BE_WIDTH)      -1:0]  c2s_ruserstrb;

// AXI Master Interface
input                                       m_areset_n;
input                                       m_aclk;
input                                       m_awvalid;
output                                      m_awready;
input   [15:0]                              m_awaddr;
input                                       m_wvalid;
output                                      m_wready;
input   [31:0]                              m_wdata;
input   [3:0]                               m_wstrb;
output                                      m_bvalid;
input                                       m_bready;
output  [1:0]                               m_bresp;
input                                       m_arvalid;
output                                      m_arready;
input   [15:0]                              m_araddr;
output                                      m_rvalid;
input                                       m_rready;
output  [31:0]                              m_rdata;
output  [1:0]                               m_rresp;
output  [INT_VECTORS-1:0]                   m_interrupt;

// AXI Target Interface
input                                       t_areset_n;
input                                       t_aclk;
output                                      t_awvalid;
input                                       t_awready;
output  [2:0]                               t_awregion;
output  [T_AXI_ADDR_WIDTH-1:0]              t_awaddr;
output  [AXI_LEN_WIDTH-1:0]                 t_awlen;
output  [2:0]                               t_awsize;
output                                      t_wvalid;
input                                       t_wready;
output  [AXI_DATA_WIDTH-1:0]                t_wdata;
output  [AXI_BE_WIDTH-1:0]                  t_wstrb;
output                                      t_wlast;
input                                       t_bvalid;
output                                      t_bready;
input   [1:0]                               t_bresp;
output                                      t_arvalid;
input                                       t_arready;
output  [2:0]                               t_arregion;
output  [T_AXI_ADDR_WIDTH-1:0]              t_araddr;
output  [AXI_LEN_WIDTH-1:0]                 t_arlen;
output  [2:0]                               t_arsize;
input                                       t_rvalid;
output                                      t_rready;
input   [AXI_DATA_WIDTH-1:0]                t_rdata;
input   [1:0]                               t_rresp;
input                                       t_rlast;
endmodule

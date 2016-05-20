//-----------------------------------------------------------------------------
//
// (c) Copyright 2009 Xilinx, Inc. All rights reserved.
//
// This file contains confidential and proprietary information of Xilinx, Inc.
// and is protected under U.S. and international copyright and other
// intellectual property laws.
//
// DISCLAIMER
//
// This disclaimer is not a license and does not grant any rights to the
// materials distributed herewith. Except as otherwise provided in a valid
// license issued to you by Xilinx, and to the maximum extent permitted by
// applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
// FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
// IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
// MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
// and (2) Xilinx shall not be liable (whether in contract or tort, including
// negligence, or under any other theory of liability) for any loss or damage
// of any kind or nature related to, arising under or in connection with these
// materials, including for any direct, or any indirect, special, incidental,
// or consequential loss or damage (including loss of data, profits, goodwill,
// or any type of loss or damage suffered as a result of any action brought by
// a third party) even if such damage or loss was reasonably foreseeable or
// Xilinx had been advised of the possibility of the same.
//
// CRITICAL APPLICATIONS
//
// Xilinx products are not designed or intended to be fail-safe, or for use in
// any application requiring fail-safe performance, such as life-support or
// safety devices or systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any other
// applications that could lead to death, personal injury, or severe property
// or environmental damage (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and liability of any use of
// Xilinx products in Critical Applications, subject only to applicable laws
// and regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
// AT ALL TIMES.
//
//-----------------------------------------------------------------------------
// Project    : Kintex-7 Integrated Block for PCI Express
// File       : board.v
// Description:  Top level testbench
//
//------------------------------------------------------------------------------

`timescale 1ps/1ps

`include "board_common.v"
`include "dut_defines.v"
`include "user_defines.v"

module board;

parameter          VENDOR_ID                    = 16'h10EE;

parameter          SUBSYSTEM_VENDOR_ID          = 16'h10EE;

parameter          DEVICE_ID                    = 16'h0007;

parameter          CLASS_CODE                   = 24'h050000;

parameter          REF_CLK_FREQ                 = 0;      // 0 - 100 MHz, 1 - 125 MHz,  2 - 250 MHz

`ifdef PCIEx8
parameter          CAPABILITY_LINK_WIDTH        = 6'h08;  // 1 - x1, 2 - x2, 4 - x4, 8 - x8  
`else
parameter          CAPABILITY_LINK_WIDTH        = 6'h04;  // 1 - x1, 2 - x2, 4 - x4, 8 - x8  
`endif

parameter          PCIE_EXT_CLK                 = "TRUE";        
parameter          USER_CLK_FREQ                = 3;      // 0 - 31.25 MHz, 1 - 62.5 MHz,  2 - 125 MHz, 3 - 250 MHz

parameter          CAPABILITY_MAX_PAYLOAD_SIZE  = 3'h2;   // 0 - 128B, 1 - 256B, 2 - 512B, 3 - 1024B

`ifdef GEN2_CAP
parameter          UPCONFIG_CAPABLE             = "TRUE"; 
parameter          LINK_CAP_MAX_LINK_SPEED      = 4'h2;
parameter          LINK_CTRL2_TARGET_LINK_SPEED = 4'h2;
`else
parameter          UPCONFIG_CAPABLE             = "FALSE"; 
parameter          LINK_CAP_MAX_LINK_SPEED      = 4'h1;
parameter          LINK_CTRL2_TARGET_LINK_SPEED = 4'h1;
`endif

parameter          VC0_TX_LASTPACKET            = 29;
parameter          VC0_RX_RAM_LIMIT             = 13'h7FF;
parameter          VC0_CPL_INFINITE             = "FALSE"; 
parameter          VC0_TOTAL_CREDITS_PD         = 437;
parameter          VC0_TOTAL_CREDITS_CD         = 461;

parameter          BAR0                         = 32'hFFFFFF80;
parameter          BAR1                         = 32'h00000000;
parameter          BAR2                         = 32'hFFFFFF80;
parameter          BAR3                         = 32'h00000000;
parameter          BAR4                         = 32'h00000000;
parameter          BAR5                         = 32'h00000000;
parameter          EXPANSION_ROM                = 32'h00000000;
parameter          LINK_CAP_DLL_LINK_ACTIVE_REPORTING_CAP = "TRUE" ; 
parameter          DQ_WIDTH                     = 64;
                                    
localparam         REF_CLK_HALF_CYCLE = (REF_CLK_FREQ == 0) ? 5000 :
                                        (REF_CLK_FREQ == 1) ? 4000 :
                                        (REF_CLK_FREQ == 2) ? 2000 : 0;
`ifdef GEN2_CAP
  localparam USRCLK_FREQ              = 3;
`else
  localparam USRCLK_FREQ              = (CAPABILITY_LINK_WIDTH == 4) ? 2 : 3; 
`endif


integer            i_;

localparam MEMORY_WIDTH = 8;
localparam NUM_COMP = DQ_WIDTH/MEMORY_WIDTH;
localparam CS_WIDTH = 1;
localparam CKE_WIDTH = 1;
localparam CK_WIDTH =1;
localparam nCS_PER_RANK = 1;
//
// System reset
//

reg                sys_reset_n;

//
// System clocks
//

reg rp_sys_clk;
reg ep_sys_clk;
reg sys_clk_i;
wire sys_clk_p;
wire sys_clk_n;
//reg clk_ref_i;
//wire clk_ref_p;
//wire clk_ref_n;

wire [13:0]                               ddr_addr;               
wire [2:0]                                ddr_ba;                 
wire                                      ddr_cas_n;              
wire [CK_WIDTH-1:0]                       ddr_ck;                 
wire [CK_WIDTH-1:0]                       ddr_ck_n;               
wire [CKE_WIDTH-1:0]                      ddr_cke;                
wire [(CS_WIDTH*nCS_PER_RANK)-1:0]        ddr_cs_n;               
wire [(CS_WIDTH*nCS_PER_RANK)-1:0]        ddr_odt;               
wire [7:0]                                ddr_dm;                 
wire [63:0]                               ddr_dq;                 
wire [7:0]                                ddr_dqs;                
wire [7:0]                                ddr_dqs_n;              
wire                                      ddr_ras_n;              
wire                                      ddr_reset_n;            
wire                                      ddr_we_n;               
//
// PCI-Express Serial Interconnect
//

wire  [(CAPABILITY_LINK_WIDTH - 1):0]  ep_pci_exp_txn;
wire  [(CAPABILITY_LINK_WIDTH - 1):0]  ep_pci_exp_txp;
wire  [(CAPABILITY_LINK_WIDTH - 1):0]  rp_pci_exp_txn;
wire  [(CAPABILITY_LINK_WIDTH - 1):0]  rp_pci_exp_txp;

`ifdef USE_PIPE_SIM
  parameter PIPE_SIM = "TRUE";
  defparam board.dut.pcie_inst.inst.inst.PIPE_SIM_MODE = "TRUE";
  defparam board.RP.rport.PIPE_SIM_MODE = "TRUE";
`else
  parameter PIPE_SIM = "FALSE";
  defparam board.dut.pcie_inst.inst.inst.PIPE_SIM_MODE = "FALSE";
  defparam board.RP.rport.PIPE_SIM_MODE = "FALSE";
`endif

//- 5000 * 1ps = 5ns, 200 MHz clock - used for DDR3 MCB
 `define CLKIN_PERIOD 5000

k7_pcie_dma_ddr3_base #(
  .PL_FAST_TRAIN  ("TRUE"),
  .PCIE_EXT_CLK("TRUE")
)
dut (

    .perst_n(sys_reset_n),        // PCI Express slot PERST# reset signal
        
    .pcie_clk_p(ep_sys_clk),     // PCIe differential reference clock input
    .pcie_clk_n(~ep_sys_clk),     // PCIe differential reference clock input

    .tx_p(ep_pci_exp_txp),           // PCIe differential transmit output
    .tx_n(ep_pci_exp_txn),           // PCIe differential transmit output

    .rx_p(rp_pci_exp_txp),           // PCIe differential receive output
    .rx_n(rp_pci_exp_txn),           // PCIe differential receive output

`ifdef USE_DDR3_FIFO
     .sys_clk_p         (sys_clk_p),
     .sys_clk_n         (sys_clk_n),

     .ddr_addr          (ddr_addr),
     .ddr_ba            (ddr_ba),
     .ddr_cas_n         (ddr_cas_n),
     .ddr_ck_p          (ddr_ck),
     .ddr_ck_n          (ddr_ck_n),
     .ddr_cke           (ddr_cke),
     .ddr_cs_n          (ddr_cs_n),
     .ddr_dm            (ddr_dm),
     .ddr_dq            (ddr_dq),
     .ddr_dqs_p         (ddr_dqs),
     .ddr_dqs_n         (ddr_dqs_n),
     .ddr_odt           (ddr_odt),
     .ddr_ras_n         (ddr_ras_n),
     .ddr_reset_n       (ddr_reset_n),
     .ddr_we_n          (ddr_we_n),
`endif     
     .led ()            // Diagnostic LEDs
 
    );
   
//
// PCI-Express Model Root Port Instance
//

 
xilinx_pcie_2_1_rport_7x # (

  .REF_CLK_FREQ(REF_CLK_FREQ),
  .PL_FAST_TRAIN("TRUE"),
  .PIPE_SIM_MODE("TRUE"),
  .LINK_CAP_MAX_LINK_WIDTH(CAPABILITY_LINK_WIDTH),
  .DEVICE_ID(16'h7100),
  .C_DATA_WIDTH(64),
  .LINK_CAP_MAX_LINK_SPEED(LINK_CAP_MAX_LINK_SPEED),
  .LINK_CTRL2_TARGET_LINK_SPEED(LINK_CTRL2_TARGET_LINK_SPEED),
  .DEV_CAP_MAX_PAYLOAD_SUPPORTED(CAPABILITY_MAX_PAYLOAD_SIZE),
  .VC0_TX_LASTPACKET(VC0_TX_LASTPACKET),
  .VC0_RX_RAM_LIMIT(VC0_RX_RAM_LIMIT),
  .VC0_CPL_INFINITE("TRUE"),
  .VC0_TOTAL_CREDITS_PD(VC0_TOTAL_CREDITS_PD),
  .VC0_TOTAL_CREDITS_CD(VC0_TOTAL_CREDITS_CD),
  .USER_CLK_FREQ(USER_CLK_FREQ),
  .USRCLK_FREQ(USRCLK_FREQ),
  //.UPCONFIG_CAPABLE(UPCONFIG_CAPABLE),
  .LINK_CAP_DLL_LINK_ACTIVE_REPORTING_CAP(LINK_CAP_DLL_LINK_ACTIVE_REPORTING_CAP)

)
RP (

  // SYS Inteface
  .sys_clk(rp_sys_clk),
  .sys_rst_n(sys_reset_n),

  // PCI-Express Interface
  .pci_exp_txn(rp_pci_exp_txn),
  .pci_exp_txp(rp_pci_exp_txp),
  .pci_exp_rxn(ep_pci_exp_txn),
  .pci_exp_rxp(ep_pci_exp_txp)

);


//
// Instantiate memories
//

`ifdef USE_DDR3_FIFO
genvar i,r;
  for (r = 0; r < CS_WIDTH; r = r + 1) begin: mem_rnk
    for (i = 0; i < NUM_COMP; i = i + 1) begin: gen_mem
      
          ddr3_model #(
            .DEBUG (0)
          ) u_comp_ddr3
            (
             .rst_n   (ddr_reset_n),
             .ck      (ddr_ck[(i*MEMORY_WIDTH)/72]),
             .ck_n    (ddr_ck_n[(i*MEMORY_WIDTH)/72]),
             .cke     (ddr_cke[((i*MEMORY_WIDTH)/72)+(nCS_PER_RANK*r)]),
             .cs_n    (ddr_cs_n[((i*MEMORY_WIDTH)/72)+(nCS_PER_RANK*r)]),
             .ras_n   (ddr_ras_n),
             .cas_n   (ddr_cas_n),
             .we_n    (ddr_we_n),
             .dm_tdqs (ddr_dm[i]),
             .ba      (ddr_ba),
             .addr    (ddr_addr),
             .dq      (ddr_dq[MEMORY_WIDTH*(i+1)-1:MEMORY_WIDTH*(i)]),
             .dqs     (ddr_dqs[i]),
             .dqs_n   (ddr_dqs_n[i]),
             .tdqs_n  (),
             .odt     (ddr_odt[((i*MEMORY_WIDTH)/72)+(nCS_PER_RANK*r)])
             );
      end
    end  
`endif

// 100 MHz clock for PCIe 
 initial 
 begin
   rp_sys_clk = 1'b0;
   forever #(5000) rp_sys_clk = ~rp_sys_clk;
 end

 initial 
 begin
   ep_sys_clk = 1'b0;
   forever #(5000) ep_sys_clk = ~ep_sys_clk;
 end

 // MCB clock generation
  initial
  begin
    sys_clk_i = 1'b0;
  
    forever #(`CLKIN_PERIOD/2.0)sys_clk_i =  ~sys_clk_i;
  end
  
  assign sys_clk_p = sys_clk_i;
  assign sys_clk_n = ~sys_clk_i;

  `include "pipe_interconnect.v"

initial begin

  $display("[%t] : System Reset Asserted...", $realtime);

  sys_reset_n = 1'b0;

  for (i_ = 0; i_ < 100; i_ = i_ + 1) begin

    @(posedge rp_sys_clk);

  end

  $display("[%t] : System Reset De-asserted...", $realtime);

  sys_reset_n = 1'b1;

end

endmodule // BOARD

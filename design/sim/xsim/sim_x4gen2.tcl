# Vivado Launch Script

#### Change design settings here #######
set design k7_pcie_dma_ddr3_base_x4gen2
set top board 
set device xc7k325t-2-ffg900
set proj_dir vivado_sim_x4gen2
########################################

set CurrWrkDir [pwd]

# Project Settings
create_project -name ${design} -force -dir "./${proj_dir}" -part ${device}
set_property top ${top} [get_filesets sim_1]
set_property include_dirs { ../../tb ../../tb/dsport ../include ../../source/gen_chk ./} [get_filesets sim_1]
set_property verilog_define {{SIMULATION=1} {USE_DDR3_FIFO=1} {GEN2_CAP=1} {x1Gb=1} {sg125=1} {x8=1} {USE_7SERIES=1} {USE_PIPE_SIM=1}} [get_filesets sim_1]

set_property runtime {} [get_filesets sim_1]
set_property -name xsim.more_options -value {-testplusarg TESTNAME=basic_test} -objects [get_filesets sim_1]

# Project Design Files from IP Catalog (comment out IPs using legacy Coregen cores)
import_ip -file {../../ip_catalog/pcie_7x_x4gen2.xci} -name pcie_7x 
import_ip -file {../../ip_catalog/axi_ic.xci} -name axi_interconnect_4m_1s
import_ip -file {../../ip_catalog/axis_async_fifo.xci} -name axis_async_fifo  
import_ip -file {../../ip_catalog/mig.xci} -name mig_7x

#- NWL Packet DMA source
read_verilog "../../ip_cores/dma/models/dma_back_end_axi/dma_back_end_axi_model.v"
read_verilog "../../source/virtual_packet_fifo/packetizer/control_word_insert.v"
read_verilog "../../source/virtual_packet_fifo/packetizer/control_word_strip.v"
read_verilog "../../source/virtual_packet_fifo/vfifo_controller/address_manager.v"
read_verilog "../../source/virtual_packet_fifo/vfifo_controller/egress_fifo.v"
read_verilog "../../source/virtual_packet_fifo/vfifo_controller/ingress_fifo.v"
read_verilog "../../source/virtual_packet_fifo/vfifo_controller/vfifo_controller.v"
read_verilog "../../source/virtual_packet_fifo/packetized_vfifo_controller.v"
read_verilog "../../source/virtual_packet_fifo/multiport_mc.v"
read_verilog "../../source/virtual_packet_fifo/virtual_packet_fifo.v"
read_verilog "../../source/virtual_packet_fifo_wrapper.v"
read_verilog "../../source/synchronizer_simple.v"
read_verilog "../../source/pcie_performance_monitor.v"
read_verilog "../../source/raw_data_packet.v"
read_verilog "../../source/k7_pcie_dma_ddr3_base.v"
read_verilog "../../source/modified_ip_files/dma/packet_dma_axi.v"
read_verilog "../../source/modified_ip_files/dma/register_map.v"

read_verilog "../../tb/board.v"
read_verilog "../../tb/pcie_7x_gt_top_pipe_mode.v"
read_verilog "../../tb/pcie_7x_pipe_clock.v"
read_verilog "../../tb/ddr3_model.v"
read_verilog "../../tb/dsport/xilinx_pcie_2_1_rport_7x.v"
read_verilog "../../tb/dsport/pcie_2_1_rport_7x.v"
read_verilog "../../tb/dsport/pcie_axi_trn_bridge.v"
read_verilog "../../tb/dsport/pci_exp_usrapp_com.v"
read_verilog "../../tb/dsport/pci_exp_usrapp_tx.v"
read_verilog "../../tb/dsport/pci_exp_usrapp_cfg.v"
read_verilog "../../tb/dsport/pci_exp_usrapp_rx.v"
read_verilog "../../tb/dsport/pci_exp_usrapp_pl.v"

update_compile_order -fileset sources_1
set_property top ${top} [get_filesets sim_1]

import_files -norecurse {../../sim/include/dut_defines.v ../../sim/include/board_common.v ../../sim/include/user_defines.v}
set_property file_type {Verilog Header} [get_files  ./vivado_sim_x4gen2/k7_pcie_dma_ddr3_base_x4gen2.srcs/sources_1/imports/include/board_common.v]
add_files -norecurse ./vivado_sim_x4gen2/k7_pcie_dma_ddr3_base_x4gen2.srcs/sources_1/ip/mig_7x/mig_7x/example_design/sim/ddr3_model_parameters.vh
update_compile_order -fileset sources_1

generate_target all [get_files vivado_sim_x4gen2/k7_pcie_dma_ddr3_base_x4gen2.srcs/sources_1/ip/pcie_7x/pcie_7x.xci]
#add_files -fileset sim_1 -norecurse vivado_sim_x4gen2/k7_pcie_dma_ddr3_base_x4gen2.srcs/sources_1/ip/pcie_7x/pcie_7x/source/pcie_7x_gt_top_pipe_mode.v

## Generate MIG IP
generate_target all [get_files vivado_sim_x4gen2/k7_pcie_dma_ddr3_base_x4gen2.srcs/sources_1/ip/mig_7x/mig_7x.xci]
# Include search path 
set_property include_dirs { ../../tb ../../tb/dsport ../include vivado_sim_x4gen2/k7_pcie_dma_ddr3_base_x4gen2.srcs/sources_1/ip/mig_7x/mig_7x/example_design/sim} [get_filesets sim_1]
#set_property include_dirs { vivado_sim_x4gen2/k7_pcie_dma_ddr3_base.srcs/sources_1/ip/mig_7x/mig_7x/example_design/sim} [get_filesets sim_1]
# Add the DDR3 model files to the simset
add_files -fileset sim_1 -norecurse vivado_sim_x4gen2/k7_pcie_dma_ddr3_base_x4gen2.srcs/sources_1/ip/mig_7x/mig_7x/example_design/sim/ddr3_model.v

update_compile_order -fileset sources_1

launch_xsim -simset sim_1 -mode behavioral 
run all
exit

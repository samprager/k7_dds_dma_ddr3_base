###############################################################################
#
# This XDC is intended for use with the Xilinx KC705 Development Board with a 
# xc7k325t-ffg900-2 part
#
###############################################################################


#########################################################
# MIG Constraints
#########################################################

#--------------------
# Timing Constraints
#--------------------
create_clock -name sys_clk -period 5 [get_ports sys_clk_p]
          


##-----------------------------------------------------------------------------
## Project    : Series-7 Integrated Block for PCI Express
## File       : xilinx_pcie_2_1_ep_7x_04_lane_gen2_xc7k325t-ffg900-1_KC705_REVC.xdc
## Version    : 1.3
#

###############################################################################
# Timing Constraints
###############################################################################

#
# Timing requirements and related constraints.
#

create_clock -name pcie_clk -period 10 [get_pins pcie_clk_ibuf/O]

create_generated_clock -name clk_125mhz -source [get_pins pcie_clk_ibuf/O] -edges {1 2 3} -edge_shift {0 -1 -2} [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT0]

create_generated_clock -name clk_250mhz -source [get_pins pcie_clk_ibuf/O] -edges {1 2 3} -edge_shift {0 -3 -6} [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT1]


create_generated_clock -name clk_userclk -source [get_pins pcie_clk_ibuf/O] -edges {1 2 3} -edge_shift {0 -3 -6} [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT2]

create_generated_clock -name clk_userclk2 -source [get_pins pcie_clk_ibuf/O] -edges {1 2 3} -edge_shift {0 -3 -6} [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT3]



###############################################################################
# End PCI Express Constraints
###############################################################################


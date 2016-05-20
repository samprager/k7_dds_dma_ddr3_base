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
create_clock -period 5 -name sys_clk [get_ports sys_clk_p]
set_propagated_clock sys_clk_p

# Note: the following CLOCK_DEDICATED_ROUTE constraint will cause a warning in place similar
# to the following:
#   WARNING:Place:1402 - A clock IOB / PLL clock component pair have been found that are not
#   placed at an optimal clock IOB / PLL site pair.
# This warning can be ignored.  See the Users Guide for more information.

#set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets sys_clk_p]
#set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_pins -hierarchical *pll*CLKIN1]



#------------------------------------------------------------------------------------------
# I/O standards and pin locations
#------------------------------------------------------------------------------------------
#

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[0]}]
set_property SLEW FAST [get_ports {ddr_dq[0]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[0]}]
set_property LOC AA15 [get_ports {ddr_dq[0]}]

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[1]}]
set_property SLEW FAST [get_ports {ddr_dq[1]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[1]}]
set_property LOC AA16 [get_ports {ddr_dq[1]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[2]}]
set_property SLEW FAST [get_ports {ddr_dq[2]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[2]}]
set_property LOC AC14 [get_ports {ddr_dq[2]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[3]}]
set_property SLEW FAST [get_ports {ddr_dq[3]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[3]}]
set_property LOC AD14 [get_ports {ddr_dq[3]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[4]}]
set_property SLEW FAST [get_ports {ddr_dq[4]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[4]}]
set_property LOC AA17 [get_ports {ddr_dq[4]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[5]}]
set_property SLEW FAST [get_ports {ddr_dq[5]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[5]}]
set_property LOC AB15 [get_ports {ddr_dq[5]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[6]}]
set_property SLEW FAST [get_ports {ddr_dq[6]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[6]}]
set_property LOC AE15 [get_ports {ddr_dq[6]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[7]}]
set_property SLEW FAST [get_ports {ddr_dq[7]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[7]}]
set_property LOC Y15 [get_ports {ddr_dq[7]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[8]}]
set_property SLEW FAST [get_ports {ddr_dq[8]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[8]}]
set_property LOC AB19 [get_ports {ddr_dq[8]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[9]}]
set_property SLEW FAST [get_ports {ddr_dq[9]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[9]}]
set_property LOC AD16 [get_ports {ddr_dq[9]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[10]}]
set_property SLEW FAST [get_ports {ddr_dq[10]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[10]}]
set_property LOC AC19 [get_ports {ddr_dq[10]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[11]}]
set_property SLEW FAST [get_ports {ddr_dq[11]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[11]}]
set_property LOC AD17 [get_ports {ddr_dq[11]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[12]}]
set_property SLEW FAST [get_ports {ddr_dq[12]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[12]}]
set_property LOC AA18 [get_ports {ddr_dq[12]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[13]}]
set_property SLEW FAST [get_ports {ddr_dq[13]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[13]}]
set_property LOC AB18 [get_ports {ddr_dq[13]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[14]}]
set_property SLEW FAST [get_ports {ddr_dq[14]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[14]}]
set_property LOC AE18 [get_ports {ddr_dq[14]}]

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[15]}]
set_property SLEW FAST [get_ports {ddr_dq[15]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[15]}]
set_property LOC AD18 [get_ports {ddr_dq[15]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[16]}]
set_property SLEW FAST [get_ports {ddr_dq[16]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[16]}]
set_property LOC AG19 [get_ports {ddr_dq[16]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[17]}]
set_property SLEW FAST [get_ports {ddr_dq[17]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[17]}]
set_property LOC AK19 [get_ports {ddr_dq[17]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[18]}]
set_property SLEW FAST [get_ports {ddr_dq[18]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[18]}]
set_property LOC AG18 [get_ports {ddr_dq[18]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[19]}]
set_property SLEW FAST [get_ports {ddr_dq[19]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[19]}]
set_property LOC AF18 [get_ports {ddr_dq[19]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[20]}]
set_property SLEW FAST [get_ports {ddr_dq[20]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[20]}]
set_property LOC AH19 [get_ports {ddr_dq[20]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[21]}]
set_property SLEW FAST [get_ports {ddr_dq[21]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[21]}]
set_property LOC AJ19 [get_ports {ddr_dq[21]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[22]}]
set_property SLEW FAST [get_ports {ddr_dq[22]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[22]}]
set_property LOC AE19 [get_ports {ddr_dq[22]}]

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[23]}]
set_property SLEW FAST [get_ports {ddr_dq[23]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[23]}]
set_property LOC AD19 [get_ports {ddr_dq[23]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[24]}]
set_property SLEW FAST [get_ports {ddr_dq[24]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[24]}]
set_property LOC AK16 [get_ports {ddr_dq[24]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[25]}]
set_property SLEW FAST [get_ports {ddr_dq[25]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[25]}]
set_property LOC AJ17 [get_ports {ddr_dq[25]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[26]}]
set_property SLEW FAST [get_ports {ddr_dq[26]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[26]}]
set_property LOC AG15 [get_ports {ddr_dq[26]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[27]}]
set_property SLEW FAST [get_ports {ddr_dq[27]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[27]}]
set_property LOC AF15 [get_ports {ddr_dq[27]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[28]}]
set_property SLEW FAST [get_ports {ddr_dq[28]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[28]}]
set_property LOC AH17 [get_ports {ddr_dq[28]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[29]}]
set_property SLEW FAST [get_ports {ddr_dq[29]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[29]}]
set_property LOC AG14 [get_ports {ddr_dq[29]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[30]}]
set_property SLEW FAST [get_ports {ddr_dq[30]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[30]}]
set_property LOC AH15 [get_ports {ddr_dq[30]}]

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[31]}]
set_property SLEW FAST [get_ports {ddr_dq[31]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[31]}]
set_property LOC AK15 [get_ports {ddr_dq[31]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[32]}]
set_property SLEW FAST [get_ports {ddr_dq[32]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[32]}]
set_property LOC AK8 [get_ports {ddr_dq[32]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[33]}]
set_property SLEW FAST [get_ports {ddr_dq[33]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[33]}]
set_property LOC AK6 [get_ports {ddr_dq[33]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[34]}]
set_property SLEW FAST [get_ports {ddr_dq[34]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[34]}]
set_property LOC AG7 [get_ports {ddr_dq[34]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[35]}]
set_property SLEW FAST [get_ports {ddr_dq[35]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[35]}]
set_property LOC AF7 [get_ports {ddr_dq[35]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[36]}]
set_property SLEW FAST [get_ports {ddr_dq[36]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[36]}]
set_property LOC AF8 [get_ports {ddr_dq[36]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[37]}]
set_property SLEW FAST [get_ports {ddr_dq[37]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[37]}]
set_property LOC AK4 [get_ports {ddr_dq[37]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[38]}]
set_property SLEW FAST [get_ports {ddr_dq[38]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[38]}]
set_property LOC AJ8 [get_ports {ddr_dq[38]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[39]}]
set_property SLEW FAST [get_ports {ddr_dq[39]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[39]}]
set_property LOC AJ6 [get_ports {ddr_dq[39]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[40]}]
set_property SLEW FAST [get_ports {ddr_dq[40]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[40]}]
set_property LOC AH5 [get_ports {ddr_dq[40]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[41]}]
set_property SLEW FAST [get_ports {ddr_dq[41]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[41]}]
set_property LOC AH6 [get_ports {ddr_dq[41]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[42]}]
set_property SLEW FAST [get_ports {ddr_dq[42]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[42]}]
set_property LOC AJ2 [get_ports {ddr_dq[42]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[43]}]
set_property SLEW FAST [get_ports {ddr_dq[43]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[43]}]
set_property LOC AH2 [get_ports {ddr_dq[43]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[44]}]
set_property SLEW FAST [get_ports {ddr_dq[44]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[44]}]
set_property LOC AH4 [get_ports {ddr_dq[44]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[45]}]
set_property SLEW FAST [get_ports {ddr_dq[45]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[45]}]
set_property LOC AJ4 [get_ports {ddr_dq[45]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[46]}]
set_property SLEW FAST [get_ports {ddr_dq[46]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[46]}]
set_property LOC AK1 [get_ports {ddr_dq[46]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[47]}]
set_property SLEW FAST [get_ports {ddr_dq[47]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[47]}]
set_property LOC AJ1 [get_ports {ddr_dq[47]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[48]}]
set_property SLEW FAST [get_ports {ddr_dq[48]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[48]}]
set_property LOC AF1 [get_ports {ddr_dq[48]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[49]}]
set_property SLEW FAST [get_ports {ddr_dq[49]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[49]}]
set_property LOC AF2 [get_ports {ddr_dq[49]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[50]}]
set_property SLEW FAST [get_ports {ddr_dq[50]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[50]}]
set_property LOC AE4 [get_ports {ddr_dq[50]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[51]}]
set_property SLEW FAST [get_ports {ddr_dq[51]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[51]}]
set_property LOC AE3 [get_ports {ddr_dq[51]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[52]}]
set_property SLEW FAST [get_ports {ddr_dq[52]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[52]}]
set_property LOC AF3 [get_ports {ddr_dq[52]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[53]}]
set_property SLEW FAST [get_ports {ddr_dq[53]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[53]}]
set_property LOC AF5 [get_ports {ddr_dq[53]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[54]}]
set_property SLEW FAST [get_ports {ddr_dq[54]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[54]}]
set_property LOC AE1 [get_ports {ddr_dq[54]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[55]}]
set_property SLEW FAST [get_ports {ddr_dq[55]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[55]}]
set_property LOC AE5 [get_ports {ddr_dq[55]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[56]}]
set_property SLEW FAST [get_ports {ddr_dq[56]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[56]}]
set_property LOC AC1 [get_ports {ddr_dq[56]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[57]}]
set_property SLEW FAST [get_ports {ddr_dq[57]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[57]}]
set_property LOC AD3 [get_ports {ddr_dq[57]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[58]}]
set_property SLEW FAST [get_ports {ddr_dq[58]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[58]}]
set_property LOC AC4 [get_ports {ddr_dq[58]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[59]}]
set_property SLEW FAST [get_ports {ddr_dq[59]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[59]}]
set_property LOC AC5 [get_ports {ddr_dq[59]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[60]}]
set_property SLEW FAST [get_ports {ddr_dq[60]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[60]}]
set_property LOC AE6 [get_ports {ddr_dq[60]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[61]}]
set_property SLEW FAST [get_ports {ddr_dq[61]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[61]}]
set_property LOC AD6 [get_ports {ddr_dq[61]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[62]}]
set_property SLEW FAST [get_ports {ddr_dq[62]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[62]}]
set_property LOC AC2 [get_ports {ddr_dq[62]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dq[63]}]
set_property SLEW FAST [get_ports {ddr_dq[63]}]
set_property IOSTANDARD SSTL15_T_DCI [get_ports {ddr_dq[63]}]
set_property LOC AD4 [get_ports {ddr_dq[63]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[13]}]
set_property SLEW FAST [get_ports {ddr_addr[13]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[13]}]
set_property LOC AH11 [get_ports {ddr_addr[13]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[12]}]
set_property SLEW FAST [get_ports {ddr_addr[12]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[12]}]
set_property LOC AJ11 [get_ports {ddr_addr[12]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[11]}]
set_property SLEW FAST [get_ports {ddr_addr[11]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[11]}]
set_property LOC AE13 [get_ports {ddr_addr[11]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[10]}]
set_property SLEW FAST [get_ports {ddr_addr[10]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[10]}]
set_property LOC AF13 [get_ports {ddr_addr[10]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[9]}]
set_property SLEW FAST [get_ports {ddr_addr[9]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[9]}]
set_property LOC AK14 [get_ports {ddr_addr[9]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[8]}]
set_property SLEW FAST [get_ports {ddr_addr[8]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[8]}]
set_property LOC AK13 [get_ports {ddr_addr[8]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[7]}]
set_property SLEW FAST [get_ports {ddr_addr[7]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[7]}]
set_property LOC AH14 [get_ports {ddr_addr[7]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[6]}]
set_property SLEW FAST [get_ports {ddr_addr[6]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[6]}]
set_property LOC AJ14 [get_ports {ddr_addr[6]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[5]}]
set_property SLEW FAST [get_ports {ddr_addr[5]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[5]}]
set_property LOC AJ13 [get_ports {ddr_addr[5]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[4]}]
set_property SLEW FAST [get_ports {ddr_addr[4]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[4]}]
set_property LOC AJ12 [get_ports {ddr_addr[4]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[3]}]
set_property SLEW FAST [get_ports {ddr_addr[3]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[3]}]
set_property LOC AF12 [get_ports {ddr_addr[3]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[2]}]
set_property SLEW FAST [get_ports {ddr_addr[2]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[2]}]
set_property LOC AG12 [get_ports {ddr_addr[2]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[1]}]
set_property SLEW FAST [get_ports {ddr_addr[1]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[1]}]
set_property LOC AG13 [get_ports {ddr_addr[1]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_addr[0]}]
set_property SLEW FAST [get_ports {ddr_addr[0]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_addr[0]}]
set_property LOC AH12 [get_ports {ddr_addr[0]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_ba[2]}]
set_property SLEW FAST [get_ports {ddr_ba[2]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_ba[2]}]
set_property LOC AK9 [get_ports {ddr_ba[2]}]

# Bank 33: 
set_property VCCAUX_IO HIGH [get_ports {ddr_ba[1]}]
set_property SLEW FAST [get_ports {ddr_ba[1]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_ba[1]}]
set_property LOC AG9 [get_ports {ddr_ba[1]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports {ddr_ba[0]}]
set_property SLEW FAST [get_ports {ddr_ba[0]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_ba[0]}]
set_property LOC AH9 [get_ports {ddr_ba[0]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports ddr_ras_n]
set_property SLEW FAST [get_ports ddr_ras_n]
set_property IOSTANDARD SSTL15 [get_ports ddr_ras_n]
set_property LOC AD9 [get_ports ddr_ras_n]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports ddr_cas_n]
set_property SLEW FAST [get_ports ddr_cas_n]
set_property IOSTANDARD SSTL15 [get_ports ddr_cas_n]
set_property LOC AC11 [get_ports ddr_cas_n]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports ddr_we_n]
set_property SLEW FAST [get_ports ddr_we_n]
set_property IOSTANDARD SSTL15 [get_ports ddr_we_n]
set_property LOC AE9 [get_ports ddr_we_n]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports ddr_reset_n]
set_property SLEW FAST [get_ports ddr_reset_n]
set_property IOSTANDARD LVCMOS15 [get_ports ddr_reset_n]
set_property LOC AK3 [get_ports ddr_reset_n]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports ddr_cke]
set_property SLEW FAST [get_ports ddr_cke]
set_property IOSTANDARD SSTL15 [get_ports ddr_cke]
set_property LOC AF10 [get_ports ddr_cke]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports ddr_odt]
set_property SLEW FAST [get_ports ddr_odt]
set_property IOSTANDARD SSTL15 [get_ports ddr_odt]
set_property LOC AD8 [get_ports ddr_odt]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports ddr_cs_n]
set_property SLEW FAST [get_ports ddr_cs_n]
set_property IOSTANDARD SSTL15 [get_ports ddr_cs_n]
set_property LOC AC12 [get_ports ddr_cs_n]

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dm[0]}]
set_property SLEW FAST [get_ports {ddr_dm[0]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_dm[0]}]
set_property LOC Y16 [get_ports {ddr_dm[0]}]

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dm[1]}]
set_property SLEW FAST [get_ports {ddr_dm[1]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_dm[1]}]
set_property LOC AB17 [get_ports {ddr_dm[1]}]

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dm[2]}]
set_property SLEW FAST [get_ports {ddr_dm[2]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_dm[2]}]
set_property LOC AF17 [get_ports {ddr_dm[2]}]

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dm[3]}]
set_property SLEW FAST [get_ports {ddr_dm[3]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_dm[3]}]
set_property LOC AE16 [get_ports {ddr_dm[3]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dm[4]}]
set_property SLEW FAST [get_ports {ddr_dm[4]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_dm[4]}]
set_property LOC AK5 [get_ports {ddr_dm[4]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dm[5]}]
set_property SLEW FAST [get_ports {ddr_dm[5]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_dm[5]}]
set_property LOC AJ3 [get_ports {ddr_dm[5]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dm[6]}]
set_property SLEW FAST [get_ports {ddr_dm[6]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_dm[6]}]
set_property LOC AF6 [get_ports {ddr_dm[6]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dm[7]}]
set_property SLEW FAST [get_ports {ddr_dm[7]}]
set_property IOSTANDARD SSTL15 [get_ports {ddr_dm[7]}]
set_property LOC AC7 [get_ports {ddr_dm[7]}]

# Bank 33: 
set_property VCCAUX_IO DONTCARE [get_ports sys_clk_p]
set_property IOSTANDARD DIFF_SSTL15 [get_ports sys_clk_p]
set_property LOC AD12 [get_ports sys_clk_p]

# Bank 33: 
set_property VCCAUX_IO DONTCARE [get_ports sys_clk_n]
set_property IOSTANDARD DIFF_SSTL15 [get_ports sys_clk_n]
#set_property LOC AD11 [get_ports sys_clk_n]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_p[0]}]
set_property SLEW FAST [get_ports {ddr_dqs_p[0]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_p[0]}]
set_property LOC AC16 [get_ports {ddr_dqs_p[0]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_n[0]}]
set_property SLEW FAST [get_ports {ddr_dqs_n[0]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_n[0]}]
set_property LOC AC15 [get_ports {ddr_dqs_n[0]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_p[1]}]
set_property SLEW FAST [get_ports {ddr_dqs_p[1]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_p[1]}]
set_property LOC Y19 [get_ports {ddr_dqs_p[1]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_n[1]}]
set_property SLEW FAST [get_ports {ddr_dqs_n[1]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_n[1]}]
set_property LOC Y18 [get_ports {ddr_dqs_n[1]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_p[2]}]
set_property SLEW FAST [get_ports {ddr_dqs_p[2]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_p[2]}]
set_property LOC AJ18 [get_ports {ddr_dqs_p[2]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_n[2]}]
set_property SLEW FAST [get_ports {ddr_dqs_n[2]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_n[2]}]
set_property LOC AK18 [get_ports {ddr_dqs_n[2]}]

# Bank 32:
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_p[3]}]
set_property SLEW FAST [get_ports {ddr_dqs_p[3]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_p[3]}]
set_property LOC AH16 [get_ports {ddr_dqs_p[3]}]

# Bank 32: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_n[3]}]
set_property SLEW FAST [get_ports {ddr_dqs_n[3]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_n[3]}]
set_property LOC AJ16 [get_ports {ddr_dqs_n[3]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_p[4]}]
set_property SLEW FAST [get_ports {ddr_dqs_p[4]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_p[4]}]
set_property LOC AH7 [get_ports {ddr_dqs_p[4]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_n[4]}]
set_property SLEW FAST [get_ports {ddr_dqs_n[4]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_n[4]}]
set_property LOC AJ7 [get_ports {ddr_dqs_n[4]}]

# Bank 34:
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_p[5]}]
set_property SLEW FAST [get_ports {ddr_dqs_p[5]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_p[5]}]
set_property LOC AG2 [get_ports {ddr_dqs_p[5]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_n[5]}]
set_property SLEW FAST [get_ports {ddr_dqs_n[5]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_n[5]}]
set_property LOC AH1 [get_ports {ddr_dqs_n[5]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_p[6]}]
set_property SLEW FAST [get_ports {ddr_dqs_p[6]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_p[6]}]
set_property LOC AG4 [get_ports {ddr_dqs_p[6]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_n[6]}]
set_property SLEW FAST [get_ports {ddr_dqs_n[6]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_n[6]}]
set_property LOC AG3 [get_ports {ddr_dqs_n[6]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_p[7]}]
set_property SLEW FAST [get_ports {ddr_dqs_p[7]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_p[7]}]
set_property LOC AD2 [get_ports {ddr_dqs_p[7]}]

# Bank 34: 
set_property VCCAUX_IO HIGH [get_ports {ddr_dqs_n[7]}]
set_property SLEW FAST [get_ports {ddr_dqs_n[7]}]
set_property IOSTANDARD DIFF_SSTL15_T_DCI [get_ports {ddr_dqs_n[7]}]
set_property LOC AD1 [get_ports {ddr_dqs_n[7]}]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports ddr_ck_p]
set_property SLEW FAST [get_ports ddr_ck_p]
set_property IOSTANDARD DIFF_SSTL15 [get_ports ddr_ck_p]
set_property LOC AG10 [get_ports ddr_ck_p]

# Bank 33:
set_property VCCAUX_IO HIGH [get_ports ddr_ck_n]
set_property SLEW FAST [get_ports ddr_ck_n]
set_property IOSTANDARD DIFF_SSTL15 [get_ports ddr_ck_n]
set_property LOC AH10 [get_ports ddr_ck_n]




#------------------------------------------------------------------------------------------
# PHASER internal location constraints
#------------------------------------------------------------------------------------------
set_property LOC PHASER_OUT_PHY_X1Y3  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y2  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y1  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y0  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y6  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_1.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y5  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_1.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y4  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_1.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y11 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y10 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y9  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/phaser_out]
set_property LOC PHASER_OUT_PHY_X1Y8  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/phaser_out]

set_property LOC PHASER_IN_PHY_X1Y3 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/phaser_in_gen.phaser_in]
set_property LOC PHASER_IN_PHY_X1Y2 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/phaser_in_gen.phaser_in]
set_property LOC PHASER_IN_PHY_X1Y1 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/phaser_in_gen.phaser_in]
set_property LOC PHASER_IN_PHY_X1Y0 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/phaser_in_gen.phaser_in]
##
##
##
set_property LOC PHASER_IN_PHY_X1Y11 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/phaser_in_gen.phaser_in]
set_property LOC PHASER_IN_PHY_X1Y10 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/phaser_in_gen.phaser_in]
set_property LOC PHASER_IN_PHY_X1Y9  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/phaser_in_gen.phaser_in]
set_property LOC PHASER_IN_PHY_X1Y8  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/phaser_in_gen.phaser_in]

set_property LOC OUT_FIFO_X1Y3  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/out_fifo]
set_property LOC OUT_FIFO_X1Y2  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/out_fifo]
set_property LOC OUT_FIFO_X1Y1  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/out_fifo]
set_property LOC OUT_FIFO_X1Y0  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/out_fifo]
set_property LOC OUT_FIFO_X1Y6  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_1.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/out_fifo]
set_property LOC OUT_FIFO_X1Y5  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_1.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/out_fifo]
set_property LOC OUT_FIFO_X1Y4  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_1.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/out_fifo]
set_property LOC OUT_FIFO_X1Y11 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/out_fifo]
set_property LOC OUT_FIFO_X1Y10 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/out_fifo]
set_property LOC OUT_FIFO_X1Y9  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/out_fifo]
set_property LOC OUT_FIFO_X1Y8  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/out_fifo]

set_property LOC IN_FIFO_X1Y3  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/in_fifo_gen.in_fifo]
set_property LOC IN_FIFO_X1Y2  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/in_fifo_gen.in_fifo]
set_property LOC IN_FIFO_X1Y1  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/in_fifo_gen.in_fifo]
set_property LOC IN_FIFO_X1Y0  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/in_fifo_gen.in_fifo]
set_property LOC IN_FIFO_X1Y11 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/in_fifo_gen.in_fifo]
set_property LOC IN_FIFO_X1Y10 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/in_fifo_gen.in_fifo]
set_property LOC IN_FIFO_X1Y9  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/in_fifo_gen.in_fifo]
set_property LOC IN_FIFO_X1Y8  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/in_fifo_gen.in_fifo]

set_property LOC PHY_CONTROL_X1Y0 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/phy_control_i]
set_property LOC PHY_CONTROL_X1Y1 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_1.u_ddr_phy_4lanes/phy_control_i]
set_property LOC PHY_CONTROL_X1Y2 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/phy_control_i]

set_property LOC PHASER_REF_X1Y0 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/phaser_ref_i]
set_property LOC PHASER_REF_X1Y1 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_1.u_ddr_phy_4lanes/phaser_ref_i]
set_property LOC PHASER_REF_X1Y2 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/phaser_ref_i]

set_property LOC OLOGIC_X1Y43  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/ddr_byte_group_io/*slave_ts]
set_property LOC OLOGIC_X1Y31  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/ddr_byte_group_io/*slave_ts]
set_property LOC OLOGIC_X1Y19  [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/ddr_byte_group_io/*slave_ts]
set_property LOC OLOGIC_X1Y7   [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_2.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/ddr_byte_group_io/*slave_ts]
set_property LOC OLOGIC_X1Y143 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_D.ddr_byte_lane_D/ddr_byte_group_io/*slave_ts]
set_property LOC OLOGIC_X1Y131 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_C.ddr_byte_lane_C/ddr_byte_group_io/*slave_ts]
set_property LOC OLOGIC_X1Y119 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_B.ddr_byte_lane_B/ddr_byte_group_io/*slave_ts]
set_property LOC OLOGIC_X1Y107 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/mem_intfc0/ddr_phy_top0/u_ddr_mc_phy_wrapper/u_ddr_mc_phy/ddr_phy_4lanes_0.u_ddr_phy_4lanes/ddr_byte_lane_A.ddr_byte_lane_A/ddr_byte_group_io/*slave_ts]

#------------------------------------------------------------------------------------------
# Clock resources internal location constraints
#------------------------------------------------------------------------------------------
set_property LOC MMCME2_ADV_X1Y1 [get_cells virtual_pfifo_wrapper_inst/mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_ddr3_infrastructure/gen_mmcm.mmcm_i]

#------------------------------------------------------------------------------------------
# False paths for MIG core
#------------------------------------------------------------------------------------------

set_false_path -through [get_pins -filter {NAME =~ */DQSFOUND} -of [get_cells -hier -filter {REF_NAME == PHASER_IN_PHY}]]
set_multicycle_path -through [get_pins -filter {NAME =~ */OSERDESRST} -of [get_cells -hier -filter {REF_NAME == PHASER_OUT_PHY}]] -setup 2 -start
set_multicycle_path -through [get_pins -filter {NAME =~ */OSERDESRST} -of [get_cells -hier -filter {REF_NAME == PHASER_OUT_PHY}]] -hold 1 -start

set_multicycle_path -to   [get_cells -hier -filter {NAME =~ */temp_mon_enabled.u_tempmon/device_temp_sync_r1*}] -setup 12 -end
set_multicycle_path -to   [get_cells -hier -filter {NAME =~ */temp_mon_enabled.u_tempmon/device_temp_sync_r1*}] -hold 11 -end
set_multicycle_path -to   [get_cells -hier -filter {NAME =~ */temp_mon_enabled.u_tempmon/xadc_supplied_temperature.rst_r1*}] -setup 2 -end
set_multicycle_path -to   [get_cells -hier -filter {NAME =~ */temp_mon_enabled.u_tempmon/xadc_supplied_temperature.rst_r1*}] -hold 1 -end

##-----------------------------------------------------------------------------
## Project    : Series-7 Integrated Block for PCI Express
## File       : xilinx_pcie_2_1_ep_7x_04_lane_gen2_xc7k325t-ffg900-1_KC705_REVC.xdc
## Version    : 1.3
#
###############################################################################
# User Time Names / User Time Groups / Time Specs
###############################################################################

###############################################################################
# User Physical Constraints
###############################################################################


###############################################################################
# Pinout and Related I/O Constraints
###############################################################################

#
# perst_n (input) signal.  The perst_n signal should be
# obtained from the PCI Express interface if possible.  For
# slot based form factors, a system reset signal is usually
# present on the connector.  For cable based form factors, a
# system reset signal may not be available.  In this case, the
# system reset signal must be generated locally by some form of
# supervisory circuit.  You may change the IOSTANDARD and LOC
# to suit your requirements and VCCO voltage banking rules.
# Some 7 series devices do not have 3.3 V I/Os available.
# Therefore the appropriate level shift is required to operate
# with these devices that contain only 1.8 V banks.
#

set_false_path -from [get_ports perst_n]
set_property IOSTANDARD LVCMOS25 [get_ports perst_n]
set_property PULLUP true [get_ports perst_n]
set_property LOC G25 [get_ports perst_n]

#
#
# SYS clock 100 MHz (input) signal. The pcie_clk_p and pcie_clk_n
# signals are the PCI Express reference clock. Virtex-7 GT
# Transceiver architecture requires the use of a dedicated clock
# resources (FPGA input pins) associated with each GT Transceiver.
# To use these pins an IBUFDS primitive (refclk_ibuf) is
# instantiated in user's design.
# Please refer to the Virtex-7 GT Transceiver User Guide
# (UG) for guidelines regarding clock resource selection.
#

set_property LOC IBUFDS_GTE2_X0Y1 [get_cells pcie_clk_ibuf]
#set_property IOSTANDARD LVDS [get_ports pcie_clk_p]
#set_property IOSTANDARD LVDS [get_ports pcie_clk_n]

#
# Transceiver instance placement.  This constraint selects the
# transceivers to be used, which also dictates the pinout for the
# transmit and receive differential pairs.  Please refer to the
# Virtex-7 GT Transceiver User Guide (UG) for more information.
#

# PCIe Lane 0
set_property LOC GTXE2_CHANNEL_X0Y7 [get_cells {pcie_inst/inst/inst/gt_top.gt_top_i/pipe_wrapper_i/pipe_lane[0].gt_wrapper_i/gtx_channel.gtxe2_channel_i}]
# PCIe Lane 1
set_property LOC GTXE2_CHANNEL_X0Y6 [get_cells {pcie_inst/inst/inst/gt_top.gt_top_i/pipe_wrapper_i/pipe_lane[1].gt_wrapper_i/gtx_channel.gtxe2_channel_i}]
# PCIe Lane 2
set_property LOC GTXE2_CHANNEL_X0Y5 [get_cells {pcie_inst/inst/inst/gt_top.gt_top_i/pipe_wrapper_i/pipe_lane[2].gt_wrapper_i/gtx_channel.gtxe2_channel_i}]
# PCIe Lane 3
set_property LOC GTXE2_CHANNEL_X0Y4 [get_cells {pcie_inst/inst/inst/gt_top.gt_top_i/pipe_wrapper_i/pipe_lane[3].gt_wrapper_i/gtx_channel.gtxe2_channel_i}]


#
# PCI Express Block placement. This constraint selects the PCI Express
# Block to be used.
#

set_property LOC PCIE_X0Y0 [get_cells pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_block_i]

#
# BlockRAM placement
#
set_property LOC RAMB36_X5Y35 [get_cells {pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_bram_top/pcie_brams_rx/brams[3].ram/use_tdp.ramb36/genblk*.bram36_tdp_bl.bram36_tdp_bl}]
set_property LOC RAMB36_X4Y36 [get_cells {pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_bram_top/pcie_brams_rx/brams[2].ram/use_tdp.ramb36/genblk*.bram36_tdp_bl.bram36_tdp_bl}]
set_property LOC RAMB36_X4Y35 [get_cells {pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_bram_top/pcie_brams_rx/brams[1].ram/use_tdp.ramb36/genblk*.bram36_tdp_bl.bram36_tdp_bl}]
set_property LOC RAMB36_X4Y34 [get_cells {pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_bram_top/pcie_brams_rx/brams[0].ram/use_tdp.ramb36/genblk*.bram36_tdp_bl.bram36_tdp_bl}]
set_property LOC RAMB36_X4Y33 [get_cells {pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_bram_top/pcie_brams_tx/brams[0].ram/use_tdp.ramb36/genblk*.bram36_tdp_bl.bram36_tdp_bl}]
set_property LOC RAMB36_X4Y32 [get_cells {pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_bram_top/pcie_brams_tx/brams[1].ram/use_tdp.ramb36/genblk*.bram36_tdp_bl.bram36_tdp_bl}]
set_property LOC RAMB36_X4Y31 [get_cells {pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_bram_top/pcie_brams_tx/brams[2].ram/use_tdp.ramb36/genblk*.bram36_tdp_bl.bram36_tdp_bl}]
set_property LOC RAMB36_X4Y30 [get_cells {pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_bram_top/pcie_brams_tx/brams[3].ram/use_tdp.ramb36/genblk*.bram36_tdp_bl.bram36_tdp_bl}]

###############################################################################
# Timing Constraints
###############################################################################

#
# Timing requirements and related constraints.
#

create_clock -period 10 -name pcie_clk [get_pins pcie_clk_ibuf/O]

#create_generated_clock -name clk_125mhz -source [get_pins pcie_clk_ibuf/O] -edges {1 2 3} -edge_shift {0.000 -1.000 -2.000} [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT0]

#create_generated_clock -name clk_250mhz -source [get_pins pcie_clk_ibuf/O] -edges {1 2 3} -edge_shift {0.000 -3.000 -6.000} [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT1]


#create_generated_clock -name clk_userclk -source [get_pins pcie_clk_ibuf/O] -edges {1 2 3} -edge_shift {0.000 -3.000 -6.000} [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT2]

#create_generated_clock -name clk_userclk2 -source [get_pins pcie_clk_ibuf/O] -edges {1 2 3} -edge_shift {0.000 -3.000 -6.000} [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT3]


set_false_path -through [get_pins pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_block_i/PLPHYLNKUPN*]
set_false_path -through [get_pins pcie_inst/inst/inst/pcie_top_i/pcie_7x_i/pcie_block_i/PLRECEIVEDHOTRST*]

set_false_path -through [get_nets * -hierarchical -filter {NAME =~ */gt_top.gt_top_i/pipe_wrapper_i/*pipe_reset*/resetdone*}]
set_false_path -through [get_nets {pcie_inst/inst/inst/gt_top.gt_top_i/pipe_wrapper_i/pipe_lane[0].pipe_rate.pipe_rate_i/*}]
set_false_path -through [get_nets {pcie_inst/inst/inst/gt_top.gt_top_i/pipe_wrapper_i/pipe_lane[1].pipe_rate.pipe_rate_i/*}]
set_false_path -through [get_nets {pcie_inst/inst/inst/gt_top.gt_top_i/pipe_wrapper_i/pipe_lane[2].pipe_rate.pipe_rate_i/*}]
set_false_path -through [get_nets {pcie_inst/inst/inst/gt_top.gt_top_i/pipe_wrapper_i/pipe_lane[3].pipe_rate.pipe_rate_i/*}]

set_case_analysis 0 [get_pins ext_clk.pipe_clock_i/pclk_i1_bufgctrl.pclk_i1/S0]
set_case_analysis 1 [get_pins ext_clk.pipe_clock_i/pclk_i1_bufgctrl.pclk_i1/S1]
                                               
set_false_path -through [get_nets {ext_clk.pipe_clock_i/pclk_sel*}]
set_false_path -through [get_cells {pcie_inst/inst/inst/gt_top.gt_top_i/pipe_wrapper_i/pipe_reset.pipe_reset_i/cpllreset_reg*}]
       
#set_clock_groups -name ___clk_groups_generated_0_1_0_0_0 -physically_exclusive -group  [get_clocks clk_125mhz] -group  [get_clocks clk_250mhz]

create_clock -name txoutclk -period 10 [get_pins pcie_inst/pipe_txoutclk_out]

set_false_path -to [get_pins {ext_clk.pipe_clock_i/pclk_i1_bufgctrl.pclk_i1/S*}]

#set clk_125mhz [get_clocks -of_objects [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT0]]
#set clk_250mhz [get_clocks -of_objects [get_pins ext_clk.pipe_clock_i/mmcm_i/CLKOUT1]]
#set_clock_groups -name pcieclkmux -physically_exclusive -group clk_125mhz -group clk_250mhz

#create_generated_clock -name clk_125mhz \
#                        -source [get_pins ext_clk.pipe_clock_i/pclk_i1_bufgctrl.pclk_i1/I0] \
#                        -divide_by 1 \
#                        [get_pins ext_clk.pipe_clock_i/pclk_i1_bufgctrl.pclk_i1/O]

#create_generated_clock -name clk_250mhz \
#                        -source [get_pins ext_clk.pipe_clock_i/pclk_i1_bufgctrl.pclk_i1/I1] \
#                        -divide_by 1 -add -master_clock [get_clocks -of [get_pins ext_clk.pipe_clock_i/pclk_i1_bufgctrl.pclk_i1/I1]] \
#                        [get_pins ext_clk.pipe_clock_i/pclk_i1_bufgctrl.pclk_i1/O]



###############################################################################
# Physical Constraints
###############################################################################

###############################################################################
# End PCI Express Constraints
###############################################################################

#########################################################
# FIFO Generator constraints
#########################################################

#----------------------------------------------------------
# FIFO Generator constraints for Independent clock domains
#-----------------------------------------------------------
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ *IGP/wr_preview/*wr_pntr_*}] -to [get_cells * -hierarchical -filter {NAME =~ *IGP/wr_preview/*Q_*}]
                                        
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ *IGP/wr_preview/*rd_pntr_*}] -to [get_cells * -hierarchical -filter {NAME =~ *IGP/wr_preview/*Q_*}]

set_false_path -from [get_cells * -hierarchical -filter {NAME =~ *EGP/rd_preview/*wr_pntr_*}] -to [get_cells * -hierarchical -filter {NAME =~ *EGP/rd_preview/*Q_*}]

set_false_path -from [get_cells * -hierarchical -filter {NAME =~ *EGP/rd_preview/*rd_pntr_*}] -to [get_cells * -hierarchical -filter {NAME =~ *EGP/rd_preview/*Q_*}]

#########################################################
# Constraints relevant to the Targeted Reference Design
#########################################################
#------------------------------------------------------------------------------------------
# Internal VREF constraints
#------------------------------------------------------------------------------------------
set_property DCI_CASCADE {32 34} [get_iobanks 33]

#----------------------------------------
# FLASH programming - BPI Sync Mode fast 
#----------------------------------------

set_property IOSTANDARD LVCMOS25 [get_ports emcclk]
set_property LOC R24 [get_ports emcclk]

#-------------------------------------
# LED Status Pinout   (bottom to top)
#-------------------------------------
set_property IOSTANDARD LVCMOS15 [get_ports {led[0]}]
set_property SLEW SLOW [get_ports {led[0]}]
set_property DRIVE 4 [get_ports {led[0]}]
set_property LOC AB8 [get_ports {led[0]}]
set_property IOSTANDARD LVCMOS15 [get_ports {led[1]}]
set_property SLEW SLOW [get_ports {led[1]}]
set_property DRIVE 4 [get_ports {led[1]}]
set_property LOC AA8 [get_ports {led[1]}]
set_property IOSTANDARD LVCMOS15 [get_ports {led[2]}]
set_property SLEW SLOW [get_ports {led[2]}]
set_property DRIVE 4 [get_ports {led[2]}]
set_property LOC AC9 [get_ports {led[2]}]
set_property IOSTANDARD LVCMOS15 [get_ports {led[3]}]
set_property SLEW SLOW [get_ports {led[3]}]
set_property DRIVE 4 [get_ports {led[3]}]
set_property LOC AB9 [get_ports {led[3]}]
set_property IOSTANDARD LVCMOS25 [get_ports {led[4]}]
set_property SLEW SLOW [get_ports {led[4]}]
set_property DRIVE 4 [get_ports {led[4]}]
set_property LOC AE26 [get_ports {led[4]}]
set_property IOSTANDARD LVCMOS25 [get_ports {led[5]}]
set_property SLEW SLOW [get_ports {led[5]}]
set_property DRIVE 4 [get_ports {led[5]}]
set_property LOC G19 [get_ports {led[5]}]
set_property IOSTANDARD LVCMOS25 [get_ports {led[6]}]
set_property SLEW SLOW [get_ports {led[6]}]
set_property DRIVE 4 [get_ports {led[6]}]
set_property LOC E18 [get_ports {led[6]}]
set_property IOSTANDARD LVCMOS25 [get_ports {led[7]}]
set_property SLEW SLOW [get_ports {led[7]}]
set_property DRIVE 4 [get_ports {led[7]}]
set_property LOC F16 [get_ports {led[7]}]


## 1 on SW1 DIP switch (active-high)
set_property IOSTANDARD LVCMOS25 [get_ports {gpio_dip_sw[0]}]
## 2 on SW1 DIP switch (active-high)
set_property IOSTANDARD LVCMOS25 [get_ports {gpio_dip_sw[1]}]
## 3 on SW1 DIP switch (active-high)
set_property IOSTANDARD LVCMOS25 [get_ports {gpio_dip_sw[2]}]
## 4 on SW1 DIP switch (active-high)
set_property IOSTANDARD LVCMOS25 [get_ports {gpio_dip_sw[3]}]
#
set_property PACKAGE_PIN Y29 [get_ports {gpio_dip_sw[0]}]
set_property PACKAGE_PIN W29 [get_ports {gpio_dip_sw[1]}]
set_property PACKAGE_PIN AA28 [get_ports {gpio_dip_sw[2]}]
set_property PACKAGE_PIN Y28 [get_ports {gpio_dip_sw[3]}]
#--------------------------------------------
# Constraints for cross clock domains
#--------------------------------------------
# Manually added by Vandana put back by Chris after hardware errors ** check if errors are gone now
set_false_path -through [get_nets *start_addr*]
set_false_path -through [get_nets *end_addr*]
set_false_path -through [get_nets *wr_burst_size*]
set_false_path -through [get_nets *rd_burst_size*]
set_false_path -through [get_nets *ddr3_fifo_empty*]

set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/*init_calib_complete_r*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/*rstblk/*wr_rst_asreg*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/*init_calib_complete_r*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/*rstblk/*RST_FULL_GEN*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/*init_calib_complete_r*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/*rstblk/*grst_full*rst_d*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/*init_calib_complete_r*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/EGP/rd_preview/*rstblk/*rd_rst_asreg*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/*init_calib_complete_r*}] -to [get_cells * -hierarchical -filter {NAME =~ */sync_to_user_clk/sync_2reg_*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/*init_calib_complete_r*}] -to [get_cells * -hierarchical -filter {NAME =~ */sync_to_user_clk/sync_1reg_*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/u_memc_ui_top_axi/*init_calib_complete_r*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/*rstblk/*RST_FULL_GEN*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/*init_calib_complete_r*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/*rstblk/*rst_d1_reg*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/mp_mc_inst/mig_inst/u_mig_7x_mig/*init_calib_complete_r*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/*rstblk/*rst_d2_reg*}]

#set_false_path -from [get_cells user_lnk_up_int_i*]     

set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/sync_to_user_clk_*/sync_2reg_reg*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/U0/*/*rstblk/*RST_FULL_GEN*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/sync_to_user_clk_*/sync_2reg_reg*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/U0/*/*rstblk/*rd_rst_asreg*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/sync_to_user_clk_*/sync_2reg_reg*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/U0/*/*rstblk/*wr_rst_asreg*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/sync_to_user_clk_*/sync_2reg_reg*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/IGP/wr_preview/U0/*/*rstblk/*rst_d*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/ADDR_MGR/ddr3_fifo_empty_reg*}] -to [get_cells  * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/sync_to_user_clk_2/sync_1reg_reg*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/sync_to_user_clk_*/sync_2reg_reg*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/EGP/rd_preview/*/*rstblk/*rd_rst_asreg*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */sync_2reg_reg*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/multiple_pfifo.genblk1[*].pvfifo_ctlr_inst/vfifo_ctrl_inst/sync_to_user_clk_*/sync_1reg_reg*}]

set_false_path -from [get_cells * -hierarchical -filter {NAME =~ */axi_ic_mig_shim_rst_n_reg*}] -to [get_cells * -hierarchical -filter {NAME =~ */mp_pfifo_inst/sync_to_user_clk_*/sync_1reg_reg*}]

set_false_path -from [get_cells * -hierarchical -filter {NAME =~ packet_dma_axi_inst/*r2_c2s_areset_n}] -to [get_cells * -hierarchical -filter {NAME =~ *sync_to_user_clk_*/sync_1reg_*}]
set_false_path -from [get_cells * -hierarchical -filter {NAME =~ packet_dma_axi_inst/*r2_s2c_areset_n}] -to [get_cells * -hierarchical -filter {NAME =~ *sync_to_user_clk_*/sync_1reg_*}]
    ## * -hierarchical -filter {NAME =~ *IGP/wr_preview/*wr_pntr_*}


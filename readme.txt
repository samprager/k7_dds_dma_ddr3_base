
*************************************************************************
   ____  ____ 
  /   /\/   / 
 /___/  \  /   
 \   \   \/    © Copyright 2014 Xilinx, Inc. All rights reserved.
  \   \        This file contains confidential and proprietary 
  /   /        information of Xilinx, Inc. and is protected under U.S. 
 /___/   /\    and international copyright and other intellectual 
 \   \  /  \   property laws. 
  \___\/\___\ 
 
*************************************************************************
Vendor: Xilinx
Current readme.txt Version: 1.6
Date Last Modified: 24JUL2014
Associated Filename: rdf0281-kc705-base-trd-2014-2.zip
Associated Document: UG 882 and UG 883
Supported Device: Kintex-7 (xc7k325t)
Purpose: Targeted Reference Design
TRD version: 1.6  

***************************************************************************
Disclaimer: 

      This disclaimer is not a license and does not grant any rights to 
      the materials distributed herewith. Except as otherwise provided in 
      a valid license issued to you by Xilinx, and to the maximum extent 
      permitted by applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE 
      "AS IS" AND WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL 
      WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, 
      INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, 
      NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and 
      (2) Xilinx shall not be liable (whether in contract or tort, 
      including negligence, or under any other theory of liability) for 
      any loss or damage of any kind or nature related to, arising under 
      or in connection with these materials, including for any direct, or 
      any indirect, special, incidental, or consequential loss or damage 
      (including loss of data, profits, goodwill, or any type of loss or 
      damage suffered as a result of any action brought by a third party) 
      even if such damage or loss was reasonably foreseeable or Xilinx 
      had been advised of the possibility of the same.

Critical Applications:

      Xilinx products are not designed or intended to be fail-safe, or 
      for use in any application requiring fail-safe performance, such as 
      life-support or safety devices or systems, Class III medical 
      devices, nuclear facilities, applications related to the deployment 
      of airbags, or any other applications that could lead to death, 
      personal injury, or severe property or environmental damage 
      (individually and collectively, "Critical Applications"). Customer 
      assumes the sole risk and liability of any use of Xilinx products 
      in Critical Applications, subject only to applicable laws and 
      regulations governing limitations on product liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS 
FILE AT ALL TIMES.


*******************************************************************************/

1.  REQUIREMENTS
    ------------
    a. Hardware
      i.  KC705 Evaluation Kit (Rev D board with GES silicon)
      ii. PC with PCI Express slot (x4/x8/x16 PCIe v3.0 compliant)
      iii.Keyboard & Mouse

    b. Software
      i. Vivado Design Suite version 2014.2
        - Vivado install
        - Before running any command line scripts, refer to the Xilinx Design Tools: Installation and Licensing
          document to learn how to set the appropriate environment variables for your operating system.
          All scripts mentioned in this readme file assume the XILINX environment variable has been set.

2. DIRECTORY STRUCTURE
   -------------------

    k7_pcie_dma_ddr3_base : Main TRD folder
    |
    |--design : Design files and scripts for simulation & implementation
    |  |
    |  |
    |  |---ip_cores : IPs delivered from Xilinx CORE Generator and Third Party IPs
    |  |   |
    |  |   |----dma (IP provided by Northwest Logic - Xilinx Third Party Alliance version 1.08). This TRD is using
    |  |            the evaluation version of the DMA which times out after 12 hours. To use the full version 
    |  |            please contact Northwest Logic)
    |  |    
    |  |---ip_catalog : IPs delivered from Xilinx IP Catalog - only the xci files are included
    |  |
    |  |---implement  : Implementation scripts, XDC files specific to KC705. Sub folders for Vivado flow.
    |  |
    |  |---sim : Simulation folder
    |  |   |
    |  |   |----include : Common files for simulation
    |  |   |
    |  |   |----mti : QuestaSim simulation script
    |  |
    |  |---source : Source deliverables for the TRD
    |  |   |
    |  |   |----modified_ip_files : IP source files that have been modified for this reference design 
    |  |        | 
    |  |        |---- dma : packet_dma_axi.v is top level dma file created for this TRD. It is based on top level 
    |  |        |           dma file provided by NWL. register_map.v has been modified to add TRD regsiters.
    |  |        |
    |  | 
    |  |---tb : Testbench for out-of-box simulation
    |      |
    |      |----dsport  : Wrapper files and tasks for downstream port
    |
    |---configuring_kc705 : bit and mcs files for x4 Gen2 and x8 Gen1 PCIe configuration. This directory also has
    |                       script for programming the BPI flash on KC705. Make sure to install LabTools before you run the script.
    |
    |--doc  : Getting Started Guide for the TRD
    |
    |--linux_driver : Software driver source and GUI related files
    |
    |--k7_lin_trd_quickstart : Script that automatically runs Makefiles for easy operation

3. IMPLEMENTATION FLOW
   -------------------
   a. Vivado flow for Windows and Linux 

      For Windows,navigate to 'design/implement/vivado_flow_x4gen2' in the Vivado tcl shell or Vivado tcl command window 
      Run the following command to invoke the Vivado GUI. The design with x4 gen2 PCIe configuration is loaded. 
        $ launch_x4gen2.bat
      
      For Linux,navigate to 'design/implement/vivado_flow_x4gen2' and run the following command to invoke the Vivado GUI.
      The design with x4 gen2 PCIe configuration is loaded.
	$ vivado -source vivado_flow_x4gen2.tcl	
        
        Click on Run Synthesis in the Project Manager window. A window with message "Synthesis Completed Successfully" will appear
        after Vivado generates a design netlist. Close the message window.  Alternatively, you can skip this step and go straight
        to Run Implementation.  The tool will tell you that you need to run synthesis first, click OK to close dialog box to run
        synthesis immediately followed by implementation.

        Click on Run Implementation in the Project Manager window. A window with message "Implementation Completed Successfully" 
        will appear after place and route processes are done. Close the message window.

        Click on Generate Bitstream. Click on OK to generate bitstream.
        A window with message "Generate Bitstream Completed Successfully" will appear at the end of this process and a design bit file 
        will be available in design/implement/vivado_flow_x4gen2/vivado_proj_1/k7_pcie_dma_ddr3_base_x4gen2.runs/impl_1
       
      Close Vivado GUI.
	
      To run Vivado tool flow in batch mode:
          vivado -mode batch -source k7_base_cmd.tcl       

      Design bit file will be available in design/implement/vivado_flow_x4gen2/vivado_proj_1/k7_pcie_dma_ddr3_base_x4gen2.runs/impl_1 folder.
	
      Navigate to design/implement/vivado_flow_x4gen2 folder and run the following command to generate a mcs file. 
	    vivado -source k7_base_x4_gen2_flash.tcl    
     
      A MCS file (KC705.mcs)  will be available in k7_pcie_dma_ddr3_base/configuring_kc705 folder
     
      To program the flash using Vivado follow the below steps:
           i) Open a hardware session in the Vivado GUI.
          ii) Connect to the hardware device (KC705 board).
         iii) Navigate to k7_pcie_dma_ddr3_base/configuring_kc705 directory and source program_flash_x4_gen2.tcl script.   
      The program on the flash will be loaded after the board undergoes a power cycle ( power off and on).

    b. In Windows,navigate to 'design/implement/vivado_flow_x8gen1' in the Vivado tcl shell or Vivado tool command window
       Run the following command to invoke the Vivado GUI. The design with x8 gen1 PCIe configuration is loaded. 
        $ launch_x8gen1.bat
      
       For Linux,navigate to 'design/implement/vivado_flow_x8gen1' and run the following command to invoke the Vivado GUI.
       The design with x8 gen1 PCIe configuration is loaded.
	$ vivado -source vivado_flow_x8gen1.tcl	
 
        Click on Run Synthesis in the Project Manager window. The first time Synthesis is run, all the necessary IP's from IP Catalog
        will be generated automatically.  A window with message "Synthesis Completed Successfully" will appear
        after Vivado generates a design netlist. Close the message window.  Alternatively, you can skip this step and go straight
        to Run Implementation.  The tool will tell you that you need to run synthesis first, click OK to close dialog box to run
        synthesis immediately followed by implementation.

        Click on Run Implementation in the Project Manager window. A window with message "Implementation Completed Successfully" 
        will appear after place and route processes are done. Close the message window.

        Click on Generate Bitstream. Click on OK to generate bitstream.
        A window with message "Generate Bitstream Completed Successfully" will appear at the end of this process and a design bit file 
        will be available in design/implement/vivado_flow_x8gen1/vivado_proj_1/k7_pcie_dma_ddr3_base_x8gen1.runs/impl_1
       
      Close Vivado GUI.

       To run Vivado tool flow in batch mode:
          vivado -mode batch -source k7_base_cmd.tcl       

       Design bit file will be available in design/implement/vivado_flow_x8gen1/vivado_proj_1/k7_pcie_dma_ddr3_base_x8gen1.runs/impl_1 folder.
      
      Navigate to design/implement/vivado_flow_x8gen1 folder and run the following command to generate a mcs file.
	    vivado -source k7_base_x8_gen1_flash.tcl    
     
      A MCS file (KC705_x8gen1.mcs)  will be available in k7_pcie_dma_ddr3_base/configuring_kc705 folder
     
      To program the flash using Vivado follow the below steps:
           i) Open a hardware session in the Vivado GUI.
          ii) Connect to the hardware device (KC705 board).
         iii) Navigate to k7_pcie_dma_ddr3_base/configuring_kc705 directory and source program_flash_x8_gen1.tcl script.   
      The program on the flash will be loaded after the board undergoes a power cycle ( power off and on).


      Note: If the configuration width selected is x8 gen1 then the following changes need to be made to the driver code to work 
      with the TRD
        - Set PCI_DEVICE_ID_DMA to 0x7081 in linux_driver/xdma/xdma_base.c

 

4. SIMULATION FLOW
   ---------------
   Simulation using QuestaSim 10.2a

         Requires Xilinx tools and QuestaSim.
         a. Simulation library compilation
              Compile Xilinx Simulation Libraries using compxlib.
      
         b. Simulation using the scripts provided
              Navigate to the 'design/sim/mti' folder.
              Set MODELSIM environment variable to point to the 'modelsim.ini' file which contains paths to the compiled libraries.
              Run the following script to simulate the design with PCIe core confgiured as x4 link.
              Windows: $ launch_sim_x4gen2.bat
              Linux:   $ ./launch_sim_x4gen2.sh
              
              After libraries and the design files are compiled, the QuestaSim tool shows a prompt in Transcript window.
              Run the following command to start simulation
               
              VSIM > run -all
      
              Run the following script to simulate the design with PCIe core confgiured as x8 link.
              Windows: $ launch_sim_x8gen1.bat
              Linux:   $ ./launch_sim_x8gen1.sh
              
              After libraries and the design files are compiled, the QuestaSim tool shows a prompt in Transcript window.
              Run the following command to start simulation
               
              VSIM > run -all
      
   Simulation using Vivado Simulator 

         a. Simulation using the scripts provided
              Navigate to the 'design/sim/xsim' folder.
              Run the following script to simulate the design with PCIe core confgiured as x4 link.
              Windows: $ launch_sim_x4gen2.bat
              Linux:   $ ./launch_sim_x4gen2.sh
      
              Run the following script to simulate the design with PCIe core confgiured as x8 link.
              Windows: $ launch_sim_x8gen1.bat
              Linux:   $ ./launch_sim_x8gen1.sh

          NOTE: Simulation using Vivado Simulator requires the machine to be 64-bit with minimum of 8GB RAM.

5. TESTING
   -------
   To test the design in hardware, refer to Getting Started Chapter in UG882 available under "Docs & Designs" tab of
   http://www.xilinx.com/products/boards-and-kits/EK-K7-KC705-G.htm


6. KNOWN RESTRICTIONS
   ------------------
   
   a. If design files are changed there is a possibility that timing will not be met. Users may try different options 
      available in Vivado implementation to meet timing. Every effort has been made to have the design meet timing with 
      default options.
     
   b. On some Intel x58 chipsets the Base TRD driver doesn't unload successfully(driver module xdma_k7 is not removed). 
      UG882 details two ways to remove the driver 
        i.    Using the  k7_trd_lin_quickstart script, which removes the driver when the Peformance Montior is closed.
        ii.   Using the make remove command (Appendix E).
      This issue can be resolved as follows.When Fedora 16 Live is booting, 
        i.    Press tab for full configuration options on menu items. 
        ii.   Add iommu=pt64 at the end of the line vmlinuz0 initrd=initrd0 ...rd.md=0 rd.dm=0
        iii.  Hit the enter key to boot with this option. 
  
   c. On some Intel x58 chipsets the DMA performance on Base TRD fluctuates and is lower than expected. 
      This issue can be resolved as follows.When Fedora 16 Live is booting, 
        i.    Press tab for full configuration options on menu items. 
        ii.   Add iommu=pt64 at the end of the line vmlinuz0 initrd=initrd0 ...rd.md=0 rd.dm=0
        iii.  Hit the enter key to boot with this option. 

   d. Fedora 16 may not boot on all PCs. Here are two scenarios.  
        i.    Fedora 16 was released November 8, 2011 and uses open source drivers. PC systems containing motherboards with 
              chipsets or graphics cards that are developed later than this date or from a vendor who does not support the 
              open source drivers many not work. Note* NVIDIA does not directly support the open source drivers 
              so some cards may be not compatible with FC16. This link will give more details on NVIDIA HW families. 
              http://nouveau.freedesktop.org.
        ii.   Fedora 16 uses GNOME 3  for its graphical environment and this requires 3d HW acceleration support. Very old 
              graphics cards many not support 3d HW acceleration and may not be compatible. 
      If your system has an incompatible graphics card then a new  low cost card such as ATI Radeon HD 6450 or Radeon HD 4670 
      or NVIDIA Geforce 210  can be purchased that are compatible with Fedora 16. These cards have been tested at Xilinx. 
 
   e. When generating the MIG core in Core Generator or IP Catalog, validating the pinout (Step 'f' in the IP Core Generation 
      instructions, above) may cause a bunch of warnings about slew rate values.  These warnings can be safely ignored.

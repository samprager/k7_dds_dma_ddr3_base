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

/***************************** System Include Files **************************/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <glib.h>

/***************************** Local Include Files ***************************/
#include <xpmon_be.h>
#include "ConfigGui.h"
#include "xp_types.h"

/***************************** Preprocessor Constants ************************/

#if  !defined(K7_TRD)
Warning! Cannot without K7 macros.
#endif

#define UP      1
#define DOWN    2

#define MAINLABEL   "Xilinx Performance & Status Monitor - Kintex-7 Base TRD"

#ifdef K7_TRD
#define LABEL1      "Raw Data Path0:"
#define LABEL2      "Raw Data Path1:"
#endif

#define GLABEL1     "Transmit"
#define GLABEL2     "Receive"
#define FILENAME    "/dev/xdma_stat"
#define XLNX_LOGO   "xlnx_logo.gif"
#define START_TEST  "Start Test"
#define STOP_TEST   "Stop Test"

#ifdef K7_TRD
#define DEV_LOGO    "k7_logo.png"
#endif

#define MAX_Y_CNT   4       /* Max gradations on Y-axis of graph */
#define MAX_Y       16      /* Max Y-value displayed on graph */
#define DIVISOR     (1024*1024*1024)    /* Graph is in Gbits/s */

#define SDIVISOR    (1024*1024*1024)    /* Main status page must show Gbps */
#define MULTIPLIER  8                   /* Because graph is in bits/s */
#define RATE        "Throughput (Gbps)" /* For showing on main status page */

#define MIN_PKTSIZE 64
#define MAX_PKTSIZE (32*1024)

char * Yaxis[MAX_Y_CNT] =
{ "   16", "   12", "    8", "    4" };


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/***************************** Function Prototypes ***************************/
///* Function to clear Text in the Text view */
//gboolean ClearTextView(GtkWidget *Widget, gpointer Data);

gboolean quit_window( GtkWidget *Widget, gpointer Data );

gboolean LBCallback1(GtkWidget *Widget, gpointer String);
gboolean LBCallback2(GtkWidget *Widget, gpointer String);
gboolean NonLBCallback1(GtkWidget *Widget, gpointer String);
gboolean NonLBCallback2(GtkWidget *Widget, gpointer String);
gboolean StartTestCallback(GtkWidget *Widget, gpointer String);
gboolean StatusCallback(GtkWidget *Widget, gpointer String);
gboolean StartTest(int engine);
gboolean StopTest(int engine);

gboolean DisplayMainPage(GtkWidget *Widget, gpointer  String);

void DrawWidget(struct screen *MyGuiScreen,void *PWidgets[],int i);
gboolean DrawPayloadGraph1(GtkWidget *Widget, gpointer Data);
gboolean DrawPayloadGraph2(GtkWidget *Widget, gpointer Data);
gboolean DrawPayloadGraph3(GtkWidget *Widget, gpointer Data);
gboolean DrawPayloadGraph4(GtkWidget *Widget, gpointer Data);
gboolean DrawTRNGraph1(GtkWidget *Widget, gpointer Data);
gboolean DrawTRNGraph2(GtkWidget *Widget, gpointer Data);
gboolean DrawGraph (GtkWidget *, int, int, int, int, char *, int *, int, int, int *, int, int);

/************************** Variable Definitions *****************************/

char xlnx_logo[PATH_MAX];
char dev_logo[PATH_MAX];
char mainlabel[128];

#define MAX_STATS 350

struct
{
    int Engine;         /* Engine number - for communicating with driver */
    char *name;         /* Name to be used in Setup screen */
    int mode;           /* TX/RX - incase of specific screens */
} DMAConfig[MAX_ENGS] =
{
    {0, LABEL1, TX_MODE },
    {32, LABEL1, RX_MODE },
    {1, LABEL2, TX_MODE },
    {33, LABEL2, RX_MODE }
};

struct
{
    float LBR;
    unsigned int LAT;
    unsigned int LWT;
    unsigned int BDe;
    unsigned int BDSe;
    unsigned int BDs;
    unsigned int Bufs;
    unsigned int MinPkt;
    unsigned int MaxPkt;
    unsigned int TestMode;
    int TXEnab;
    int LBEnab;
    int IntEnab;
    int PktGenEnab;
    int PktChkEnab;
} EngStats[MAX_ENGS] = { 0 };

struct
{
    float LTX;
    float LRX;
} TRNStats = { 0 };

/* array of Pointers to Signals emitted from Widgets */
char *Signal[]={"delete_event", //0
                 "activate",    //1
                 "clicked",     //2
                 "toggled",     //3
                 "pressed",     //4
                 "changed",     //5
                 "expose_event",//6
                 "insert_text"  //7
                 };
enum SIndex
{
    DELETE=0,
    ACTIVATE,
    CLICKED,
    TOGGLED,
    PRESSED,
    CHANGED,
    EXPOSE_EVENT,
    INSERT_TEXT
} Sig;

/* IMPORTANT: Indices are used to point to particular widget during callback
 * hence order of index should be exactly same as the order of widgets
 * definition.
 */

enum WidgetIndex        /* Corresponds to GuiWidgets */
{
    MAIN_WINDOW_INDEX=0,
    VPANED_INDEX,
    MAIN_TAB_INDEX,

    ALIGN_IMAGE1_INDEX=3,               // Xilinx Logo
    ALIGN_IMAGE2_INDEX=5,               // Device Logo

    LABEL1_INDEX=8,                     // Test Setup
    ENABLE_LOOPBACK_LABEL1_INDEX=9,
    ENABLE_LOOPBACK_BUTTON1_INDEX,

    MIN_PKT_LABEL1_INDEX=11,
    PKT_ENTRY1_INDEX,
    TXCHECKER_LABEL1_INDEX,
    TXCHECKER_BUTTON1_INDEX,
    RXGENERATOR_LABEL1_INDEX,
    RXGENERATOR_BUTTON1_INDEX,
    START_BUTTON1_INDEX,

    LOOPBACK_LABEL2_INDEX=20,
    LOOPBACK_BUTTON2_INDEX,
    TXCHECKER_LABEL2_INDEX,
    TXCHECKER_BUTTON2_INDEX,
    RXGENERATOR_LABEL2_INDEX,
    RXGENERATOR_BUTTON2_INDEX,
    PKT_ENTRY2_INDEX=27,
    START_BUTTON2_INDEX,

    NOTEBOOK_INDEX=29,
    FRAME_INDEX,
    TEXTVIEW_FRAME_INDEX=32,
    SCROLLED_WINDOW_INDEX,
    TEXTVIEW_INDEX
} MainIndex;

enum StatusIndex        /* Corresponds to StatusWidgets */
{
    PERF_FRAME_INDEX=0,
    PERF_TABLE_INDEX,
    PERF_DRAWING1_INDEX,
    PERF_DRAWING2_INDEX,
    PERF_DRAWING3_INDEX,
    PERF_DRAWING4_INDEX,

    START_FRAME_INDEX=6,
    START_TABLE_INDEX,  /* No macros for the label widgets */

    LBR1_ENTRY_INDEX=19,
    LAT1_ENTRY_INDEX,
    LWT1_ENTRY_INDEX,
    BDE1_ENTRY_INDEX,
    BDSE1_ENTRY_INDEX,
    BDS1_ENTRY_INDEX,
    BUFS1_ENTRY_INDEX,

    LBR2_ENTRY_INDEX=26,
    LAT2_ENTRY_INDEX,
    LWT2_ENTRY_INDEX,
    BDE2_ENTRY_INDEX,
    BDSE2_ENTRY_INDEX,
    BDS2_ENTRY_INDEX,
    BUFS2_ENTRY_INDEX,

    INTENAB1_ENTRY_INDEX=34,
  //  LBENAB1_LABEL_INDEX,
  //  LBENAB1_ENTRY_INDEX,

    LBR3_ENTRY_INDEX=46,
    LAT3_ENTRY_INDEX,
    LWT3_ENTRY_INDEX,
    BDE3_ENTRY_INDEX,
    BDSE3_ENTRY_INDEX,
    BDS3_ENTRY_INDEX,
    BUFS3_ENTRY_INDEX,

    LBR4_ENTRY_INDEX=53,
    LAT4_ENTRY_INDEX,
    LWT4_ENTRY_INDEX,
    BDE4_ENTRY_INDEX,
    BDSE4_ENTRY_INDEX,
    BDS4_ENTRY_INDEX,
    BUFS4_ENTRY_INDEX,

    INTENAB3_ENTRY_INDEX=61,

    TRN_TX_ENTRY_INDEX=63,
    TRN_RX_ENTRY_INDEX=65,

    PCIE_LINK_STATUS_ENTRY_INDEX=69,
    PCIE_LINK_SPEED_ENTRY_INDEX=71,
    PCIE_LINK_WIDTH_ENTRY_INDEX=73,
    PCIE_INTMODE_ENTRY_INDEX=75,

    PCIE_VENDOR_ENTRY_INDEX=77,
    PCIE_DEVICE_ENTRY_INDEX=79,
    PCIE_MPS_ENTRY_INDEX=81,
    PCIE_MRRS_ENTRY_INDEX=83,

    PCIE_FC_PH_ENTRY_INDEX=87,
    PCIE_FC_NPH_ENTRY_INDEX=89,
    PCIE_FC_CPLH_ENTRY_INDEX=91,
    PCIE_FC_PD_ENTRY_INDEX=93,
    PCIE_FC_NPD_ENTRY_INDEX=95,
    PCIE_FC_CPLD_ENTRY_INDEX=97,

    TRN_FRAME_INDEX=98,
    TRN_TABLE_INDEX,
    TRN_DRAWING1_INDEX,
    TRN_DRAWING2_INDEX
};

enum SetupIndex         /* Corresponds to SetupWidgets */
{
    SETUP_FRAME_INDEX=0,
};

PCIState PCIInfo;

static int IterCounter = 0;

#define START_PATH1     0x0001
#define START_PATH2     0x1000

/* flag to store Test status */
static int TestStartFlag1=ENABLE_LOOPBACK;
static int TestStartFlag2=ENABLE_LOOPBACK;

/* File pointer for the stats file */
int statsfd = -1;

/* Y-axis points */
int max_y_count=MAX_Y_CNT;

/* Number of points to plot */
int DMAarray[MAX_ENGS][400], SWarray[MAX_ENGS][400], TRNarray[2][400];

/* Variables to hold max values for graph plotting.
 * The Write value always points to the location where the next reading
 * will be written. So, the graph should always display from Read to the
 * value before Write.
 */
static int dsRead[MAX_ENGS], dsWrite[MAX_ENGS], dsNum[MAX_ENGS];
static int ssRead[MAX_ENGS], ssWrite[MAX_ENGS], ssNum[MAX_ENGS];
static int tsRead[2], tsWrite[2], tsNum[2];

/* Graph refresh duration */
#define GRAPH_TIMEOUT        1000
static guint PerfGraphID;

/* level of logging */
enum print display=MSG_INFO;

/* variables related to TextView terminal */
static GString *TextString;
static GtkTextMark *BeginPos;


/* The start frame is to be connected to the notebook of the main page.
 * If the argument passed to the callback is "-1" it represents connect
 * to the main GUI screen and not to the setup screen.
 */
/* Struct definition for Main window */
struct screen GuiScreen[]=
{/*   name      type    left,right,top,bottom,  parent, row, col, active, func pointer, signal, argument */
    { mainlabel, WINDOW, 20,0,0,0, -1, 900,740, TRUE,
    /*0 TopLevel window */               quit_window, DELETE, NULL },
    { "", VPANED, 0,0,0,0, MAIN_WINDOW_INDEX, 0,0, TRUE,
    /*1 Panel */                         NULL, (int)NULL, NULL },
    { "table", TABLE, UP,0,0,0, VPANED_INDEX, 15,15, FALSE,
    /*2 TopLevel table */                NULL, (int)NULL, NULL },

    { "Align", ALIGN, 0,1,0,1, MAIN_TAB_INDEX, 0,0, TRUE,
    /*3 alignment */                     NULL, (int)NULL, NULL },
    { dev_logo, IMAGE, 0,0,0,0, ALIGN_IMAGE1_INDEX, 0,0, TRUE,
    /*4 Left Image */                    NULL, (int)NULL, NULL },
    { "Align", ALIGN, 13,14,0,1, MAIN_TAB_INDEX, 0,0, TRUE,
    /*5 alignment */                     NULL, (int)NULL, NULL },
    { xlnx_logo, IMAGE, 0,0,0,0, ALIGN_IMAGE2_INDEX, 0,0, TRUE,
    /*6 Right Image */                   NULL, (int)NULL, NULL },
    { "Separator", HSEPARATOR, 0,14,1,2, MAIN_TAB_INDEX, 0,0, TRUE,
    /*7 Horiz. Separator */                   NULL, (int)NULL, NULL },

    { LABEL1, LABEL, 1,3,2,3, MAIN_TAB_INDEX, 0,0, TRUE,
    /*8 1st path Label*/                 NULL, (int)NULL, "blue" },
    { "Enable TX->RX Loopback", LABEL, 6,7,2,3, MAIN_TAB_INDEX,
    /*9 1st path */                      0,0, FALSE, NULL, (int)NULL, NULL },
    { "", RCHECK_BUTTON, 7,8,2,3, MAIN_TAB_INDEX, 0,0, FALSE,
    /*10 1st path */                      LBCallback1, CLICKED, NULL },
    { "Packet Size", LABEL, 8,9,2,3, MAIN_TAB_INDEX, 0,0, TRUE,
    /*11 1st path label */               NULL, (int)NULL, "black" },
    { "", ENTRY, 9,10,2,3, MAIN_TAB_INDEX, 0,6, TRUE,
    /*12 1st path entry */               NULL, (int)NULL, NULL },
    { "Enable TX Checker", LABEL, 6,7,3,4, MAIN_TAB_INDEX, 0,0,FALSE,
    /*13 1st path label */               NULL, (int)NULL, NULL },
    { "", RCHECK_BUTTON, 7,8,3,4, MAIN_TAB_INDEX, 0,0, FALSE,
    /*14 1st path entry */               NonLBCallback1, CLICKED, NULL },
    { "Enable RX Generator", LABEL, 6,7,4,5, MAIN_TAB_INDEX,
    /*22 2nd path */                      0,0, FALSE, NULL, (int)NULL, NULL },
    { "", RCHECK_BUTTON, 7,8,4,5, MAIN_TAB_INDEX, 0,0, FALSE,
    /*23 2nd path */                     NonLBCallback1, CLICKED, NULL },
    { START_TEST, BUTTON, 12,13,2,3, MAIN_TAB_INDEX, 0,0, TRUE,
    /*15 1st path */                     StartTestCallback, CLICKED, NULL },
    { "Separator", HSEPARATOR, 0,14,5,6, MAIN_TAB_INDEX, 0,0, TRUE,
    /*16 Horiz. Separator */                   NULL, (int)NULL, NULL },

    { LABEL2, LABEL, 1,3,6,7, MAIN_TAB_INDEX, 0,0, TRUE,
    /*17 2nd path Label*/                 NULL, (int)NULL, "blue" },
    { "Enable TX->RX Loopback", LABEL, 6,7,6,7, MAIN_TAB_INDEX, 0,0, FALSE,
    /*18 2nd path */                      NULL, (int)NULL, NULL },
    { "", RCHECK_BUTTON, 7,8,6,7, MAIN_TAB_INDEX, 0,0, FALSE,
    /*19 2nd path */                      LBCallback2, CLICKED, NULL },
    { "Enable TX Checker", LABEL, 6,7,7,8, MAIN_TAB_INDEX,
    /*20 2nd path */                      0,0, FALSE, NULL, (int)NULL, NULL },
    { "", RCHECK_BUTTON, 7,8,7,8, MAIN_TAB_INDEX, 0,0, FALSE,
    /*21 2nd path */                      NonLBCallback2, CLICKED, NULL },
    { "Enable RX Generator", LABEL, 6,7,8,9, MAIN_TAB_INDEX,
    /*22 2nd path */                      0,0, FALSE, NULL, (int)NULL, NULL },
    { "", RCHECK_BUTTON, 7,8,8,9, MAIN_TAB_INDEX, 0,0, FALSE,
    /*23 2nd path */                      NonLBCallback2, CLICKED, NULL },

    { "Packet Size", LABEL, 8,9,6,7, MAIN_TAB_INDEX, 0,0, TRUE,
    /*24 2nd path label */               NULL, (int)NULL, "black" },
    { "", ENTRY, 9,10,6,7, MAIN_TAB_INDEX, 0,6, TRUE,
    /*25 2nd path entry */               NULL, (int)NULL, NULL },

    { START_TEST, BUTTON, 12,13,6,7, MAIN_TAB_INDEX, 0,0, TRUE,
    /*26 2nd path */                     StartTestCallback, CLICKED, NULL },

    { "Notebook", NOTEBOOK, 0,14,9,11, MAIN_TAB_INDEX, 0,0, TRUE,
    /*27 Notebook */                     NULL, (int)NULL, NULL },
    { "System Status", FRAME, 20,0,0,0, NOTEBOOK_INDEX, 720,400, TRUE,
    /*28 Display status & graphs */      StatusCallback, EXPOSE_EVENT, NULL },
    { "Table in Notepad", TABLE, 0,0,0,0, FRAME_INDEX, 16,8, TRUE,
    /*29 Table */                        NULL, (int)NULL, NULL },
    { NULL, FRAME, 0,0,0,0, VPANED_INDEX, 300,300, TRUE,
    /*30 Frame to hold text view */      NULL, (int)NULL, NULL },
    { "Scrolled View", SCROLLED_WINDOW, 0,0,0,0, TEXTVIEW_FRAME_INDEX, 0,0,
    /*31 Scrolled window in textview */  TRUE, NULL, (int)NULL, NULL },
    { "Text View", TEXTVIEW, 0,0,0,0, SCROLLED_WINDOW_INDEX, 0,0, FALSE,
    /*32 Textview*/                      NULL, (int)NULL, NULL }
};

/* screen definition when start button gets pressed */
struct screen StatusScreen[]=
{
/* name type left,right,top,bottom, parent, row,col, active, func pointer, signal, argument */
    /* Graphs */
    { "Payload Statistics", FRAME, 20,0,0,0, NOTEBOOK_INDEX, 720,430, TRUE,
    /*0*/                               NULL, (int)NULL, (gpointer)-1 },
    { "table in Graph", TABLE, 0,0,0,0, PERF_FRAME_INDEX, 1,2, TRUE,
    /*1*/                               NULL, (int)NULL, NULL },
    { "DMA Transmit Statistics", DRAWING_AREA, 0,1,0,1, PERF_TABLE_INDEX, 400,400,
    /*2*/                  TRUE, DrawPayloadGraph1, EXPOSE_EVENT, (gpointer)1 },
    { "DMA Receive Statistics", DRAWING_AREA, 1,2,0,1, PERF_TABLE_INDEX, 400,400,
    /*3*/                  TRUE, DrawPayloadGraph2, EXPOSE_EVENT, (gpointer)2 },
    { "DMA Transmit Statistics", DRAWING_AREA, 0,1,1,2, PERF_TABLE_INDEX, 400,400,
    /*4*/                  TRUE, DrawPayloadGraph3, EXPOSE_EVENT, (gpointer)3 },
    { "DMA Receive Statistics", DRAWING_AREA, 1,2,1,2, PERF_TABLE_INDEX, 400,400,
    /*5*/                  TRUE, DrawPayloadGraph4, EXPOSE_EVENT, (gpointer)4 },

    { "System Status", FRAME, 20,0,0,0, NOTEBOOK_INDEX, 720,400, TRUE,
    /*6 FRAME in start screen */        NULL, (int)NULL, (gpointer)-1 },
    { "table in Notepad", TABLE, 0,0,0,0, START_FRAME_INDEX, 40,14, TRUE,
    /*7 TopLevel table */               NULL, (int)NULL, NULL },
    { "DMA & Software Status", LABEL, 0,3,0,2, START_TABLE_INDEX, 0,0, TRUE,
    /*8 DMA Status label */             NULL, (int)NULL, "blue" },

    /* 1st Path */
    { LABEL1, LABEL, 0,2,3,5, START_TABLE_INDEX, 0,0, TRUE,
    /*9 1st path label */               NULL, (int)NULL, "blue" },
    { GLABEL1, LABEL, 3,5,3,5, START_TABLE_INDEX, 10,0, FALSE,
    /*10 1st path Tx Stats */           NULL, (int)NULL, "blue" },
    { GLABEL2, LABEL, 5,7,3,5, START_TABLE_INDEX, 10,0, FALSE,
    /*11 1st path Rx Stats */           NULL, (int)NULL, "blue" },

    /* Labels */
    { RATE, LABEL, 0,3,6,8, START_TABLE_INDEX, 10,0, FALSE,
    /*12 1st path CompBytes */          NULL, (int)NULL, "black" },
    { "DMA Active Time (ns)", LABEL, 0,3,9,11, START_TABLE_INDEX, 0,0, TRUE,
    /*13*/                              NULL, (int)NULL, "black" },
    { "DMA Wait Time (ns)", LABEL, 0,3,12,14, START_TABLE_INDEX, 0,0, TRUE,
    /*14*/                              NULL, (int)NULL, "black" },
    { "BD Errors", LABEL, 0,2,15,17, START_TABLE_INDEX, 0,0, TRUE,
    /*15*/                              NULL, (int)NULL, "black" },
    { "BD Short Errors", LABEL, 0,2,18,20, START_TABLE_INDEX, 0,0, TRUE,
    /*16*/                              NULL, (int)NULL, "black" },
    { "# SW BDs", LABEL, 0,2,21,23, START_TABLE_INDEX, 0,0, TRUE,
    /*17*/                              NULL, (int)NULL, "black" },
    { "# SW Buffers", LABEL, 0,2,24,26, START_TABLE_INDEX, 0,0, TRUE,
    /*18*/                              NULL, (int)NULL, "black" },

    /* TX stats */
    { "", ENTRY, 3,5,6,8, START_TABLE_INDEX, 10,0, FALSE,
    /*19*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 3,5,9,11, START_TABLE_INDEX, 0,0, TRUE,
    /*20*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 3,5,12,14, START_TABLE_INDEX, 10,0, FALSE,
    /*21*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 3,5,15,17, START_TABLE_INDEX, 10,0, FALSE,
    /*22*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 3,5,18,20, START_TABLE_INDEX, 10,0, FALSE,
    /*23*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 3,5,21,23, START_TABLE_INDEX, 10,0, FALSE,
    /*24*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 3,5,24,26, START_TABLE_INDEX, 10,0, FALSE,
    /*25*/                              NULL, (int)NULL, "black" },

    /* RX stats */
    { "", ENTRY, 5,7,6,8, START_TABLE_INDEX, 10,0, FALSE,
    /*26*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 5,7,9,11, START_TABLE_INDEX, 0,0, TRUE,
    /*27*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 5,7,12,14, START_TABLE_INDEX, 10,0, FALSE,
    /*28*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 5,7,15,17, START_TABLE_INDEX, 10,0, FALSE,
    /*29*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 5,7,18,20, START_TABLE_INDEX, 10,0, FALSE,
    /*30*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 5,7,21,23, START_TABLE_INDEX, 10,0, FALSE,
    /*31*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 5,7,24,26, START_TABLE_INDEX, 10,0, FALSE,
    /*32*/                              NULL, (int)NULL, "black" },

    { "Interrupts Enabled", LABEL, 0,3,28,30, START_TABLE_INDEX, 0,0, TRUE,
    /*33*/                              NULL, (int)NULL, "black" },
    { "", CHECK_BUTTON, 3,5,28,30, START_TABLE_INDEX, 10,0, FALSE,
    /*34*/                              NULL, (int)NULL, "black" },
    { "", HSEPARATOR, 0,14,30,31, START_TABLE_INDEX, 0,0, FALSE,
    /*37*/                              NULL, (int)NULL, (gpointer)NULL },

    /* 2nd Path */
    { LABEL2, LABEL, 7,9,3,5, START_TABLE_INDEX, 0,0, TRUE,
    /*38 1st path label */              NULL, (int)NULL, "blue" },
    { GLABEL1, LABEL, 10,12,3,5, START_TABLE_INDEX, 10,0, FALSE,
    /*39 1st path Tx Stats */           NULL, (int)NULL, "blue" },
    { GLABEL2, LABEL, 12,14,3,5, START_TABLE_INDEX, 10,0, FALSE,
    /*40 1st path Rx Stats */           NULL, (int)NULL, "blue" },

    /* Labels */
    { RATE, LABEL, 7,10,6,8, START_TABLE_INDEX, 10,0, FALSE,
    /*41 1st path CompBytes */          NULL, (int)NULL, "black" },
    { "DMA Active Time (ns)", LABEL, 7,10,9,11, START_TABLE_INDEX, 0,0, TRUE,
    /*42*/                              NULL, (int)NULL, "black" },
    { "DMA Wait Time (ns)", LABEL, 7,10,12,14, START_TABLE_INDEX, 0,0, TRUE,
    /*43*/                              NULL, (int)NULL, "black" },
    { "BD Errors", LABEL, 7,10,15,17, START_TABLE_INDEX, 0,0, TRUE,
    /*44*/                              NULL, (int)NULL, "black" },
    { "BD Short Errors", LABEL, 7,10,18,20, START_TABLE_INDEX, 0,0, TRUE,
    /*45*/                              NULL, (int)NULL, "black" },
    { "# SW BDs", LABEL, 7,10,21,23, START_TABLE_INDEX, 0,0, TRUE,
    /*46*/                              NULL, (int)NULL, "black" },
    { "# SW Buffers", LABEL, 7,10,24,26, START_TABLE_INDEX, 0,0, TRUE,
    /*47*/                              NULL, (int)NULL, "black" },

    /* TX stats */
    { "", ENTRY, 10,12,6,8, START_TABLE_INDEX, 10,0, FALSE,
    /*48*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 10,12,9,11, START_TABLE_INDEX, 0,0, TRUE,
    /*49*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 10,12,12,14, START_TABLE_INDEX, 10,0, FALSE,
    /*50*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 10,12,15,17, START_TABLE_INDEX, 10,0, FALSE,
    /*51*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 10,12,18,20, START_TABLE_INDEX, 10,0, FALSE,
    /*52*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 10,12,21,23, START_TABLE_INDEX, 10,0, FALSE,
    /*53*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 10,12,24,26, START_TABLE_INDEX, 10,0, FALSE,
    /*54*/                              NULL, (int)NULL, "black" },

    /* RX stats */
    { "", ENTRY, 12,14,6,8, START_TABLE_INDEX, 10,0, FALSE,
    /*55*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 12,14,9,11, START_TABLE_INDEX, 0,0, TRUE,
    /*56*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 12,14,12,14, START_TABLE_INDEX, 10,0, FALSE,
    /*57*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 12,14,15,17, START_TABLE_INDEX, 10,0, FALSE,
    /*58*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 12,14,18,20, START_TABLE_INDEX, 10,0, FALSE,
    /*59*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 12,14,21,23, START_TABLE_INDEX, 10,0, FALSE,
    /*60*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 12,14,24,26, START_TABLE_INDEX, 10,0, FALSE,
    /*61*/                              NULL, (int)NULL, "black" },

    { "Interrupts Enabled", LABEL, 7,10,28,30, START_TABLE_INDEX, 0,0, TRUE,
    /*62*/                              NULL, (int)NULL, "black" },
    { "", CHECK_BUTTON, 10,12,28,30, START_TABLE_INDEX, 10,0, FALSE,
    /*63*/                              NULL, (int)NULL, "black" },

    /* PCIe TRN Stats */
    { "PCIe Transmit (writes) (Gbps)", LABEL, 0,4,32,34, START_TABLE_INDEX, 0,0,
    /*64*/                              TRUE, NULL, (int)NULL, "blue" },
    { "", ENTRY, 4,5,32,34, START_TABLE_INDEX, 10,0, FALSE,
    /*65*/                              NULL, (int)NULL, "black" },
    { "PCIe Receive (reads) (Gbps)", LABEL, 0,4,35,37, START_TABLE_INDEX, 0,0,
    /*66*/                              TRUE, NULL, (int)NULL, "blue" },
    { "", ENTRY, 4,5,35,37, START_TABLE_INDEX, 10,0, FALSE,
    /*67*/                              NULL, (int)NULL, "black" },
    { "", HSEPARATOR, 0,14,38,39, START_TABLE_INDEX, 0,0, FALSE,
    /*68*/                              NULL, (int)NULL, (gpointer)NULL },

    /* PCIe Status */
    { "PCIe Endpoint Status", LABEL, 0,3,39,41, START_TABLE_INDEX, 0,0, TRUE,
    /*69*/                              NULL, (int)NULL, "blue" },
    { "Link Status", LABEL, 0,2,42,44, START_TABLE_INDEX, 0,0, TRUE,
    /*70*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 2,3,42,44, START_TABLE_INDEX, 10,0, FALSE,
    /*71*/                              NULL, (int)NULL, "black" },
    { "Link Speed", LABEL, 0,2,45,47, START_TABLE_INDEX, 0,0, TRUE,
    /*72*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 2,3,45,47, START_TABLE_INDEX, 10,0, FALSE,
    /*73*/                              NULL, (int)NULL, NULL },
    { "Link Width", LABEL, 0,2,48,50, START_TABLE_INDEX, 0,0, TRUE,
    /*74*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 2,3,48,50, START_TABLE_INDEX, 10,0, FALSE,
    /*75*/                              NULL, (int)NULL, "black" },
    { "Interrupts", LABEL, 0,2,51,53, START_TABLE_INDEX, 0,0, TRUE,
    /*76*/                             NULL, (int)NULL, "black" },
    { "", ENTRY, 2,3,51,53, START_TABLE_INDEX, 10,0, FALSE,
    /*77*/                             NULL, (int)NULL, NULL },

    { "Vendor ID", LABEL, 3,5,42,44, START_TABLE_INDEX, 0,0, TRUE,
    /*78*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 5,6,42,44, START_TABLE_INDEX, 10,0, FALSE,
    /*79*/                              NULL, (int)NULL, NULL },
    { "Device ID", LABEL, 3,5,45,47, START_TABLE_INDEX, 0,0, TRUE,
    /*80*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 5,6,45,47, START_TABLE_INDEX, 10,0, FALSE,
    /*81*/                              NULL, (int)NULL, NULL },
    { "MPS (bytes)", LABEL, 3,5,48,50, START_TABLE_INDEX, 0,0, TRUE,
    /*82*/                             NULL, (int)NULL, "black" },
    { "", ENTRY, 5,6,48,50, START_TABLE_INDEX, 10,0, FALSE,
    /*83*/                             NULL, (int)NULL, NULL },
    { "MRRS (bytes)", LABEL, 3,5,51,53, START_TABLE_INDEX, 0,0, TRUE,
    /*84*/                             NULL, (int)NULL, "black" },
    { "", ENTRY, 5,6,51,53, START_TABLE_INDEX, 10,0, FALSE,
    /*85*/                             NULL, (int)NULL, NULL },

    { "", VSEPARATOR, 6,7,40,53, START_TABLE_INDEX, 0,0, FALSE,
    /*86*/                              NULL, (int)NULL, (gpointer)NULL },

    { "Host System's Initial Flow Control Credits", LABEL, 7,12,39,41, START_TABLE_INDEX, 0,0, TRUE,
    /*87*/                              NULL, (int)NULL, "blue" },
    { "Posted Header", LABEL, 7,10,42,44, START_TABLE_INDEX, 0,0, TRUE,
    /*88*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 10,11,42,44, START_TABLE_INDEX, 10,0, FALSE,
    /*89*/                              NULL, (int)NULL, NULL },
    { "Non-Posted Header", LABEL, 7,10,45,47, START_TABLE_INDEX, 0,0, TRUE,
    /*90*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 10,11,45,47, START_TABLE_INDEX, 10,0, FALSE,
    /*91*/                              NULL, (int)NULL, NULL },
    { "Completion Header", LABEL, 7,10,48,50, START_TABLE_INDEX, 0,0, TRUE,
    /*92*/                             NULL, (int)NULL, "black" },
    { "", ENTRY, 10,11,48,50, START_TABLE_INDEX, 10,0, FALSE,
    /*93*/                             NULL, (int)NULL, NULL },
    { "Posted Data", LABEL, 11,13,42,44, START_TABLE_INDEX, 0,0, TRUE,
    /*94*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 13,14,42,44, START_TABLE_INDEX, 10,0, FALSE,
    /*95*/                              NULL, (int)NULL, NULL },
    { "Non-Posted Data", LABEL, 11,13,45,47, START_TABLE_INDEX, 0,0, TRUE,
    /*96*/                              NULL, (int)NULL, "black" },
    { "", ENTRY, 13,14,45,47, START_TABLE_INDEX, 10,0, FALSE,
    /*97*/                              NULL, (int)NULL, NULL },
    { "Completion Data", LABEL, 11,13,48,50, START_TABLE_INDEX, 0,0, TRUE,
    /*98*/                             NULL, (int)NULL, "black" },
    { "", ENTRY, 13,14,48,50, START_TABLE_INDEX, 10,0, FALSE,
    /*99*/                             NULL, (int)NULL, NULL },

    { "PCIe Statistics", FRAME, 20,0,0,0, NOTEBOOK_INDEX, 720,430, TRUE,
    /*100*/                             NULL, (int)NULL, (gpointer)-1 },
    { "table in Graph", TABLE, 0,0,0,0, TRN_FRAME_INDEX, 1,2, TRUE,
    /*101*/                             NULL, (int)NULL, NULL },
    { "PCIe Transmit (writes) Statistics", DRAWING_AREA, 0,1,0,1, TRN_TABLE_INDEX,
    /*102*/           400,400, TRUE, DrawTRNGraph1, EXPOSE_EVENT, (gpointer)1 },
    { "PCIe Receive (reads) Statistics", DRAWING_AREA, 1,2,0,1, TRN_TABLE_INDEX,
    /*103*/           400,400, TRUE, DrawTRNGraph2, EXPOSE_EVENT, (gpointer)2 }
};

/* Array of pointers to hold Widgets of Main screen */
void* GuiWidgets[ sizeof(GuiScreen) / sizeof(GuiScreen[0])];

/* Array of pointers to hold Widgets of status screen */
void* StatusWidgets[ sizeof(StatusScreen) / sizeof(StatusScreen[0])];

/* Function to return maximum value of an array */
//float array_max (float * array, int iteration)
int array_max (float * array, int iteration)
{
    int count;
    float max=0;

    //max = array[0];
    for(count = 0; count < iteration; count++)
    {
        if (array[count] > max)
            max = array[count];
    }
    return max;
}

/* function to redirect user information to Text view */
void CreateText( const gchar *String )
{
    GtkTextIter Iter,Start,End;
    GtkTextBuffer *Buffer;
    GtkTextTag *Tag;
    /* only display the debug message to the background console */
    if (display==MSG_DEBUG)
    {
        printf ("[DEBUG] %s", String);
        return;
    }

    TextString=g_string_new(NULL);
    Buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (GuiWidgets[TEXTVIEW_INDEX]));

    if(display==MSG_WARNING)
        {
        Tag=gtk_text_buffer_create_tag (Buffer, NULL,"foreground",
                "blue", NULL);
        g_string_append( TextString, "[WARNING] " );}
    else if(display==MSG_ERROR)
        { Tag=gtk_text_buffer_create_tag (Buffer, NULL,"foreground",
            "red", NULL);
            g_string_append( TextString, "[ERROR] " );}
    else if(display==MSG_DEBUG)
        { Tag=gtk_text_buffer_create_tag (Buffer, NULL,"foreground",
            "green", NULL);
            g_string_append( TextString, "[DEBUG] " );}
    else /* if(display==MSG_INFO) or any other value */
        { Tag=gtk_text_buffer_create_tag (Buffer, NULL,"foreground",
            "black", NULL);
        g_string_append( TextString, "[INFO] " );}

    g_string_append( TextString, String );

    gtk_text_buffer_get_end_iter(Buffer,&Iter);
    gtk_text_buffer_get_end_iter(Buffer,&Start);
    BeginPos = gtk_text_buffer_create_mark (Buffer, "begin_pos", &Iter, TRUE);
    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(GuiWidgets[TEXTVIEW_INDEX]), BeginPos);

    gtk_text_buffer_insert (Buffer,&Iter, TextString->str, -1);
    gtk_text_buffer_get_end_iter(Buffer,&End);
    gtk_text_buffer_get_iter_at_mark( Buffer,    &Start,    BeginPos );

    gtk_text_buffer_apply_tag (Buffer,Tag , &Start, &End);

    gtk_widget_show_all (GuiWidgets[SCROLLED_WINDOW_INDEX]);
}





/* This callback quits the program */
gboolean quit_window( GtkWidget *Widget, gpointer Data )

{
    /* Killing GUI need not result in stopping hardware - for now */
    gtk_main_quit ();
    return FALSE;
}

static void draw_axis_label(GtkWidget *widget, int x, int y, int gap, char *label)
{
    int i;
    char * str = "Gbps";
    char ** yline;
    PangoLayout * layout;
    PangoFontDescription * desc;

    layout = gtk_widget_create_pango_layout(widget, str);
    desc = pango_font_description_from_string ("Bold");
    pango_layout_set_font_description (layout, desc);
    pango_font_description_free (desc);

    gdk_draw_layout(widget->window,
         widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
         0, y-15, layout);
/*
    gdk_draw_layout(widget->window,
         widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
         0, y-15, gtk_widget_create_pango_layout(widget, str));
*/

    yline = Yaxis;

    for(i=0; i<max_y_count; i++)
    {
        gdk_draw_layout(widget->window,
             widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
             0, (i*gap)+y, gtk_widget_create_pango_layout(widget, yline[i]));
    }

    gdk_draw_layout(widget->window,
         widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
         x+100, (max_y_count*gap)+y+10, gtk_widget_create_pango_layout(widget, label));
}

int get_trnstats(void)
{
    int j, retval=0;
    TRNStatsArray tsa;
    static TRNStatistics ts[8];
    double dd;

    tsa.Count = 8;
    tsa.trnptr = ts;
#ifdef HARDWARE
    if(ioctl(statsfd, IGET_TRN_STATISTICS, &tsa) != 0)
    {
        perror("IGET_TRN_STATISTICS failed\n");
        return FALSE;
    }
#endif

    for(j=0; j<tsa.Count; j++)
    {
        dd = ((double)(ts[j].LTX) * MULTIPLIER) / DIVISOR;
        TRNarray[0][tsWrite[0]] = dd;
        TRNStats.LTX = ((double)(ts[j].LTX) * MULTIPLIER);
        dd = ((double)(ts[j].LRX) * MULTIPLIER) / DIVISOR;
        TRNarray[1][tsWrite[1]] = dd;
        TRNStats.LRX = ((double)(ts[j].LRX) * MULTIPLIER);

        //printf("...InTX %2.3f..Rx%2.3f\n",TRNStats.LTX,TRNStats.LRX);
        tsWrite[0] += 1;
        if(tsWrite[0] >= MAX_STATS) tsWrite[0] = 0;
        tsWrite[1] = tsWrite[0];

        if(tsNum[0] < MAX_STATS)
            tsNum[0] += 1;
        else
        {
            tsRead[0] += 1;
            if(tsRead[0] >= MAX_STATS) tsRead[0] = 0;
        }
        tsNum[1] = tsNum[0];
        tsRead[1] = tsRead[0];
    }

    return TRUE;
}

static DMAStatistics ds[MAX_STATS];
int get_dmastats(void)
{
    int j, retval=0;
    unsigned int at, wt, cb;
    int len, done;
    EngStatsArray es;
    double dd;

    es.Count = MAX_STATS;
    es.engptr = ds;
#ifdef HARDWARE
    if(ioctl(statsfd, IGET_DMA_STATISTICS, &es) != 0)
    {
        printf("IGET_DMA_STATISTICS on engines failed\n");
        return FALSE;
    }
#endif

    for(j=0; j<es.Count; j++)
    {
        int k, eng;

        /* Driver engine number does not directly map to that of GUI */
        for(k=0; k<MAX_ENGS; k++)
        {
            if(DMAConfig[k].Engine == ds[j].Engine)
                break;
        }

        if(k >= MAX_ENGS) continue;
        eng = k;

        dd = ((double)(ds[j].LBR) * MULTIPLIER ) / DIVISOR;
        DMAarray[eng][dsWrite[eng]] = dd;
        EngStats[eng].LBR = ((double)(ds[j].LBR) * MULTIPLIER);
        EngStats[eng].LAT = ds[j].LAT;
        EngStats[eng].LWT = ds[j].LWT;

        dsWrite[eng] += 1;
        if(dsWrite[eng] >= MAX_STATS) dsWrite[eng] = 0;

        if(dsNum[eng] < MAX_STATS)
            dsNum[eng] += 1;
        else
        {
            dsRead[eng] += 1;
            if(dsRead[eng] >= MAX_STATS) dsRead[eng] = 0;
        }
    }

    return TRUE;
}

int get_swstats(void)
{
    int j, retval=0;
    int len, done;
    SWStatsArray ssa;
    static SWStatistics ss[32];
    double dd;

    ssa.Count = 32;
    ssa.swptr = ss;
#ifdef HARDWARE
    if(ioctl(statsfd, IGET_SW_STATISTICS, &ssa) != 0)
    {
        printf("IGET_SW_STATISTICS failed\n");
        return FALSE;
    }
#endif

    for(j=0; j<ssa.Count; j++)
    {
        int k, eng;

        /* Driver engine number does not directly map to that of GUI */
        for(k=0; k<MAX_ENGS; k++)
        {
            if(DMAConfig[k].Engine == ss[j].Engine)
                break;
        }

        if(k >= MAX_ENGS) continue;
        eng = k;

        dd = ((double)(ss[j].LBR) * MULTIPLIER) / DIVISOR;
        SWarray[eng][ssWrite[eng]] = dd;

        ssWrite[eng] += 1;
        if(ssWrite[eng] >= MAX_STATS) ssWrite[eng] = 0;

        if(ssNum[eng] < MAX_STATS)
            ssNum[eng] += 1;
        else
        {
            ssRead[eng] += 1;
            if(ssRead[eng] >= MAX_STATS) ssRead[eng] = 0;
        }
    }

    return TRUE;
}

int get_engstate(void)
{
    int i;
    EngState EngInfo;
    int state;

    /* Get the state of all the engines */
    for(i=0; i<MAX_ENGS; i++)
    {
        EngInfo.Engine = DMAConfig[i].Engine;
#ifdef HARDWARE
        if(ioctl(statsfd, IGET_ENG_STATE, &EngInfo) != 0)
        {
            printf("IGET_ENG_STATE on Engine %d failed\n", EngInfo.Engine);
            memset(&(EngStats[i]), 0, (sizeof(EngStats) / (sizeof(EngStats[0]))));
        }
        else
#endif
        {
            unsigned int testmode;

            EngStats[i].BDs = EngInfo.BDs;
            EngStats[i].Bufs = EngInfo.Buffers;
            EngStats[i].BDe = EngInfo.BDerrs;
            EngStats[i].BDSe = EngInfo.BDSerrs;
            EngStats[i].MinPkt = EngInfo.MinPktSize;
            EngStats[i].MaxPkt = EngInfo.MaxPktSize;
            EngStats[i].IntEnab = EngInfo.IntEnab;

            testmode = EngInfo.TestMode;
            EngStats[i].TestMode = testmode;

            state = (testmode & (TEST_START|TEST_IN_PROGRESS)) ? 1 : -1;
            EngStats[i].TXEnab = state;
            state = (testmode & ENABLE_LOOPBACK)? 1 : -1;
            EngStats[i].LBEnab = state;
            state = (testmode & ENABLE_PKTGEN)? 1 : -1;
            EngStats[i].PktGenEnab = state;
            state = (testmode & ENABLE_PKTCHK)? 1 : -1;
            EngStats[i].PktChkEnab = state;
        }
    }
}

int get_PCIstate(void)
{
#ifdef HARDWARE
    if(ioctl(statsfd, IGET_PCI_STATE, &PCIInfo) != 0)
    {
        printf("IGET_PCI_STATE failed\n");
        return -1;
    }
#endif
}
#if 0
void PutTestButtons1(void)
{
    int state;

    printf("PutTestButtons1: TestStartFlag1 %x\n", TestStartFlag1);

    /* Display the current values of the test-related buttons. This should
     * be called only when loading the GUI for the first time.
     */

    /* Display the GT Loopback setting on XAUI path */
    state = (TestStartFlag1 & ENABLE_LOOPBACK) ? TRUE : FALSE;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                        (GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX]), state);
}
#endif

void PutTestButtons1(void)
{
    int state;

    printf("PutTestButtons1: TestStartFlag1 %x\n", TestStartFlag1);

    /* Display the current values of the test-related buttons. This should
 *      * be called only when loading the GUI for the first time, or when
 *           * about to start/stop the test.
 *                */

    /* Display the Loopback / Checker setting on Raw Data path */
    if(TestStartFlag1 & (ENABLE_PKTCHK|ENABLE_PKTGEN))
    {
        gtk_widget_set_sensitive (GuiWidgets[ENABLE_LOOPBACK_LABEL1_INDEX], FALSE);
        gtk_widget_set_sensitive (GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX], FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                    (GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX]), FALSE);
        gtk_widget_set_sensitive (GuiWidgets[TXCHECKER_LABEL1_INDEX], TRUE);
        gtk_widget_set_sensitive (GuiWidgets[TXCHECKER_BUTTON1_INDEX], TRUE);
        gtk_widget_set_sensitive (GuiWidgets[RXGENERATOR_LABEL1_INDEX], TRUE);
        gtk_widget_set_sensitive (GuiWidgets[RXGENERATOR_BUTTON1_INDEX], TRUE);
        if(TestStartFlag1 & ENABLE_PKTCHK)
        {
            /* Checker mode - disable loopback */
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                        (GuiWidgets[TXCHECKER_BUTTON1_INDEX]), TRUE);
        }
        if(TestStartFlag1 & ENABLE_PKTGEN)
        {
            /* Generator mode - disable loopback */
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                        (GuiWidgets[RXGENERATOR_BUTTON1_INDEX]), TRUE);
        }
    }
    else
    {
        gtk_widget_set_sensitive (GuiWidgets[ENABLE_LOOPBACK_LABEL1_INDEX], TRUE);
        gtk_widget_set_sensitive (GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX], TRUE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                    (GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX]), TRUE);

        /* Loopback mode - disable checker & generator */
        gtk_widget_set_sensitive (GuiWidgets[TXCHECKER_LABEL1_INDEX], FALSE);
        gtk_widget_set_sensitive (GuiWidgets[TXCHECKER_BUTTON1_INDEX], FALSE);
        gtk_widget_set_sensitive (GuiWidgets[RXGENERATOR_LABEL1_INDEX], FALSE);
        gtk_widget_set_sensitive (GuiWidgets[RXGENERATOR_BUTTON1_INDEX], FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                    (GuiWidgets[TXCHECKER_BUTTON1_INDEX]), FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                    (GuiWidgets[RXGENERATOR_BUTTON1_INDEX]), FALSE);
    }
}


void PutTestButtons2(void)
{
    int state;

    printf("PutTestButtons2: TestStartFlag2 %x\n", TestStartFlag2);

    /* Display the current values of the test-related buttons. This should
     * be called only when loading the GUI for the first time, or when
     * about to start/stop the test.
     */

    /* Display the Loopback / Checker setting on Raw Data path */
    if(TestStartFlag2 & (ENABLE_PKTCHK|ENABLE_PKTGEN))
    {
        gtk_widget_set_sensitive (GuiWidgets[LOOPBACK_LABEL2_INDEX], FALSE);
        gtk_widget_set_sensitive (GuiWidgets[LOOPBACK_BUTTON2_INDEX], FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                    (GuiWidgets[LOOPBACK_BUTTON2_INDEX]), FALSE);
        gtk_widget_set_sensitive (GuiWidgets[TXCHECKER_LABEL2_INDEX], TRUE);
        gtk_widget_set_sensitive (GuiWidgets[TXCHECKER_BUTTON2_INDEX], TRUE);
        gtk_widget_set_sensitive (GuiWidgets[RXGENERATOR_LABEL2_INDEX], TRUE);
        gtk_widget_set_sensitive (GuiWidgets[RXGENERATOR_BUTTON2_INDEX], TRUE);
        if(TestStartFlag2 & ENABLE_PKTCHK)
        {
            /* Checker mode - disable loopback */
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                        (GuiWidgets[TXCHECKER_BUTTON2_INDEX]), TRUE);
        }
        if(TestStartFlag2 & ENABLE_PKTGEN)
        {
            /* Generator mode - disable loopback */
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                        (GuiWidgets[RXGENERATOR_BUTTON2_INDEX]), TRUE);
        }
    }
    else
    {
        gtk_widget_set_sensitive (GuiWidgets[LOOPBACK_LABEL2_INDEX], TRUE);
        gtk_widget_set_sensitive (GuiWidgets[LOOPBACK_BUTTON2_INDEX], TRUE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                    (GuiWidgets[LOOPBACK_BUTTON2_INDEX]), TRUE);

        /* Loopback mode - disable checker & generator */
        gtk_widget_set_sensitive (GuiWidgets[TXCHECKER_LABEL2_INDEX], FALSE);
        gtk_widget_set_sensitive (GuiWidgets[TXCHECKER_BUTTON2_INDEX], FALSE);
        gtk_widget_set_sensitive (GuiWidgets[RXGENERATOR_LABEL2_INDEX], FALSE);
        gtk_widget_set_sensitive (GuiWidgets[RXGENERATOR_BUTTON2_INDEX], FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                    (GuiWidgets[TXCHECKER_BUTTON2_INDEX]), FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                    (GuiWidgets[RXGENERATOR_BUTTON2_INDEX]), FALSE);
    }
}

void PutTestSettings()
{
    /* Set/Clear the check-buttons for both paths. This function is also
     * called when the GUI is first loaded, in order to show the current
     * status at that time.
     */

    if(TestStartFlag1 & TEST_START)
        gtk_button_set_label(GTK_BUTTON(GTK_WIDGET(
                            GuiWidgets[START_BUTTON1_INDEX])), STOP_TEST);
    else
        gtk_button_set_label(GTK_BUTTON(GTK_WIDGET
                                (GuiWidgets[START_BUTTON1_INDEX])), START_TEST);

    if(TestStartFlag2 & TEST_START)
        gtk_button_set_label(GTK_BUTTON(GTK_WIDGET(
                            GuiWidgets[START_BUTTON2_INDEX])), STOP_TEST);
    else
        gtk_button_set_label(GTK_BUTTON(GTK_WIDGET
                                (GuiWidgets[START_BUTTON2_INDEX])), START_TEST);
}

/* Timeout callback function that periodically reads driver/engine status */
static int TimeoutCallback()
{
    int i, index;
    static int counter = 0;
    char str[16];
    float f1;
    static int firsttime = 1;

    /* Display a log message regarding the Hardware version information */
    /* Printing it here because of strange errors seen in main() */
    if(firsttime)
    {
        unsigned int ver, major, minor;
        firsttime = 0;

        ver = PCIInfo.Version;
        printf("version is %x\n", ver);
        major = (ver & 0x00000F00) >> 8;
        minor = (ver & 0x000000F0) >> 4;
        if((ver & 0x0000000F) == 1)
            msg_info("Kintex-7 Base TRD v%d.%d\n", major, minor);
        else
            msg_info("Kintex-7 Base TRD v%d.%d\n", major, minor);
       // if((major <= 1)&&(minor < 2))
      //      msg_warning("Update to latest version of Hardware\n");
    }

    counter++;
    if(!(counter % 5))      /* Check every 5 seconds */
    {
        get_engstate();
        get_PCIstate();
    }

    get_dmastats();
    //get_swstats();
    get_trnstats();

    /* Update screen elements */
    index = LBR1_ENTRY_INDEX;
    for(i=0; i<MAX_ENGS; i++)
    {
        f1 = EngStats[i].LBR ;
        f1 *= (8/MULTIPLIER); f1 /= SDIVISOR;
        sprintf(str,"%2.3f", f1);
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[index]), str);
        index++;

        sprintf(str,"%lu",EngStats[i].LAT);
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[index]), str);
        index++;

        sprintf(str,"%lu",EngStats[i].LWT);
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[index]), str);
        index++;

        sprintf(str,"%lu",EngStats[i].BDe);
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[index]), str);
        index++;

        if(i==0 || i==2)
           sprintf(str,"%lu",EngStats[i].BDSe);
        else
           sprintf(str,"n/a",EngStats[i].BDSe);
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[index]), str);
        index++;

        sprintf(str,"%lu",EngStats[i].BDs);
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[index]), str);
        index++;

        sprintf(str,"%lu",EngStats[i].Bufs);
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[index]), str);
        index++;

        if(i==0) index = LBR2_ENTRY_INDEX;
        else if(i==1) index = LBR3_ENTRY_INDEX;
        else if(i==2) index = LBR4_ENTRY_INDEX;
    }

    PutTestSettings();

    if(EngStats[0].IntEnab == 1)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                (StatusWidgets[INTENAB1_ENTRY_INDEX]), TRUE);
    else
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                (StatusWidgets[INTENAB1_ENTRY_INDEX]), FALSE);
#if 0
    if(EngStats[0].LBEnab == 1)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                (StatusWidgets[LBENAB1_ENTRY_INDEX]), TRUE);
    else
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                (StatusWidgets[LBENAB1_ENTRY_INDEX]), FALSE);
#endif

    if(EngStats[2].IntEnab == 1)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                (StatusWidgets[INTENAB3_ENTRY_INDEX]), TRUE);
    else
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                (StatusWidgets[INTENAB3_ENTRY_INDEX]), FALSE);

    f1 = TRNStats.LTX;
    f1 *= (8/MULTIPLIER); f1 /= SDIVISOR;
    sprintf(str,"%2.3f", f1);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[TRN_TX_ENTRY_INDEX]), str);
    f1 = TRNStats.LRX;
    f1 *= (8/MULTIPLIER); f1 /= SDIVISOR;
    sprintf(str,"%2.3f", f1);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[TRN_RX_ENTRY_INDEX]), str);

    if(PCIInfo.LinkState == LINK_UP)
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_LINK_STATUS_ENTRY_INDEX]), "Up");
    else
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_LINK_STATUS_ENTRY_INDEX]), "Down");

    if(PCIInfo.LinkSpeed == LINK_SPEED_5)
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_LINK_SPEED_ENTRY_INDEX]), "5 Gbps");
    else if(PCIInfo.LinkSpeed == LINK_SPEED_25)
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_LINK_SPEED_ENTRY_INDEX]), "2.5 Gbps");
    else
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_LINK_SPEED_ENTRY_INDEX]), "Unknown");

    sprintf(str,"x%d", PCIInfo.LinkWidth);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_LINK_WIDTH_ENTRY_INDEX]), str);

    sprintf(str, "0x%x", PCIInfo.VendorId);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_VENDOR_ENTRY_INDEX]), str);
    sprintf(str, "0x%x", PCIInfo.DeviceId);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_DEVICE_ENTRY_INDEX]), str);

    if(PCIInfo.IntMode == INT_MSIX)
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_INTMODE_ENTRY_INDEX]), "MSI-X");
    else if(PCIInfo.IntMode == INT_MSI)
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_INTMODE_ENTRY_INDEX]), "MSI");
    else if(PCIInfo.IntMode == INT_LEGACY)
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_INTMODE_ENTRY_INDEX]), "Legacy");
    else if(PCIInfo.IntMode == INT_NONE)
        gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_INTMODE_ENTRY_INDEX]), "None");

    sprintf(str, "%d", PCIInfo.MPS);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_MPS_ENTRY_INDEX]), str);

    sprintf(str, "%d", PCIInfo.MRRS);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_MRRS_ENTRY_INDEX]), str);

    sprintf(str, "%d", PCIInfo.InitFCCplD);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_FC_CPLD_ENTRY_INDEX]), str);
    sprintf(str, "%d", PCIInfo.InitFCCplH);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_FC_CPLH_ENTRY_INDEX]), str);
    sprintf(str, "%d", PCIInfo.InitFCNPD);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_FC_NPD_ENTRY_INDEX]), str);
    sprintf(str, "%d", PCIInfo.InitFCNPH);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_FC_NPH_ENTRY_INDEX]), str);
    sprintf(str, "%d", PCIInfo.InitFCPD);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_FC_PD_ENTRY_INDEX]), str);
    sprintf(str, "%d", PCIInfo.InitFCPH);
    gtk_entry_set_text(GTK_ENTRY(StatusWidgets[PCIE_FC_PH_ENTRY_INDEX]), str);

    /* function to queue redraw of Drawing widgets */
    gtk_widget_queue_draw(StatusWidgets[PERF_DRAWING1_INDEX]);
    gtk_widget_queue_draw(StatusWidgets[PERF_DRAWING2_INDEX]);
    gtk_widget_queue_draw(StatusWidgets[PERF_DRAWING3_INDEX]);
    gtk_widget_queue_draw(StatusWidgets[PERF_DRAWING4_INDEX]);
    gtk_widget_queue_draw(StatusWidgets[TRN_DRAWING1_INDEX]);
    gtk_widget_queue_draw(StatusWidgets[TRN_DRAWING2_INDEX]);

    return TRUE;
}

/* Macro for checking minimum packet size */
#define CheckMinPktSize(size)   \
{                               \
    if(size < MIN_PKTSIZE)      \
    {                           \
        msg_warning("Minimum allowed packet size is %d bytes\n", MIN_PKTSIZE); \
        size = MIN_PKTSIZE;     \
    }                           \
}

/* Macro for checking maximum packet size */
#define CheckMaxPktSize(size)   \
{                               \
    if(size > MAX_PKTSIZE)      \
    {                           \
        msg_warning("Maximum allowed packet size is %d bytes\n", MAX_PKTSIZE); \
        size = MAX_PKTSIZE;     \
    }                           \
}

gboolean GetTestSettings(TestCmd * cmd, int engine)
{
    cmd->TestMode = 0;
    cmd->MinPktSize = MIN_PKTSIZE;
    cmd->MaxPktSize = MAX_PKTSIZE;

    if(engine == 0)
    {
        cmd->Engine = DMAConfig[0].Engine;

        if (gtk_toggle_button_get_active (GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX]))
            /* Set bit for Loopback on first path */
            cmd->TestMode |= ENABLE_LOOPBACK;

        if (gtk_toggle_button_get_active (GuiWidgets[TXCHECKER_BUTTON1_INDEX]))
            /* Set bit for Pkt Checker on second path */
            cmd->TestMode |= ENABLE_PKTCHK;
        if (gtk_toggle_button_get_active (GuiWidgets[RXGENERATOR_BUTTON1_INDEX]))
            /* Set bit for Pkt Generator on second path */
            cmd->TestMode |= ENABLE_PKTGEN;

        cmd->MinPktSize = cmd->MaxPktSize = strtoul(gtk_entry_get_text(
                              GuiWidgets[PKT_ENTRY1_INDEX]), NULL, 10);
    //    cmd->MaxPktSize = strtoul(gtk_entry_get_text(
    //                        GuiWidgets[MAX_PKT_ENTRY1_INDEX]), NULL, 10);
    }
    else
    {
        cmd->Engine = DMAConfig[2].Engine;

        if (gtk_toggle_button_get_active (GuiWidgets[LOOPBACK_BUTTON2_INDEX]))
            /* Set bit for Loopback on second path */
            cmd->TestMode |= ENABLE_LOOPBACK;
        if (gtk_toggle_button_get_active (GuiWidgets[TXCHECKER_BUTTON2_INDEX]))
            /* Set bit for Pkt Checker on second path */
            cmd->TestMode |= ENABLE_PKTCHK;
        if (gtk_toggle_button_get_active (GuiWidgets[RXGENERATOR_BUTTON2_INDEX]))
            /* Set bit for Pkt Generator on second path */
            cmd->TestMode |= ENABLE_PKTGEN;

        cmd->MinPktSize = cmd->MaxPktSize = strtoul(gtk_entry_get_text(
                              GuiWidgets[PKT_ENTRY2_INDEX]), NULL, 10);
        printf("PktSize is %d\n", cmd->MinPktSize);
    }

    /* Note that the packet size bounds must be checked on both the minimum
     * and maximum sizes.
     */
    CheckMinPktSize(cmd->MinPktSize);
    CheckMaxPktSize(cmd->MinPktSize);
    CheckMinPktSize(cmd->MaxPktSize);
    CheckMaxPktSize(cmd->MaxPktSize);

    return TRUE;
}

/* Callback for Graphs page */
gboolean StatusCallback(GtkWidget *Widget, gpointer String)
{
    int i;
    int max;

    gtk_notebook_remove_page ((GtkNotebook *)GuiWidgets[NOTEBOOK_INDEX], 2);
    gtk_notebook_remove_page ((GtkNotebook *)GuiWidgets[NOTEBOOK_INDEX], 1);
    gtk_notebook_remove_page ((GtkNotebook *)GuiWidgets[NOTEBOOK_INDEX], 0);

    max = sizeof(StatusScreen) / sizeof(StatusScreen[0]) ;

    /* Display screens */
    for(i=0; i<max; i++)
    {
        struct screen * scr;

        scr = &(StatusScreen[i]);

        DrawWidget(StatusScreen, StatusWidgets, i);
        if(scr->Callback != NULL)
            g_signal_connect(StatusWidgets[i], Signal[scr->SignalIndex],
                (GtkSignalFunc)scr->Callback, GINT_TO_POINTER(scr->Data));
    }
    gtk_widget_show_all(GuiWidgets[0]);
    gtk_widget_show_all(GuiWidgets[0]);


    /* Initialize the timer to poll hardware status */
    PerfGraphID=g_timeout_add(GRAPH_TIMEOUT,
                    (GtkFunction) TimeoutCallback, NULL);
    return TRUE;
}

/* Stop the running test */
gboolean StopTest(int engine)
{
    int retval=FALSE;
    TestCmd testCmd;

    testCmd.Engine = engine;

    GetTestSettings(&testCmd, engine);
    testCmd.TestMode &= ~(TEST_START);

    printf("mode is %x\n", testCmd.TestMode);
#ifdef HARDWARE
    if(ioctl(statsfd, ISTOP_TEST, &testCmd) != 0)
    {
        msg_error("STOP_TEST on engine %d failed\n", engine);
        return FALSE;
    }
#endif
    retval = TRUE;

    /* Clear test flag */
    if(engine == 0)
    {
        TestStartFlag1 &= ~(TEST_START);
        printf("TestStartFlag1 is %x\n", TestStartFlag1);
    }
    else
    {
        TestStartFlag2 &= ~(TEST_START);
        printf("TestStartFlag2 is %x\n", TestStartFlag2);
    }

    msg_info("Test stopped\n");

    return retval;
}

gboolean StartTest(int engine)
{
    int retval=FALSE;
    TestCmd testCmd;

    GetTestSettings(&testCmd, engine);
    testCmd.TestMode |= TEST_START;

    printf("mode is %x\n", testCmd.TestMode);
#ifdef HARDWARE
    retval = ioctl(statsfd, ISTART_TEST, &testCmd);
    if(retval != 0)
    {
        msg_error("START_TEST on Eng %d failed\n", engine);

        return FALSE;
    }
#endif
    retval = TRUE;

    /* Set test flag */
    if(engine == 0)
    {
        TestStartFlag1 = testCmd.TestMode;
        printf("TestStartFlag1 is %x\n", TestStartFlag1);

    }
    else
    {
        TestStartFlag2 = testCmd.TestMode;
        printf("TestStartFlag2 is %x\n", TestStartFlag2);
    }

    msg_info("Test Started\n");
    return retval;
}




/* callback for check buttons for various test options */
gboolean LBCallback1(GtkWidget *Widget, gpointer String)
{
    int state;

    printf("Came to LBCallback1\n");

    /* If test is already running, don't do anything */
    if(TestStartFlag1 & (TEST_START|TEST_IN_PROGRESS)) return;

    /* Will come here only for second path */
    state = gtk_toggle_button_get_active
                (GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX]) ? FALSE : TRUE;

    gtk_widget_set_sensitive(GuiWidgets[RXGENERATOR_LABEL1_INDEX], state);
    gtk_widget_set_sensitive(GuiWidgets[RXGENERATOR_BUTTON1_INDEX], state);
    gtk_widget_set_sensitive(GuiWidgets[TXCHECKER_LABEL1_INDEX], state);
    gtk_widget_set_sensitive(GuiWidgets[TXCHECKER_BUTTON1_INDEX], state);

    /* Atleast one choice must be selected in order to start test */
    if(!(gtk_toggle_button_get_active(GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX])) &&
       !(gtk_toggle_button_get_active(GuiWidgets[RXGENERATOR_BUTTON1_INDEX])) &&
       !(gtk_toggle_button_get_active(GuiWidgets[TXCHECKER_BUTTON1_INDEX])))
        gtk_widget_set_sensitive(GuiWidgets[START_BUTTON1_INDEX], FALSE);
    else
        gtk_widget_set_sensitive(GuiWidgets[START_BUTTON1_INDEX], TRUE);
}

gboolean LBCallback2(GtkWidget *Widget, gpointer String)
{
    int state;

    printf("Came to LBCallback2\n");

    /* If test is already running, don't do anything */
    if(TestStartFlag2 & (TEST_START|TEST_IN_PROGRESS)) return;

    /* Will come here only for second path */
    state = gtk_toggle_button_get_active
                (GuiWidgets[LOOPBACK_BUTTON2_INDEX]) ? FALSE : TRUE;

    gtk_widget_set_sensitive(GuiWidgets[RXGENERATOR_LABEL2_INDEX], state);
    gtk_widget_set_sensitive(GuiWidgets[RXGENERATOR_BUTTON2_INDEX], state);
    gtk_widget_set_sensitive(GuiWidgets[TXCHECKER_LABEL2_INDEX], state);
    gtk_widget_set_sensitive(GuiWidgets[TXCHECKER_BUTTON2_INDEX], state);

    /* Atleast one choice must be selected in order to start test */
    if(!(gtk_toggle_button_get_active(GuiWidgets[LOOPBACK_BUTTON2_INDEX])) &&
       !(gtk_toggle_button_get_active(GuiWidgets[RXGENERATOR_BUTTON2_INDEX])) &&
       !(gtk_toggle_button_get_active(GuiWidgets[TXCHECKER_BUTTON2_INDEX])))
        gtk_widget_set_sensitive(GuiWidgets[START_BUTTON2_INDEX], FALSE);
    else
        gtk_widget_set_sensitive(GuiWidgets[START_BUTTON2_INDEX], TRUE);
}

/* callback for check buttons for various test options */
gboolean NonLBCallback1(GtkWidget *Widget, gpointer String)
{
    int state;

    /* If test is already running, don't do anything */
    if(TestStartFlag1 & (TEST_START|TEST_IN_PROGRESS)) return;

    /* Will come here only for second path */
    if(gtk_toggle_button_get_active(GuiWidgets[RXGENERATOR_BUTTON1_INDEX]) ||
       gtk_toggle_button_get_active(GuiWidgets[TXCHECKER_BUTTON1_INDEX]))
        state = FALSE;
    else
        state = TRUE;

    gtk_widget_set_sensitive(GuiWidgets[ENABLE_LOOPBACK_LABEL1_INDEX], state);
    gtk_widget_set_sensitive(GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX], state);

    /* Atleast one choice must be selected in order to start test */
    if(!(gtk_toggle_button_get_active(GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX])) &&
       !(gtk_toggle_button_get_active(GuiWidgets[RXGENERATOR_BUTTON1_INDEX])) &&
       !(gtk_toggle_button_get_active(GuiWidgets[TXCHECKER_BUTTON1_INDEX])))
        gtk_widget_set_sensitive(GuiWidgets[START_BUTTON1_INDEX], FALSE);
    else
        gtk_widget_set_sensitive(GuiWidgets[START_BUTTON1_INDEX], TRUE);
}
/* callback for check buttons for various test options */
gboolean NonLBCallback2(GtkWidget *Widget, gpointer String)
{
    int state;

    /* If test is already running, don't do anything */
    if(TestStartFlag2 & (TEST_START|TEST_IN_PROGRESS)) return;

    /* Will come here only for second path */
    if(gtk_toggle_button_get_active(GuiWidgets[RXGENERATOR_BUTTON2_INDEX]) ||
       gtk_toggle_button_get_active(GuiWidgets[TXCHECKER_BUTTON2_INDEX]))
        state = FALSE;
    else
        state = TRUE;

    gtk_widget_set_sensitive(GuiWidgets[LOOPBACK_LABEL2_INDEX], state);
    gtk_widget_set_sensitive(GuiWidgets[LOOPBACK_BUTTON2_INDEX], state);

    /* Atleast one choice must be selected in order to start test */
    if(!(gtk_toggle_button_get_active(GuiWidgets[LOOPBACK_BUTTON2_INDEX])) &&
       !(gtk_toggle_button_get_active(GuiWidgets[RXGENERATOR_BUTTON2_INDEX])) &&
       !(gtk_toggle_button_get_active(GuiWidgets[TXCHECKER_BUTTON2_INDEX])))
        gtk_widget_set_sensitive(GuiWidgets[START_BUTTON2_INDEX], FALSE);
    else
        gtk_widget_set_sensitive(GuiWidgets[START_BUTTON2_INDEX], TRUE);
}

/* callback for start button to start/stop test */
gboolean StartTestCallback(GtkWidget *Widget, gpointer String)
{
    int i=0, retval=FALSE;
    int engine, testing;
    GtkWidget * button = GTK_WIDGET(Widget);
    char str[16];
    TestCmd testCmd;

    /* First, check for path selection */
    if(Widget == GuiWidgets[START_BUTTON1_INDEX]) engine = 0;
    else if(Widget == GuiWidgets[START_BUTTON2_INDEX]) engine = 1;
    else engine = -1;

    if(engine == 0) testing = (TestStartFlag1 & TEST_START);
    else if(engine == 1) testing = (TestStartFlag2 & TEST_START);
    printf("TestDeviceCallback: TestStartFlag2 is %x\n", TestStartFlag2);

    if(engine != -1)
    {
        if(testing)
        {
            /* Test is to be stopped */
            retval = StopTest(engine);
            printf("StopTest returned %d\n", retval);
            gtk_button_set_label(GTK_BUTTON(button), START_TEST);

            if(engine == 0) PutTestButtons1();
            else if(engine == 1) PutTestButtons2();
        }
        else
        {
            /* Test is to be started */
            retval = StartTest(engine);
            printf("StartTest returned %d\n", retval);
            gtk_button_set_label(GTK_BUTTON(button), STOP_TEST);

            /* Now read the values back from the driver in order to update the
             * set packet sizes in the test.
             */
            get_engstate();

            /* Display the test settings */
            if(engine == 0)
            {
                sprintf(str,"%lu",EngStats[0].MaxPkt);
                gtk_entry_set_text(GTK_ENTRY(GuiWidgets[PKT_ENTRY1_INDEX]), str);

            /* Display the other settings */
                gtk_widget_set_sensitive
                                (GuiWidgets[ENABLE_LOOPBACK_LABEL1_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[ENABLE_LOOPBACK_BUTTON1_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[TXCHECKER_LABEL1_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[TXCHECKER_BUTTON1_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[RXGENERATOR_LABEL1_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[RXGENERATOR_BUTTON1_INDEX], FALSE);
            }
            else
            {
                sprintf(str,"%lu",EngStats[2].MaxPkt);
                gtk_entry_set_text(GTK_ENTRY(GuiWidgets[PKT_ENTRY2_INDEX]), str);

                /* Display the other settings */
                gtk_widget_set_sensitive
                                (GuiWidgets[LOOPBACK_LABEL2_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[LOOPBACK_BUTTON2_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[TXCHECKER_LABEL2_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[TXCHECKER_BUTTON2_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[RXGENERATOR_LABEL2_INDEX], FALSE);
                gtk_widget_set_sensitive
                                (GuiWidgets[RXGENERATOR_BUTTON2_INDEX], FALSE);
            }
        }
    }

    gtk_widget_show_all(GuiWidgets[0]);

    return TRUE;
}

void DrawWidget(struct screen *MyGuiScreen,void *PWidgets[],int i)
{
    GdkColor   colorRed;
    int Flag;
    static int GlistCount=0;
    GtkWidget**  MyGuiWidgets;
    MyGuiWidgets = (GtkWidget**)PWidgets;
    switch(MyGuiScreen[i].Type)
    {
        case WINDOW:
        MyGuiWidgets[i]= (gtk_window_new(GTK_WINDOW_TOPLEVEL));
        gtk_window_set_position(GTK_WINDOW(MyGuiWidgets[i]),GTK_WIN_POS_CENTER_ALWAYS);
        gtk_widget_set_usize (GTK_WIDGET(MyGuiWidgets[i]), MyGuiScreen[i].row, MyGuiScreen[i].col);
        gtk_window_set_title( GTK_WINDOW (MyGuiWidgets[i]),MyGuiScreen[i].Name);
        gtk_widget_show_all(MyGuiWidgets[i]);
        break;

        case VPANED:
        MyGuiWidgets[i]=gtk_vpaned_new();
        gtk_container_add(GTK_CONTAINER(MyGuiWidgets[MyGuiScreen[i].Parent]),GuiWidgets[i]);
        break;

        case TABLE:
        MyGuiWidgets[i]=gtk_table_new(MyGuiScreen[i].row,
            MyGuiScreen[i].col,MyGuiScreen[i].active);
        /* In a paned window the location is decided by "Left" field of
         * screen structure
         */
        if(MyGuiScreen[i].Parent ==VPANED_INDEX){
        if(MyGuiScreen[i].Left == UP)
        gtk_paned_add1 (GTK_PANED (MyGuiWidgets[MyGuiScreen[i].Parent]), MyGuiWidgets[i]);
        else
        gtk_paned_add2 (GTK_PANED (MyGuiWidgets[MyGuiScreen[i].Parent]), MyGuiWidgets[i]);
        }
        else{
        gtk_container_add(GTK_CONTAINER
        (MyGuiWidgets[MyGuiScreen[i].Parent]), MyGuiWidgets[i]);
        }
        gtk_widget_show(MyGuiWidgets[i]);
        break;

        case DRAWING_AREA:
        MyGuiWidgets[i] = gtk_drawing_area_new ();
        gtk_widget_set_size_request (MyGuiWidgets[i], MyGuiScreen[i].row, MyGuiScreen[i].col);
        //gtk_widget_set_size_request (MyGuiWidgets[i], -1, -1);
        gtk_table_attach_defaults
            (GTK_TABLE(MyGuiWidgets[MyGuiScreen[i].Parent]),
            MyGuiWidgets[i],
            MyGuiScreen[i].Left, MyGuiScreen[i].Right,
            MyGuiScreen[i].Top, MyGuiScreen[i].Bottom);
        break;

        case NOTEBOOK:
        MyGuiWidgets[i] = gtk_notebook_new ();
        gtk_notebook_set_tab_pos ((GtkNotebook *) (MyGuiWidgets[i]), GTK_POS_TOP);
        gtk_table_attach_defaults (GTK_TABLE
        (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],
        MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,
        MyGuiScreen[i].Bottom);
        break;

        case ALIGN:
        MyGuiWidgets[i]=gtk_alignment_new (1, 1, 0, 0);
        gtk_table_attach(GTK_TABLE
        (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],
        MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,
        MyGuiScreen[i].Bottom,FALSE,FALSE,0,0);
        break;

        case IMAGE:
        MyGuiWidgets[i] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_file (MyGuiScreen[i].Name, NULL)  );
        gtk_container_add(GTK_CONTAINER
        (MyGuiWidgets[MyGuiScreen[i].Parent]), MyGuiWidgets[i]);
        break;

        case BUTTON:
        MyGuiWidgets[i]=gtk_button_new_with_label (MyGuiScreen[i].Name);
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        gtk_widget_set_sensitive(MyGuiWidgets[i],MyGuiScreen[i].active) ;
        gtk_widget_show_all(MyGuiWidgets[i]);
        break;

        case CHECK_BUTTON:
        MyGuiWidgets[i]=gtk_check_button_new_with_label (MyGuiScreen[i].Name);
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (MyGuiWidgets[i]), MyGuiScreen[i].active);
        break;

        case RCHECK_BUTTON:
        MyGuiWidgets[i]=gtk_check_button_new();
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (MyGuiWidgets[i]), MyGuiScreen[i].active);
        break;

        case RADIO_BUTTON:
        {
        void * ptr;
        printf("Data is %s, row is %d\n", MyGuiScreen[i].Data, MyGuiScreen[i].row);
        if(!strcmp(MyGuiScreen[i].Data, "new"))
            ptr = NULL;
        else
            ptr = (void *)MyGuiWidgets[MyGuiScreen[i].row];
        MyGuiWidgets[i] = gtk_radio_button_new_with_label(ptr, MyGuiScreen[i].Name);
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        }
        break;

        case RADIO_GROUP:
        printf("Data is %s\n", MyGuiScreen[i].Data);
        MyGuiWidgets[i] = (GtkWidget *)gtk_radio_button_group(GTK_RADIO_BUTTON(MyGuiWidgets[MyGuiScreen[i].Parent]));
        break;

        case TOGGLE_BUTTON:
        MyGuiWidgets[i]=gtk_toggle_button_new_with_label(MyGuiScreen[i].Name);
        gtk_table_attach (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]),MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom, GTK_SHRINK|GTK_EXPAND, GTK_SHRINK|GTK_EXPAND, 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (MyGuiWidgets[i]), MyGuiScreen[i].active);
        break;

        case HSEPARATOR:
        MyGuiWidgets[i] = gtk_hseparator_new();
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        break;

        case VSEPARATOR:
        MyGuiWidgets[i] = gtk_vseparator_new();
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        break;

        case SCROLLED_WINDOW:
        MyGuiWidgets[i] = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy
        (GTK_SCROLLED_WINDOW(MyGuiWidgets[i]),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_add(
        GTK_CONTAINER(MyGuiWidgets[MyGuiScreen[i].Parent]),
        MyGuiWidgets[i]);
        break;

        case TEXTVIEW:
        MyGuiWidgets[i]= gtk_text_view_new();
        gtk_container_add(GTK_CONTAINER
        (MyGuiWidgets[MyGuiScreen[i].Parent]), MyGuiWidgets[i]);
        gtk_text_view_set_editable( GTK_TEXT_VIEW(MyGuiWidgets[i]), MyGuiScreen[i].active );
        break;

        case FRAME:
        Flag = (int)MyGuiScreen[i].Data;
        MyGuiWidgets[i]=gtk_frame_new (" ");
        /* "Left" field of frame is overloaded to hold border width */
        if(MyGuiScreen[i].Left!=0)
        gtk_container_set_border_width (GTK_CONTAINER (MyGuiWidgets[i]), MyGuiScreen[i].Left);

        /* if "Top" and "Bottom" fields of frame are non zero it means frame is to be attached to
        * a table else it has its own length and breadth
        */
        if(MyGuiScreen[i].Top || MyGuiScreen[i].Bottom)
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        else
        gtk_widget_set_usize(MyGuiWidgets[i],MyGuiScreen[i].row,
        MyGuiScreen[i].col);

        gtk_frame_set_shadow_type ((GtkFrame*) MyGuiWidgets[i], GTK_SHADOW_NONE);
        if(MyGuiScreen[i].Name !=NULL){
        GtkWidget *Label = gtk_label_new (MyGuiScreen[i].Name);
        /* Attaching Frame to Notebook
        * Argument of callback is overloaded to inform where to attach frame
        * If argument is "-1" it implies frame has to be attached to Notebook of Main page
        */

        if(Flag <0){
        gtk_notebook_append_page ((GtkNotebook *)
        (GuiWidgets[MyGuiScreen[i].Parent]), MyGuiWidgets[i], Label);
        }
        else{
        gtk_notebook_append_page ((GtkNotebook *)
        (MyGuiWidgets[MyGuiScreen[i].Parent]), MyGuiWidgets[i], Label);
        }
        }
        /* exceptionally handel attachment of frame of textview */
        if(i==TEXTVIEW_FRAME_INDEX)
        gtk_paned_add2 (GTK_PANED (MyGuiWidgets[MyGuiScreen[i].Parent]), MyGuiWidgets[i]);

        gtk_widget_show_all(MyGuiWidgets[i]);
        break;

        case GRAPH:
        MyGuiWidgets[i] = gtk_drawing_area_new ();
        gtk_widget_set_size_request (MyGuiWidgets[i], MyGuiScreen[i].row,
                MyGuiScreen[i].col);
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        gtk_widget_show(MyGuiWidgets[i]);
        break;

        case LABEL:
        MyGuiWidgets[i]=gtk_label_new(MyGuiScreen[i].Name);
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        if((MyGuiScreen[i].Data != NULL) && (MyGuiScreen[i].Data > 0))
        {
            gdk_color_parse (MyGuiScreen[i].Data, &colorRed);
            gtk_widget_modify_fg (MyGuiWidgets[i], GTK_STATE_NORMAL, &colorRed);
        }
        //if(MyGuiScreen[i].Left >= 10)
        //    gtk_misc_set_alignment(GTK_MISC(MyGuiWidgets[i]), 1.0f, 0.0f);
        gtk_widget_show(MyGuiWidgets[i]);
        break;

        case ENTRY:
        MyGuiWidgets[i]=gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(MyGuiWidgets[i]), MyGuiScreen[i].col);
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        gtk_entry_set_editable(GTK_ENTRY(MyGuiWidgets[i]), MyGuiScreen[i].active);
        gtk_widget_show(MyGuiWidgets[i]);
        break;

        case COMBO:
        MyGuiWidgets[i]=gtk_combo_new();
        gtk_table_attach_defaults (GTK_TABLE (MyGuiWidgets[MyGuiScreen[i].Parent]) ,MyGuiWidgets[i],MyGuiScreen[i].Left,MyGuiScreen[i].Right,MyGuiScreen[i].Top,MyGuiScreen[i].Bottom);
        break;

        case GLIST_START:
        GlistCount=0;
        //glist=NULL;
        MyGuiWidgets[i]=NULL;
        break;

        case GLIST:
        GlistCount++;
        //                glist = g_list_append(glist,MyGuiScreen[i].Name );
        MyGuiWidgets[i-GlistCount] =(GtkWidget*) g_list_append(
        (GList *)MyGuiWidgets[i-GlistCount],MyGuiScreen[i].Name );

        //MyGuiWidgets[i-GlistCount]=(GList*)g_list_append((GList*)MyGuiWidgets[i-GlistCount],MyGuiScreen[i].Name);
        break;
        case GLIST_END:
        gtk_combo_set_popdown_strings( GTK_COMBO
        (MyGuiWidgets[i-GlistCount-2]),
        (GList*) MyGuiWidgets[i-GlistCount-1]) ;
        //                gtk_combo_set_popdown_strings( GTK_COMBO(MyGuiWidgets[i-GlistCount-1]),(GList*) MyGuiWidgets[i-GlistCount]) ;
        break;
    }
}

char label[64];

gboolean DrawPayloadGraph1 (GtkWidget *Widget, gpointer Data)
{
    int minX = 35, maxX = 384;
    int minY = 15, maxY = 150;

    strcpy(label, LABEL1);
    strcat(label, " ");
    strcat(label, StatusScreen[PERF_DRAWING1_INDEX].Name);
    DrawGraph(Widget, minX, maxX, minY, maxY, label,
              DMAarray[0], dsRead[0], dsNum[0],
              NULL /*SWarray[0]*/, ssRead[0], ssNum[0]);

    return TRUE;
}

gboolean DrawPayloadGraph2 (GtkWidget *Widget, gpointer Data)
{
    int minX = 35, maxX = 384;
    int minY = 15, maxY = 150;

    strcpy(label, LABEL1);
    strcat(label, " ");
    strcat(label, StatusScreen[PERF_DRAWING2_INDEX].Name);
    DrawGraph(Widget, minX, maxX, minY, maxY, label,
              DMAarray[1], dsRead[1], dsNum[1],
              NULL /*SWarray[1]*/, ssRead[1], ssNum[1]);

    return TRUE;
}

gboolean DrawPayloadGraph3 (GtkWidget *Widget, gpointer Data)
{
    int minX = 35, maxX = 384;
    int minY = 15, maxY = 150;

    strcpy(label, LABEL2);
    strcat(label, " ");
    strcat(label, StatusScreen[PERF_DRAWING3_INDEX].Name);
    DrawGraph(Widget, minX, maxX, minY, maxY, label,
              DMAarray[2], dsRead[2], dsNum[2],
              NULL /*SWarray[2]*/, ssRead[2], ssNum[2]);

    return TRUE;
}

gboolean DrawPayloadGraph4 (GtkWidget *Widget, gpointer Data)
{
    int minX = 35, maxX = 384;
    int minY = 15, maxY = 150;

    strcpy(label, LABEL2);
    strcat(label, " ");
    strcat(label, StatusScreen[PERF_DRAWING4_INDEX].Name);
    DrawGraph(Widget, minX, maxX, minY, maxY, label,
              DMAarray[3], dsRead[3], dsNum[3],
              NULL /*SWarray[3]*/, ssRead[3], ssNum[3]);

    return TRUE;
}

gboolean DrawTRNGraph1 (GtkWidget *Widget, gpointer Data)
{
    int minX = 35, maxX = 384;
    int minY = 15, maxY = 339;

    DrawGraph(Widget, minX, maxX, minY, maxY,
                   StatusScreen[TRN_DRAWING1_INDEX].Name,
                   TRNarray[0], tsRead[0], tsNum[0],
                   NULL, 0, 0);

    return TRUE;
}

gboolean DrawTRNGraph2 (GtkWidget *Widget, gpointer Data)
{
    int minX = 35, maxX = 384;
    int minY = 15, maxY = 339;

    DrawGraph(Widget, minX, maxX, minY, maxY,
                   StatusScreen[TRN_DRAWING2_INDEX].Name,
                   TRNarray[1], tsRead[1], tsNum[1],
                   NULL, 0, 0);

    return TRUE;
}

gboolean DrawGraph (GtkWidget *Widget, int minX, int maxX, int minY, int maxY, char * label, int * Yarray1, int from1, int tot1, int * Yarray2, int from2, int tot2)
{
    int maxy;
    int i, i1, i2, j, gap;
    GdkGC *Gc;
    GdkColor mycolor;

    maxy = MAX_Y;

    /* Two y-axis arrays are passed, if two graphs are to be shown,
     * else one of them will be NULL.
     */

    /* Drawing x-axis */
    gdk_draw_line(Widget->window,
          Widget->style->fg_gc[GTK_WIDGET_STATE (Widget)],
          minX, maxY, maxX, maxY);

    /* Drawing y-axis */
    gdk_draw_line(Widget->window,
          Widget->style->fg_gc[GTK_WIDGET_STATE (Widget)],
          minX, maxY, minX, minY);

    gap = (maxY - minY) / max_y_count;
    draw_axis_label(Widget, minX, minY, gap, label);

    Gc=gdk_gc_new (Widget->window);
    mycolor.pixel = 0;
    mycolor.red = 0xffff;
    mycolor.green = 0;
    mycolor.blue = 0;
    gdk_gc_set_rgb_fg_color(Gc,&mycolor);
    gdk_gc_set_rgb_bg_color(Gc,&mycolor);

    gap = maxy / (maxY - minY);         // MBps or Mbps per y-point
    /* Not using gap because values are lost in the calculation */
    i1 = from1;
    for(j=0;j<MAX_STATS;j++)
    {
        if(tot1 < 2) break;
        i2 = i1+1;
        if(i2 >= MAX_STATS) i2 = 0;

        gdk_draw_line(Widget->window, Gc,
                 (minX + j),
                 (maxY - ((Yarray1[i1] * (maxY - minY))/maxy)),
                 (minX + (j+1)),
                 (maxY - ((Yarray1[i2] * (maxY - minY))/maxy)));
        i1++;
        if(i1 >= MAX_STATS) i1 = 0;
        tot1--;
    }

    if(Yarray2 == NULL)
        return TRUE;

    mycolor.pixel = 0;
    mycolor.red = 0;
    mycolor.green = 0;
    mycolor.blue = 0xffff;
    gdk_gc_set_rgb_fg_color(Gc,&mycolor);
    gdk_gc_set_rgb_bg_color(Gc,&mycolor);

    gap = maxy / (maxY - minY);         // MBps or Mbps per y-point
    /* Not using gap because values are lost in the calculation */
    i1 = from2;
    for(j=0;j<MAX_STATS;j++)
    {
        if(tot2 < 2) break;
        i2 = i1+1;
        if(i2 >= MAX_STATS) i2 = 0;

        gdk_draw_line(Widget->window, Gc,
                 (minX + j),
                 (maxY - ((Yarray2[i1] * (maxY - minY))/maxy)),
                 (minX + (j+1)),
                 (maxY - ((Yarray2[i2] * (maxY - minY))/maxy)));
        i1++;
        if(i1 >= MAX_STATS) i1 = 0;
        tot2--;
    }

    return TRUE;
}

int main( int   argc,char *argv[] )
{
    int i, k, Max;
    char str[16];
    char * str1;
    char * token;
    unsigned int testmode;
    int state;

    /* Set up the GUI's main label */
    strcpy(mainlabel, MAINLABEL);

    /* First find the absolute path of the executable. Required for
     * locating the logo files.
     */
    strcpy(xlnx_logo, argv[0]);
    str1 = strrchr(xlnx_logo, '/');
    if(str1 == NULL)                            // If no slash is found.
        str1 = xlnx_logo;
    else
        str1++;                                 // To include slash.
    *str1 = '\0';
    strcpy(dev_logo, xlnx_logo);
    strcat(xlnx_logo, XLNX_LOGO);
    strcat(dev_logo, DEV_LOGO);
    printf("Logos are: %p %s %p %s\n", xlnx_logo, xlnx_logo, dev_logo, dev_logo);

    /* Open driver device file */
#ifdef HARDWARE
    if((statsfd = open(FILENAME, O_RDONLY)) < 0)
    {
        printf("Failed to open statistics file %s\n", FILENAME);
        return FALSE;
    }
#endif
    get_engstate();
    get_PCIstate();

    /* Based on the hardware version number, the GUI's main label changes */
  //  if((PCIInfo.Version & 0x0000000F) == 1)
  //      strcat(mainlabel, " - AXI TRD");

    /* Incase a test is going on, then enable the buttons to be correct */
    testmode = EngStats[0].TestMode;
    if(testmode & TEST_IN_PROGRESS)
    {
        TestStartFlag1 |= TEST_START;
        if(testmode & ENABLE_PKTCHK) TestStartFlag1 |= ENABLE_PKTCHK;
        if(testmode & ENABLE_LOOPBACK) TestStartFlag1 |= ENABLE_LOOPBACK;
    }
    if(testmode & ENABLE_PKTGEN) TestStartFlag1 |= ENABLE_PKTGEN;
    printf("TestStartFlag1 is %x\n", TestStartFlag1);

    testmode = EngStats[2].TestMode;
    if(testmode & TEST_IN_PROGRESS)
    {
        TestStartFlag2 |= TEST_START;
        if(testmode & ENABLE_PKTCHK) TestStartFlag2 |= ENABLE_PKTCHK;
        if(testmode & ENABLE_LOOPBACK) TestStartFlag2 |= ENABLE_LOOPBACK;
    }
    if(testmode & ENABLE_PKTGEN) TestStartFlag2 |= ENABLE_PKTGEN;
    printf("TestStartFlag2 is %x\n", TestStartFlag2);

    /* Initialise arrays of graph readings. */
    for(k=0; k<MAX_ENGS; k++)
    {
        for(i=0; i<MAX_STATS; i++)
            DMAarray[k][i] = SWarray[k][i] = 0;
        dsRead[k] = dsWrite[k] = dsNum[k] = 0;
        ssRead[k] = ssWrite[k] = ssNum[k] = 0;
    }
    for(k=0; k<2; k++)
    {
        for(i=0; i<MAX_STATS; i++)
            TRNarray[k][i] = 0;
        tsRead[k] = tsWrite[k] = tsNum[k] = 0;
    }

    gtk_init(&argc, &argv);

    g_set_print_handler (CreateText);

    Max = sizeof(GuiScreen) / sizeof(GuiScreen[0]) ;
    for(i=0;i<Max;i++){
        DrawWidget(GuiScreen,GuiWidgets,i);
        if(GuiScreen[i].Callback !=NULL)
        g_signal_connect(GuiWidgets[i], Signal[GuiScreen[i].SignalIndex],
            (GtkSignalFunc)GuiScreen[i].Callback,GINT_TO_POINTER(GuiScreen[i].Data));
        //ConnectWidget(GuiScreen,GuiWidgets,i)
    }

    /* Display the test settings */
    sprintf(str,"%lu",EngStats[0].MaxPkt);
    gtk_entry_set_text(GTK_ENTRY(GuiWidgets[PKT_ENTRY1_INDEX]), str);
    sprintf(str,"%lu",EngStats[0].MaxPkt);
 //   gtk_entry_set_text(GTK_ENTRY(GuiWidgets[MAX_PKT_ENTRY1_INDEX]), str);
    sprintf(str,"%lu",EngStats[2].MaxPkt);
    gtk_entry_set_text(GTK_ENTRY(GuiWidgets[PKT_ENTRY2_INDEX]), str);

    PutTestSettings();
    PutTestButtons1();
    PutTestButtons2();


    gtk_widget_show_all(GuiWidgets[0]);
    gtk_main();

#ifdef HARDWARE
    close(statsfd);
#endif

    return TRUE;
}

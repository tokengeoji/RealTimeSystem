/*
 *  ======== main.c ========
 */

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/cfg/global.h>

#include <stdio.h>
#include "init.h"
#include "LED_DIPSW.h"


#include "image.h"

#define LINES_PER_DISP  (Y_SIZE*3)
#define PIXELS_PER_LINE (X_SIZE*3)
#define PIXELS_IN_IMAGE (LINES_PER_DISP*PIXELS_PER_LINE)

#pragma DATA_SECTION( image, ".image")
Uint8 image[PIXELS_IN_IMAGE];

CSL_Edma3ccRegsOvly edma3ccRegs = (CSL_Edma3ccRegsOvly)(CSL_EDMA30CC_0_REGS);

void ResetEDMA3( void ) {
    edma3ccRegs->TPCC_EMCR       = 0xFFFFFFFF;
    edma3ccRegs->TPCC_CCERRCLR   = 0xFFFFFFFF;
    edma3ccRegs->TPCC_SECR       = 0xFFFFFFFF;
    edma3ccRegs->TPCC_ECR        = 0xFFFFFFFF;
    edma3ccRegs->TPCC_ICR        = 0xFFFFFFFF;
    edma3ccRegs->TPCC_EECR       = 0xFFFFFFFF;
    edma3ccRegs->TPCC_IECR       = 0xFFFFFFFF;
}


/*
void SetupEDMA3( void )
{
    Uint32 edma_evt;

    edma_evt = EDMA_EVENT5;

    edma3ccRegs->PARAMSET[edma_evt].SRC = (Uint32)image;
    edma3ccRegs->PARAMSET[edma_evt].DST = (Uint32)image + PIXELS_IN_IMAGE/3 + PIXELS_PER_LINE/3;

    edma3ccRegs->PARAMSET[edma_evt].OPT =             CSL_TPCC_PARAM_OPT_RESETVAL
                                                    | CSL_FMKT(TPCC_PARAM_OPT_TCINTEN, ENABLE)
                                                    | CSL_FMK(TPCC_PARAM_OPT_TCC, edma_evt)
                                                    | CSL_FMKT(TPCC_PARAM_OPT_SYNCDIM, ABSYNC);

    edma3ccRegs->PARAMSET[edma_evt].A_B_CNT =         CSL_FMK(TPCC_PARAM_A_B_CNT_BCNT, Y_SIZE)
                                                    | CSL_FMK(TPCC_PARAM_A_B_CNT_ACNT, X_SIZE);
    edma3ccRegs->PARAMSET[edma_evt].SRC_DST_BIDX =    CSL_FMK(TPCC_PARAM_SRC_DST_BIDX_DSTBIDX, PIXELS_PER_LINE)
                                                    | CSL_FMK(TPCC_PARAM_SRC_DST_BIDX_SRCBIDX, PIXELS_PER_LINE);
    edma3ccRegs->PARAMSET[edma_evt].LINK_BCNTRLD =    CSL_FMK(TPCC_PARAM_LINK_BCNTRLD_LINK, 0xFFFF);
    edma3ccRegs->PARAMSET[edma_evt].SRC_DST_CIDX = 0;
    edma3ccRegs->PARAMSET[edma_evt].CCNT = 1;
}
*/

void SetupEDMA3( Uint32 edma_evt, int i, int j )
{
    edma3ccRegs->PARAMSET[edma_evt].SRC = (Uint32)image;
    edma3ccRegs->PARAMSET[edma_evt].DST = (Uint32)image + i*PIXELS_IN_IMAGE/3 + j*PIXELS_PER_LINE/3;

    edma3ccRegs->PARAMSET[edma_evt].OPT =             CSL_TPCC_PARAM_OPT_RESETVAL
                                                    | CSL_FMKT(TPCC_PARAM_OPT_TCINTEN, ENABLE)
                                                    | CSL_FMK(TPCC_PARAM_OPT_TCC, edma_evt)
                                                    | CSL_FMKT(TPCC_PARAM_OPT_SYNCDIM, ABSYNC);

    edma3ccRegs->PARAMSET[edma_evt].A_B_CNT =         CSL_FMK(TPCC_PARAM_A_B_CNT_BCNT, Y_SIZE)
                                                    | CSL_FMK(TPCC_PARAM_A_B_CNT_ACNT, X_SIZE);
    edma3ccRegs->PARAMSET[edma_evt].SRC_DST_BIDX =    CSL_FMK(TPCC_PARAM_SRC_DST_BIDX_DSTBIDX, PIXELS_PER_LINE)
                                                    | CSL_FMK(TPCC_PARAM_SRC_DST_BIDX_SRCBIDX, PIXELS_PER_LINE);
    edma3ccRegs->PARAMSET[edma_evt].LINK_BCNTRLD =    CSL_FMK(TPCC_PARAM_LINK_BCNTRLD_LINK, 0xFFFF);
    edma3ccRegs->PARAMSET[edma_evt].SRC_DST_CIDX = 0;
    edma3ccRegs->PARAMSET[edma_evt].CCNT = 1;
}


void SetupEDMA3Chain( Uint32 edma_evt, int i, int j )
{
    edma3ccRegs->PARAMSET[edma_evt].SRC = (Uint32)image;
    edma3ccRegs->PARAMSET[edma_evt].DST = (Uint32)image + i*PIXELS_IN_IMAGE/3 + j*PIXELS_PER_LINE/3;

    edma3ccRegs->PARAMSET[edma_evt].OPT =             CSL_TPCC_PARAM_OPT_RESETVAL
                                                    | CSL_FMKT(TPCC_PARAM_OPT_TCCHEN, ENABLE)
                                                    | CSL_FMK(TPCC_PARAM_OPT_TCC, edma_evt+1)
                                                    | CSL_FMKT(TPCC_PARAM_OPT_SYNCDIM, ABSYNC);

    edma3ccRegs->PARAMSET[edma_evt].A_B_CNT =         CSL_FMK(TPCC_PARAM_A_B_CNT_BCNT, Y_SIZE)
                                                    | CSL_FMK(TPCC_PARAM_A_B_CNT_ACNT, X_SIZE);
    edma3ccRegs->PARAMSET[edma_evt].SRC_DST_BIDX =    CSL_FMK(TPCC_PARAM_SRC_DST_BIDX_DSTBIDX, PIXELS_PER_LINE)
                                                    | CSL_FMK(TPCC_PARAM_SRC_DST_BIDX_SRCBIDX, PIXELS_PER_LINE);
    edma3ccRegs->PARAMSET[edma_evt].LINK_BCNTRLD =    CSL_FMK(TPCC_PARAM_LINK_BCNTRLD_LINK, 0xFFFF);
    edma3ccRegs->PARAMSET[edma_evt].SRC_DST_CIDX = 0;
    edma3ccRegs->PARAMSET[edma_evt].CCNT = 1;
}

void main()
{
    Int32 i, j, k=0;

    puts("\n=====================================================");
    puts("          Lab6B - 2D Memory Move");
    puts("=====================================================");

    SysConfigForPinMux( );
    LED_DIPSW_Init( );

    for(i=0; i < PIXELS_IN_IMAGE;i++)
            image[i] = 200;                 // gray background

    for( i=0; i < Y_SIZE; i++ )
        for( j=0; j < X_SIZE; j++ )
            image[i*PIXELS_PER_LINE + j] = org_img[k++];

    puts("Stop here");
    BIOS_start( );
}


void Task_edma( void )
{

#if 0
    ResetEDMA3( );
    SetupEDMA3(EDMA_EVENT5, 2,2);
    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E5, 1);
    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT5)) == 0 );


#elif 0
    for( int i=0; i<=2; i++)
        for( int j=0; j<=2; j++)
        {
            if( i==0 && j==0 )  continue;
            ResetEDMA3( );
            SetupEDMA3( EDMA_EVENT5, i, j );
            CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E5, 1);
            while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT5)) == 0 );
        }
#elif 1
    SetupEDMA3Chain( EDMA_EVENT5, 0, 1 );
    SetupEDMA3Chain( EDMA_EVENT6, 0, 2 );
    SetupEDMA3Chain( EDMA_EVENT7, 1, 0 );
    SetupEDMA3Chain( EDMA_EVENT8, 1, 1 );
    SetupEDMA3Chain( EDMA_EVENT9, 1, 2 );
    SetupEDMA3Chain( EDMA_EVENT10, 2, 0 );
    SetupEDMA3Chain( EDMA_EVENT11, 2, 1 );
    SetupEDMA3( EDMA_EVENT12, 2, 2 );

    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E5, 1);
    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT12)) == 0 );

#else

    SetupEDMA3( EDMA_EVENT5, 0, 1 );
    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E5, 1);
    SetupEDMA3( EDMA_EVENT6, 0, 2 );
    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E6, 1);
    SetupEDMA3( EDMA_EVENT7, 1, 0 );
    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E7, 1);
    SetupEDMA3( EDMA_EVENT8, 1, 1 );
    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E8, 1);
    SetupEDMA3( EDMA_EVENT9, 1, 2 );
    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E9, 1);
    SetupEDMA3( EDMA_EVENT10, 2, 0 );
    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E10, 1);
    SetupEDMA3( EDMA_EVENT11, 2, 1 );
    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E11, 1);
    SetupEDMA3( EDMA_EVENT12, 2, 2 );
    CSL_FINS(edma3ccRegs->TPCC_ESR, TPCC_TPCC_ESR_E12, 1);

    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT5)) == 0 );
    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT6)) == 0 );
    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT7)) == 0 );
    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT8)) == 0 );
    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT9)) == 0 );
    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT10)) == 0 );
    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT11)) == 0 );
    while( (edma3ccRegs->TPCC_IPR & (1<< EDMA_EVENT12)) == 0 );

#endif

    System_printf(">> Program Terminated!\n");

    BIOS_exit( 0 );
}




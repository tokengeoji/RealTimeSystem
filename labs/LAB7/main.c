/*
 *  ======== main.c ========
 */

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/cfg/global.h>

#include <stdio.h>
#include "init.h"
#include "define.h"
#include "McASP.h"
#include "EDMA_McASP.h"

void SetupEDMA3( void )
{
    EDMAReset( );

    /********************************************************************************/
    /*                             For Receiver                                     */
    /********************************************************************************/
#ifdef EDMA_LINK
    ConfigEDMAForR( R_LINK_PING, (Uint32)in_ping_buffer, BUFLEN, ELEMENT_SIZE, R_LINK_PONG, TCINT );
    ConfigEDMAForR( R_LINK_PONG, (Uint32)in_pong_buffer, BUFLEN, ELEMENT_SIZE, R_LINK_PING, TCINT );

    ConfigEDMAForR( REVT, (Uint32)in_ping_buffer, BUFLEN, ELEMENT_SIZE, R_LINK_PONG, TCINT );
#else
    ConfigEDMAForR( REVT, (Uint32)in_ping_buffer, BUFLEN, ELEMENT_SIZE, REVT, TCINT );
#endif

    /********************************************************************************/
    /*                             For Transmiter                                   */
    /********************************************************************************/
#ifdef EDMA_LINK
    ConfigEDMAForX( X_LINK_PING, (Uint32)out_ping_buffer, BUFLEN, ELEMENT_SIZE, X_LINK_PONG, TCINT );
    ConfigEDMAForX( X_LINK_PONG, (Uint32)out_pong_buffer, BUFLEN, ELEMENT_SIZE, X_LINK_PING, TCINT );

    ConfigEDMAForX( XEVT, (Uint32)out_ping_buffer, BUFLEN, ELEMENT_SIZE, X_LINK_PONG, TCINT );
#else
    ConfigEDMAForX( XEVT, (Uint32)out_ping_buffer, BUFLEN, ELEMENT_SIZE, XEVT, TCINT );
#endif

    //  E0  : McASP RCV event Channel
    //  E1  : McASP XMT event Channel

    CSL_FINS(edma3ccRegs->TPCC_EESR, TPCC_TPCC_EESR_E0, 1);
    CSL_FINS(edma3ccRegs->TPCC_EESR, TPCC_TPCC_EESR_E1, 1);
    CSL_FINS(edma3ccRegs->TPCC_IESR, TPCC_TPCC_IESR_IESR0, 1);
    CSL_FINS(edma3ccRegs->TPCC_DRA[CSL_EDMA3_REGION_1].DRAE, TPCC_TPCC_DRAE1_DRAE1_E0, 1);
}

void main(void)
{
    void ClearBuffer( void );
    puts("\n======================================");
    puts("     Lab7 - Double Buffering (EDMA)");
    puts("======================================");

    SysConfigForPinMux( );
    LED_DIPSW_Init( );
    I2C_Init( 400 );

    ClearBuffer( );

    CodecInit( SAMPLING_FREQ, WORD_LEN_16BIT, LINE_IN );
    ConfigMcASP( MCASP_32BIT, MCASP_1SLOT, CFG, NO_XINT, NO_XINT );

    SetupEDMA3( );

    EnableInterrupt( INT_5 );

    InitMcASP( RECEIVE, CFG );
    InitMcASP( TRANSMIT, CFG );

    BIOS_start( );
}



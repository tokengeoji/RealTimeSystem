/*
 * ISR.c
 *
 *  Created on: 2020. 9. 21.
 *      Author: USER0
 */

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Swi.h>
#include <xdc/cfg/global.h>
#include <math.h>
#include "LED_DIPSW.h"
#include "Timer.h"
#include "interrupt.h"
#include "McASP.h"
#include "define.h"
#include "sinef.h"
#include "EDMA_McASP.h"

#define TALKTHR     0
#define DELEY       1

volatile int32_t op_mode = TALKTHR;

void TIMER1_TINT12_ISR( void ) {
    StopTimer( (CSL_TmrRegsOvly)CSL_TMR_1_REGS );
    GPIO_ClearInterruptState( GP2 );
    ClearInterrupt( INT_NUM_GPIO_B2 );
    EnableInterrupt( INT_NUM_GPIO_B2 );
}

void GPIO_PUSHBUTTON_ISR( void ) {

    op_mode = !op_mode;

    DisableInterrupt( INT_NUM_GPIO_B2 );
    StartTimer( (CSL_TmrRegsOvly)CSL_TMR_1_REGS );
}


//-----------------------------------------------------------

uint32_t in_ping_buffer[BUFLEN];
uint32_t in_pong_buffer[BUFLEN];
uint32_t out_ping_buffer[BUFLEN];
uint32_t out_pong_buffer[BUFLEN];


void ClearBuffer( void ) {
    for(int i=0; i < BUFLEN; i++ )
        out_ping_buffer[i] = out_pong_buffer[i] = 0;
}

void EDMA_ISR( void ) {
    static int32_t which_buffer=PING;

    CSL_FINS( edma3ccRegs->TPCC_ICR, TPCC_TPCC_ICR_ICR0, 1 );
    while( CSL_FEXT( edma3ccRegs->TPCC_IPR, TPCC_TPCC_IPR_IPR1 ) != 1 );
    CSL_FINS( edma3ccRegs->TPCC_ICR, TPCC_TPCC_ICR_ICR1, 1 );

    if( which_buffer == PING ) {
        edma3ccRegs->PARAMSET[REVT].DST = (uint32_t)in_pong_buffer;
        edma3ccRegs->PARAMSET[XEVT].SRC = (uint32_t)out_pong_buffer;
    }
    else {
        edma3ccRegs->PARAMSET[REVT].DST = (uint32_t)in_ping_buffer;
        edma3ccRegs->PARAMSET[XEVT].SRC = (uint32_t)out_ping_buffer;
    }

    edma3ccRegs->PARAMSET[REVT].A_B_CNT =    CSL_FMK(TPCC_PARAM_A_B_CNT_BCNT, BUFLEN)
                                           | CSL_FMK(TPCC_PARAM_A_B_CNT_ACNT, ELEMENT_SIZE);

    edma3ccRegs->PARAMSET[XEVT].A_B_CNT =    CSL_FMK(TPCC_PARAM_A_B_CNT_BCNT, BUFLEN)
                                           | CSL_FMK(TPCC_PARAM_A_B_CNT_ACNT, ELEMENT_SIZE);

    edma3ccRegs->PARAMSET[REVT].CCNT =   1;
    edma3ccRegs->PARAMSET[XEVT].CCNT =   1;

    edma3ccRegs->PARAMSET[REVT].LINK_BCNTRLD =  (Uint32)(&(edma3ccRegs->PARAMSET[REVT]));
    edma3ccRegs->PARAMSET[XEVT].LINK_BCNTRLD =  (Uint32)(&(edma3ccRegs->PARAMSET[XEVT]));

    which_buffer = ~which_buffer;

    Swi_or(SWI0, which_buffer);

}



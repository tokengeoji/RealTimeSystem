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


void ClearData( void ) {
    for(int i=0; i < BUFLEN; i++ )
        out_ping_buffer[i] = out_pong_buffer[i] = 0;
}

void MCASP_ISR( void ) {
    static uint32_t sample_cnt=0;
    static uint32_t *in = in_ping_buffer, *out = out_ping_buffer;
    static int32_t which_buffer=PING;

    in[sample_cnt] = mcaspRegs->RBUF14;
    mcaspRegs->XBUF13 = out[sample_cnt++];

    if( sample_cnt == BUFLEN ) {
        sample_cnt = 0;

        if( which_buffer == PING ) {
            in  = in_pong_buffer;
            out = out_pong_buffer;
        }
        else {
            in  = in_ping_buffer;
            out = out_ping_buffer;
        }

        which_buffer = ~which_buffer;

        Swi_or(SWI0, which_buffer);
    }
}



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
static int32_t led_cnt = 0;
static int32_t which_buffer=PING;

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

        Swi_post(SWI0);
    }

     ++led_cnt;
}


void ProcessSwi0( void )
{
    uint32_t *in, *out;
    static int32_t index=0;
    static float sine[BUFLEN];

    if( which_buffer == PING ) {
        in  = in_pong_buffer;
        out = out_pong_buffer;
    }
    else {
        in  = in_ping_buffer;
        out = out_ping_buffer;
    }

    if( op_mode == TALKTHR )
        for( int i=0; i < BUFLEN; i++ ) out[i] = in[i];
    else {
        index = GenSine( sine, 2.f, SAMPLING_FREQ, BUFLEN, index );
        for( int i=0; i < BUFLEN; i++ ) {
            Int16 left  = (in[i] >> 16);
            Int16 right = (in[i] & 0xFFFF);

            left  *= sine[i];
            right *= sine[i];
            out[i] = (left<<16) | (right & 0xFFFF);
        }
    }
}



void IdleLED( void ) {
    if( led_cnt == (SAMPLING_FREQ>>1) ) {
        LED_Toggle( LED_D4 );
        led_cnt = 0;
    }
}

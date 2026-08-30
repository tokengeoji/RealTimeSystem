/*
 * ISR.c
 *
 *  Created on: 2020. 9. 21.
 *      Author: USER0
 */

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


#define BUFLEN  40

static uint32_t in_buffer[BUFLEN];
static uint32_t out_buffer[BUFLEN];


void ClearData( void ) {
    for(int i=0; i < BUFLEN; i++ )
        in_buffer[i] = out_buffer[i] = 0;
}

static float sine[BUFLEN];

void MCASP_ISR( void ) {
    int16_t right, left;
    static int sample_cnt = 0;
    static int index = 0;

    in_buffer[sample_cnt] = mcaspRegs -> RBUF14;
    mcaspRegs -> XBUF13 = out_buffer[sample_cnt++];

    if( sample_cnt == BUFLEN ) {
        if( op_mode == TALKTHR ) {
            for( int i=0; i < BUFLEN; i++ )
                out_buffer[i] = in_buffer[i];
        }
        else {
            index = GenSine( sine, 2.f, SAMPLING_FREQ, BUFLEN, index );

            for( int i=0; i < BUFLEN; i++ ) {
                left  = in_buffer[i] >> 16;
                right = in_buffer[i] & 0x0000ffff;

                left  = left  * sine[i];
                right = right * sine[i];

                out_buffer[i] = (left << 16) | (right & 0x0000ffff );
            }

        }

        sample_cnt = 0;
    }

    led_cnt++;
}



void IdleLED( void ) {
    if( led_cnt == (SAMPLING_FREQ>>1) ) {
        LED_Toggle( LED_D4 );
        led_cnt = 0;
    }
}

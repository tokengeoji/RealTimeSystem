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

#define BUFLEN  (SAMPLING_FREQ/2)
static uint32_t data[BUFLEN];

void ClearData( void ) {
    for(int i=0; i < BUFLEN; i++ )
        data[i] = 0;
}

void MCASP_ISR( void ) {
    volatile uint32_t sample;
    int16_t right, left;
    static int cnt = 0;

    sample = mcaspRegs -> RBUF14;

    if( op_mode == TALKTHR )
        mcaspRegs -> XBUF13 = sample;
    else {
        left  = (sample >> 16);
        right = (data[cnt] & 0x0000FFFF);

        mcaspRegs -> XBUF13 = (left << 16) | ( right & 0xFFFF);
    }

    data[cnt++] = sample;

    if( cnt == BUFLEN ) cnt = 0;

    led_cnt++;
}

void IdleLED( void ) {
    if( led_cnt == (SAMPLING_FREQ>>1) ) {
        LED_Toggle( LED_D4 );
        led_cnt = 0;
    }
}

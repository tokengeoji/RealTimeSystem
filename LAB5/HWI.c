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
#define GENSINE     1

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


#if 0

void MCASP_ISR( void ) {
    uint32_t sample;
    int16_t right, left;

    sample = mcaspRegs->RBUF14;
    //mcaspRegs->XBUF13 = sample;

    left = sample >> 16;
    //left = 0;
    right = sample & 0x0000FFFF;


    mcaspRegs->XBUF13 = (left << 16 ) | (right & 0x0000FFFF);

    led_cnt++;
}

#else

#define PI      3.141592654
#define FREQ    300.
#define AMP     10000.

static float w = 2.*PI*FREQ/SAMPLING_FREQ;

void MCASP_ISR( void ) {
    uint32_t sample;
    int16_t right, left;
    static int32_t index=0;

    sample = mcaspRegs -> RBUF14;

    if( op_mode == TALKTHR ) {
        left  = sample >> 16;
        right = sample & 0x0000FFFF;
    }
    else {
        left = right = AMP*sin( w * index++ );
        if( index == SAMPLING_FREQ ) index=0;
    }

    mcaspRegs -> XBUF13 = (left << 16) | (right & 0x0FFFF);

    led_cnt++;
}

#endif


void IdleLED( void ) {
    if( led_cnt == (SAMPLING_FREQ>>1) ) {
        LED_Toggle( LED_D4 );
        led_cnt = 0;
    }
}

/*
#if 0

interrupt void GPIO_PUSHBUTTON_ISR( void ) {

    led_flag = !led_flag;
}

#else

interrupt void GPIO_PUSHBUTTON_ISR( void ) {
    Uint8 is_pb_sw1, is_pb_sw2;
    static int led_mode = LED_ONOFF;

    is_pb_sw1 = GPIO_InterruptState( GP2, GP2P4 );
    is_pb_sw2 = GPIO_InterruptState( GP2, GP2P5 );

    if( is_pb_sw1 == 1)
        led_flag = !led_flag;
    else if( is_pb_sw2 == 1 ) {
        if( led_mode == LED_ONOFF ) {
            LED_Off( LED_D4 );
            LED_Off( LED_D5 );
            LED_On( LED_D6 );
            LED_On( LED_D7 );
        }
        else {
            LED_Off( LED_D4 );
            LED_Off( LED_D5 );
            LED_Off( LED_D6 );
            LED_Off( LED_D7 );
        }

        led_mode = !led_mode;
    }
}

#endif

*/



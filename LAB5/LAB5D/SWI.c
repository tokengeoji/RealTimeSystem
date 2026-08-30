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

static int32_t led_cnt = 0;

void ProcessSwi0( void )
{
    uint32_t *in, *out;
    static int32_t index=0;
    static float sine[BUFLEN];

    int32_t which_buffer = Swi_getTrigger( );

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

    led_cnt++;
}



void IdleLED( void ) {
    if( led_cnt == (SAMPLING_FREQ>>1)/BUFLEN ) {
        LED_Toggle( LED_D4 );
        led_cnt = 0;
    }
}


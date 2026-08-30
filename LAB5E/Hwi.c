/***********************************************************
* 	
*	Interrupt Service Routine Module  
*                              
*	File Name : ISR.c 
*	Date Created :
*	Last modified :
*
************************************************************/

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Swi.h>
#include <xdc/cfg/global.h>
#include <math.h>
#include "define.h"
#include "mcasp.h"
#include "codec.h"
#include "LED_DIPSW.h"


static Int32 led_cnt=0;


void MCASP_ISR( void ) {
    Uint32  dummy;
    Int16 out_val;
    static Int16 prev=0;
    extern volatile Int32 time_index;
    extern volatile float omega;
    extern volatile Int16 done, temp_done;

    dummy = mcaspRegs->RBUF14;

    out_val = (Int16)( 15000.*sinf( omega * time_index++ ) );

    if( time_index == SAMPLING_FREQ_48000 )
        time_index = 0;

    mcaspRegs->XBUF13 = out_val;

    if( temp_done == 1 )
        if( prev < 0 && out_val >= 0 )
            done = 1;

    prev = out_val;

    ++led_cnt;
}


void IdleLED( void ) {
	if( led_cnt == (48000 >> 1) ) {
		LED_Toggle(LED_D4);
		led_cnt = 0;
	}
}




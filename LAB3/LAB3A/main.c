#include <stdio.h>
#include "init.h"
#include "LED_DIPSW.h"
#include "sine.h"

#pragma diag_suppress=112

#define SAMPLING_RATE   8000.f
#define BUFLEN          200

short data[BUFLEN];
float freq = 200.f;
float vol  = 5000.f;

void MicroSecDelay( unsigned delay ) {
    volatile unsigned i;

    for( i=0; i < delay * 10; i++ )
        asm(" nop");
}

/****************************************************************
 *                            main.c
 ****************************************************************/
int main(void)
{
    int index = 0;

    puts("\n===============================================");
    puts("    LAB3A -- Sine Waveform Generation & GUI");
    puts("===============================================");

    SysConfigForPinMux( );
    LED_DIPSW_Init( );

    for( int i=0; i < 1000; i++) {
        GenSine( data, freq, vol, SAMPLING_RATE, BUFLEN, i*10 );
        MicroSecDelay( 300000 );
    }

    puts("Program Terminated!!!");
    return 0;
}



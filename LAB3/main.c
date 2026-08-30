#include <stdio.h>
#include "init.h"
#include "LED_DIPSW.h"
#include "sine.h"

#pragma diag_suppress=112

#define SAMPLING_RATE   8000.f
#define BUFLEN          200
#define STEP            10

short data[BUFLEN];
float freq = 250.f;
float vol  = 5000.f;

/****************************************************************
 *                            main.c
 ****************************************************************/
int main(void)
{
    int index = 0;

    puts("\n===============================================");
    puts("        LAB3 -- Sine Waveform Generation");
    puts("===============================================");

    SysConfigForPinMux( );
    LED_DIPSW_Init( );

    //GenSine( data, 250.f, 8000.f, BUFLEN, 0 );

    for( int i=0; i < BUFLEN; i+=STEP)
        index = GenSine( &data[i], freq, vol, SAMPLING_RATE, STEP, index );

    puts("Program Terminated!!!");
    return 0;
}



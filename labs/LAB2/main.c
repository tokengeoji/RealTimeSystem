#include <stdio.h>
#include "init.h"
#include "LED_DIPSW.h"
#include "Interrupt.h"


void MicroSecDelay( unsigned delay ) {
    volatile unsigned i;

    for( i=0; i < delay * 10; i++ )
        asm(" nop");
}

#pragma diag_suppress=112

volatile int led_flag = LED_ON;

/****************************************************************
 *                            main.c
 ****************************************************************/
int main(void)
{
    void GPIO_PUSHBUTTON_ISR( void );

    puts("\n===============================================");
    puts("          LAB2 -- LED BLINKING(Interrupt)");
    puts("===============================================");

    SysConfigForPinMux( );
    LED_DIPSW_Init( );
    ConfigInterrupt( );

    InstallISR( INT_NUM_GPIO_B2, GPIO_PUSHBUTTON_ISR );

    EnableInterrupt( INT_NUM_GPIO_B2 );
    _enable_interrupts( );

    while(1) {

        if( led_flag == LED_ON ) {
            LED_Toggle( LED_D4 );
            LED_Toggle( LED_D5 );
            LED_Toggle( LED_D6 );
            LED_Toggle( LED_D7 );
        }
        MicroSecDelay( 500000 );
    }

    return 0;
}



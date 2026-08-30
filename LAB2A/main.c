#include <stdio.h>
#include "init.h"
#include "LED_DIPSW.h"
#include "Interrupt.h"
#include "Timer.h"

#pragma diag_suppress=112

volatile int led_flag = LED_ON;

/****************************************************************
 *                            main.c
 ****************************************************************/
int main(void)
{
    void GPIO_PUSHBUTTON_ISR( void );
    void TIMER_TINT12_ISR( void );

    puts("\n===============================================");
    puts("       LAB2A -- LED BLINKING(Timer Interrupt)");
    puts("===============================================");

    SysConfigForPinMux( );
    LED_DIPSW_Init( );
    ConfigInterrupt( );

    ConfigTimer_32bit( 500 );

    InstallISR( INT_NUM_GPIO_B2, GPIO_PUSHBUTTON_ISR );
    InstallISR( INT_NUM_T64P1, TIMER_TINT12_ISR );

    EnableInterrupt( INT_NUM_GPIO_B2 );
    EnableInterrupt( INT_NUM_T64P1 );
    _enable_interrupts( );

    StartTimer( );

    while(1);

    return 0;
}



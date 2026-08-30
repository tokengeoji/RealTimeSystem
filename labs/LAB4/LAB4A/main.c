/*
 *  ======== main.c ========
 */

#include <stdio.h>
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include "init.h"
#include "LED_DIPSW.h"

int pb1, pb2;
/*
 *  ======== main ========
 */
Int main()
{ 
    System_printf("Hello World - SysBios!!\n");
    printf("Hello World!!\n");

    SysConfigForPinMux( );
    LED_DIPSW_Init( );

    BIOS_start();    /* does not return */
    return(0);
}


void ProcessClk1( void ) {

    if( pb1 == PUSH_DOWN ) {
        LED_Toggle( LED_D4 );
        LED_Toggle( LED_D5 );
    }
    else {
        LED_Off( LED_D4 );
        LED_Off( LED_D5 );
    }
}

void ProcessClk2( void ) {

    if( pb2 == PUSH_DOWN ) {
        LED_Toggle( LED_D6 );
        LED_Toggle( LED_D7 );
    }
    else {
        LED_Off( LED_D6 );
        LED_Off( LED_D7 );
    }
}


void ProcessIdle1( void ) {
    pb1 = GetPushButton_Status( PUSH_BTN_1 );
}


void ProcessIdle2( void ) {
    pb2 = GetPushButton_Status( PUSH_BTN_2 );
}










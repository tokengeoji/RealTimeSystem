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
#include "LED_DIPSW.h"

void TIMER_ISR( void ) {
    LED_Toggle( LED_D4 );
}





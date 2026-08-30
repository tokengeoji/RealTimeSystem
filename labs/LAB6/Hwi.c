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

void EDMA_ISR( void ) {
    extern volatile int edma_done;

    edma_done = 1;
}





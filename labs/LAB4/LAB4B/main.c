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
#include "Timer.h"

/*
 *  ======== main ========
 */
Int main()
{ 
    puts("\n===============================================");
    puts("           LAB4B -- HWI Thread (Timer)");
    puts("===============================================");

    SysConfigForPinMux( );
    LED_DIPSW_Init( );

    ConfigTimer_32bit( (CSL_TmrRegsOvly)CSL_TMR_1_REGS, 500 );

    //StartTimer( (CSL_TmrRegsOvly)CSL_TMR_1_REGS );

    BIOS_start();    /* does not return */
    return(0);
}











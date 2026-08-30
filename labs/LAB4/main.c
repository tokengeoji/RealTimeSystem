/*
 *  ======== main.c ========
 */

#include <stdio.h>
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>


/*
 *  ======== main ========
 */
Int main()
{ 
    System_printf("Hello World - SysBios!!\n");
    printf("Hello World!!\n");

    //BIOS_start();    /* does not return */
    return(0);
}

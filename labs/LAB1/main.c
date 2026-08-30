#include <stdio.h>
#include "init.h"
#include "LED_DIPSW.h"


void MicroSecDelay( unsigned delay ) {
    volatile unsigned i;

    for( i=0; i < delay * 10; i++ )
        asm(" nop");
}

#pragma diag_suppress=112

#if 0
/****************************************************************
 *                            main.c
 ****************************************************************/
int main(void)
{
    puts("===============================================");
	puts("          LAB1 -- LED BLINKING");
	puts("===============================================");

	SysConfigForPinMux( );
	LED_DIPSW_Init( );

	while(1) {
	    int dip1 = GetDIP_Status( DIP_1 );
	    int dip2 = GetDIP_Status( DIP_2 );
	    int dip3 = GetDIP_Status( DIP_3 );
	    int dip4 = GetDIP_Status( DIP_4 );

	    if( dip1 == DIP_ON ) LED_Toggle( LED_D4 );
	    if( dip2 == DIP_ON ) LED_Toggle( LED_D5 );
	    if( dip3 == DIP_ON ) LED_Toggle( LED_D6 );
	    if( dip4 == DIP_ON ) LED_Toggle( LED_D7 );

	    MicroSecDelay( 500000 );
	}

	return 0;
}

#else

/****************************************************************
 *                            main.c
 ****************************************************************/
int main(void)
{
    puts("===============================================");
    puts("          LAB1A -- LED BLINKING");
    puts("===============================================");

    SysConfigForPinMux( );
    LED_DIPSW_Init( );

    while(1) {
        int pb1 = GetPushButton_Status( PUSH_BTN_1 );
        int pb2 = GetPushButton_Status( PUSH_BTN_2 );

        if( pb1 == PUSH_DOWN ) {
            LED_Toggle( LED_D4 );
            LED_Toggle( LED_D5 );
        }

        if( pb2 == PUSH_DOWN ) {
            LED_Toggle( LED_D6 );
            LED_Toggle( LED_D7 );
        }

        if( pb1 != PUSH_DOWN && pb2 != PUSH_DOWN ) {
            LED_Off( LED_D4 );
            LED_Off( LED_D5 );
            LED_Off( LED_D6 );
            LED_Off( LED_D7 );
        }

        MicroSecDelay( 500000 );
    }

    return 0;
}

#endif

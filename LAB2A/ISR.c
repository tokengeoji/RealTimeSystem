/*
 * ISR.c
 *
 *  Created on: 2020. 9. 21.
 *      Author: USER0
 */

#include "LED_DIPSW.h"

extern volatile int led_flag;

interrupt void TIMER_TINT12_ISR( void ) {
    if( led_flag == LED_ON ) {
        LED_Toggle( LED_D4 );
        LED_Toggle( LED_D5 );
        LED_Toggle( LED_D6 );
        LED_Toggle( LED_D7 );
    }
}


#if 0

interrupt void GPIO_PUSHBUTTON_ISR( void ) {

    led_flag = !led_flag;
}

#else

interrupt void GPIO_PUSHBUTTON_ISR( void ) {
    Uint8 is_pb_sw1, is_pb_sw2;
    static int led_mode = LED_ONOFF;

    is_pb_sw1 = GPIO_InterruptState( GP2, GP2P4 );
    is_pb_sw2 = GPIO_InterruptState( GP2, GP2P5 );

    if( is_pb_sw1 == 1)
        led_flag = !led_flag;
    else if( is_pb_sw2 == 1 ) {
        if( led_mode == LED_ONOFF ) {
            LED_Off( LED_D4 );
            LED_Off( LED_D5 );
            LED_On( LED_D6 );
            LED_On( LED_D7 );
        }
        else {
            LED_Off( LED_D4 );
            LED_Off( LED_D5 );
            LED_Off( LED_D6 );
            LED_Off( LED_D7 );
        }

        led_mode = !led_mode;
    }
}

#endif

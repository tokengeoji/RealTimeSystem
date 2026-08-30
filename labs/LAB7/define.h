/*
 * define.h
 *
 *  Created on: 2025. 10. 20.
 *      Author: 212-0
 */

#ifndef DEFINE_H_
#define DEFINE_H_

#include "Codec.h"

#define SAMPLING_FREQ   SAMPLING_FREQ_48000
#define BUFLEN          400
#define ELEMENT_SIZE    4

#define PING    0
#define PONG    (~PING)

extern volatile int32_t op_mode;
extern int32_t which_buffer;

extern uint32_t in_ping_buffer[BUFLEN];
extern uint32_t in_pong_buffer[BUFLEN];
extern uint32_t out_ping_buffer[BUFLEN];
extern uint32_t out_pong_buffer[BUFLEN];

#endif /* DEFINE_H_ */

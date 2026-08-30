/*
 * define.h
 *
 *  Created on: 2025. 10. 20.
 *      Author: 212-0
 */

#ifndef DEFINE_H_
#define DEFINE_H_

#include "Codec.h"

typedef struct {
    int16_t key;
    uint32_t duration;
} Note;

#define NUM_KEY     88
#define MAX_NOTE    1000
#define DUR         500.


#endif /* DEFINE_H_ */

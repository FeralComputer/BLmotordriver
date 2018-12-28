/*
 * motordriver.h
 *
 *  Created on: Dec 28, 2018
 *      Author: kevin
 */

#ifndef MOTORDRIVER_H_
#define MOTORDRIVER_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

#define TIMER_A_PERIOD 32768
#define MAXSTATES 3
#define MAXPULSECOUNT 30
#define NUMBEROFPINS 3
#define MAXDUTYCYCLE 100
#define isOdd(x) ((x)&0x01)
#define isEven(x) !isOdd(x)
#define MAXPULSEWIDTH 1700
#define MINPULSEWIDTH 30000
#define abs(x) ((x)>0?(x):-(x))
#define DEFAULTDUTYCYCLESTEP 10
#define DEBUG 0
#define INITIALDUTYCYCLE 10
#define MINDELTAPULSEWIDTHSTEP 100

uint8_t state;
uint16_t pulsecount;
uint16_t dutycycle;
uint16_t dutycyclestep;
uint16_t pulsewidth;
uint16_t wantedpulsewidth;

void motordriver_init(void);

#endif /* MOTORDRIVER_H_ */

/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432 Empty Project
 *
 * Description: An empty project that uses DriverLib. In this project, DriverLib
 * is built from source instead of the usual library.
 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 * Author: 
 *******************************************************************************/
/* DriverLib Includes */
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

uint8_t state = 0;
uint16_t pulsecount = 0;
uint16_t dutycycle = INITIALDUTYCYCLE;
uint16_t dutycyclestep = DEFAULTDUTYCYCLESTEP;
uint16_t pulsewidth=MINPULSEWIDTH;
uint16_t wantedpulsewidth=MAXPULSEWIDTH;

Timer_A_ContinuousModeConfig timerA0 = {
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_1,
        TIMER_A_TAIE_INTERRUPT_ENABLE,
        TIMER_A_SKIP_CLEAR };

Timer_A_CompareModeConfig timerA0_compare = {
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,
        TIMER_A_OUTPUTMODE_OUTBITVALUE, 100 };

typedef struct
{
    uint8_t port;
    uint16_t pin;
} PINS;

PINS PWMpins[NUMBEROFPINS] =
{
    { GPIO_PORT_P2, GPIO_PIN4 },
    { GPIO_PORT_P2, GPIO_PIN6 },
    { GPIO_PORT_P5, GPIO_PIN6 }
};

PINS pins[NUMBEROFPINS]=
{
     { GPIO_PORT_P2, GPIO_PIN0 },
     { GPIO_PORT_P2, GPIO_PIN2 },
     { GPIO_PORT_P2, GPIO_PIN1 }
};

inline void pwmControl(PINS* pin);
inline void incrementTimerCompare(uint16_t increment);

int main(void)
{
    /* Stop Watchdog */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();

    //init
    if(DEBUG)
    {
        MAP_CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
        MAP_CS_initClockSignal(CS_SMCLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    }
    else
    {
        //Set core voltage to VCORE1
            MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

            //Set flash wait states for flash bank 0 and 1
            MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
            MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

            //init clock system
            MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
            MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
            MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
            MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
            MAP_CS_initClockSignal(CS_SMCLK,CS_DCOCLK_SELECT,CS_CLOCK_DIVIDER_2);
    }

    MAP_Timer_A_configureContinuousMode(TIMER_A0_BASE, &timerA0);
    Timer_A_initCompare(TIMER_A0_BASE, &timerA0_compare);
    int i;
    for (i = 0; i < NUMBEROFPINS; i++)
    {
        MAP_GPIO_setAsOutputPin(PWMpins[i].port, PWMpins[i].pin);
        MAP_GPIO_setOutputLowOnPin(PWMpins[i].port, PWMpins[i].pin);

        MAP_GPIO_setAsOutputPin(pins[i].port, pins[i].pin);
        MAP_GPIO_setOutputLowOnPin(pins[i].port, pins[i].pin);
    }

    //enable interrupts
    MAP_Interrupt_enableInterrupt(INT_TA0_N);
    //enable master interrupt
    MAP_Interrupt_enableMaster();
    //start timera0
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);

    while (1)
    {

    }
}

void TA0_N_IRQHandler(void)
{
    uint32_t status = MAP_Timer_A_getCaptureCompareEnabledInterruptStatus(
            TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
    MAP_Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_1);

    switch (state)
    {
    case 0:
        if(!pulsecount)
        {
            MAP_GPIO_setOutputLowOnPin(pins[0].port,pins[0].pin);
            MAP_GPIO_setOutputHighOnPin(pins[1].port,pins[1].pin);

            MAP_GPIO_setOutputLowOnPin(PWMpins[2].port, PWMpins[2].pin);
        }

        pwmControl(&PWMpins[state]);
        break;
    case 1:
        if(!pulsecount)
        {
            MAP_GPIO_setOutputLowOnPin(pins[1].port,pins[1].pin);
            MAP_GPIO_setOutputHighOnPin(pins[2].port,pins[2].pin);

            MAP_GPIO_setOutputLowOnPin(PWMpins[0].port,PWMpins[0].pin);
        }

        pwmControl(&PWMpins[state]);
        break;
    case 2:
        if(!pulsecount)
        {
            MAP_GPIO_setOutputLowOnPin(pins[2].port,pins[2].pin);
            MAP_GPIO_setOutputHighOnPin(pins[0].port,pins[0].pin);

            MAP_GPIO_setOutputLowOnPin(PWMpins[1].port,PWMpins[1].pin);
        }
        pwmControl(&PWMpins[state]);
        break;
    default:

        break;
    }

}

inline void pwmControl(PINS* pin)
{
    if (pulsecount >= (MAXPULSECOUNT - 1 - 2*dutycycle / DEFAULTDUTYCYCLESTEP))
    {//bring pwm back down before change in state
        dutycyclestep = -abs(dutycyclestep);
        dutycycle += dutycyclestep;
    }
    //set pin
    if (dutycycle <= 0)
    {
        MAP_GPIO_setOutputLowOnPin(pin->port, pin->pin);
        pulsecount += 2;
//        dutycyclestep = abs(dutycyclestep);
        incrementTimerCompare(pulsewidth);
        dutycycle += dutycyclestep;
    }
    else if (dutycycle >= MAXDUTYCYCLE)
    {
        MAP_GPIO_setOutputHighOnPin(pin->port, pin->pin);
        pulsecount += 2;
        incrementTimerCompare(pulsewidth);

    }
    else
    {
        if (isOdd(pulsecount))
        { //high
            MAP_GPIO_setOutputHighOnPin(pin->port, pin->pin);
            uint64_t increment = dutycycle * pulsewidth / MAXDUTYCYCLE;
            incrementTimerCompare(increment);
        }
        else
        { //low

            MAP_GPIO_setOutputLowOnPin(pin->port, pin->pin);
            uint64_t increment = (MAXDUTYCYCLE - dutycycle) * pulsewidth
                    / MAXDUTYCYCLE;
            incrementTimerCompare(increment);
            dutycycle += dutycyclestep;
        }
        pulsecount++;
    }

    //glue logic for state change
    if (pulsecount >= MAXPULSECOUNT)
    {
        //put in its own function!!!
        //speed up motor
        int16_t delta=wantedpulsewidth-pulsewidth;
        if(delta!=0)
        {
            if(abs(delta)<=MINDELTAPULSEWIDTHSTEP)
            {
                pulsewidth=wantedpulsewidth;
            }
            else
            {
                if(delta<0)
                {
                    pulsewidth-=MINDELTAPULSEWIDTHSTEP;
                }
                else
                {
                    pulsewidth+=MINDELTAPULSEWIDTHSTEP;
                }
            }
        }
        //check limits
        if(pulsewidth<MAXPULSEWIDTH)
        {
            pulsewidth=MAXPULSEWIDTH;
        }
        else if(pulsewidth>MINPULSEWIDTH)
        {
            pulsewidth=MINPULSEWIDTH;
        }


        pulsecount = 0;
        dutycycle = INITIALDUTYCYCLE;
        dutycyclestep=DEFAULTDUTYCYCLESTEP;
        state++;
        if (state >= MAXSTATES)
        {
            state = 0;
        }
    }
}

inline void incrementTimerCompare(uint16_t increment)
{
    Timer_A_setCompareValue(TIMER_A0_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_1,
                            Timer_A_getCounterValue(TIMER_A0_BASE) + increment);
}

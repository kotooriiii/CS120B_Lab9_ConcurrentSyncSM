/*	Author: Carlos Miranda cmira039@ucr.edu
 *  Partner(s) Name: n/a
 *	Lab Section: 23
 *	Assignment: Lab #9  Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: 
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

/* Timer global variables */
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

/* Three LED State machine */
enum ThreeLEDsSM {LED3SM_INIT, LED3SM_BIT0, LED3SM_BIT1, LED3SM_BIT2} LED3SM_STATE;
unsigned char threeLEDs = 0x00; //output shared var

/* BlinkingLED State machine */
enum BlinkingLEDSM {BLEDSM_INIT, BLEDSM_BIT3_ON, BLEDSM_BIT3_OFF} BLEDSM_STATE;
unsigned char blinkingLED = 0x00;

/* CombineLED SM */
enum CombineLEDsSM {CLEDSM_INIT, CLEDSM_OUTPUT} CLEDSM_STATE;

/*  SoundSM */
enum SoundSM {SSM_INIT, SSM_ON, SSM_OFF}  SSM_STATE;
unsigned char soundState = 0x00;

/* Helper methods */
unsigned char IPINA()
{
	return ~PINA;
}

unsigned char isA2()
{
	return  IPINA() & 0x04;
}


/* Timer functions */
void TimerISR()
{
	TimerFlag = 1;
}

void TimerOff()
{
	TCCR1B = 0x00;
}

void TimerOn()
{
	TCCR1B = 0x0B;

	OCR1A = 125;

	TIMSK1 = 0x02;

	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;

	SREG |= 0x80;

}

ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0)
	{
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

/* SM tick functions */

void tickThreeLEDsSM()
{
	switch(LED3SM_STATE)
	{
		case LED3SM_INIT:
		LED3SM_STATE = LED3SM_BIT0;
		break;
		
		case LED3SM_BIT0:
		LED3SM_STATE = LED3SM_BIT1;
		break;
		
		case LED3SM_BIT1:
		LED3SM_STATE = LED3SM_BIT2;
		break;
		
		case LED3SM_BIT2:
		LED3SM_STATE = LED3SM_BIT0;
		break;
		
		default:
		LED3SM_STATE = LED3SM_INIT;
		break;
	}
	
	switch(LED3SM_STATE)
	{
		case LED3SM_INIT:
		break;
		case LED3SM_BIT0:
		threeLEDs = 0x01;
		break;
		case LED3SM_BIT1:
		threeLEDs = 0x02;
		break;
		case LED3SM_BIT2:
		threeLEDs = 0x04;
		break;
		default:
		threeLEDs = 0x00;
		break;
	}
}

void tickBlinkingLEDSM()
{
	switch(BLEDSM_STATE)
	{
		case BLEDSM_INIT:
		BLEDSM_STATE = BLEDSM_BIT3_ON;
		break;		
		
		case BLEDSM_BIT3_ON:
		BLEDSM_STATE = BLEDSM_BIT3_OFF;
		break;
		
		case BLEDSM_BIT3_OFF:
		BLEDSM_STATE = BLEDSM_BIT3_ON;
		break;
		
		default:
		BLEDSM_STATE = BLEDSM_INIT;
		break;
	}
	
	switch(BLEDSM_STATE)
	{
		case BLEDSM_INIT:
		break;		
		
		case BLEDSM_BIT3_ON:
		blinkingLED = 0x08;
		break;
		
		case BLEDSM_BIT3_OFF:
		blinkingLED = 0x00;
		break;
		
		default:
		blinkingLED = 0x00;
		break;
	}
}

void tickSoundSM()
{
	switch(SSM_STATE)
	{
		case SSM_INIT:
		if(isA2())
		{
			SSM_STATE = SSM_ON;
		}
		else
		{
			SSM_STATE = SSM_OFF;
		}
		break;
		
		case SSM_ON:
		SSM_STATE = SSM_OFF;
		break;
		
		case SSM_OFF:
		if(isA2())
		{
			SSM_STATE = SSM_ON;
		}
		else
		{
			SSM_STATE = SSM_OFF;
		}
		break;
		
		default:
		SSM_STATE = SSM_INIT;
		break;
	}
	
	switch(SSM_STATE)
	{
		case SSM_INIT:
		soundState = 0x00;
		break;
		
		case SSM_ON:
		soundState = 0x10;
		break;
		
		case SSM_OFF:
		soundState = 0x00;
		break;
		
		default:
		soundState = 0x00;
		break;
	}
}


void tickCombineLEDsSM()
{
	switch(CLEDSM_STATE)
	{
		case CLEDSM_INIT:
		CLEDSM_STATE = CLEDSM_OUTPUT;
		break;		
		
		case CLEDSM_OUTPUT:
		CLEDSM_STATE = CLEDSM_OUTPUT;
		break;
		
		default:
		break;
	}
	
	switch(CLEDSM_STATE)
	{
		case CLEDSM_INIT:
		break;		
		
		case CLEDSM_OUTPUT:
		PORTB = (blinkingLED | threeLEDs) | soundState;
		break;
		
		default:
		break;
	}
}

/* Main */
int main(void) 
{
	
	//Outputs
	DDRB = 0xFF; PORTB = 0x00;
        //Inputs	
	DDRA = 0x00; PORTA = 0xFF;

	//Different period 
	unsigned long LED3SM_elapsedTime = 300;
    unsigned long BLEDSM_elapsedTime = 1000;
	unsigned long SSM_elapsedTime = 2;
	
    const unsigned long timerPeriod = 1;
	
	//Flag
	unsigned char isUpdated = 0x00;
	
	//Init
	LED3SM_STATE = LED3SM_INIT;
	BLEDSM_STATE = BLEDSM_STATE;
	SSM_STATE = SSM_INIT;
	CLEDSM_STATE = CLEDSM_INIT;
	
	
	TimerSet(timerPeriod);
	TimerOn();
		
	while(1) 
	{
		
		if (LED3SM_elapsedTime >= 300) 
		{ // 300 ms period
			tickThreeLEDsSM(); // Execute one tick of 3LEDSM
			LED3SM_elapsedTime = 0;    
			isUpdated = 0x01;			
		}
		
		if (BLEDSM_elapsedTime >= 1000)
		{ // 1000 ms period
         	tickBlinkingLEDSM(); // Execute one tick of BLEDSM
			BLEDSM_elapsedTime = 0;
			isUpdated = 0x01;			
		}
		
		if(SSM_elapsedTime >= 2)
		{
			tickSoundSM();
			SSM_elapsedTime = 0;
			isUpdated = 0x01;
		}
		
		if(isUpdated)
		{
			tickCombineLEDsSM();
		}
		
		while(!TimerFlag);
		TimerFlag = 0;
		isUpdated = 0x00;			
		LED3SM_elapsedTime += timerPeriod;
		BLEDSM_elapsedTime += timerPeriod;
		SSM_elapsedTime += timerPeriod;
	}
	return 0;
}

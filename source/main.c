/*	Author: Jack Huang
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *	https://drive.google.com/drive/folders/1JBIqqJb-m900203LVLXI8yLaMciH493w?usp=sharing
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "timer.h"
#include <stdlib.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned short x;
int array[100];

typedef struct _task{
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime; 
	int (*TickFct)(int); 
}task;

unsigned long int findGCD (unsigned long int a, unsigned long int b) {
	unsigned long int c;
	while(1){
		c = a%b;
		if(c == 0){
			return b;
		}
		a = b;
		b = c;
	}
	return 0;
}

void ADC_init(){
	ADCSRA |= (1<<ADEN) | (1<<ADSC) | (1<<ADATE);
}

//stationary B = 0x01F
//right B = 0x1F0
//left B = 0x27 on PINC = 1
//up B = 0x18 on PINC = 2
//down B = 0x1E on PINC = 0

enum lightstates{start,display,waitdisplay,check,waitcheck,right,wrong};
int levelcounter = 1;
int scorecounter = 0;
int tempcounter = 0;
int triescounter = 0;

int lights(int state){
	x = ADC;
	PORTB = (char)x;
        PORTD = (char)(x >> 8);
	switch(state){
		case start:
			if((~PINA & 0x04) == 0x04){
				state = display;
			}
			else{
				state = start;
			}
			break;
		case display:
			if((scorecounter < 100) && (tempcounter < levelcounter)){
				state = waitdisplay;
			}
			else{
				state = check;	
			}
			break;
		case waitdisplay:
			if(tempcounter == levelcounter){
				tempcounter = 0;
				state = check;
			}
			else{
				state = display;
			}
			break;
		case check:
			PORTC = 0x00;
			x = ADC;
			unsigned char tmp = (char)x;
			if(((char)x == 0x1F) || ((char)x == 0x1E)){
				state = check;
			}
			if(array[tempcounter] == 8){
				if(tmp == 0xF0){//right
					state = waitcheck;	
				}
				else{
					state = check;
					//triescounter++;
				}
			}
			if(array[tempcounter] == 4){
				if((char)x == 0x0C){
					state = waitcheck;
				}
				else{
					state = check;
					//triescounter++;
				}
			}
			if(array[tempcounter] == 2){
				if((char)x == 0x27){//left
					state = waitcheck;
				}	
				else{
					state = check;
					//triescounter++;
				}
			}
			if(array[tempcounter] == 1){
				if((char)x == 0x18){//down
					state = waitcheck;
				}
				else{
					state = check;
					//triescounter++;
				}
			}
			if(triescounter == 10){
				state = wrong;
			}
			else{
				state = check;
			}
			break;
		case waitcheck:
			tempcounter++;
			if(tempcounter == levelcounter){
				state = right;
			}
			else{
				state = check;
			}
			break;
		case right:
			state = display;
			break;
		case wrong:
			state = display;
			break;
		default:
			state = start;
			break;
	}
	switch(state){
		case start:
			break;
		case display:
			PORTC = array[tempcounter];
			break;
		case waitdisplay:
			triescounter = 0;
			tempcounter++;
			break;
		case check:
			break;
		case waitcheck:
			break;
		case right:
			scorecounter++;
			levelcounter++;
			tempcounter = 0;
			break;
		case wrong:
			scorecounter = 0;
			levelcounter = 0;
			tempcounter = 0;
			break;
	}
	return state;
}

//int sound(int state){
//	return state;
//}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    const char start = -1;

    task1.state = start;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &lights;

    //task2.state = start;
    //task2.period = 100;
    //task2.elapsedTime = task2.period;
    //task2.TickFct = &sound;

    unsigned long GCD = tasks[0]->period;
    for (int i = 0; i < numTasks; i++){
        GCD = findGCD(GCD,tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();
    ADC_init();
 
    for(int i = 0; i < 100; i++){
	int temp = rand() % 4;
	if(temp == 0){
		temp = 1;	
	}
	else if(temp == 1){
		temp = 2;
	}
	else if(temp == 2){
		temp = 4;
	}
	else if(temp == 3){
		temp = 8;
	}
    	array[i] = temp;
    }
    /* Insert your solution below */
    while (1) {
	x = ADC;
	for (int i = 0; i < numTasks; i++){
	    if(tasks[i]->elapsedTime >= tasks[i]->period){
	        tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
		tasks[i]->elapsedTime = 0;
	    }
	    tasks[i]->elapsedTime += GCD;
	}
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}

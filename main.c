#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//Source of LED Matrix Code: http://davidegironi.blogspot.com/2013/08/avr-atmega-multiple-8x8-led-matrix.html#.XPwhe1xKjtR
#include "ledmatrix7219d88.h"
#include "ledmatrix7219d88.c"
#include "max7219.h"
#include "max7219.c"
//Purpose of the included files above is to get the LED matrix display working.
//Source of Nokia 5110 LCD Screen: https://github.com/LittleBuster/avr-nokia5110
#include "nokia5110.h"
#include "nokia5110.c"
//Purpose of the included files above is to set up the LCD screen to display
#define ADC_0 0
void ADC_Initial();
volatile unsigned char TimerFlag = 0;
enum StackState {Init, MenuSelect, OnRow1, OnRow2, OnRow3, OnRow4, OnRow5, OnRow6, OnRow7, OnRow8, GameWon, GameLost}state;
uint8_t rows[11] = {0b11100000,0b01110000,0b00111000,0b00011100,0b00001110,0b00000111,0b00001110,0b00011100,0b00111000,0b01110000,0b11100000};
	
unsigned long GameSpeed;	
unsigned char tempD;
unsigned char cnt1;
unsigned char SetRow1;
unsigned char SetRow2;
unsigned char SetRow3;
unsigned char SetRow4;
unsigned char SetRow5;
unsigned char SetRow6;
unsigned char SetRow7;
unsigned char SetRow8;
uint8_t ledmatrix = 0;

void ADC_Initial()
{
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void StackerTick()
{
	switch(state)
	{
		case Init:
		ledmatrix7219d88_resetmatrix(ledmatrix);
		nokia_lcd_clear();
		state = MenuSelect;
		break;

		//The block of nokia function calls display to the user to select a difficulty by pressing a button.
		case MenuSelect:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Select the" ,1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_write_string("difficulty" ,1);
		nokia_lcd_set_cursor(20, 20);
		nokia_lcd_write_string("Easy(PD2)", 1);
		nokia_lcd_set_cursor(20, 30);
		nokia_lcd_write_string("Normal(PD3)", 1);
		nokia_lcd_set_cursor(20, 40);
		nokia_lcd_write_string("Hard(PD4)", 1);
		nokia_lcd_render();
		//The following if else statement determines the difficulty of the game(how fast the game goes with a butotn press)
		if(tempD == 0x04)
		{
			GameSpeed =	800;
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("Easy Mode", 1);
			nokia_lcd_set_cursor(20,10);
			nokia_lcd_write_string("Stage 1", 1);
			nokia_lcd_render();
			state = OnRow1;
		}
		else if(tempD == 0x08)
		{
			GameSpeed = 550;
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("Normal Mode", 1);
			nokia_lcd_set_cursor(10,10);
			nokia_lcd_write_string("Stage 1", 1);
			nokia_lcd_render();
			state = OnRow1;
		}
		else if(tempD == 0x10)
		{
			GameSpeed = 350;
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("Hard Mode", 1);
			nokia_lcd_set_cursor(10,10);
			nokia_lcd_write_string("Stage 1", 1);
			nokia_lcd_render();
			state = OnRow1;
		}
		else
		{
			state = MenuSelect;
		}
		break;

		case OnRow1:
		ADMUX = 0x00;
		//Initializes the while loop counter
		cnt1 = 0;
		//While loop will have the LEDs move back and forth as long as the joystick is not tilted up. Note that 'cnt' will always be less than 10
		while (cnt1 < 11 && (ADC) < 600)
		{
			ledmatrix7219d88_setrow(ledmatrix , 7 , rows[cnt1]);		//This sets the row to move the LEDs back and forth
			_delay_ms(GameSpeed);										//The speed of LED's moving back and forth is adjusted with game difficulty variable 'GameSpeed'
			if((ADC) > 600)
			{
				SetRow1 = rows[cnt1];
				state = OnRow2;
			}
			if(cnt1 == 10)
			{
				cnt1 = 0;
			}
			cnt1++;
		}
		break;

		case OnRow2:
		ADMUX = 0x00;
		nokia_lcd_clear();
		nokia_lcd_set_cursor(20,10);
		nokia_lcd_write_string("Stage 2", 1);
		nokia_lcd_render();
		ADMUX = 0x00;
		//Reset the while loop counter after leaving row 1
		cnt1 = 0;
		//While loop will have the LEDs move back and forth as long as the joystick is not tilted up. Note 'cnt' will never be >= 11
		//first if else statement compares the current row and previous row (excluding the very first row) and determines whether you go up a row or lose.
		//The first two comment lines are the same for rows 3-8.
		while (cnt1 < 11 && (ADC) < 600)
		{
			ledmatrix7219d88_setrow(ledmatrix , 6 , rows[cnt1]);
			_delay_ms(GameSpeed);
			if((ADC) > 600)
			{
				SetRow2 = rows[cnt1];
				if((SetRow2 & SetRow1) == 0x07 || (SetRow2 & SetRow1) == 0x0E || (SetRow2 & SetRow1) == 0x01C || (SetRow2 & SetRow1) == 0x38 || (SetRow2 & SetRow1) == 0x70 || (SetRow2 & SetRow1) == 0xE0)
				{
					state = OnRow3;
				}
				else
				{
					state = GameLost;
				}
			}			
			if(cnt1 == 10)
			{
				cnt1 = 0;
			}
			cnt1++;
		}
		break;

		case OnRow3:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(20,10);
		nokia_lcd_write_string("Stage 3", 1);
		nokia_lcd_render();
		cnt1 = 0;
		while (cnt1 < 11 && (ADC) < 600)
		{
			ledmatrix7219d88_setrow(ledmatrix , 5 , rows[cnt1]);
			_delay_ms(GameSpeed);
			if((ADC) > 600)
			{
				SetRow3 = rows[cnt1];
				if((SetRow3 & SetRow2) == 0x07 || (SetRow3 & SetRow2) == 0x0E || (SetRow3 & SetRow2) == 0x01C || (SetRow3 & SetRow2) == 0x38 || (SetRow3 & SetRow2) == 0x70 || (SetRow3 & SetRow2) == 0xE0)
				{
					state = OnRow4;
				}
				else
				{
					state = GameLost;
				}
			}
			if(cnt1 == 10)
			{
				cnt1 = 0;
			}
			cnt1++;
		}
		break;

		case OnRow4:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(20,10);
		nokia_lcd_write_string("Stage 4", 1);
		nokia_lcd_render();
		cnt1 = 0;
		while (cnt1 < 11 && (ADC) < 600)
		{
			ledmatrix7219d88_setrow(ledmatrix , 4 , rows[cnt1]);
			_delay_ms(GameSpeed);
			if((ADC) > 600)
			{
				SetRow4 = rows[cnt1];
				if((SetRow4 & SetRow3) == 0x07 || (SetRow4 & SetRow3) == 0x0E || (SetRow4 & SetRow3) == 0x01C || (SetRow4 & SetRow3) == 0x38 || (SetRow4 & SetRow3) == 0x70 || (SetRow4 & SetRow3) == 0xE0)
				{
					state = OnRow5;
				}
				else
				{
					state = GameLost;
				}
			}
			if(cnt1 == 10)
			{
				cnt1 = 0;
			}
			cnt1++;
		}
		break;

		case OnRow5:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(20,10);
		nokia_lcd_write_string("Stage 5", 1);
		nokia_lcd_render();
		cnt1 = 0;
		while (cnt1 < 11 && (ADC) < 600)
		{
			ledmatrix7219d88_setrow(ledmatrix , 3 , rows[cnt1]);
			_delay_ms(GameSpeed);
			if((ADC) > 600)
			{
				SetRow5 = rows[cnt1];
				if((SetRow5 & SetRow4) == 0x07 || (SetRow5 & SetRow4) == 0x0E || (SetRow5 & SetRow4) == 0x01C || (SetRow5 & SetRow4) == 0x38 || (SetRow5 & SetRow4) == 0x70 || (SetRow5 & SetRow4) == 0xE0)
				{
					state = OnRow6;
				}
				else
				{
					state = GameLost;
				}
			}
			if(cnt1 == 10)
			{
				cnt1 = 0;
			}
			cnt1++;
		}
		break;

		case OnRow6:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(20,10);
		nokia_lcd_write_string("Stage 6", 1);
		nokia_lcd_render();
		cnt1 = 0;
		while (cnt1 < 11 && (ADC) < 600)
		{
			ledmatrix7219d88_setrow(ledmatrix , 2 , rows[cnt1]);
			_delay_ms(GameSpeed);
			if((ADC) > 600)
			{
				SetRow6 = rows[cnt1];
				if((SetRow6 & SetRow5) == 0x07 || (SetRow6 & SetRow5) == 0x0E || (SetRow6 & SetRow5) == 0x01C || (SetRow6 & SetRow5) == 0x38 || (SetRow6 & SetRow5) == 0x70 || (SetRow6 & SetRow5) == 0xE0)
				{
					state = OnRow7;
				}
				else
				{
					state = GameLost;
				}
			}
			if(cnt1 == 10)
			{
				cnt1 = 0;
			}
			cnt1++;
		}
		break;

		case OnRow7:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(20,10);
		nokia_lcd_write_string("Stage 7", 1);
		nokia_lcd_render();
		cnt1 = 0;
		while (cnt1 < 11 && (ADC) < 600)
		{
			ledmatrix7219d88_setrow(ledmatrix , 1 , rows[cnt1]);
			_delay_ms(GameSpeed);
			if((ADC) > 600)
			{
				SetRow7 = rows[cnt1];
				if((SetRow7 & SetRow6) == 0x07 || (SetRow7 & SetRow6) == 0x0E || (SetRow7 & SetRow6) == 0x01C || (SetRow7 & SetRow6) == 0x38 || (SetRow7 & SetRow6) == 0x70 || (SetRow7 & SetRow6) == 0xE0)
				{
					state = OnRow8;
				}
				else
				{
					state = GameLost;
				}
			}
			if(cnt1 == 10)
			{
				cnt1 = 0;
			}
			cnt1++;
		}
		break;

		case OnRow8:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(20,10);
		nokia_lcd_write_string("Stage 8", 1);
		nokia_lcd_render();
		cnt1 = 0;
		//For the first if else statement, you will either win or lose the game.
		while (cnt1 < 11 && (ADC) < 600)
		{
			ledmatrix7219d88_setrow(ledmatrix , 0 , rows[cnt1]);
			_delay_ms(GameSpeed);
			if((ADC) > 600)
			{
				SetRow8 = rows[cnt1];
				if((SetRow8 & SetRow7) == 0x07 || (SetRow8 & SetRow7) == 0x0E || (SetRow8 & SetRow7) == 0x01C || (SetRow8 & SetRow7) == 0x38 || (SetRow8 & SetRow7) == 0x70 || (SetRow8 & SetRow7) == 0xE0)
				{
					state = GameWon;
				}
				else
				{
					state = GameLost;
				}
			}
			if(cnt1 == 10)
			{
				cnt1 = 0;
			}
			cnt1++;
		}
		break;

		case GameWon:
		ledmatrix = 0;
		ledmatrix7219d88_resetmatrix(ledmatrix);
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0,0);
		nokia_lcd_write_string("You Win!",1);
		nokia_lcd_set_cursor(0,10);
		nokia_lcd_write_string("Press PD1 to",1);
		nokia_lcd_set_cursor(0,20);
		nokia_lcd_write_string("play again",1);
		nokia_lcd_render();
		switch(tempD)
		{
			case 0x02:
			state = Init;
			break;

			default:
			state = GameWon;
			break;
		}
		break;

		case GameLost:
		ledmatrix = 0;
		ledmatrix7219d88_resetmatrix(ledmatrix);
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0,0);
		nokia_lcd_write_string("You Lost!",1);
		nokia_lcd_set_cursor(0,10);
		nokia_lcd_write_string("Press PD1 to",1);
		nokia_lcd_set_cursor(0,20);
		nokia_lcd_write_string("play again",1);
		nokia_lcd_render();
		switch(tempD)
		{
			case 0x02:
			state = Init;
			break;

			default:
			state = GameLost;
			break;
		}
		break;


		default:
		state = Init;
		break;
	}

	switch(state)
	{
		case Init:
		break;

		case MenuSelect:
		break;

		case OnRow1:
		break;

		case OnRow2:
		break;

		case OnRow3:
		break;

		case OnRow4:
		break;

		case OnRow5:
		break;

		case OnRow6:
		break;

		case OnRow7:
		break;

		case OnRow8:
		break;

		case GameWon:
		break;
		
		case GameLost:
		break;

		default:
		break;
	}
}

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRD = 0x00; PORTD = 0xFF;
	state = Init;
	TimerSet(150);
	TimerOn();
	nokia_lcd_init();
	nokia_lcd_clear();
	ledmatrix = 0;
	ledmatrix7219d88_init();
	while(1)
	{
		ADC_Initial();
		tempD = ~PIND;
		StackerTick();
	}
}
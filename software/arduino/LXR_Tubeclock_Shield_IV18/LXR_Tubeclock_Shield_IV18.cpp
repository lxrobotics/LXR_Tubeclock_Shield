/**
 * @author Alexander Entinger, MSc / LXRobotics
 * @brief this module implements the multiplexed control of the digits of the VFD IV-18 tube
 * @file LXR_Tubeclock_Shield_IV18.cpp
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */
 
#include "LXR_Tubeclock_Shield_IV18.h"
#include "LXR_Tubeclock_Shield.h"

#include <avr/interrupt.h>
#include <avr/wdt.h>

// defines 
#define DP			(1<<0)
#define A			(1<<1)
#define B			(1<<2)
#define C			(1<<3)
#define D			(1<<4)
#define E			(1<<5)
#define F			(1<<6)
#define G			(1<<7)
#define	DIG_NUM_0	(A | B | C | D | E | F)
#define	DIG_NUM_1	(B | C)
#define DIG_NUM_2       (A | B | G | E | D)
#define DIG_NUM_3	(A | B | C | D | G)
#define DIG_NUM_4	(B | C | F | G)
#define DIG_NUM_5	(A | F | G | C | D)
#define DIG_NUM_6	(A | F | E | D | C | G)
#define DIG_NUM_7	(A | B | C)
#define DIG_NUM_8	(A | B | C | D | E | F | G)
#define DIG_NUM_9	(A | B | C | D | G | F)
#define DIG_BLANK	(0)
#define DIG_DEGREE      (A | B | G | F)
#define DIG_C           (A | F | E | D)
#define DIG_MINUS       (G)

typedef enum {_0 = 0, _1 = 1, _2 = 2, _3 = 3, _4 = 4, _5 = 5, _6 = 6, _7 = 7, _8 = 8, _9 = 9, _BLANK = 10, _DEGREE = 11, _C = 12, _MINUS=13} E_DIGIT_VALUE;

// global variables
static volatile uint8_t const digit_map[] = {
	DIG_NUM_0, DIG_NUM_1, DIG_NUM_2, DIG_NUM_3, DIG_NUM_4, DIG_NUM_5, DIG_NUM_6, DIG_NUM_7, DIG_NUM_8, DIG_NUM_9, DIG_BLANK, DIG_DEGREE, DIG_C, DIG_MINUS
	}; 
	
static volatile s_segment_map map;

static volatile uint8_t display_content[DISPLAY_SIZE] = {_BLANK};

/**
 * @brief this function initializes the iv 18 module
 */
void LXR_Tubeclock_Shield_IV18::begin() {
	// init the tube clock shield relevant functions
	LXR_Tubeclock_Shield::set_tube_voltage(35);
	LXR_Tubeclock_Shield::enable_tube_voltage();
	LXR_Tubeclock_Shield::enable_vfd_controller();
	map.all = 0;
	LXR_Tubeclock_Shield::vfd_controller_write(map);
	LXR_Tubeclock_Shield::vfd_controller_load();
	LXR_Tubeclock_Shield::vfd_controller_blank_off();	
	
	// init timer 2 for multiplexing
	TCCR2A = 0; // make sure this reg is empty
	TCNT2 = 0; // reset timer
	TIMSK2 |= (1<<TOIE2); // enable timer 2 overflow interrupt
	// prescaler = 128, fMultiplex = 490 Hz
	TCCR2B = (1<<CS22) | (1<<CS20);	// run timer
}

/** 
 * @brief this function sets a digit of the iv 18 module
 * @param digit_num number of the digit to be set
 * @param val value for the digit to display
 */
void iv18_set_digit(uint8_t const digit_num, uint8_t const val) {
	map.all = 0;
	
	if(digit_map[val] & DP) map.segment.segOUT4 = 1; else map.segment.segOUT4 = 0;
	if(digit_map[val] & A) map.segment.segOUT17 = 1; else map.segment.segOUT17 = 0;
	if(digit_map[val] & B) map.segment.segOUT19 = 1; else map.segment.segOUT19 = 0;
	if(digit_map[val] & C) map.segment.segOUT2 = 1; else map.segment.segOUT2 = 0;
	if(digit_map[val] & D) map.segment.segOUT3 = 1; else map.segment.segOUT3 = 0;
	if(digit_map[val] & E) map.segment.segOUT1 = 1; else map.segment.segOUT1 = 0;
	if(digit_map[val] & F) map.segment.segOUT18 = 1; else map.segment.segOUT18 = 0;
	if(digit_map[val] & G) map.segment.segOUT0 = 1; else map.segment.segOUT0 = 0;
	
	switch(digit_num) {
		case 0: map.segment.segOUT15 = 1; break;
		case 1: map.segment.segOUT8 = 1; break;
		case 2: map.segment.segOUT14 = 1; break;
		case 3: map.segment.segOUT9 = 1; break;
		case 4: map.segment.segOUT13 = 1; break;
		case 5: map.segment.segOUT10 = 1; break;
		case 6: map.segment.segOUT11 = 1; break;
		case 7: map.segment.segOUT12 = 1; break;
		default: break;		
	}
	
	LXR_Tubeclock_Shield::vfd_controller_write(map);
	LXR_Tubeclock_Shield::vfd_controller_load();
}

/**
 * @brief writes a string to the display, user has to ensure that the string is 8 byte long
 * @param data pointer to that string
 */
void LXR_Tubeclock_Shield_IV18::write_string(uint8_t const * const data) {
	for(uint8_t i=0; i < DISPLAY_SIZE; i++) {
		if(data[i] >= '0' && data[i] <= '9') display_content[DISPLAY_SIZE - i - 1] = data[i] - '0';
                else if(data[i] == '*') display_content[DISPLAY_SIZE - i - 1] = _DEGREE;
                else if(data[i] == 'C') display_content[DISPLAY_SIZE - i - 1] = _C;
                else if(data[i] == '-') display_content[DISPLAY_SIZE - i - 1] = _MINUS;
		else display_content[DISPLAY_SIZE - i - 1] = _BLANK;		
	}
}

static volatile uint8_t current_digit = 0;
/** 
 * @brief timer 2 overflow isr : we use it to achieve time multiplexed control of the display
 */
ISR(TIMER2_OVF_vect) {
	iv18_set_digit(current_digit, display_content[current_digit]);
	if(current_digit == (DISPLAY_SIZE-1)) current_digit = 0;
	else current_digit++;	
}	

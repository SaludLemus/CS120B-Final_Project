/*
 * main_driver.c
 *
 * This .c file contains the main driver components for the
 * project
 *
 * Created: 2/26/2019 10:15:10 AM
 * Author : Salud Lemus
 */

#include <avr/io.h>
#include "joystick.h"

int main() {
	DDRA = 0x00; PORTA = 0xFF; // PA is input -> set to 1
	DDRB = 0xFF; PORTB = 0x00; // PB is output -> set to 0
	DDRD = 0xFF; PORTD = 0x00; // PD is output -> set to 0
	
	ADC_init(); // set ADC

	while (1) {
		joystick_position = getJoystickInput(); // get current position
		
		switch (joystick_position) { // determine position
			case UP: PORTB = 1; break;
			case DOWN: PORTB = 2; break;
			case LEFT: PORTB = 4; break;
			case RIGHT: PORTB = 8; break;
			default: PORTB = 0; break;
		}
	}
}


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
#include <avr/interrupt.h>
#include "timer.h"
#include "joystick.h"
#include "led_matrix_display.h"

unsigned char player_pos_row = 0;
unsigned char player_pos_col = 0;

int main() {
	 DDRD = 0xFF; PORTD = 0x00; // PD is output
	 DDRA = 0xF0; PORTA = 0x0F; // PA is output
	 DDRB = 0xFF; PORTB = 0x00;
	 
	 ADC_init();
	 
	 TimerSet(1); // 500 ms --> 1/2 a second
	 TimerOn();
	 
	 unsigned char joystick_timer = 50;
	 while (1) {
		 if (joystick_timer >= 50) {
			 joystick_position = getJoystickInput();
			 game_map[player_pos_row][player_pos_col] = 1;
			 switch (joystick_position) {
				 case UP:
					if (player_pos_row > 0) {--player_pos_row;}
					break;
				 case DOWN:
					if (player_pos_row < 7) {++player_pos_row;}
					break;
				 case LEFT:
					if (player_pos_col > 0) {--player_pos_col;}
					break;
				 case RIGHT:
					if (player_pos_col < 7) {++player_pos_col;}
					break;
				 default:
					break;
			 }
			 game_map[player_pos_row][player_pos_col] = 0;
			 joystick_timer = 0;
		 }
		 displayLEDMatrix();
		 while (!TimerFlag) {}
		 TimerFlag = 0; // start over
		 joystick_timer += 1;
		 //PORTB = 0;
	 }
}


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

typedef struct _task {
	unsigned char state; // current state in SM
	unsigned long int period; // task's period to tick
	unsigned long int elapsedTime; // how long has passed
	int (*TickFct)(int); // tick function
} task;

enum GameMenuStates {GameMenuInit, GameMenuOff, GameMenuPlay,
					 GameMenuEndGame, GameMenuWait} game_menu_state;

/*
	game_on:
		if 1 -> the game is still playing
		if 0 -> the game has finished or not started
*/
unsigned char game_on; // whether the game is ON/OFF

/*
	game_over: hit detection SM sets it
		if 1 -> the game has finished
		if 0 -> the game is still playing
*/
unsigned char game_over; // whether the game is over				

/*
	end_game: end game SM sets it
		if 1 -> end game is still going on
		if 0 -> game has not finished
*/
unsigned char end_game;

int TickFunct_GameMenu(int state) {
	
	switch (state) { // state transitions
		case GameMenuInit:
			state = GameMenuOff; // the game is off
			game_on = 0;
			break;
			
		case GameMenuOff:
			// PA2 --> start button
			if (~PINA & 0x04) { // start button pressed
				state = GameMenuPlay;
				game_on = 1; // start playing the game
			}
			else if (!(~PINA & 0x04)) { // still not pressed
				state = GameMenuOff;
			}
			break;
			
		case GameMenuPlay:
			// continue playing until game_over is set
			if (!game_over) {
				state = GameMenuPlay;
			}
			else if (game_over) {
				state = GameMenuEndGame; // go to the end game
				game_on = 0; // stop all activity
				PORTA |= 0x0F; // set least sig. bits in case of transmit_data() called
			}
			break;
			
		case GameMenuEndGame:
			// still displaying player's score and highest score if applicable
			if (end_game) {
				state = GameMenuEndGame;
			}
			else if (!end_game) { // start new game
				state= GameMenuWait; // wait reset button press
			}
			break;
			
		case GameMenuWait:
			// PA3 --> reset button
			if (~PINA & 0x08) { // reset button pressed
				state = GameMenuPlay;
				game_on = 1;
			}
			else if (!(~PINA & 0x08)) { // still not press reset button
				state = GameMenuWait;
			}
			break;
			
		default:
			state = GameMenuInit;
			break;
	}
	
	switch (state) { // state actions
		case GameMenuInit: break;
		case GameMenuOff: break;
		case GameMenuPlay: break;
		case GameMenuEndGame: break;
		case GameMenuWait: break;
		default: break;
	}
	return state;
}

enum DisplayStates {DisplayInit, DisplayOff, DisplayOn} display_state;
	
int TickFunct_Display(int state) {
	unsigned char row = 0;
	unsigned char column = 0;
	
	switch (state) { // state transitions
		case DisplayInit:
			state = DisplayOff; // do not display to LED matrix
			break;
		case DisplayOff:
			if (!game_on) { // not pressed start button
				state = DisplayOff;
			}
			else if (game_on) {
				state = DisplayOn;
				player_pos_row = 6; // set start position for the player
				player_pos_col = 4;
				
				// iterate through game_map and set map
				for (row = 0; row < 8; ++row) { // iterate through rows
					for (column = 0; column < 8; ++column) { // for each row, iterate its columns
						if (column == player_pos_col && row == player_pos_row) { // starting position matched
							game_map[row][column] = 3; // turn on [row][column] led (3 is for blue)
						}
						else { // not a start position
							game_map[row][column] = 0; // turn off [row][column] led (0 is for led off)
						}
					}
				}
			}
			break;
		case DisplayOn:
			if (game_on) { // game is still going on --> display LED matrix
				state = DisplayOn;
			}
			else if (!game_on) { // game over --> turn off LED matrix
				state = DisplayOff;
				
				// iterate through game_map and turn map off
				for (row = 0; row < 8; ++row) { // iterate through rows
					for (column = 0; column < 8; ++column) { // for each row, iterate its columns
						game_map[row][column] = 0; // turn off [row][column] led
					}
				}
				
				displayLEDMatrix(); // turn all the leds off
			}
			break;
	}
	
	switch (state) { // state actions
		case DisplayInit:
			break;
		case DisplayOff:
			break;
		case DisplayOn:
			displayLEDMatrix(); // display current game status
			break;
	}
	
	return state;
}

int main() {
	 DDRD = 0xFF; PORTD = 0x00; // PD is output
	 DDRA = 0xF0; PORTA = 0x0F; // PA is output
	 DDRB = 0xFF; PORTB = 0x00;
	 
	 ADC_init();
	 
	 TimerSet(1); // 1ms
	 TimerOn();
	 
	 unsigned char joystick_timer = 50;
	 while (1) {
		 if (joystick_timer >= 50) {
			 joystick_position = getJoystickInput();
			 game_map[player_pos_row][player_pos_col] = 0;
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
			 game_map[player_pos_row][player_pos_col] = 3;
			 joystick_timer = 0;
		 }
		 displayLEDMatrix();
		/*transmit_data(0, 0);
		transmit_data(0xFE, 1);
		transmit_data(0x7F, 3);
		transmit_data(0x01, 0);
		 while (!TimerFlag) {}
		 TimerFlag = 0; // start over
		 transmit_data(0, 0);
		  transmit_data(0xFE, 3);
		  transmit_data(0x7F, 1);
		 transmit_data(0x80, 0);*/
		 while (!TimerFlag) {}
		 TimerFlag = 0; // start over
		 joystick_timer += 1;
		 //PORTB = 0;
	 }
}


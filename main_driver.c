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
#include <stdlib.h> // used for generating random numbers
#include "io.c"
#include "timer.h"
#include "joystick.h"
#include "led_matrix_display.h"

typedef struct _task {
	unsigned char state; // current state in SM
	unsigned long int* period; // task's period to tick
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
			// PA2 --> start/reset button
			if (~PINA & 0x04) { // button pressed
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
			// PA2 --> start/reset button
			if (~PINA & 0x04) { // button pressed
				state = GameMenuPlay;
				game_on = 1;
			}
			else if (!(~PINA & 0x04)) { // still not press button
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
		default:
			state = DisplayInit;
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

enum PlayerStates {PlayerInit, PlayerOff, PlayerMove} player_state;

int TickFunct_Player(int state) {
	
	switch (state) { // state transition
		case PlayerInit:
			state = PlayerOff;
			break;
		case PlayerOff:
			if (game_on) { // game is starting
				state = PlayerMove;
				player_pos_row = 6; // set player position
				player_pos_col = 4;
			}
			else if (!game_on) { // game has not started yet
				state = PlayerOff;
			}
			break;
		case PlayerMove:
			if (game_on) { // game is still on --> continue to get joystick input
				state = PlayerMove;
				
				joystick_position = getJoystickInput(); // get current player input
				game_map[player_pos_row][player_pos_col] = 0; // turn off old position
				switch (joystick_position) {
					case UP: // moved up
						if (player_pos_row > 0) {--player_pos_row;}
						break;
					case DOWN: // moved down
						if (player_pos_row < 7) {++player_pos_row;}
						break;
					case LEFT: // moved left
						if (player_pos_col > 0) {--player_pos_col;}
						break;
					case RIGHT: // moved right
						if (player_pos_col < 7) {++player_pos_col;}
						break;
					default: // did not move
						break;
				}
				game_map[player_pos_row][player_pos_col] = 3; // turn new position to blue
				/* ---------------SUGGESTION-----------------
				// might add if statement to check if player's position is equal to [row][!open_column} of the enemy
				//			- i.e. player_pos_row == enemy_row_num && player_pos_col != open_column
				//	if so, update game_over flag
				*/
			}
			else if (!game_on) { // player lost the game
				state = PlayerOff;
				player_pos_col = 0; // make player's position 0
				player_pos_row = 0;
			}
			break;
		default:
			state = PlayerInit;
			break;
	}
	
	switch (state) { // state actions
		case PlayerInit:
			break;
		case PlayerOff:
			break;
		case PlayerMove:
			break;
		default:
			break;
	}
	
	return state;
}

enum EnemyStates {EnemyInit, EnemyOff, EnemyMove} enemy_state;

unsigned char enemy_row[1][8];
/*
	enemy_current_row --> current row to display red
		- the first time game_on is 1 --> generate random numbers, only one column can be off
*/

unsigned char is_column_open; // used to determine whether an open column has been filled already
signed char enemy_row_num; // current row number of the enemy
signed char open_column; // current open column of the current row
unsigned char is_scored; // whether the player passed the enemy row

/*
	is _score is 0 before the first enemy is created and before a new enemy is created
*/

void generateNewEnemy(); // generates new enemy by assigning new values to the 2D array
/*
	function updates is_column_open, open_column, and enemy_row
*/

int TickFunct_Enemy(int state) {
	unsigned char i;
	
	switch (state) { // state transitions
		case EnemyInit:
			state = EnemyOff;
			
			enemy_row_num = -1;
			is_column_open = 0;
			open_column = -1;
			is_scored = 0;
			break;
		case EnemyOff:
			if (game_on) { // game is starting
				state = EnemyMove;
				
				is_scored = 0;
				generateNewEnemy(); // create new enemy
			}
			else if (!game_on) { // game is still not on
				state = EnemyOff;
			}
			break;
		case EnemyMove:
			if (game_on) { // the game is still going on
				state = EnemyMove;
				
				if (enemy_row_num < 7) { // move down one row
					// current row is 0 or 1 or 2 or ... or 6
					if (enemy_row_num >= 0) { // clear the current row in map --> assign it 0
						for (i = 0; i < 8; ++i) { // iterate through current row and update columns
							game_map[enemy_row_num][i] = 0;
						}
					}
					
					++enemy_row_num; // move to the next row
					
					// update the map with the new row --> get values from enemy_row 2D array
					for (i = 0; i < 8; ++i) { // iterate through current row's columns
						game_map[enemy_row_num][i] = enemy_row[0][i];
					}
					/* ---------------SUGGESTION-----------------
					// might add if statement to check if player's position is equal to current [row][column] of the enemy
					//			- i.e. player_pos_row == enemy_row_num && player_pos_col != open_column
					//	if so, update game_over flag
					*/
				}
				else if (!(enemy_row_num < 7)) {
					 // enemy is on the last row --> "delete it" and "replace it" with a new enemy (i.e. start over)
					 for (i = 0; i < 8; ++i) { // delete current row's columns
						 game_map[enemy_row_num][i] = 0; // assign 0 --> do not turn led on
					 }
					 
					 enemy_row_num = -1; // go back to row -1
					 is_column_open = 0; // reset bool var
					 open_column = -1; // get new open column
					 is_scored = 0; // reset
					 generateNewEnemy(); // generate new enemy
				}
			}
			else if (!game_on) { // game is over --> reset local variables
				state = EnemyOff;
				is_column_open = 0;
				open_column = -1;
				enemy_row_num = -1;
				is_scored = 0;
			}
			break;
		default:
			state = EnemyInit;
			break;
	}
	
	switch (state) { // state actions
		case EnemyInit:
			break;
		case EnemyOff:
			break;
		case EnemyMove:
			break;
		default:
			break;
	}
	
	return state;
}

void generateNewEnemy() {
	unsigned char i;
	unsigned short random_num;
	
	for (i = 0; i < 8; ++i) { // iterate through columns and fill them with 0 or 1
		random_num = ((unsigned short)(rand())) % 2; // generate random number
		
		if ((random_num == 0) && !is_column_open) { // found open column for enemy row
			enemy_row[0][i] = 0; // do not turn on current column
			is_column_open = 1; // found open column
			open_column = i; // open column is column i
		}
		else if (i == 7 && !is_column_open) { // make the last column not light up if not achieved before
			is_column_open = 1;
			enemy_row[0][i] = 0;
			open_column = i;
		}
		else { // either the open column has been filled or result is 1 after %
			enemy_row[0][i] = 1; // make current column 1 (1 is for red led)s
		}
	}
	return;
}

enum HitDetectionStates {HitDectectionInit, HitDetectionOff, HitDetectionOn} hit_detection_state;

int TickFunct_HitDetection(int state) {
	
	switch (state) { // state transitions
		case HitDectectionInit:
			state = HitDetectionOff;
			game_over = 0;
			break;
		case HitDetectionOff:
			if (game_on) { // the game is about to start
				state = HitDetectionOn;
				game_over = 0;
			}
			else if (!game_on) { // the user still has not pressed the button
				state = HitDetectionOff;
			}
			break;
		case HitDetectionOn:
			if (game_on) { // the game is on
				state = HitDetectionOn;
				
				// detect when the player's row is the same as the enemy and not the same column
				// as the open column (the led that is not on)
				if ((player_pos_row == enemy_row_num) && (player_pos_col != open_column)) {
					game_over = 1;
				}
			}
			else if (!game_on) { // the game is over
				state = HitDetectionOff;
			}
			break;
		default:
			state = HitDectectionInit;
			break;
	}
	
	switch (state) { // state actions
		case HitDectectionInit:
			break;
		case HitDetectionOff:
			break;
		case HitDetectionOn:
			break;
		default:
			break;
	}
	
	return state;
}

enum UpdateScoreStates {UpdateScoreInit, UpdateScoreOff, UpdateScoreOn} update_score_state;

unsigned char player_score; // player's score throughout the game
unsigned long int enemy_period; // a period for the enemy that moves faster as enemy_period is smaller

int TickFunct_UpdateScore(int state) {
	
	switch (state) { // state transitions
		case UpdateScoreInit:
			state = UpdateScoreOff;
			player_score = 0;
			enemy_period = 1500;
			break;
		case UpdateScoreOff:
			if (game_on) { // the game is about to start
				state = UpdateScoreOn;
				player_score = 0;
				enemy_period = 1500; // the enemy moves (i.e. ticks) every 1 and a half seconds
			}
			else if (!game_on) { // the user still has not pressed the button
				state = UpdateScoreOff;
			}
			break;
		case UpdateScoreOn:
			if (game_on) { // the game is still going on
				state = UpdateScoreOn;
				
				// update score when the player's row is greater than the enemy's row
				// and have not updated the score for the current enemy
				if (!is_scored && (player_pos_row > enemy_row_num)) {
					++player_score; // add one to current score
					is_scored = 1; // not update anymore until new enemy is generated
					
					if (enemy_period > 200) { // make enemy move faster (i.e. tick faster)
						enemy_period -= 150;	
					}
				}
			}
			else if (!game_on) { // the game is over
				state = UpdateScoreOff;
			}
			break;
		default:
			state = UpdateScoreInit;
			break;
	}
	
	switch (state) { // state actions
		case UpdateScoreInit:
			break;
		case UpdateScoreOff:
			break;
		case UpdateScoreOn:
			break;
		default:
			break;
	}
	
	return state;
}

enum EndGameStates {EndGameInit, EndGameOff, EndGamePlaying,
					EndGameEndGame, EndGameGameOver, EndGamePlayerScore,
					EndGameHighScore} end_game_state;

int TickFunct_Endgame(int state) {
	
	switch (state) {
		case EndGameInit:
			state = EndGameOff;
			break;
		case EndGameOff:
		
			break;
		case EndGamePlaying:
		
			break;
		case EndGameEndGame:
		
			break;
		case EndGameGameOver:
		
			break;
		case EndGamePlayerScore:
			
			break;
		case EndGameHighScore:
		
			break;
		default:
			state = EndGameInit;
			break;
	}
	
	switch (state) {
		case EndGameInit:
			break;
		case EndGameOff:
		
			break;
		case EndGamePlaying:
		
			break;
		case EndGameEndGame:
		
			break;
		case EndGameGameOver:
		
			break;
		case EndGamePlayerScore:
		
			break;
		case EndGameHighScore:
		
			break;
		default:
			break;
	}
	
	return state;
}

int main() {
	 DDRD = 0xFF; PORTD = 0x00; // PD is output (shift register handles red leds)
	 DDRA = 0xF0; PORTA = 0x0F; // PA is output
	 DDRB = 0xFF; PORTB = 0x00; // PB is output (shift register handles blue leds)
	 
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


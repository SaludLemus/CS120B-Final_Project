#ifndef __LED_MATRIX_DISPLAY_H__
#define __LED_MATRIX_DISPLAY_H__

/*
	player_pos_row --> current row
	player_pos_col --> current column
*/
unsigned char player_pos_row = 6;
unsigned char player_pos_col = 4;

// game map for the game
unsigned char game_map[8][8] =
{
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,3,0,0,0},
	{0,0,0,0,0,0,0,0}
};

void transmit_data(unsigned char, unsigned char);

void displayLEDMatrix() {
	unsigned char column = 0; unsigned char row = 0;
	unsigned char current_column;
	unsigned char blue_leds, red_leds;
	for (column = 0; column < 8; ++column) { // columns -> from col 1 to col 8
		current_column =  (0x01 << column); // goes from left to right on 2D array
		
		transmit_data(0, 0); // disable all columns
		
		blue_leds = red_leds = 0; // get blue and red values for the current column
		
		for (row = 0; row < 8; ++row) { // data to send to led matrix --> rows
			// [column] is constant --> current column
			// [row] is not constant --> iterate through rows
			
			unsigned char current_value = game_map[row][column]; // get current value in 2D map
			
			if (current_value == 0) { // 0 value in [row][column]
				blue_leds |= (1 << row); // set to 1 --> do not turn on the blue led
				red_leds |= (1 << row); // set to 1 --> do not turn on the red led
			}
			
			else if (current_value == 1) { // red led in [row][column]
				blue_leds |= (1 << row); // set to 1 --> do not turn on the blue led
				red_leds |= (0 << row); // set to 0 --> turn on the red led
			}
			
			else if (current_value == 3) { // blue led in [row][column]
				blue_leds |= (0 << row); // set to 0 --> turn on the blue led
				red_leds |= (1 << row); // set to 1 --> do not turn on the red led
			}
			
			else if (current_value == 4) { // game over value in [row][column]
				blue_leds |= (0 << row); // set to 0 --> turn on the blue led
				red_leds |= (0 << row); // set to 0 --> turn on the red led
			}
		}
		
		transmit_data(blue_leds, 3); // display blue leds for current column
		transmit_data(red_leds, 1); // display red leds for current column
		
		transmit_data(current_column, 0); // turn ON current column
	}
	//PORTB = 0;
	return;
}

/*
t_output:
	if t_output is set to 0, then we are turning ON a column (i.e. modify PORTA for the columns)
	if t_output is set to 1, then we are turning ON the red LEDs for that column (i.e. modify PORTD for the red LEDs)
	if t_output is set to 3, then we are turning ON the blue LEDs for that column (i.e. modify PORTB for the blue LEDs)
*/
void transmit_data(unsigned char data, unsigned char t_output) {
	signed char k = 0;
	unsigned char j = 0;
	for (k = 7; k >= 0; --k) { // transfer each data bit to the shift register
		
		/* PD0 = SER (red led); PD1 = RCLK; PD2 = SRCLK; PD3 = SRCLR
		// PB0 = SER (blue led); PB1 = RCLK; PB2 = SRCLK; PB3 = SRCLR
		// PA4 = SER; PA5 = RCLK; PA6 = SRCLK; PA7 = SRCLR
		*/
		
		switch (t_output) { // set SRCLR to high (1) and SRCLK to low (0)
			case 0: j = PORTA & 0x0F; PORTA = 0x80; break;
			case 1: PORTD = 0x08; break;
			case 3: PORTB = 0x08; break;
			default: break;
		}
		
		switch (t_output) { // set SER to current bit of data (1 or 0)
			case 0: PORTA |= (((data >> k) << 4) & 0x10); break;
			case 1: PORTD |= ((data >> k) & 0x01); break;
			case 3: PORTB |= ((data >> k) & 0x01); break;
			default: break;
		}
		
		switch (t_output) { // set SRCLK to high (1)
			case 0: PORTA |= 0x40; break;
			case 1: PORTD |= 0x04; break;
			case 3: PORTB |= 0x04; break;
			default: break;
		}
	}
	switch (t_output) { // set RCLK to high (1)
		case 0: PORTA |= 0x20; break;
		case 1: PORTD |= 0x02; break;
		case 3: PORTB |= 0x02; break;
		default: break;
	}
	
	switch (t_output) { // clear port for next data
		case 0: PORTA = j; break;
		case 1: PORTD = 0; break;
		case 3: PORTB = 0; break;
		default: break;
	}
	return;
}

#endif /* __LED_DISPLAY_H__ */
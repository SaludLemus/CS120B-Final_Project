#ifndef __LED_MATRIX_DISPLAY_H__
#define __LED_MATRIX_DISPLAY_H__

// game map for the game
unsigned char game_map[8][8] =
{
	{0,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1}
};

void transmit_data(unsigned char, unsigned char);

void displayLEDMatrix() {
	unsigned char column = 0; unsigned char row = 0;
	unsigned char current_column, row_values;
	
	for (column = 0; column < 8; ++column) { // columns -> from col 1 to col 8
		current_column =  (0x01 << column); // goes from left to right on 2D array
		
		transmit_data(0, 0); // disable all columns
		
		row_values = 0; // get current row values
		
		for (row = 0; row < 8; ++row) { // data to send --> rows
			// [column] is constant --> current column
			// [row] is not constant --> iterate through rows
			row_values |= (game_map[row][column] << row);
		}
		
		transmit_data(row_values, 1); // send current row data
		
		transmit_data(current_column, 0); // turn ON current column
		
	}
	//PORTB = 0;
	return;
}

/*
t_output:
	if t_output is set to 0, then we are turning ON a column (i.e. modify PORTD for the columns)
	if t_output is set to 1, then we are turning ON the blue LEDs for that column (i.e. modify PORTA for the blue LEDs)
*/
void transmit_data(unsigned char data, unsigned char t_output) {
	signed char k = 0;
	unsigned char j = 0;
	for (k = 7; k >= 0; --k) { // transfer each data bit to the shift register
		// PD0 = SER; PD1 = RCLK; PD2 = SRCLK; PD3 = SRCLR
		// PA4 = SER; PA5 = RCLK; PA6 = SRCLK; PA7 = SRCLR
		
		if (t_output == 1) {
    		PORTD = 0x08; // set SRCLR to high (1) and SRCLK to low (0)
		}
		else {
			j = PORTA & 0x0F;
			PORTA = 0x80;
		}
		
		if (t_output == 1) {
			PORTD |= ((data >> k) & 0x01); // set SER to current bit of data (1 or 0)
		}
		else {
			PORTA |= (((data >> k) << 4) & 0x10);
		}
		
		if (t_output == 1) {
			PORTD |= 0x04; // set SRCLK to high (1)
		}
		else {
			PORTA |= 0x40;
		}
	}
	
	if (t_output == 1) {
		PORTD |= 0x02; // set RCLK to high (1)
	}
	else {
		PORTA |= 0x20;
	}
	
	if (t_output == 1) {
		PORTD = 0; // clear port for next data
	}
	else {
		//PORTA = 0;
		PORTA = j;
	}
	return;
}

#endif /* __LED_DISPLAY_H__ */
/*
 * test_sr.c
 *
 * Created: 3/2/2019 3:15:21 PM
 * Author : salud
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
volatile unsigned char TimerFlag = 0x00; // TimerISR() sets it to 1 -> clear to 0 by programmer
unsigned long _avr_timer_M = 1; // start from here and countdown to 0
unsigned long _avr_timer_cntcurr = 0; // internal count of 1ms ticks

// add sync. function calls
void TimerOn() {
    // AVR timer/counter controller register TCCR1
    TCCR1B = 0x0B;  // bit3 = 0: CTC mode (clear timer on compare)
    // bit2bit1bit0=011: pre-scaler /64
    // 00001011: 0x0B
    // SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
    // Thus, TCNT1 register will count at 125,000 ticks/s

    // AVR output compare register OCR1A
    OCR1A = 125;    // Timer interrupt will be generated when TCNT1 == OCR1A
    // We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
    // So when TCNT1 register equals 125,
    // 1 ms has passed. Thus, we compare to 125.
    
    // AVR timer interrupt mask register
    TIMSK1 = 0x02; // bit1 : OCIE1A  -- enables compare match interrupt
    
    // intialize avr counter
    TCNT1 = 0;
    
    _avr_timer_cntcurr = _avr_timer_M;
    // TimeISR will be called every _avr_timer_cntcurr ms
    
    // enable global interrupts
    SREG |= 0x80; // 0x80: 100 0000
    return;
}

void TimerOff() {
    TCCR1B = 0x00; // bit3bit1bit0 = 000: timer off
    return;
}

void TimeISR() {
    TimerFlag = 1;
    return;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
    // cpu automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
    _avr_timer_cntcurr--; // countdown to 0
    
    if (_avr_timer_cntcurr == 0) { // more efficient compare
        TimeISR(); // call ISR -> set TimerFlag to 1
        _avr_timer_cntcurr = _avr_timer_M; // start countdown all over
    }
}

void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
    return;
}

void transmit_data(unsigned char, unsigned char);

// copied from internet
// TODO: [INSERT LINK]
void ADC_init() {
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// also copied from internet
// TODO: [INSERT SAME LINK]
uint16_t adc_read(uint8_t ch) {
	// select the corresponding channel 0~7
	// ANDing with ’7? will always keep the value
	// of ‘ch’ between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write ’1? to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes ’0? again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

/* enum is to determine which position the joystick is at */
enum JOYSTICK_POSITIONS {UP, DOWN, LEFT, RIGHT, NONE};

enum JOYSTICK_POSITIONS joystick_position = NONE;

enum JOYSTICK_POSITIONS getJoystickInput() {
	unsigned short value_x = 0;
	unsigned short value_y = 0;
	PORTA |= 0x0F;
	value_y = adc_read(0) / 8;
	value_x = adc_read(1) / 8;
	PORTB = value_y;
	if ((unsigned char)value_y >= 20 && (unsigned char)value_y <= 28 && (unsigned char)value_x < 70
	&& (unsigned char)value_x >= 64) { // up
		return UP;
	}
	else if ((unsigned char)value_y <= 12 && (unsigned char)value_x < 70
	&& (unsigned char)value_x >= 64) { // down
		return DOWN;
	}
	else if((unsigned char)value_x <= 15 && (unsigned char)value_x >= 1
	&& (unsigned char)value_y >= 12 && (unsigned char)value_y < 20) { // left
		return LEFT;
	}
	else if ((unsigned char)value_x > 120 && (unsigned char)value_x <= 130
	&& (unsigned char)value_y >= 12 && (unsigned char)value_y < 20) { // right
		return RIGHT;
	}
	return NONE;
}

char test_map[8][8] =
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

unsigned char player_pos_row = 0;
unsigned char player_pos_col = 0;

void stuff_1();
int main(void)
{
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
			test_map[player_pos_row][player_pos_col] = 1;
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
			test_map[player_pos_row][player_pos_col] = 0;
			joystick_timer = 0;
		}
		stuff_1();
		while (!TimerFlag) {}
		TimerFlag = 0; // start over
		joystick_timer += 1;
		//PORTB = 0;
    }
}

void stuff_1() {
	unsigned char column = 0; unsigned char row = 0;
	unsigned char current_column, row_values;
	
	for (column = 0; column < 8; ++column) { // columns -> from col 1 to col 8
		current_column =  (0x01 << column); // goes from left to right on 2D array
		
		transmit_data(0, 0); // disable all columns
		
		row_values = 0; // get current row values
		
		for (row = 0; row < 8; ++row) { // data to send --> rows
			// [column] is constant --> current column
			// [row] is not constant --> iterate through rows
			row_values |= (test_map[row][column] << row);
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


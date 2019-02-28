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

void ADC_init() {
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

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

int main() {
	DDRA = 0x00; PORTA = 0xFF; // PA is input -> set to 1
	DDRB = 0xFF; PORTB = 0x00; // PB is output -> set to 0
	DDRD = 0xFF; PORTD = 0x00; // PD is output -> set to 0
	ADC_init(); // set ADC
	
	unsigned short value_x = 0;
	unsigned short value_y = 0;
	
	while (1) {
		value_y = adc_read(0);
		value_x = adc_read(1) / 8;
		//PORTB = (unsigned char) value_x;
		
		if ((unsigned char)value_y > 200 && (unsigned char)value_x < 70 && (unsigned char)value_x >= 64) { // up
			PORTB = 1;
		}
		else if ((unsigned char)value_y < 20 && (unsigned char)value_x < 70 && (unsigned char)value_x >= 64) { // down
			PORTB = 2;
		}
		else if ((unsigned char)value_x > 120 && (unsigned char)value_x <= 130 && (unsigned char)value_y < 140 && (unsigned char)value_y >= 128) { // right
			PORTB = 4;
		}
		else if((unsigned char)value_x <= 22 && (unsigned char)value_x >= 2 && (unsigned char)value_y >= 128 && (unsigned char)value_y < 140) { // left
			PORTB = 8;
		}
		else {
			PORTB = 0;
		}
	}
}


#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

// copied from internet
// LINK: http://maxembedded.com/2011/06/the-adc-of-the-avr/
void ADC_init() {
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// also copied from internet
// LINK: http://maxembedded.com/2011/06/the-adc-of-the-avr/
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
	
	value_y = adc_read(0);
	value_x = adc_read(1) / 8;
	//PORTB = value_x;
	if ((unsigned char)value_y > 200 && (unsigned char)value_x < 70
		&& (unsigned char)value_x >= 64) { // up
		return UP;
	}
	else if ((unsigned char)value_y < 20 && (unsigned char)value_x < 70
		 && (unsigned char)value_x >= 64) { // down
		return DOWN;
	}
	else if((unsigned char)value_x <= 15 && (unsigned char)value_x >= 2
		 && (unsigned char)value_y >= 128 && (unsigned char)value_y < 140) { // left
		return LEFT;
	}
	else if ((unsigned char)value_x > 120 && (unsigned char)value_x <= 130
		 && (unsigned char)value_y < 140 && (unsigned char)value_y >= 128) { // right
		return RIGHT;
	}
	return NONE;
}

#endif /* __JOYSTICK_H__ */
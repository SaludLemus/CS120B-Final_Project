#ifndef __TIMER_H__
#define __TIMER_H__

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

#endif /* __TIMER_H__ */
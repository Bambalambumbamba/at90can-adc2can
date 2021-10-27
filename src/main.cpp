#include <avr/interrupt.h>
#include <util/delay.h>
#include "avr_can/avr_can.h"
#include "./byteorder.h"

#define CAN_ID_MISURA_ROOT    0x316

//volatile uint16_t misura; //10 bit da inviare, invio 2 byte

// CAN_FRAME test_interrupt; 

/*
ISR (ADC_vect){
	misura = (ADCH * 2^8) + ADCL;
	test_interrupt.data.s0 = misura;
	Can0.sendFrame(test_interrupt);
}
*/

int main(void)
{
	cli();
	DDRF |= (1 << PF4); // set PF4 to OUTPUT
	PORTF &= ~(1 << PF4); // set PF4 to LOW
/*
	test_interrupt.id = CAN_ID_MISURA_ROOT;
	test_interrupt.extended = false;
	test_interrupt.priority = 0;
	test_interrupt.length = 2;

	ADMUX = (1<<REFS1)|(1<<REFS0);				//impostata VREF bandgap @ 2.56V
	ADCSRA = (1<<ADIE);							//imposta interrupt: per avviare conversione, aggiungere ADEN e ADSC 
	ADCSRA = (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);  //prescaler a 128
	ADCSRA |= (1<<ADEN) | (1<<ADSC);	
*/


	CAN_FRAME dummy1;
	dummy1.id = 0x100;
	dummy1.extended = 0;
	dummy1.priority = 0;
	dummy1.length =1;
	dummy1.data.bytes[0] = 0x00;

	CAN_FRAME dummy2;
	dummy2.id = 0x101;
	dummy2.extended = 0;
	dummy2.priority = 0;
	dummy2.length =1;
	dummy2.data.bytes[0] = 0x01;

	Can0.begin(CAN_BPS_1000K);

	sei();

	while(1){
		Can0.sendFrame(dummy1);
		_delay_ms(100);
		Can0.sendFrame(dummy2);
		_delay_ms(100);
	}
}



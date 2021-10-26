#include <avr/io.h>
#include <avr/interrupt.h>
#include "avr_can/avr_can.h"

#define CAN_ID_MISURA_ROOT    0x316

volatile uint16_t dato_misura; //10 bit da inviare, invio 2 byte

int ingresso_adc;
CAN_FRAME frame_misura[8]; //dichiaro struttura messaggio_can

[[noreturn]]int main(void)
{
	cli();
	
	for (int i=0; i<8; i++) {
		frame_misura[i].id = (CAN_ID_MISURA_ROOT + i);
		frame_misura[i].extended = false;
		frame_misura[i].priority = 0;
		frame_misura[i].length = 2;
	}
	
  DDRF |= (1 << PF4); // set PF4 to OUTPUT
  PORTF &= ~(1 << PF4); // set PF4 to LOW
	
  ingresso_adc = 0;

	Can0.begin(CAN_BPS_1000K);
		
	//init ADC
	
	ADMUX = (1<<REFS1)|(1<<REFS0);				//impostata VREF bandgap @ 2.56V
	ADCSRA = (1<<ADIE);							//per avviare conversione, aggiungere ADEN e ADSC 
	ADCSRA = (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //prescaler a 128
	
	sei();
	
	ADCSRA |= (1<<ADEN) | (1<<ADSC);
	
    while (1) 
    {
	    dato_misura = ((uint16_t)ADCH << 8) | ADCL;
    }
	
}

/*
ISR (ADC_vect){
	frame_misura[ingresso_adc].data.s0 = dato_misura;
	Can0.sendFrame(frame_misura[ingresso_adc]);
	ingresso_adc = (ingresso_adc < 7) ? ingresso_adc++ : 0;
};
*/
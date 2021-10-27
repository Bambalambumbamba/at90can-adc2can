#include <avr/interrupt.h>
#include <util/delay.h>
#include "avr_can/avr_can.h"
#include "./byteorder.h"

#define CAN_ID_MISURA_ROOT    0x316

volatile uint16_t dato_misura; //10 bit da inviare, invio 2 byte

int ingresso_adc;
CAN_FRAME frame_misura[2]; //dichiaro struttura messaggio_can



int main(void)
{
/*
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
	
	int counter = 0;

    while (1) 
    {
	    dato_misura = counter;
		frame_misura[counter].data.s0 = uint16_t(dato_misura);
		Can0.sendFrame(frame_misura[counter]);
		_delay_ms(1000);
		if (counter < 7) counter++;
			else counter = 0;
    }
	*/
		frame_misura[0].id = CAN_ID_MISURA_ROOT;
		frame_misura[0].extended = false;
		frame_misura[0].priority = 0;
		frame_misura[0].length = 1;

		frame_misura[1].id = (CAN_ID_MISURA_ROOT + 10);
		frame_misura[1].extended = false;
		frame_misura[1].priority = 0;
		frame_misura[1].length = 2;

		CAN_FRAME test_frame;
		test_frame.id = CAN_ID_MISURA_ROOT+11;
		test_frame.extended = false;
		test_frame.priority = 0;
		test_frame.length = 2;
		
		CAN_FRAME canverter[8];
		for (int i = 0; i < 7; i++) {
			canverter[i].id = CAN_ID_MISURA_ROOT + i;
			canverter[i].extended = false;
			canverter[i].priority = 0;
			canverter[i].length = 2; 
		}

		DDRF |= (1 << PF4); // set PF4 to OUTPUT
  		PORTF &= ~(1 << PF4); // set PF4 to LOW

		frame_misura[0].data.byte[0] = 0x0A;
		frame_misura[1].data.s0 = (byteorder::ctohs(0xFEED));
		test_frame.data.s0 = 0xABCD;
		//test_frame.data.byte[2] = 0xEE;

		Can0.begin(CAN_BPS_1000K);

		for (int k = 0; k <7; k++){
			canverter[k].data.s0 = uint16_t(k);
		}

		while(1){
			for (int c = 0; c < 7; c++){
			Can0.sendFrame(frame_misura[0]);
			_delay_ms(100);
			Can0.sendFrame(frame_misura[1]);
			_delay_ms(100);
			Can0.sendFrame(test_frame);
			_delay_ms(100);
			Can0.sendFrame(canverter[c]);
			}
		}

}


/*
ISR (ADC_vect){
	frame_misura[ingresso_adc].data.s0 = dato_misura;
	Can0.sendFrame(frame_misura[ingresso_adc]);
	ingresso_adc = (ingresso_adc < 7) ? ingresso_adc++ : 0;
};
*/
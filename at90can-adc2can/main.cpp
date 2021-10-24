/*
 * CanOpener-Datalogger.cpp
 *
 * Created: 23/10/2021 00:05:44
 * Author : samu_vanno
 */ 

#define F_CPU 16000000UL;

#include <avr/io.h>
#include <avr/interrupt.h>
#include "CAN/can_lib.h"

#define CAN_ID_ROOT = 0x400;

uint8_t dato_misura[2]; //10 bit da inviare, invio 2 byte

volatile int ingresso_adc = 0;






int main(void)
{
	cli();
	
	st_cmd_t messaggio_can; //dichiaro struttura messaggio_can
	
	//init CAN
	
	messaggio_can.pt_data = &dato_misura[0]; //punta a indirizzo della roba da inviare - FORSE NECESSARIO BIT REVERSAL
	messaggio_can.ctrl.ide = 0; //messaggio CAN standard (no extended)
	messaggio_can.dlc = 2; //numero byte da mandare - lo stesso di dato_misura
	messaggio_can.id.std  = CAN_ID_ROOT; //id, da ridefinire meglio probabilmente
	messaggio_can.cmd = CMD_TX_DATA; //configura message object
	
	//init ADC
	
	ADMUX = (1<<REFS1)|(1<<REFS0); //impostata VREF bandgap @ 2.56V
	ADCSRA = (1<<ADIE);//per avviare conversione, aggiungere ADEN e ADSC 
	ADCSRA = (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //prescaler a 128
	
	sei();
	
	ADCSRA |= (1<<ADEN) | (1<<ADSC);
	
    while (1) 
    {
		dato_misura[0] = ADCL;
		dato_misura[1] = ADCH;
    }
	
}

ISR (ADC_vect){
	messaggio_can.id.std = CAN_ID_ROOT + ingresso_adc;
	if (ingresso_adc < 7)
		ingresso_adc++;
	else ingresso_adc = 0;
};

//ISR (ADC_vect)  {
//	uscita.id = 0x700 + (ADMUX & muxmask);
//	uscita.data
//}
//
//int CAN_setup(void) {
//	CAN_FRAME uscita;
//	uscita.id = 0x700;
//	uscita.extended = false;
//	uscita.length = 2;
//	Can0.setBigEndian(true);
//	Can0.enable();
//}

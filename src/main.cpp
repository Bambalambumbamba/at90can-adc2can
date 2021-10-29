/*
samu_vanno @ 31/10/2021

adc2can
manda un numero arbitrario di ingressi dell'adc (PORTF) su diversi ID del canbus.
*/
#include <util/delay.h>
#include <avr/io.h>

#include "avr_can/avr_can.h"
#include "adc/adc_drv.h"
#include "byteorder.h"

#define INPUT_NUMBER 2

#define CAN_ROOT_ID 0x300


int main(void){

    Can0.begin(CAN_BPS_1000K);


    CAN_FRAME measure_frame[INPUT_NUMBER];
    for (int c = 0; c < INPUT_NUMBER; c++){
        measure_frame[c].id = (CAN_ROOT_ID + c);
        measure_frame[c].extended =  false;
        measure_frame[c].priority = 0;
        measure_frame[c].length = 2;
    }

    int selected_pin = 0;
    uint16_t measurement;

    while(true){

        adc_init(AVCC_AS_VREF, false, selected_pin);
        measurement = byteorder::htocs(adc_single_conversion(selected_pin));
        measure_frame[selected_pin].data.s0 = measurement;

        Can0.sendFrame(measure_frame[selected_pin]);

        if (selected_pin==(INPUT_NUMBER-1)) selected_pin = 0;
        else selected_pin = selected_pin +1;
    }
}

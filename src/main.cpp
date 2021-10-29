#include <util/delay.h>
#include <avr/io.h>

#include "avr_can/avr_can.h"
#include "adc/adc_drv.h"
#include "byteorder.h"

#define INPUT_NUMBER 2
#define CAN_ROOT_ID 0x300


int main(void){

    Can0.setBigEndian(true);

    CAN_FRAME measure_frame[INPUT_NUMBER];
    for (int c = 0; c < INPUT_NUMBER-1; c++){
        measure_frame[c].id = (CAN_ROOT_ID + c);
        measure_frame[c].extended =  false;
        measure_frame[c].priority = 0;
        measure_frame[c].length = 2;
    }

    adc_init(EXTERNAL_AREF, false, 0);

    Can0.begin(CAN_BPS_1000K);

    int selected_pin = 0;
    uint16_t measurement;

    while(true){
        measurement = adc_single_conversion(selected_pin);
        measure_frame[selected_pin].data.s0 = byteorder::htocs(measurement);
        Can0.sendFrame(measure_frame[selected_pin]);
        selected_pin = (selected_pin < (INPUT_NUMBER - 1)) ? selected_pin+1 : 0;
    }
}

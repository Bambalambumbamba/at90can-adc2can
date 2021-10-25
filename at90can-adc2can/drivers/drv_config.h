/*
 * config.h
 *
 * Created: 20/03/2021 02:20:06
 *  Author: gfabiano
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_

#define FOSC 16000
#define CAN_BAUDRATE 1000

#define USE_TIMER16 TIMER16_3
#define USE_TIMER8 TIMER8_0

//#define UART_BAUDRATE  VARIABLE_UART_BAUDRATE
//#define USE_UART UART_1


#include <avr/io.h>

#endif /* CONFIG_H_ */
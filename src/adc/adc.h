/*
 * adc.h
 *
 * Created: 22/03/2021 00:15:03
 *  Author: gfabiano
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/interrupt.h>
#include <string.h>

#include "../drv_config.h"

// TODO: put in another file the ring buffer;
#include <util/atomic.h>

template <class T, size_t max_size>
class ring_buffer
{
	public:
		explicit ring_buffer() {}
			
			
		void reset()
		{
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				head_ = tail_;
				full_ = false;
			}
		}
		
		
		void put(T item)
		{
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				if (full_) {
					tail_ = (tail_ + 1) % max_size;
					
					// se sono a tappo tolgo l'elemento che vado a sovrascrivere
					// dalla somma, per evitare risultati errati;
					sum_ -= buf_[tail_];
				}
				
				buf_[head_] = item;
				head_ = (head_ + 1) % max_size;
				
				full_ = head_ == tail_;
						
				sum_ += item;
			}
		}
		
		T get()
		{
			if (empty()) {
				return T();
			}
			
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				T val = buf_[tail_];
				full_ = false;
				tail_ = (tail_ + 1) % max_size;
				
				sum_ -= val;
				return val;
			}
		}
		
		//! This version does not delete elements in the ring
		//! return the last element inserted!
		T get_last()
		{
			if (!empty()) {
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
				{
					return buf_[head_];
				}
			}
			
			return T(); 
		}
		
		bool empty() const { return (!full_ && (head_ == tail_)); }
		bool full() const { return full_; }
		size_t capacity() const { return max_size; }
		
		size_t size() const
		{
			size_t size = max_size;
			if (!full) {
				size = [&] {
					if (head_ >= tail_) { return head_ - tail_ ;}
					
					return max_size + head_ - tail_;
				} ();
			}
			
			return size;		
		}
			
	private:
		volatile T buf_[max_size];
		size_t head_{0};
		size_t tail_{0};
		bool full_{0};
		T sum_{0};
};


// Due to the nature of hardware it is better to make a singleton
// to access resources that are unique during all the time.

namespace {
	volatile uint8_t current_pin;
	volatile bool busy{false};
	ring_buffer<uint16_t, 16> analog_raw[8]{};
}


class EtADC
{
	public:
		static EtADC& get_instance()
		{
			// Guaranteed to be destroyed, instantiated on first use.
			static EtADC _instance;
			// if the singleton is initialized return, else configure it!
			if (_instance._initialized) {
				return _instance;
			}
			
			// singleton initialization here
			
			memset(_instance.enabled_pin, false, sizeof(_instance.enabled_pin));
			
			_instance.disable_adc();
			_instance.enable_interrupt();
			_instance.enable_avcc_as_vref();
			_instance.clear_left_adjust();
			_instance.set_adc_prescaler(ADC_PRESCALER());
			
			_instance.enable_adc();
			
			_instance._initialized = true;
			
			return _instance;
		}
		
		//! Configure ADC Vref as external Aref (AREF pin).
		void enable_external_aref() const {
			ADMUX  &= ~((1<<REFS1)|(1<<REFS0));
		}
		
		//! Configure ADC Vref as AVCC.
		void enable_avcc_as_vref() const {
			ADMUX  &= ~(1<<REFS1);
			ADMUX  |=  (1<<REFS0);
		}
		
		//! Configure ADC Vref as internal vref (2560 mV).
		void enable_internal_vref() const {
			ADMUX  |=  ((1<<REFS1)|(1<<REFS0));
		}
		
		//! Get voltage reference setting.
		//!
		//! @return  voltage reference selection as it is in the ADC register
		//!
		uint16_t get_vref() const { return ADMUX & ((1<<REFS1)|(1<<REFS0)); }
			
		//! ADC result = ADCH[1,0] / ACDL[7..0].
		void clear_left_adjust() const { ADMUX &= ~(1<<ADLAR); }
		
		//! ADC result = ADCH[7..0] / ACDL[7,6].
		void set_left_adjust() const { ADMUX |= (1<<ADLAR); }
		
		//! Select ADC Channel.
		//!
		//! @param adc_channel   0 <= adc_channel <= 0x1F
		//!
		void select_adc_channel(uint8_t adc_channel) const {
			ADMUX  &= ~(0x1F<<MUX0);
			ADMUX |= (adc_channel<<MUX0);
		}
		
		//! Configure Prescaler.
		//!
		//! @param adc_prescaler   0 <= adc_prescaler <= 7
		//!
		void set_adc_prescaler(uint8_t adc_prescaler) const {
			ADCSRA &= ~(0x07<<ADPS0);
			ADCSRA |= (adc_prescaler<<ADPS0);
		}
		
		//! Turn ON the ADC.
		void enable_adc() const {ADCSRA |= (1<<ADEN); }
			
		//! Turn OFF the ADC
		void disable_adc() const {ADCSRA &= ~(1<<ADEN); }
			
		//! Enable High Speed Mode.
		//void set_hsm() const { ADCSRB |= (1<<HSM); }
		
		//! Disable High Speed Mode.
		//void clear_hsm() const { ADCSRB &= ~(1<<HSM); }
		
		//! Start ADC conversion in single conversion mode.
		void start_conv() const {
			ADCSRA &= ~(1<<ADATE);
			ADCSRA |=  (1<<ADSC);
		}
		
		//! ADC conversion complete status: if false the conversion is running;
		bool conv_complete() const { return (ADCSRA  &  (1<<ADIF)); }
			
		// ADC interrupts!
		void enable_interrupt() const { ADCSRA |=  (1<<ADIE); }
		void disable_interrupt() const { ADCSRA &= ~(1<<ADIE); }
		void clear_interrupt_flag() const { ADCSRA |=  (1<<ADIF); }
			
			
		// User functions
		bool pin_enable(uint8_t pin)
		{
			if (pin < 8) {
				enabled_pin[pin] = true;
				enabled_pin_count++;
				return true;
			}
			
			return false;	
	}
	
	bool pin_disable(uint8_t pin)
	{
		if (pin < 8) {
			enabled_pin[pin] = false;
			enabled_pin_count--;
			return true;
		}
		
		return false;
	}
	
	void init()
	{
		disable_adc();
		enable_interrupt();
		clear_interrupt_flag();
		enable_avcc_as_vref();
		clear_left_adjust();
		set_adc_prescaler(ADC_PRESCALER());
		enable_adc();
	}
	
	void eventloop()
	{
			if (!busy) {
				busy = true;
				next_pin();
				if (!enabled_pin[current_pin]) {
					busy = false;
					return;
				}
				
				clear_interrupt_flag();
				disable_interrupt();
				disable_adc();

				select_adc_channel(current_pin);
				enable_interrupt();
				enable_adc();
			
			
				start_conv();
			}
			//while(!conv_complete());
		//}
	}
	
	uint16_t read_raw(uint8_t pin)
	{
		return analog_raw[pin].get_last();
	}
	
	uint16_t test_adc_read(uint8_t pin)
	{
		disable_adc();
		enable_interrupt();
			//disable_interrupt();
			//clear_interrupt_flag();
			enable_avcc_as_vref();
			
			clear_left_adjust();
			
			select_adc_channel(pin);
			set_adc_prescaler(ADC_PRESCALER());
			
			enable_adc();
			start_conv();
			while(!conv_complete());
			uint16_t out = ADC;
			clear_interrupt_flag();
			
			return out;
		}


	private:
	
		EtADC(){}

		bool _initialized{false};
				
		bool enabled_pin[8]{false};
		uint8_t enabled_pin_count{0};
			
		void next_pin()
		{
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				current_pin = (current_pin + 1) % 8;
				return;

			for (uint8_t i = 1; i <= 8; ++i) {
				uint8_t new_pin = (current_pin + i) % 8;
				if (enabled_pin[new_pin]) {
					current_pin = new_pin;
				}
			}
			}
		}
		
		enum class REF{ EXTERNAL_AREF=0, AVCC_AS_VREF=1, INTERNAL_VREF=3 };
		enum class ADJUST{ LEFT_ADJUST=1, NO_LEFT_ADJUST };
		
		static uint16_t const ADC_FULL_RANGE = 0x03ff;		// 10-bit ADC 
		static uint16_t const INTERNAL_VREF_VALUE = 256;	// 2.56 volts
		
		static constexpr uint8_t ADC_PRESCALER() {
			#ifndef FOSC
				#error You must define FOSC in config.h
			#elif FOSC >= 20000
				#error Highes FOSC definition
			#else
				if (FOSC < 500) {
					return 0x01;
				} else if (FOSC < 1000) {
					return 0x02;
				} else if (FOSC < 2000) {
					return 0x03;
				} else if (FOSC < 4000) {
					return 0x04;
				} else if (FOSC < 8000) {
					return 0x05;
				} else if (FOSC < 16000) {
					return 0x06;
				} else {
					return 0x07;
				}
			#endif
		};
	public:
		EtADC(EtADC const&)				= delete;
		void operator=(EtADC const&)	= delete;
};

ISR(ADC_vect)
{
	uint16_t result = ADC;
	if (result > 512) {
		//PORTF = 0b00000010;
		PORTF = 0xff;
	}  else {
		//PORTF = 0;
		
		PORTF = 0b11111101;
	}
	
	analog_raw[current_pin].put(result);
	//analog_raw[3].put(current_pin);
	busy = false;
}



#endif /* ADC_H_ */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"


ISR(ADC_vect)
{
	print_hex_value(ADCH);
	uart_printstr("\r\n");
}

void adc_init(void)
{
	//setting prescaler at 128 because 16 000 000 / 128 = 125 000 wich is in the range of the ADC
	ADCSRA = (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);


	ADCSRA |= 1 << ADIE;
	ADCSRA |= 1 << ADEN; //enabling ADC

	ADMUX = (1 << REFS0); //setting reference voltage as AREF pin / AVcc
	ADMUX |= (1 << ADLAR); //enabling left adjustment in data registers
	
	//leaving MUX selection bits as false in ADMUX since i only want ADC0 as input
}

int main()
{
	uart_init();
	adc_init();

	sei();
	for(;;)
	{
		_delay_ms(20);
		while (ADCSRA & 1 << ADSC);//if conversion not finished, wait
		ADCSRA |= 1 << ADSC; // start conversion
	}
}

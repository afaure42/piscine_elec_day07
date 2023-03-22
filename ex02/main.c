#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define RV1_PIN (0)
#define LDR_PIN (1)
#define NTC_PIN (2)

void adc_init(void)
{
	//setting prescaler at 128 because 16 000 000 / 128 = 125 000 wich is in the range of the ADC
	ADCSRA = (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);


	ADCSRA |= 1 << ADEN; //enabling ADC

	ADMUX = (1 << REFS0); //setting reference voltage as AREF pin / AVcc
	// ADMUX |= (1 << ADLAR); //enabling left adjustment in data registers
	
	//leaving MUX selection bits as false in ADMUX since i only want ADC0 as input
}

void adc_set_input(uint8_t input)
{
	ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));

	ADMUX |= input;
}

void adc_print_input(uint8_t input)
{
	while(ADCSRA & (1 << ADSC));
	adc_set_input(input);
	ADCSRA |= 1 << ADSC;
	while(ADCSRA & (1 << ADSC));
	uint16_t value = ADCL;

	value |= ADCH << 8;
	uart_putnbr(value);
}

int main()
{
	uart_init();
	adc_init();

	for(;;)
	{
		adc_print_input(RV1_PIN);
		uart_printstr(", ");
		adc_print_input(LDR_PIN);
		uart_printstr(", ");
		adc_print_input(NTC_PIN);
		uart_printstr("\r\n");
		_delay_ms(20);
	}
}

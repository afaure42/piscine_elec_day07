#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define RV1_PIN (0b0000)
#define LDR_PIN (0b0001)
#define NTC_PIN (0b0010)

#define LED_D5_R (1 << PD5)
#define LED_D5_G (1 << PD6)
#define LED_D5_B (1 << PD3)

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

uint8_t adc_read_input(uint8_t input)
{
	while(ADCSRA & (1 << ADSC));
	adc_set_input(input);
	ADMUX |= 1 << ADLAR;

	ADCSRA |= 1 << ADSC;
	while(ADCSRA & (1 << ADSC));

	return (ADCH);
}

void init_rgb()
{
	//set mode PWM with FF as top ( mode 3)
	//output mode is clear at bottom set at match so values are inverted
	//( in this mode a true 0 duty  cycle can be done, RTFM)
	TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0); 

	//set basic clock without prescaler
	TCCR0B = (1 << CS00);

	OCR0A = 255;
	OCR0B = 255;

	//same setting for timer 2
	TCCR2A = (1 << WGM21) | (1 << WGM20) | (1 << COM2B1) | (1 << COM2B0);
	TCCR2B = (1 << CS20);

	OCR2B = 255;
	// OCR2A = 0xFF;
	// TIMSK2 = 1 << OCIE2A;
}

void set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	OCR0B = 255 - r;
	OCR0A = 255 - g;
	OCR2B = 255 - b;
}

void wheel(uint8_t pos)
{
	pos = 255 - pos;
	if (pos < 85)
	{
		set_rgb(255 - pos * 3, 0, pos * 3);
	}
	else if (pos < 170)
	{
		pos = pos - 85;
		set_rgb(0, pos * 3, 255 - pos * 3);
	}
	else
	{
		pos = pos - 170;
		set_rgb(pos * 3, 255 - pos * 3, 0);
	}
}

int main()
{
	adc_init();
	init_rgb();

	uint8_t reading;

	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);
	DDRD = LED_D5_B | LED_D5_G | LED_D5_R;

	for(;;)
	{
		_delay_ms(20);
		reading = adc_read_input(RV1_PIN);
		wheel(reading);
		PORTB = 0;
		if (reading >= 64)
			PORTB |= (1 << PB0);
		if (reading >= 128)
			PORTB |= (1 << PB1);
		if (reading >= 192)
			PORTB |= (1 << PB2);
		if (reading >= 255)
			PORTB |= (1 << PB4);
	}
}

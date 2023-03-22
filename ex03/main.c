#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define RV1_PIN (0b0000)
#define LDR_PIN (0b0001)
#define NTC_PIN (0b0010)
#define INTERNAL_SENSOR_PIN (0b1000)

// #define TS_OFFSET_H 0x00
// #define TS_OFFSET_L 0x02
// #define TS_GAIN_H 0x00
// #define TS_GAIN_L 0x03

// #define Z_REG_L (*(volatile uint8_t *)(0x1E))
// #define Z_REG_H (*(volatile uint8_t *)(0x1F))


// volatile uint16_t g_ts_offset;
// volatile uint16_t g_ts_gain;

// void load_signature_values(void)
// {
// 	//first load ts_offset address
// 	Z_REG_L = TS_OFFSET_L;
// 	Z_REG_H = TS_OFFSET_H;
// 	SPMCSR |= 1 << SIGRD | 1 << SPMEN;
// 	__asm__("lpm r16, Z");

// 	g_ts_offset = 
// }

void adc_init(void)
{
	//setting prescaler at 128 because 16 000 000 / 128 = 125 000 wich is in the range of the ADC
	ADCSRA = (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);


	ADCSRA |= 1 << ADEN; //enabling ADC

	ADMUX = 1 << REFS0 | 1 << REFS1; //enaling internal 1.1 voltage reference with external capacitor
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
	uint16_t value_high = ADCH;

	value |= value_high << 8;
	uart_putnbr(value >> 8);
}

void adc_read_temp(void)
{
	while(ADCSRA & (1 <<ADSC));
	adc_set_input(INTERNAL_SENSOR_PIN);
	ADCSRA |= 1 << ADSC;
	while(ADCSRA & (1 << ADSC));
	uint16_t read_value = ADCL;
	read_value |= (uint16_t)(ADCH << 8);

	int16_t temp_value = (float)(read_value - 324.31) / 1.22;
	uart_putnbr(temp_value);
	uart_printstr("\r\n");
}

int main()
{
	uart_init();
	adc_init();

	for(;;)
	{
		adc_read_temp();
		_delay_ms(20);
	}
}

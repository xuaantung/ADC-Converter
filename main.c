#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define BUFFER_SIZE 50

volatile static uint16_t adc_value = 0;
volatile static int adc_done = 0;
static float voltage_history[BUFFER_SIZE];
static float curr_volt = 0;
int amplifier = 31; // boost sensitity of light sensor

float convert_volt(uint16_t adc)
{
    return (adc * (2.56 / 1024.0));
}

void init_adc()
{
    // set 2.56V
    ADMUX |= _BV(REFS1) | _BV(REFS0);

    // enable ADC, set ADC prescale to 8 - 125000Hz
    ADCSRA |= _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);
}

uint16_t convert_adc()
{
    // right adjusted -> 8 bits in low, 2 bits in high
    uint8_t lower = 0;
    uint8_t upper = 0;

    // start adc
    ADCSRA |= _BV(ADSC);
    while (ADCSRA & (1 << ADSC)) // break when adc conversion finish
        ;
    lower = ADCL;
    upper = ADCH;
    uint16_t result = (lower | (upper << 8));
    return result;
}

float average_volt(int i, int filled)
{
    float avg = 0;
    if (filled)
        i = BUFFER_SIZE;
    for (int j = 0; j < i; j++)
        avg += voltage_history[j];
    return (avg / i);
}

int main(void)
{
    // set direction - 1 for output (LED)
    DDRB = 0xFF;
    PORTB = 0xFF; // 1 - off (pull up resistor)

    init_adc();
    int i = 0;
    int filled = 0;
    for (;;)
    {
        // getting adc value from light sensor
        adc_value = convert_adc();
        // convert adc to volts
        curr_volt = convert_volt(adc_value);
        if (i >= BUFFER_SIZE)
        {
            filled = 1;
            i = 0;
        }
        // do history
        voltage_history[i] = curr_volt;
        i++;
        // averaging voltage
        curr_volt = average_volt(i, filled);
        // scale voltage to level of brightness
        int level = curr_volt * 100; // convert 2.56V to 256 Level of Brightness
        // turn respective LEDs
        int leds = level / 32; // 256 levels with 8 LEDs
        uint8_t temp = 0x00;
        PORTB = 0xff;
        if (level != 0)
        {
            for (int j = 0; j <= leds; j++)
                temp |= _BV(j);
        }

        PORTB &= ~(temp);
        // PORTH = ~(adc_value);
    }
}
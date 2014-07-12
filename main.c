#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/*
 
 
         ##          ##
           ##      ##        
         ##############
       ####  ######  ####
     ######################
     ##  ##############  ##    
     ##  ##          ##  ##
           ####  ####
 
 
*/


volatile unsigned long t; // long
volatile unsigned long u; // long

volatile uint8_t snd; // 0...255
volatile uint8_t sfx; // 0...255

volatile uint8_t pot1; // 0...255
volatile uint8_t pot2; // 0...255
volatile uint8_t pot3; // 0...255

//volatile int j = 50;

volatile uint8_t btn_toggle = 0;
volatile uint8_t songs = 0;
volatile unsigned int btn_hold; // long
volatile uint8_t btn_previous = 1;

//ADMUX ADC

volatile uint8_t adc1 = _BV(ADLAR) | _BV(MUX0); //PB2-ADC1 pot1
volatile uint8_t adc2 = _BV(ADLAR) | _BV(MUX1); //PB4-ADC2 pot2
volatile uint8_t adc3 = _BV(ADLAR) | _BV(MUX0) | _BV(MUX1); //PB3-ADC3 pot3


#define ENTER_CRIT()    {byte volatile saved_sreg = SREG; cli()
#define LEAVE_CRIT()    SREG = saved_sreg;}


#define true 1
#define false 0

//button state
#define BUTTON_NORMAL 0
#define BUTTON_PRESS 1
#define BUTTON_RELEASE 2
#define BUTTON_HOLD 3

uint8_t button_timer_enabled = false;
unsigned int button_timer = 0;
unsigned int button_last_pressed = 0;


//loop mode on button hold
uint8_t start_loop = false;
unsigned int loop_timer = 0;
unsigned int loop_max = 0;
unsigned int hold_timer = 0;


PROGMEM prog_char  waveTable[] =

{  
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,

  1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
  61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,
  114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,
  157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,
  200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,
  243,244,245,246,247,248,249,250,251,252,253,254,

  1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,
  33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,
  67,69,71,73,75,77,79,81,83,85,87,89,91,93,95,97,99,
  101,103,105,107,109,111,113,115,117,119,121,123,125,127,129,131,133,
  135,137,139,141,143,145,147,149,151,153,155,157,159,161,163,165,167,
  169,171,173,175,177,179,181,183,185,187,189,191,193,195,197,199,201,
  203,205,207,209,211,213,215,217,219,221,223,225,227,229,231,233,235,
  237,239,241,243,245,247,249,251,253,255,254,252,250,248,246,244,242,
  240,238,236,234,232,230,228,226,224,222,220,218,216,214,212,210,208,
  206,204,202,200,198,196,194,192,190,188,186,184,182,180,178,176,174,
  172,170,168,166,164,162,160,158,156,154,152,150,148,146,144,142,
  140,138,136,134,132,130,128,126,124,122,120,118,116,114,112,110,108,
  106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,
  64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,
  20,18,16,14,12,10,8,6,4,2,0,
};


void adc_init()
{
    ADCSRA |= _BV(ADIE); //adc interrupt enable
    ADCSRA |= _BV(ADEN); //adc enable
    ADCSRA |= _BV(ADATE); //auto trigger
    ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //prescale 128
    ADMUX  = adc1;
    ADCSRB = 0;
}

void adc_start()
{
    ADCSRA |= _BV(ADSC); //start adc conversion   
}

void timer_init()
{
    //PWM SOUND OUTPUT
    TCCR0A |= (1<<WGM00)|(1<<WGM01); //Fast pwm
    //TCCR0A |= (1<<WGM00) ; //Phase correct pwm
    TCCR0A |= (1<<COM0A1); //Clear OC0A/OC0B on Compare Match when up-counting.
    TCCR0B |= (1<<CS00);//no prescale
        
    //TIMER1 SOUND GENERATOR @ 44100hz
    //babygnusb attiny85 clock frequency = 16.5 Mhz
    
    //TIMER SETUP
    TCCR1 |= _BV(CTC1); //clear timer on compare
    TIMSK |= _BV(OCIE1A); //activate compare interruppt
    TCNT1 = 0; //init count

    //TIMER FREQUENCY
    //TCCR1 |= _BV(CS10); // prescale 1
    //TCCR1 |= _BV(CS11); // prescale 2
    TCCR1 |= _BV(CS10)|_BV(CS12); // prescale 16    
    //TCCR1 |= _BV(CS11)|_BV(CS12); // prescale 32
    //TCCR1 |= _BV(CS10)|_BV(CS11)|_BV(CS12); // prescale 64
    //TCCR1 |= _BV(CS13); // prescale 128
    //TCCR1 |= _BV(CS10) | _BV(CS13); // prescale 256    
    
    //SAMPLE RATE
    //OCR1C = 128; // (16500000/16)/8000 = 128
    OCR1C = 93; // (16500000/16)/11025 = 93
    //OCR1C = 46; // (16500000/16)/22050 = 46
    //OCR1C = 23; // (16500000/16)/44100 = 23

    
    // babygnusb led pin
    DDRB |= (1<<PB0); //pin connected to led
    
}


void button_init()
{
    DDRB &= ~(1<<PB1); //set to input
    PORTB |= (1<<PB1); //pin btn       
}


int button_is_pressed(uint8_t button_pin, uint8_t button_bit)
{
    /* the button is pressed when BUTTON_BIT is clear */
    if (!bit_is_clear(button_pin, button_bit))
    {
        //_delay_ms(25);
        if (!bit_is_clear(button_pin, button_bit)) return 1;
    }
    
    return 0;
}

int button_is_changed()
{
    return 0;        
}


void disable_button_timer()
{
    button_timer_enabled = false;
    button_timer = 0;
}

void enable_button_timer()
{
    button_timer_enabled = true;
    button_timer = 0;
}

int main(void)
{
    timer_init();// initialize timer & Pwm	
    adc_init(); //init adc
    button_init();
    sei(); //enable global interrupt
    adc_start(); //start adc conversion
    
    // run forever
    
    while(1)
    {
        /* 
        uint8_t btn_now = button_is_pressed(PINB, PB1);
        if ( btn_previous != btn_now && btn_now == 1 ) { 
            songs++;
            if (songs > 3) songs = 0;
            btn_previous = btn_now;
        }else{
            btn_previous = btn_now;
        }      
        */
        switch (songs)
        {
            case 0:
            snd = {template};
            break;
            case 1:
            snd =(t*9&t>>pot1|t*pot2&t>>7|t*3&t/1024)-1;
            break;
            case 2:
            snd = (t * (t>>5|t>>pot1))>>(t>>pot2);
            break;
            case 3:
            snd = (t*pot1&t>>7)|(t*pot2&t>>10);
            break;
            case 4:
            snd =  t * ((pot1>>12|t>>8)&pot2&t>>4);
            break;
        }

    }
    return 0;
}

ISR(TIMER1_COMPA_vect)
{

    uint8_t btn_now = button_is_pressed(PINB, PB1);
    uint8_t button_state = 0;

    if ( btn_previous != btn_now && btn_now == 0 ) {    
        button_state  = BUTTON_PRESS;
    }else if( btn_previous != btn_now && btn_now == 1  )
    {
        button_state  = BUTTON_RELEASE;
    }else if (btn_previous == btn_now && btn_now == 0)
    {
        button_state  = BUTTON_HOLD;
    }else{
        button_state  = BUTTON_NORMAL;
    }   


    if (button_state == BUTTON_PRESS)
    {
        start_loop = false;
        u = t;

        if (button_timer_enabled == false)
        {
            enable_button_timer();
        }else{
            if (button_timer > 1000)
            {
                if (button_timer < 7350)
                {
                    songs++;
                    if (songs > 4) songs = 0;
                    disable_button_timer();
                }else{
                    disable_button_timer();
                }
            }

        }
    }


    if (button_state == BUTTON_HOLD)
    {
       //u = t;
       //snd = 0;
       hold_timer++;
    }


    if (button_state == BUTTON_RELEASE)
    {
        //t = 0;
        
        if (btn_previous == 0)
        {
            loop_max = (t-u);
            if (hold_timer > 2756)
            {
                start_loop = true;
            }
            hold_timer = 0;
        }
    }


    
    if (start_loop)
    {
        loop_timer++;
        if (loop_timer > loop_max)
        {
            loop_timer = 0;
            t = u;
        }
    }
    
    
    OCR0A = snd;
    t++;

    btn_previous = btn_now;

    if (button_timer_enabled)
    {
        button_timer++;
        if (button_timer > 11025)
        {
            disable_button_timer();
        } 
    }
}

ISR(ADC_vect)
{
    //http://joehalpin.wordpress.com/2011/06/19/multi-channel-adc-with-an-attiny85/
    
    static uint8_t firstTime = 1;
    static uint8_t val;
    
    val = ADCH;
    
    if (firstTime == 1)
        firstTime = 0;
    else if (ADMUX  == adc1) {
        pot1 = val;
        ADMUX = adc2;
    }
    else if ( ADMUX == adc2) {
        pot2  = val;
        ADMUX = adc1;
    }
    
}
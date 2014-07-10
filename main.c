#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

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
        u = t;
        //t = 0;
        //songs++;
        //if (songs > 4) songs = 0;

        if (button_timer_enabled == false)
        {
            button_timer_enabled = true;
        }else{
            if (button_timer < 5512)
            {
                songs++;
                if (songs > 4) songs = 0;
                button_timer_enabled  = false;
            }
        }

    }


    if (button_state == BUTTON_HOLD)
    {
        loop_timer++;
        if (loop_timer > (1378))
        {
            t = u;
            loop_timer = 0;
        }  
    }


    if (button_state == BUTTON_RELEASE)
    {
        //t = t - 11025;
        //t = 0;
    }


    btn_previous = btn_now;

      
    //OCR1C = pot2;
    //delay_efek[t%99] = ((t * ((t>>12|t>>8)&63&t>>4))/2);
    //int value50 = 30;

    //    if (btn_toggle)
    //    {
    //        snd =(t*9&t>>pot1|t*pot2&t>>7|t*3&t/1024)-1;
    //    }else{
    //        snd = (t * (t>>5|t>>pot1))>>(t>>pot2); //viznut
    //    }

    //snd = {template};
    //snd =t*(t/256)-t*(t/pot1)+t*(t>>5|t>>pot2|t<<2&t>>1);
    //snd = (t>>5)|(t<<pot1)|((t&1023)^1981)|((t-67)>>pot2);
    //snd = (t*9&t>>29|t*5&t>>24|t*3&t/1024)-1 ;
    //snd = ((t*("36364689"[t>>13&7]&15))/12&128)+(((((t>>pot1)^(t>>12)-2)%11*t)/4|t>>pot2)&127)
    //snd = (t * (t>>5|t>>pot1))>>(t>>pot2); //viznut
    //snd = (t * ((t>>pot2|t>>8)&pot1&t>>4));//floor((delay_efek[t%99]);
    //sfx = (snd >> 4 << 7);
    //sfx = (snd >> 2 << 3);
    //sfx = sfx | j;
    //snd = (t*pot1&t>>7)|(t*pot2&t>>10)  ;
    //snd =  t * ((t>>12|t>>8)&63&t>>4);
    //snd = (t * ((t>>pot2|t>>pot1)&63&t>>4));//floor((delay_efek[t%99]);
    //sfx = (snd >> pot2 << pot1);
    //sfx = (snd >> 2 << 3);
    //sfx = sfx | j;
    //
    //
    
    
    OCR0A = snd;
    t++;

    //if (button_is_pressed(PINB, PB1)) {
    //    t = t - 300;
    //}
    //OCR1C = pot1;
    
    //j = j - 1;
    //if (j <= 0) {
    //    j = value50;
    //}
    
    if (button_timer_enabled)
    {
        button_timer++;
        if (button_timer > 33075)
        {
            button_timer = 0;
            button_timer_enabled = false;
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
#include <xc.h>
#include <stdint.h>
#include <sys/attribs.h>
#include <stdlib.h>

#define SYS_FREQ 40000000
#define shift(a) a=a<<5
#define parity_even(a) a = a+ 0x10
#define add_msb(a) a = a + 0x8000
#define add_lsb(a) a = a + 0x01

int16_t adc_value =0;
int16_t pwm_ref=0;
int16_t adc_out = 0;

//Internal Oscillator is 40MHZ of pic32mx120f032c

int count_ones(int hex){
    unsigned int ones = 0;
    
    while(hex > 0) {
        ones += hex & 0x1;
        hex >>= 1;
    }

    return ones;
};

void delay_us(unsigned int us)
{
    us *= SYS_FREQ/1000000/2; 
    _CP0_SET_COUNT(0); 
    while (us > _CP0_GET_COUNT()){
    
    
    }; // Wait until Core Timer count reaches the number we calculated earlier
}

void format_output(){
    shift(adc_value);
    add_lsb(adc_value);

    int num_ones;
    num_ones = count_ones(adc_value);
    
    if(num_ones % 2 == 0){
        parity_even(adc_value);
    }
    
    add_msb(adc_value);
};

void send_output(){
    int counter = 0;
    adc_out = adc_value;
        
    while (counter < 16){
        //debugging bit
        LATCbits.LATC1 = 0;
        //debugging bit
        if (adc_out & 1){
            LATCbits.LATC0 = 1;
        }
        else {
            LATCbits.LATC0 = 0;
        }
        //debugging bit
        LATCbits.LATC1 = 1;
        //debugging bit
        adc_out = adc_out >> 1;
        delay_us(62.5);
        counter += 1;
        //debugging bit
        LATCbits.LATC1 = 0;
        //debugging bit
    }
};

int main(void){
    AD1CON1bits.ON = 1; // Enable ADC
    AD1CON1bits.FORM = 0; // See family reference 
    //AD1CON1bits.MODE12 = 1; //set as 10bit by default
    AD1CON1bits.ASAM = 0; // Do NOT auto-sample
    AD1CON1bits.SSRC = 0; // Clearing SAMP will initiate conversion
    AD1CON2bits.VCFG = 0; // Reference voltage is the power supply (~3.3V)
    AD1CON2bits.CSCNA = 0; // Only one channel is used, or manual scanning is
    AD1CHSbits.CH0NA = 0b000; // Selects AVSS (essentially, ground)
    AD1CHSbits.CH0SA = 0b00011; // Selects AN3
    TRISBbits.TRISB1 = 1; // Input mode
    ANSELBbits.ANSB1 = 1; // Analog mode
    
    //debugging bit
    ANSELCbits.ANSC0 = 0;    //set digital 
    TRISCbits.TRISC0 = 0;   // Set RC0 to be an output
    ANSELCbits.ANSC1 = 0;    //set digital 
    TRISCbits.TRISC1 = 0;
    ANSELCbits.ANSC3 = 0;    //set digital 
    TRISCbits.TRISC3 = 0;
     //debugging bit
    
    AD1CON1bits.SAMP = 1;
    for (;;){
        if(AD1CON1bits.DONE){ //DONE =1 , ADC is done
            adc_value = ADC1BUF0;
            format_output();
            AD1CON1bits.DONE = 0;
            AD1CON1bits.SAMP = 1; //SAMP = 1 ADC SHA is sampling 
        }
        
        if (AD1CON1bits.SAMP){
            //send_output is set to be equal to 1ms; will essetially serve as the delay
            send_output(); //total of 1ms
            //delay_us(1000);
            AD1CON1bits.SAMP = 0;
        }    
        
    }
    // This return must never be reached
    return 1;
}
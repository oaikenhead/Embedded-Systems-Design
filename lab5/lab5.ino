/**
 * Olliver Aikenhead
 * Lab 5
 * 
 * Write a program for Arduino Mega 2650 that monitors inputs from keyboard and turns it into a piano
 * keys A, B, C, D, E, F, and G (along with sharp notes) are pressed and their corresponding frequencies are played 
 * to a speaker for the respective Hertz for that note
 */

/* register init */
unsigned char *myTCCR1A = (unsigned char *)0x80;              // timer counter control register 1A
unsigned char *myTCCR1B = (unsigned char *)0x81;              // timer counter control register 1B (sets prescalar and waveform generation mode)
unsigned char *myTCCR1C = (unsigned char *)0x82;              // timer counter control register 1C (for setting bits 5-7 to 0 for normal mode)
unsigned char *myTIMSK1 = (unsigned char *)0x6F;              // timer interrupt mask register (for enabling/disabling interrupts)
unsigned char *myTIFR1 = (unsigned char *)0x36;               // timer interrupt flag register
unsigned int *myTCNT1 = (unsigned int *)0x84;                 // timer counter register

/* port init */
volatile unsigned char *portDDRB = (unsigned char *)0x24;     
volatile unsigned char *portB = (unsigned char *)0x25;        
volatile unsigned char *pinB = (unsigned char *)0x23;         

/* global variables */
uint16_t DELAY_VALUE = 200;

/* function prototypes */
/**
 * calc_ticks()
 * takes unsigned int frequency
 * sets the period of the clock, then calculates ticks at 50% duty cycle then full
 */
unsigned int calc_ticks(unsigned int freq);

/**
 * freq_from_input()
 * takes character input from keyboard
 * converts the input from keyboard into frequency
 */
unsigned int freq_from_input(char input);

/**
 * myDelay()
 * takes the ticks generated from setting clock cycle
 * sets timer values and overflow flags for cpu
 */
void myDelay(unsigned int ticks);

/* program execution */
void setup() {
  // set PB6 to output
  *portDDRB = 0x40;       
  // initialize PB6 to low                                        
  *portB = 0x00;
  //*portB = 0x1;                                                 

  // init timer control registers
  *myTCCR1A = 0x00;                                               
  *myTCCR1B = 0x00;                                               
  *myTCCR1C = 0x00;                                               
  Serial.begin(9600);
}

void loop() {
  unsigned char in_char;
  unsigned int ticks;
  // check if data has been sent from the computer
  if (Serial.available()) {
    while (in_char != 'q') {  
      // read the most recent byte
      in_char = Serial.read();
  
      // ECHO the value that was read, back to the serial port
      Serial.write(in_char);
  
      // reading input
      freq_from_input(in_char);
  
      // setting PB6 to high
      *portB |= 0x40;
    
      // call myDelay
      myDelay(ticks);
    
      // set PB6 to low
      *portB &= 0xBF;
  
      // take delay of ticks
      myDelay(ticks);
    }
  }
}

/* function definitions */
// calc ticks
unsigned int calc_ticks(unsigned int freq){
  // setting the clock period
  double clk_per_def = 0.0000000625;
  double freq_dbl = (double) freq;                                
  double period = 1.0/freq_dbl;
  // 50% duty cycle                                   
  double half_period = period / 2.0;                              
  double tick_dbl = half_period*2000000;
  unsigned int ticks = (int) tick_dbl;
  Serial.println(ticks);
  return ticks;
}

// frequency from input
unsigned int freq_from_input(char input) {
  unsigned char letters[] = {'A','a','B','C','c','D','d','E','F','f','G','g'}; 
  unsigned int freq_arr[] = {440,466,494,523,554,587,624,659,698,740,784,831};  
  unsigned int index = -1;
  unsigned int freq;

  // search through letters array to the index of character
  for (int i=0; i<12; i++) {
    if (input == letters[i]) {
      index = i;
      freq = freq_arr[i];
    }
  }
  
  // return value from that index from freq_array
  // debug for index number
  //Serial.println(index);

  // debug for frequency assignment
  Serial.println(freq);

  calc_ticks(freq);
}

// delay from ticks
void myDelay(unsigned int ticks) {
  // set timer value
  *myTCNT1 = (unsigned int)(65536 - ticks);  
  
  // set normal mode                  
  *myTCCR1A = 0; 

  // start the timer with the right prescalar                                                
  *myTCCR1B = 0x04;
  
  // wait for TIFR overflow flag bit to be set                                            
  while((*myTIFR1 & 0x01)==0);
                                             
  // writing LSB to 1, reset the overflow flag                                                            
  *myTIFR1 = 0x01;
}

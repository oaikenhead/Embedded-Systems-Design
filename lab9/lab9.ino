/**
 * Olliver Aikenhead
 * 
 * Lab 9
 */

/* macro definitions */
#define RDA 0x80
#define TBE 0x20  

/* register init */
volatile unsigned char *myUCSR0A = (unsigned char*)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char*)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char*)0x00C2;
volatile unsigned char *myTCCR1A = (volatile unsigned char*)0x80;
volatile unsigned char *myTCCR1B = (volatile unsigned char*)0x81;
volatile unsigned char *myTCCR1C = (volatile unsigned char*)0x82;
volatile unsigned int  *myUBRR0  = (unsigned int*)0x00C4;
volatile unsigned char *myUDR0   = (unsigned char*)0x00C6;
volatile unsigned char *myTIMSK1 = (volatile unsigned char*)0x6F;
volatile unsigned int  *myTCNT1 = (volatile unsigned int*)0x84;
volatile unsigned char *myTIFR1 =  (volatile unsigned char*)0x36;
volatile unsigned char *portB = (volatile unsigned char*)0x25;
volatile unsigned char *pinB = (volatile unsigned char*)0x23;
volatile unsigned char *ddrB = (volatile unsigned char*)0x24;

/* global variables */
unsigned int ticks[13] = {18181, 17167, 16184, 15296, 14440, 13628, 12820, 12139, 11461, 10810, 10204, 9627};
unsigned int total_ticks = 65535;
unsigned int output;

/* function prototypes */
void U0init(unsigned long U0baud);
unsigned char U0kbhit();
unsigned char U0getchar();
void U0putchar(unsigned char ch);
unsigned int calc_ticks(char input);
unsigned int freq_from_input(char input);
void timer_reg();

/* program execution */
void setup() {
  *ddrB = 0b10111111;
  *portB &= 0b01000000;
  U0init(9600);
  timer_reg();
}

void loop() {
  unsigned int cs1;
  char input;
  
  if (U0kbhit()) {
    input = U0getchar();
    
    // writes the character
    U0putchar(input);

    // getting ticks from global array
    output = calc_ticks(input); 
  }
  if (*myTCCR1B != 0b00000001) {
    *myTCCR1B |= 0b00000001;
  } else if (input == 'q' | input == 'Q') {
      *myTCCR1B &= 0b11111000; // turns timer off
      *portB &= 0b10111111;
  }
}

/* function definitions */
// init 
void U0init(unsigned long U0baud) {
  unsigned long FCPU = 16000000; 
  unsigned int tbaud; 
  tbaud = (((unsigned long)(FCPU / ((unsigned long)(16 * U0baud)))) - 1); 
  *myUCSR0A = 0x20; 
  *myUCSR0B = 0x18; 
  *myUCSR0C = 0x06;  
  *myUBRR0 = tbaud; 
}

// returning RDA
unsigned char U0kbhit() {
  return (*myUCSR0A & RDA); 
}

// getting char
unsigned char U0getchar() {
  return *myUDR0;
}

// putting char
void U0putchar(unsigned char ch) {
  while (!(*myUCSR0A & TBE));

  *myUDR0 = ch;
}

// calculating ticks
unsigned int calc_ticks(char input) {
  // setting the clock period
  double clk_per_def = 0.0000000625;
  double freq_dbl = (double) input;                                
  double period = 1.0/freq_dbl;
  // 50% duty cycle                                   
  double half_period = period / 2.0;                              
  double tick_dbl = half_period*2000000;
  unsigned int ticks = (int) tick_dbl;
  Serial.println(ticks);
  return ticks;
}

// getting frequency from input 
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

// init timers at start of program
void timer_reg() {
  // timer register init
  *myTCCR1A = 0b00000000;
  *myTCCR1B = 0b00000000;
  *myTCCR1C = 0b00000000;
  
  // reset TOV flag
  *myTIFR1 |= 0b00000001;
  
  // enable interrupt
  *myTIMSK1 |= 0b00000001;
}

// ISR timer function
ISR(TIMER1_OVF_vect) {
  // turns timer off
  *myTCCR1B &= 0b11111000;
  
  // loads counter
  *myTCNT1 = total_ticks - output; 
  
  // turns timer on
  *myTCCR1B |= 0b00000001; 

  // xor to toggle PB6
  if (output != 65535) {
    *portB ^= 0b01000000;
  }
}

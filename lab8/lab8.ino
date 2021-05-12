/**
 * Olliver Aikenhead
 * 
 * Lab 8
 * 
 * Using the arduino 2650 ATmega analog to digital converters by using a mux and adc.
 * A photo resistor is attached along with an axial resistor in series. Values get sent over serial
 * to the PC.
 */
 
/* ADC and MUX register init */
volatile unsigned char* myADMUX = (unsigned char*)0x7C;
volatile unsigned char* myADCSRB = (unsigned char*)0x7B;
volatile unsigned char* myADCSRA = (unsigned char*)0x7A;
volatile unsigned int* ADC_DATA = (unsigned int*)0x78;

/* port b init */
volatile unsigned char* port_b = (unsigned char*)0x25;
volatile unsigned char* ddr_b = (unsigned char*)0x24;
volatile unsigned char* pin_b = (unsigned char*)0x23;

/* funtion prototypes */

/**
 * adc_init()
 * ANSI C function
 * initializes the analog to digital converter to be ready to read analog data
 */
void adc_init();

/**
 * adc_read()
 * reads the analog voltage of divider
 */
unsigned int adc_read(unsigned char adc_channel);

/* program execution */
void setup()  {
  // setup the UART
  Serial.begin(9600);
  
  // setup the ADC
  adc_init();
}
void loop() {
  // get the reading from the ADC
  unsigned int adc_reading = adc_read(0);
  // print it to the serial port
  Serial.println(adc_reading);
}

/* function definitions */
// initialize the adc for use
void adc_init() {
  // init register A
  *myADCSRA |= 0b10000000;    // set bit   7 to 1 to enable the ADC
  *myADCSRA &= 0b11011111;    // clear bit 5 to 0 to disable the ADC trigger mode
  *myADCSRA &= 0b11110111;    // clear bit 3 to 0 to disable the ADC interrupt
  *myADCSRA &= 0b11111000;    // clear bit 2-0 to 0 to set prescaler selection to slow reading
  
  // init register B
  *myADCSRB &= 0b11110111;    // clear bit 3 to 0 to reset the channel and gain bits
  *myADCSRB &= 0b11111000;    // clear bit 2-0 to 0 to set free running mode
  
  // init MUX
  *myADMUX &= 0b01111111;     // clear bit 7 to 0 for AVCC analog reference
  *myADMUX |= 0b01000000;     // set bit   6 to 1 for AVCC analog reference
  *myADMUX &= 0b11011111;     // clear bit 5 to 0 for right adjust result
  *myADMUX &= 0b11100000;     // clear bit 4-0 to 0 to reset the channel and gain bits
}

// read data coming from adc 
unsigned int adc_read(unsigned char adc_channel) {
  // reset the channel and gain bits
  *myADMUX &= 0b11100000;
  
  // clear the channel selection bits
  *myADCSRB &= 0b11110111;
  
  // set the channel number
  if(adc_channel > 7) {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel -= 8;
    // set MUX bit 
    *myADCSRB |= 0b00001000;
  }
  
  // set the channel selection bits
  *myADMUX += adc_channel;
  *myADCSRA |= 0b01000000;
  
  // wait for the conversion to complete
  while((*myADCSRA & 0b01000000) != 0);
  // return the result in the ADC data register
  return *ADC_DATA;
}

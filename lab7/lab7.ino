/**
 * Olliver Aikenhead
 * 
 * Lab7 
 * 
 * This program interfaces with an arduino mega 2560. A push button and 7 segment LCD display are connected to the arduino,
 * display increments by one character each time button is pressed. Switch debouncing is accounted for, and onboard EEPROM
 * memory is used to retain previous display after a power cycle.
 */

/* character array, mapped so first element displays a '0' on LCD */
unsigned char chr_array[16] = {0x3F, 0x06, 0x5B, 0x4F, 
                               0x66, 0x6D, 0x7C, 0x07, 
                               0x7F, 0x67, 0x77, 0x7C, 
                               0x39, 0x5E, 0x79, 0x71};

/* register init */
volatile unsigned int* EEPROM_ADDR_REG = (unsigned int*)0x41;           // get a pointer to the EEPROM ADDRESS REGISTER (16-bit)
volatile unsigned char* EEPROM_DATA_REG = (unsigned char*)0x40;         // get a pointer to the EEPROM Data Register (8-bit)
volatile unsigned char* EEPROM_CNTRL_REG = (unsigned char*)0x3F;        // get a pointer to the EEPROM Control Register (8-bit)
volatile unsigned char *myTCCR1A = (unsigned char *)0x80;
volatile unsigned char *myTCCR1B = (unsigned char *)0x81;
volatile unsigned char *myTCCR1C = (unsigned char *)0x82;

/* port init */
volatile unsigned char* ddr_k  = (unsigned char*)0x107;
volatile unsigned char* port_k = (unsigned char*)0x108;
volatile unsigned char* pin_k  = (unsigned char*)0x106;
volatile unsigned char* ddr_f = (unsigned char*)0x30;
volatile unsigned char* port_f = (unsigned char*)0x31;
volatile unsigned char* pin_f = (unsigned char*)0x2F;

/* macros */
#define COUNTER_EEPROM_ADDRESS 0x0025

/* global variables */
unsigned int counter = 0;
unsigned int address = 0x0025;

/* function prototypes */
/**
 * displayValue()
 * takes in count kept by register
 * this displays the value kept in the EEPROM register to the LCD display
 * returns nothing
 */
void displayValue(unsigned int counter);

/**
 * eeprom_write()
 * takes in the address for the register, and data being passed to the register
 * returns nothing 
 */
void eeprom_write(unsigned int address, unsigned char data_in);

/**
 * eeprom_read()
 * takes in address of register
 * reads data from register
 */
unsigned char eeprom_read(unsigned int address);

/* program execution */
void setup() {
  *myTCCR1A = 0x00;
  *myTCCR1B = 0x00;
  *myTCCR1C = 0x00;
  *ddr_k |= 0b11111111;                             // setting ddr_k output
  *ddr_f &= 0b01111111;                             // setting ddr_f input
  *port_f |= 0b10000000;                            // setting port_f input
  counter = eeprom_read(address);                   // counter is reading the address
  counter %= 16;                      
  displayValue(counter);                            // displaying number to LCD
}
void loop () {
  // reading input
  if ((*pin_k & 0b00000001) == 0b00000000) {        // checking pin_k for reading low
    for (int i=0; i<1000; i++) {                    // accounting for button debouncing
      if ((*pin_k & 0b00000001) == 0b00000001) {    // 0 pin is high
        break;
      }
    }
    displayValue(counter);                          // display LCD
    eeprom_write(address, counter);                 // write address to memory
  }
  //displaying initial output 0
  displayValue(counter);
  eeprom_write(address, counter);
}

/* function definitions */
// display value to LCD display
void displayValue(unsigned int counter) {
  *port_k = chr_array[counter];
}

// writes data given to a register
void eeprom_write(unsigned int address, unsigned char data_in) {
  /* Wait for completion of previous write */
  while(*EEPROM_CNTRL_REG & 0x02);
  /* Set up address and Data Registers */
  *EEPROM_ADDR_REG = address;
  *EEPROM_DATA_REG = data_in;
  /* Write logical one to EEMPE */
  *EEPROM_CNTRL_REG |= 0x04;
  /* Write logical zero to EEPE */
  *EEPROM_CNTRL_REG &= ~(0x02);
  /* Write logical one to EEPE */
  *EEPROM_CNTRL_REG |= 0x02;
}

// reads data from register address
unsigned char eeprom_read(unsigned int address) {
  /* Wait for completion of previous write */
  while(*EEPROM_CNTRL_REG & 0x02);
  /* Set up address register */
  *EEPROM_ADDR_REG = address;
  /* Start eeprom read by writing EERE */
  *EEPROM_CNTRL_REG |= 0x01;
  /* Return data from Data Register */
  return *EEPROM_DATA_REG;
}

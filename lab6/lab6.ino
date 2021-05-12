/**
 * Olliver Aikenhead
 * 
 * Lab 6
 * 
 * Writing UART Functions to poll serial port on Arduino
 * Enter number, returns hexadecimal translation to console
 */

/* macro init */
#define RDA 0x80 
#define TBE 0x20 

/* register init */
volatile unsigned char *myUCSR0A = (unsigned char *)0xC0; 
volatile unsigned char *myUCSR0B = (unsigned char *)0xC1; 
volatile unsigned char *myUCSR0C = (unsigned char *)0xC2; 
volatile unsigned char *myUBRR0L = (unsigned char *)0xC4; 
volatile unsigned char *myUBRR0H = (unsigned char *)0xC5; 
volatile unsigned char *myUDR0 = (unsigned char *)0xC6; 

/* function prototypes */

/**
 * U0init(unsigned long baudRate)
 * initializes the serial port, init U(S)ART in different modes
 * returns nothing
 */
void U0init(unsigned long baudRate); 

/**
 * U0kbhit()
 * checks the RDA status bit
 * returns true if bit is set, false if bit is clear
 */
unsigned char U0kbhit(); 

/**
 * U0getchar()
 * return the character recieved by UART
 */
unsigned char U0getchar(); 

/**
 * U0putchar(unsigned char U0pdata)
 * wait until serial port TBE status bit is high
 * take character, and write to transmit buffer
 */
void U0putchar(unsigned char U0pdata); 

/* setup() and loop() are main program execution */
void setup() { 
  U0init(9600); 
} 

void loop() { 
  unsigned char var;
  unsigned char charHex_1;
  unsigned char charHex_2;
  while(U0kbhit() == 0); 
  var = U0getchar(); 
  charHex_1 = charHex( ((int) var)/ 16);
  charHex_2 = charHex( ((int) var)% 16);

  //writing the characters in order in serial monitor
  U0putchar(var);
  U0putchar(':');
  U0putchar(' ');
  U0putchar('0');
  U0putchar('x');
  U0putchar(charHex_1);
  U0putchar(charHex_2);
  U0putchar('\n');
}

/* function definitions */
// init serial port
void U0init( unsigned long baudRate ) { 
  unsigned long FCPU = 16000000; 
  unsigned int tbaud; 
  tbaud = (((unsigned long)(FCPU / ((unsigned long)(16 * baudRate)))) - 1); 
  *myUCSR0A = 0x20; 
  *myUCSR0B = 0x18; 
  *myUCSR0C = 0x06; 
  *myUBRR0H = (tbaud >> 8); 
  *myUBRR0L = (tbaud & 0xFF); 
}

// checking RDA status bit
unsigned char U0kbhit() { 
  return *myUCSR0A & RDA; 
} 

// return char recieved by UART
unsigned char U0getchar() { 
  return *myUDR0; 
} 

// wait until serial is high, then take char and write to transmit buffer
void U0putchar(unsigned char U0pdata) { 
  //wait for TBE to go high 
  while ((*myUCSR0A &= TBE)!= TBE);
  *myUDR0 = U0pdata; //echo character 
} 

unsigned char charHex (int x) {
  return x + ((x<10) ? '0' : ('A'-10));
}

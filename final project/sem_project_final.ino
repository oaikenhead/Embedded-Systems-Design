/*
 * sem_project_final
 *
 * Olliver Aikenhead
 * Russell Kieth
 * 
 * CPE 301 Group 5 Project
 * 
 * May 4, 2021
 */
 
#include <LiquidCrystal.h>
#include <Servo.h>
#include "RTClib.h"
#include "DHT.h"

/* servo init, creating RTC and DHT objects */
Servo myServo;
RTC_DS1307 rtc;
DHT dht(9,DHT11);

/* init lib with nums of interface pins */
LiquidCrystal lcd(2,3,4,5,6,7);

/* port init */
volatile unsigned char *pin_a = (unsigned char*)0x20;
volatile unsigned char *ddr_a = (unsigned char*)0x21;
volatile unsigned char *port_a = (unsigned char*)0x22;
volatile unsigned char *pin_b = (unsigned char*)0x23;
volatile unsigned char *ddr_b = (unsigned char*)0x24;
volatile unsigned char *port_b = (unsigned char*)0x25;
volatile unsigned char *pin_c = (unsigned char*)0x26;
volatile unsigned char *ddr_c = (unsigned char*)0x27;
volatile unsigned char *port_c = (unsigned char*)0x28;
volatile unsigned char *pin_d = (unsigned char*)0x29;
volatile unsigned char *ddr_d = (unsigned char*)0x2A;
volatile unsigned char *port_d = (unsigned char*)0x2B;
volatile unsigned char *ddr_h = (unsigned char*)0x101;
volatile unsigned char *port_h = (unsigned char*)0x102;

/* timer init */
volatile unsigned char *myTCCR1A  = 0x80;
volatile unsigned char *myTCCR1B  = 0x81;
volatile unsigned char *myTCCR1C  = 0x82;
volatile unsigned char *myTIMSK1  = 0x6F;
volatile unsigned char *myTIFR1   = 0x36;
volatile unsigned int  *myTCNT1   = 0x84;

/* global vars */
int temperaturePin = A0;
int buttonOn = false;
unsigned long timeBegin;
unsigned long timeEnd;

/* function prototypes */
/**
 * detectHand()
 * this function detects when the user puts their hand under the faucet
 * returns a value to detect if user put hands under or not
 */
int detectHand();

/** 
 * wash()
 * starts timer countdown of 30sec for washing hands, if user removes hands before 30sec
 * system should blink warningLight(), restart counter, and wait for user to put hands in
 * returns nothing
 */
void wash();

/**
 * warningLight()
 * this function is called in wash(), and blinks a RED LED when user removes hands before
 * washing for 30sec
 * returns nothing
 */
void warningLight();

/**
 * displayHands()
 * displays user has washed hands sufficiently by blinking GREEN light, called in wash()
 * returns nothing
 */
void displayHands();

/**
 * startFan()
 * a fan starts after user is done washing their hands, fan stays on for 15 sec;
 * returns nothing
 */
void startFan();

/**
 * faucetStart()
 * this will simulate the faucet by turning a motor, faucetStart simulates turning faucet
 * on, returns nothing
 */
void faucetStart();

/** 
 * faucetEnd()
 * this simulates turning the faucet off by turning the motor other direction
 * returns nothing
 */
void faucetEnd();

/**
 * timeToLCD()
 * sets the time to count down, and displays it
 * returns nothing
 */
void timeToLCD(int time);

/** 
 * tempToLCD()
 * sends the temp to the LCD screen
 * returns nothing
 */
void tempToLCD();

/**
 * ultraSetup()
 * prepares the sonar to detect
 */
void ultraSetup();

/**
 * prepare timer registers
 */
void setup_timer_regs();

/**
 * delays program a certain amount of time
 * min .00001 second
 * max 1 second
 */
void my_delay(float);

/**
 * delays program in seconds
 * min 0sec
 */
void delaySec(int);

/* program execution */
void setup() {
  // setup hand detect
  ultraSetup();

  // timer register setup
  setup_timer_regs();
  
  // begin serial
  Serial.begin(9600);

  // fan pin 8 output
  *ddr_h |= 0x20;
  *port_h &= 0xDF;
  
  // redLED
  *ddr_b |= 0x10;
  *port_b &= 0xEF;

  // LCD matrix setup
  lcd.begin(16, 2);

  // RTC setup
  rtc.begin();

  // dht setup temperature sensor
  dht.begin();

  // servo motor pin 12
  myServo.attach(13);
}

void loop() {
  lcd.display();
  if (*pin_b & 0xDF) {
    timeToLCD(0,false);
    tempToLCD(false);
    if (*pin_b & 0x20) {
      faucetStart();
      wash();
      Serial.println("all done");
    }
  }
}

/* function definitions */
// detects when hand is under faucet
int detectHand() {
  int counter = 0, distance;
  
  // write to pin 22 high 
  *port_a |= 0x01;
  my_delay(.0001);

  // write pin 22 low
  *port_a &= 0xFE; 
  
  // transmit and record time in duration
  float duration = pulseIn(24,HIGH); 
  distance = .0348*(duration/2);

  // determine if range
  if (distance<20 & distance>6) {
    return 1;
  } else {
    return 0;
  }
}

// wash func def
void wash() {
  tempToLCD(true);
  int currTime = 30;
  
  while (detectHand() == LOW) {
    Serial.println("waiting for hands");
  };

  while ((detectHand() == HIGH) || (currTime >= 0)) {
    timeToLCD(currTime,true);
    delaySec(.5);
    // if user removes hands
    if (detectHand() == LOW) {
      warningLight();
      // if user doesn't replace hands
      if (detectHand() == LOW) {
        faucetEnd();
        lcd.clear();
        break;
      }
      currTime=30;
    } else if (currTime<=0) {    // all done washing hands
      // turn off faucet singal that hands are done
      faucetEnd();
      displayHands();
     
      // clear LCD display
      lcd.clear();
      lcd.home();
      lcd.noDisplay();
      delaySec(2);
      startFan();
      break;
    }
    currTime--;
  }
}

// blink RED light, hands are removed
void warningLight() {
  // display starting clock time to LCD
  timeToLCD(30,true);
  int checkTime = 10;
  
  // flashes warning light until 10 sec is up
  while (detectHand() == LOW && checkTime >= 0) {
    // setting pin 10 to high
    *port_b |= 0x10;
    my_delay(.5);
    // setting pin 10 to low
    *port_b &= 0xEF;
    my_delay(.5);
    checkTime --;
  }
}

// turns LED green, hands washed sufficiently
void displayHands() {
  // buttonOn = false;
  // setting pin 12 to high
  *port_b |= 0x40;
  delaySec(5);
  // setting pin 9 to low
  *port_b &= 0xBF;
}

// fan runs after user finishes washing hands
void startFan() {
  // turn fan on
  analogWrite(8, 100); //pin 8 for fan not full power 
  delaySec(15); //DC motor will draw too much power and ruin LCD display if writen to a high
  digitalWrite(8,LOW);
  *port_h &= 0xDF;
  // trun off after 15 sec
  ;
}

// simulate faucet opening
void faucetStart() {
  // turn servo from 0 degrees to 180
  for (int i=0; i<=180; i++) {
    myServo.write(i);
    my_delay(.015);
  }
}

// simulate faucet closeing
void faucetEnd() {
  // turn servo from 180 degrees to 0
  for (int j=180; j>=0; j--) {
    myServo.write(j);
    my_delay(.015);
  }
}

// sends time as display to LCD screen
void timeToLCD(int sec, bool ifPrint) {
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  if (ifPrint) {
    lcd.setCursor(6, 0);
    if (sec<10) {
      lcd.print(" ");
      lcd.setCursor(7, 0);
      lcd.print(sec);
    } else {
      lcd.print(sec);
    }
  }
}

// sends the temperature as display to LCD
void tempToLCD(bool ifPrint) {
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
  if (ifPrint) {
    lcd.setCursor(6,1);
    // displays in Fahrenheit
    lcd.print(dht.readTemperature(true));
  }
}

// setup function for detectHand() to use sonar
void ultraSetup() {
  // trigger pin 22 on - PA0 output
  *ddr_a |= 0x01;

  // echo on pin 24 - PA2 input
  *ddr_a &= 0xFB;

  *port_a |= 0x04;
}

// init timer registers in setup
void setup_timer_regs() {
  // setup the timer control registers
  *myTCCR1A= 0x00;
  *myTCCR1B= 0X00;
  *myTCCR1C= 0x00;
  *myTIMSK1 |= 0x00;
}

// main delay function used
void my_delay(float Time) {
  // calculate number of ticks
  float freq= 1/(Time*2);
  float FCLK = (16000000);
  float TCLK = 1/(FCLK/256);
  float Twave = (1.0/freq)/2.0; // %50 duty cycle
  unsigned int ticks = Twave/TCLK;
  
  // unsigned int ticks = Time;
  // stop the timer
  *myTCCR1B &= (0x00);
  // set the counts
  *myTCNT1 = (unsigned int) (65536 - ticks);
  // start the timer
  *myTCCR1B |= (0x04);
  // wait for overflow
  while ((*myTIFR1 & (0x01))==0) {}
  // stop the timer
  *myTCCR1B &= (0x00); 
  // reset TOV - You actually have to set this bit to 1 to reset it, not 0 as would be logical (don't ask)
  *myTIFR1 |= (0x01);
}

// delay helper function
void delaySec(int timeSec) {
  for (int i=0; i<=timeSec; i++) {
    my_delay(1);
  }
}

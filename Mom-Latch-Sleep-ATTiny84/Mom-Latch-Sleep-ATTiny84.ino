#include <avr/sleep.h>
#include <avr/interrupt.h>

//ADC disabled as not needed for click tracker (uncomment ADCSRA |= _BV(ADEN) to turn on)

//Guide for how to hookup and program the ATtiny84 with an Arduino uno:
//https://42bots.com/tutorials/programming-attiny84-attiny44-with-arduino-uno/
//Modification to that guide for Arduino IDE 1.6.4 or above:
//https://42bots.com/tutorials/programming-attiny-ics-with-arduino-uno-and-the-arduino-ide-1-6-4-or-above/

//ATTiny84 Pin diagram and connections:
//Column 2 and 5 are the arduino pin mappings

//           Vcc--GND
//        10 PB0--PA0 0 LED1
//         9 PB1--PA1 1 LED2
//           PB3--PA2 2 LED3
//         8 PB2--PA3 3 LED4
// Switch4 6 PA7--PA4 4 Switch1
// Switch3 6 PA6--PA5 5 Switch2

//sleep stuff
long sleep_time = 1.2e6; //20 minutes

const int wakePin = 4;                        //set the wake pin to 4 (PA4) - need to adjust in sleep loop too.
const int statusLED = 10;                     //set satus LED to pin 10 (PB0).
unsigned long previousMillis;                 //sleep timer variable               
boolean delayActive = true;                  //default false, the delay has not started yet

//program stuff
// A current limiting resistor should be connected in line with the LED.
int LED[4] = {0, 1, 2, 3};
// Switches pull to ground
int Switch[4] = {4, 5, 6, 7};
// initalise flags variable array to zero
int flags[4] = {0,0,0,0};

//debounce stuff
int buttonState[4];


void setup() {
//sleep stuff 
    pinMode(wakePin, INPUT_PULLUP);
    pinMode(statusLED, OUTPUT);
    
    // initialize the LED digital pins as outputs.
    pinMode(LED[0], OUTPUT);
    pinMode(LED[1], OUTPUT);
    pinMode(LED[2], OUTPUT);
    pinMode(LED[3], OUTPUT);
    // initialise the switch digital pins as inputs
    pinMode(Switch[0], INPUT_PULLUP);
    pinMode(Switch[1], INPUT_PULLUP);
    pinMode(Switch[2], INPUT_PULLUP);
    pinMode(Switch[3], INPUT_PULLUP); 

    // Flash quick sequence so we know setup has started
    for (int k = 0; k < 4; k = k + 1) {
        if (k % 2 == 0) {
            digitalWrite(statusLED, HIGH);
            }
        else {
            digitalWrite(statusLED, LOW);
            }
        delay(250);
        } // for
    } // setup

void sleep() {

    GIMSK |= _BV(PCIE0);                    // Enable Pin Change Interrupts (adding the PCIE0 bits to GIMSK with the bitwise OR operator)
    PCMSK0 |= _BV(PCINT4);                  // Use pin 4 (PA4) as interrupt pin
    ADCSRA &= ~_BV(ADEN);                   // ADC off
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // Setting the sleep mode- full sleep here

    sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
    digitalWrite(statusLED, LOW);           // Turn off status LED to show sleep enabled
    sei();                                  // Enable interrupts
    sleep_cpu();                            // activate sleep mode 

    //next line of code activated after the interupt

    cli();                                  // Disable interrupts (stops the ISR running again)
    PCMSK0 &= ~_BV(PCINT4);                 // Turn off PA4 as interrupt pin - prevents loop situation here and lets us use that pin for something else
    sleep_disable();                        // Clear SE bit to disable sleep mode
    //ADCSRA |= _BV(ADEN);                  // ADC on (don't need ADC for ClickTracker)

    sei();                                  // Enable interrupts (lets the code use delay)
    digitalWrite(statusLED, HIGH);          // Turn LED on to show sleep over

    previousMillis = millis();              //setting variable previous millis to the current system time
    delayActive = true;
    
    } // sleep

ISR(PCINT0_vect) {
    // This is called when the interrupt occurs, but can be left blank
    }

void loop() {   

      if(delayActive = true){                           //if a sleep cycle has happened and the system has been woken up then this flag will be true
        if(millis() - previousMillis >= sleep_time){    //checking to see if the system time since wake has exceeded the target
          delayActive = false;                          //set the dealy flag to false
          digitalWrite(LED[0], LOW);                    //turn all the LED's off before sleep
          digitalWrite(LED[1], LOW);
          digitalWrite(LED[2], LOW);
          digitalWrite(LED[3], LOW);
          sleep();                                      //sleep loop
        }  
     }
//      do a thing here  
      
      for(int step = 0; step < 4; step++) //cycle through four times for the four switches/LEDs 
      {
         int reading;
         reading = digitalRead(Switch[step]); //read the button state for this step (button) into a local variable

          //this seems to work really well for debouncing as it checks before the if loops then checks on them too. Probably magic.
        
            //if the button has changed
            if (reading != buttonState[step]) {
                buttonState[step] = reading;   //set the state to the new reading         

              if (flags[step] == 0 && reading == LOW) //if the flag is 0 (i.e. LED is off) & the button has been pushed
              {
                digitalWrite(LED[step], HIGH);   // turn the LED on (HIGH is the voltage level)
                flags[step] = 1; // set the flag to show the LED is on
                
              }
              else if (flags[step] == 1 && reading == LOW) //if the flag is 1 (i.e. the LED is on) & the button has been pushed
              {
                digitalWrite(LED[step], LOW); //if the flag is 1 (i.e. LED is on)
                flags[step] = 0; //set the flag to show the LED off
              }
                        
            }
        
      }    
    }

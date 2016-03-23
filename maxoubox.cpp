//
//  maxoubox.c
//  maxoubox
//
//  Created by Olivier on 09/03/2016.
//  Copyright © 2016 Maestun. All rights reserved.
//

#include "maxoubox.h"

// ============================================================================
#pragma mark - CONFIGURATION
// ============================================================================
#define PIN_LED_LATCH 				2
#define PIN_LED_CLOCK 				3
#define PIN_LED_DATA				4

#define PIN_BUTT_LATCH 				5
#define PIN_BUTT_CLOCK 				6
#define PIN_BUTT_DATA				7

#define PIN_RESET					8

#define USE_LCD                     1

#define NUM_BUTTONS                 20
#define CHRONO_DURATION_SEC         (unsigned long)(180)
#define MESSAGE_DURATION_SEC        (unsigned long)(10)

#define MESSAGE_EMPTY               ("                ")
#define MESSAGE_HELLO_LINE_1        ("Bienvenue       ")
#define MESSAGE_HELLO_LINE_2        ("sur la MaxouBox!")

#define MESSAGE_LOST_LINE_1         ("CHRONOMETRE     ")
#define MESSAGE_LOST_LINE_2         ("EXPIRE :-(      ")

#define MESSAGE_WIN_LINE_1          ("Indice:         ")
#define MESSAGE_WIN_LINE_2          ("PAPRIKA MOULU   ")

#define MESSAGE_DEBUG_LINE_1        ("0123456789ABCDEF")
#define MESSAGE_DEBUG_LINE_2        ("FEDCBA9876543210")


// TODO: import http://playground.arduino.cc/Main/ShiftOutX
// shiftOutX gShiftOut(PIN_LED_LATCH, PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, 4); 

#ifdef USE_LCD
	#include "LiquidCrystal_I2C.h"
    // Set the LCD address to 0x27 for a 16 chars and 2 line display
    // SCL => A5
    // SDA => A4
	LiquidCrystal_I2C         gLCD(0x27, 16, 2);
#endif

// sequence: 0...NUM_BUTTON
uint32_t	gButtons;
const int   SEQUENCE[] =                    {3, 6, 11, 1, 12, 7};


// ============================================================================
#pragma mark - Hardware routines
// ============================================================================
void LCD_Setup() {
    dprintln(F("LCD_Setup"));
#ifdef USE_LCD
    gLCD.init();
    gLCD.backlight();
#endif
}


void LCD_Clear() {
#ifdef USE_LCD
    gLCD.clear();
#endif
}


void LCD_Display(const char * aLine1, const char * aLine2) {
    /*dprint(F("LCD1: "));
    dprintln(aLine1);
    dprint(F("LCD2: "));
    dprintln(aLine2);*/
#ifdef USE_LCD
    if(aLine1 != NULL) {
        gLCD.setCursor(0, 0);
        gLCD.print(aLine1);
    }
    if(aLine2 != NULL) {
        gLCD.setCursor(0, 1);
        gLCD.print(aLine2);
    }
#endif
}


uint8_t gLedBuf[4] = {0, 0, 0, 0};
void LED_Setup() {
    dprintln(F("LED_Setup"));
	  pinMode(PIN_LED_LATCH, OUTPUT);
	  pinMode(PIN_LED_CLOCK, OUTPUT);
	  pinMode(PIN_LED_DATA, OUTPUT);
}


#define CHK_BIT(val, b) 		(val & (1 << (b)))
void debug_uint32_t(uint32_t val) {
	  dprint(F("0b"));
    for(uint8_t b = 31; b >= 0; b--) {
        if(CHK_BIT(val, b)) {
            dprint("1");
        }
        else {
            dprint("0");
        }
    }
	  dprintln("next");
}

void debug8(uint8_t val) {
    dprint(F("0b"));
    for(int b = 7; b >= 0; b--) {
        if(CHK_BIT(val, b)) {
            dprint("1");
        }
        else {
            dprint("0");
        }
    }
    dprintln(";");
}

void LED_Enable(int aLed, bool aEnable) {
    
    dprint(F("LED_Enable "));
    Serial.print(aLed, DEC);
    dprint(F(": "));
    dprintln(aEnable ? F("ON") : F("OFF"));

    int idx = aLed / 8;
    int dec = aLed - (8 * idx);
    gLedBuf[idx] = aEnable ? (gLedBuf[idx] | (1 << dec)) : (gLedBuf[idx] & ~(1 << dec));

    digitalWrite(PIN_LED_LATCH, LOW);
    for(int i = 0; i < 4; i++) {
      debug8(gLedBuf[i]);
      shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, gLedBuf[i]);

    }
    digitalWrite(PIN_LED_LATCH, HIGH);
	  delay(20);
}


void LED_EnableAll(bool aEnable) {
	  uint8_t output = aEnable ? 0xff : 0x0;
    dprintln(aEnable ? F("ENABLE ALL LEDS") : F("DISABLE ALL LEDS"));
	  digitalWrite(PIN_LED_LATCH, LOW);
	  shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
	  shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
	  shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
    shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
	  // shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
	  digitalWrite(PIN_LED_LATCH, HIGH);
	  delay(50);
}


void BUTT_Setup() {
    dprintln(F("BUTT_Setup"));
	  pinMode(PIN_BUTT_LATCH, OUTPUT);
	  pinMode(PIN_BUTT_CLOCK, OUTPUT);
	  pinMode(PIN_BUTT_DATA, INPUT);	
}


int BUTT_NumPressed() {
#ifdef USE_SNES_PAD
    // Get the state of all buttons
    // INVERTED, easy parsing of SINGLE button presses at ONCE
    gSNESBits = gSNESPad.getButtons();
    int count = 0;
    if(gSNESBits != 0b1111000000000000) {
        // store all the pressed buttons in a buffer
        for(int button = BTN_B; button <= BTN_R; button = (button << 1)) {
            if(gSNESBits & button) {
              count++;
            }
        }
    }
    return count;
#endif
}


bool BUTT_IsPressed(int aButton) {
  bool ret = false;
#ifdef USE_SNES_PAD
  // Get the state of all buttons
  // INVERTED, easy parsing of SINGLE button presses at ONCE
  gSNESBits = gSNESPad.getButtons();

  if(gSNESBits != 0b1111000000000000) {
    // a button is being pressed !
    if(aButton == -1) {
      // dprintln(F("any press detect"));
      // we want to check if at least a button is being pressed, so exit now
      ret = true;
    }
    else {
      ret = (gSNESBits & aButton);
      /*if(ret) {
        dprint(F("button press detect: "));
        dprintln(aButton);
      }*/
    }
  }

#else
    for (int i = 0; i < NUM_BUTTONS; i++) {
        // TODO: matrix
        if(digitalRead(aButton) != 0 || aButton == -1) {
            ret = true;
            break;
        }
    }
#endif
    return ret;
}



// ============================================================================
#pragma mark - Main Loop
// ============================================================================
unsigned long       gChronoStartMS  = 0;
unsigned long       gRemainingSEC   = 0;
unsigned long       gTimestamp      = 0;
bool                gWon            = false;
unsigned int        gSequenceIndex  = 0;
const unsigned int  gSequenceLength = sizeof(SEQUENCE) / sizeof(SEQUENCE[0]);


void maxou_setup() {
    dprintinit(9600);
    
    // configure GPIO
    BUTT_Setup();
    LED_Setup();
    
    // configure LCD
    LCD_Setup();
    
    // debug mode ?
    /*
    if(BUTT_IsPressed(-1)) {
        LCD_Display(MESSAGE_DEBUG_LINE_1, MESSAGE_DEBUG_LINE_2);
        LED_EnableAll(true);
        delay(MESSAGE_DURATION_SEC * 1000);
        LCD_Display(MESSAGE_EMPTY, MESSAGE_EMPTY);
    }
    */
}


void maxou_loop() {
__reset:
    // ====================================================================
    // 0 - hello screen
    LCD_Display(MESSAGE_HELLO_LINE_1, MESSAGE_HELLO_LINE_2);
    for (int i = 0; i < NUM_BUTTONS; i++) {
        LED_Enable(i, 0);
    }
    
    // sleep until a button is pressed
    while(BUTT_IsPressed(-1) == 0) {
        delay(1);
    }

    LCD_Clear();

    // ====================================================================
    // 1 - launch chronometer
    gChronoStartMS = millis();
    gRemainingSEC = CHRONO_DURATION_SEC;

    gWon = false;
    while (gRemainingSEC != 0 && gWon == false) {

        delay(1);

        // ====================================================================
        // 1.1 - read the currently pressed buttons
        gSequenceIndex = 0;
        // loop thru all pressed buttons
        for(int idx = 0; idx < BUTT_NumPressed(); idx++) {
            // does the current pressed button matches the order in sequence ?
            if(BUTT_IsPressed(SEQUENCE[idx])) {
                gSequenceIndex++;
                LED_Enable(SEQUENCE[idx], true);
            }
            else {
                dprintln(F("** WRONG BUTTON, SEQ BROKEN"));
                LED_EnableAll(false);
                gSequenceIndex = 0;
                break;
            }
        }


        // ====================================================================
        // 1.2 - check the sequence match result
        if(gSequenceIndex == 0) {
            // dprintln(F("** SEQ BROKEN"));
            LCD_Display(MESSAGE_EMPTY, NULL);
        }
        else if(gSequenceIndex == gSequenceLength) {
            dprintln(F("** SEQ OKAY ! win **"));
            gWon = true;
        }
        else {
            //dprintln(F("** SEQ IN PROGRESS"));
            char buf[16] = "";
            for(int i=0; i < gSequenceIndex;i++)
              buf[i] = '.';
            buf[gSequenceIndex] = '\0';
            LCD_Display(buf, NULL);
        }


        // ====================================================================
        // 1.3 - time management - check if a second has elapsed
        gTimestamp = millis();
        if(gTimestamp - gChronoStartMS > 1000) {
            gChronoStartMS = gTimestamp;
            gRemainingSEC--;

            if(gRemainingSEC == 0) {
                dprintln(F("CHRONO ZERO !"));
            }

            unsigned int seconds = (unsigned int)(gRemainingSEC % 60);
            unsigned int minutes = (unsigned int)(gRemainingSEC / 60);
            char buf[16] = "";
            sprintf(buf, "%d:%02d", minutes, seconds);

            dprint("time: ");
            dprintln(buf);
            
            LCD_Display(NULL, buf);
        }
		
		
		// ====================================================================
        // 1.4 - hacky dirty spooky reset management :p
		if(digitalRead(PIN_RESET)) {
			goto __reset;
		}
    }

    
	  dprintln(F("CHRONO SEQ ENDED, CHECK WON"));

    
    // ====================================================================
    // 2 - check game result
    if(gWon) {
        // players have won: all LEDs on
        LED_EnableAll(true);
        
        // show win message
        LCD_Display(MESSAGE_WIN_LINE_1, MESSAGE_WIN_LINE_2);
        delay(MESSAGE_DURATION_SEC * 1000);
        
        // TODO: MAX ensuite on fait quoi ???
        dprintln(F("TODO 1: MAX ensuite on fait quoi ???"));
        while (1) {
            delay(1);
        }        
    }
    else {
        // chronometer has elapsed: all LEDs off
        LED_EnableAll(false);
        
        // blink zero LCD TODO: ya pas + simple dans la lib ?
        for (int i = 0; i < 6; i++) {
            LCD_Display(MESSAGE_EMPTY, "0:00");
            delay(500);
            LCD_Display(MESSAGE_EMPTY, MESSAGE_EMPTY);
            delay(500);
        }

        // show lose message
        LCD_Display(MESSAGE_LOST_LINE_1, MESSAGE_LOST_LINE_2);
        delay(MESSAGE_DURATION_SEC * 1000);
        
        // TODO: MAX ensuite on fait quoi ???
        dprintln(F("TODO 2: MAX ensuite on fait quoi ???"));
        while (1) {
            delay(1);
        }
    }
}

void maxou_test_led_old() {
    LED_EnableAll(true);
    delay(10000);
    LED_EnableAll(false);
    for(uint8_t button = 0; button < NUM_BUTTONS; button++) {
        LED_Enable(button, true);
        delay(2000);
    }
    for(uint8_t button = 0; button < NUM_BUTTONS; button++) {
        LED_Enable(button, false);
        delay(2000);
    }
    delay(1000);
}

void maxou_test_led() {


  LED_EnableAll(false);
  for(int i= 0; i < 32; i++) {
    LED_Enable(i, true);
    delay(20);
  }


  for(;;) {
    
  }
  /*
  
    LED_EnableAll(true);
    delay(2000);
    LED_EnableAll(false);
    pinMode(PIN_RESET, INPUT);
    int led = 0;
    bool enable = true;
    for(;;) {
        if(digitalRead(PIN_RESET)) {
            delay(1000); // debounce
            LED_Enable(led, enable);
            led++;
            if(led == NUM_BUTTONS) {
                dprintln("RESET");
                led = 0;
                enable = !enable;
            }
        }
        delay(10);
    }*/
}




void maxou_test_butt() {
    for(uint8_t button = 0; button < NUM_BUTTONS; button++) {
        if(BUTT_IsPressed(button)) {
            LED_Enable(button, true);
        }
        else {
            LED_Enable(button, false);
        }
    }
}
// EOF


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
#define CHRONO_DURATION_SEC         (unsigned long)(1800)
#define MESSAGE_DURATION_SEC        (unsigned long)(10)
#define RESET_DURATION_SEC          (unsigned long)(10)

#define MESSAGE_EMPTY               ("                ")
#define MESSAGE_HELLO_LINE_1        ("Bienvenue       ")
#define MESSAGE_HELLO_LINE_2        ("sur la MaxouBox!")

#define MESSAGE_LOST_LINE_1         ("CHRONOMETRE     ")
#define MESSAGE_LOST_LINE_2         ("EXPIRE :-(      ")

#define MESSAGE_WIN_LINE_1          ("Indice:         ")
#define MESSAGE_WIN_LINE_2          ("PAPRIKA MOULU   ")

#define MESSAGE_DEBUG_LINE_1        ("0123456789ABCDEF")
#define MESSAGE_DEBUG_LINE_2        ("FEDCBA9876543210")


uint8_t LED_BY_BUTTON[NUM_BUTTONS] = {31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 16, 15, 14, 13, 12, 21, 20, 19, 18, 17};



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
const int   SEQUENCE[] =                    {3, 6, 11, 1};


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
#define CLR_BIT(val, b)   ((val) &= (~(1) << (b)))
#define SET_BIT(val, b)         ((val) |= (1 << (b)))
void debug32(uint32_t val) {
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
/*    
    dprint(F("LED_Enable "));
    Serial.print(aLed, DEC);
    dprint(F(": "));
    dprintln(aEnable ? F("ON") : F("OFF"));
*/
    int bytenum = aLed / 8;
    int bitnum = aLed - (8 * bytenum);
    //static uint8_t gLedBuf[4] = {0, 0, 0, 0};
    bool is_set = CHK_BIT(gLedBuf[bytenum], bitnum);
    bool upd = false;
    if(is_set && !aEnable) {
        // led is on, disable it
        CLR_BIT(gLedBuf[bytenum], bitnum);
        upd = true;
    }
    else if(is_set == false && aEnable) {
        // led is off, enable it
        SET_BIT(gLedBuf[bytenum], bitnum);
        upd = true;
    }
    
    //gLedBuf[idx] = aEnable ? (gLedBuf[idx] | (1 << dec)) : (gLedBuf[idx] & ~(1 << dec));
    if(upd) {
        digitalWrite(PIN_LED_LATCH, LOW);
        for(int i = 0; i < 4; i++) {
            shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, LSBFIRST, gLedBuf[i]);
        }
        digitalWrite(PIN_LED_LATCH, HIGH);
    }
}


void LED_EnableAll(bool aEnable) {
    dprintln(aEnable ? F("ENABLE ALL LEDS") : F("DISABLE ALL LEDS"));
	  uint8_t output = aEnable ? 0xff : 0x0;
    digitalWrite(PIN_LED_LATCH, LOW);
    for(int i = 0; i < 4; i++) {
          gLedBuf[i] = output;
          shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
    }
    digitalWrite(PIN_LED_LATCH, HIGH);
    /*
	  digitalWrite(PIN_LED_LATCH, LOW);
	  shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
	  shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
	  shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
    shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
	  // shiftOut(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, output);
	  digitalWrite(PIN_LED_LATCH, HIGH);
	  delay(50);
    */
}


uint8_t gButtBuf[4] = {0, 0, 0, 0};
void BUTT_Setup() {
    dprintln(F("BUTT_Setup"));
	  pinMode(PIN_BUTT_LATCH, OUTPUT);
	  pinMode(PIN_BUTT_CLOCK, OUTPUT);
	  pinMode(PIN_BUTT_DATA, INPUT);	
    pinMode(PIN_RESET, INPUT);  
}


void BUTT_Update() {
    // latch
    digitalWrite(PIN_BUTT_LATCH, HIGH);
    digitalWrite(PIN_BUTT_LATCH, LOW);
    
    // loop thru n bytes
    for(int bytenum = 0; bytenum < 3 /* nombre de puces 4021 */; bytenum++) {
        gButtBuf[bytenum] = 0;
        // loop thru 8 bits
        for(int bitnum = 0; bitnum < 8; bitnum++) {
            if(digitalRead(PIN_BUTT_DATA)) {
                SET_BIT(gButtBuf[bytenum], bitnum);
                dprint(F("Pressing "));
                Serial.println(((bytenum * 8) + bitnum), DEC);
            }
            
            // clock
            digitalWrite(PIN_BUTT_CLOCK, HIGH);
            digitalWrite(PIN_BUTT_CLOCK, LOW);
        }
    }
}


int BUTT_NumPressed() {
    int count = 0;
    // loop thru n bytes
    for(int bytenum = 0; bytenum < 3 /* nombre de puces 4021 */; bytenum++) {
        // loop thru 8 bits
        for(int bitnum = 0; bitnum < 8; bitnum++) {
            if(CHK_BIT(gButtBuf[bytenum], bitnum)) {
                count++;
            }
        }
    }

if(count>0) {
  dprint(F("numpress "));
  Serial.println(count, DEC);
}
    
    return count;
}


bool BUTT_IsPressed(int aButton) {
    int bytenum = aButton / 8;
    int bitnum = aButton - (bytenum * 8);

    if(aButton == -1) {
        for(int bytenum = 0; bytenum < 3 /* nombre de puces 4021 */; bytenum++) { 
            for(int bitnum = 0; bitnum < 8; bitnum++) {
                if(CHK_BIT(gButtBuf[bytenum], bitnum)) {
                    dprintln("any press");
                    return true;
                }
            }
        }
    }
    else if(CHK_BIT(gButtBuf[bytenum], bitnum)) {
        Serial.print(aButton, DEC);
        dprintln(" pressed *****");
        return true;
    }
    return false;
  
    // latch
    digitalWrite(PIN_BUTT_LATCH, HIGH);
    digitalWrite(PIN_BUTT_LATCH, LOW);
    for(int bytenum = 0; bytenum < 3 /* nombre de puces 4021 */; bytenum++) {
        gButtBuf[bytenum] = 0;
        for(int bitnum = 0; bitnum < 8; bitnum++) {
            if(digitalRead(PIN_BUTT_DATA)) {
                if(aButton == -1) {
                    dprintln("any press");
                    return true;
                }
                else if(aButton == ((bytenum * 8) + bitnum)) {
                  Serial.print(aButton, DEC);
                  dprintln(" pressed *****");
                  return false;
                }
            }
            digitalWrite(PIN_BUTT_CLOCK, HIGH);
            digitalWrite(PIN_BUTT_CLOCK, LOW);  
        }
    }
    return false;
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

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
void maxou_setup() {
    dprintinit(9600);
    
    // configure GPIO
    BUTT_Setup();
    LED_Setup();
    
    // configure LCD
    LCD_Setup();

    // debug mode ?
    if(digitalRead(PIN_RESET)) {
        LCD_Display(MESSAGE_DEBUG_LINE_1, MESSAGE_DEBUG_LINE_2);
        LED_EnableAll(true);
        delay(MESSAGE_DURATION_SEC * 1000);
        maxou_test_butt();
    }
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
    while(BUTT_IsPressed(-1) == false) {
        BUTT_Update();
        delay(10);
    }

    LCD_Clear();

    // ====================================================================
    // 1 - launch chronometer
    gChronoStartMS = millis();
    gRemainingSEC = CHRONO_DURATION_SEC;

    gWon = false;
    bool seq_broken = false;
    while (gRemainingSEC != 0 && gWon == false) {
        delay(10);

        // ====================================================================
        // 1.1 - read the currently pressed buttons
        gSequenceIndex = 0;
        // loop thru all pressed buttons
        BUTT_Update();
        if(seq_broken == false) {
            for(int idx = 0; idx < BUTT_NumPressed(); idx++) {
                // does the current pressed button matches the order in sequence ?
                if(BUTT_IsPressed(SEQUENCE[idx])) {
                    dprintln(F("SEQ++"));
                    gSequenceIndex++;
                    //LED_Enable(SEQUENCE[idx], true);
                    LED_Enable(LED_BY_BUTTON[SEQUENCE[idx]], true);
                }
                else {
                    seq_broken = true;
                    dprintln(F("** WRONG BUTTON, SEQ BROKEN"));
                    LED_EnableAll(false);
                    //LED_Enable(LED_BY_BUTTON[SEQUENCE[idx]], false);
                    gSequenceIndex = 0;
                    break;
                }
            }
        }
        
        // if seq broken, wait for all buttons to be released before letting users play again
        if(seq_broken && BUTT_NumPressed() == 0) {
            seq_broken = false;
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
            for(int i = 0; i < gSequenceIndex;i++)
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

            //dprint("time: ");
            //dprintln(buf);
            
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
        
        // attendre un peu avant de reset
        delay(RESET_DURATION_SEC * 1000);
      
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
        
        // attendre un peu avant de reset
        delay(RESET_DURATION_SEC * 1000);
    }
}

void maxou_test_led() {
    LED_EnableAll(true);
    delay(1000);
    LED_EnableAll(false);
    delay(1000);

    for(int i = 0; i < NUM_BUTTONS; i++) {
        LED_Enable(LED_BY_BUTTON[i], true);
        delay(1000);
        LED_Enable(LED_BY_BUTTON[i], false);
    }
}

void maxou_test_led2() {
  
  // TODO: déterminer quels sont les bon n°s de LED !!!
  // TODO: tester avec LSB/MSB
  LED_EnableAll(false);
  int i = 0;
  for(;;) {
    if(digitalRead(PIN_RESET)) {
        delay(500);
        LED_EnableAll(false);
        LED_Enable(i, true);
        Serial.println(i, DEC);
        i++;
    }
  }
}


void maxou_test_lcd() {
  LCD_Display(MESSAGE_DEBUG_LINE_1, MESSAGE_DEBUG_LINE_2);
  while(1)
    ;
}

void maxou_test_butt() {
    gButtons = 0;
    
    // latch
    digitalWrite(PIN_BUTT_LATCH, HIGH);
    digitalWrite(PIN_BUTT_LATCH, LOW);
    int numbutt = 0;
    for(int bytenum = 0; bytenum < 3 /* nombre de puces 4021 */; bytenum++) {
      gButtBuf[bytenum] = 0;
      for(int bitnum = 0; bitnum < 8; bitnum++) {
        if(digitalRead(PIN_BUTT_DATA)) {
          
          if(CHK_BIT(gButtBuf[bytenum], bitnum)) {
              CLR_BIT(gButtBuf[bytenum], bitnum);
              LED_Enable(LED_BY_BUTTON[numbutt], false);
          }
          else {
              SET_BIT(gButtBuf[bytenum], bitnum);
              LED_Enable(LED_BY_BUTTON[numbutt], true);
          }
          
          dprint("pressed ");
          Serial.println(numbutt, DEC);
          
        }
        else {
          gButtBuf[bytenum] |= (0 << bitnum);
          //LED_Enable(LED_BY_BUTTON[numbutt], false);
        }
        numbutt++;
        digitalWrite(PIN_BUTT_CLOCK, HIGH);
        digitalWrite(PIN_BUTT_CLOCK, LOW);  
      }

    }
    //dprintln(" ");

    if(digitalRead(PIN_RESET)) {
      dprintln("RESET");
    }
}
// EOF


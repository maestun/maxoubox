//
//  maxoubox.c
//  maxoubox
//
//  Created by Olivier on 09/03/2016.
//  Copyright © 2016 Maestun. All rights reserved.
//
#include "Arduino.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "maxoubox.h"

// ============================================================================
#pragma mark - CONFIGURATION
// ============================================================================
// nombre de boutons à appuyer pour gagner
#define SEQUENCE_LENGTH             6

// mettre en commentaire pour ne pas générer la séquence aléatoirement à chaque fois
#define MODE_PUTE

// si pas de mode pute, séquence en dur définie ci-dessous.
// ATTENTION, le nombre de boutons doit être égal à SEQUENCE_LENGTH
// les index des boutons vont de 0...NUM_BUTTONS
uint8_t SEQUENCE[SEQUENCE_LENGTH] =  {0, 1, 2, 3, 4, 5};

#define CHRONO_DURATION_SEC         (unsigned long)(240)
#define MESSAGE_DURATION_SEC        (unsigned long)(10)

// définition des messages à afficher sur le LCD
#define MESSAGE_EMPTY               ("                ")
#define MESSAGE_HELLO_LINE_1        ("Boite d'analyse ")
#define MESSAGE_HELLO_LINE_2        ("moleculaire v2.0")

#define MESSAGE_LOST_LINE_1         ("CHRONOMETRE     ")
#define MESSAGE_LOST_LINE_2         ("EXPIRE :-(      ")

#define MESSAGE_CHRONO              ("DECOMPTE: ")

#define MESSAGE_WIN_LINE_1          ("CE N'EST PAS UNE")
#define MESSAGE_WIN_LINE_2          ("CRISE CARDIAQUE ")

#define MESSAGE_DEBUG_LINE_1        ("0123456789ABCDEF")
#define MESSAGE_DEBUG_LINE_2        ("FEDCBA9876543210")


// ============================================================================
#pragma mark - CONFIGURATION HARDWARE (spécifique à chaque maxoubox)
// ============================================================================
#define NUM_BUTTONS                 20

#define PIN_LED_LATCH 				      2
#define PIN_LED_CLOCK 				      3
#define PIN_LED_DATA				        4

#define PIN_BUTT_LATCH 				      5
#define PIN_BUTT_CLOCK 				      6
#define PIN_BUTT_DATA				        7

#define PIN_RESET					          8
uint8_t LED_BY_BUTTON[NUM_BUTTONS] = {31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 16, 15, 14, 13, 12, 21, 20, 19, 18, 17};


// Set the LCD address to 0x27 for a 16 chars and 2 line display
// SCL => A5
// SDA => A4
LiquidCrystal_I2C                   gLCD(0x27, 16, 2);
// sequence: 0...NUM_BUTTON

#define dprintinit(x)               Serial.begin(x)
#define dprint(x)                   Serial.print(x)
#define dprintln(x)                 Serial.println(x)


// ============================================================================
#pragma mark - LCD SCREEN
// ============================================================================
void LCD_Setup() {
    dprintln(F("LCD_Setup"));
    gLCD.init();
    gLCD.backlight();
}


void LCD_Clear() {
    gLCD.clear();
}


void LCD_Display(const char * aLine1, const char * aLine2) {
    if(aLine1 != NULL) {
        gLCD.setCursor(0, 0);
        gLCD.print(aLine1);
    }
    if(aLine2 != NULL) {
        gLCD.setCursor(0, 1);
        gLCD.print(aLine2);
    }
}


// ============================================================================
#pragma mark - LED ARRAY
// ============================================================================
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
}


// ============================================================================
#pragma mark - BUTTON ARRAY
// ============================================================================
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
  
    return count;
}


bool BUTT_IsPressed(int aButton) {
    int bytenum = aButton / 8;
    int bitnum = aButton - (bytenum * 8);

    if(aButton == -1) {
        for(int bytenum = 0; bytenum < 3 /* nombre de puces 4021 */; bytenum++) { 
            for(int bitnum = 0; bitnum < 8; bitnum++) {
                if(CHK_BIT(gButtBuf[bytenum], bitnum)) {
                    return true;
                }
            }
        }
    }
    else if(CHK_BIT(gButtBuf[bytenum], bitnum)) {
        return true;
    }
    return false;
}


// ============================================================================
#pragma mark - MAIN PROGRAM
// ============================================================================
unsigned long       gChronoStartMS  = 0;
unsigned long       gRemainingSEC   = 0;
unsigned long       gTimestamp      = 0;
bool                gWon            = false;
unsigned int        gSequenceIndex  = 0;

void maxou_pute() {
// init pseudo seq
#ifdef MODE_PUTE
    randomSeed(analogRead(A6));
    for(int seq = 0; seq < SEQUENCE_LENGTH;) {
        uint8_t num = random(0, NUM_BUTTONS);
        bool found = false;
        for(int i = 0; i < seq; i++) {
            if(SEQUENCE[i] == num) {
                found = true;
                break;
            }
        }

        if(!found) {
            SEQUENCE[seq] = num;
            Serial.println(SEQUENCE[seq], DEC);
            seq++;
        }
    }
#endif
}


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
    }
}


void maxou_loop() {
__reset:

    maxou_pute();

    LED_EnableAll(false);
    BUTT_Update();
    
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
    int prev_numpress = 0;
    
    while (gRemainingSEC != 0 && gWon == false) {
        delay(10);

        // ====================================================================
        // 1.1 - read the currently pressed buttons
        gSequenceIndex = 0;

        // get buttons
        BUTT_Update();

        // check we didn't unpress anything since last loop
        if(BUTT_NumPressed() < prev_numpress) {
            // unpress occurred, sequence broken
            dprintln(F("** SEQ BROKE RELEASE"));
            seq_broken = true;
            prev_numpress = 0;
            LCD_Display(MESSAGE_EMPTY, NULL);
            LED_EnableAll(false);
        }
        else {
            // save current number of buttons pressed
            prev_numpress = BUTT_NumPressed();
        }

        
        // loop thru all pressed buttons
        if(seq_broken == false) { // test: if we just broke the sequence, wait for all buttons to be released
            for(int idx = 0; idx < BUTT_NumPressed(); idx++) {
                // does the current pressed button matches the order in sequence AND
                // is seq index equal to num pressed ?
                if(BUTT_IsPressed(SEQUENCE[idx])) {
                    gSequenceIndex++;
                    dprint(F("SEQ++ "));
                    Serial.println(gSequenceIndex, DEC);
                    LED_Enable(LED_BY_BUTTON[SEQUENCE[idx]], true);
                }
                else {
                    seq_broken = true;
                    dprintln(F("** WRONG BUTTON, SEQ BROKEN"));
                    LED_EnableAll(false);
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
        BUTT_Update();
        if(gSequenceIndex == 0) {
            dprintln(F("** SEQ BROKE BAD BUTT"));
            prev_numpress = 0;
            LCD_Display(MESSAGE_EMPTY, NULL);
            LED_EnableAll(false);
        }
        else if(gSequenceIndex == SEQUENCE_LENGTH) {
            dprintln(F("** SEQ OKAY ! win **"));
            gWon = true;
        }
        else {
            //dprintln(F("** SEQ IN PROGRESS"));
            
            char buf[16] = "";
            for(int i = 0; i < gSequenceIndex;i++)
              buf[i] = 'o';
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
            sprintf(buf, "%s%d:%02d", MESSAGE_CHRONO, minutes, seconds);

            //dprint("time: ");
            //dprintln(buf);
            
            LCD_Display(NULL, buf);
        }
		
		
		    // ====================================================================
        // 1.4 - hacky dirty spooky reset management :p
		    if(digitalRead(PIN_RESET)) {
			    goto __reset;
		    }
    } // end while remaining secs
    
	  dprintln(F("CHRONO SEQ ENDED, CHECK WON"));

    
    // ====================================================================
    // 2 - check game result
    if(gWon) {
        // players have won: all LEDs on
        LED_EnableAll(true);
        
        // show win message
        LCD_Display(MESSAGE_WIN_LINE_1, MESSAGE_WIN_LINE_2);
        delay(MESSAGE_DURATION_SEC * 1000);
    }
    else {
        // chronometer has elapsed: all LEDs off
        LED_EnableAll(false);
        
        // blink zero LCD TODO: ya pas + simple dans la lib ?
        for (int i = 0; i < 6; i++) {
            LCD_Display(MESSAGE_EMPTY, "0:00");
            LED_EnableAll(true);
            delay(300);
            LCD_Display(MESSAGE_EMPTY, MESSAGE_EMPTY);
            LED_EnableAll(false);
            delay(300);
        }

        // show lose message
        LCD_Display(MESSAGE_LOST_LINE_1, MESSAGE_LOST_LINE_2);
        delay(MESSAGE_DURATION_SEC * 1000);
    }
}

// EOF

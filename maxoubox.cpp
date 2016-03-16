//
//  maxoubox.c
//  maxoubox
//
//  Created by Olivier on 09/03/2016.
//  Copyright © 2016 Maestun. All rights reserved.
//

#include "maxoubox.h"

#ifdef USE_SNES_PAD
  SNESpaduino gSNESPad(2, 3, 4); // latch, clock, data
  uint16_t    gSNESBits;
  const int   SEQUENCE[] =                    {BTN_Y, BTN_X, BTN_A, BTN_B, BTN_UP};
#else
  const int   SEQUENCE[] =                    {3, 6, 11, 1, 12, 7};
#endif


#ifdef __APPLE__
unsigned long millis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    __int64_t milliSeconds = (__int64_t)((__int64_t)time.tv_sec * (__int64_t)1000);
    __int64_t returnValue = milliSeconds + (time.tv_usec / 1000);
    return (unsigned long) returnValue;
}
#endif


/**
 * Retourne le nombre de millisecondes depuis le démarrage du programme.
 *
 * @return Le nombre de millisecondes depuis le démarrage du programme sous la forme d'un
 * nombre entier sur 64 bits (unsigned long long).
 */
unsigned long long superMillis() {
    static unsigned long nbRollover = 0;
    static unsigned long previousMillis = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis < previousMillis) {
        nbRollover++;
    }
    previousMillis = currentMillis;
    
    unsigned long long finalMillis = nbRollover;
    finalMillis <<= 32;
    finalMillis +=  currentMillis;
    return finalMillis;
}


// ============================================================================
#pragma mark - Hardware routines
// ============================================================================
void LCD_Setup() {
    dprintln(F("LCD_Setup"));
}


void LCD_Display(const char * aLine1, const char * aLine2) {
#ifdef __APPLE__
    printf("%s\n", aLine1);
    printf("%s\n", aLine2);
#else
    dprint(F("LCD1: "));
    dprintln(aLine1);
    dprint(F("LCD2: "));
    dprintln(aLine2);
#endif
}


void LED_Setup() {
    dprintln(F("LED_Setup"));
#ifdef __APPLE__
#else
#endif
}


void LED_Enable(int aLed, int aEnable) {
#ifdef __APPLE__
    printf("LED_Enable %d: %s\n", aLed, aEnable ? "ON" : "OFF");
#else
    /*
    dprint(F("LED_Enable "));
    Serial.print(aLed, DEC);
    dprint(F(": "));
    dprintln(aEnable ? F("ON") : F("OFF"));
    */
#endif
}


void BUTT_Setup() {
    dprintln(F("BUTT_Setup"));
#ifdef __APPLE__
#else
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
      if(ret) {
        dprint(F("button press detect: "));
        dprintln(aButton);
      }
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
unsigned long     gChronoStartMS  = 0;
unsigned long     gRemainingSEC    = 0;
unsigned long     gTimestamp      = 0;
bool            gWon            = false;
unsigned int    gSequenceIndex  = 0;
const int       gSequenceLength = sizeof(SEQUENCE) / sizeof(SEQUENCE[0]);

void maxou_setup() {
    dprintinit(9600);
    
    // configure GPIO
    BUTT_Setup();
    LED_Setup();
    
    // configure LCD
    LCD_Setup();
    
    // debug mode ?
    if(BUTT_IsPressed(-1)) {
        LCD_Display(MESSAGE_DEBUG_LINE_1, MESSAGE_DEBUG_LINE_2);
        for (int i = 0; i < NUM_BUTTONS; i++) {
            LED_Enable(i, 1);
        }
        
        delay(MESSAGE_DURATION_SEC * 1000);

        LCD_Display(MESSAGE_EMPTY, MESSAGE_EMPTY);
    }
}


bool SEQ_Okay(int aIndex) {
    bool ret = true;
    if(BUTT_IsPressed(-1)) {
        // at least one button is pressed
        for(int idx = 0; idx <= aIndex; idx++) {
            // check that every button up to index is the right one
            ret &= BUTT_IsPressed(SEQUENCE[idx]);
            if(ret == false) {
                // sequence broken, no need to continue
                break;
            }
        }
    }
    else {
      ret = false;
    }
    return ret;
}


void maxou_loop() {
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
    

    // ====================================================================
    // 1 - launch chronometer
    gChronoStartMS = millis();
    gRemainingSEC = CHRONO_DURATION_SEC;

    dprint(F("CHRONO START "));
    Serial.print(gChronoStartMS, DEC);
    dprint(F(" / "));
    Serial.println(gRemainingSEC, DEC);
    
    while (gRemainingSEC != 0 || gWon) {

        delay(500);
      
        // check buttons
        if(SEQ_Okay(gSequenceIndex)) {
            dprint(F("** SEQ OK index "));
            Serial.println(gSequenceIndex, DEC);

            // sequence is still okay: light up all the right buttons
            for(int idx = 0; idx <= gSequenceIndex; idx++) {
                LED_Enable(SEQUENCE[idx], 1);
            }
            
            gSequenceIndex++;
            if(gSequenceIndex == gSequenceLength) {
                // sequence finished, we won !
                gWon = true;
            }
        }
        else {
            // invalid sequence: turn everything off
            dprintln(F("** SEQ BROKEN"));
            gSequenceIndex = 0;
            for (int i = 0; i < NUM_BUTTONS; i++) {
                LED_Enable(i, 0);
            }
        }

        // ====================================================================
        // 1.5 - time management - check if a second has elapsed
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
            LCD_Display(MESSAGE_EMPTY, buf);
        }
    }


    dprintln(F("CHRONO SEQ ENDED, CHECK WON"));
    
    // ====================================================================
    // 2 - check game result
    if(gWon) {
        // players have won: all LEDs on
        for (int i = 0; i < NUM_BUTTONS; i++) {
            LED_Enable(i, 1);
        }
        
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
        for (int i = 0; i < NUM_BUTTONS; i++) {
            LED_Enable(i, 0);
        }
        
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




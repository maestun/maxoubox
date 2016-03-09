//
//  maxoubox.c
//  maxoubox
//
//  Created by Olivier on 09/03/2016.
//  Copyright © 2016 Maestun. All rights reserved.
//

#include "maxoubox.h"

#ifdef __APPLE__
unsigned long millis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    int64_t milliSeconds = (int64_t)((int64_t)time.tv_sec * (int64_t)1000);
    int64_t returnValue = milliSeconds + (time.tv_usec / 1000);
    return (unsigned long) returnValue;
}
#endif


// ============================================================================
#pragma mark - Hardware routines
// ============================================================================
void LCD_Setup() {
    printf("LCD_Setup\n");
}


void LCD_Display(char * aLine1, char * aLine2) {
    printf("%s\n", aLine1);
    printf("%s\n", aLine2);
}


void LED_Setup() {

}


void LED_Enable(int aLed, int aEnable) {
    printf("LED_Enable %d: %s\n", aLed, aEnable ? "ON" : "OFF");
}


void BUTT_Setup() {
    printf("BT_Setup\n");
}


int BUTT_IsPressed(int aButton) {
    int ret = 0;
    
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if(digitalRead(aButton) != 0 || aButton == -1) {
            ret = 1;
            break;
        }
    }
    return ret;
}


// ============================================================================
#pragma mark - Main Loop
// ============================================================================
unsigned long   gChronoStartMS  = 0;
long            gRemainingMS    = 0;
unsigned int    gWon            = 0;
unsigned int    gSequenceIndex  = 0;


void maxou_setup() {
    
    // configure button GPIO
    BUTT_Setup();
    
    // configure LCD
    LCD_Setup();
    
    // debug mode ?
    if(BUTT_IsPressed(-1)) {
        LCD_Display(MESSAGE_DEBUG_LINE_1, MESSAGE_DEBUG_LINE_2);
        for (int i = 0; i < NUM_BUTTONS; i++) {
            LED_Enable(i, 1);
        }
        
        delay(MESSAGE_DURATION_MS);

        LCD_Display(MESSAGE_EMPTY, MESSAGE_EMPTY);
        for (int i = 0; i < NUM_BUTTONS; i++) {
            LED_Enable(i, 0);
        }
    }
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
    gRemainingMS = CHRONO_DURATION_MS;
    while (gRemainingMS >= 0) {

        // check buttons
        
        
        
        
        
        // check if a second has elapsed
        unsigned long ts = millis();
        if(ts - gRemainingMS > 1000) {
            gRemainingMS -= 1000;
            if(gRemainingMS < 0) {
                gRemainingMS = 0;
            }
            int seconds = (int)(gRemainingMS / 1000);
            int minutes = (int)(gRemainingMS / 60);
            char buf[16] = MESSAGE_EMPTY;
            sprintf(buf, "%d:%0d", minutes, seconds);
            LCD_Display(MESSAGE_EMPTY, buf);
        }
    }
    
    
    // ====================================================================
    // 2 - check game result
    if(gWon) {
        // players have won: all LEDs on
        for (int i = 0; i < NUM_BUTTONS; i++) {
            LED_Enable(i, 1);
        }
        
        // show win message
        LCD_Display(MESSAGE_WIN_LINE_1, MESSAGE_WIN_LINE_2);
        delay(MESSAGE_DURATION_MS);
        
        // TODO: MAX ensuite on fait quoi ???
        while (1) {
            delay(1);
        }

        
    }
    else {
        // chronometer has elapsed: all LEDs off
        for (int i = 0; i < NUM_BUTTONS; i++) {
            LED_Enable(i, 0);
        }
        
        // blink zero LCD TODO: ya pas + simlpe dans la lib ?
        for (int i = 0; i < 6; i++) {
            LCD_Display(MESSAGE_EMPTY, "0:00");
            delay(500);
            LCD_Display(MESSAGE_EMPTY, MESSAGE_EMPTY);
            delay(500);
        }

        // show lose message
        LCD_Display(MESSAGE_HELLO_LINE_1, MESSAGE_HELLO_LINE_2);
        delay(MESSAGE_DURATION_MS);
        
        // TODO: MAX ensuite on fait quoi ???
        while (1) {
            delay(1);
        }
    }
}




//
//  maxoubox.h
//  maxoubox
//
//  Created by Olivier on 09/03/2016.
//  Copyright © 2016 Maestun. All rights reserved.
//

#ifdef __APPLE__

#include <sys/time.h>
#include <stdio.h>
#define dprintinit(x)
#define dprint(x)               printf(x)
#define dprintln(x)             printf(x); printf("\n")
#define F(x)                    (x)
#define digitalRead(x)          (0)
#define digitalWrite(x)
#define pinMode(x, y)
#define delay(ms)
#define INPUT                   0
unsigned long millis();


#else

#include "Arduino.h"
#include "SNESpaduino.h"


#define dprintinit(x)           Serial.begin(x)
#define dprint(x)               Serial.print(x)
#define dprintln(x)             Serial.println(x)

#endif


#define USE_SNES_PAD                1
#define USE_LCD                     1


#define NUM_BUTTONS                 15
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


// prototypes

// checks if the given button is pressed. Pass -1 as parameter to check any button


void maxou_setup();
void maxou_loop();



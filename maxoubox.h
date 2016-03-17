//
//  maxoubox.h
//  maxoubox
//
//  Created by Olivier on 09/03/2016.
//  Copyright © 2016 Maestun. All rights reserved.
//

#ifndef maxoubox_h
#define maxoubox_h



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

#define BTN_B 0x01
#define BTN_Y 0x02
#define BTN_SELECT 0x04
#define BTN_START 0x08
#define BTN_UP 0x10
#define BTN_DOWN 0x20
#define BTN_LEFT 0x40
#define BTN_RIGHT 0x80
#define BTN_A 0x100
#define BTN_X 0x200
#define BTN_L 0x400
#define BTN_R 0x800

#else

#include "Arduino.h"
#include "SNESpaduino.h"

#define dprintinit(x)           Serial.begin(x)
#define dprint(x)               Serial.print(x)
#define dprintln(x)             Serial.println(x)

#define USE_SNES_PAD                1

#endif




#define NUM_BUTTONS                 15
#define CHRONO_DURATION_SEC         (unsigned long)(180)
#define MESSAGE_DURATION_SEC        (unsigned long)(10)

#define MESSAGE_EMPTY               ("                ")
#define MESSAGE_HELLO_LINE_1        ("Bienvenue       ")
#define MESSAGE_HELLO_LINE_2        ("sur la MaxouBox!")

#define MESSAGE_LOST_LINE_1         ("Le chronometre  ")
#define MESSAGE_LOST_LINE_2         ("a expiré :(     ")

#define MESSAGE_WIN_LINE_1          ("Indice:         ")
#define MESSAGE_WIN_LINE_2          ("PAPRIKA MOULU   ")

#define MESSAGE_DEBUG_LINE_1        ("0123456789ABCDEF")
#define MESSAGE_DEBUG_LINE_2        ("FEDCBA9876543210")


// prototypes

// checks if the given button is pressed. Pass -1 as parameter to check any button


void maxou_setup();
void maxou_loop();

#endif /* maxoubox_h */

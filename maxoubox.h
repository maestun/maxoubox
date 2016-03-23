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
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define dprintinit(x)           Serial.begin(x)
#define dprint(x)               Serial.print(x)
#define dprintln(x)             Serial.println(x)

#endif

// prototypes
void maxou_setup();
void maxou_loop();
void maxou_test_led();
void maxou_test_lcd();
void maxou_test_butt();



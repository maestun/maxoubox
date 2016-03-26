//
//  maxoubox.h
//  maxoubox
//
//  Created by Olivier on 09/03/2016.
//  Copyright © 2016 Maestun. All rights reserved.
//

#include "Arduino.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define dprintinit(x)           Serial.begin(x)
#define dprint(x)               Serial.print(x)
#define dprintln(x)             Serial.println(x)

// prototypes
void maxou_setup();
void maxou_loop();

// EOF

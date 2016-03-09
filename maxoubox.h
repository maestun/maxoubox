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

#define F(x)                    (x)
#define digitalRead(x)          (0)
#define digitalWrite(x)
#define pinMode(x, y)
#define delay(ms)            

#define INPUT                   0

unsigned long millis();

#endif




#include <stdio.h>


#define NUM_BUTTONS                 15

#define SEQUENCE                    [3, 6, 11, 1, 12, 7]

#define CHRONO_DURATION_MS          (3 * 60 * 1000)
#define MESSAGE_DURATION_MS         (30 * 1000)

#define MESSAGE_EMPTY               F("                ")
#define MESSAGE_HELLO_LINE_1        F("Bienvenue       ")
#define MESSAGE_HELLO_LINE_2        F("sur la MaxouBox!")

#define MESSAGE_LOST_LINE_1         F("Le chronometre  ")
#define MESSAGE_LOST_LINE_2         F("a expiré :(     ")

#define MESSAGE_WIN_LINE_1          F("Indice:         ")
#define MESSAGE_WIN_LINE_2          F("PAPRIKA MOULU   ")

#define MESSAGE_DEBUG_LINE_1        F("0123456789ABCDEF")
#define MESSAGE_DEBUG_LINE_2        F("FEDCBA9876543210")




// prototypes

// checks if the given button is pressed. Pass -1 as parameter to check any button


void maxou_setup();
void maxou_loop();

#endif /* maxoubox_h */

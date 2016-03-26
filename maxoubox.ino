#include "maxoubox.h"

/*
 TODO:
 - utiliser une carte SD pour la configuration
 - tracer le nb de parties gagnées (avec le temps) et perdues (avec le sequence max)
 - rajouter du son pour partie gagnée / perdue
 */

void setup() {
  maxou_setup();
}

void loop() {
  maxou_loop();
}


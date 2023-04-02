#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "pico/stdlib.h"

// LED defines.
#ifdef PICO_DEFAULT_LED_PIN
#define LED_PIN PICO_DEFAULT_LED_PIN
#define LED_STATE_ON (!(PICO_DEFAULT_LED_PIN_INVERTED))
#endif

// VALVE Defines
#define VALVE0_PIN 2
#define VALVE1_PIN 3

#endif  // APP_CONFIG_H

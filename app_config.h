#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "pico/types.h"

// LED defines.
constexpr uint LED_RED_PIN = 18;
constexpr bool LED_RED_ON = false;
constexpr uint LED_GRN_PIN = 19;
constexpr bool LED_GRN_ON = false;
constexpr uint LED_BLU_PIN = 20;
constexpr bool LED_BLU_ON = false;

// VALVE Defines
constexpr uint VALVE0_PIN = 2;
constexpr bool VALVE0_ON = true;
constexpr uint VALVE1_PIN = 3;
constexpr bool VALVE1_ON = true;

#endif  // APP_CONFIG_H

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "pico/stdlib.h"

// LED defines.
#ifdef PICO_DEFAULT_LED_PIN
#define LED_PIN PICO_DEFAULT_LED_PIN
#define LED_STATE_ON (!(PICO_DEFAULT_LED_PIN_INVERTED))
#endif

#if 0
// UART defines
#if defined(PICO_DEFAULT_UART_TX_PIN) && defined(PICO_DEFAULT_UART_RX_PIN) && \
    defined(PICO_DEFAULT_UART)
#define CFG_BOARD_UART_BAUDRATE 115200
#define UART_DEV PICO_DEFAULT_UART
#define UART_TX_PIN PICO_DEFAULT_UART_TX_PIN
#define UART_RX_PIN PICO_DEFAULT_UART_RX_PIN
#endif
#endif

// VALVE Defines
#define VALVE0_PIN 2
#define VALVE1_PIN 3

#if 0
#include "hardware/spi.h"
// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for
// information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19
#endif

#if 0
#include "hardware/i2c.h"
// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for
// information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#endif

#endif  // APP_CONFIG_H

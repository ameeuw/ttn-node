/*******************************************************************************
 *
 *  File:         bsf_lopy.h
 *
 *  Description:  Board Support File for Pycom LoPy4.
 *
 *  Copyright:    Copyright (c) 2021 Leonel Lopes Parente
 *
 *  License:      MIT License. See accompanying LICENSE file.
 *
 *  Author:       Leonel Lopes Parente
 *
 *  Description:  This Board has no onboard USB and no onboard display.
 *                Optionally an external display can be connected.
 *                Has onboard LED but this is a special WS2812 RGB LED which requires
 *                a separate library and is currently not supported by LMIC-node.
 *
 *                The standard SPI pins defined in the BSP do not match the
 *                GPIO pins that the SX1276 LoRa chip is connected to.
 *                LMIC uses the default SPI parameters for initializing the SPI bus
 *                which will not work here. Therefore the SPI object is explicitly
 *                initialized with the correct pins (see boardInit() below).
 *
 *                For firmware upload and serial monitor use a USB to serial adapter.
 *                Lopy4 is made for use with MicroPython and does not provide
 *                automatic upload for C/C++ firmware (and no GPIO0 button).
 *                To put an ESP32 in firmware upload mode: First press the GPIO0
 *                button, then while keeping it pressed press the reset button,
 *                then release the reset button and then release the GPIO0 button.
 *                Because the Lopy4 does not have a GPIO0 button, instead of pressing
 *                the GPIO0 button connect a wire from GPIO0 (labeled '2' on the PCB)
 *                to GND and instead of releasing the GPIO0 button remove the wire again.
 *                Pycom also sells an Expansion board with onboard USB that
 *                supports firmware upload and serial over USB. The upload is not
 *                automatic however and the ESP32 must still be put in upload mode
 *                manually which still requires a wire because it also has no GPIO0 button.
 *                After firmware upload the Lopy4 must be manually reset (with button).
 *
 *                LoRa DIO0, DIO1 and DIO2 are all wired (via diodes) to the same single GPIO port.
 *
 *                WARNING: The 3.3V pin is OUTPUT ONLY don't use it to power the board!!
 *                         The board must be powered on pin Vin with +3.5V to +5V.
 *
 *                Connect the LoRa module and optional display
 *                according to below connection details.
 *
 *                CONNECTIONS AND PIN DEFINITIONS:
 *
 *                Indentifiers between parentheses are defined in the board's
 *                Board Support Package (BSP) which is part of the Arduino core.
 *
 *                Leds                GPIO
 *                ----                ----
 *                WS2812 <--------->   0  (LED_BUILTIN) WS2812 RGB LED, not regular LED!
 *                                        pin labeled '2' on the PCB.
 *
 *                I2C [display]       GPIO
 *                ---                 ----
 *                SDA   <――――――――――>  12  (SDA)
 *                SCL   <――――――――――>  13  (SCL)
 *
 *                SPI                 GPIO
 *                ---                 ----
 *                MOSI  <――――――――――>  22  (MOSI)
 *                MISO  <――――――――――>  37  (MISO)
 *                SCK   <――――――――――>  13  (SCK)
 *                SS    <――――――――――>  18  (SS)
 *
 *                LoRa                GPIO
 *                ----                ----
 *                MOSI  <――――――――――>  27  (LORA_MOSI)
 *                MISO  <――――――――――>  19  (LORA_MISO)
 *                SCK   <――――――――――>   5  (LORA_SCK)
 *                NSS   <――――――――――>  17  (LORA_CS)
 *                RST                  -
 *                RXTX                 -
 *                DIO0  <――――――――――>  23  (LORA_IRQ, LORA_IO0)
 *                DIO1  <――――――――――>  23  (LORA_IRQ, LORA_IO1)
 *                DIO2  <――――――――――>  23  (LORA_IRQ, LORA_IO2)
 *
 *                Serial              GPIO
 *                ------              ----
 *                RX    <――――――――――>   3  (RX) pin labeled '0' on the PCB.
 *                TX    <――――――――――>   1  (TX) pin labeled '1' on the PCB.
 *
 *  Docs:         https://docs.platformio.org/en/latest/boards/espressif32/lopy4.html
 *
 *  Identifiers:  LMIC-node
 *                    board-id:      lopy4
 *                PlatformIO
 *                    board:         lopy4
 *                    platform:      espressif32
 *                Arduino
 *                    board:         ARDUINO_LoPy4
 *                    architecture:  ARDUINO_ARCH_ESP32
 *
 ******************************************************************************/

#pragma once

#ifndef bsf_lopy_H_
#define bsf_lopy_H_

enum class InitType
{
    Hardware,
    PostInitSerial
};
enum class PrintTarget
{
    All,
    Serial,
    Display
};

#include "lmic.h"
#include "hal/hal.h"
#include "SPI.h"
#include "Arduino.h"

#define DEVICEID_DEFAULT "lopy" // Default deviceid value

// Wait for Serial
// Can be useful for boards with MCU with integrated USB support.
// #define WAITFOR_SERIAL_SECONDS_DEFAULT 10   // -1 waits indefinitely

// LMIC Clock Error
// This is only needed for slower 8-bit MCUs (e.g. 8MHz ATmega328 and ATmega32u4).
// Value is defined in parts per million (of MAX_CLOCK_ERROR).
// #ifndef LMIC_CLOCK_ERROR_PPM
//     #define LMIC_CLOCK_ERROR_PPM 0
// #endif

#endif // bsf_lopy_H_
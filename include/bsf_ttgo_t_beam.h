/*******************************************************************************
 *
 *  File:         bsf_ttgo_t_beam.h
 *
 *  Description:  Board Support File for TTGO T-Beam (aka T22) V0.5, V0.6 and V0.7.
 *
 *  Copyright:    Copyright (c) 2021 Leonel Lopes Parente
 *
 *  License:      MIT License. See accompanying LICENSE file.
 *
 *  Author:       Leonel Lopes Parente
 *
 *  Description:  This board has onboard USB (provided by onboard USB to serial).
 *                It supports automatic firmware upload and serial over USB.
 *                No onboard display. Optionally an external display con be connected.
 *                Also has onboard GPS which is not used by LMIC-node.
 *
 *                Connect an optional display according to below connection details.
 *
 *                CONNECTIONS AND PIN DEFINITIONS:
 *
 *                Indentifiers between parentheses are defined in the board's
 *                Board Support Package (BSP) which is part of the Arduino core.
 *
 *                Leds                GPIO
 *                ----                ----
 *                LED   <――――――――――>  14  (LED_BUILTIN)  Active-high
 *
 *                I2C [display]       GPIO
 *                ----                ----
 *                SDA   <――――――――――>  21  (SDA)
 *                SCL   <――――――――――>  22  (SCL)
 *                RST                  -
 *
 *                SPI/LoRa            GPIO
 *                ---                 ----
 *                MOSI  <――――――――――>  27  (MOSI) (LORA_MOSI)
 *                MISO  <――――――――――>  19  (MISO) (LORA_MISO)
 *                SCK   <――――――――――>   5  (SCK)  (LORA_SCK)
 *                NSS   <――――――――――>  18  (SS)   (LORA_CS)
 *                RST   <――――――――――>  23         (LORA_RST)
 *                DIO0  <――――――――――>  26         (LORA_IO0)
 *                DIO1  <――――――――――>  33         (LORA_IO1)
 *                DIO2  <――――――――――>  32         (LORA_IO2)
 *
 *                GPS                 GPIO
 *                ---                 ----
 *                RX    <――――――――――>  15
 *                TX    <――――――――――>  12
 *
 *  Docs:         https://docs.platformio.org/en/latest/boards/espressif32/ttgo-t-beam.html
 *
 *  Identifiers:  LMIC-node
 *                    board:         ttgo_t_beam
 *                PlatformIO
 *                    board:         ttgo-t-beam
 *                    platform:      espressif32
 *                Arduino
 *                    board:         ARDUINO_T_Beam
 *                    architecture:  ARDUINO_ARCH_ESP32
 *
 ******************************************************************************/

#pragma once

#ifndef BSF_TTGO_T_BEAM_H_
#define BSF_TTGO_T_BEAM_H_

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

#ifdef USE_DISPLAY
#include "U8x8lib.h"
#endif

#define DEVICEID_DEFAULT "ttgo-tbeam" // Default deviceid value

// Wait for Serial
// Can be useful for boards with MCU with integrated USB support.
// #define WAITFOR_SERIAL_SECONDS_DEFAULT 10   // -1 waits indefinitely

// LMIC Clock Error
// This is only needed for slower 8-bit MCUs (e.g. 8MHz ATmega328 and ATmega32u4).
// Value is defined in parts per million (of MAX_CLOCK_ERROR).
// #ifndef LMIC_CLOCK_ERROR_PPM
//     #define LMIC_CLOCK_ERROR_PPM 0
// #endif

#endif // BSF_TTGO_T_BEAM_H_
/*******************************************************************************
 *
 *  File:         bsf_ttgo_lora32_v2.h
 *
 *  Description:  Board Support File for TTGO LoRa32 (aka T3) v2.0.
 *
 *  Copyright:    Copyright (c) 2021 Leonel Lopes Parente
 *
 *  License:      MIT License. See accompanying LICENSE file.
 *
 *  Author:       Leonel Lopes Parente
 *
 *  Description:  This board has onboard USB (provided by onboard USB to serial).
 *                It supports automatic firmware upload and serial over USB.
 *                Has onboard display.
 *
 *                ██ DIO1 MUST BE MANUALLY WIRED TO GPIO33 (see below) ██
 *
 *                LED_BUILTIN and I2C SCL share the same pin.
 *                Therefore cannot use USE_DISPLAY and USE_LED together.
 *
 *                OLED_RST and LORA_RST are defined in BSP but neither is connected to GPIO.
 *
 *                CONNECTIONS AND PIN DEFINITIONS:
 *
 *                Indentifiers between parentheses are defined in the board's
 *                Board Support Package (BSP) which is part of the Arduino core.
 *
 *                Leds                GPIO
 *                ----                ----
 *                LED   <――――――――――>  22  (LED_BUILTIN) (SCL)  Active-low
 *                                        Shared with I2C SCL
 *
 *                I2C/Display         GPIO
 *                ---                 ----
 *                SDA   <――――――――――>  21  (SDA)  (OLED_SDA)
 *                SCL   <――――――――――>  22  (SCL)  (OLED_SCL) (LED_BUILTIN)
 *                RST                  -  OLED_RST is defined in BSP but not wired to GPIO
 *
 *                SPI/LoRa            GPIO
 *                MOSI  <――――――――――>  27  (MOSI) (LORA_MOSI)
 *                MISO  <――――――――――>  19  (MISO) (LORA_MISO)
 *                SCK   <――――――――――>   5  (SCK)  (LORA_SCK)
 *                NSS   <――――――――――>  18  (SS)   (LORA_CS)
 *                RST   <――――――――――>   -         Not LORA_RST
 *                DIO0  <――――――――――>  26         (LORA_IRQ)
 *                DIO1  <---------->  33  ██ NOT WIRED on PCB ██
 *                DIO2                 -  Not needed for LoRa
 *
 *  Docs:         https://docs.platformio.org/en/latest/boards/espressif32/ttgo-lora32-v2.html
 *
 *  Identifiers:  LMIC-node
 *                    board:         ttgo_lora32_v2
 *                PlatformIO
 *                    board:         ttgo-lora32-v2
 *                    platform:      espressif32
 *                Arduino
 *                    board:         ARDUINO_TTGO_LoRa32_V2
 *                    architecture:  ARDUINO_ARCH_ESP32
 *
 ******************************************************************************/

#pragma once

#ifndef BSF_TTGO_LORA32_V2_H_
#define BSF_TTGO_LORA32_V2_H_

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

#ifdef USE_DISPLAY
#include "U8x8lib.h"
#endif

#define DEVICEID_DEFAULT "ttgo-lora32-v2" // Default deviceid value

// Wait for Serial
// Can be useful for boards with MCU with integrated USB support.
// #define WAITFOR_SERIAL_SECONDS_DEFAULT 10   // -1 waits indefinitely

// LMIC Clock Error
// This is only needed for slower 8-bit MCUs (e.g. 8MHz ATmega328 and ATmega32u4).
// Value is defined in parts per million (of MAX_CLOCK_ERROR).
// #ifndef LMIC_CLOCK_ERROR_PPM
//     #define LMIC_CLOCK_ERROR_PPM 0
// #endif

#if defined(USE_DISPLAY) && defined(USE_LED) && LED_PIN == LED_ONBOARD
#error Invalid option: USE_DISPLAY and USE_LED cannot be used together.
#endif

#endif // BSF_TTGO_LORA32_V2_H_
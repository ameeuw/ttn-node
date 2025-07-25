/*******************************************************************************
 *
 *  File:         LMIC-node.h
 *
 *  Function:     LMIC-node main header file.
 *
 *  Copyright:    Copyright (c) 2021 Leonel Lopes Parente
 *                Portions Copyright (c) 2018 Terry Moore, MCCI
 *
 *                Permission is hereby granted, free of charge, to anyone
 *                obtaining a copy of this document and accompanying files to do,
 *                whatever they want with them without any restriction, including,
 *                but not limited to, copying, modification and redistribution.
 *                The above copyright notice and this permission notice shall be
 *                included in all copies or substantial portions of the Software.
 *
 *                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY.
 *
 *  License:      MIT License. See accompanying LICENSE file.
 *
 *  Author:       Leonel Lopes Parente
 *
 ******************************************************************************/

#ifndef LMIC_NODE_H_
#define LMIC_NODE_H_

#include "include.h"

const dr_t DefaultABPDataRate = DR_SF7;
const s1_t DefaultABPTxPower = 14;

extern bool boardInit(InitType initType);

#ifdef USE_SERIAL
extern HardwareSerial &serial;
#endif

#ifdef USE_DISPLAY
// Create U8x8 instance for SSD1306 OLED display (no reset) using hardware I2C.
extern U8X8_SSD1306_128X64_NONAME_HW_I2C display;
#endif

// Forward declarations

extern TaskHandle_t LmicTask;
void lmicTask(void *);

void processDownlink(ostime_t eventTimestamp, uint8_t fPort, uint8_t *data, uint8_t dataLength);
void onLmicEvent(void *pUserData, ev_t ev);
void initLmic(void);
int16_t getSnrTenfold(void);
int16_t getRssi(int8_t snr);
lmic_tx_error_t scheduleUplink(uint8_t fPort, uint8_t *data, uint8_t dataLength, bool confirmed);
extern bool joined;

#ifndef DO_WORK_INTERVAL_SECONDS     // Should be set in platformio.ini
#define DO_WORK_INTERVAL_SECONDS 300 // Default 5 minutes if not set
#endif

#define TIMESTAMP_WIDTH 12 // Number of columns to display eventtime (zero-padded)
#define MESSAGE_INDENT TIMESTAMP_WIDTH + 3

// Determine which LMIC library is used
#ifdef _LMIC_CONFIG_PRECONDITIONS_H_
#define MCCI_LMIC
#else
#define CLASSIC_LMIC
#endif

#if !defined(ABP_ACTIVATION) && !defined(OTAA_ACTIVATION)
#define OTAA_ACTIVATION
#endif

enum class ActivationMode
{
    OTAA,
    ABP
};
#ifdef OTAA_ACTIVATION
const ActivationMode activationMode = ActivationMode::OTAA;
#else
const ActivationMode activationMode = ActivationMode::ABP;
#endif

#if defined(ABP_ACTIVATION) && defined(OTAA_ACTIVATION)
#error Only one of ABP_ACTIVATION and OTAA_ACTIVATION can be defined.
#endif

#if defined(ABP_ACTIVATION) && defined(ABP_DEVICEID)
const char deviceId[] = ABP_DEVICEID;
#elif defined(DEVICEID)
const char deviceId[] = DEVICEID;
#else
const char deviceId[] = DEVICEID_DEFAULT;
#endif

// Allow WAITFOR_SERIAL_SECONDS to be defined in platformio.ini.
// If used it shall be defined in the [common] section.
// The common setting will only be used for boards that have
// WAITFOR_SERIAL_SECONDS_DEFAULT defined (in BSP) with a value != 0
#if defined(WAITFOR_SERIAL_SECONDS_DEFAULT) && WAITFOR_SERIAL_SECONDS_DEFAULT != 0
#ifdef WAITFOR_SERIAL_SECONDS
#define WAITFOR_SERIAL_S WAITFOR_SERIAL_SECONDS
#else
#define WAITFOR_SERIAL_S WAITFOR_SERIAL_SECONDS_DEFAULT
#endif
#else
#define WAITFOR_SERIAL_S 0
#endif

#if defined(ABP_ACTIVATION) && defined(CLASSIC_LMIC)
#error Do NOT use ABP activation when using the deprecated IBM LMIC framework library. \
           On The Things Network V3 this will cause a downlink message for EVERY uplink message \
           because it does properly handle MAC commands.
#endif

#ifdef OTAA_ACTIVATION
#if !defined(ASCII_DEVEUI) || !defined(ASCII_APPEUI) || !defined(ASCII_APPKEY)
#error One or more LoRaWAN keys (ASCII_DEVEUI, ASCII_APPEUI, ASCII_APPKEY) are not defined.
#endif
#else
// ABP activation
#if !defined(ABP_DEVADDR) || !defined(ABP_NWKSKEY) || !defined(ABP_APPSKEY)
#error One or more LoRaWAN keys (ABP_DEVADDR, ABP_NWKSKEY, ABP_APPSKEY) are not defined.
#endif
#endif

// Determine if a valid region is defined.
// This actually has little effect because
// CLASSIC LMIC: defines CFG_eu868 by default,
// MCCI LMIC: if no region is defined it
// sets CFG_eu868 as default.
#if (                                                                         \
    (defined(CLASSIC_LMIC) && !(defined(CFG_eu868) || defined(CFG_us915))) || \
    (defined(MCCI_LMIC) && !(defined(CFG_as923) || defined(CFG_as923jp) || defined(CFG_au915) || defined(CFG_eu868) || defined(CFG_in866) || defined(CFG_kr920) || defined(CFG_us915))))
#Error No valid LoRaWAN region defined
#endif

#ifndef MCCI_LMIC
#define LMIC_ERROR_SUCCESS 0
typedef int lmic_tx_error_t;

// In MCCI LMIC these are already defined.
// This macro can be used to initalize an array of event strings
#define LEGACY_LMIC_EVENT_NAME_TABLE__INIT                            \
    "<<zero>>",                                                       \
        "EV_SCAN_TIMEOUT", "EV_BEACON_FOUND",                         \
        "EV_BEACON_MISSED", "EV_BEACON_TRACKED", "EV_JOINING",        \
        "EV_JOINED", "EV_RFU1", "EV_JOIN_FAILED", "EV_REJOIN_FAILED", \
        "EV_TXCOMPLETE", "EV_LOST_TSYNC", "EV_RESET",                 \
        "EV_RXCOMPLETE", "EV_LINK_DEAD", "EV_LINK_ALIVE"

// If working on an AVR (or worried about memory size), you can use this multi-zero
// string and put this in a single const F() string to store it in program memory.
// Index through this counting up from 0, until you get to the entry you want or
// to an entry that begins with a \0.
#define LEGACY_LMIC_EVENT_NAME_MULTISZ__INIT \
    "<<zero>>\0" \ "EV_SCAN_TIMEOUT\0"       \
                   "EV_BEACON_FOUND\0"       \
                   "EV_BEACON_MISSED\0"      \
                   "EV_BEACON_TRACKED\0"     \
                   "EV_JOINING\0"            \
                   "EV_JOINED\0"             \
                   "EV_RFU1\0"               \
                   "EV_JOIN_FAILED\0"        \
                   "EV_REJOIN_FAILED\0"      \
                   "EV_TXCOMPLETE\0"         \
                   "EV_LOST_TSYNC\0"         \
                   "EV_RESET\0"              \
                   "EV_RXCOMPLETE\0"         \
                   "EV_LINK_DEAD\0"          \
                   "EV_LINK_ALIVE\0"
#endif // LMIC_MCCI

#ifdef MCCI_LMIC
static const char *const lmicEventNames[] = {LMIC_EVENT_NAME_TABLE__INIT};
static const char *const lmicErrorNames[] = {LMIC_ERROR_NAME__INIT};
#else
static const char *const lmicEventNames[] = {LEGACY_LMIC_EVENT_NAME_TABLE__INIT};
#endif

#endif // LMIC_NODE_H_

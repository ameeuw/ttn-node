/*******************************************************************************
 *
 *  File:         radiolib-node.h
 *
 *  Function:     RadioLib-node main header file.
 *
 *  Copyright:    Copyright (c) 2024 LMIC Node Rebuild Project
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
 *  Author:       LMIC Node Rebuild Project
 *
 ******************************************************************************/

#ifndef RADIOLIB_NODE_H_
#define RADIOLIB_NODE_H_

#include "include.h"

// Default LoRaWAN parameters
const uint8_t DefaultABPDataRate = 5; // SF7BW125 equivalent
const int8_t DefaultABPTxPower = 14;

extern bool boardInit(InitType initType);

#ifdef USE_SERIAL
extern HardwareSerial &serial;
#endif

#ifdef USE_DISPLAY
// Create U8x8 instance for SSD1306 OLED display (no reset) using hardware I2C.
extern U8X8_SSD1306_128X64_NONAME_HW_I2C display;
#endif

// Forward declarations
extern TaskHandle_t RadioLibTask;
void radioLibTask(void *);

void processDownlink(uint32_t timestamp, uint8_t fPort, uint8_t *data, uint8_t dataLength);
void onRadioLibEvent(uint16_t eventType, void* eventData);
void initRadioLib(void);
int16_t getSnrTenfold(void);
int16_t getRssi(void);
int16_t scheduleUplink(uint8_t fPort, uint8_t *data, uint8_t dataLength, bool confirmed);
extern bool joined;

#ifndef DO_WORK_INTERVAL_SECONDS     // Should be set in platformio.ini
#define DO_WORK_INTERVAL_SECONDS 300 // Default 5 minutes if not set
#endif

#define TIMESTAMP_WIDTH 12 // Number of columns to display eventtime (zero-padded)
#define MESSAGE_INDENT TIMESTAMP_WIDTH + 3

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
// USE_SERIAL defined and WAITFOR_SERIAL_SECONDS > 0 defined.
#ifndef WAITFOR_SERIAL_SECONDS_DEFAULT
#define WAITFOR_SERIAL_SECONDS_DEFAULT 10
#endif
#ifndef WAITFOR_SERIAL_SECONDS
#define WAITFOR_SERIAL_SECONDS WAITFOR_SERIAL_SECONDS_DEFAULT
#endif

// Constants for display layout (from original LMIC implementation)
#define COL_0 0
#define FRMCNTRS_ROW 6
#define MONITOR_SPEED 115200
#define WAITFOR_SERIAL_S WAITFOR_SERIAL_SECONDS

// Device ID defaults
#ifndef DEVICEID_DEFAULT
#define DEVICEID_DEFAULT "RadioLib-Node"
#endif

// RadioLib specific declarations
extern Module* radio;
extern LoRaWANNode* node;

// RadioLib event types
#define RADIOLIB_EVENT_JOIN_ACCEPT 0x01
#define RADIOLIB_EVENT_JOIN_FAILED 0x02
#define RADIOLIB_EVENT_UPLINK_COMPLETE 0x03
#define RADIOLIB_EVENT_DOWNLINK_RECEIVED 0x04
#define RADIOLIB_EVENT_LINK_CHECK 0x05

// RadioLib error handling
typedef int16_t radiolib_error_t;
#define RADIOLIB_ERR_NONE 0

#endif // RADIOLIB_NODE_H_

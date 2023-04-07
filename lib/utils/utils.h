#ifndef UTILS_H_
#define UTILS_H_

#ifdef USE_DISPLAY
#include <Wire.h>
#include "U8x8lib.h"
#endif

#ifdef USE_DISPLAY
#define ROW_0 0
#define ROW_1 1
#define ROW_2 2
#define ROW_3 3
#define ROW_4 4
#define ROW_5 5
#define ROW_6 6
#define ROW_7 7
#define HEADER_ROW ROW_0
#define DEVICEID_ROW ROW_1
#define INTERVAL_ROW ROW_2
#define TIME_ROW ROW_4
#define EVENT_ROW ROW_5
#define STATUS_ROW ROW_6
#define FRMCNTRS_ROW ROW_7
#define COL_0 0
#define ABPMODE_COL 10
#define CLMICSYMBOL_COL 14
#define TXSYMBOL_COL 15
#endif

void displayTxSymbol(bool visible);
void printChars(Print &printer, char ch, uint8_t count, bool linefeed = false);
void printSpaces(Print &printer, uint8_t count, bool linefeed = false);
void printHex(Print &printer, uint8_t *bytes, size_t length = 1, bool linefeed = false, char separator = 0);
void setTxIndicatorsOn(bool on = true);
void initDisplay();
bool initSerial(unsigned long speed = 115200, int16_t timeoutSeconds = 0);

#ifdef USE_SERIAL
extern HardwareSerial &serial;
#endif

#ifdef USE_DISPLAY
extern U8X8_SSD1306_128X64_NONAME_HW_I2C display;
#endif
#endif

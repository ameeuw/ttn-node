#include "utils.h"

#if defined(USE_SERIAL) || defined(USE_DISPLAY)

void printEvent(ostime_t timestamp,
                const char *const message,
                PrintTarget target,
                bool clearDisplayStatusRow,
                bool eventLabel)
{
#ifdef USE_DISPLAY
    if (target == PrintTarget::All || target == PrintTarget::Display)
    {
        display.clearLine(TIME_ROW);
        display.setCursor(COL_0, TIME_ROW);
        display.print(F("Time:"));
        display.print(timestamp);
        display.clearLine(EVENT_ROW);
        if (clearDisplayStatusRow)
        {
            display.clearLine(STATUS_ROW);
        }
        display.setCursor(COL_0, EVENT_ROW);
        display.print(message);
    }
#endif

#ifdef USE_SERIAL
    // Create padded/indented output without using printf().
    // printf() is not default supported/enabled in each Arduino core.
    // Not using printf() will save memory for memory constrainted devices.
    String timeString(timestamp);
    uint8_t len = timeString.length();
    uint8_t zerosCount = TIMESTAMP_WIDTH > len ? TIMESTAMP_WIDTH - len : 0;

    if (target == PrintTarget::All || target == PrintTarget::Serial)
    {
        printChars(serial, '0', zerosCount);
        serial.print(timeString);
        serial.print(":  ");
        if (eventLabel)
        {
            serial.print(F("Event: "));
        }
        serial.println(message);
    }
#endif
}

void printEvent(ostime_t timestamp,
                ev_t ev,
                PrintTarget target,
                bool clearDisplayStatusRow)
{
#if defined(USE_DISPLAY) || defined(USE_SERIAL)
    printEvent(timestamp, lmicEventNames[ev], target, clearDisplayStatusRow, true);
#endif
}

void printSessionKeys(void)
{
#if defined(USE_SERIAL) && defined(MCCI_LMIC)
    u4_t networkId = 0;
    devaddr_t deviceAddress = 0;
    u1_t networkSessionKey[16];
    u1_t applicationSessionKey[16];
    LMIC_getSessionKeys(&networkId, &deviceAddress,
                        networkSessionKey, applicationSessionKey);

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Network Id: "));
    serial.println(networkId, DEC);

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Device Address: "));
    serial.println(deviceAddress, HEX);

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Application Session Key: "));
    printHex(serial, applicationSessionKey, 16, true, '-');

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Network Session Key:     "));
    printHex(serial, networkSessionKey, 16, true, '-');
#endif
}

void printFrameCounters(PrintTarget target)
{
#ifdef USE_DISPLAY
    if (target == PrintTarget::Display || target == PrintTarget::All)
    {
        display.clearLine(FRMCNTRS_ROW);
        display.setCursor(COL_0, FRMCNTRS_ROW);
        display.print(F("Up:"));
        display.print(LMIC.seqnoUp);
        display.print(F(" Dn:"));
        display.print(LMIC.seqnoDn);
    }
#endif

#ifdef USE_SERIAL
    if (target == PrintTarget::Serial || target == PrintTarget::All)
    {
        printSpaces(serial, MESSAGE_INDENT);
        serial.print(F("Up: "));
        serial.print(LMIC.seqnoUp);
        serial.print(F(",  Down: "));
        serial.println(LMIC.seqnoDn);
    }
#endif
}

void printDownlinkInfo(void)
{
#if defined(USE_SERIAL) || defined(USE_DISPLAY)

    uint8_t dataLength = LMIC.dataLen;
    // bool ackReceived = LMIC.txrxFlags & TXRX_ACK;

    int16_t snrTenfold = getSnrTenfold();
    int8_t snr = snrTenfold / 10;
    int8_t snrDecimalFraction = snrTenfold % 10;
    int16_t rssi = getRssi(snr);

    uint8_t fPort = 0;
    if (LMIC.txrxFlags & TXRX_PORT)
    {
        fPort = LMIC.frame[LMIC.dataBeg - 1];
    }

#ifdef USE_DISPLAY
    display.clearLine(EVENT_ROW);
    display.setCursor(COL_0, EVENT_ROW);
    display.print(F("RX P:"));
    display.print(fPort);
    if (dataLength != 0)
    {
        display.print(" Len:");
        display.print(LMIC.dataLen);
    }
    display.clearLine(STATUS_ROW);
    display.setCursor(COL_0, STATUS_ROW);
    display.print(F("RSSI"));
    display.print(rssi);
    display.print(F(" SNR"));
    display.print(snr);
    display.print(".");
    display.print(snrDecimalFraction);
#endif

#ifdef USE_SERIAL
    printSpaces(serial, MESSAGE_INDENT);
    serial.println(F("Downlink received"));

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("RSSI: "));
    serial.print(rssi);
    serial.print(F(" dBm,  SNR: "));
    serial.print(snr);
    serial.print(".");
    serial.print(snrDecimalFraction);
    serial.println(F(" dB"));

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Port: "));
    serial.println(fPort);

    if (dataLength != 0)
    {
        printSpaces(serial, MESSAGE_INDENT);
        serial.print(F("Length: "));
        serial.println(LMIC.dataLen);
        printSpaces(serial, MESSAGE_INDENT);
        serial.print(F("Data: "));
        printHex(serial, LMIC.frame + LMIC.dataBeg, LMIC.dataLen, true, ' ');
    }
#endif
#endif
}

void printHeader(void)
{
#ifdef USE_DISPLAY
    display.clear();
    display.setCursor(COL_0, HEADER_ROW);
    display.print(F("LMIC-node"));
#ifdef ABP_ACTIVATION
    display.drawString(ABPMODE_COL, HEADER_ROW, "ABP");
#endif
#ifdef CLASSIC_LMIC
    display.drawString(CLMICSYMBOL_COL, HEADER_ROW, "*");
#endif
    display.drawString(COL_0, DEVICEID_ROW, deviceId);
    display.setCursor(COL_0, INTERVAL_ROW);
#endif

#ifdef USE_SERIAL
    serial.println(F("\n\nLMIC-node\n"));
    serial.print(F("Device-id:     "));
    serial.println(deviceId);
    serial.print(F("LMIC library:  "));
#ifdef MCCI_LMIC
    serial.println(F("MCCI"));
#else
    serial.println(F("Classic [Deprecated]"));
#endif
    serial.print(F("Activation:    "));
#ifdef OTAA_ACTIVATION
    serial.println(F("OTAA"));
#else
    serial.println(F("ABP"));
#endif
#if defined(LMIC_DEBUG_LEVEL) && LMIC_DEBUG_LEVEL > 0
    serial.print(F("LMIC debug:    "));
    serial.println(LMIC_DEBUG_LEVEL);
#endif
    if (activationMode == ActivationMode::OTAA)
    {
        serial.println();
    }
#endif
}

void printChars(Print &printer, char ch, uint8_t count, bool linefeed)
{
    for (uint8_t i = 0; i < count; ++i)
    {
        printer.print(ch);
    }
    if (linefeed)
    {
        printer.println();
    }
}

void printSpaces(Print &printer, uint8_t count, bool linefeed)
{
    printChars(printer, ' ', count, linefeed);
}

void printHex(Print &printer, uint8_t *bytes, size_t length, bool linefeed, char separator)
{
    for (size_t i = 0; i < length; ++i)
    {
        if (i > 0 && separator != 0)
        {
            printer.print(separator);
        }
        if (bytes[i] <= 0x0F)
        {
            printer.print('0');
        }
        printer.print(bytes[i], HEX);
    }
    if (linefeed)
    {
        printer.println();
    }
}

void setTxIndicatorsOn(bool on)
{
    if (on)
    {
#ifdef USE_LED
        led.on();
#endif
#ifdef USE_DISPLAY
        displayTxSymbol(true);
#endif
    }
    else
    {
#ifdef USE_LED
        led.off();
#endif
#ifdef USE_DISPLAY
        displayTxSymbol(false);
#endif
    }
}

#endif // USE_SERIAL || USE_DISPLAY

#ifdef USE_DISPLAY
uint8_t transmitSymbol[8] = {0x18, 0x18, 0x00, 0x24, 0x99, 0x42, 0x3c, 0x00};

void initDisplay()
{
    display.begin();
    display.setFont(u8x8_font_victoriamedium8_r);
}

void displayTxSymbol(bool visible = true)
{
    if (visible)
    {
        display.drawTile(TXSYMBOL_COL, ROW_0, 1, transmitSymbol);
    }
    else
    {
        display.drawGlyph(TXSYMBOL_COL, ROW_0, char(0x20));
    }
}
#endif // USE_DISPLAY

#ifdef USE_SERIAL
bool initSerial(unsigned long speed, int16_t timeoutSeconds)
{
    // Initializes the serial port.
    // Optionally waits for serial port to be ready.
    // Will display status and progress on display (if enabled)
    // which can be useful for tracing (e.g. ATmega328u4) serial port issues.
    // A negative timeoutSeconds value will wait indefinitely.
    // A value of 0 (default) will not wait.
    // Returns: true when serial port ready,
    //          false when not ready.

    serial.begin(speed);

#if WAITFOR_SERIAL_S != 0
    if (timeoutSeconds != 0)
    {
        bool indefinite = (timeoutSeconds < 0);
        uint16_t secondsLeft = timeoutSeconds;
#ifdef USE_DISPLAY
        display.setCursor(0, ROW_1);
        display.print(F("Waiting for"));
        display.setCursor(0, ROW_2);
        display.print(F("serial port"));
#endif

        while (!serial && (indefinite || secondsLeft > 0))
        {
            if (!indefinite)
            {
#ifdef USE_DISPLAY
                display.clearLine(ROW_4);
                display.setCursor(0, ROW_4);
                display.print(F("timeout in "));
                display.print(secondsLeft);
                display.print('s');
#endif
                --secondsLeft;
            }
            delay(1000);
        }
#ifdef USE_DISPLAY
        display.setCursor(0, ROW_4);
        if (serial)
        {
            display.print(F("Connected"));
        }
        else
        {
            display.print(F("NOT connected"));
        }
#endif
    }
#endif

    return serial;
}
#endif

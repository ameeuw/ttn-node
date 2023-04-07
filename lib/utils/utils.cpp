#include "utils.h"

#if defined(USE_SERIAL) || defined(USE_DISPLAY)

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

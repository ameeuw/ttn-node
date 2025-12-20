#if defined(BOARD_LOPY) || defined(BOARD_DEV)
#include "bsf_lopy.h"

// RadioLib module instances (LoPy uses SX1272)
SX1272 radioModule = new Module(17, 23, RADIOLIB_NC, 23);
LoRaWANNode lorawanNode(&radioModule, &EU868);

#ifdef USE_SERIAL
HardwareSerial &serial = Serial;
#endif

#ifdef USE_LED
#error "Invalid option: USE_LED. Onboard WS2812 RGB LED is currently not supported."
#endif

#ifdef USE_DISPLAY
// Create U8x8 instance for SSD1306 OLED display (no reset) using hardware I2C.
U8X8_SSD1306_128X64_NONAME_HW_I2C display(/*rst*/ U8X8_PIN_NONE, /*scl*/ SCL, /*sda*/ SDA);
#endif

bool boardInit(InitType initType)
{
    // This function is used to perform board specific initializations.
    // Required as part of standard template.

    // InitType::Hardware        Must be called at start of setup() before anything else.
    // InitType::PostInitSerial  Must be called after initSerial() before other initializations.

    bool success = true;
    switch (initType)
    {
    case InitType::Hardware:
        // Note: Serial port and display are not yet initialized and cannot be used use here.

        // Initialize standard SPI object with non-standard SPI pins for LoRa module.
        // These pins will be remembered and will not change if any library
        // later calls SPI.begin() without parameters.
        SPI.begin(5, 19, 27, 18);
        
        // Set global RadioLib pointers
        radio = &radioModule;
        node = &lorawanNode;
        break;

    case InitType::PostInitSerial:
        // Note: If enabled Serial port and display are already initialized here.
        // No actions required for this board.
        break;
    }
    return success;
}

#endif
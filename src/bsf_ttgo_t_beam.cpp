#ifdef BOARD_TBEAM
#include "bsf_ttgo_t_beam.h"

// Pin mappings for LoRa tranceiver
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 23,
    .dio = {
        /*dio0*/ 26,
        /*dio1*/ 33,
        /*dio2*/ 32}
#ifdef MCCI_LMIC
    ,
    .rxtx_rx_active = 0,
    .rssi_cal = 10,
    .spi_freq = 8000000 /* 8 MHz */
#endif
};

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
        // No actions required for this board.
        break;

    case InitType::PostInitSerial:
        // Note: If enabled Serial port and display are already initialized here.
        // No actions required for this board.
        break;
    }
    return success;
}

#endif
# LMIC Node Rebuild

A comprehensive LoRaWAN gateway solution for ESP32-based devices that bridges local IoT networks with The Things Network (TTN). This project extends the LMIC-Node framework with advanced task management, power management, GPS synchronization, MQTT broker functionality, and a modern web interface.

## 🚀 Features

- **LoRaWAN Gateway**: Acts as a bridge between local IoT devices and TTN
- **Multi-Task Architecture**: FreeRTOS-based task management for concurrent operations
- **Power Management**: Advanced power management with sleep modes and battery monitoring
- **GPS Integration**: Time synchronization and location tracking
- **MQTT Broker**: Local MQTT broker for device communication
- **Web Interface**: Modern Alpine.js-based dashboard served by the ESP32
- **Payload Parsing**: Automatic payload formatter generation for TTN console
- **Tasmota Integration**: Registry and management of Tasmota devices
- **OTA Updates**: Over-the-air firmware updates

## 📋 Supported Hardware

### Tested Devices
- **TTGO T-Beam V1.2**
  - GPS module
  - OLED display
  - Power management IC (AXP192)
  - LoRa radio (SX1276)
- **TTGO LoRa32 V2**
  - OLED display
  - SD card support
  - LoRa radio (SX1276)
- **LoPy v1**
  - Real-time clock
  - SD card support
  - LoRa radio (SX1272)

## 🏗️ Project Structure

```
LMIC Node Rebuild/
├── src/                    # Main ESP32 source code
│   ├── main.cpp           # Main application entry point
│   ├── lmic-node.cpp      # LoRaWAN functionality
│   ├── mqtt.cpp           # MQTT broker and client handling
│   ├── gps.cpp            # GPS time sync and location
│   ├── power.cpp          # Power management
│   ├── parsers.cpp        # Message parsing utilities
│   └── utils.cpp          # Utility functions
├── include/               # Header files
│   ├── lmic-node.h        # Main LoRaWAN interface
│   ├── typedef.h          # Data structures and payload definitions
│   ├── mqtt.h             # MQTT interface
│   └── *.h                # Board-specific and utility headers
├── web/                   # Frontend application
│   ├── src/               # Source files
│   ├── build/             # Built web assets
│   └── package.json       # Node.js dependencies
├── data/                  # Web assets served by ESP32
├── payload-formatters/    # TTN payload formatter generator
└── platformio.ini         # PlatformIO configuration
```

## 🔧 Architecture

### Task Management
The application uses FreeRTOS tasks for concurrent operations:

- **LMIC Task**: Handles LoRaWAN communication
- **MQTT Task**: Manages MQTT broker and client connections
- **GPS Task**: GPS time synchronization and location tracking
- **Status Task**: System monitoring and web interface
- **Power Task**: Power management and sleep modes

### Message Queues
- **UplinkQueue**: Messages to be sent to TTN
- **DownlinkQueue**: Messages received from TTN
- **MQTTQueue**: Local MQTT messages

### Payload Structures
The system supports multiple payload types with automatic port mapping:

- **Telemetry** (Port 11): Temperature, humidity, voltage, current, power
- **Tracer** (Port 12): Battery and solar system data
- **Meter** (Port 13): Power consumption data
- **CO2** (Port 14): Air quality sensors
- **GPS** (Port 15): Location and movement data
- **Coolbox** (Port 16): Cooling system monitoring

## 🚀 Getting Started

### Prerequisites
- PlatformIO IDE or CLI
- ESP32 development board
- LoRa radio module
- GPS module (for T-Beam)
- The Things Network account

### Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd "LMIC Node Rebuild"
   ```

2. **Configure LoRaWAN keys**
   - Copy `keyfiles/lorawan-keys_example.h` to `keyfiles/lorawan-keys.h`
   - Add your TTN device credentials

3. **Build and upload**
   ```bash
   # For TTGO T-Beam
   pio run -e tbeam
   pio run -e tbeam -t upload
   
   # For TTGO LoRa32
   pio run -e ttgo-lora32-v2
   pio run -e ttgo-lora32-v2 -t upload
   
   # For LoPy
   pio run -e lopy
   pio run -e lopy -t upload
   ```

4. **Build web interface**
   ```bash
   cd web
   npm install
   npm run build
   ```

5. **Generate payload formatters**
   ```bash
   cd payload-formatters
   npm install
   npm run build
   ```

### Configuration

#### PlatformIO Configuration
The `platformio.ini` file contains build configurations for different boards:

- **tbeam**: TTGO T-Beam with GPS and power management
- **ttgo-lora32-v2**: TTGO LoRa32 with display
- **lopy**: LoPy with SD card support

#### LoRaWAN Configuration
- Supports both OTAA and ABP activation
- EU868 frequency plan
- Configurable data rates and power levels

#### MQTT Configuration
- Local MQTT broker on port 1883
- Automatic device discovery
- Topic structure: `tasmota/discovery/<MAC>/config`

## 🌐 Web Interface

The web interface provides real-time monitoring and control:

### Features
- **System Status**: CPU, memory, uptime, and task information
- **LoRa Status**: Connection status, SNR, RSSI, and message queues
- **Device Registry**: Connected Tasmota devices and WiFi clients
- **GPS Information**: Current location and time synchronization
- **Payload Parsing**: Real-time message parsing and visualization

### Access
- **Local Network**: `http://<device-ip>`
- **Captive Portal**: Automatically redirects when connecting to device WiFi

## 📡 TTN Integration

### Payload Formatters
The system automatically generates payload formatters for TTN console:

1. **Build formatters**:
   ```bash
   cd payload-formatters
   npm run build
   ```

2. **Copy to TTN**: Use the generated JavaScript in your TTN application

### Message Flow
1. Local devices send data via MQTT
2. ESP32 parses and queues messages
3. Messages are sent to TTN via LoRaWAN
4. Downlink messages are forwarded to local devices

## 🔋 Power Management

### Features
- **Deep Sleep**: Configurable sleep intervals
- **Battery Monitoring**: Real-time battery status
- **Power Optimization**: Automatic power mode switching
- **Wake Sources**: Timer, GPIO, or external events

### Configuration
```cpp
#define DO_WORK_INTERVAL_SECONDS 60  // Work interval in seconds
#define SLEEP_VAR RTC_DATA_ATTR      // RTC memory for sleep state
```

## 📱 Tasmota Integration

### Device Discovery
The system automatically discovers Tasmota devices on the network:

```json
{
  "ip": "192.168.2.196",
  "dn": "Tasmota",
  "hn": "tasmota-80FF6D-8045",
  "mac": "E0980680FF6D",
  "md": "Generic",
  "sw": "13.3.0.1"
}
```

### Registry Management
- Automatic device registration
- Status monitoring
- Configuration management
- Time synchronization

## 🛠️ Development

### Adding New Payload Types
1. Define structure in `include/typedef.h`
2. Add parsing logic in `src/parsers.cpp`
3. Rebuild payload formatters
4. Update web interface if needed

### Board Support
To add support for a new board:

1. Create board-specific header in `include/`
2. Create board-specific source in `src/`
3. Add build environment in `platformio.ini`
4. Update pin definitions and features

### Debugging
- Serial output at 115200 baud
- Web interface status pages
- MQTT status messages
- Task monitoring

## 📊 Monitoring and Logging

### System Metrics
- Task heap usage
- Free memory
- CPU utilization
- Network statistics

### LoRaWAN Metrics
- Connection status
- Signal quality (SNR/RSSI)
- Message success rates
- Queue status

### Logging
- Serial console output
- Web interface logs
- MQTT status messages
- File system logging (SD card)

## 🔒 Security

### LoRaWAN Security
- AES-128 encryption
- OTAA/ABP authentication
- Secure key management

### Network Security
- WiFi encryption
- MQTT authentication
- HTTPS for web interface

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- **TTN Mapper T-beam**: Blueprint for using TBeam power management & GPS (https://github.com/bitconnector/ttn_mapper_t-beam)
- **LMIC-Node**: Base LoRaWAN functionality (https://github.com/lnlp/LMIC-node)
- **PicoMQTT**: Lightweight MQTT broker (https://github.com/mlesniew/PicoMQTT)
- **Tasmota**: IoT device firmware
- **The Things Network**: LoRaWAN network infrastructure

## 📞 Support

For issues and questions:
- Check the existing issues
- Review the documentation
- Test with supported hardware
- Provide detailed error information

---

**Note**: This project is actively maintained and supports multiple ESP32-based LoRaWAN gateways with advanced IoT integration capabilities.
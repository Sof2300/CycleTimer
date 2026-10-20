# CycleTimer

A WiFi-enabled cycle timer for Arduino (ESP8266/ESP32) that automates relay/valve control with configurable on/off periods. Features a web-based interface for real-time monitoring and configuration.

![License](https://img.shields.io/badge/license-CC0--1.0-blue)
![Platform](https://img.shields.io/badge/platform-ESP8266%20%7C%20ESP32-green)
![Version](https://img.shields.io/badge/version-0.2-orange)

## Features

- **Configurable Timing**: Set custom on-duration and total cycle duration
- **Web Interface**: User-friendly HTML/JavaScript UI for control and monitoring
- **WiFi Connectivity**: Supports both Station (client) and Access Point modes
- **mDNS Support**: Access via `http://cycletimer.local` on your local network
- **Persistent Configuration**: Settings saved to JSON file
- **Time Synchronization**: Syncs time with connected browser
- **Logging System**: Tracks events and state changes to log file
- **Relay Control**: Configurable GPIO pin with optional reverse logic
- **Cycle Control**: Start, stop, suspend, and resume cycles

## Hardware Requirements

- **Microcontroller**: ESP8266 or ESP32
- **Relay Module**: 5V relay for switching
- **Valve/Device**: The device to be controlled (e.g., irrigation valve, pump)
- **Power Supply**: Appropriate power for your ESP and relay
- **Optional**: Buzzer for audible alerts

## Installation

### Prerequisites

1. Install [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x or 2.x)
2. Install ESP8266/ESP32 board support via Arduino Board Manager
3. Install required libraries (see Dependencies section)

### Dependencies

This project uses a custom library framework. Ensure you have the following in your Arduino libraries folder:

- `ioktopuslib.h` - Main library framework (previously named `mylib.h`)
- `datastruct/` - Data structures (DataMap, GenString, JSON)
- `net/` - Network utilities (BasicServer, WifiMan)
- `TimeUtil.h` - Time utilities

 

### Compilation

#### Arduino (ESP8266/ESP32)

1. Clone or download this repository
2. Open `CycleTimer.ino` in Arduino IDE
3. Select your board (ESP8266 or ESP32) from Tools > Board
4. Select the correct COM port
5. Click Upload

#### x86 Desktop Build (Debugging)

The project includes `CycleTimerx86.c++` for compilation and testing on desktop systems (x86 architecture). 
To compile for x86, use a standard C++ compiler:
```bash
g++ CycleTimerx86.c++ -o CycleTimerx86
./CycleTimerx86
```

**Note**: The x86 build simulates the hardware environment but may have limitations in testing hardware-specific features (GPIO, WiFi, timers).

## Usage

### First-Time Setup

1. **Power on the device** - It will boot into Access Point mode by default
2. **Connect to WiFi** - Look for "CycleTimer" in your WiFi networks
3. **Open web interface** - Navigate to `http://192.168.4.1` or use mDNS: `http://cycletimer.local`
4. **Configure WiFi** - Enter your home WiFi credentials to switch to Station mode
5. **Access the interface** - Once connected to your network, access via mDNS or the assigned IP

### Web Interface

The web interface provides:

- **Cycle Status**: View current state (ON/OFF), cycle running status
- **Timing Controls**: Adjust on-duration and cycle duration (in seconds)
- **Manual Control**: Switch relay ON/OFF, suspend/resume cycle
- **Configuration**: Set relay pin, reverse logic, WiFi settings
- **Log Viewer**: View system events and state changes
- **System Info**: Free space, IP address, time sync status

### Configuration

Key configuration parameters (stored in `/config.json`):

```json
{
  "/config/mdns": "CycleTimer",
  "/config/mode": "ST",
  "/config/ssid": "YourNetwork",
  "/config/password": "YourPassword",
  "/data/cycle-duration-sec": "3600",
  "/data/on-duration-sec": "60",
  "/data/relay-pin": "5",
  "/data/reverse": "1"
}
```

**Parameters**:
- `cycle-duration-sec`: Total cycle time in seconds (default: 3600 = 1 hour)
- `on-duration-sec`: Time relay stays ON in seconds (default: 60)
- `relay-pin`: GPIO pin for relay (default: 5)
- `reverse`: Set to "1" to invert relay logic (default: "0")

### Important Notes

- **Switching ON/OFF suspends the cycle** until you explicitly resume it
- **Restarting the cycle** resets the timing from the current moment
- **Time synchronization** occurs automatically when you open the web interface
- **Configuration changes** are saved immediately to the file system

## Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────┐
│                    CycleTimer.ino                       │
│  - Main setup/loop                                      │
│  - Component initialization                            │
│  - Server orchestration                                │
└────────────────────┬────────────────────────────────────┘
                     │
        ┌────────────┼────────────┐
        │            │            │
        ▼            ▼            ▼
┌──────────────┐ ┌──────────┐ ┌──────────────┐
│CycleManager  │ │WifiManager│ │DataManager   │
│              │ │          │ │              │
│- Relay control│ │- WiFi    │ │- JSON serving│
│- Timing logic │ │- mDNS    │ │- Time sync   │
│- State mgmt  │ │- AP/ST   │ │- Storage info│
└──────────────┘ └──────────┘ └──────────────┘
        │                            │
        └────────────┬───────────────┘
                     ▼
              ┌─────────────┐
              │  DataMap    │
              │             │
              │- Central    │
              │  data store │
              │- JSON I/O   │
              │- Events     │
              └─────────────┘
```

### Core Components

#### CycleManager.h
- **CycleActivator**: Handles relay switching using Ticker timers
  - Manages on/off timing with hardware timers
  - Supports suspend/resume functionality
  - Handles time synchronization corrections
  - Provides pin change and duration adjustment

- **CycleManager**: High-level cycle control
  - Initializes from configuration
  - Maps cycle state to DataMap
  - Handles HTTP API endpoints (`/set`, `/eraselog`, `x-Date`)
  - Logs state changes

#### ConfigManager.h
- Loads and parses JSON configuration from file system
- Uses custom JSON parser (HeavyJSONLoader)
- Populates DataMap with configuration values
- Filters out runtime-only data when saving

#### WifiManager.h
- Manages WiFi connections (Station and Access Point modes)
- Handles mode switching via web interface
- Integrates with DataMap for persistent settings
- Provides automatic reconnection

#### DataManager.h
- Serves JSON data to web interface via `/json` endpoint
- Handles time synchronization from browser (via `x-Date` header)
- Provides system information (free space, boot time, timezone)
- Filters sensitive data (passwords) from JSON output

#### Mapping.h (Future Use)
- Expression evaluation framework
- Supports conditional logic, comparisons, and data transfers
- Designed for sensor-based cycle adjustments (not currently active)

### Data Flow

1. **Configuration Loading**: `ConfigManager` reads `/config.json` → populates `DataMap`
2. **Initialization**: Components read from `DataMap` → set initial state
3. **Runtime**: `CycleActivator` uses timers → updates `DataMap` via `CycleUpdater`
4. **Web Requests**: `BasicServer` routes to managers → managers update `DataMap`
5. **Persistence**: Changes to `DataMap` → trigger `saveDataMap()` → write to file

### Event System

The system uses an event-driven architecture:
- Components implement `EventListener` interface
- `StringMapEvent` carries key-value data
- HTTP requests trigger events via `BasicServer`
- Components respond to relevant events and update state

### File System Structure

```
/config.json           - Main configuration (persisted)
/logfile.txt          - System event log
/data/*.js            - Web interface JavaScript
/data/index.html      - Web interface HTML
```

## API Endpoints

### GET Endpoints

- `/json` - Returns current system state as JSON (excludes passwords)
- `/logfile.txt` - Returns log file contents

### POST Endpoints

- `/set` - Update cycle parameters
  - Parameters: `relay-pin`, `on-duration-sec`, `cycle-duration-sec`, `switch-state`, `cycle-state`, `last-start`
  - Returns: `"response": "ok"`

- `/switchToStation` - Switch to WiFi Station mode
  - Parameters: `ssid`, `password`, `mdns`
  - Returns: `"response": "ok"`

- `/switchToAP` - Switch to Access Point mode
  - Parameters: `ssid`, `password`, `mdns`
  - Returns: `"response": "ok"`

- `/eraselog` - Clear the log file
  - Returns: `"response": "ok"`

### Custom Headers

- `x-Date` - Timestamp for time synchronization (sent by browser)
- `x-TimeZone` - Timezone offset in hours (sent by browser)

## Development

### Code Structure

- **Header files**: `.h` files contain class definitions and implementations
- **Main file**: `CycleTimer.ino` - setup and loop, component initialization
- **Data directory**: Web interface files (uploaded to SPIFFS/LittleFS)

### Key Constants

```cpp
#define DEFAULT_RELAY_PIN 5
#define CYCLE_ON_DURATION_DEFAULT 60        // seconds
#define CYCLE_DURATION_DEFAULT 3600        // seconds (1 hour)
#define CONFIGFILENAME "/config.json"
#define LOGFILE_NAME "logfile.txt"
```

### Adding New Features

1. **Add new parameter**: Update DataMap paths in `CycleTimer.ino`
2. **Add new endpoint**: Register in `server->on()` and implement in appropriate manager
3. **Add web UI elements**: Update JavaScript files in `data/` directory
4. **Persist configuration**: Add to `DONT_SAVE_CONFIG` if runtime-only

### Library Migration

The project currently uses `#include <mylib.h>` in the source code. To use the renamed `ioktopuslib`:

1. Update the include statement in `CycleTimer.ino`:
   ```cpp
   #include <ioktopuslib.h>  // Changed from mylib.h
   ```
2. Ensure the library is installed in your Arduino libraries folder as `ioktopuslib`

### Testing

Use `CycleTimerx86.c++` for desktop testing of logic without hardware.

### Known Issues / TODO

- Time synchronization issues (see CycleTimer.ino line 26)
- Relay may need reversing logic adjustment
- Physical enclosure needed for deployment
- Future: Sensor-based duration adjustment
- Future: Improved format for durations >60 seconds

## Troubleshooting

### Device not accessible
- Check if device is in AP mode: Connect to "CycleTimer" WiFi
- Use serial monitor (115200 baud) to see startup messages
- Verify mDNS is working: Try `http://<IP address>` directly

### Relay not switching
- Check `relay-pin` configuration matches your wiring
- Try toggling `reverse` parameter in config
- Verify relay module power supply

### WiFi connection issues
- Ensure SSID and password are correct in config
- Check that WiFi network is 2.4GHz (ESP8266 doesn't support 5GHz)
- Use serial monitor to see connection status

### Time not syncing
- Ensure browser allows sending custom headers
- Check that `x-Date` and `x-TimeZone` are being sent
- Serial monitor will show sync events

## License

This project is licensed under CC0 1.0 Universal - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Acknowledgments

- Built with custom Arduino library framework
- Uses Ticker library for hardware timers
- Web interface uses vanilla JavaScript with component-based architecture

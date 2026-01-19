# CrossPoint Reader - Development Guide

## Project Overview

CrossPoint Reader is a custom firmware for ESP32-C3 based e-reader devices. It provides:
- EPUB/TXT/XTC book reading
- File browsing and transfer
- OPDS library integration
- Calibre and KOReader sync support
- Modular apps system for extending functionality

## Hardware Platform

- **MCU**: ESP32-C3 (16MB flash)
- **Display**: E-Ink display with custom SPI pins
- **Platform**: PlatformIO with Arduino framework
- **Build System**: PlatformIO + custom Python scripts

## Architecture

### Activity System

The firmware uses an Activity-based architecture similar to Android:

- **Activity**: Base class for all screens/modes (`src/activities/Activity.h`)
  - `onEnter()`: Called when entering the activity
  - `onExit()`: Called when exiting the activity
  - `loop()`: Main loop, called repeatedly while active
  - Activities can optionally use FreeRTOS tasks for rendering

- **ActivityWithSubactivity**: Extends Activity to support nested activities
  - Used for screens that launch sub-screens (e.g., Settings → KOReader Settings)
  - Manages subactivity lifecycle automatically

### Activity Navigation

Activities are managed through function pointers in `main.cpp`:
```cpp
void exitActivity();           // Exit current activity
void enterNewActivity(Activity* activity);  // Enter new activity
Activity* currentActivity;     // Global current activity pointer
```

Common navigation functions:
- `onGoHome()`: Return to home screen
- `onGoToReader()`: Open reader activity
- `onGoToSettings()`: Open settings
- `onGoToFileTransfer()`: Open file transfer
- `onGoToBrowser()`: Open OPDS browser
- `onGoToApps()`: Open apps menu

### Apps System

A modular system for adding lightweight programs to the firmware:

#### Core Components

1. **App Interface** (`src/apps/App.h`)
   - Base class for all apps
   - Pure virtual methods: `getName()`, `onEnter()`, `onExit()`, `loop()`, `render()`, `shouldExit()`
   - Apps receive references to `GfxRenderer` and `MappedInputManager`

2. **AppRegistry** (`src/apps/AppRegistry.h`)
   - Singleton registry for managing available apps
   - Apps registered with factory functions
   - Provides app enumeration and creation

3. **AppsActivity** (`src/activities/apps/AppsActivity.h`)
   - Menu activity for browsing and launching apps
   - Lists all registered apps
   - Similar UX to SettingsActivity

4. **AppRunnerActivity** (`src/activities/apps/AppRunnerActivity.h`)
   - Wrapper that runs an App as an Activity
   - Manages app lifecycle and rendering task
   - Handles exit when app signals completion

#### Creating New Apps

1. Create app header and implementation in `src/apps/`
2. Inherit from `App` base class
3. Implement all pure virtual methods
4. Register in `main.cpp` setup function:

```cpp
AppRegistry::getInstance().registerApp("My App", [](GfxRenderer& r, MappedInputManager& m) -> App* {
  return new MyApp(r, m);
});
```

Example apps:
- `HelloWorldApp`: Simple demonstration app showing uptime
- Future: WiFi beacon tools, system utilities, games, etc.

### Rendering System

- **GfxRenderer**: High-level graphics API (`open-x4-sdk/libs/display/GfxRenderer`)
  - Text rendering with multiple fonts
  - Basic shapes (rectangles, lines, polygons)
  - Bitmap/image rendering
  - Buffer management and display refresh

- **Fonts**: Pre-loaded font families
  - UI fonts: Ubuntu 10pt, 12pt
  - Reading fonts: Bookerly, Noto Sans, OpenDyslexic (multiple sizes)
  - Font IDs defined in `src/fontIds.h`

### Input System

- **InputManager**: Low-level hardware button reading
- **MappedInputManager**: High-level input abstraction
  - Configurable button mappings based on context
  - Four logical buttons: Back, Confirm, Up/Left, Down/Right
  - Button state tracking (pressed, released, held)
  - Context-aware button labels for UI hints

### FreeRTOS Integration

Most activities use FreeRTOS tasks for rendering:
```cpp
void onEnter() override {
  renderingMutex = xSemaphoreCreateMutex();
  xTaskCreate(&taskTrampoline, "TaskName", 4096, this, 1, &displayTaskHandle);
}

void displayTaskLoop() {
  while (true) {
    xSemaphoreTake(renderingMutex, portMAX_DELAY);
    render();
    xSemaphoreGive(renderingMutex);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
```

## Code Style Guidelines

### C++ Standards
- C++20 enabled (`-std=c++2a`)
- Use modern C++ features where appropriate
- Prefer RAII for resource management

### Naming Conventions
- **Classes**: PascalCase (`HomeActivity`, `AppRegistry`)
- **Functions**: camelCase (`onEnter`, `getAppCount`)
- **Member variables**: camelCase with descriptive names
- **Constants**: ALL_CAPS for macros, otherwise camelCase

### Memory Management
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) where appropriate
- Manual memory management with `new`/`delete` is acceptable for activities
- Always clean up in `onExit()` methods
- Be mindful of ESP32-C3 limited RAM (~400KB)

### File Organization
- Header files (`.h`) contain class declarations
- Implementation files (`.cpp`) contain definitions
- Use `#pragma once` for header guards
- Group related functionality in directories

### Comments
- Use `//` for single-line comments
- Use `/** */` for documentation comments
- Document public APIs and non-obvious implementation details
- Keep comments concise and up-to-date

## Building and Flashing

```bash
# Build firmware
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor

# Build and upload
pio run --target upload && pio device monitor
```

## Project Structure

```
crosspoint-reader/
├── src/
│   ├── activities/       # Activity implementations
│   │   ├── apps/        # Apps-related activities
│   │   ├── boot_sleep/  # Boot and sleep screens
│   │   ├── browser/     # OPDS browser
│   │   ├── home/        # Home screen
│   │   ├── network/     # WiFi and network activities
│   │   ├── reader/      # Book reader activities
│   │   ├── settings/    # Settings screens
│   │   └── util/        # Utility activities
│   ├── apps/            # App implementations
│   ├── network/         # Network utilities
│   ├── util/            # General utilities
│   └── main.cpp         # Application entry point
├── open-x4-sdk/         # Hardware abstraction libraries
├── platformio.ini       # PlatformIO configuration
└── CLAUDE.md           # This file
```

## Key Configuration Files

- `platformio.ini`: Build configuration, dependencies, compiler flags
- `partitions.csv`: Flash memory partition table
- `src/CrossPointSettings.h`: User settings structure
- `src/fontIds.h`: Font identifier constants

## External Dependencies

Managed through PlatformIO:
- `BatteryMonitor`: Battery level reading
- `InputManager`: Hardware button input
- `EInkDisplay`: E-Ink display driver
- `SDCardManager`: SD card file system
- `ArduinoJson`: JSON parsing/serialization
- `QRCode`: QR code generation
- `WebSockets`: WebSocket server support

## Design Patterns Used

1. **Activity Pattern**: Screen lifecycle management
2. **Singleton**: AppRegistry, settings, state management
3. **Factory Pattern**: App creation through AppRegistry
4. **Observer Pattern**: Input event handling
5. **Strategy Pattern**: Configurable button mappings

## Best Practices

### For Activities
- Always create/destroy FreeRTOS resources in `onEnter()`/`onExit()`
- Use semaphores when rendering from background tasks
- Clean up all allocated resources in `onExit()`
- Handle navigation through callbacks passed in constructor

### For Apps
- Keep apps lightweight and focused
- Signal exit through `shouldExit()` return value
- Use `onEnter()` for initialization, `onExit()` for cleanup
- Render at reasonable intervals (don't spam display updates)

### Memory Management
- Monitor stack usage (default task stack: 4096 bytes)
- Use static allocation for large buffers when possible
- Profile memory usage during development
- Consider e-ink display buffer size (~60KB for typical display)

### Display Optimization
- Minimize full screen refreshes (they're slow on e-ink)
- Use partial updates where supported
- Batch drawing operations before `displayBuffer()` call
- Consider user-configurable refresh rates

## Adding New Features

### New Activity
1. Create header/implementation in appropriate `src/activities/` subdirectory
2. Inherit from `Activity` or `ActivityWithSubactivity`
3. Implement required virtual methods
4. Add navigation function in `main.cpp`
5. Wire up navigation from other activities

### New App
1. Create header/implementation in `src/apps/`
2. Inherit from `App` base class
3. Implement all pure virtual methods
4. Register in `main.cpp` setup:
   ```cpp
   AppRegistry::getInstance().registerApp("App Name", factoryFunction);
   ```

### New Setting
1. Add field to `CrossPointSettings` struct
2. Add to settings list in `SettingsActivity.cpp`
3. Use appropriate `SettingInfo` type (Toggle, Enum, Value, Action)
4. Settings auto-save on change

## Troubleshooting

### Build Issues
- Check PlatformIO platform version matches `platformio.ini`
- Verify all symlinked libraries in `lib_deps` exist
- Clean build folder: `pio run --target clean`

### Runtime Issues
- Monitor serial output for debug messages
- Check stack size if experiencing crashes in tasks
- Verify SD card is properly formatted and accessible
- Check battery level (low battery can cause instability)

### Display Issues
- Ensure proper initialization sequence
- Verify SPI pin configuration matches hardware
- Check for mutex deadlocks in rendering tasks
- Test with simplified rendering to isolate issues

## Future Enhancement Ideas

### Apps to Implement
- **WiFi Scanner**: Scan and display nearby networks
- **System Monitor**: RAM usage, uptime, battery stats
- **Simple Games**: Snake, Tetris, Conway's Game of Life
- **Utilities**: Calculator, timer, stopwatch
- **WiFi Tools**: Beacon spammer, deauth monitor (defensive use only)
- **Text Editor**: Simple note-taking app
- **Image Viewer**: Browse images on SD card

### System Improvements
- App settings/preferences system
- App data storage API
- Inter-app communication
- Background app execution
- App permissions system

## Contributing

When contributing to this project:
1. Follow existing code style and patterns
2. Test thoroughly on hardware
3. Document new features in this file
4. Keep memory usage in check
5. Consider e-ink display characteristics (refresh time, ghosting, etc.)

## Resources

- ESP32-C3 Documentation: https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/
- PlatformIO Docs: https://docs.platformio.org/
- Arduino ESP32 Core: https://github.com/espressif/arduino-esp32
- FreeRTOS API: https://www.freertos.org/a00106.html

## License

Check main repository for license information.


add apps 


1. Create your app class in src/apps/YourApp.h/cpp
  2. Inherit from App base class
  3. Implement: getName(), onEnter(), onExit(), loop(), render(), shouldExit()
  4. Register in main.cpp setup (line 312):
  AppRegistry::getInstance().registerApp("Your App Name",
    [](GfxRenderer& r, MappedInputManager& m) -> App* {
      return new YourApp(r, m);
    });

    
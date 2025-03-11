# Topas4Locator C++ Implementation

This project provides a C++ implementation of the Topas4Locator class, which discovers Topas4 devices on a local area network using UDP multicast. It's a port of the original Python implementation.

## Features

- Locates Topas4 devices on the same local network
- Works on Windows, macOS, and Linux platforms
- Uses UDP multicast and loopback discovery
- Filters duplicate device responses
- Returns device information including serial numbers and REST API URLs

## Requirements

- C++11 compatible compiler
- CMake 3.10 or newer
- nlohmann/json library (automatically fetched by CMake)
- On Windows: Windows Socket library (part of the Windows SDK)

## Project Structure

```
topas4_project/
├── CMakeLists.txt        # Project build configuration
├── main.cc               # Main application entry point
├── Topas4Locator.hh      # Class declaration header
└── Topas4Locator.cc      # Class implementation
```

## Building the Project

```bash
# Clone the repository
git clone https://github.com/yourusername/topas4-locator.git
cd topas4-locator

# Create a build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .
```

## Usage Example

```cpp
#include <iostream>
#include "Topas4Locator.hh"

int main() {
    // Create a locator instance
    Topas4Locator locator;
    
    // Find devices on the network
    auto devices = locator.locate();
    
    // Print the results
    std::cout << "Found " << devices.size() << " Topas4 devices:" << std::endl;
    
    for (const auto& device : devices) {
        std::cout << "Serial Number: " << device["SerialNumber"] << std::endl;
        std::cout << "REST API URL: " << device["PublicApiRestUrl_Version0"] << std::endl;
        std::cout << "------------------------" << std::endl;
    }
    
    return 0;
}
```

## How It Works

The locator works by:

1. Creating a UDP socket
2. Sending the message "Topas4?" to both a multicast address (239.0.0.181:7415) and localhost
3. Collecting and parsing JSON responses from devices
4. Filtering out duplicate responses based on device GUIDs
5. Returning a vector of device information

## Next Steps

After discovering a device, you can communicate with it using its REST API URL. The typical workflow is:

1. Locate devices using `Topas4Locator`
2. Select a device by serial number or other criteria
3. Use the device's `PublicApiRestUrl_Version0` to communicate via HTTP requests

## License

[Your chosen license]
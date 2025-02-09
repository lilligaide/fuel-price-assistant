# EV Charging Station Finder

A modern C++ web application for finding electric vehicle charging stations using real-time data from OpenChargeMap API.

## Features

- Real-time charging station search
- Interactive map interface with Leaflet.js
- Filtering by connector type and power output
- Radius-based search
- Geocoding support for address search
- Local caching for improved performance
- Mobile-responsive design
- Route planning with waypoints
- Detailed station information including:
  - Available connectors
  - Power output
  - Operating hours
  - Distance calculation
  - Operator information

## Prerequisites

- C++ 20 or later
- CMake 3.15 or later
- vcpkg package manager
- OpenSSL
- SQLite3

## Dependencies

The project uses the following libraries:

- Crow (C++ web framework)
- Drogon ORM (database operations)
- {fmt} (string formatting)
- nlohmann/json (JSON handling)
- cpr (HTTP client)
- OpenSSL (HTTPS support)
- Leaflet.js (map interface)

## Building the Project

1. Install vcpkg if not already installed:
```bash
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
```

2. Install dependencies using vcpkg:
```bash
vcpkg install crow:x64-linux
vcpkg install drogon:x64-linux
vcpkg install fmt:x64-linux
vcpkg install nlohmann-json:x64-linux
vcpkg install cpr:x64-linux
vcpkg install openssl:x64-linux
vcpkg install sqlite3:x64-linux
```

3. Clone the repository:
```bash
git clone https://github.com/yourusername/ev-charging-finder.git
cd ev-charging-finder
```

4. Create a build directory and build the project:
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
```

## Configuration

1. Create a `config.json` file in the project root:
```json
{
    "port": 8080,
    "host": "0.0.0.0",
    "db_path": "stations.db",
    "cache_duration_seconds": 3600,
    "use_ssl": false,
    "ssl_cert": "",
    "ssl_key": "",
    "rate_limit_requests": 100,
    "rate_limit_window_seconds": 60
}
```

2. Set your OpenChargeMap API key in `src/utils/Config.hpp`:
```cpp
const std::string API_KEY = "your-api-key-here";
```

## Running the Application

1. Start the server:
```bash
./ev-charging-finder
```

2. Open a web browser and navigate to:
```
http://localhost:8080
```

## API Endpoints

### Station Search
```
GET /api/stations/search
Parameters:
- lat: Latitude (required)
- lon: Longitude (required)
- radius: Search radius in meters (optional, default: 5000)
- minPower: Minimum power output in kW (optional)
- connectors: Comma-separated list of connector types (optional)
```

### Geocoding
```
GET /api/geocode
Parameters:
- address: Address to geocode (required)
```

### Reverse Geocoding
```
GET /api/reverse-geocode
Parameters:
- lat: Latitude (required)
- lon: Longitude (required)
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- OpenChargeMap API for providing charging station data
- OpenStreetMap contributors for map data
- All the open-source libraries used in this project 
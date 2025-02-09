cmake_minimum_required(VERSION 3.15)
project(ev_charging_finder VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Crow CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Drogon CONFIG REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    src/server/Server.cpp
    src/server/Routes.cpp
    src/api/ChargingAPI.cpp
    src/api/GeocodingAPI.cpp
    src/services/StationService.cpp
    src/services/GeocodingService.cpp
    src/db/Database.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Crow::Crow
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    cpr::cpr
    fmt::fmt
    SQLite::SQLite3
    Drogon::Drogon
)

# Copy static files and templates to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/static ${CMAKE_BINARY_DIR}/static
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/templates ${CMAKE_BINARY_DIR}/templates
)

# Enable testing
enable_testing()
add_subdirectory(tests) 
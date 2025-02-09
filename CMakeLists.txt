cmake_minimum_required(VERSION 3.20)
project(fuel-price-assistant VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
enable_testing()

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)

# Set source files
set(SOURCES
    src/main.cpp
    src/api/TankerkoenigAPI.cpp
    src/notifications/TeamsNotificationService.cpp
    src/utils/Config.cpp
    src/utils/RouteCalculator.cpp
)

# Set header files
set(HEADERS
    include/api/TankerkoenigAPI.hpp
    include/models/FuelStation.hpp
    include/models/PriceStatistics.hpp
    include/notifications/NotificationService.hpp
    include/notifications/TeamsNotificationService.hpp
    include/utils/Config.hpp
    include/utils/RouteCalculator.hpp
)

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# Set compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Configure tests
add_subdirectory(tests)

# Install configuration
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Generate documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif() 
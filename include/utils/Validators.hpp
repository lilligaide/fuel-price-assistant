#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace Validators {
    bool isValidLatitude(double lat);
    bool isValidLongitude(double lon);
    bool isValidRadius(int meters);
    bool isValidPower(double kw);
    
    struct CoordinateValidationResult {
        bool isValid;
        std::string error;
    };
    
    CoordinateValidationResult validateCoordinates(double lat, double lon);
    
    std::string sanitizeInput(std::string_view input);
    
    template<typename T>
    std::optional<T> validateNumericInput(const std::string& input, T min, T max) {
        try {
            if constexpr (std::is_floating_point_v<T>) {
                T value = std::stod(input);
                return (value >= min && value <= max) ? std::optional<T>(value) : std::nullopt;
            } else {
                T value = std::stoi(input);
                return (value >= min && value <= max) ? std::optional<T>(value) : std::nullopt;
            }
        } catch (...) {
            return std::nullopt;
        }
    }
    
    // Constants for validation
    constexpr double MIN_LATITUDE = -90.0;
    constexpr double MAX_LATITUDE = 90.0;
    constexpr double MIN_LONGITUDE = -180.0;
    constexpr double MAX_LONGITUDE = 180.0;
    constexpr int MIN_RADIUS = 100;      // 100 meters
    constexpr int MAX_RADIUS = 50000;    // 50 kilometers
    constexpr double MIN_POWER = 0.0;    // 0 kW
    constexpr double MAX_POWER = 500.0;  // 500 kW
} 
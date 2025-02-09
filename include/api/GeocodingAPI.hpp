#pragma once

#include <cpr/cpr.h>
#include <string>
#include <optional>
#include "../models/Location.hpp"
#include "../utils/Config.hpp"

class GeocodingAPI {
public:
    explicit GeocodingAPI(const APIConfig& config);
    
    struct GeocodingResult {
        double latitude;
        double longitude;
        std::string formattedAddress;
    };
    
    std::optional<GeocodingResult> geocodeAddress(const std::string& address);
    std::optional<std::string> reverseGeocode(double latitude, double longitude);
    
private:
    cpr::Session session;
    std::string baseUrl;
    
    void setupSession(const APIConfig& config);
}; 
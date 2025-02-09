#pragma once

#include <memory>
#include <optional>
#include <string>
#include "../api/GeocodingAPI.hpp"
#include "../models/Location.hpp"
#include "../utils/Config.hpp"

class GeocodingService {
public:
    explicit GeocodingService(const Config& config);
    
    std::optional<Location> geocodeAddress(const std::string& address);
    std::string reverseGeocode(double latitude, double longitude);
    
private:
    std::unique_ptr<GeocodingAPI> api;
}; 
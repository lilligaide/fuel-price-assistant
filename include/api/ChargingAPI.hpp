#pragma once

#include <cpr/cpr.h>
#include <string>
#include <vector>
#include <optional>
#include "../models/Station.hpp"
#include "../utils/Config.hpp"

class ChargingAPI {
public:
    explicit ChargingAPI(const APIConfig& config);
    
    std::vector<ChargingStation> findStations(
        double latitude,
        double longitude,
        int radiusMeters,
        int maxResults = 100
    );
    
    std::optional<ChargingStation> getStationDetails(int64_t stationId);
    
private:
    cpr::Session session;
    std::string baseUrl;
    
    void setupSession(const APIConfig& config);
}; 
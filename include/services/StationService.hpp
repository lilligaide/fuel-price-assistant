#pragma once

#include "../models/Station.hpp"
#include "../utils/Config.hpp"
#include "../db/Database.hpp"
#include <vector>
#include <optional>
#include <memory>
#include <cpr/cpr.h>

class StationService {
public:
    explicit StationService(const Config& config);
    
    std::vector<ChargingStation> findNearbyStations(
        double latitude,
        double longitude,
        int radiusMeters,
        const std::vector<std::string>& connectorTypes = {},
        double minPowerKW = 0.0
    );
    
    std::optional<ChargingStation> getStationById(int64_t id);
    
    std::vector<ChargingStation> findStationsAlongRoute(
        const std::vector<std::pair<double, double>>& routePoints,
        int corridorWidthMeters
    );

private:
    std::vector<ChargingStation> fetchFromAPI(
        double latitude,
        double longitude,
        int radiusMeters
    );
    
    std::vector<ChargingStation> fetchFromCache(
        double latitude,
        double longitude,
        int radiusMeters
    );
    
    void updateCache(const std::vector<ChargingStation>& stations);
    
    double calculateDistance(
        double lat1,
        double lon1,
        double lat2,
        double lon2
    );
    
    Config config;
    std::unique_ptr<Database> db;
    cpr::Session httpClient;
}; 
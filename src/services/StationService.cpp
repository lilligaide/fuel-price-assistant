#include "StationService.hpp"
#include "../utils/Logger.hpp"
#include <nlohmann/json.hpp>
#include <cmath>
#include <algorithm>

using json = nlohmann::json;

StationService::StationService(const Config& config) : config(config) {
    db = std::make_unique<Database>(config.db_path);
    
    // Configure HTTP client
    httpClient.SetTimeout(cpr::Timeout{config.api.timeout_ms});
    httpClient.SetVerifySsl(config.api.verify_ssl);
    httpClient.SetHeader({
        {"Content-Type", "application/json"},
        {"X-API-Key", config.api.key}
    });
}

std::vector<ChargingStation> StationService::findNearbyStations(
    double latitude,
    double longitude,
    int radiusMeters,
    const std::vector<std::string>& connectorTypes,
    double minPowerKW
) {
    // Try cache first
    auto stations = fetchFromCache(latitude, longitude, radiusMeters);
    
    // If cache miss or expired, fetch from API
    if (stations.empty()) {
        stations = fetchFromAPI(latitude, longitude, radiusMeters);
        updateCache(stations);
    }
    
    // Apply filters
    stations.erase(
        std::remove_if(stations.begin(), stations.end(),
            [&](const ChargingStation& station) {
                // Filter by minimum power
                if (minPowerKW > 0) {
                    bool hasSufficientPower = false;
                    for (const auto& conn : station.connections) {
                        if (conn.powerKW >= minPowerKW) {
                            hasSufficientPower = true;
                            break;
                        }
                    }
                    if (!hasSufficientPower) return true;
                }
                
                // Filter by connector types
                if (!connectorTypes.empty()) {
                    bool hasMatchingConnector = false;
                    for (const auto& conn : station.connections) {
                        if (std::find(connectorTypes.begin(), connectorTypes.end(),
                                    conn.type) != connectorTypes.end()) {
                            hasMatchingConnector = true;
                            break;
                        }
                    }
                    if (!hasMatchingConnector) return true;
                }
                
                return false;
            }
        ),
        stations.end()
    );
    
    // Calculate distances
    for (auto& station : stations) {
        station.distance_km = calculateDistance(
            latitude, longitude,
            station.location.latitude,
            station.location.longitude
        );
    }
    
    // Sort by distance
    std::sort(stations.begin(), stations.end(),
        [](const ChargingStation& a, const ChargingStation& b) {
            return a.distance_km < b.distance_km;
        }
    );
    
    return stations;
}

std::vector<ChargingStation> StationService::fetchFromAPI(
    double latitude,
    double longitude,
    int radiusMeters
) {
    std::vector<ChargingStation> stations;
    
    try {
        auto url = fmt::format("{}/poi", config.api.endpoint);
        auto response = httpClient.Get(cpr::Url{url},
            cpr::Parameters{
                {"latitude", std::to_string(latitude)},
                {"longitude", std::to_string(longitude)},
                {"distance", std::to_string(radiusMeters)},
                {"distanceunit", "meters"},
                {"maxresults", "100"}
            }
        );
        
        if (response.status_code == 200) {
            auto j = json::parse(response.text);
            stations = j.get<std::vector<ChargingStation>>();
            Logger::info("Fetched {} stations from API", stations.size());
        } else {
            Logger::error("API request failed: {} - {}", 
                         response.status_code, response.text);
        }
    } catch (const std::exception& e) {
        Logger::error("Error fetching stations from API: {}", e.what());
    }
    
    return stations;
}

std::vector<ChargingStation> StationService::fetchFromCache(
    double latitude,
    double longitude,
    int radiusMeters
) {
    return db->getStationsInRadius(latitude, longitude, radiusMeters);
}

void StationService::updateCache(const std::vector<ChargingStation>& stations) {
    db->updateStations(stations);
}

double StationService::calculateDistance(
    double lat1,
    double lon1,
    double lat2,
    double lon2
) {
    // Haversine formula
    const double R = 6371.0; // Earth radius in kilometers
    
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    
    double a = std::sin(dLat/2) * std::sin(dLat/2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dLon/2) * std::sin(dLon/2);
               
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    
    return R * c;
}

std::optional<ChargingStation> StationService::getStationById(int64_t id) {
    return db->getStationById(id);
}

std::vector<ChargingStation> StationService::findStationsAlongRoute(
    const std::vector<std::pair<double, double>>& routePoints,
    int corridorWidthMeters
) {
    std::vector<ChargingStation> stations;
    
    for (size_t i = 0; i < routePoints.size() - 1; ++i) {
        auto [lat1, lon1] = routePoints[i];
        auto [lat2, lon2] = routePoints[i+1];
        
        // Calculate midpoint
        double midLat = (lat1 + lat2) / 2;
        double midLon = (lon1 + lon2) / 2;
        
        // Search around midpoint with radius half the corridor width
        auto segmentStations = findNearbyStations(
            midLat, midLon, corridorWidthMeters / 2
        );
        
        // Add unique stations
        for (const auto& station : segmentStations) {
            if (std::find_if(stations.begin(), stations.end(),
                    [&](const ChargingStation& s) {
                        return s.id == station.id;
                    }) == stations.end()) {
                stations.push_back(station);
            }
        }
    }
    
    return stations;
} 
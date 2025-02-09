#pragma once

#include <string>
#include <vector>
#include <optional>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "../models/FuelStation.hpp"

namespace api {

class TankerkoenigAPI {
public:
    explicit TankerkoenigAPI(const std::string& apiKey);
    ~TankerkoenigAPI();
    
    // Disable copying
    TankerkoenigAPI(const TankerkoenigAPI&) = delete;
    TankerkoenigAPI& operator=(const TankerkoenigAPI&) = delete;
    
    // Search stations in radius
    std::vector<models::FuelStation> findStations(
        double lat,
        double lng,
        double radius,
        const std::string& fuelType = "all"
    );
    
    // Get details for a specific station
    std::optional<models::FuelStation> getStationDetails(const std::string& stationId);
    
    // Get prices for a list of stations
    std::vector<models::FuelStation> getPrices(const std::vector<std::string>& stationIds);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    nlohmann::json makeRequest(const std::string& endpoint, 
                             const std::map<std::string, std::string>& params);
    
    CURL* curl;
    std::string apiKey;
    static constexpr const char* BASE_URL = "https://creativecommons.tankerkoenig.de/json/";
    static constexpr int TIMEOUT_SECONDS = 10;
};

} // namespace api 
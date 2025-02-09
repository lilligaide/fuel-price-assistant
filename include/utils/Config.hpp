#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace utils {

struct NotificationConfig {
    std::string type;  // "teams", "email", "mqtt", etc.
    std::map<std::string, std::string> settings;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(NotificationConfig, type, settings)
};

struct LocationConfig {
    double latitude;
    double longitude;
    double searchRadius;  // in kilometers
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LocationConfig, latitude, longitude, searchRadius)
};

struct MonitoringConfig {
    std::vector<std::string> fuelTypes;  // "e5", "e10", "diesel"
    int updateInterval;  // in minutes
    double priceThreshold;  // minimum price difference to trigger notification
    bool notifyOnIncrease;  // whether to notify when price increases
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MonitoringConfig, fuelTypes, updateInterval, 
                                  priceThreshold, notifyOnIncrease)
};

struct Config {
    std::string apiKey;
    LocationConfig location;
    MonitoringConfig monitoring;
    std::vector<NotificationConfig> notifications;
    
    static Config load(const std::string& path = "config.json");
    static Config fromEnvironment();
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, apiKey, location, monitoring, notifications)
};

} // namespace utils 
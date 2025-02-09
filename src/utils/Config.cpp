#include "utils/Config.hpp"
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <fmt/format.h>

namespace utils {

Config Config::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("Failed to open config file: {}", path));
    }

    try {
        nlohmann::json json;
        file >> json;
        return json.get<Config>();
    } catch (const std::exception& e) {
        throw std::runtime_error(fmt::format("Failed to parse config file: {}", e.what()));
    }
}

Config Config::fromEnvironment() {
    Config config;

    // Required environment variables
    const char* apiKey = std::getenv("TANKERKOENIG_API_KEY");
    if (!apiKey) {
        throw std::runtime_error("TANKERKOENIG_API_KEY environment variable is not set");
    }
    config.apiKey = apiKey;

    // Location configuration
    const char* lat = std::getenv("LOCATION_LAT");
    const char* lon = std::getenv("LOCATION_LON");
    const char* radius = std::getenv("SEARCH_RADIUS");
    if (!lat || !lon || !radius) {
        throw std::runtime_error("Location environment variables (LOCATION_LAT, LOCATION_LON, SEARCH_RADIUS) are not set");
    }

    try {
        config.location.latitude = std::stod(lat);
        config.location.longitude = std::stod(lon);
        config.location.searchRadius = std::stod(radius);
    } catch (const std::exception& e) {
        throw std::runtime_error(fmt::format("Failed to parse location values: {}", e.what()));
    }

    // Monitoring configuration
    const char* fuelTypes = std::getenv("FUEL_TYPES");
    const char* updateInterval = std::getenv("UPDATE_INTERVAL");
    const char* priceThreshold = std::getenv("PRICE_THRESHOLD");
    const char* notifyOnIncrease = std::getenv("NOTIFY_ON_INCREASE");

    if (fuelTypes) {
        std::string types(fuelTypes);
        size_t pos = 0;
        while ((pos = types.find(',')) != std::string::npos) {
            config.monitoring.fuelTypes.push_back(types.substr(0, pos));
            types.erase(0, pos + 1);
        }
        if (!types.empty()) {
            config.monitoring.fuelTypes.push_back(types);
        }
    } else {
        // Default fuel types if not specified
        config.monitoring.fuelTypes = {"e5", "e10", "diesel"};
    }

    config.monitoring.updateInterval = updateInterval ? std::stoi(updateInterval) : 15;
    config.monitoring.priceThreshold = priceThreshold ? std::stod(priceThreshold) : 0.02;
    config.monitoring.notifyOnIncrease = notifyOnIncrease ? (std::string(notifyOnIncrease) == "true") : false;

    // Notification configuration
    const char* teamsWebhook = std::getenv("TEAMS_WEBHOOK_URL");
    if (teamsWebhook) {
        NotificationConfig teamsConfig;
        teamsConfig.type = "teams";
        teamsConfig.settings["webhookUrl"] = teamsWebhook;
        config.notifications.push_back(teamsConfig);
    }

    return config;
}

} // namespace utils 
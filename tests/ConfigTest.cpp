#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../include/utils/Config.hpp"
#include <filesystem>
#include <fstream>

using namespace utils;
namespace fs = std::filesystem;

TEST_CASE("Config can be loaded from JSON file", "[config]") {
    // Create a temporary config file
    fs::path tempFile = fs::temp_directory_path() / "test_config.json";
    
    std::ofstream config(tempFile);
    config << R"({
        "apiKey": "test-api-key",
        "location": {
            "latitude": 52.520008,
            "longitude": 13.404954,
            "searchRadius": 5.0
        },
        "monitoring": {
            "fuelTypes": ["e5", "e10", "diesel"],
            "updateInterval": 15,
            "priceThreshold": 0.05,
            "notifyOnIncrease": true
        },
        "notifications": [
            {
                "type": "teams",
                "settings": {
                    "webhookUrl": "https://example.webhook.office.com/test"
                }
            },
            {
                "type": "email",
                "settings": {
                    "smtp_server": "smtp.example.com",
                    "smtp_port": "587",
                    "username": "test@example.com",
                    "recipient": "user@example.com"
                }
            }
        ]
    })";
    config.close();
    
    SECTION("Load valid config file") {
        auto cfg = Config::load(tempFile.string());
        
        CHECK(cfg.apiKey == "test-api-key");
        CHECK_THAT(cfg.location.latitude, Catch::Matchers::WithinRel(52.520008));
        CHECK_THAT(cfg.location.longitude, Catch::Matchers::WithinRel(13.404954));
        CHECK_THAT(cfg.location.searchRadius, Catch::Matchers::WithinRel(5.0));
        
        REQUIRE(cfg.monitoring.fuelTypes.size() == 3);
        CHECK(cfg.monitoring.fuelTypes[0] == "e5");
        CHECK(cfg.monitoring.fuelTypes[1] == "e10");
        CHECK(cfg.monitoring.fuelTypes[2] == "diesel");
        
        CHECK(cfg.monitoring.updateInterval == 15);
        CHECK_THAT(cfg.monitoring.priceThreshold, Catch::Matchers::WithinRel(0.05));
        CHECK(cfg.monitoring.notifyOnIncrease);
        
        REQUIRE(cfg.notifications.size() == 2);
        CHECK(cfg.notifications[0].type == "teams");
        CHECK(cfg.notifications[1].type == "email");
    }
    
    SECTION("Load non-existent config file") {
        fs::remove(tempFile);
        REQUIRE_THROWS(Config::load(tempFile.string()));
    }
    
    // Clean up
    fs::remove(tempFile);
}

TEST_CASE("Config can be loaded from environment", "[config]") {
    SECTION("Load with all environment variables set") {
        // Set environment variables
        setenv("TANKERKOENIG_API_KEY", "env-api-key", 1);
        setenv("LOCATION_LAT", "52.520008", 1);
        setenv("LOCATION_LON", "13.404954", 1);
        setenv("SEARCH_RADIUS", "5.0", 1);
        setenv("FUEL_TYPES", "e5,e10,diesel", 1);
        setenv("UPDATE_INTERVAL", "15", 1);
        setenv("PRICE_THRESHOLD", "0.05", 1);
        setenv("NOTIFY_ON_INCREASE", "true", 1);
        setenv("TEAMS_WEBHOOK_URL", "https://example.webhook.office.com/test", 1);
        
        auto cfg = Config::fromEnvironment();
        
        CHECK(cfg.apiKey == "env-api-key");
        CHECK_THAT(cfg.location.latitude, Catch::Matchers::WithinRel(52.520008));
        CHECK_THAT(cfg.location.longitude, Catch::Matchers::WithinRel(13.404954));
        CHECK_THAT(cfg.location.searchRadius, Catch::Matchers::WithinRel(5.0));
        
        REQUIRE(cfg.monitoring.fuelTypes.size() == 3);
        CHECK(cfg.monitoring.updateInterval == 15);
        CHECK_THAT(cfg.monitoring.priceThreshold, Catch::Matchers::WithinRel(0.05));
        CHECK(cfg.monitoring.notifyOnIncrease);
        
        REQUIRE(cfg.notifications.size() == 1);
        CHECK(cfg.notifications[0].type == "teams");
        
        // Clean up environment
        unsetenv("TANKERKOENIG_API_KEY");
        unsetenv("LOCATION_LAT");
        unsetenv("LOCATION_LON");
        unsetenv("SEARCH_RADIUS");
        unsetenv("FUEL_TYPES");
        unsetenv("UPDATE_INTERVAL");
        unsetenv("PRICE_THRESHOLD");
        unsetenv("NOTIFY_ON_INCREASE");
        unsetenv("TEAMS_WEBHOOK_URL");
    }
    
    SECTION("Load with missing required variables") {
        unsetenv("TANKERKOENIG_API_KEY");
        REQUIRE_THROWS(Config::fromEnvironment());
    }
} 
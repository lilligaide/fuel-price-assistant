#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../include/api/TankerkoenigAPI.hpp"

using namespace api;
using namespace models;

TEST_CASE("TankerkoenigAPI can be constructed with API key", "[api]") {
    REQUIRE_NOTHROW(TankerkoenigAPI("test-api-key"));
}

TEST_CASE("TankerkoenigAPI can find stations", "[api]") {
    TankerkoenigAPI api(TANKERKOENIG_API_KEY);  // Set via environment variable
    
    SECTION("Find stations with valid coordinates") {
        // Coordinates for Berlin
        double lat = 52.520008;
        double lng = 13.404954;
        double radius = 5.0;  // 5 km radius
        
        auto stations = api.findStations(lat, lng, radius);
        REQUIRE_FALSE(stations.empty());
        
        // Check first station has valid data
        const auto& station = stations.front();
        CHECK_FALSE(station.id.empty());
        CHECK_FALSE(station.name.empty());
        CHECK_THAT(station.location.latitude, Catch::Matchers::WithinAbs(lat, 0.5));
        CHECK_THAT(station.location.longitude, Catch::Matchers::WithinAbs(lng, 0.5));
    }
    
    SECTION("Find stations with invalid coordinates") {
        double lat = 91.0;  // Invalid latitude
        double lng = 0.0;
        double radius = 5.0;
        
        REQUIRE_THROWS(api.findStations(lat, lng, radius));
    }
    
    SECTION("Find stations with specific fuel type") {
        double lat = 52.520008;
        double lng = 13.404954;
        double radius = 5.0;
        
        auto stations = api.findStations(lat, lng, radius, "e10");
        REQUIRE_FALSE(stations.empty());
        
        // Check all stations have E10 prices
        for (const auto& station : stations) {
            bool hasE10 = false;
            for (const auto& price : station.prices) {
                if (price.fuelType == "e10") {
                    hasE10 = true;
                    break;
                }
            }
            CHECK(hasE10);
        }
    }
}

TEST_CASE("TankerkoenigAPI can get station details", "[api]") {
    TankerkoenigAPI api(TANKERKOENIG_API_KEY);
    
    SECTION("Get details for valid station") {
        // First find a station to get a valid ID
        auto stations = api.findStations(52.520008, 13.404954, 5.0);
        REQUIRE_FALSE(stations.empty());
        
        auto stationId = stations.front().id;
        auto details = api.getStationDetails(stationId);
        
        REQUIRE(details.has_value());
        CHECK(details->id == stationId);
        CHECK_FALSE(details->name.empty());
        CHECK_FALSE(details->prices.empty());
    }
    
    SECTION("Get details for invalid station") {
        auto details = api.getStationDetails("invalid-id");
        CHECK_FALSE(details.has_value());
    }
}

TEST_CASE("TankerkoenigAPI can get prices for multiple stations", "[api]") {
    TankerkoenigAPI api(TANKERKOENIG_API_KEY);
    
    SECTION("Get prices for valid stations") {
        // First find some stations
        auto stations = api.findStations(52.520008, 13.404954, 5.0);
        REQUIRE(stations.size() >= 2);
        
        std::vector<std::string> stationIds;
        stationIds.push_back(stations[0].id);
        stationIds.push_back(stations[1].id);
        
        auto priceInfo = api.getPrices(stationIds);
        REQUIRE(priceInfo.size() == 2);
        
        for (const auto& station : priceInfo) {
            CHECK_FALSE(station.prices.empty());
        }
    }
    
    SECTION("Get prices with some invalid stations") {
        std::vector<std::string> stationIds = {"invalid-id-1", "invalid-id-2"};
        auto priceInfo = api.getPrices(stationIds);
        CHECK(priceInfo.empty());
    }
} 
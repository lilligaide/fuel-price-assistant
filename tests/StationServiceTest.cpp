#include <gtest/gtest.h>
#include "../src/services/StationService.hpp"
#include "../src/utils/Config.hpp"

class StationServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        config.api.endpoint = "https://api.openchargemap.io/v3";
        config.api.key = "test-api-key";
        config.db_path = ":memory:";  // Use in-memory SQLite database for testing
        service = std::make_unique<StationService>(config);
    }
    
    Config config;
    std::unique_ptr<StationService> service;
};

TEST_F(StationServiceTest, FindNearbyStations_ValidCoordinates_ReturnsStations) {
    double lat = 51.5074;
    double lon = -0.1278;
    int radius = 5000;
    
    auto stations = service->findNearbyStations(lat, lon, radius);
    
    EXPECT_FALSE(stations.empty());
    for (const auto& station : stations) {
        EXPECT_GT(station.id, 0);
        EXPECT_FALSE(station.title.empty());
        EXPECT_GT(station.location.latitude, 0);
        EXPECT_LT(station.location.longitude, 0);
    }
}

TEST_F(StationServiceTest, FindNearbyStations_FilterByPower_ReturnsFilteredStations) {
    double lat = 51.5074;
    double lon = -0.1278;
    int radius = 5000;
    double minPower = 50.0;
    
    auto stations = service->findNearbyStations(lat, lon, radius, {}, minPower);
    
    EXPECT_FALSE(stations.empty());
    for (const auto& station : stations) {
        bool hasSufficientPower = false;
        for (const auto& conn : station.connections) {
            if (conn.powerKW >= minPower) {
                hasSufficientPower = true;
                break;
            }
        }
        EXPECT_TRUE(hasSufficientPower);
    }
}

TEST_F(StationServiceTest, FindNearbyStations_FilterByConnectorType_ReturnsFilteredStations) {
    double lat = 51.5074;
    double lon = -0.1278;
    int radius = 5000;
    std::vector<std::string> connectorTypes = {"CCS"};
    
    auto stations = service->findNearbyStations(lat, lon, radius, connectorTypes);
    
    EXPECT_FALSE(stations.empty());
    for (const auto& station : stations) {
        bool hasMatchingConnector = false;
        for (const auto& conn : station.connections) {
            if (conn.type == "CCS") {
                hasMatchingConnector = true;
                break;
            }
        }
        EXPECT_TRUE(hasMatchingConnector);
    }
}

TEST_F(StationServiceTest, GetStationById_ExistingStation_ReturnsStation) {
    // First find a station to get a valid ID
    auto stations = service->findNearbyStations(51.5074, -0.1278, 5000);
    ASSERT_FALSE(stations.empty());
    
    int64_t stationId = stations[0].id;
    auto station = service->getStationById(stationId);
    
    EXPECT_TRUE(station.has_value());
    EXPECT_EQ(station->id, stationId);
    EXPECT_FALSE(station->title.empty());
}

TEST_F(StationServiceTest, GetStationById_NonexistentStation_ReturnsNullopt) {
    auto station = service->getStationById(-1);
    EXPECT_FALSE(station.has_value());
}

TEST_F(StationServiceTest, FindStationsAlongRoute_ValidRoute_ReturnsStations) {
    std::vector<std::pair<double, double>> route = {
        {51.5074, -0.1278},  // London
        {51.7520, -1.2577}   // Oxford
    };
    int corridorWidth = 5000;
    
    auto stations = service->findStationsAlongRoute(route, corridorWidth);
    
    EXPECT_FALSE(stations.empty());
    
    // Check that stations are roughly along the route
    for (const auto& station : stations) {
        bool isNearRoute = false;
        for (size_t i = 0; i < route.size() - 1; ++i) {
            double midLat = (route[i].first + route[i+1].first) / 2;
            double midLon = (route[i].second + route[i+1].second) / 2;
            
            double distance = service->calculateDistance(
                midLat, midLon,
                station.location.latitude,
                station.location.longitude
            );
            
            if (distance * 1000 <= corridorWidth) {  // Convert km to meters
                isNearRoute = true;
                break;
            }
        }
        EXPECT_TRUE(isNearRoute);
    }
} 
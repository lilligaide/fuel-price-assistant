#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../include/utils/RouteCalculator.hpp"

using namespace utils;
using namespace models;

TEST_CASE("RouteCalculator can calculate distances", "[route]") {
    SECTION("Calculate distance between two points") {
        // Berlin to Hamburg
        double distance = RouteCalculator::calculateDistance(
            52.520008, 13.404954,  // Berlin
            53.551086, 9.993682    // Hamburg
        );
        
        // Expected distance is approximately 255 km
        CHECK_THAT(distance, Catch::Matchers::WithinRel(255.0, 0.1));
    }
    
    SECTION("Calculate distance for same point") {
        double distance = RouteCalculator::calculateDistance(
            52.520008, 13.404954,
            52.520008, 13.404954
        );
        
        CHECK_THAT(distance, Catch::Matchers::WithinAbs(0.0, 0.0001));
    }
}

TEST_CASE("RouteCalculator can find stations along route", "[route]") {
    std::vector<Waypoint> waypoints = {
        {52.520008, 13.404954},  // Berlin
        {53.551086, 9.993682},   // Hamburg
        {50.937531, 6.960279}    // Cologne
    };
    
    std::vector<FuelStation> stations;
    
    // Add test stations
    FuelStation station1;
    station1.id = "1";
    station1.name = "Station 1";
    station1.location.latitude = 52.520008;
    station1.location.longitude = 13.404954;
    stations.push_back(station1);
    
    FuelStation station2;
    station2.id = "2";
    station2.name = "Station 2";
    station2.location.latitude = 53.551086;
    station2.location.longitude = 9.993682;
    stations.push_back(station2);
    
    FuelStation station3;
    station3.id = "3";
    station3.name = "Station 3";
    station3.location.latitude = 51.000000;
    station3.location.longitude = 10.000000;
    stations.push_back(station3);
    
    SECTION("Find stations within corridor") {
        double corridorWidth = 5.0;  // 5 km
        auto result = RouteCalculator::findStationsAlongRoute(waypoints, stations, corridorWidth);
        
        // Stations at waypoints should be found
        REQUIRE(result.size() == 2);
        CHECK(result[0].id == "1");  // Berlin station
        CHECK(result[1].id == "2");  // Hamburg station
    }
    
    SECTION("Find stations with wider corridor") {
        double corridorWidth = 100.0;  // 100 km
        auto result = RouteCalculator::findStationsAlongRoute(waypoints, stations, corridorWidth);
        
        // All stations should be found
        REQUIRE(result.size() == 3);
    }
    
    SECTION("Find stations with empty waypoints") {
        std::vector<Waypoint> emptyWaypoints;
        auto result = RouteCalculator::findStationsAlongRoute(emptyWaypoints, stations, 5.0);
        
        CHECK(result.empty());
    }
    
    SECTION("Find stations with single waypoint") {
        std::vector<Waypoint> singleWaypoint = {waypoints[0]};
        auto result = RouteCalculator::findStationsAlongRoute(singleWaypoint, stations, 5.0);
        
        CHECK(result.empty());
    }
}

TEST_CASE("RouteCalculator can check if point is in corridor", "[route]") {
    Waypoint start{52.520008, 13.404954};  // Berlin
    Waypoint end{53.551086, 9.993682};     // Hamburg
    
    SECTION("Point on route should be in corridor") {
        // Point exactly between Berlin and Hamburg
        bool inCorridor = RouteCalculator::isPointInCorridor(
            start, end,
            53.035547, 11.699318,
            5.0  // 5 km corridor
        );
        
        CHECK(inCorridor);
    }
    
    SECTION("Point far from route should not be in corridor") {
        // Munich
        bool inCorridor = RouteCalculator::isPointInCorridor(
            start, end,
            48.137154, 11.576124,
            5.0  // 5 km corridor
        );
        
        CHECK_FALSE(inCorridor);
    }
    
    SECTION("Start point should be in corridor") {
        bool inCorridor = RouteCalculator::isPointInCorridor(
            start, end,
            start.latitude, start.longitude,
            5.0
        );
        
        CHECK(inCorridor);
    }
    
    SECTION("End point should be in corridor") {
        bool inCorridor = RouteCalculator::isPointInCorridor(
            start, end,
            end.latitude, end.longitude,
            5.0
        );
        
        CHECK(inCorridor);
    }
} 
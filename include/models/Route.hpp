#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "Location.hpp"

struct RoutePoint {
    Coordinates coordinates;
    std::string name;
    double distanceFromStart;  // in kilometers
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RoutePoint, coordinates, name, distanceFromStart)
};

struct Route {
    std::vector<RoutePoint> points;
    double totalDistance;  // in kilometers
    int estimatedDuration;  // in seconds
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Route, points, totalDistance, estimatedDuration)
}; 
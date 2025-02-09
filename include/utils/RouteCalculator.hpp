#pragma once

#include <vector>
#include <tuple>
#include <cmath>
#include "../models/FuelStation.hpp"

namespace utils {

struct Waypoint {
    double latitude;
    double longitude;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Waypoint, latitude, longitude)
};

struct RouteSegment {
    Waypoint start;
    Waypoint end;
    double distance;  // in kilometers
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RouteSegment, start, end, distance)
};

class RouteCalculator {
public:
    // Find stations along a route within a corridor
    static std::vector<models::FuelStation> findStationsAlongRoute(
        const std::vector<Waypoint>& waypoints,
        const std::vector<models::FuelStation>& allStations,
        double corridorWidth  // in kilometers
    );
    
    // Calculate distance between two points using Haversine formula
    static double calculateDistance(
        double lat1, double lon1,
        double lat2, double lon2
    );
    
    // Calculate if a point is within a corridor defined by two waypoints
    static bool isPointInCorridor(
        const Waypoint& start,
        const Waypoint& end,
        double lat, double lon,
        double corridorWidth
    );

private:
    // Split route into segments
    static std::vector<RouteSegment> createRouteSegments(
        const std::vector<Waypoint>& waypoints
    );
    
    // Calculate distance from point to line segment
    static double pointToSegmentDistance(
        double px, double py,
        double x1, double y1,
        double x2, double y2
    );
    
    // Convert degrees to radians
    static constexpr double toRadians(double degrees) {
        return degrees * M_PI / 180.0;
    }
    
    // Earth's radius in kilometers
    static constexpr double EARTH_RADIUS = 6371.0;
};

} // namespace utils 
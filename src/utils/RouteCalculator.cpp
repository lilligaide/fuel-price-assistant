#include "utils/RouteCalculator.hpp"
#include <algorithm>

namespace utils {

std::vector<models::FuelStation> RouteCalculator::findStationsAlongRoute(
    const std::vector<Waypoint>& waypoints,
    const std::vector<models::FuelStation>& allStations,
    double corridorWidth
) {
    if (waypoints.size() < 2) {
        return {};
    }

    // Create route segments
    auto segments = createRouteSegments(waypoints);
    
    // Find stations within corridor
    std::vector<models::FuelStation> result;
    for (const auto& station : allStations) {
        // Check each segment
        for (const auto& segment : segments) {
            if (isPointInCorridor(
                segment.start, segment.end,
                station.location.latitude,
                station.location.longitude,
                corridorWidth
            )) {
                // Calculate distance to route
                double distanceToRoute = pointToSegmentDistance(
                    station.location.latitude,
                    station.location.longitude,
                    segment.start.latitude,
                    segment.start.longitude,
                    segment.end.latitude,
                    segment.end.longitude
                );
                
                // Add distance to station object
                auto stationCopy = station;
                stationCopy.distance = distanceToRoute;
                result.push_back(stationCopy);
                break;  // Station is already added, skip other segments
            }
        }
    }
    
    // Sort by distance from route
    std::sort(result.begin(), result.end(),
        [](const auto& a, const auto& b) {
            return a.distance < b.distance;
        }
    );
    
    return result;
}

double RouteCalculator::calculateDistance(
    double lat1, double lon1,
    double lat2, double lon2
) {
    double lat1Rad = toRadians(lat1);
    double lon1Rad = toRadians(lon1);
    double lat2Rad = toRadians(lat2);
    double lon2Rad = toRadians(lon2);
    
    double dLat = lat2Rad - lat1Rad;
    double dLon = lon2Rad - lon1Rad;
    
    double a = std::sin(dLat/2) * std::sin(dLat/2) +
               std::cos(lat1Rad) * std::cos(lat2Rad) *
               std::sin(dLon/2) * std::sin(dLon/2);
               
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    
    return EARTH_RADIUS * c;
}

bool RouteCalculator::isPointInCorridor(
    const Waypoint& start,
    const Waypoint& end,
    double lat, double lon,
    double corridorWidth
) {
    double distance = pointToSegmentDistance(
        lat, lon,
        start.latitude, start.longitude,
        end.latitude, end.longitude
    );
    
    return distance <= corridorWidth;
}

std::vector<RouteSegment> RouteCalculator::createRouteSegments(
    const std::vector<Waypoint>& waypoints
) {
    std::vector<RouteSegment> segments;
    segments.reserve(waypoints.size() - 1);
    
    for (size_t i = 0; i < waypoints.size() - 1; ++i) {
        double distance = calculateDistance(
            waypoints[i].latitude, waypoints[i].longitude,
            waypoints[i+1].latitude, waypoints[i+1].longitude
        );
        
        segments.push_back({
            .start = waypoints[i],
            .end = waypoints[i+1],
            .distance = distance
        });
    }
    
    return segments;
}

double RouteCalculator::pointToSegmentDistance(
    double px, double py,
    double x1, double y1,
    double x2, double y2
) {
    // Convert to simpler coordinate system for calculation
    double dx = x2 - x1;
    double dy = y2 - y1;
    
    // If segment is actually a point
    if (dx == 0 && dy == 0) {
        return calculateDistance(px, py, x1, y1);
    }
    
    // Calculate projection
    double t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);
    
    if (t < 0) {
        // Point is beyond start of segment
        return calculateDistance(px, py, x1, y1);
    } else if (t > 1) {
        // Point is beyond end of segment
        return calculateDistance(px, py, x2, y2);
    }
    
    // Point projects onto segment
    double projX = x1 + t * dx;
    double projY = y1 + t * dy;
    return calculateDistance(px, py, projX, projY);
}

} // namespace utils 
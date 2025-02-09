# Fuel Price Assistant Documentation

## Route Calculation System

### Overview
The route calculation system helps users find fuel stations along their planned route. It uses a corridor-based approach to identify stations that are conveniently accessible from the route.

### Key Components

#### 1. Waypoints
- Represent points along the route (latitude/longitude pairs)
- Used to define the route's path
- Can be cities, intersections, or any points of interest
- Must have at least 2 waypoints to form a valid route

Example waypoints:
```cpp
std::vector<Waypoint> waypoints = {
    {52.520008, 13.404954},  // Berlin
    {53.551086, 9.993682},   // Hamburg
    {50.937531, 6.960279}    // Cologne
};
```

#### 2. Route Segments
- Created between consecutive waypoints
- Each segment has:
  - Start point (latitude/longitude)
  - End point (latitude/longitude)
  - Distance (in kilometers)
- Used to break down the route into manageable sections

Example segment:
```cpp
RouteSegment segment = {
    .start = {52.520008, 13.404954},  // Berlin
    .end = {53.551086, 9.993682},     // Hamburg
    .distance = 255.0                  // kilometers
};
```

#### 3. Corridor Width
- Defines how far from the route to search for stations
- Measured in kilometers
- Creates a buffer zone around the route
- Adjustable based on user preferences

### Calculation Methods

#### 1. Distance Calculation
```cpp
double distance = RouteCalculator::calculateDistance(
    52.520008, 13.404954,  // Point 1 (Berlin)
    53.551086, 9.993682    // Point 2 (Hamburg)
);
```
- Uses Haversine formula for accurate Earth distances
- Takes into account Earth's curvature
- Returns distance in kilometers

#### 2. Corridor Check
```cpp
bool inCorridor = RouteCalculator::isPointInCorridor(
    startPoint,    // Route segment start
    endPoint,      // Route segment end
    stationLat,    // Station latitude
    stationLon,    // Station longitude
    5.0           // Corridor width in km
);
```
- Determines if a point lies within the route corridor
- Uses perpendicular distance to route segments
- Handles edge cases at segment endpoints

#### 3. Station Finding
```cpp
auto stations = RouteCalculator::findStationsAlongRoute(
    waypoints,     // Route waypoints
    allStations,   // Available stations
    5.0           // Corridor width in km
);
```
- Finds all stations within the route corridor
- Calculates exact distance from route
- Sorts stations by distance from route
- Returns stations with their distances

### Usage Examples

1. Basic Route Query:
```cpp
// Define route waypoints
std::vector<Waypoint> waypoints = {
    {52.520008, 13.404954},  // Berlin
    {53.551086, 9.993682}    // Hamburg
};

// Find stations along route
auto stations = routeCalculator.findStationsAlongRoute(
    waypoints,
    allStations,
    5.0  // 5 km corridor
);
```

2. Multi-Stop Route:
```cpp
// Define route with multiple stops
std::vector<Waypoint> waypoints = {
    {52.520008, 13.404954},  // Berlin
    {53.551086, 9.993682},   // Hamburg
    {50.937531, 6.960279}    // Cologne
};

// Find stations with wider corridor
auto stations = routeCalculator.findStationsAlongRoute(
    waypoints,
    allStations,
    10.0  // 10 km corridor
);
```

### Testing

1. Distance Calculations:
```cpp
TEST_CASE("Calculate distance between cities") {
    double distance = RouteCalculator::calculateDistance(
        52.520008, 13.404954,  // Berlin
        53.551086, 9.993682    // Hamburg
    );
    CHECK_THAT(distance, WithinRel(255.0, 0.1));
}
```

2. Corridor Checks:
```cpp
TEST_CASE("Check if point is in corridor") {
    bool inCorridor = RouteCalculator::isPointInCorridor(
        berlinPoint,
        hamburgPoint,
        53.035547, 11.699318,  // Point between cities
        5.0                    // 5 km corridor
    );
    CHECK(inCorridor);
}
```

### Performance Considerations

1. Optimization Techniques:
   - Pre-calculation of route segments
   - Early exit from corridor checks
   - Efficient distance calculations
   - Sorted results by distance

2. Memory Usage:
   - Minimal copying of station data
   - Efficient vector usage
   - Stack-based calculations where possible

3. Accuracy vs. Performance:
   - Haversine formula for accuracy
   - Corridor approximation for speed
   - Balance between precision and performance

### Error Handling

1. Input Validation:
   - Minimum two waypoints required
   - Valid coordinate ranges
   - Positive corridor width

2. Edge Cases:
   - Empty route handling
   - Single waypoint handling
   - Zero-distance segments
   - Points at segment endpoints

### Best Practices

1. Route Planning:
   - Use reasonable corridor widths (5-10 km)
   - Include major intersections as waypoints
   - Space waypoints appropriately
   - Consider fuel consumption between points

2. Station Selection:
   - Check station operating hours
   - Verify fuel availability
   - Consider price variations
   - Account for detour time

3. Performance:
   - Cache route calculations
   - Limit number of waypoints
   - Use appropriate corridor width
   - Pre-filter stations by region 
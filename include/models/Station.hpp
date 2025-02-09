#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Location {
    double latitude;
    double longitude;
    std::string address;
    std::string city;
    std::string country;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Location, latitude, longitude, address, city, country)
};

struct Connection {
    std::string type;
    double powerKW;
    bool isOperational;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Connection, type, powerKW, isOperational)
};

struct ChargingStation {
    int64_t id;
    std::string title;
    Location location;
    std::vector<Connection> connections;
    
    // Additional metadata
    std::string operatorInfo;
    std::string accessType;  // public, private, restricted
    bool isOpen24Hours;
    std::string lastVerified;
    
    // Calculated fields
    double distance_km;  // Distance from search point
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ChargingStation, id, title, location, connections,
                                  operatorInfo, accessType, isOpen24Hours, lastVerified,
                                  distance_km)
}; 
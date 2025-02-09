#pragma once

#include <string>
#include <nlohmann/json.hpp>

struct Coordinates {
    double latitude;
    double longitude;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Coordinates, latitude, longitude)
};

struct Address {
    std::string street;
    std::string city;
    std::string state;
    std::string country;
    std::string postalCode;
    std::string formatted;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Address, street, city, state, country, postalCode, formatted)
};

struct Location {
    Coordinates coordinates;
    Address address;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Location, coordinates, address)
}; 
#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace models {

struct FuelPrice {
    std::string fuelType;  // e.g., "e5", "e10", "diesel"
    double price;
    std::string lastUpdate;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(FuelPrice, fuelType, price, lastUpdate)
};

struct Location {
    double latitude;
    double longitude;
    std::string street;
    std::string houseNumber;
    std::string postalCode;
    std::string city;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Location, latitude, longitude, street, houseNumber, postalCode, city)
};

struct FuelStation {
    std::string id;
    std::string name;
    std::string brand;
    Location location;
    bool isOpen;
    std::vector<FuelPrice> prices;
    double distance;  // Distance from search point in km
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(FuelStation, id, name, brand, location, isOpen, prices, distance)
};

} // namespace models 
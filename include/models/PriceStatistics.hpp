#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <nlohmann/json.hpp>
#include "FuelStation.hpp"

namespace models {

struct DailyStatistics {
    double minPrice;
    double maxPrice;
    double avgPrice;
    std::string bestTimeToRefuel;  // HH:MM format
    std::string worstTimeToRefuel; // HH:MM format
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DailyStatistics, minPrice, maxPrice, avgPrice, 
                                  bestTimeToRefuel, worstTimeToRefuel)
};

struct WeeklyStatistics {
    std::map<std::string, DailyStatistics> dailyStats;  // Key: day of week (Mon, Tue, etc.)
    std::string cheapestDay;
    std::string mostExpensiveDay;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(WeeklyStatistics, dailyStats, cheapestDay, mostExpensiveDay)
};

struct StationStatistics {
    std::string stationId;
    std::string stationName;
    WeeklyStatistics weeklyStats;
    double averagePrice;
    bool isTypicallyCheapest;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(StationStatistics, stationId, stationName, weeklyStats, 
                                  averagePrice, isTypicallyCheapest)
};

struct PriceStatistics {
    std::string fuelType;
    std::vector<StationStatistics> stationStats;
    double areaAveragePrice;
    std::string lastUpdate;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PriceStatistics, fuelType, stationStats, 
                                  areaAveragePrice, lastUpdate)
};

} // namespace models 
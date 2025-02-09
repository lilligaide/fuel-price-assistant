#pragma once

#include "../models/Station.hpp"
#include <drogon/orm/DbClient.h>
#include <vector>
#include <optional>
#include <memory>
#include <string>

class Database {
public:
    explicit Database(const std::string& dbPath);
    ~Database();
    
    // Station operations
    std::vector<ChargingStation> getStationsInRadius(
        double latitude,
        double longitude,
        int radiusMeters
    );
    
    std::optional<ChargingStation> getStationById(int64_t id);
    
    void updateStations(const std::vector<ChargingStation>& stations);
    
    // Cache management
    void cleanExpiredCache();
    
private:
    void initializeDatabase();
    void createTables();
    
    std::shared_ptr<drogon::orm::DbClient> client;
    std::string dbPath;
    
    // SQL statements
    static constexpr const char* CREATE_STATIONS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS stations (
            id INTEGER PRIMARY KEY,
            title TEXT NOT NULL,
            latitude REAL NOT NULL,
            longitude REAL NOT NULL,
            address TEXT,
            city TEXT,
            country TEXT,
            operator_info TEXT,
            access_type TEXT,
            is_open_24h BOOLEAN,
            last_verified TEXT,
            cached_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    static constexpr const char* CREATE_CONNECTIONS_TABLE = R"(
        CREATE TABLE IF NOT EXISTS connections (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            station_id INTEGER NOT NULL,
            type TEXT NOT NULL,
            power_kw REAL NOT NULL,
            is_operational BOOLEAN,
            FOREIGN KEY (station_id) REFERENCES stations(id) ON DELETE CASCADE
        )
    )";
    
    static constexpr const char* CREATE_SPATIAL_INDEX = R"(
        CREATE INDEX IF NOT EXISTS idx_stations_location 
        ON stations (latitude, longitude)
    )";
}; 
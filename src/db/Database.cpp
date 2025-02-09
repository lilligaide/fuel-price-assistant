#include "Database.hpp"
#include "../utils/Logger.hpp"
#include <drogon/orm/Exception.h>
#include <cmath>

Database::Database(const std::string& dbPath) : dbPath(dbPath) {
    try {
        client = drogon::orm::DbClient::newSqlite3Client(
            fmt::format("filename={}", dbPath),
            1  // Number of connections in pool
        );
        initializeDatabase();
    } catch (const drogon::orm::Exception& e) {
        Logger::error("Failed to initialize database: {}", e.base().what());
        throw;
    }
}

Database::~Database() {
    // Cleanup will be handled by shared_ptr
}

void Database::initializeDatabase() {
    try {
        createTables();
        Logger::info("Database initialized successfully");
    } catch (const drogon::orm::Exception& e) {
        Logger::error("Database initialization failed: {}", e.base().what());
        throw;
    }
}

void Database::createTables() {
    auto trans = client->newTransaction();
    trans->execSqlSync(CREATE_STATIONS_TABLE);
    trans->execSqlSync(CREATE_CONNECTIONS_TABLE);
    trans->execSqlSync(CREATE_SPATIAL_INDEX);
}

std::vector<ChargingStation> Database::getStationsInRadius(
    double latitude,
    double longitude,
    int radiusMeters
) {
    std::vector<ChargingStation> stations;
    
    try {
        // Convert radius to rough lat/lon bounds for initial filtering
        double radiusInDegrees = radiusMeters / 111000.0;  // rough conversion
        
        auto result = client->execSqlSync(
            "SELECT s.*, c.type, c.power_kw, c.is_operational "
            "FROM stations s "
            "LEFT JOIN connections c ON s.id = c.station_id "
            "WHERE s.latitude BETWEEN ? AND ? "
            "AND s.longitude BETWEEN ? AND ? "
            "AND (cached_at > datetime('now', '-1 hour'))",
            latitude - radiusInDegrees,
            latitude + radiusInDegrees,
            longitude - radiusInDegrees,
            longitude + radiusInDegrees
        );
        
        // Process results and construct ChargingStation objects
        int64_t currentStationId = -1;
        ChargingStation* currentStation = nullptr;
        
        for (const auto& row : result) {
            int64_t stationId = row["id"].as<int64_t>();
            
            if (stationId != currentStationId) {
                if (currentStation) {
                    stations.push_back(*currentStation);
                }
                
                stations.emplace_back();
                currentStation = &stations.back();
                currentStationId = stationId;
                
                // Fill station data
                currentStation->id = stationId;
                currentStation->title = row["title"].as<std::string>();
                currentStation->location.latitude = row["latitude"].as<double>();
                currentStation->location.longitude = row["longitude"].as<double>();
                currentStation->location.address = row["address"].as<std::string>();
                currentStation->location.city = row["city"].as<std::string>();
                currentStation->location.country = row["country"].as<std::string>();
                currentStation->operatorInfo = row["operator_info"].as<std::string>();
                currentStation->accessType = row["access_type"].as<std::string>();
                currentStation->isOpen24Hours = row["is_open_24h"].as<bool>();
                currentStation->lastVerified = row["last_verified"].as<std::string>();
            }
            
            // Add connection if present
            if (!row["type"].isNull()) {
                Connection conn;
                conn.type = row["type"].as<std::string>();
                conn.powerKW = row["power_kw"].as<double>();
                conn.isOperational = row["is_operational"].as<bool>();
                currentStation->connections.push_back(conn);
            }
        }
        
        // Add the last station if exists
        if (currentStation && !stations.empty() && 
            stations.back().id != currentStationId) {
            stations.push_back(*currentStation);
        }
        
    } catch (const drogon::orm::Exception& e) {
        Logger::error("Database query failed: {}", e.base().what());
    }
    
    return stations;
}

std::optional<ChargingStation> Database::getStationById(int64_t id) {
    try {
        auto result = client->execSqlSync(
            "SELECT s.*, c.type, c.power_kw, c.is_operational "
            "FROM stations s "
            "LEFT JOIN connections c ON s.id = c.station_id "
            "WHERE s.id = ?",
            id
        );
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        ChargingStation station;
        station.id = id;
        
        // Fill station data from first row
        const auto& firstRow = result.front();
        station.title = firstRow["title"].as<std::string>();
        station.location.latitude = firstRow["latitude"].as<double>();
        station.location.longitude = firstRow["longitude"].as<double>();
        station.location.address = firstRow["address"].as<std::string>();
        station.location.city = firstRow["city"].as<std::string>();
        station.location.country = firstRow["country"].as<std::string>();
        station.operatorInfo = firstRow["operator_info"].as<std::string>();
        station.accessType = firstRow["access_type"].as<std::string>();
        station.isOpen24Hours = firstRow["is_open_24h"].as<bool>();
        station.lastVerified = firstRow["last_verified"].as<std::string>();
        
        // Add connections from all rows
        for (const auto& row : result) {
            if (!row["type"].isNull()) {
                Connection conn;
                conn.type = row["type"].as<std::string>();
                conn.powerKW = row["power_kw"].as<double>();
                conn.isOperational = row["is_operational"].as<bool>();
                station.connections.push_back(conn);
            }
        }
        
        return station;
        
    } catch (const drogon::orm::Exception& e) {
        Logger::error("Database query failed: {}", e.base().what());
        return std::nullopt;
    }
}

void Database::updateStations(const std::vector<ChargingStation>& stations) {
    try {
        auto trans = client->newTransaction();
        
        for (const auto& station : stations) {
            // Update or insert station
            trans->execSqlSync(
                "INSERT OR REPLACE INTO stations "
                "(id, title, latitude, longitude, address, city, country, "
                "operator_info, access_type, is_open_24h, last_verified) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                station.id,
                station.title,
                station.location.latitude,
                station.location.longitude,
                station.location.address,
                station.location.city,
                station.location.country,
                station.operatorInfo,
                station.accessType,
                station.isOpen24Hours,
                station.lastVerified
            );
            
            // Delete old connections
            trans->execSqlSync(
                "DELETE FROM connections WHERE station_id = ?",
                station.id
            );
            
            // Insert new connections
            for (const auto& conn : station.connections) {
                trans->execSqlSync(
                    "INSERT INTO connections "
                    "(station_id, type, power_kw, is_operational) "
                    "VALUES (?, ?, ?, ?)",
                    station.id,
                    conn.type,
                    conn.powerKW,
                    conn.isOperational
                );
            }
        }
        
        trans->commit();
        Logger::info("Updated {} stations in database", stations.size());
        
    } catch (const drogon::orm::Exception& e) {
        Logger::error("Failed to update stations: {}", e.base().what());
        throw;
    }
}

void Database::cleanExpiredCache() {
    try {
        auto result = client->execSqlSync(
            "DELETE FROM stations WHERE cached_at < datetime('now', '-1 hour')"
        );
        Logger::info("Cleaned expired cache entries");
    } catch (const drogon::orm::Exception& e) {
        Logger::error("Failed to clean expired cache: {}", e.base().what());
    }
} 
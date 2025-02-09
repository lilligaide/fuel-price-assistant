#include <gtest/gtest.h>
#include "../src/db/Database.hpp"
#include <memory>
#include <vector>

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_unique<Database>(":memory:");  // Use in-memory SQLite database
    }
    
    void TearDown() override {
        db.reset();
    }
    
    ChargingStation createTestStation(int64_t id) {
        ChargingStation station;
        station.id = id;
        station.title = "Test Station " + std::to_string(id);
        station.location.latitude = 51.5074;
        station.location.longitude = -0.1278;
        station.location.address = "123 Test St";
        station.location.city = "London";
        station.location.country = "UK";
        station.operatorInfo = "Test Operator";
        station.accessType = "public";
        station.isOpen24Hours = true;
        station.lastVerified = "2024-01-01";
        
        Connection conn1;
        conn1.type = "CCS";
        conn1.powerKW = 50.0;
        conn1.isOperational = true;
        station.connections.push_back(conn1);
        
        Connection conn2;
        conn2.type = "Type2";
        conn2.powerKW = 22.0;
        conn2.isOperational = true;
        station.connections.push_back(conn2);
        
        return station;
    }
    
    std::unique_ptr<Database> db;
};

TEST_F(DatabaseTest, UpdateAndRetrieveStation) {
    // Create and store a test station
    auto station = createTestStation(1);
    std::vector<ChargingStation> stations = {station};
    db->updateStations(stations);
    
    // Retrieve the station
    auto retrieved = db->getStationById(1);
    
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->id, station.id);
    EXPECT_EQ(retrieved->title, station.title);
    EXPECT_EQ(retrieved->location.latitude, station.location.latitude);
    EXPECT_EQ(retrieved->location.longitude, station.location.longitude);
    EXPECT_EQ(retrieved->location.address, station.location.address);
    EXPECT_EQ(retrieved->connections.size(), station.connections.size());
}

TEST_F(DatabaseTest, GetStationsInRadius) {
    // Create and store multiple test stations
    std::vector<ChargingStation> stations;
    for (int i = 1; i <= 5; ++i) {
        auto station = createTestStation(i);
        // Adjust locations to create a pattern
        station.location.latitude += i * 0.001;
        station.location.longitude += i * 0.001;
        stations.push_back(station);
    }
    db->updateStations(stations);
    
    // Search for stations in radius
    double centerLat = 51.5074;
    double centerLon = -0.1278;
    int radiusMeters = 5000;
    
    auto found = db->getStationsInRadius(centerLat, centerLon, radiusMeters);
    
    EXPECT_FALSE(found.empty());
    EXPECT_LE(found.size(), stations.size());
}

TEST_F(DatabaseTest, UpdateExistingStation) {
    // Create and store initial station
    auto station = createTestStation(1);
    db->updateStations({station});
    
    // Modify and update the station
    station.title = "Updated Station";
    station.connections.push_back({
        .type = "CHAdeMO",
        .powerKW = 62.5,
        .isOperational = true
    });
    
    db->updateStations({station});
    
    // Retrieve and verify the update
    auto retrieved = db->getStationById(1);
    
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->title, "Updated Station");
    EXPECT_EQ(retrieved->connections.size(), 3);
}

TEST_F(DatabaseTest, NonexistentStation) {
    auto station = db->getStationById(-1);
    EXPECT_FALSE(station.has_value());
}

TEST_F(DatabaseTest, CacheExpiration) {
    // Create and store a test station
    auto station = createTestStation(1);
    db->updateStations({station});
    
    // Clean expired cache
    db->cleanExpiredCache();
    
    // Verify the station is still there (shouldn't be expired yet)
    auto retrieved = db->getStationById(1);
    EXPECT_TRUE(retrieved.has_value());
    
    // Note: Testing actual expiration would require time manipulation,
    // which is beyond the scope of this basic test
}

TEST_F(DatabaseTest, BulkUpdate) {
    // Create multiple stations
    std::vector<ChargingStation> stations;
    for (int i = 1; i <= 100; ++i) {
        stations.push_back(createTestStation(i));
    }
    
    // Bulk update
    EXPECT_NO_THROW(db->updateStations(stations));
    
    // Verify a few random stations
    for (int i : {1, 50, 100}) {
        auto retrieved = db->getStationById(i);
        ASSERT_TRUE(retrieved.has_value());
        EXPECT_EQ(retrieved->id, i);
    }
}

TEST_F(DatabaseTest, RadiusSearch_EmptyResult) {
    // Search in an area where we haven't added any stations
    double centerLat = 0.0;
    double centerLon = 0.0;
    int radiusMeters = 1000;
    
    auto found = db->getStationsInRadius(centerLat, centerLon, radiusMeters);
    
    EXPECT_TRUE(found.empty());
} 
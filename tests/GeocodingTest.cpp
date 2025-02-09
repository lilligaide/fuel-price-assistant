#include <gtest/gtest.h>
#include "../src/services/GeocodingService.hpp"
#include "../src/utils/Config.hpp"
#include <memory>

class GeocodingTest : public ::testing::Test {
protected:
    void SetUp() override {
        config.api.endpoint = "https://api.openchargemap.io/v3";
        config.api.key = "test-api-key";
        service = std::make_unique<GeocodingService>(config);
    }
    
    Config config;
    std::unique_ptr<GeocodingService> service;
};

TEST_F(GeocodingTest, GeocodeAddress_ValidAddress_ReturnsLocation) {
    auto location = service->geocodeAddress("10 Downing Street, London, UK");
    
    ASSERT_TRUE(location.has_value());
    EXPECT_NEAR(location->lat, 51.5034, 0.01);  // Approximate coordinates
    EXPECT_NEAR(location->lon, -0.1276, 0.01);
    EXPECT_FALSE(location->address.empty());
}

TEST_F(GeocodingTest, GeocodeAddress_InvalidAddress_ReturnsNullopt) {
    auto location = service->geocodeAddress("Invalid Address 12345, Nowhere");
    EXPECT_FALSE(location.has_value());
}

TEST_F(GeocodingTest, GeocodeAddress_EmptyString_ReturnsNullopt) {
    auto location = service->geocodeAddress("");
    EXPECT_FALSE(location.has_value());
}

TEST_F(GeocodingTest, ReverseGeocode_ValidCoordinates_ReturnsAddress) {
    auto address = service->reverseGeocode(51.5074, -0.1278);  // London coordinates
    EXPECT_FALSE(address.empty());
    EXPECT_TRUE(address.find("London") != std::string::npos);
}

TEST_F(GeocodingTest, ReverseGeocode_InvalidCoordinates_ReturnsEmptyString) {
    auto address = service->reverseGeocode(91.0, 181.0);  // Invalid coordinates
    EXPECT_TRUE(address.empty());
}

TEST_F(GeocodingTest, GeocodeAddress_SpecialCharacters_HandlesCorrectly) {
    auto location = service->geocodeAddress("Königstraße 1, Berlin, Germany");
    
    ASSERT_TRUE(location.has_value());
    EXPECT_FALSE(location->address.empty());
}

TEST_F(GeocodingTest, GeocodeAddress_MultipleResults_ReturnsFirstMatch) {
    auto location = service->geocodeAddress("Main Street, London");
    
    ASSERT_TRUE(location.has_value());
    EXPECT_FALSE(location->address.empty());
}

TEST_F(GeocodingTest, ReverseGeocode_WaterLocation_ReturnsNearestLand) {
    // Coordinates in the English Channel
    auto address = service->reverseGeocode(50.5, -1.0);
    EXPECT_FALSE(address.empty());
}

TEST_F(GeocodingTest, GeocodeAddress_PostalCode_ReturnsLocation) {
    auto location = service->geocodeAddress("SW1A 1AA, London, UK");
    
    ASSERT_TRUE(location.has_value());
    EXPECT_NEAR(location->lat, 51.5014, 0.01);
    EXPECT_NEAR(location->lon, -0.1419, 0.01);
}

TEST_F(GeocodingTest, ReverseGeocode_PreciseLocation_ReturnsDetailedAddress) {
    // Coordinates of Big Ben
    auto address = service->reverseGeocode(51.500729, -0.124625);
    
    EXPECT_FALSE(address.empty());
    EXPECT_TRUE(address.find("Westminster") != std::string::npos ||
                address.find("London") != std::string::npos);
} 
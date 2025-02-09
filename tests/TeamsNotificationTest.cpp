#include <catch2/catch_test_macros.hpp>
#include "../include/notifications/TeamsNotificationService.hpp"

using namespace notifications;
using namespace models;

TEST_CASE("TeamsNotificationService can be constructed with webhook URL", "[notifications]") {
    REQUIRE_NOTHROW(TeamsNotificationService("https://example.webhook.office.com/test"));
}

TEST_CASE("TeamsNotificationService can send price alerts", "[notifications]") {
    TeamsNotificationService service(TEAMS_WEBHOOK_URL);  // Set via environment variable
    
    SECTION("Send price decrease alert") {
        PriceAlertMessage message;
        message.title = "Price Drop Alert!";
        message.body = "Fuel prices have decreased at your favorite station";
        message.timestamp = "2024-01-20T10:00:00Z";
        message.station = FuelStation{
            .id = "test-station-1",
            .name = "Test Station",
            .brand = "TestBrand",
            .location = Location{
                .latitude = 52.520008,
                .longitude = 13.404954,
                .street = "Test Street",
                .houseNumber = "123",
                .postalCode = "12345",
                .city = "Berlin"
            },
            .isOpen = true,
            .prices = {
                FuelPrice{
                    .fuelType = "e10",
                    .price = 1.799,
                    .lastUpdate = "2024-01-20T10:00:00Z"
                }
            },
            .distance = 1.5
        };
        message.previousPrice = 1.899;
        message.currentPrice = 1.799;
        message.priceChange = -0.10;
        message.isBestPrice = true;
        
        REQUIRE_NOTHROW(service.sendPriceAlert(message));
    }
    
    SECTION("Send price increase alert") {
        PriceAlertMessage message;
        message.title = "Price Increase Alert";
        message.body = "Fuel prices have increased at your monitored station";
        message.timestamp = "2024-01-20T10:00:00Z";
        message.station = FuelStation{
            .id = "test-station-2",
            .name = "Another Station",
            .brand = "TestBrand",
            .location = Location{
                .latitude = 52.520008,
                .longitude = 13.404954,
                .street = "Test Avenue",
                .houseNumber = "456",
                .postalCode = "12345",
                .city = "Berlin"
            },
            .isOpen = true,
            .prices = {
                FuelPrice{
                    .fuelType = "diesel",
                    .price = 1.699,
                    .lastUpdate = "2024-01-20T10:00:00Z"
                }
            },
            .distance = 2.0
        };
        message.previousPrice = 1.599;
        message.currentPrice = 1.699;
        message.priceChange = 0.10;
        message.isBestPrice = false;
        
        REQUIRE_NOTHROW(service.sendPriceAlert(message));
    }
}

TEST_CASE("TeamsNotificationService can send statistics reports", "[notifications]") {
    TeamsNotificationService service(TEAMS_WEBHOOK_URL);
    
    SECTION("Send weekly statistics report") {
        StatisticsReportMessage message;
        message.title = "Weekly Fuel Price Report";
        message.body = "Here's your weekly summary of fuel prices";
        message.timestamp = "2024-01-20T10:00:00Z";
        message.reportPeriod = "Weekly";
        
        PriceStatistics stats;
        stats.fuelType = "e10";
        stats.areaAveragePrice = 1.799;
        stats.lastUpdate = "2024-01-20T10:00:00Z";
        
        StationStatistics stationStats;
        stationStats.stationId = "test-station-1";
        stationStats.stationName = "Test Station";
        stationStats.averagePrice = 1.789;
        stationStats.isTypicallyCheapest = true;
        
        WeeklyStatistics weeklyStats;
        weeklyStats.cheapestDay = "Tuesday";
        weeklyStats.mostExpensiveDay = "Friday";
        
        DailyStatistics tuesdayStats;
        tuesdayStats.minPrice = 1.759;
        tuesdayStats.maxPrice = 1.819;
        tuesdayStats.avgPrice = 1.789;
        tuesdayStats.bestTimeToRefuel = "18:00";
        tuesdayStats.worstTimeToRefuel = "07:00";
        
        weeklyStats.dailyStats["Tuesday"] = tuesdayStats;
        stationStats.weeklyStats = weeklyStats;
        stats.stationStats.push_back(stationStats);
        
        message.statistics = stats;
        
        REQUIRE_NOTHROW(service.sendStatisticsReport(message));
    }
} 
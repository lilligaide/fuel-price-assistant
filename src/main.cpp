#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <fmt/format.h>
#include "api/TankerkoenigAPI.hpp"
#include "notifications/TeamsNotificationService.hpp"
#include "utils/Config.hpp"

using namespace std::chrono_literals;

class FuelPriceMonitor {
public:
    explicit FuelPriceMonitor(const utils::Config& config)
        : config(config), api(config.apiKey) {
        // Initialize notification services
        for (const auto& notifConfig : config.notifications) {
            if (notifConfig.type == "teams") {
                auto webhookUrl = notifConfig.settings.at("webhookUrl");
                notificationServices.push_back(
                    std::make_unique<notifications::TeamsNotificationService>(webhookUrl)
                );
            }
        }

        // Initialize price history
        updatePrices();
    }

    void run() {
        std::cout << "Starting fuel price monitoring..." << std::endl;
        
        while (true) {
            try {
                updatePrices();
                checkPriceChanges();
                
                // Sleep for the configured interval
                std::this_thread::sleep_for(std::chrono::minutes(config.monitoring.updateInterval));
            } catch (const std::exception& e) {
                std::cerr << fmt::format("Error during monitoring: {}", e.what()) << std::endl;
                // Sleep for a minute before retrying
                std::this_thread::sleep_for(1min);
            }
        }
    }

private:
    void updatePrices() {
        for (const auto& fuelType : config.monitoring.fuelTypes) {
            auto stations = api.findStations(
                config.location.latitude,
                config.location.longitude,
                config.location.searchRadius,
                fuelType
            );

            for (const auto& station : stations) {
                for (const auto& price : station.prices) {
                    if (price.fuelType == fuelType) {
                        auto key = std::make_pair(station.id, fuelType);
                        priceHistory[key] = price.price;
                    }
                }
            }
        }
    }

    void checkPriceChanges() {
        for (const auto& fuelType : config.monitoring.fuelTypes) {
            auto stations = api.findStations(
                config.location.latitude,
                config.location.longitude,
                config.location.searchRadius,
                fuelType
            );

            for (const auto& station : stations) {
                for (const auto& price : station.prices) {
                    if (price.fuelType != fuelType) continue;

                    auto key = std::make_pair(station.id, fuelType);
                    auto it = priceHistory.find(key);
                    
                    if (it != priceHistory.end()) {
                        double priceChange = price.price - it->second;
                        
                        // Check if price change exceeds threshold
                        if (std::abs(priceChange) >= config.monitoring.priceThreshold) {
                            if (priceChange < 0 || config.monitoring.notifyOnIncrease) {
                                sendPriceAlert(station, price, it->second, priceChange);
                            }
                        }
                        
                        // Update price history
                        it->second = price.price;
                    }
                }
            }
        }
    }

    void sendPriceAlert(
        const models::FuelStation& station,
        const models::FuelPrice& price,
        double previousPrice,
        double priceChange
    ) {
        notifications::PriceAlertMessage message;
        message.title = priceChange < 0 ? "⬇️ Price Drop Alert!" : "⬆️ Price Increase Alert";
        message.body = fmt::format(
            "{} prices at {} have {} by {:.3f}€",
            price.fuelType,
            station.name,
            priceChange < 0 ? "decreased" : "increased",
            std::abs(priceChange)
        );
        message.timestamp = price.lastUpdate;
        message.station = station;
        message.previousPrice = previousPrice;
        message.currentPrice = price.price;
        message.priceChange = priceChange;
        
        // Find if this is the best price in the area
        message.isBestPrice = true;
        for (const auto& [key, otherPrice] : priceHistory) {
            if (key.second == price.fuelType && otherPrice < price.price) {
                message.isBestPrice = false;
                break;
            }
        }

        for (const auto& service : notificationServices) {
            service->sendPriceAlert(message);
        }
    }

    utils::Config config;
    api::TankerkoenigAPI api;
    std::vector<std::unique_ptr<notifications::NotificationService>> notificationServices;
    std::map<std::pair<std::string, std::string>, double> priceHistory;  // (stationId, fuelType) -> price
};

int main(int argc, char* argv[]) {
    try {
        utils::Config config;
        
        if (argc > 1) {
            config = utils::Config::load(argv[1]);
        } else {
            config = utils::Config::fromEnvironment();
        }

        FuelPriceMonitor monitor(config);
        monitor.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << fmt::format("Fatal error: {}", e.what()) << std::endl;
        return 1;
    }
} 
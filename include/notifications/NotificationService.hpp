#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../models/FuelStation.hpp"
#include "../models/PriceStatistics.hpp"

namespace notifications {

// Base class for notification messages
struct NotificationMessage {
    std::string title;
    std::string body;
    std::string timestamp;
    
    virtual ~NotificationMessage() = default;
};

// Price alert notification
struct PriceAlertMessage : NotificationMessage {
    models::FuelStation station;
    double previousPrice;
    double currentPrice;
    double priceChange;
    bool isBestPrice;
};

// Statistics report notification
struct StatisticsReportMessage : NotificationMessage {
    models::PriceStatistics statistics;
    std::string reportPeriod;  // e.g., "Weekly", "Daily"
};

// Interface for notification services
class NotificationService {
public:
    virtual ~NotificationService() = default;
    
    virtual void sendPriceAlert(const PriceAlertMessage& message) = 0;
    virtual void sendStatisticsReport(const StatisticsReportMessage& message) = 0;
    
protected:
    virtual std::string formatMessage(const NotificationMessage& message) = 0;
};

// Factory function to create notification services
std::unique_ptr<NotificationService> createNotificationService(
    const std::string& type,
    const std::map<std::string, std::string>& config
);

} // namespace notifications 
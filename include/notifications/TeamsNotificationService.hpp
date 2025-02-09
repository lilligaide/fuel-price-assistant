#pragma once

#include "NotificationService.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace notifications {

class TeamsNotificationService : public NotificationService {
public:
    explicit TeamsNotificationService(const std::string& webhookUrl);
    ~TeamsNotificationService();
    
    // Implement notification methods
    void sendPriceAlert(const PriceAlertMessage& message) override;
    void sendStatisticsReport(const StatisticsReportMessage& message) override;

private:
    std::string formatMessage(const NotificationMessage& message) override;
    nlohmann::json createAdaptiveCard(const NotificationMessage& message);
    bool sendWebhookRequest(const nlohmann::json& payload);
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    CURL* curl;
    std::string webhookUrl;
    static constexpr int TIMEOUT_SECONDS = 10;
    
    // Card templates
    static const char* PRICE_ALERT_TEMPLATE;
    static const char* STATISTICS_REPORT_TEMPLATE;
};

} // namespace notifications 
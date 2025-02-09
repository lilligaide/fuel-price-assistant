#include "notifications/TeamsNotificationService.hpp"
#include <fmt/format.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace notifications {

// Define card templates
const char* TeamsNotificationService::PRICE_ALERT_TEMPLATE = R"({
    "type": "AdaptiveCard",
    "version": "1.4",
    "body": [
        {
            "type": "TextBlock",
            "size": "Large",
            "weight": "Bolder",
            "text": "${title}"
        },
        {
            "type": "FactSet",
            "facts": [
                {
                    "title": "Station",
                    "value": "${station_name}"
                },
                {
                    "title": "Address",
                    "value": "${address}"
                },
                {
                    "title": "Previous Price",
                    "value": "${previous_price} €"
                },
                {
                    "title": "Current Price",
                    "value": "${current_price} €"
                },
                {
                    "title": "Change",
                    "value": "${price_change} €"
                }
            ]
        },
        {
            "type": "TextBlock",
            "text": "${message}",
            "wrap": true
        }
    ]
})";

const char* TeamsNotificationService::STATISTICS_REPORT_TEMPLATE = R"({
    "type": "AdaptiveCard",
    "version": "1.4",
    "body": [
        {
            "type": "TextBlock",
            "size": "Large",
            "weight": "Bolder",
            "text": "${title}"
        },
        {
            "type": "FactSet",
            "facts": [
                {
                    "title": "Period",
                    "value": "${period}"
                },
                {
                    "title": "Fuel Type",
                    "value": "${fuel_type}"
                },
                {
                    "title": "Area Average",
                    "value": "${area_average} €"
                },
                {
                    "title": "Best Day",
                    "value": "${best_day}"
                },
                {
                    "title": "Best Time",
                    "value": "${best_time}"
                }
            ]
        },
        {
            "type": "TextBlock",
            "text": "${summary}",
            "wrap": true
        }
    ]
})";

TeamsNotificationService::TeamsNotificationService(const std::string& webhookUrl)
    : webhookUrl(webhookUrl) {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

TeamsNotificationService::~TeamsNotificationService() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

void TeamsNotificationService::sendPriceAlert(const PriceAlertMessage& message) {
    auto card = createAdaptiveCard(message);
    if (!sendWebhookRequest(card)) {
        throw std::runtime_error("Failed to send price alert to Teams");
    }
}

void TeamsNotificationService::sendStatisticsReport(const StatisticsReportMessage& message) {
    auto card = createAdaptiveCard(message);
    if (!sendWebhookRequest(card)) {
        throw std::runtime_error("Failed to send statistics report to Teams");
    }
}

std::string TeamsNotificationService::formatMessage(const NotificationMessage& message) {
    return fmt::format("{}\n\n{}", message.title, message.body);
}

nlohmann::json TeamsNotificationService::createAdaptiveCard(const NotificationMessage& message) {
    if (auto priceAlert = dynamic_cast<const PriceAlertMessage*>(&message)) {
        std::string address = fmt::format("{} {}, {} {}",
            priceAlert->station.location.street,
            priceAlert->station.location.houseNumber,
            priceAlert->station.location.postalCode,
            priceAlert->station.location.city);

        std::string priceChangeStr = fmt::format("{:+.3f}", priceAlert->priceChange);
        std::string currentPriceStr = fmt::format("{:.3f}", priceAlert->currentPrice);
        std::string previousPriceStr = fmt::format("{:.3f}", priceAlert->previousPrice);

        nlohmann::json card = nlohmann::json::parse(PRICE_ALERT_TEMPLATE);
        card["body"][0]["text"] = priceAlert->title;
        card["body"][1]["facts"][0]["value"] = priceAlert->station.name;
        card["body"][1]["facts"][1]["value"] = address;
        card["body"][1]["facts"][2]["value"] = previousPriceStr + " €";
        card["body"][1]["facts"][3]["value"] = currentPriceStr + " €";
        card["body"][1]["facts"][4]["value"] = priceChangeStr + " €";
        card["body"][2]["text"] = priceAlert->body;

        return {{"type", "message"}, {"attachments", {{{"contentType", "application/vnd.microsoft.card.adaptive"}, {"content", card}}}};
    }
    else if (auto statsReport = dynamic_cast<const StatisticsReportMessage*>(&message)) {
        nlohmann::json card = nlohmann::json::parse(STATISTICS_REPORT_TEMPLATE);
        card["body"][0]["text"] = statsReport->title;
        card["body"][1]["facts"][0]["value"] = statsReport->reportPeriod;
        card["body"][1]["facts"][1]["value"] = statsReport->statistics.fuelType;
        card["body"][1]["facts"][2]["value"] = fmt::format("{:.3f} €", statsReport->statistics.areaAveragePrice);

        // Find the cheapest day and time
        std::string bestDay;
        std::string bestTime;
        double lowestPrice = std::numeric_limits<double>::max();

        for (const auto& station : statsReport->statistics.stationStats) {
            if (station.weeklyStats.cheapestDay.empty()) continue;
            
            const auto& dailyStats = station.weeklyStats.dailyStats.at(station.weeklyStats.cheapestDay);
            if (dailyStats.minPrice < lowestPrice) {
                lowestPrice = dailyStats.minPrice;
                bestDay = station.weeklyStats.cheapestDay;
                bestTime = dailyStats.bestTimeToRefuel;
            }
        }

        card["body"][1]["facts"][3]["value"] = bestDay;
        card["body"][1]["facts"][4]["value"] = bestTime;
        card["body"][2]["text"] = statsReport->body;

        return {{"type", "message"}, {"attachments", {{{"contentType", "application/vnd.microsoft.card.adaptive"}, {"content", card}}}};
    }

    throw std::runtime_error("Unknown message type");
}

bool TeamsNotificationService::sendWebhookRequest(const nlohmann::json& payload) {
    std::string payloadStr = payload.dump();
    
    curl_easy_setopt(curl, CURLOPT_URL, webhookUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    return res == CURLE_OK;
}

size_t TeamsNotificationService::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

} // namespace notifications 
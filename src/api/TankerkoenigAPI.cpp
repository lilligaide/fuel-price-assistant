#include "api/TankerkoenigAPI.hpp"
#include <sstream>
#include <stdexcept>
#include <fmt/format.h>

namespace api {

TankerkoenigAPI::TankerkoenigAPI(const std::string& apiKey) : apiKey(apiKey) {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

TankerkoenigAPI::~TankerkoenigAPI() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

std::vector<models::FuelStation> TankerkoenigAPI::findStations(
    double lat,
    double lng,
    double radius,
    const std::string& fuelType
) {
    std::map<std::string, std::string> params = {
        {"lat", std::to_string(lat)},
        {"lng", std::to_string(lng)},
        {"rad", std::to_string(radius)},
        {"type", fuelType},
        {"apikey", apiKey}
    };

    auto response = makeRequest("list.php", params);
    
    if (!response["ok"].get<bool>()) {
        throw std::runtime_error(response["message"].get<std::string>());
    }

    std::vector<models::FuelStation> stations;
    for (const auto& item : response["stations"]) {
        models::FuelStation station;
        station.id = item["id"].get<std::string>();
        station.name = item["name"].get<std::string>();
        station.brand = item["brand"].get<std::string>();
        
        station.location.latitude = item["lat"].get<double>();
        station.location.longitude = item["lng"].get<double>();
        station.location.street = item["street"].get<std::string>();
        station.location.houseNumber = item["houseNumber"].get<std::string>();
        station.location.postalCode = item["postCode"].get<std::string>();
        station.location.city = item["place"].get<std::string>();
        
        station.isOpen = item["isOpen"].get<bool>();
        station.distance = item["dist"].get<double>();

        // Add prices if available
        if (!item["e5"].is_null()) {
            station.prices.push_back({
                .fuelType = "e5",
                .price = item["e5"].get<double>(),
                .lastUpdate = item["lastChange"].get<std::string>()
            });
        }
        if (!item["e10"].is_null()) {
            station.prices.push_back({
                .fuelType = "e10",
                .price = item["e10"].get<double>(),
                .lastUpdate = item["lastChange"].get<std::string>()
            });
        }
        if (!item["diesel"].is_null()) {
            station.prices.push_back({
                .fuelType = "diesel",
                .price = item["diesel"].get<double>(),
                .lastUpdate = item["lastChange"].get<std::string>()
            });
        }

        stations.push_back(station);
    }

    return stations;
}

std::optional<models::FuelStation> TankerkoenigAPI::getStationDetails(const std::string& stationId) {
    std::map<std::string, std::string> params = {
        {"id", stationId},
        {"apikey", apiKey}
    };

    auto response = makeRequest("detail.php", params);
    
    if (!response["ok"].get<bool>()) {
        return std::nullopt;
    }

    const auto& item = response["station"];
    models::FuelStation station;
    station.id = item["id"].get<std::string>();
    station.name = item["name"].get<std::string>();
    station.brand = item["brand"].get<std::string>();
    
    station.location.latitude = item["lat"].get<double>();
    station.location.longitude = item["lng"].get<double>();
    station.location.street = item["street"].get<std::string>();
    station.location.houseNumber = item["houseNumber"].get<std::string>();
    station.location.postalCode = item["postCode"].get<std::string>();
    station.location.city = item["place"].get<std::string>();
    
    station.isOpen = item["isOpen"].get<bool>();

    // Add prices if available
    if (!item["e5"].is_null()) {
        station.prices.push_back({
            .fuelType = "e5",
            .price = item["e5"].get<double>(),
            .lastUpdate = item["lastChange"].get<std::string>()
        });
    }
    if (!item["e10"].is_null()) {
        station.prices.push_back({
            .fuelType = "e10",
            .price = item["e10"].get<double>(),
            .lastUpdate = item["lastChange"].get<std::string>()
        });
    }
    if (!item["diesel"].is_null()) {
        station.prices.push_back({
            .fuelType = "diesel",
            .price = item["diesel"].get<double>(),
            .lastUpdate = item["lastChange"].get<std::string>()
        });
    }

    return station;
}

std::vector<models::FuelStation> TankerkoenigAPI::getPrices(const std::vector<std::string>& stationIds) {
    if (stationIds.empty()) {
        return {};
    }

    std::stringstream ss;
    for (size_t i = 0; i < stationIds.size(); ++i) {
        if (i > 0) ss << ",";
        ss << stationIds[i];
    }

    std::map<std::string, std::string> params = {
        {"ids", ss.str()},
        {"apikey", apiKey}
    };

    auto response = makeRequest("prices.php", params);
    
    if (!response["ok"].get<bool>()) {
        return {};
    }

    std::vector<models::FuelStation> stations;
    for (const auto& [id, item] : response["prices"].items()) {
        if (!item["status"].get<std::string>().empty()) {
            models::FuelStation station;
            station.id = id;
            station.isOpen = item["status"] == "open";
            
            if (!item["e5"].is_null()) {
                station.prices.push_back({
                    .fuelType = "e5",
                    .price = item["e5"].get<double>(),
                    .lastUpdate = item["lastChange"].get<std::string>()
                });
            }
            if (!item["e10"].is_null()) {
                station.prices.push_back({
                    .fuelType = "e10",
                    .price = item["e10"].get<double>(),
                    .lastUpdate = item["lastChange"].get<std::string>()
                });
            }
            if (!item["diesel"].is_null()) {
                station.prices.push_back({
                    .fuelType = "diesel",
                    .price = item["diesel"].get<double>(),
                    .lastUpdate = item["lastChange"].get<std::string>()
                });
            }

            stations.push_back(station);
        }
    }

    return stations;
}

size_t TankerkoenigAPI::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

nlohmann::json TankerkoenigAPI::makeRequest(
    const std::string& endpoint,
    const std::map<std::string, std::string>& params
) {
    std::string url = BASE_URL + endpoint + "?";
    bool first = true;
    for (const auto& [key, value] : params) {
        if (!first) url += "&";
        url += key + "=" + value;
        first = false;
    }

    std::string response_string;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error(fmt::format("CURL request failed: {}", curl_easy_strerror(res)));
    }

    return nlohmann::json::parse(response_string);
}

} // namespace api 
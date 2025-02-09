#include "Server.hpp"
#include "../utils/Logger.hpp"
#include <nlohmann/json.hpp>
#include <fmt/core.h>

using json = nlohmann::json;

Server::Server(const Config& config) : config(config) {
    stationService = std::make_unique<StationService>(config);
    geocodingService = std::make_unique<GeocodingService>(config);
    setupMiddleware();
    setupRoutes();
    setupStaticFiles();
}

void Server::setupMiddleware() {
    // Setup CORS
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
        .global()
        .headers("Content-Type")
        .methods("GET"_method, "POST"_method, "OPTIONS"_method);
}

void Server::setupRoutes() {
    CROW_ROUTE(app, "/api/stations/search")
        .methods("GET"_method)
        ([this](const crow::request& req) {
            return handleStationSearch(req);
        });

    CROW_ROUTE(app, "/api/geocode")
        .methods("GET"_method)
        ([this](const crow::request& req) {
            return handleGeocoding(req);
        });

    CROW_ROUTE(app, "/api/reverse-geocode")
        .methods("GET"_method)
        ([this](const crow::request& req) {
            return handleReverseGeocoding(req);
        });

    CROW_ROUTE(app, "/")
        ([this](const crow::request& req) {
            return serveMapPage(req);
        });
}

void Server::setupStaticFiles() {
    CROW_ROUTE(app, "/static/<path>")
        ([](const std::string& path) {
            return crow::response(200, path);
        });
}

crow::response Server::handleStationSearch(const crow::request& req) {
    try {
        auto lat = std::stod(req.url_params.get("lat"));
        auto lon = std::stod(req.url_params.get("lon"));
        auto radius = std::stoi(req.url_params.get("radius", "5000"));

        auto stations = stationService->findNearbyStations(lat, lon, radius);
        return crow::response(200, json(stations).dump());
    } catch (const std::exception& e) {
        Logger::error("Station search error: {}", e.what());
        return crow::response(400, fmt::format("{{\"error\": \"{}\"}}", e.what()));
    }
}

crow::response Server::handleGeocoding(const crow::request& req) {
    try {
        auto address = req.url_params.get("address");
        if (!address) {
            return crow::response(400, "{\"error\": \"Address parameter required\"}");
        }

        auto location = geocodingService->geocodeAddress(*address);
        if (!location) {
            return crow::response(404, "{\"error\": \"Location not found\"}");
        }

        return crow::response(200, json(*location).dump());
    } catch (const std::exception& e) {
        Logger::error("Geocoding error: {}", e.what());
        return crow::response(400, fmt::format("{{\"error\": \"{}\"}}", e.what()));
    }
}

crow::response Server::handleReverseGeocoding(const crow::request& req) {
    try {
        auto lat = std::stod(req.url_params.get("lat"));
        auto lon = std::stod(req.url_params.get("lon"));

        auto address = geocodingService->reverseGeocode(lat, lon);
        return crow::response(200, fmt::format("{{\"address\": \"{}\"}}", address));
    } catch (const std::exception& e) {
        Logger::error("Reverse geocoding error: {}", e.what());
        return crow::response(400, fmt::format("{{\"error\": \"{}\"}}", e.what()));
    }
}

crow::response Server::serveMapPage(const crow::request& req) {
    return crow::response("map.html");
}

void Server::start() {
    Logger::info("Starting server on port {}", config.port);
    app.port(config.port).multithreaded().run();
}

void Server::stop() {
    Logger::info("Stopping server...");
    app.stop();
} 
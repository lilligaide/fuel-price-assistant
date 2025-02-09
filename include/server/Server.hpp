#pragma once

#include <crow.h>
#include <memory>
#include <string>
#include "../utils/Config.hpp"
#include "../services/StationService.hpp"
#include "../services/GeocodingService.hpp"

class Server {
public:
    explicit Server(const Config& config);
    void start();
    void stop();

private:
    void setupRoutes();
    void setupMiddleware();
    void setupStaticFiles();
    
    crow::App<crow::CORSHandler> app;
    std::unique_ptr<StationService> stationService;
    std::unique_ptr<GeocodingService> geocodingService;
    Config config;
    
    // Route handlers
    crow::response handleStationSearch(const crow::request& req);
    crow::response handleGeocoding(const crow::request& req);
    crow::response handleReverseGeocoding(const crow::request& req);
    crow::response serveMapPage(const crow::request& req);
}; 
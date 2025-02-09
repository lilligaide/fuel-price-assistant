#pragma once

#include <crow.h>
#include "../services/StationService.hpp"
#include "../services/GeocodingService.hpp"

namespace Routes {
    void setupRoutes(
        crow::App<crow::CORSHandler>& app,
        StationService& stationService,
        GeocodingService& geocodingService
    );
    
    crow::response handleStationSearch(
        const crow::request& req,
        StationService& service
    );
    
    crow::response handleGeocoding(
        const crow::request& req,
        GeocodingService& service
    );
    
    crow::response handleReverseGeocoding(
        const crow::request& req,
        GeocodingService& service
    );
    
    crow::response serveMapPage(const crow::request& req);
} 
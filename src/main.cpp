#include "server/Server.hpp"
#include "utils/Config.hpp"
#include "utils/Logger.hpp"
#include <fmt/core.h>
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // Initialize logger
        Logger::init();
        Logger::info("Starting EV Charging Finder application...");

        // Load configuration
        auto config = Config::load();
        
        // Initialize and start server
        Server server(config);
        server.start();

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        Logger::error("Fatal error: {}", e.what());
        return EXIT_FAILURE;
    }
} 
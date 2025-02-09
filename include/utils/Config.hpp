#pragma once

#include <string>
#include <nlohmann/json.hpp>

struct APIConfig {
    std::string endpoint = "https://api.openchargemap.io/v3";
    std::string key = "7c4dfce6-966f-4239-b1e2-a95c86e0dbe3";  // Replace in production
    int timeout_ms = 5000;
    bool verify_ssl = true;
};

struct Config {
    static Config load(const std::string& configPath = "config.json");
    
    int port = 8080;
    std::string host = "0.0.0.0";
    APIConfig api;
    std::string db_path = "stations.db";
    int cache_duration_seconds = 3600;
    
    // SSL configuration
    bool use_ssl = false;
    std::string ssl_cert;
    std::string ssl_key;
    
    // Rate limiting
    int rate_limit_requests = 100;
    int rate_limit_window_seconds = 60;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, port, host, db_path, 
                                  cache_duration_seconds, use_ssl, 
                                  ssl_cert, ssl_key, rate_limit_requests, 
                                  rate_limit_window_seconds)
}; 
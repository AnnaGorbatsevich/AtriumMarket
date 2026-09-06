#include "config.hpp"

#include <cstdlib>
#include <stdexcept>

namespace gateway {

const std::string& UserServiceUrl() {
    static const std::string url = [] {
        const char* value = std::getenv("USER_SERVICE_URL");
        if (value == nullptr || value[0] == '\0') {
            throw std::runtime_error("USER_SERVICE_URL environment variable is not set");
        }
        return std::string{value};
    }();
    return url;
}

const std::string& ListingServiceUrl() {
    static const std::string url = [] {
        const char* value = std::getenv("LISTING_SERVICE_URL");
        if (value == nullptr || value[0] == '\0') {
            throw std::runtime_error("LISTING_SERVICE_URL environment variable is not set");
        }
        return std::string{value};
    }();
    return url;
}

}  // namespace gateway

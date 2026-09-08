#pragma once

#include <string>
#include <userver/clients/http/client.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace gateway {
struct HttpRequest {
public:
    HttpRequest(const userver::components::ComponentContext& context);
    std::shared_ptr<userver::v3_2_rc::clients::http::Response> Get(
         std::string url, std::string handle,
        const std::initializer_list<std::pair<userver::utils::zstring_view, userver::utils::zstring_view>>& headers,
        int timeout) const;
    std::shared_ptr<userver::v3_2_rc::clients::http::Response> Post(
        std::string url, std::string handle,
        const std::initializer_list<std::pair<userver::utils::zstring_view, userver::utils::zstring_view>>& headers,
        int timeout, std::string request
    ) const;
private:
    userver::clients::http::Client& http_client_;
};

} // namespace gateway
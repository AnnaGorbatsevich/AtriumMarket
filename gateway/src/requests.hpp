#pragma once

#include <string>
#include <userver/clients/http/client.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace gateway {
struct HttpRequest {
public:
    HttpRequest(const userver::components::ComponentContext& context);
    std::shared_ptr<userver::clients::http::Response> Get(
         std::string url, std::string handle,
        const std::initializer_list<std::pair<userver::utils::zstring_view, userver::utils::zstring_view>>& headers,
        int timeout) const;
    std::shared_ptr<userver::clients::http::Response> Post(
        std::string url, std::string handle,
        const std::initializer_list<std::pair<userver::utils::zstring_view, userver::utils::zstring_view>>& headers,
        int timeout, std::string request
    ) const;
    std::shared_ptr<userver::clients::http::Response> GetMe(const userver::server::http::HttpRequest& request) const;
    std::shared_ptr<userver::clients::http::Response> GetBasket(int64_t buyer_id) const;
    int64_t GetAvailability(int64_t variant_id) const;
    std::shared_ptr<userver::clients::http::Response> GetVariant(int64_t variant_id) const;
    std::shared_ptr<userver::clients::http::Response> DecreaseAvailability(int64_t variant_id, int64_t quantity) const;
    std::shared_ptr<userver::clients::http::Response> ResetBasketQuantity(int64_t buyer_id, int64_t variant_id, int64_t max_quantity) const;
private:
    userver::clients::http::Client& http_client_;
};

} // namespace gateway
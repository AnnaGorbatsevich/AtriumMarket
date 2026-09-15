#pragma once

#include <userver/clients/http/client.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace gateway {

struct GetOrdersProxyHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-orders";

    GetOrdersProxyHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context
    ) const override;

private:
    userver::clients::http::Client& http_client_;
};

}  // namespace gateway

#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include "db.hpp"
#include "events.hpp"

namespace order_service {

struct UpdateOrderStatusHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-update-order-status";

    UpdateOrderStatusHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    OrderDAO db_dao_;
    OrderEventPublisher event_publisher_;
};

}  // namespace order_service

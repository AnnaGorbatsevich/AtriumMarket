#pragma once

#include <memory>

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/clickhouse/fwd.hpp>

namespace stats_service {

struct SellerStatsHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-seller-stats";

    SellerStatsHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    std::shared_ptr<userver::storages::clickhouse::Cluster> clickhouse_;
};

}  // namespace stats_service

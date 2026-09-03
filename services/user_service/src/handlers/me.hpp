#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace user_service {

struct MeHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-me";

    MeHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace user_service

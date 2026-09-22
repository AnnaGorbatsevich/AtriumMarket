#include "seller_stats.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <userver/components/component_context.hpp>
#include <userver/formats/common/type.hpp>
#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/storages/clickhouse/cluster.hpp>
#include <userver/storages/clickhouse/component.hpp>
#include <userver/storages/clickhouse/io/columns/common_columns.hpp>
#include <userver/storages/clickhouse/query.hpp>

namespace stats_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

// order_events has one row per status transition, so a plain count-by-status would count an
// order once for every status it ever passed through. Each order is folded down to its latest
// status first (argMax by event_time), then counted/summed once per order.
struct SellerStatusStat {
    std::string status;
    std::int64_t orders;
    std::int64_t revenue;
};

}  // namespace

}  // namespace stats_service

template <>
struct userver::storages::clickhouse::io::CppToClickhouse<stats_service::SellerStatusStat> {
    using mapped_type = std::tuple<
        userver::storages::clickhouse::io::columns::StringColumn,
        userver::storages::clickhouse::io::columns::Int64Column,
        userver::storages::clickhouse::io::columns::Int64Column>;
};

namespace stats_service {

namespace {

const userver::storages::clickhouse::Query kSellerStatsQuery{
    R"~(
    SELECT status, toInt64(count()) AS orders, toInt64(sum(total)) AS revenue
    FROM (
        SELECT
            order_id,
            argMax(status, event_time) AS status,
            argMax(quantity * price, event_time) AS total
        FROM order_events
        WHERE seller_id = {}
        GROUP BY order_id
    )
    GROUP BY status
    ORDER BY status
    )~"
};

}  // namespace

SellerStatsHandler::SellerStatsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      clickhouse_(context.FindComponent<userver::components::ClickHouse>("clickhouse-database").GetCluster()) {}

std::string SellerStatsHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    if (!request.HasArg("sellerId")) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Missing required query parameter: sellerId"}
        );
    }

    std::int64_t seller_id = 0;
    try {
        seller_id = std::stoll(request.GetArg("sellerId"));
    } catch (const std::exception&) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"sellerId must be an integer"}
        );
    }

    auto rows = clickhouse_->Execute(kSellerStatsQuery, seller_id).AsContainer<std::vector<SellerStatusStat>>();

    userver::formats::json::ValueBuilder response_body{userver::formats::common::Type::kArray};
    for (const auto& row : rows) {
        userver::formats::json::ValueBuilder stat;
        stat["status"] = row.status;
        stat["orders"] = row.orders;
        stat["revenue"] = row.revenue;
        response_body.PushBack(std::move(stat));
    }

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace stats_service

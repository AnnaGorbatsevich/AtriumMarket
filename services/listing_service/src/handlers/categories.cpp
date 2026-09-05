#include "categories.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/common/type.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/http/content_type.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/optional.hpp>

namespace listing_service {

namespace {

void SetCorsHeaders(userver::server::http::HttpResponse& response) {
    response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
    response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, OPTIONS"});
    response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
}

constexpr std::string_view kSelectCategoriesQuery = R"~(
SELECT id, name, parent_id FROM categories ORDER BY name
)~";

}  // namespace

CategoriesHandler::CategoriesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

std::string CategoriesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto& http_response = request.GetHttpResponse();
    SetCorsHeaders(http_response);

    if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
        return {};
    }

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Query{std::string{kSelectCategoriesQuery}}
    );

    userver::formats::json::ValueBuilder response_body{userver::formats::common::Type::kArray};
    for (const auto& row : result) {
        userver::formats::json::ValueBuilder category;
        category["id"] = row["id"].As<std::int64_t>();
        category["name"] = row["name"].As<std::string>();
        category["parentId"] = row["parent_id"].As<std::optional<std::int64_t>>();
        response_body.PushBack(std::move(category));
    }

    http_response.SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(response_body.ExtractValue());
}

}  // namespace listing_service

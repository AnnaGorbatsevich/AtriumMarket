#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "db.hpp"

namespace listing_service {

struct VariantHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-variant";

    VariantHandler(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override;

private:
    ListingDAO db_dao_;
};

}  // namespace listing_service

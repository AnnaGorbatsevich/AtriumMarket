#pragma once

#include <userver/server/handlers/http_handler_base.hpp>

namespace user_service {

struct HelloHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-hello";

    using HttpHandlerBase::HttpHandlerBase;

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override {
        request.GetHttpResponse().SetContentType(userver::http::content_type::kTextPlain);
        return "Hello world!\n";
    }

};

}  // namespace user_service

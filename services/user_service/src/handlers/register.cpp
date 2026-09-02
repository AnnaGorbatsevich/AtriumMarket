#include "register.hpp"
#include "security.hpp"

#include <atomic>

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/http/content_type.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>

namespace {
    void SetCorsHeaders(userver::server::http::HttpResponse& response) {
        response.SetHeader(std::string{"Access-Control-Allow-Origin"}, std::string{"*"});
        response.SetHeader(std::string{"Access-Control-Allow-Methods"}, std::string{"GET, POST, OPTIONS"});
        response.SetHeader(std::string{"Access-Control-Allow-Headers"}, std::string{"Content-Type"});
    }
}

namespace user_service {

    std::string RegisterHandler::HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const {
        auto& http_response = request.GetHttpResponse();
        //SetCorsHeaders(http_response);

        if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
            return {};
        }

        userver::formats::json::Value payload;
        try {
            payload = userver::formats::json::FromString(request.RequestBody());
        } catch (const userver::formats::json::Exception&) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{"Invalid JSON body"}
            );
        }

        auto jwt_token = create_jwt_token(payload["email"].As<std::string>());



        request.GetHttpResponse().SetContentType(userver::http::content_type::kTextPlain);
        return "Hello world!\n" + jwt_token + " " + decode_jwt_token(jwt_token);
    }
}
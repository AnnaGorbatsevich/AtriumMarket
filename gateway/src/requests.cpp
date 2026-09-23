#include "requests.hpp"
#include "config.hpp"

#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/components/component_context.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/clients/http/error.hpp>

namespace gateway {
    HttpRequest::HttpRequest(const userver::components::ComponentContext& context) :
        http_client_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()) {}

    std::shared_ptr<userver::clients::http::Response> HttpRequest::Get(
        std::string url, std::string handle,
        const std::initializer_list<std::pair<userver::utils::zstring_view, userver::utils::zstring_view>>& headers,
        int timeout) const {
        auto response = http_client_.CreateRequest()
            .get(url + handle)
            .headers(headers)
            .timeout(std::chrono::milliseconds(timeout))
            .perform();
        return response;
    }
    std::shared_ptr<userver::clients::http::Response> HttpRequest::Post(std::string url, std::string handle,
        const std::initializer_list<std::pair<userver::utils::zstring_view, userver::utils::zstring_view>>& headers,
        int timeout, std::string request) const {
        auto response = http_client_.CreateRequest()
            .post(
                url + handle,
                request
            )
            .headers(headers)
            .timeout(std::chrono::milliseconds(timeout))
            .perform();
        return response;
    }

    std::shared_ptr<userver::clients::http::Response> HttpRequest::GetMe(const userver::server::http::HttpRequest& request) const {
        return Get(UserServiceUrl(), "/me", {{"Authorization", request.GetHeader("Authorization")}}, 2000);
    }

    std::shared_ptr<userver::clients::http::Response> HttpRequest::GetBasket(int64_t buyer_id) const {
        return Get(OrderServiceUrl(), "/get_basket?sellerId=" + std::to_string(buyer_id), {}, 2000);
    }

    int64_t HttpRequest::GetAvailability(int64_t variant_id) const{
        auto result = GetVariant(variant_id);
        userver::formats::json::Value variant = userver::formats::json::FromString(result->body());
        return variant["quantity"].As<std::int64_t>();
    }

    std::shared_ptr<userver::clients::http::Response> HttpRequest::GetVariant(int64_t variant_id) const{
        return Get(
            ListingServiceUrl(),
            "/variant?variantId=" + std::to_string(variant_id),
            {},
            2000
        );
    }

    std::shared_ptr<userver::clients::http::Response> HttpRequest::DecreaseAvailability(int64_t variant_id, int64_t quantity_to_be_purchased) const {
        userver::formats::json::ValueBuilder forwarded_payload;
        forwarded_payload["variantId"] = variant_id;
        forwarded_payload["quantity"] = quantity_to_be_purchased;

        return Post(
            ListingServiceUrl(),
            "/decrease_availability",
            {{"Content-Type", "application/json"}},
            2000,
            userver::formats::json::ToString(forwarded_payload.ExtractValue())
        );
    }

} // namespace gateway
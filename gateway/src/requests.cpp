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

    std::shared_ptr<userver::v3_2_rc::clients::http::Response> HttpRequest::Get(
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
    std::shared_ptr<userver::v3_2_rc::clients::http::Response> HttpRequest::Post(std::string url, std::string handle,
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

    std::shared_ptr<userver::v3_2_rc::clients::http::Response> HttpRequest::GetMe(const userver::server::http::HttpRequest& request) const {
        return Get(UserServiceUrl(), "/me", {{"Authorization", request.GetHeader("Authorization")}}, 2000);
    }

} // namespace gateway
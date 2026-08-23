#include "register_proxy.hpp"

#include <userver/http/content_type.hpp>

namespace gateway {


std::string RegisterProxyHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kTextPlain);
    return "Hello world!\n";
}


}  // namespace gateway

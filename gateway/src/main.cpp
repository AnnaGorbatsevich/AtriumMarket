
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/clients/http/component_core.hpp>
#include <userver/clients/http/middlewares/pipeline_component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/register_proxy.hpp"
#include "handlers/hello.hpp"
#include "handlers/login_proxy.hpp"
#include "handlers/me_proxy.hpp"
#include "handlers/add_product_proxy.hpp"
#include "handlers/categories_proxy.hpp"
#include "handlers/products_proxy.hpp"
#include "handlers/catalog_proxy.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                               .Append<gateway::RegisterProxyHandler>()
                               .Append<gateway::HelloHandler>()
                               .Append<gateway::LoginProxyHandler>()
                               .Append<gateway::MeProxyHandler>()
                               .Append<gateway::AddProductProxyHandler>()
                               .Append<gateway::CategoriesProxyHandler>()
                               .Append<gateway::ProductsProxyHandler>()
                               .Append<gateway::CatalogProxyHandler>()
                               .Append<userver::components::HttpClientCore>()
                               .Append<userver::clients::http::MiddlewarePipelineComponent>()
                               .Append<userver::components::HttpClient>()
                               .Append<userver::clients::dns::Component>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}

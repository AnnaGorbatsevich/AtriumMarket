
#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/add_product.hpp"
#include "handlers/catalog.hpp"
#include "handlers/categories.hpp"
#include "handlers/hello.hpp"
#include "handlers/products.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                               .Append<listing_service::HelloHandler>()
                               .Append<listing_service::AddProductHandler>()
                               .Append<listing_service::CategoriesHandler>()
                               .Append<listing_service::ProductsHandler>()
                               .Append<listing_service::CatalogHandler>()
                               .Append<userver::components::Postgres>("postgres-db")
                               .Append<userver::components::TestsuiteSupport>()
                               .Append<userver::clients::dns::Component>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}

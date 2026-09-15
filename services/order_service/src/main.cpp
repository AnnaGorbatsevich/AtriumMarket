
#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/hello.hpp"
#include "handlers/add_basket.hpp"
#include "handlers/get_basket.hpp"
#include "handlers/get_orders.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                               .Append<order_service::HelloHandler>()
                               .Append<order_service::AddBasketHandler>()
                               .Append<order_service::GetBasketHandler>()
                               .Append<order_service::GetOrderHandler>()
                               .Append<userver::components::TestsuiteSupport>()
                               .Append<userver::components::Postgres>("postgres-db")
                               .Append<userver::clients::dns::Component>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}

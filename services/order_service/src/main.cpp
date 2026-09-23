
#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/kafka/producer_component.hpp>
#include <userver/server/handlers/server_monitor.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/hello.hpp"
#include "handlers/add_basket.hpp"
#include "handlers/get_basket.hpp"
#include "handlers/get_orders.hpp"
#include "handlers/update_basket.hpp"
#include "handlers/checkout.hpp"
#include "handlers/update_order_status.hpp"
#include "handlers/reset_basket.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                               .Append<order_service::HelloHandler>()
                               .Append<order_service::AddBasketHandler>()
                               .Append<order_service::GetBasketHandler>()
                               .Append<order_service::GetOrderHandler>()
                               .Append<order_service::UpdateBasketHandler>()
                               .Append<order_service::CheckoutHandler>()
                               .Append<order_service::UpdateOrderStatusHandler>()
                               .Append<order_service::ResetBasketHandler>()
                               .Append<userver::server::handlers::ServerMonitor>()
                               .Append<userver::components::TestsuiteSupport>()
                               .Append<userver::components::Postgres>("postgres-db")
                               .Append<userver::components::Secdist>()
                               .Append<userver::components::DefaultSecdistProvider>()
                               .Append<userver::kafka::ProducerComponent>("kafka-producer")
                               .Append<userver::clients::dns::Component>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}

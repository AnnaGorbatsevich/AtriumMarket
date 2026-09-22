#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/kafka/consumer_component.hpp>
#include <userver/storages/clickhouse/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/hello.hpp"
#include "handlers/seller_stats.hpp"
#include "order_event_consumer.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                               .Append<stats_service::HelloHandler>()
                               .Append<stats_service::SellerStatsHandler>()
                               .Append<userver::components::Secdist>()
                               .Append<userver::components::DefaultSecdistProvider>()
                               .Append<userver::components::ClickHouse>("clickhouse-database")
                               .Append<userver::kafka::ConsumerComponent>("kafka-consumer")
                               .Append<stats_service::OrderEventConsumer>()
                               .Append<userver::components::TestsuiteSupport>()
                               .Append<userver::clients::dns::Component>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}

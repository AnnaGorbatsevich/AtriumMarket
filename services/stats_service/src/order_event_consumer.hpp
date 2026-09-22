#pragma once

#include <memory>
#include <string_view>

#include <userver/components/component_base.hpp>
#include <userver/kafka/consumer_scope.hpp>
#include <userver/kafka/message.hpp>
#include <userver/storages/clickhouse/fwd.hpp>

namespace stats_service {

class OrderEventConsumer final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "order-event-consumer";

    OrderEventConsumer(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

private:
    void Consume(userver::kafka::MessageBatchView messages);

    std::shared_ptr<userver::storages::clickhouse::Cluster> clickhouse_;

    userver::kafka::ConsumerScope consumer_;
};

}  // namespace stats_service

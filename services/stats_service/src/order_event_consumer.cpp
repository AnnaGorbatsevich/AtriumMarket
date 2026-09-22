#include "order_event_consumer.hpp"

#include <chrono>
#include <cstdint>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/kafka/consumer_component.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/clickhouse/cluster.hpp>
#include <userver/storages/clickhouse/component.hpp>
#include <userver/storages/clickhouse/query.hpp>

namespace stats_service {

OrderEventConsumer::OrderEventConsumer(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : ComponentBase(config, context),
      clickhouse_(context.FindComponent<userver::components::ClickHouse>("clickhouse-database").GetCluster()),
      consumer_(context.FindComponent<userver::kafka::ConsumerComponent>("kafka-consumer").GetConsumer()) {
    consumer_.Start([this](userver::kafka::MessageBatchView messages) {
        Consume(messages);
        consumer_.AsyncCommit();
    });
}

void OrderEventConsumer::Consume(userver::kafka::MessageBatchView messages) {
    const auto now = std::chrono::system_clock::now();
    const userver::storages::clickhouse::Query kInsertQuery{
        "INSERT INTO order_events (order_id, buyer_id, seller_id, variant_id, quantity, price, status, event_time) "
        "VALUES ({}, {}, {}, {}, {}, {}, {}, {})"
    };
    for (const auto& message : messages) {
        try {
            const auto payload = userver::formats::json::FromString(message.GetPayload());
            clickhouse_->Execute(
                kInsertQuery,
                payload["orderId"].As<std::int64_t>(),
                payload["buyerId"].As<std::int64_t>(),
                payload["sellerId"].As<std::int64_t>(),
                payload["variantId"].As<std::int64_t>(),
                payload["quantity"].As<std::int64_t>(),
                payload["price"].As<std::int64_t>(),
                payload["status"].As<std::string>(),
                now
            );
        } catch (const std::exception& e) {
            LOG_WARNING() << message.GetOffset() << " : " << e.what();
        }
    }
}

}  // namespace stats_service

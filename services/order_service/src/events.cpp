#include "events.hpp"

#include <userver/formats/json.hpp>
#include <userver/kafka/producer_component.hpp>
#include <userver/logging/log.hpp>

namespace order_service {

OrderEventPublisher::OrderEventPublisher(const userver::components::ComponentContext& context)
    : producer_(context.FindComponent<userver::kafka::ProducerComponent>("kafka-producer").GetProducer()) {}

void OrderEventPublisher::Publish(const OrderEvent& event) const {
    userver::formats::json::ValueBuilder message;
    message["orderId"] = event.order_id;
    message["buyerId"] = event.buyer_id;
    message["sellerId"] = event.seller_id;
    message["variantId"] = event.variant_id;
    message["quantity"] = event.quantity;
    message["price"] = event.price;
    message["status"] = event.status;

    try {
        producer_.Send(
            kOrderEventsTopic,
            std::to_string(event.order_id),
            userver::formats::json::ToString(message.ExtractValue())
        );
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to publish order event (orderId=" << e.what();
    }
}

}  // namespace order_service

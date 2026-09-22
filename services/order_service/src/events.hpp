#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <userver/components/component_context.hpp>
#include <userver/kafka/producer.hpp>

namespace order_service {

inline constexpr const char* kOrderEventsTopic = "order-events";

struct OrderEvent {
    std::int64_t order_id;
    std::int64_t buyer_id;
    std::int64_t seller_id;
    std::int64_t variant_id;
    std::int64_t quantity;
    std::int64_t price;
    std::string status;
};

class OrderEventPublisher {
public:
    explicit OrderEventPublisher(const userver::components::ComponentContext& context);

    void Publish(const OrderEvent& event) const;

private:
    const userver::kafka::Producer& producer_;
};

}  // namespace order_service

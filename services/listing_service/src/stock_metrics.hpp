#pragma once

#include <userver/components/component_base.hpp>
#include <userver/utils/statistics/rate_counter.hpp>

namespace listing_service {

class StockMetrics final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "stock-metrics";

    StockMetrics(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    void AccountDecrease(bool ok);
    void AccountRestock(bool ok);

private:
    userver::utils::statistics::RateCounter decrease_ok_;
    userver::utils::statistics::RateCounter decrease_rejected_;
    userver::utils::statistics::RateCounter restock_ok_;
    userver::utils::statistics::RateCounter restock_rejected_;
};

}  // namespace listing_service

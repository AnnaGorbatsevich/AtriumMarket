#include "stock_metrics.hpp"

#include <userver/components/statistics_storage.hpp>
#include <userver/utils/statistics/writer.hpp>

namespace listing_service {

StockMetrics::StockMetrics(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : ComponentBase(config, context) {
    userver::utils::statistics::RegisterWriterScope(
        context,
        "stock_adjustments",
        [this](userver::utils::statistics::Writer& writer) {
            writer.ValueWithLabels(decrease_ok_, {{"direction", "decrease"}, {"result", "ok"}});
            writer.ValueWithLabels(decrease_rejected_, {{"direction", "decrease"}, {"result", "rejected"}});
            writer.ValueWithLabels(restock_ok_, {{"direction", "restock"}, {"result", "ok"}});
            writer.ValueWithLabels(restock_rejected_, {{"direction", "restock"}, {"result", "rejected"}});
        }
    );
}

void StockMetrics::AccountDecrease(bool ok) { (ok ? decrease_ok_ : decrease_rejected_).Increment(); }

void StockMetrics::AccountRestock(bool ok) { (ok ? restock_ok_ : restock_rejected_).Increment(); }

}  // namespace listing_service

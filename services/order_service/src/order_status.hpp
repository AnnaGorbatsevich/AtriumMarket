#pragma once

#include <string_view>

namespace order_service {

inline bool IsOrderStatus(std::string_view status) {
    return status == "ordered" || status == "accepted" || status == "shipped" || status == "ready_for_pickup" ||
           status == "received" || status == "cancelled";
}

inline bool IsTransitionAllowed(std::string_view role, std::string_view from, std::string_view to) {
    if (role == "seller") {
        if (from == "ordered") return to == "accepted" || to == "cancelled";
        if (from == "accepted") return to == "shipped";
        if (from == "shipped") return to == "ready_for_pickup";
        return false;
    }
    if (role == "buyer") {
        return from == "ready_for_pickup" && (to == "received" || to == "cancelled");
    }
    return false;
}

}  // namespace order_service

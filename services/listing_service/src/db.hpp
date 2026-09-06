#pragma once
#include <userver/storages/postgres/cluster.hpp>
#include <vector>


namespace listing_service {
struct ListingDAO {
public:
    ListingDAO(const userver::components::ComponentContext& context);
    userver::v3_2_rc::storages::postgres::ResultSet GetProducts(int seller_id) const;
    userver::v3_2_rc::storages::postgres::ResultSet GetCategories() const;
    userver::v3_2_rc::storages::postgres::ResultSet GetCatalog(std::optional<long int> category_id) const;
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};
} // namespace listing_service 
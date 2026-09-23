#pragma once
#include <userver/storages/postgres/cluster.hpp>
#include <vector>


namespace listing_service {
struct ListingDAO {
public:
    ListingDAO(const userver::components::ComponentContext& context);
    userver::storages::postgres::ResultSet GetProducts(int seller_id) const;
    userver::storages::postgres::ResultSet GetCategories() const;
    userver::storages::postgres::ResultSet GetCatalog(std::optional<long int> category_id) const;
    userver::storages::postgres::ResultSet GetVariant(std::int64_t variant_id) const;
    void InsertProduct(userver::formats::json::Value payload) const;
    userver::storages::postgres::ResultSet DecreaseAvailability(std::int64_t variant_id, std::int64_t quantity) const;
    //void InsertVariants();
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};
} // namespace listing_service 
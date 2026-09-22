CREATE TABLE IF NOT EXISTS order_events
(
    order_id   Int64,
    buyer_id   Int64,
    seller_id  Int64,
    variant_id Int64,
    quantity   Int64,
    price      Int64,
    status     LowCardinality(String),
    event_time DateTime64(3, 'UTC')
)
ENGINE = MergeTree()
ORDER BY (status, event_time);

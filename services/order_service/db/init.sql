CREATE TABLE orders (
    id              BIGSERIAL PRIMARY KEY,
    buyer_id        BIGINT NOT NULL,
    seller_id       BIGINT NOT NULL,
    variant_id      BIGINT NOT NULL,
    quantity        INT NOT NULL CHECK (quantity > 0),
    price           BIGINT,
    status          TEXT NOT NULL DEFAULT 'cart'
                        CHECK (status IN ('cart', 'ordered', 'accepted', 'shipped', 'ready_for_pickup', 'received', 'cancelled'))
);

CREATE INDEX idx_orders_buyer_id ON orders(buyer_id);
CREATE INDEX idx_orders_seller_id ON orders(seller_id);
CREATE INDEX idx_orders_status ON orders(status);
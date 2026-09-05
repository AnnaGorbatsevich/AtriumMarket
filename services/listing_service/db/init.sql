CREATE TYPE product_status AS ENUM ('active', 'hidden', 'deleted');

CREATE TABLE categories (
    id          BIGSERIAL PRIMARY KEY,
    name        TEXT NOT NULL,
    parent_id   BIGINT REFERENCES categories(id)
);

CREATE INDEX idx_categories_parent_id ON categories(parent_id);

CREATE TABLE products (
    id            BIGSERIAL PRIMARY KEY,
    seller_id     BIGINT NOT NULL,
    name          TEXT NOT NULL,
    description   TEXT,
    category_id   BIGINT REFERENCES categories(id),
    attributes    JSONB NOT NULL DEFAULT '{}'::jsonb,
    image_url     TEXT,
    status        product_status NOT NULL DEFAULT 'active',
    created_at    TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at    TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_products_seller_id ON products(seller_id);
CREATE INDEX idx_products_category_id ON products(category_id);

CREATE TABLE variants (
    id            BIGSERIAL PRIMARY KEY,
    product_id    BIGINT NOT NULL REFERENCES products(id) ON DELETE CASCADE,
    price         INTEGER NOT NULL CHECK (price >= 0),
    quantity      INTEGER NOT NULL DEFAULT 0 CHECK (quantity >= 0),
    options       JSONB NOT NULL DEFAULT '{}'::jsonb
);

CREATE INDEX idx_variants_product_id ON variants(product_id);

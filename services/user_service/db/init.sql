-- Schema for user_service. Only user_service talks to this database directly.

CREATE TYPE user_role AS ENUM ('buyer', 'seller');

CREATE TABLE users (
    id              BIGSERIAL PRIMARY KEY,
    role            user_role NOT NULL,
    full_name       TEXT NOT NULL,
    company_name    TEXT,
    email           TEXT NOT NULL UNIQUE,
    phone           TEXT NOT NULL,
    tax_id          TEXT,
    address         TEXT,
    description     TEXT,
    password_hash   TEXT NOT NULL,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);


CREATE TABLE refresh_tokens (
    id           BIGSERIAL PRIMARY KEY,
    user_id      BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    token_hash   TEXT NOT NULL UNIQUE,
    issued_at    TIMESTAMPTZ NOT NULL DEFAULT now(),
    expires_at   TIMESTAMPTZ NOT NULL,
    revoked_at   TIMESTAMPTZ
);

CREATE INDEX idx_refresh_tokens_user_id ON refresh_tokens(user_id);

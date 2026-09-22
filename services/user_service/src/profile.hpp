#pragma once

#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/storages/postgres/result_set.hpp>

namespace user_service {

std::string AuthenticatedEmail(const userver::server::http::HttpRequest& request);

userver::formats::json::Value ProfileToJson(const std::string& email, const userver::storages::postgres::Row& row);

}  // namespace user_service

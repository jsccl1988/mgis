// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// Shared mapd fake-HTTP bind for contract / e2e tests.

#pragma once

#include "fake_http_transport.h"

#include <concepts>
#include <memory>
#include <string>
#include <string_view>

namespace sg {
namespace detail {

inline constexpr std::string_view k_mapd_caps_golden = R"({
  "service": "mapd",
  "crs": "EPSG:3857",
  "data_version": "contract-2026-09-12.1",
  "query_layers": ["basemap"],
  "path_q": {
    "features": true,
    "analysis": true,
    "postgis_configured": false,
    "primary_store": "table"
  },
  "path_r": { "raster": true, "vector": true }
})";

inline constexpr std::string_view k_mapd_style_minimal =
    R"({"version":8,"sources":{},"layers":[]})";

inline constexpr std::string_view k_mapd_query_polygon = R"({
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "id": 1,
      "properties": { "osm_id": 42, "geom_type": "Polygon" },
      "geometry": {
        "type": "Polygon",
        "coordinates": [[[116.3, 39.8], [116.5, 39.8], [116.5, 40.0], [116.3, 40.0], [116.3, 39.8]]]
      }
    }
  ]
})";

inline constexpr std::string_view k_mapd_query_empty =
    R"({"type":"FeatureCollection","features":[]})";

}  // namespace detail

template <typename T>
concept mapd_fake_bind_like = requires {
  { T::caps } -> std::convertible_to<std::string_view>;
  { T::style } -> std::convertible_to<std::string_view>;
  { T::query_body } -> std::convertible_to<std::string_view>;
  T::reject_sql;
  T::reject_raw_sql;
};

struct mapd_e2e_fake_bind {
  static constexpr std::string_view caps = detail::k_mapd_caps_golden;
  static constexpr std::string_view style = detail::k_mapd_style_minimal;
  static constexpr std::string_view query_body = detail::k_mapd_query_polygon;
  static constexpr bool reject_sql = false;
  static constexpr bool reject_raw_sql = false;
};

struct mapd_link_fake_bind : mapd_e2e_fake_bind {
  static constexpr bool reject_sql = true;
};

struct mapd_client_fake_bind : mapd_e2e_fake_bind {
  static constexpr std::string_view query_body = detail::k_mapd_query_empty;
  static constexpr bool reject_sql = true;
  static constexpr bool reject_raw_sql = true;
};

template <mapd_fake_bind_like Bind = mapd_e2e_fake_bind>
std::shared_ptr<FakeHttpTransport> make_mapd_ready_transport() {
  auto transport = std::make_shared<FakeHttpTransport>();
  transport->set_handler("/mapd/api/v1/health", [](const HttpRequest&) {
    HttpResponse r;
    r.status = 200;
    r.body = R"({"ok":true,"service":"mapd"})";
    return r;
  });
  transport->set_handler("/mapd/api/v1/capabilities", [](const HttpRequest&) {
    HttpResponse r;
    r.status = 200;
    r.body.assign(Bind::caps);
    return r;
  });
  transport->set_handler("/mapd/style.json", [](const HttpRequest&) {
    HttpResponse r;
    r.status = 200;
    r.body.assign(Bind::style);
    return r;
  });
  transport->set_handler("/mapd/api/v1/features/query",
                         [](const HttpRequest& req) {
                           HttpResponse r;
                           const bool sql = req.body.find("\"sql\"") !=
                                            std::string::npos;
                           const bool raw =
                               req.body.find("\"raw_sql\"") !=
                                   std::string::npos ||
                               req.body.find("\"raw_query\"") !=
                                   std::string::npos;
                           if ((Bind::reject_sql && sql) ||
                               (Bind::reject_raw_sql && raw)) {
                             r.status = 400;
                             r.body = R"({"error":"forbidden_sql"})";
                             return r;
                           }
                           r.status = 200;
                           r.body.assign(Bind::query_body);
                           return r;
                         });
  return transport;
}

}  // namespace sg

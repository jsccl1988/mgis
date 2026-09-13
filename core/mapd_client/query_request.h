// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// 待 pin 替换: matches smartgis `include/sg/clients/query_request.h`.
// Path Q whitelist — no sql / raw_sql / raw_query field.

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sg {

struct QueryFilter {
  std::string column;
  std::string op;
  std::string value;
};

struct QueryRequest {
  std::string layer;
  std::optional<std::array<double, 4>> bbox;
  std::optional<double> lon;
  std::optional<double> lat;
  std::optional<double> buffer_m;
  std::optional<std::int64_t> id;
  std::vector<QueryFilter> filters;
  std::optional<int> limit;

  std::string ToJson() const;
};

static_assert(sizeof(QueryRequest) > 0, "QueryRequest must remain whitelist-only");

}  // namespace sg

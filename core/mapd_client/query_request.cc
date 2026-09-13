// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.

#include "query_request.h"

#include <cstdio>
#include <sstream>

namespace sg {
namespace {

std::string EscapeJsonString(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        out += c;
        break;
    }
  }
  return out;
}

std::string FormatDouble(double v) {
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.10g", v);
  return buf;
}

}  // namespace

std::string QueryRequest::ToJson() const {
  std::ostringstream o;
  o << "{\"layer\":\"" << EscapeJsonString(layer) << "\"";
  if (bbox) {
    o << ",\"bbox\":[" << FormatDouble((*bbox)[0]) << ","
      << FormatDouble((*bbox)[1]) << "," << FormatDouble((*bbox)[2]) << ","
      << FormatDouble((*bbox)[3]) << "]";
  }
  if (lon) {
    o << ",\"lon\":" << FormatDouble(*lon);
  }
  if (lat) {
    o << ",\"lat\":" << FormatDouble(*lat);
  }
  if (buffer_m) {
    o << ",\"buffer_m\":" << FormatDouble(*buffer_m);
  }
  if (id) {
    o << ",\"id\":" << *id;
  }
  if (!filters.empty()) {
    o << ",\"filters\":[";
    for (std::size_t i = 0; i < filters.size(); ++i) {
      if (i) {
        o << ",";
      }
      o << "{\"column\":\"" << EscapeJsonString(filters[i].column)
        << "\",\"op\":\"" << EscapeJsonString(filters[i].op) << "\",\"value\":\""
        << EscapeJsonString(filters[i].value) << "\"}";
    }
    o << "]";
  }
  if (limit) {
    o << ",\"limit\":" << *limit;
  }
  o << "}";
  return o.str();
}

}  // namespace sg

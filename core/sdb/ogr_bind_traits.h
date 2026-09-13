// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.
//
// Field / geometry type maps for OGR Memory bind. Schema strings → OGR
// types live in traits so sdbd_ogr_mem and tests share one table.

#pragma once

#include <gdal_priv.h>
#include <ogrsf_frmts.h>

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>

namespace core {
namespace sdb {

template <typename T>
concept ogr_alias_bind = requires {
  { T::ogr };
  T::names;
};

struct int64_field_bind {
  static constexpr OGRFieldType ogr = OFTInteger64;
  static constexpr std::string_view names[] = {"uint64", "int64", "integer"};
};

struct real_field_bind {
  static constexpr OGRFieldType ogr = OFTReal;
  static constexpr std::string_view names[] = {"real", "double"};
};

struct polygon_geom_bind {
  static constexpr OGRwkbGeometryType ogr = wkbPolygon;
  static constexpr std::string_view names[] = {"polygon"};
};

struct point_geom_bind {
  static constexpr OGRwkbGeometryType ogr = wkbPoint;
  static constexpr std::string_view names[] = {"point"};
};

struct line_geom_bind {
  static constexpr OGRwkbGeometryType ogr = wkbLineString;
  static constexpr std::string_view names[] = {"linestring", "line"};
};

namespace detail {

template <typename Range>
constexpr bool alias_contains(const Range& names, std::string_view type) {
  for (const auto name : names) {
    if (name == type) {
      return true;
    }
  }
  return false;
}

inline std::string ascii_tolower(std::string_view s) {
  std::string out(s);
  for (char& c : out) {
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }
  return out;
}

}  // namespace detail

// Compile-time alias tables; first match wins. Unknown → fallback.
template <typename Fallback, ogr_alias_bind... Binds>
struct ogr_alias_map {
  // MSVC C3892: decltype(static constexpr member) is const; fold assign needs a mutable copy.
  using value_type = std::remove_cv_t<decltype(Fallback::ogr)>;

  static constexpr value_type type_of(std::string_view type) {
    value_type out = Fallback::ogr;
    ((detail::alias_contains(Binds::names, type) ? (out = Binds::ogr, true)
                                                 : false) ||
     ...);
    return out;
  }
};

struct string_field_fallback {
  static constexpr OGRFieldType ogr = OFTString;
};

struct unknown_geom_fallback {
  static constexpr OGRwkbGeometryType ogr = wkbUnknown;
};

using ogr_field_bind_traits =
    ogr_alias_map<string_field_fallback, int64_field_bind, real_field_bind>;

using ogr_geom_bind_traits =
    ogr_alias_map<unknown_geom_fallback, polygon_geom_bind, point_geom_bind,
                  line_geom_bind>;

struct ogr_bind_traits {
  static constexpr OGRFieldType field_type_of(std::string_view type) {
    return ogr_field_bind_traits::type_of(type);
  }

  static OGRwkbGeometryType geom_type_of(std::string_view type) {
    return ogr_geom_bind_traits::type_of(detail::ascii_tolower(type));
  }
};

inline GDALDataset* create_memory_dataset(const std::string& name) {
  GDALAllRegister();
  GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("Memory");
  if (!driver) {
    driver = GetGDALDriverManager()->GetDriverByName("MEM");
  }
  if (!driver) {
    return nullptr;
  }
  GDALDataset* ds =
      driver->Create(name.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
  if (ds) {
    return ds;
  }
  return driver->Create(name.c_str(), 1, 1, 1, GDT_Byte, nullptr);
}

}  // namespace sdb
}  // namespace core

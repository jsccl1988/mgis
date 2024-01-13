// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GEOMETRY_GEOMETRY_H
#define GEOMETRY_GEOMETRY_H

#include "core/core.h"
/* For geometry operations */
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>

/* For WKT read/write */
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>

namespace core {
/* Geometry/GeometryFactory */
using geos::geom::Geometry;
using geos::geom::GeometryFactory;

/* WKTReader/WKTWriter */
using geos::io::WKTReader;
using geos::io::WKTWriter;
}  // namespace core
#endif  // GEOMETRY_GEOMETRY_H
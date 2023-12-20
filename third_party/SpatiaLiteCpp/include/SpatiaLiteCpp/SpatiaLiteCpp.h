/**
 * @file    SpatiaLiteCpp.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main header file to gain access to all functionality.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
/**
 * @defgroup SpatiaLiteCpp SpatiaLiteC++
 * @brief    SpatiaLiteC++ is a C++ SpatiaLite wrapper that is based on the
 *           SQLite++ wrapper.
 */
#pragma once

// Include useful headers of SpatiaLiteC++
#include "SpatiaLiteCpp/Auxiliary.h"
#include "SpatiaLiteCpp/Blob.h"
#include "SpatiaLiteCpp/Buffer.hpp"
#include "SpatiaLiteCpp/Checksum.h"
#include "SpatiaLiteCpp/Converter.h"
#include "SpatiaLiteCpp/Dbf.h"
#include "SpatiaLiteCpp/DbfField.h"
#include "SpatiaLiteCpp/DbfList.h"
#include "SpatiaLiteCpp/DynamicLine.h"
#include "SpatiaLiteCpp/ExifTagList.h"
#include "SpatiaLiteCpp/GeometryCollection.h"
#include "SpatiaLiteCpp/LineString.h"
#include "SpatiaLiteCpp/OutputBuffer.h"
#include "SpatiaLiteCpp/Point.h"
#include "SpatiaLiteCpp/Polygon.h"
#include "SpatiaLiteCpp/Ring.h"
#include "SpatiaLiteCpp/Shapefile.h"
#include "SpatiaLiteCpp/SpatialDatabase.h"
#include "SpatiaLiteCpp/VectorLayersList.h"
#include "SpatiaLiteCpp/WfsCatalog.h"
#include "SpatiaLiteCpp/WfsSchema.h"
#include "SpatiaLiteCpp/XmlBlob.h"
#include "SpatiaLiteCpp/XmlDocument.h"

/**
 * @def SPATIALITECPP_PTRTYPE
 * @brief Determines which pointer type to use
 * @details Defined values:
 *     - 0 = Raw pointer (Caller handles memory deallocation.
 *           Should define SPATIALITECPP_PTR to a custom smart pointer.)
 *     - 1 = SRombauts/shared_ptr
 *     - 2 = std::tr1::shared_ptr
 *     - 3 = std::shared_ptr (C++11)
 *     - 4 = boost::shared_ptr
 */
/**
 * @def SPATIALITECPP_PTR(x)
 * @brief Defines the wrapper pointer.
 * @details Should be defined if SPATIALITECPP_PTRTYPE = 0. For example,
 * @code #define SPATIALITECPP_PTR(x) std::shared_ptr<x> @endcode
 */
#ifndef SPATIALITECPP_PTRTYPE
    #define SPATIALITECPP_PTRTYPE 1
#endif
#if SPATIALITECPP_PTRTYPE == 1
    #include "shared_ptr.hpp"
    #define SPATIALITECPP_PTR(x) shared_ptr<x>
#elif SPATIALITECPP_PTRTYPE == 2
    #include <memory>
    #define SPATIALITECPP_PTR(x) std::tr1::shared_ptr<x>
#elif SPATIALITECPP_PTRTYPE == 3
    #include <memory>
    #define SPATIALITECPP_PTR(x) std::shared_ptr<x>
#elif SPATIALITECPP_PTRTYPE == 4
    #include "boost/shared_ptr.hpp"
    #define SPATIALITECPP_PTR(x) boost::shared_ptr<x>
#else
    #ifndef SPATIALITECPP_PTR
        #define SPATIALITECPP_PTR(x) x*
    #endif
#endif

// Define useful typedefs for shared pointers
/**
 * Main SpatiaLiteC++ namespace
 */
namespace SpatiaLite
{
    /**
     * Blob buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::Blob) BlobPtr;
    /**
     * Checksum buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::Checksum) ChecksumPtr;
    /**
     * Converter buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::Converter) ConverterPtr;
    /**
     * DBF buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::Dbf) DbfPtr;
    /**
     * DBF Field buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::DbfField) DbfFieldPtr;
    /**
     * DBF List buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::DbfList) DbfListPtr;
    /**
     * Dynamic Line buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::DynamicLine) DynamicLinePtr;
    /**
     * EXIF Tag List buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::ExifTagList) ExifTagListPtr;
    /**
     * Geometry buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::GeometryBuffer) GeometryBufferPtr;
    /**
     * Geometry Collection buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::GeometryCollection) GeometryCollectionPtr;
    /**
     * Line String buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::LineString) LineStringPtr;
    /**
     * Output Buffer buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::OutputBuffer) OutputBufferPtr;
    /**
     * Point buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::Point) PointPtr;
    /**
     * Polygon buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::Polygon) PolygonPtr;
    /**
     * Ring buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::Ring) RingPtr;
    /**
     * Shapefile buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::Shapefile) ShapefilePtr;
    /**
     * Spatial Database buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::SpatialDatabase) SpatialDatabasePtr;
    /**
     * Vector Layers List buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::VectorLayersList) VectorLayersListPtr;
    /**
     * WFS Catalog buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::WfsCatalog) WfsCatalogPtr;
    /**
     * WFS Schema buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::WfsSchema) WfsSchemaPtr;
    /**
     * XML Blob buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::XmlBlob) XmlBlobPtr;
    /**
     * XMl Document buffer pointer
     */
    typedef SPATIALITECPP_PTR(SpatiaLite::XmlDocument) XmlDocumentPtr;
}

#include "SQLiteCpp/SQLiteCpp.h"

/**
 * @def SPATIALITECPP_VERSION
 * @brief Version number string
 * @details Evaluates to a string literal in the SQLite format "X.Y.Z" where
 *              - X is the major version number
 *              - Y is the minor version number
 *              - Z is the release number
 */
#define SPATIALITECPP_VERSION           "0.1.0"

/**
 * @def SPATIALITECPP_VERSION_NUMBER
 * @brief Version number
 * @details Resolves to an integer with the value (X*1000000 + Y*1000 + Z)
 *          where X, Y, and Z are the same numbers used in
 *          SPATIALITECPP_VERSION.
 */
#define SPATIALITECPP_VERSION_NUMBER    0001000

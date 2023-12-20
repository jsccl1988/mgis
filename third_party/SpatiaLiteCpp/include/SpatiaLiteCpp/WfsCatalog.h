/**
 * @file    WfsCatalog.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main WfsCatalog class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

extern "C"
{
#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gg_wfs.h"
}

#include <string>

namespace SpatiaLite
{

    // Forward declarations
    class SpatialDatabase;

    /**
     * WFS catalog buffer data type
     */
    typedef gaiaWFScatalogPtr WfsCatalogType;

    /**
     * @brief RAII management of a gaiaWFScatalogPtr.
     */
    class SPATIALITECPP_ABI WfsCatalog : public Buffer<WfsCatalogType>
    {

    public:

        /**
         * @brief Create WFS catalog
         * @param[in] path Pointer to some WFS-GetCapabilities XML Document
         *                 (could be a pathname or an URL)
         * @returns New WfsCatalog pointer or NULL on error
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer.
         *          Should be owned by a WfsCatalogPtr.
         */
        static WfsCatalog * create(const char * path);

        /**
         * @brief Loads data from some WFS source.
         * @param[in] database     Current spatial database
         * @param[in] path         Pointer to some WFS-GetFeature XML Document
         *                         (could be a pathname or an URL).
         * @param[in] uri          An alternative URI for DescribeFeatureType
         *                         to be used if no one is found within the XML
         *                         document returned by GetFeature.
         * @param[in] layer        The name of the WFS layer.
         * @param[in] swap         If TRUE the X and Y axes will be swapped
         * @param[in] table        The name of the table to be created
         * @param[in] primary      Name of the Primary Key column; if NULL or
         *                         mismatching then "PK_UID" will be assumed by
         *                         default.
         * @param[in] spatial      If TRUE an R*Tree Spatial Index will be
         *                         created
         * @param[in] rows         On completion will contain the total number
         *                         of actually imported rows
         * @param[in] progress     Pointer to a callback function to be invoked
         *                         immediately after processing each WFS page
         *                         (could be NULL)
         * @param[in] callback     An arbitrary pointer (to be passed as the
         *                         second argument by the callback function).
         * @throws std::runtime_error on failure
         */
        static void load(SpatialDatabase const & database,
                         const char * path,
                         const char * uri,
                         const char * layer,
                         const int swap,
                         const char * table,
                         const char * primary,
                         int spatial,
                         int * rows,
                         void (* progress)(int, void *),
                         void * callback);

        /**
         * @brief Loads data from some WFS source (using WFS paging)
         * @param[in] database     Current spatial database
         * @param[in] path         Pointer to some WFS-GetFeature XML Document
         *                         (could be a pathname or an URL).
         * @param[in] uri          An alternative URI for DescribeFeatureType
         *                         to be used if no one is found within the XML
         *                         document returned by GetFeature.
         * @param[in] layer        The name of the WFS layer.
         * @param[in] swap         If TRUE the X and Y axes will be swapped
         * @param[in] table        The name of the table to be created
         * @param[in] primary      Name of the Primary Key column; if NULL or
         *                         mismatching then "PK_UID" will be assumed by
         *                         default.
         * @param[in] spatial      If TRUE an R*Tree Spatial Index will be
         *                         created
         * @param[in] page         Max number of features for each single WFS
         *                         call; if zero or negative a single monolithic
         *                         page is assumed (i.e. paging will not be
         *                         applied).
         * @param[in] rows         On completion will contain the total number
         *                         of actually imported rows
         * @param[in] progress     Pointer to a callback function to be invoked
         *                         immediately after processing each WFS page
         *                         (could be NULL)
         * @param[in] callback     An arbitrary pointer (to be passed as the
         *                         second argument by the callback function).
         * @throws std::runtime_error on failure
         */
        static void paged(SpatialDatabase const & database,
                          const char * path,
                          const char * uri,
                          const char * layer,
                          const int swap,
                          const char * table,
                          const char * primary,
                          int spatial,
                          int page,
                          int * rows,
                          void (* progress)(int, void *),
                          void * callback);

    private:

        /**
         * @brief Takes ownership of an existing WFS catalog pointer
         * @param[in] catalog Pointer to WFS catalog.
         */
        explicit WfsCatalog(WfsCatalogType catalog);

    };

}

/**
 * @file    WfsCatalog.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main WfsCatalog class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/WfsCatalog.h"

#include "SpatiaLiteCpp/Auxiliary.h"
#include "SpatiaLiteCpp/SpatialDatabase.h"

#include "SQLiteCpp/SQLiteCpp.h"

#include <stdexcept>

namespace SpatiaLite
{

    WfsCatalog::WfsCatalog(WfsCatalogType catalog) :
        Buffer<WfsCatalogType>(catalog, destroy_wfs_catalog)
    {
    }

    WfsCatalog * WfsCatalog::create(const char * path)
    {
        char * message = 0;
        WfsCatalogType catalog = create_wfs_catalog(path, &message);
        if (!catalog)
        {
            std::string error = Auxiliary::toString(
                message, "Failed to create WFS catalog!");
            throw std::runtime_error(error);
        }
        return new WfsCatalog(catalog);
    }

    void WfsCatalog::load(SpatialDatabase const & database,
                          const char * path,
                          const char * uri,
                          const char * layer,
                          const int swap,
                          const char * table,
                          const char * primary,
                          int spatial,
                          int * rows,
                          void (* progress)(int, void *),
                          void * callback)
    {
        char * message = 0;
        int status = load_from_wfs(database.getDatabase()->getHandle(),
                                   path,
                                   uri,
                                   layer,
                                   swap,
                                   table,
                                   primary,
                                   spatial,
                                   rows,
                                   &message,
                                   progress,
                                   callback);
        if (!status)
        {
            std::string error = Auxiliary::toString(
                message, "Failed to load WFS catalog!");
            throw std::runtime_error(error);
        }
        return;
    }

    void WfsCatalog::paged(SpatialDatabase const & database,
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
                           void * callback)
    {
        char * message = 0;
        int status = load_from_wfs_paged(database.getDatabase()->getHandle(),
                                         path,
                                         uri,
                                         layer,
                                         swap,
                                         table,
                                         primary,
                                         spatial,
                                         page,
                                         rows,
                                         &message,
                                         progress,
                                         callback);
        if (!status)
        {
            std::string error = Auxiliary::toString(
                message, "Failed to load WFS catalog using paging!");
            throw std::runtime_error(error);
        }
        return;
    }

}

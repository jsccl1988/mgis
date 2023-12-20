/**
 * @file    SpatialDatabase.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main SpatialDatabase class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/SpatialDatabase.h"

extern "C"
{
#include "spatialite.h"
}

#include "SQLiteCpp/SQLiteCpp.h"

#include <sstream>

namespace SpatiaLite
{

    SpatialDatabase::SpatialDatabase(const std::string & filename,
                                     const int           flags,
                                     const int           timeout,
                                     const std::string & vfs,
                                     const int           verbose)
    {
        // ==================================================
        // Open an in-memory database connection
        // --------------------------------------------------
        this->_database = new SQLite::Database(filename, flags, timeout, vfs);

        // ==================================================
        // Initialize spatialite
        // --------------------------------------------------
        this->_cache = spatialite_alloc_connection();
        spatialite_init_ex(this->_database->getHandle(), this->_cache, verbose);
    }

    SpatialDatabase::~SpatialDatabase()
    {
        spatialite_cleanup_ex(this->getCache());
        delete this->getDatabase();
    }

    int SpatialDatabase::checkGeometries(const std::string & directory,
                                         int * numInvalids,
                                         std::string & error) const
    {
        int status = 0;
        char * message = 0;
        sqlite3 * handle = this->getDatabase()->getHandle();
        status = check_all_geometry_columns(handle,
                                            directory.c_str(),
                                            numInvalids,
                                            &message);
        if (message)
        {
            error = std::string(message);
            gaiaFree(message);
        }
        return status;
    }

    int SpatialDatabase::checkGeometry(const std::string & table,
                                       const std::string & geometry,
                                       const std::string & report,
                                       int * numRows,
                                       int * numInvalids,
                                       std::string & error) const
    {
        int status = 0;
        char * message = 0;
        sqlite3 * handle = this->getDatabase()->getHandle();
        status = check_geometry_column(handle,
                                       table.c_str(),
                                       geometry.c_str(),
                                       report.c_str(),
                                       numRows,
                                       numInvalids,
                                       &message);
        if (message)
        {
            error = std::string(message);
            gaiaFree(message);
        }
        return status;
    }

    void * SpatialDatabase::getCache() const
    {
        return this->_cache;
    }

    int
    SpatialDatabase::getCount(const std::string & name) const
    {

        // ==================================================
        // Build and execute SQL
        // --------------------------------------------------
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM " << name << ";";
        SQLite::Statement query(*this->getDatabase(), sql.str());
        if (query.executeStep())
        {
            return query.getColumn(0);
        }

        return 0;

    }

    SQLite::Database * SpatialDatabase::getDatabase() const
    {
        return this->_database;
    }

    std::string
    SpatialDatabase::getGeometryName(int type)
    {
        if (type == GAIA_POINT)
        {
            return "POINT";
        }
        else if (type == GAIA_LINESTRING)
        {
            return "LINESTRING";
        }
        else if (type == GAIA_POLYGON)
        {
            return "POLYGON";
        }
        else if (type == GAIA_MULTIPOINT)
        {
            return "MULTIPOINT";
        }
        else if (type == GAIA_MULTILINESTRING)
        {
            return "MULTILINESTRING";
        }
        else if (type == GAIA_MULTIPOLYGON)
        {
            return "MULTIPOLYGON";
        }
        else if (type == GAIA_GEOMETRYCOLLECTION)
        {
            return "GEOMETRYCOLLECTION";
        }
        else
        {
            return "EMPTY / NULL GEOMETRY";
        }
    }

    std::vector<std::string> SpatialDatabase::getHeaders(const std::string & name) const
    {

        std::vector<std::string> headers;

        // ==================================================
        // Build and execute SQL
        // --------------------------------------------------
        std::stringstream sql;
        sql << "PRAGMA table_info('" << name << "');";
        SQLite::Statement query(*this->getDatabase(), sql.str());
        while (query.executeStep())
        {
            headers.push_back(query.getColumn(1).getText());
        }

        return headers;

    }

    std::vector<std::string> SpatialDatabase::getTypes(const std::string & name) const
    {

        std::vector<std::string> types;

        // ==================================================
        // Build and execute SQL
        // --------------------------------------------------
        std::stringstream sql;
        sql << "PRAGMA table_info('" << name << "');";
        SQLite::Statement query(*this->getDatabase(), sql.str());
        while (query.executeStep())
        {
            types.push_back(query.getColumn(2).getText());
        }

        return types;

    }

    int SpatialDatabase::sanitizeGeometries(const std::string & prefix,
                                            const std::string & directory,
                                            int * numNotRepaired,
                                            std::string & error)
    {

        int status = 0;
        char * message = 0;
        sqlite3 * handle = this->getDatabase()->getHandle();
        status = sanitize_all_geometry_columns(handle,
                                               prefix.c_str(),
                                               directory.c_str(),
                                               numNotRepaired,
                                               &message);
        if (message)
        {
            error = std::string(message);
            gaiaFree(message);
        }
        return status;

    }

    int SpatialDatabase::sanitizeGeometry(const std::string & table,
                                          const std::string & geometry,
                                          const std::string & temp,
                                          const std::string & report,
                                          int * numInvalids,
                                          int * numRepaired,
                                          int * numDiscarded,
                                          int * numFailures,
                                          std::string & error)
    {

        int status = 0;
        char * message = 0;
        sqlite3 * handle = this->getDatabase()->getHandle();
        status = sanitize_geometry_column(handle,
                                          table.c_str(),
                                          geometry.c_str(),
                                          temp.c_str(),
                                          report.c_str(),
                                          numInvalids,
                                          numRepaired,
                                          numDiscarded,
                                          numFailures,
                                          &message);
        if (message)
        {
            error = std::string(message);
            gaiaFree(message);
        }
        return status;

    }

}

/**
 * @file    SpatialDatabase.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Spatial Database class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/SpatiaLiteCppAbi.h"

#include "sqlite3.h"

#include <string>
#include <vector>

// Forward declarations
namespace SQLite
{
    class Database;
}

namespace SpatiaLite
{

    /**
     * @brief RAII management of a Spatialite Database Connection.
     */
    class SPATIALITECPP_ABI SpatialDatabase
    {

    public:

        /**
         * @brief Open the spatialite database.
         * @param[in] filename UTF-8 path/uri to the database file ("filename"
         *                     sqlite3 parameter)
         * @param[in] flags    sqlite3 flags For File Open Operations (e.g., 
         *                     SQLITE_OPEN_READONLY, SQLITE_OPEN_READWRITE,
         *                     SQLITE_OPEN_CREATE, ...
         * @param[in] timeout  Amount of milliseconds to wait before returning
         *                     SQLITE_BUSY (see setBusyTimeout())
         * @param[in] vfs      UTF-8 name of custom VFS to use, or empty string
         *                     for sqlite3 default
         * @param[in] verbose  True if a short start-up message is shown on
         *                     stderr
         */
        SpatialDatabase(const std::string & filename = ":memory:",
                        const int           flags    = SQLITE_OPEN_READONLY,
                        const int           timeout  = 0,
                        const std::string & vfs      = "",
                        const int           verbose  = 0);

        /**
         * @brief Close the spatialite database.
         */
        ~SpatialDatabase();

        /**
         * @brief Checks all Geometry Columns for validity.
         * @param[in] directory    Pathname of the directory to be created for
         *                         report-files
         * @param[out] numInvalids If this variable is not NULL on successful
         *                         completion will contain the total number of
         *                         invalid Geometries found
         * @param[out] error       If the return status is ZERO (failure), an
         *                         appropriate error message will be returned
         * @returns 0 on failure, any other value on success
         */
        int checkGeometries(const std::string & directory,
                            int * numInvalids,
                            std::string & error) const;

        /**
         * @brief Checks a Geometry Column for validity.
         * @param[in]  table       Name of the table
         * @param[in]  geometry    Name of the column to be checked
         * @param[in]  report      Pathname of the report-file
         * @param[out] numRows     Total number of rows found into the table
         * @param[out] numInvalids Total number of invalid geometries found
         * @param[out] error       If the return status is ZERO (failure), an
         *                         appropriate error message will be returned
         * @returns 0 on failure, any other value on success
         */
        int checkGeometry(const std::string & table,
                          const std::string & geometry,
                          const std::string & report,
                          int * numRows,
                          int * numInvalids,
                          std::string & error) const;

        /**
         * @brief Get database cache
         * @returns Pointer to cache
         */
        void * getCache() const;

        /**
         * @brief Number of records
         * @param[in] name Table name
         * @returns Number of records
         */
        int getCount(const std::string & name) const;

        /**
         * Get database connection
         */
        SQLite::Database * getDatabase() const;

        /**
         * @brief Get geometry type as string.
         * @param[in] type SpatiaLite type (GAIA_XXXX)
         * @returns String name of geometry type
         */
        static std::string getGeometryName(int type);

        /**
         * @brief Get table headers
         * @param[in] name Table name
         * @returns Vector of column header names
         */
        std::vector<std::string> getHeaders(const std::string & name) const;

        /**
         * @brief Get table types
         * @param[in] name Table name
         * @returns Vector of column types
         */
        std::vector<std::string> getTypes(const std::string & name) const;

        /**
         * @brief Sanitizes all Geometry Columns making all invalid geometries
         *        to be valid.
         * @param[in] prefix          Name-prefix for temporary tables
         * @param[in] directory       Pathname of the directory to be created
         *                            for report-files
         * @param[out] numNotRepaired If this variable is not NULL on successful
         *                            completion will contain the total count of
         *                            repair failures (i.e. Geometries beyond
         *                            possible repair)
         * @param[out] error          If this variable is not NULL and the
         *                            return status is ZERO (failure), an
         *                            appropriate error message will be returned
         * @returns 0 on failure, any other value on success
         */
        int sanitizeGeometries(const std::string & prefix,
                               const std::string & directory,
                               int * numNotRepaired,
                               std::string & error);

        /**
         * @brief Sanitizes a Geometry Column making all invalid geometries to
         *        be valid.
         * @param[in]  table        Name of the table
         * @param[in]  geometry     Name of the column to be checked
         * @param[in]  temp         Name of the temporary table
         * @param[in]  report       Pathname of the report-file
         * @param[out] numInvalids  If this variable is not NULL on successful
         *                          completion will contain the total number of
         *                          invalid geometries found into the sanitize
         *                          table
         * @param[out] numRepaired  If this variable is not NULL on successful
         *                          completion will contain the total number of
         *                          repaired geometries
         * @param[out] numDiscarded If this variable is not NULL on successful
         *                          completion will contain the total number of
         *                          repaired geometries (by discarding
         *                          fragments)
         * @param[out] numFailures  If this variable is not NULL on successful
         *                          completion will contain the total number of
         *                          repair failures (i.e. Geometries beyond
         *                          possible repair)
         * @param[out] error        If the return status is ZERO (failure), an
         *                          appropriate error message will be returned
         * @returns 0 on failure, any other value on success
         */
        int sanitizeGeometry(const std::string & table,
                             const std::string & geometry,
                             const std::string & temp,
                             const std::string & report,
                             int * numInvalids,
                             int * numRepaired,
                             int * numDiscarded,
                             int * numFailures,
                             std::string & error);

private:

    // Disallow copying and assignment
    SpatialDatabase & operator=(const SpatialDatabase &);
    SpatialDatabase(const SpatialDatabase &);

    private:

        /**
         * Spatialite memory block
         */
        void * _cache;

        /**
         * Database connection
         */
        SQLite::Database * _database;

    };

}

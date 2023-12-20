/** 
 * @file demos.cpp
 * @brief SpatiaLiteC++ version of the spatialite demo code
 * @example demos.cpp
 */

#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

using namespace SpatiaLite;

/**
 * @brief Demo #1
 * @details C++ rewrite of spatialite demo1.c
 * @returns 0 if success otherwise failure
 */
int demo1(int argc, char *argv[]);

int demos(int argc, char *argv[])
{

    int status = 0;

    status = demo1(argc, argv);
    if (status != 0) return status;

    return 0;

}

int demo1(int argc, char *argv[])
{

    std::cout << "========================================" << std::endl;
    std::cout << "Demo #1" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " test_db_path" << std::endl;
	    return -1;
    }

    SpatialDatabase db(argv[1], SQLITE_OPEN_READONLY);

    std::cout << "SQLite version: " << sqlite3_libversion() << std::endl;
    std::cout << "SpatiaLite version: " << spatialite_version();

    /* 
     * SQL query #1 
     * we'll retrieve GEOMETRY tables from Spatial Metadata 
     */
    std::string sql =
        "SELECT DISTINCT f_table_name FROM geometry_columns ORDER BY 1;";
    SQLite::Statement query(*db.getDatabase(), sql);

    std::vector<std::string> tables;
    while (query.executeStep())
    {
        tables.push_back(query.getColumn(0).getText());
    }

    for (int i = 0; i < (int)tables.size(); i++)
    {
        /* now we'll scan each geotable we've found in Spatial Metadata */
        std::cout << std::endl << std::endl << std::endl;
        std::cout << "========= table '" << tables.at(i) <<
                     "' ========================";

        /*
         * SQL query #2 
         * we'll retrieve any column from the current geotable 
         */
        std::stringstream sql;
        sql << "SELECT * FROM " << tables.at(i) << ";";
        SQLite::Statement query(*db.getDatabase(), sql.str());

        int numRows = 0;
        while (query.executeStep())
        {
            numRows++;
            std::cout << std::endl << "row #" << numRows;

            for (int j = 0; j < query.getColumnCount(); j++)
            {
                std::cout << std::endl << "\t";

                SQLite::Column column = query.getColumn(j);
                std::cout << query.getColumnName(j) << " = " << column << " ";
                if (column.getType() == SQLITE_BLOB)
                {

                    GeometryCollectionPtr collection(
                        new GeometryCollection(column));

                    int type = gaiaGeometryType(collection->get());
                    std::string name = SpatialDatabase::getGeometryName(type);

                    std::cout << std::setw(10) << name << " ";
                    if (type == GAIA_UNKNOWN) continue;

                    std::cout << "SRID=" << collection->get()->Srid << " ";

                    double measure = 0;
                    if (type == GAIA_LINESTRING || type == GAIA_MULTILINESTRING)
                    {
                        int status = gaiaGeomCollLength(collection->get(),
                                                        &measure);
                        std::cout << "length=" << measure << " ";
                    }

                    if (type == GAIA_POLYGON || type == GAIA_MULTIPOLYGON)
                    {
                        int status = gaiaGeomCollArea(collection->get(),
                                                      &measure);
                        std::cout << "area=" << measure << " ";
                    }
                    continue;
                }
            }
            if (numRows >= 5)
            {
                /* we'll exit the loop after the first 5 rows -
                 * this is only a demo :-)
                 */
                break;
            }
        }
    }
    std::cout << std::endl;

    return 0;

}

/**
 * @file    samples.cpp
 * @brief   Main sample code
 * @example samples.cpp
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

#include <iostream>

/**
 * @brief Sample #1
 * @details Opens spatial database and counts the number of records.
 * @returns 0 if success otherwise failure
 */
int sample1();

/**
 * @brief Sample #2
 * @details Opens ESRI Shapefile and reads features.
 * @returns 0 if success otherwise failure
 */
int sample2();

int samples()
{

    int status = 0;

    status = sample1();
    if (status != 0) return status;

    status = sample2();
    if (status != 0) return status;

    return 0;

}

int sample1()
{

    std::cout << "========================================" << std::endl;
    std::cout << "Sample #1" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    SpatiaLite::SpatialDatabase db("states/db.sqlite");
    SQLite::Statement query(*db.getDatabase(), "SELECT COUNT(*) FROM states;");
    if (!query.executeStep()) return 1;
    std::cout << "Number of records = " << query.getColumn(0) << std::endl;

    return 0;

}

int sample2()
{

    std::cout << "========================================" << std::endl;
    std::cout << "Sample #2" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // Open Shapefile
    SpatiaLite::ShapefilePtr shapefile(new SpatiaLite::Shapefile(gaiaAllocShapefile()));
    gaiaOpenShpRead(shapefile->get(), "states/states", "UTF-8", "UTF-8");
    if (shapefile->get()->Valid == 0) return 1;

    // Read features until EOF
    int count = 0;
    while (gaiaReadShpEntity(shapefile->get(), count, 4326)) count++;
    std::cout << "Number of shapes: " << count << std::endl;

    return 0;

}

/**
 * @file    examples.cpp
 * @brief   Main examples code
 * @example examples.cpp
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#pragma warning(disable:4996)
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace SpatiaLite;

/**
 * @brief Get column at column/row
 * @param[in] database Spatial database
 * @param[in] name     Table name
 * @param[in] column   String name of column to query
 * @param[in] row      Index of row to query
 * @returns SQLite++ Column
 */
SQLite::Column getColumn(SpatialDatabase const & database,
                         const std::string & name,
                         std::string const & column,
                         int const row)
{
        // Build and execute SQL
        std::stringstream sql;
        sql << "SELECT " << column << " FROM " << name;
        sql << " LIMIT 1 OFFSET " << row << ";";
        SQLite::Statement query(*database.getDatabase(), sql.str());
        query.executeStep();
        return query.getColumn(0);
}

/**
 * Run various auxillary methods
 * @returns 0 if success otherwise failure
 */
int exAuxillary()
{
    std::cout << "========================================" << std::endl;
    std::cout << "Run auxillary methods" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    int width = 15;

    // ==================================================
    // Quote/dequote SQL
    // --------------------------------------------------
    std::string sql = "SELECT hello FROM 'world';";
    int quote = GAIA_SQL_SINGLE_QUOTE;
    std::string singleq = Auxiliary::quotedSql(sql.c_str(), quote);
    std::string doubleq = Auxiliary::doubleQuotedSql(sql.c_str());
    std::string dequote = Auxiliary::dequotedSql(doubleq.c_str());

    std::cout << "SQL quoting" << std::endl;
    std::cout << std::setw(width) << "sql: " << sql << std::endl;
    std::cout << std::setw(width) << "single quoted: " << singleq << std::endl;
    std::cout << std::setw(width) << "double quoted: " << doubleq << std::endl;
    std::cout << std::setw(width) << "dequoted: " << dequote << std::endl;

    return 0;

}

/**
 * Check database validity
 * @param[in]  db         Source database
 * @param[in]  table      Database table to query
 * @param[in]  name       Item name column header name
 * @param[in]  geometry   Table geometry column header name
 * @returns 0 if success otherwise failure
 */
int exCheckDatabase(SpatialDatabase const & db,
                    std::string const & table,
                    std::string const & name,
                    std::string const & geometry)
{

    int width = 20;

    std::cout << "========================================" << std::endl;
    std::cout << "Check database validity" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // ==================================================
    // Check all geometries
    // --------------------------------------------------
    std::cout << "Checking all geometries" << std::endl;
    int numInvalids = 0;
    std::string error;
    int status = db.checkGeometries(".", &numInvalids, error);
    if (status == 0)
    {
        std::cerr << std::setw(width) << error << std::endl;
    }
    std::cout << std::setw(width) << "# invalid: " << numInvalids << std::endl;

    // ==================================================
    // Check geometry column
    // --------------------------------------------------
    std::cout << "Checking geometry column: " << geometry << std::endl;
    int numRows = 0;
    status = db.checkGeometry(table,
                              geometry,
                              "report.html",
                              &numRows,
                              &numInvalids,
                              error);
    if (status == 0)
    {
        std::cerr << std::setw(width) << error << std::endl;
    }
    std::cout << std::setw(width) << "# invalid: " << numInvalids << std::endl;
    std::cout << std::setw(width) << "# valid: " << numRows << std::endl;

    return 0;

}

/**
 * Compute checksum of each geometry blob
 * @param[in]  db         Source database
 * @param[in]  table      Database table to query
 * @param[in]  name       Item name column header name
 * @param[in]  geometry   Table geometry column header name
 * @returns 0 if success otherwise failure
 */
int exChecksum(SpatialDatabase const & db,
               std::string const & table,
               std::string const & name,
               std::string const & geometry)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Compute checksum of geometry blobs" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    int numGeometries = db.getCount(table);
    for (int i = 0; i < numGeometries; i++)
    {

        SQLite::Column namei = getColumn(db, table, name, i);
        SQLite::Column blobi = getColumn(db, table, geometry, i);

        ChecksumPtr sum(new Checksum(gaiaCreateMD5Checksum()));
        sum->update((const unsigned char *)blobi.getBlob(), blobi.getBytes());
        std::string final = sum->finalize();

        std::cout << std::setw(20) << namei << ": " << final << std::endl;

    }

    return 0;

}

/**
 * Convert geometry names to UTF8
 * @param[in]  db    Source database
 * @param[in]  table Database table to query
 * @param[in]  name  Item name column header name
 * @returns 0 if success otherwise failure
 */
int exConvertUtf8(SpatialDatabase const & db,
                  std::string const & table,
                  std::string const & name)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Convert geometry names to UTF-8" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    int numGeometries = db.getCount(table);
    for (int i = 0; i < numGeometries; i++)
    {

        SQLite::Column namei = getColumn(db, table, name, i);
        const char * text = namei.getText();
        int size = namei.getBytes();

        ConverterPtr converter(new Converter("ASCII"));
        std::string convert = converter->convert(text, size);

        std::cout << std::setw(20) << namei << ": " << convert << std::endl;

    }

    return 0;

}

/**
 * Create various geometry blobs
 * @returns 0 if success otherwise failure
 */
int exCreateBlobs()
{

    std::cout << "========================================" << std::endl;
    std::cout << "Create various geometry blobs" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // Create points
    int srid = 4326;
    double x = 1.5;
    double y = 2.75;
    double z = 3.0;
    double m = 10;
    BlobPtr point(Point::makePoint(srid, x, y));
    BlobPtr pointZ(Point::makePoint(srid, x, y, &z));
    BlobPtr pointM(Point::makePoint(srid, x, y, 0, &m));
    BlobPtr pointZM(Point::makePoint(srid, x, y, &z, &m));

    // Create lines
    GeometryCollectionPtr point1(new GeometryCollection(*point));
    GeometryCollectionPtr point2(new GeometryCollection(*pointZ));
    BlobPtr line1(LineString::makeLine(point1->get(), point2->get()));

    GeometryCollectionPtr point3(new GeometryCollection(*pointM));
    GeometryCollectionPtr point4(new GeometryCollection(*pointZM));
    BlobPtr line2(LineString::makeLine(point3->get(), point4->get()));

    // Create various BLOB ypes
    BlobPtr compressed(Blob::toCompressedBlobWkb(point1->get()));
    BlobPtr fgf(Blob::toFgf(point1->get(), GAIA_XY));
    BlobPtr hex(Blob::toHexWkb(point1->get()));
    BlobPtr splite(Blob::toSpatiaLiteBlobWkb(point1->get()));
    BlobPtr wkb(Blob::toWkb(point1->get()));

    // Print summaries
    int width = 20;
    std::cout << "Point BLOB sizes" << std::endl;
    std::cout << std::setw(width) << "Point:" << point->getSize();
    std::cout << std::endl;
    std::cout << std::setw(width) << "PointZ:" << pointZ->getSize();
    std::cout << std::endl;
    std::cout << std::setw(width) << "PointM:" << pointM->getSize();
    std::cout << std::endl;
    std::cout << std::setw(width) << "PointZM:" << pointZM->getSize();
    std::cout << std::endl;

    std::cout << "Line BLOB sizes" << std::endl;
    std::cout << std::setw(width) << "Line #1:" << line1->getSize();
    std::cout << std::endl;
    std::cout << std::setw(width) << "Line #2:" << line2->getSize();
    std::cout << std::endl;

    std::cout << "Various BLOB sizes" << std::endl;
    std::cout << std::setw(width) << "Compressed:" << compressed->getSize();
    std::cout << std::endl;
    std::cout << std::setw(width) << "FGF:" << fgf->getSize();
    std::cout << std::endl;
    std::cout << std::setw(width) << "HEX:" << hex->getSize();
    std::cout << std::endl;
    std::cout << std::setw(width) << "SpatiaLite:" << splite->getSize();
    std::cout << std::endl;
    std::cout << std::setw(width) << "WKB:" << wkb->getSize();
    std::cout << std::endl;

    return 0;

}

/**
 * Create various geometry data structures
 * @returns 0 if success otherwise failure
 */
int exCreateGeometries()
{

    std::cout << "========================================" << std::endl;
    std::cout << "Create various geometry data structures" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // Create point
    PointPtr point(new Point(gaiaAllocPoint(1.5, 2.75)));

    // Create line
    LineStringPtr line(new LineString(gaiaAllocLinestring(5)));
    gaiaSetPoint(line->get()->Coords, 0, 1.0, 1.0);
    gaiaSetPoint(line->get()->Coords, 1, 2.0, 1.0);
    gaiaSetPoint(line->get()->Coords, 2, 2.0, 2.0);
    gaiaSetPoint(line->get()->Coords, 3, 100.0, 2.0);
    gaiaSetPoint(line->get()->Coords, 4, 100.0, 100.0);

    // Create polygon with one interior rings
    PolygonPtr polygon(new Polygon(gaiaAllocPolygon(5, 1)));

    gaiaRingPtr interior = polygon->get()->Exterior;
    gaiaSetPoint(interior->Coords, 0, 0.0, 0.0);
    gaiaSetPoint(interior->Coords, 1, 50.0, 0.0);
    gaiaSetPoint(interior->Coords, 2, 50.0, 50.0);
    gaiaSetPoint(interior->Coords, 3, 0.0, 50.0);
    gaiaSetPoint(interior->Coords, 4, 0.0, 0.0);

    interior = gaiaAddInteriorRing(polygon->get(), 0, 5);
    gaiaSetPoint(interior->Coords, 0, 40.0, 40.0);
    gaiaSetPoint(interior->Coords, 1, 41.0, 40.0);
    gaiaSetPoint(interior->Coords, 2, 41.0, 41.0);
    gaiaSetPoint(interior->Coords, 3, 40.0, 41.0);
    gaiaSetPoint(interior->Coords, 4, 40.0, 40.0);

    // Create ring
    RingPtr ring(new Ring(gaiaAllocRing(5)));
    gaiaSetPoint(ring->get()->Coords, 0, 0.0, 0.0);
    gaiaSetPoint(ring->get()->Coords, 1, 50.0, 0.0);
    gaiaSetPoint(ring->get()->Coords, 2, 50.0, 50.0);
    gaiaSetPoint(ring->get()->Coords, 3, 0.0, 50.0);
    gaiaSetPoint(ring->get()->Coords, 4, 0.0, 0.0);

    // Create dynamic  line
    DynamicLinePtr dynamic(new DynamicLine(gaiaAllocDynamicLine()));
    gaiaAppendPointMToDynamicLine(dynamic->get(), 0, 0, 0);
    gaiaAppendPointMToDynamicLine(dynamic->get(), 1, 0, 0);
    gaiaAppendPointMToDynamicLine(dynamic->get(), 2, 2, 0);
    gaiaAppendPointMToDynamicLine(dynamic->get(), 0, 1, 0);

    // Print out geometry summary
    std::cout << "Point" << std::endl;
    std::cout << "    X = " << std::setw(3) << point->get()->X;
    std::cout << "    Y = " << std::setw(3) << point->get()->Y << std::endl;

    std::cout << "Line" << std::endl;
    for (int i = 0; i < line->get()->Points; i++)
    {
        double x, y;
        gaiaGetPoint(line->get()->Coords, i, &x, &y);
        std::cout << "    X = " << std::setw(3) << x;
        std::cout << "    Y = " << std::setw(3) << y << std::endl;
    }

    std::cout << "Polygon" << std::endl;
    std::cout << "    Exterior" << std::endl;
    for (int i = 0; i < polygon->get()->Exterior->Points; i++)
    {
        double x, y;
        gaiaGetPoint(polygon->get()->Exterior->Coords, i, &x, &y);
        std::cout << "        X = " << std::setw(3) << x;
        std::cout << "        Y = " << std::setw(3) << y << std::endl;
    }
    std::cout << "    Interior" << std::endl;
    for (int i = 0; i < polygon->get()->Interiors->Points; i++)
    {
        double x, y;
        gaiaGetPoint(polygon->get()->Interiors->Coords, i, &x, &y);
        std::cout << "        X = " << std::setw(3) << x;
        std::cout << "        Y = " << std::setw(3) << y << std::endl;
    }

    std::cout << "Ring" << std::endl;
    for (int i = 0; i < ring->get()->Points; i++)
    {
        double x, y;
        gaiaGetPoint(ring->get()->Coords, i, &x, &y);
        std::cout << "    X = " << std::setw(3) << x;
        std::cout << "    Y = " << std::setw(3) << y << std::endl;
    }

    std::cout << "Dynamic Line" << std::endl;
    gaiaPointPtr pt = dynamic->get()->First;
    do
    {
        std::cout << "    X = " << std::setw(3) << pt->X;
        std::cout << "    Y = " << std::setw(3) << pt->Y << std::endl;
        pt = pt->Next;
    } while (pt != 0);

    return 0;

}

/**
 * Read DBF file
 * @returns 0 if success otherwise failure
 */
int exDbfFile(std::string filename)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Read DBF file" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    DbfPtr dbf(new Dbf(gaiaAllocDbf()));
    gaiaDbfPtr pdbf = dbf->get();

    // Open DBF file
    gaiaOpenDbfRead(pdbf, filename.c_str(), "UTF-8", "UTF-8");
    if (pdbf->Valid == 0)
    {
        std::cout << "Failed to read DBF file" << std::endl;
        return 1;
    }
    std::cout << "DBF Record Length: " << pdbf->DbfReclen << std::endl;

    // Read first entity
    int deleted = 0;
    int status = gaiaReadDbfEntity(pdbf, 0, &deleted);
    if (status == 0)
    {
        std::cerr << "Failed to read 1st entity" << std::endl;
        return 2;
    }

    // Print source DBF fields
    std::cout << "Source DBF fields and their clones." << std::endl;
    gaiaDbfFieldPtr field = pdbf->Dbf->First;
    while (field)
    {

        // Copy field
        DbfFieldPtr copy(new DbfField(gaiaCloneDbfField(field)));

        std::cout << "    Source Field: " << std::setw(10) << field->Name;
        std::cout << "    Cloned Field: " << std::setw(10) << copy->get()->Name;
        std::cout << std::endl;

        // Go to next field
        field = field->Next;

    }

    // Clone DBF List
    DbfListPtr list(new DbfList(gaiaCloneDbfEntity(pdbf->Dbf)));

    // Print clone DBF fields
    std::cout << "Clone DBF fields and their clones." << std::endl;
    gaiaDbfListPtr plist = list->get();
    field = plist->First;
    while (field)
    {

        // Copy field
        DbfFieldPtr copy(new DbfField(gaiaCloneDbfField(field)));

        std::cout << "    Source Field: " << std::setw(10) << field->Name;
        std::cout << "    Cloned Field: " << std::setw(10) << copy->get()->Name;
        std::cout << std::endl;

        // Go to next field
        field = field->Next;

    }

    return 0;

}

/**
 * Print EXIF tags in JPEG image
 * @param[in] filename Source JPEG file with EXIF tags
 * @returns 0 if success otherwise failure
 */
int exExifTag(std::string const & filename)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Summary of EXIF tags" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // ==================================================
    // Read JPEG image as blob
    // --------------------------------------------------
    std::ifstream file(filename.c_str(), std::ifstream::binary);
    std::filebuf * buffer = file.rdbuf();
    std::size_t size = buffer->pubseekoff (0, file.end, file.in);
    buffer->pubseekpos(0, file.in);
    char * blob = new char[size];
    buffer->sgetn(blob, size);
    file.close();

    // ==================================================
    // Print EXIF tags
    // --------------------------------------------------
    ExifTagListPtr tags(new ExifTagList((const unsigned char *)blob, size));

    int count = 0;
    gaiaExifTagPtr tag = tags->get()->First;
    while (tag)
    {
        std::string type = "UNKNOWN";
        switch (gaiaExifTagGetValueType(tag))
        {
        case GAIA_EXIF_NONE      : {type = "NONE";      break;}
        case GAIA_EXIF_BYTE      : {type = "BYTE";      break;}
        case GAIA_EXIF_SHORT     : {type = "SHORT";     break;}
        case GAIA_EXIF_STRING    : {type = "STRING";    break;}
        case GAIA_EXIF_LONG      : {type = "LONG";      break;}
        case GAIA_EXIF_RATIONAL  : {type = "RATIONAL";  break;}
        case GAIA_EXIF_SLONG     : {type = "SLONG";     break;}
        case GAIA_EXIF_SRATIONAL : {type = "SRATIONAL"; break;}
        }
        char name[255];
        gaiaExifTagGetName(tag, name, 255);
        std::cout << "Type: " << std::setw(10) << type << "    ";
        std::cout << "Name: " << name << std::endl;

        count++;
        tag = tag->Next;
    }
    std::cout << "Tag Count: " << count << std::endl;

    // ==================================================
    // Clean up resources
    // --------------------------------------------------
    delete[] blob;

    return 0;

}

/**
 * Merge all geometries in a database table into a single collection
 * @param[in]  db         Source database
 * @param[in]  table      Database table to query
 * @param[in]  name       Item name column header name
 * @param[in]  geometry   Table geometry column header name
 * @param[out] collection Destination geometry collection
 * @returns 0 if success otherwise failure
 */
int exMergeGeometries(SpatialDatabase const & db,
                      std::string const & table,
                      std::string const & name,
                      std::string const & geometry,
                      GeometryCollectionPtr & collection)
{

    int numGeometries = db.getCount(table);
    for (int i = 0; i < numGeometries; i++)
    {

        SQLite::Column namei = getColumn(db, table, name, i);
        SQLite::Column blobi = getColumn(db, table, geometry, i);
        GeometryCollectionPtr collectioni(new GeometryCollection(blobi));

        std::cout << "Merging " << namei << std::endl;
        if (i > 0)
        {
            gaiaGeomCollPtr geometry = gaiaMergeGeometries(collection->get(),
                                                           collectioni->get());

            if (!geometry)
            {
                std::cerr << "Invalid geometry!" << std::endl;
                continue;
            }

            // Swap collections
            GeometryCollectionPtr collectionj(new GeometryCollection(geometry));
            GeometryCollectionPtr tempi(collectioni);
            collectioni = collectionj;
            collectionj = tempi;

        }

        // Swap collections
        GeometryCollectionPtr temp(collection);
        collection = collectioni;
        collectioni = temp;

    }

    return 0;

}

/**
 * Read Shapefile
 * @param[in] filename Shapefile path
 * @param[in] srid SRID code
 * @returns 0 if success otherwise failure
 */
int exShapeFile(std::string filename, int srid)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Read Shapefile" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    ShapefilePtr shapefile(new Shapefile(gaiaAllocShapefile()));
    gaiaShapefilePtr pshp = shapefile->get();

    // Open Shapefile
    gaiaOpenShpRead(pshp, filename.c_str(), "UTF-8", "UTF-8");
    if (pshp->Valid == 0)
    {
        std::cout << "Failed to read Shapefile" << std::endl;
        return 1;
    }

    // Read shapes until EOF
    int count = 0;
    while (true)
    {
        int status = gaiaReadShpEntity(pshp, count, srid);
        if (status == 0) break;
        count++;
    }
    std::cout << "Number of Shapes: " << count << std::endl;

    return 0;

}

/**
 * Print summary of database table
 * @param[in] db    Source database
 * @param[in] table Database table to query
 * @returns 0 if success otherwise failure
 */
int exSummary(SpatialDatabase const & db,
              std::string const & table)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Summary of SpatialDatabase database" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // ==================================================
    // Print table size
    // --------------------------------------------------
    std::cout << "Number of records = " << db.getCount(table) << std::endl;

    // ==================================================
    // Print table headers
    // --------------------------------------------------
    std::vector<std::string> headers =  db.getHeaders(table);
    for (unsigned int i = 0; i < headers.size(); i++)
    {
        std::cout << headers.at(i) << ", ";
    }
    std::cout << std::endl;

    // ==================================================
    // Print table types
    // --------------------------------------------------
    std::vector<std::string> types =  db.getTypes(table);
    for (unsigned int i = 0; i < types.size(); i++)
    {
        std::cout << types.at(i) << ", ";
    }
    std::cout << std::endl;

    return 0;

}

/**
 * Find number of geometries that touch each geometry in table
 * @param[in] db       Source database
 * @param[in] table    Database table to query
 * @param[in] name     Item name column header name
 * @param[in] geometry Table geometry column header name
 * @returns 0 if success otherwise failure
 */
int exVirtualShapeTouch(SpatialDatabase const & db,
                        std::string const & table,
                        std::string const & name,
                        std::string const & geometry)
{

    std::cout << "========================================" << std::endl;
    std::cout << "SpatialDatabase touch predicate" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // ==================================================
    // Get table values
    // --------------------------------------------------
    int numGeometries = db.getCount(table);
    for (int i = 0; i < numGeometries; i++)
    {

        SQLite::Column namei = getColumn(db, table, name, i);
        SQLite::Column blobi = getColumn(db, table, geometry, i);
        GeometryCollectionPtr collectioni(new GeometryCollection(blobi));

        int numTouches = 0;
        for (int j = i+1; j < numGeometries; j++)
        {

            SQLite::Column namej = getColumn(db, table, name, j);
            SQLite::Column blobj = getColumn(db, table, geometry, j);
            GeometryCollectionPtr collectionj(new GeometryCollection(blobj));

            bool touches = gaiaGeomCollTouches(collectioni->get(),
                                               collectionj->get()) != 0;
            if (touches) numTouches++;

        }

        std::cout << std::setw(20)<< namei << ": " << numTouches << std::endl;

    }

    return 0;

}

/**
 * Print summary of Vector Layer List
 * @param[in] db Source database
 * @returns 0 if success otherwise failure
 */
int exVectorList(SpatialDatabase const & db)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Printing summary of vector layer list " << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // ==================================================
    // Extracting layers in list
    // --------------------------------------------------
    gaiaVectorLayersListPtr pl = gaiaGetVectorLayersList(
        db.getDatabase()->getHandle(), 0, 0, GAIA_VECTORS_LIST_FAST);
    VectorLayersListPtr list(new VectorLayersList(pl));
    if (!list)
    {
        throw std::runtime_error("Bad vector layer list in database");
    }

    // ==================================================
    // Print out layers in list
    // --------------------------------------------------

    gaiaVectorLayerPtr layer = list->get()->First;

    int count = 0;
    while (layer)
    {
        count++;
        std::cout << "Layer Name: " << layer->TableName << std::endl;
        layer = layer->Next;
    }
    std::cout << "# Layers Found = " << count << std::endl;

    return 0;

}

/**
 * Add virtual shape from shapefile to database
 * @param[in] db       Source database
 * @param[in] table    Database table to query
 * @param[in] filename Source shapefile filename
 * @param[in] encoding Encoding of filename
 * @param[in] srid     Shapefile SRID
 * @returns 0 if success otherwise failure
 */
int exVirtualShape(SpatialDatabase const & db,
                   std::string const & table,
                   std::string const & filename,
                   std::string const & encoding,
                   std::string const & srid)
{

    // ==================================================
    // Add virtual shapefile to database table
    // --------------------------------------------------
    std::string sql = "CREATE VIRTUAL TABLE '" + table +
                      "' USING VirtualShape('" + filename + "', '" + encoding +
                      "', " + srid + ");";
    db.getDatabase()->exec(sql);

    // ==================================================
    // Summary of virtual shapefile
    // --------------------------------------------------
    exSummary(db, table);

    return 0;

}

/**
 * Copy virtual shape from from virtual table to a real table
 * @param[in] db       Source database
 * @param[in] table    Virtual table to create
 * @param[in] filename Source shapefile filename
 * @param[in] encoding Encoding of filename
 * @param[in] srid     Shapefile SRID
 * @param[in] target   Real table to create
 * @returns 0 if success otherwise failure
 */
int exVirtualShapeCopy(SpatialDatabase const & db,
                       std::string const & table,
                       std::string const & filename,
                       std::string const & encoding,
                       std::string const & srid,
                       std::string const & target)
{

    // ==================================================
    // Drop tables if they exists
    // --------------------------------------------------
    std::string sql = "DROP TABLE IF EXISTS " + table + ";";
    db.getDatabase()->exec(sql);

    sql = "DROP TABLE IF EXISTS " + target + ";";
    db.getDatabase()->exec(sql);

    // ==================================================
    // Add virtual shapefile to database table
    // --------------------------------------------------
    sql = "CREATE VIRTUAL TABLE '" + table +
          "' USING VirtualShape('" + filename + "', '" + encoding +
          "', " + srid + ");";
    db.getDatabase()->exec(sql);

    // ==================================================
    // Create new table
    // --------------------------------------------------
    sql = "CREATE TABLE " + target + " AS SELECT * FROM " + table + ";";
    db.getDatabase()->exec(sql);

    // ==================================================
    // Drop virtual table
    // --------------------------------------------------
    sql = "DROP TABLE " + table + ";";
    db.getDatabase()->exec(sql);

    // ==================================================
    // Print summary of created table
    // --------------------------------------------------
    exSummary(db, target);

    return 0;

}

/**
 * Open a WFS catalog.
 * @param[in] filename Path to WFS schema
 * @returns 0 if success otherwise failure
 */
int exWfsCatalog(std::string filename)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Open a WFS catalog" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    std::string error;

    // Try to open a non-existant catalog
    try
    {
        std::cout << "Try to open non-existant catalog." << std::endl;
        WfsCatalogPtr catalog(WfsCatalog::create(""));
    }
    catch (std::exception const & error)
    {
        std::cout << error.what() << std::endl;
        std::cout << "Did not open non-existant catalog." << std::endl;
    }

    // Try to open a real catalog
    try
    {
        std::cout << "Try to open existant catalog." << std::endl;
        WfsCatalogPtr catalog(WfsCatalog::create(filename.c_str()));
        int count = get_wfs_catalog_count(catalog->get());
        std::cout << "Catalog count = " << count << std::endl;
    }
    catch (std::exception const & error)
    {
        std::cerr << error.what() << std::endl;
    }

    return 0;

}

/**
 * Open a WFS catalog.
 * @returns 0 if success otherwise failure
 */
int exWfsCatalogLoad()
{

    std::cout << "========================================" << std::endl;
    std::cout << "Load a WFS catalog" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1);");

    std::string path    = "wfs/test.wfs";
    char * uri          = 0;
    std::string layer   = "topp:p02";
    int swap            = 0;
    std::string table   = "test_wfs1";
    std::string primary = "objectid";
    int spatial         = 1;
    int rows            = 0;

    try
    {
        WfsCatalog::load(db,
                         path.c_str(),
                         uri,
                         layer.c_str(),
                         swap,
                         table.c_str(),
                         primary.c_str(),
                         spatial,
                         &rows,
                         0,
                         0);
    }
    catch (std::runtime_error const & error)
    {
        std::cerr << error.what() << std::endl;
        return 1;
    }
    std::cout << "Number of rows loaded = " << rows << std::endl;

    return 0;

}

/**
 * Open a WFS schema.
 * @param[in] filename Path to WFS schema
 * @param[in] layer WFS schema layer
 * @returns 0 if success otherwise failure
 */
int exWfsSchema(std::string const & filename, std::string const & layer)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Open a WFS schema" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // Try to open a non-existant schema
    try
    {
        std::cout << "Try to open non-existant schema." << std::endl;
        WfsSchemaPtr badSchema(WfsSchema::create("", ""));
    }
    catch (std::exception const & error)
    {
        std::cout << error.what() << std::endl;
        std::cout << "Did not open non-existant schema." << std::endl;
    }

    // Try to open a real schema
    try
    {
        std::cout << "Try to open existant schema." << std::endl;
        WfsSchemaPtr schema(WfsSchema::create(filename.c_str(),
                                              layer.c_str()));
        int count = get_wfs_schema_column_count(schema->get());
        std::cout << "Schema column count = " << count << std::endl;
    }
    catch (std::exception const & error)
    {
        std::cerr << error.what() << std::endl;
    }

    return 0;

}

/**
 * Write database geometries to various spatial file formats (GeoJSON, GML, KML,
 * WKT). Output files will be written to "table.ex.kml", "table.ex.json", etc.
 * @param[in] db       Source database
 * @param[in] table    Database table to query
 * @param[in] name     Item name column header name
 * @param[in] geometry Table geometry column header name
 * @returns 0 if success otherwise failure
 */
int exWriteFormats(SpatialDatabase const & db,
                   std::string const & table,
                   std::string const & name,
                   std::string const & geometry)
{

    std::cout << "========================================" << std::endl;
    std::cout << "Writing SpatialDatabase to file formats" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // ==================================================
    // Merge all geometries
    // --------------------------------------------------
    gaiaGeomCollPtr gc = gaiaAllocGeomColl();
    GeometryCollectionPtr collection(new GeometryCollection(gc));
    exMergeGeometries(db, table, name, geometry, collection);
    if (!collection)
    {
        throw std::runtime_error("Invalid collection geometry!");
    }

    // ==================================================
    // Write geometry to various formats
    // --------------------------------------------------
    std::cout << "Writing merged collection " << std::endl;

    // Write geometry to GeoJSON
    OutputBufferPtr geojson(new OutputBuffer(new gaiaOutBuffer()));
    gaiaOutGeoJSON(geojson->get(), collection->get(), 6, 5);
    if (geojson->isValid())
    {
        std::string filename = table + ".ex.json";
        std::ofstream file(filename.c_str());
        file << geojson->get()->Buffer << std::endl;
    }

    // Write geometry to GML
    OutputBufferPtr gml(new OutputBuffer(new gaiaOutBuffer()));
    gaiaOutGml(gml->get(), 2, 6, collection->get());
    if (gml->isValid())
    {
        std::string filename = table + ".ex.gml";
        std::ofstream file(filename.c_str());
        file << gml->get()->Buffer << std::endl;
    }

    // Write geometry to KML
    OutputBufferPtr kml(new OutputBuffer(new gaiaOutBuffer()));
    gaiaOutFullKml(kml->get(), "test", "", collection->get(), 6);
    if (kml->isValid())
    {
        std::string filename = table + ".ex.kml";
        std::ofstream file(filename.c_str());
        file << kml->get()->Buffer << std::endl;
    }

    // Write geometry to WKT
    OutputBufferPtr wkt(new OutputBuffer(new gaiaOutBuffer()));
    gaiaOutWkt(wkt->get(), collection->get());
    if (wkt->isValid())
    {
        std::string filename = table + ".ex.wkt";
        std::ofstream file(filename.c_str());
        file << wkt->get()->Buffer << std::endl;
    }

    return 0;

}

/**
 * Load and save XML document
 * @returns 0 if success otherwise failure
 */
int exXmlBlob()
{

    std::cout << "========================================" << std::endl;
    std::cout << "Load and save XML document" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

    // ==================================================
    // Load XML to buffer
    // --------------------------------------------------
    std::string error;
    std::string path = "xml/books.xml";
    XmlDocumentPtr xml(XmlBlob::load(path));
    if (!xml)
    {
        std::cerr << "Failed to create XML buffer" << std::endl;
    }

    // ==================================================
    // Convert XML to Blob
    // --------------------------------------------------
    XmlBlobPtr blob(XmlBlob::toBlob(db, *xml, 1, "xml/books.xsd"));

    // Get XML as string
    std::string text = blob->textFromBlob(4);
    std::cout << text << std::endl;

    // ==================================================
    // Save to file
    // --------------------------------------------------
    blob->store("store.xml", 4);

    return 0;

}

int examples()
{
    int status = 0;

    try
    {

        // ==================================================
        // Define some constants
        // --------------------------------------------------
        const int code             = 4326;
        const std::string encoding = "UTF-8";
        const std::string fileexif = "spatialite/DSC_1467.JPG";
        const std::string filedbf  = "states/states.dbf";
        const std::string filesch  = "wfs/describefeaturetype.wfs";
        const std::string fileshp  = "states/states";
        const std::string filewfs  = "wfs/getcapabilities-1.0.0.wfs";
        const std::string geometry = "Geometry";
        const std::string layersch = "sf:roads";
        const std::string name     = "STATE_NAME";
        const std::string srid     = "4326";
        const std::string table    = "states";
        const std::string target   = "states_copy";

        // ==================================================
        // Define various databases
        // --------------------------------------------------
        SpatialDatabase db("states/db.sqlite");
        SpatialDatabase list("spatialite/gpkg_test.sqlite");
        SpatialDatabase shape(":memory:", SQLITE_OPEN_READWRITE);
        SpatialDatabase shapecopy("copy.sqlite",
                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

        // ==================================================
        // Run various examples
        // --------------------------------------------------

        // Run auxillary methods
        exAuxillary();

        // Check database
        exCheckDatabase(db, table, name, geometry);

        // Compute checksum of geometries
        exChecksum(db, table, name, geometry);

        // Convert geometry names to UTF8
        exConvertUtf8(db, table, name);

        // Create geometry blobs
        exCreateBlobs();

        // Create various geometry data structures
        exCreateGeometries();

        // Read DBF file
        exDbfFile(filedbf);

        // Read EXIF tags
        exExifTag(fileexif);

        // Read Shapefile
        exShapeFile(fileshp, code);

        // Summary of file database
        exSummary(db, table);

        // Print summary of vector list
        exVectorList(list);

        // Summary of virtual shapefile
        exVirtualShape(shape, table, fileshp, encoding, srid);

        // Copy virtual shapefile to database
        exVirtualShapeCopy(shapecopy, table, fileshp, encoding, srid, target);

        // Computing touch spatial predicate
        exVirtualShapeTouch(shapecopy, target, name, geometry);

        // Open WFS catalog
        exWfsCatalog(filewfs);

        // Load WFS catalog to database
        exWfsCatalogLoad();

        // Open WFS schema
        exWfsSchema(filesch, layersch);

        // Write file to various spatial formats
        exWriteFormats(db, table, name, geometry);

        // Create XML Blobs
        exXmlBlob();

    }
    catch (std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;

}

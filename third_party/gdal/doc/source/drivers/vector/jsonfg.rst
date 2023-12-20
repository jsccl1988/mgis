.. _vector.jsonfg:

JSONFG -- OGC Features and Geometries JSON
==========================================

.. versionadded:: 3.8

.. shortname:: JSONFG

.. built_in_by_default::

This driver implements read/write support for access to features encoded
in `OGC Features and Geometries JSON - Part 1: Core <https://docs.ogc.org/DRAFTS/21-045.html>`__ format.

JSON-FG is a set of extensions over the :ref:`GeoJSON<vector.geojson>` format.

The following extensions are supported:

- capturing the feature type in a ``featureType`` element to affect features
  to separate
- capturing a coordinate reference system (not necessarily WGS 84) in a
  ``coordRefSys`` element, that is the one used by geometries written in the
  ``place`` element
- ``time`` element at Feature level
- minimum support for Polyhedron geometries (with a single outer shell)
  and Prism with Point, LineString or Polygon base.

.. warning::

    This driver implements a draft version of the standard.

Driver capabilities
-------------------

.. supports_create::

.. supports_georeferencing::

.. supports_virtualio::

Connection string
-----------------

The JSON-FG driver accepts three types of sources of data:

-  Uniform Resource Locator (`URL <http://en.wikipedia.org/wiki/URL>`__)
   - a Web address to perform
   `HTTP <http://en.wikipedia.org/wiki/HTTP>`__ request
-  Plain text file with JSON-FG data, typically with a .json extension
-  Text passed directly and encoded in JSON-FG

The URL/filename/text might be prefixed with
``JSONFG:`` to avoid any ambiguity with other drivers.

Time support
------------

On reading:

- Feature.time.date elements are mapped to a OGR ``time`` field of
  type Date. If there is a property also called ``time``, the OGR field for
  Feature.time is called ``jsonfg_time``

- Feature.time.timestamp elements are mapped to a OGR ``time`` field of
  type DateTime

- Feature.time.interval elements are mapped to a OGR ``time_start`` and
  ``time_end`` fields of type Date or DateTime. If there are properties
  with those names, the OGR fields for Feature.time are prefixed with ``jsonfg_``
  Unbounded intervals (``..``) are mapped to a unset OGR feature field.

On writing, if fields called ``time``, ``time_start``,  ``time_end`` of type
Date or DateTime are created, their content is used to set the Feature.time
element with the reverse logic as the above explained reading side.
The field names may also be prefixed with ``jsonfg_`` to distinguish them from
fields ``time``, ``time_start``, ``time_end`` that must be written in
Feature.properties

Configuration options
---------------------

The following :ref:`configuration options <configoptions>` are
available:

-  .. config:: OGR_JSONFG_MAX_OBJ_SIZE
      :choices: <MBytes>
      :default: 200

      size in MBytes of the maximum accepted single feature,
      or 0 to allow for a unlimited size (GDAL >= 3.5.2).

Open options
------------

-  .. oo:: GEOMETRY_ELEMENT
      :choices: AUTO, PLACE, GEOMETRY
      :default: AUTO

      Determines which JSON element to use to create geometry from.
      By default when the ``place`` element is set, geometries are created from
      its content. And when it is absent or null, the content of the ``geometry``
      element is used.
      Setting this option to ``PLACE`` restricts the reading to the ``place``
      elements.
      Setting this option to ``GEOMETRY`` restricts the reading to the ``geometry``
      elements.

Dataset creation options
------------------------

-  .. lco:: SINGLE_LAYER
      :choices: YES, NO
      :default: NO

      Whether a single layer will be written. If set to YES, the ``coordRefSys``
      and ``featureType`` elements can be written at the FeatureCollection
      level instead inside each Feature.

Layer creation options
----------------------

-  .. lco:: COORDINATE_PRECISION_GEOMETRY
      :choices: <integer>
      :default: 7

      Maximum number of figures after decimal separator to write in coordinates
      of ``geometry`` element.

-  .. lco:: COORDINATE_PRECISION_PLACE
      :choices: <integer>

      Maximum number of figures after decimal separator to write in coordinates
      of ``place`` element.

-  .. lco:: SIGNIFICANT_FIGURES
      :choices: <integer>
      :default: 17

      Maximum number of
      significant figures when writing floating-point numbers.
      If explicitly specified, and :lco:`COORDINATE_PRECISION_GEOMETRY` or
      :lco:`COORDINATE_PRECISION_PLACE` is not, this
      will also apply to coordinates.

-  .. lco:: WRITE_GEOMETRY
      :choices: YES, NO
      :default: YES

      By default, when a non-WGS 84 CRS is assigned to the layer, the geometry
      is written in the ``place`` element, and a reprojected version of it
      to WGS 84 is written in the ``geometry`` element.
      This option can be set to NO to avoid writing the ``geometry`` element
      when ``place`` is written.
      Setting this option to NO has no effect if the layer CRS is WGS 84,
      in which case geometries must be written in ``geometry``.

-  .. lco:: ID_FIELD

      Name of the source field that
      must be written as the 'id' member of Feature objects.

-  .. lco:: ID_TYPE
      :choices: AUTO, String, Integer

      Type of the 'id' member of Feature objects.

-  .. lco:: ID_GENERATE
      :choices: YES, NO

      Auto-generate feature ids

VSI Virtual File System API support
-----------------------------------

The driver supports reading and writing to files managed by VSI Virtual
File System API, which includes "regular" files, as well as files in the
/vsizip/ (read-write), /vsigzip/ (read-write), /vsicurl/ (read-only)
domains.

Writing to /dev/stdout or /vsistdout/ is also supported.

See Also
--------

-  `OGC Features and Geometries JSON - Part 1: Core <https://docs.ogc.org/DRAFTS/21-045.html>`__
-  :ref:`GeoJSON driver <vector.geojson>`

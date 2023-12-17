#!/usr/bin/env bash

MASON_NAME=gdal
MASON_VERSION=2.1.3
MASON_LIB_FILE=lib/libgdal.a

. ${MASON_DIR}/mason.sh

function mason_load_source {
    mason_download \
        http://download.osgeo.org/gdal/${MASON_VERSION}/gdal-${MASON_VERSION}.tar.gz \
        552231f8ffe060ba30e37f1a8e6c4665bcf3cd1d

    mason_extract_tar_gz

    export MASON_BUILD_PATH=${MASON_ROOT}/.build/${MASON_NAME}-${MASON_VERSION}
}

function mason_prepare_compile {
    cd $(dirname ${MASON_ROOT})
    # set up to fix libtool .la files
    # https://github.com/mapbox/mason/issues/61
    if [[ $(uname -s) == 'Darwin' ]]; then
        FIND="\/Users\/travis\/build\/mapbox\/mason"
    else
        FIND="\/home\/travis\/build\/mapbox\/mason"
    fi
    REPLACE="$(pwd)"
    REPLACE=${REPLACE////\\/}
    ${MASON_DIR}/mason install libtiff 4.0.7
    MASON_TIFF=$(${MASON_DIR}/mason prefix libtiff 4.0.7)
    perl -i -p -e "s/${FIND}/${REPLACE}/g;" ${MASON_TIFF}/lib/libtiff.la
    ${MASON_DIR}/mason install proj 4.9.3
    MASON_PROJ=$(${MASON_DIR}/mason prefix proj 4.9.3)
    perl -i -p -e "s/${FIND}/${REPLACE}/g;" ${MASON_PROJ}/lib/libproj.la
    ${MASON_DIR}/mason install jpeg_turbo 1.5.1
    MASON_JPEG=$(${MASON_DIR}/mason prefix jpeg_turbo 1.5.1)
    perl -i -p -e "s/${FIND}/${REPLACE}/g;" ${MASON_JPEG}/lib/libjpeg.la
    ${MASON_DIR}/mason install libpng 1.6.28
    MASON_PNG=$(${MASON_DIR}/mason prefix libpng 1.6.28)
    perl -i -p -e "s/${FIND}/${REPLACE}/g;" ${MASON_PNG}/lib/libpng.la
    ${MASON_DIR}/mason install expat 2.2.0
    MASON_EXPAT=$(${MASON_DIR}/mason prefix expat 2.2.0)
    perl -i -p -e "s/${FIND}/${REPLACE}/g;" ${MASON_EXPAT}/lib/libexpat.la
    ${MASON_DIR}/mason install libpq 9.6.2
    MASON_LIBPQ=$(${MASON_DIR}/mason prefix libpq 9.6.2)
    # depends on sudo apt-get install zlib1g-dev
    ${MASON_DIR}/mason install zlib system
    MASON_ZLIB=$(${MASON_DIR}/mason prefix zlib system)
    # depends on sudo apt-get install libc6-dev
    #${MASON_DIR}/mason install iconv system
    #MASON_ICONV=$(${MASON_DIR}/mason prefix iconv system)
    export LIBRARY_PATH=${MASON_LIBPQ}/lib:${LIBRARY_PATH:-}
    ${MASON_DIR}/mason install ccache 3.3.1
    MASON_CCACHE=$(${MASON_DIR}/mason prefix ccache 3.3.1)/bin/ccache
}

function mason_compile {
    if [[ ${MASON_PLATFORM} == 'linux' ]]; then
        mason_step "Loading patch"
        patch -N -p1 < ${MASON_DIR}/scripts/${MASON_NAME}/${MASON_VERSION}/patch.diff
    fi

    # note CFLAGS overrides defaults so we need to add optimization flags back
    export CFLAGS="${CFLAGS} -O3 -DNDEBUG"
    export CXXFLAGS="${CXXFLAGS} -O3 -DNDEBUG"

    CUSTOM_LIBS="-L${MASON_TIFF}/lib -ltiff -L${MASON_JPEG}/lib -ljpeg -L${MASON_PROJ}/lib -lproj -L${MASON_PNG}/lib -lpng -L${MASON_EXPAT}/lib -lexpat"
    CUSTOM_CFLAGS="${CFLAGS} -I${MASON_LIBPQ}/include -I${MASON_TIFF}/include -I${MASON_JPEG}/include -I${MASON_PROJ}/include -I${MASON_PNG}/include -I${MASON_EXPAT}/include"

    # very custom handling for libpq/postgres support
    # forcing our portable static library to be used
    MASON_LIBPQ_PATH=${MASON_LIBPQ}/lib/libpq.a
    if [[ $(uname -s) == 'Linux' ]]; then
        # on Linux passing -Wl will lead to libtool re-positioning libpq.a in the wrong place (no longer after libgdal.a)
        # which leads to unresolved symbols
        CUSTOM_LDFLAGS="${LDFLAGS} ${MASON_LIBPQ_PATH}"
    else
        # on OSX not passing -Wl will break libtool archive creation leading to confusing arch errors
        CUSTOM_LDFLAGS="${LDFLAGS} -Wl,${MASON_LIBPQ_PATH}"
    fi
    # we have to remove -lpq otherwise it will trigger linking to system /usr/lib/libpq
    perl -i -p -e "s/\-lpq //g;" configure
    # on linux -Wl,/path/to/libpq.a still does not work for the configure test
    # so we have to force it into LIBS. But we don't do this on OS X since it breaks libtool archive logic
    if [[ $(uname -s) == 'Linux' ]]; then
        CUSTOM_LIBS="${MASON_LIBPQ}/lib/libpq.a -pthread ${CUSTOM_LIBS}"
    fi

    export CXX="${MASON_CCACHE} ${CXX}"

    # note: we put ${STDLIB_CXXFLAGS} into CXX instead of LDFLAGS due to libtool oddity:
    # http://stackoverflow.com/questions/16248360/autotools-libtool-link-library-with-libstdc-despite-stdlib-libc-option-pass
    if [[ $(uname -s) == 'Darwin' ]]; then
        export CXX="${CXX} -stdlib=libc++ -std=c++11"
    fi

    # note: it might be tempting to build with --without-libtool
    # but I find that will only lead to a shared libgdal.so and will
    # not produce a static library even if --enable-static is passed
    LIBS="${CUSTOM_LIBS}" LDFLAGS="${CUSTOM_LDFLAGS}" CFLAGS="${CUSTOM_CFLAGS}" ./configure \
        --enable-static --disable-shared \
        ${MASON_HOST_ARG} \
        --prefix=${MASON_PREFIX} \
        --with-libz=${MASON_ZLIB} \
        --disable-rpath \
        --with-libjson-c=internal \
        --with-geotiff=internal \
        --with-expat=${MASON_EXPAT} \
        --with-threads=yes \
        --with-fgdb=no \
        --with-rename-internal-libtiff-symbols=no \
        --with-rename-internal-libgeotiff-symbols=no \
        --with-hide-internal-symbols=yes \
        --with-libtiff=${MASON_TIFF} \
        --with-jpeg=${MASON_JPEG} \
        --with-png=${MASON_PNG} \
        --with-pg=${MASON_LIBPQ}/bin/pg_config \
        --with-static-proj4=${MASON_PROJ} \
        --with-spatialite=no \
        --with-geos=no \
        --with-sqlite3=no \
        --with-curl=no \
        --with-xml2=no \
        --with-pcraster=no \
        --with-cfitsio=no \
        --with-odbc=no \
        --with-libkml=no \
        --with-pcidsk=no \
        --with-jasper=no \
        --with-gif=no \
        --with-grib=no \
        --with-freexl=no \
        --with-avx=no \
        --with-sse=no \
        --with-perl=no \
        --with-ruby=no \
        --with-python=no \
        --with-java=no \
        --with-podofo=no \
        --with-pam \
        --with-webp=no \
        --with-pcre=no \
        --with-liblzma=no \
        --with-netcdf=no \
        --with-poppler=no

    make -j${MASON_CONCURRENCY}
    make install

    relativize_gdal_config ${MASON_PREFIX}/bin/gdal-config ${MASON_PREFIX} ${MASON_ROOT}/${MASON_PLATFORM_ID}

}

function relativize_gdal_config() {
    path_to_gdal_config=${1}
    prefix_path=${2}
    build_path=${3}
    RESOLVE_SYMLINK="readlink"
    if [[ $(uname -s) == 'Linux' ]];then
        RESOLVE_SYMLINK="readlink -f"
    fi
    mv ${path_to_gdal_config} /tmp/gdal-config-backup
    # append code at start
    echo 'if test -L $0; then BASE=$( dirname $( '${RESOLVE_SYMLINK}' "$0" ) ); else BASE=$( dirname "$0" ); fi' > ${path_to_gdal_config}
    cat /tmp/gdal-config-backup >> ${path_to_gdal_config}
    chmod +x ${path_to_gdal_config}

    # now modify in place
    python -c "data=open('${path_to_gdal_config}','r').read();open('${path_to_gdal_config}','w').write(data.replace('${prefix_path}','\$( cd \"\$( dirname \${BASE} )\" && pwd )'))"
    # fix the path to dep libs (CONFIG_DEP_LIBS)
    python -c "data=open('${path_to_gdal_config}','r').read();open('${path_to_gdal_config}','w').write(data.replace('${build_path}','\$( cd \"\$( dirname \$( dirname \$( dirname \${BASE}  ) ))\" && pwd )'))"
    # hack to re-add -lpq since otherwise it will not end up in --dep-libs
    python -c "data=open('${path_to_gdal_config}','r').read();open('${path_to_gdal_config}','w').write(data.replace('\$CONFIG_DEP_LIBS','\$CONFIG_DEP_LIBS -lpq'))"
}


function mason_cflags {
    echo "-I${MASON_PREFIX}/include"
}

function mason_ldflags {
    echo $(${MASON_PREFIX}/bin/gdal-config --dep-libs --libs)
}

function mason_clean {
    make clean
}

mason_run "$@"

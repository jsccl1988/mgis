#!/usr/bin/env bash

MASON_NAME=sqlite
MASON_VERSION=3.17.0
MASON_LIB_FILE=lib/libsqlite3.a
MASON_PKGCONFIG_FILE=lib/pkgconfig/sqlite3.pc

SQLITE_FILE_VERSION=3170000

. ${MASON_DIR}/mason.sh

function mason_load_source {
    mason_download \
        https://www.sqlite.org/2017/sqlite-autoconf-${SQLITE_FILE_VERSION}.tar.gz \
        be61c4d45e72a61a014b60e57d002a1877ba9d18

    mason_extract_tar_gz

    export MASON_BUILD_PATH=${MASON_ROOT}/.build/sqlite-autoconf-${SQLITE_FILE_VERSION}
}

function mason_compile {
    # Note: setting CFLAGS overrides the default in sqlite of `-g -O2`
    # hence we add back the preferred optimization
    CFLAGS="-O3 ${CFLAGS} -DNDEBUG" ./configure \
        --prefix=${MASON_PREFIX} \
        ${MASON_HOST_ARG} \
        --enable-static \
        --with-pic \
        --disable-shared \
        --disable-dependency-tracking

    make install -j${MASON_CONCURRENCY}
}

function mason_strip_ldflags {
    shift # -L...
    shift # -lsqlite3
    echo "$@"
}

function mason_ldflags {
    mason_strip_ldflags $(`mason_pkgconfig` --static --libs)
}

function mason_clean {
    make clean
}

mason_run "$@"

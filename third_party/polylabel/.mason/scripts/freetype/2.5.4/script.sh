#!/usr/bin/env bash

MASON_NAME=freetype
MASON_VERSION=2.5.4
MASON_LIB_FILE=lib/libfreetype.a
MASON_PKGCONFIG_FILE=lib/pkgconfig/freetype2.pc

. ${MASON_DIR}/mason.sh

function mason_load_source {
    mason_download \
        http://nongnu.askapache.com/freetype/freetype-${MASON_VERSION}.tar.bz2 \
        0646f7e62a6191affe92270e2544e6011f5227e8

    mason_extract_tar_bz2

    export MASON_BUILD_PATH=${MASON_ROOT}/.build/freetype-${MASON_VERSION}
}

function mason_compile {
    ./configure --prefix=${MASON_PREFIX} \
     --enable-static \
     --disable-shared ${MASON_HOST_ARG} \
     --with-zlib=yes \
     --with-bzip2=no \
     --with-harfbuzz=no \
     --with-png=no \
     --with-quickdraw-toolbox=no \
     --with-quickdraw-carbon=no \
     --with-ats=no \
     --with-fsref=no \
     --with-fsspec=no \

    make -j${MASON_CONCURRENCY}
    make install
}

function mason_ldflags {
    echo "-lfreetype -lz"
}

function mason_clean {
    make clean
}

mason_run "$@"

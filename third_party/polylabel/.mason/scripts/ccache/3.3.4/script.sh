#!/usr/bin/env bash

MASON_NAME=ccache
MASON_VERSION=3.3.4
MASON_LIB_FILE=bin/ccache

. ${MASON_DIR}/mason.sh

function mason_load_source {
    mason_download \
        https://www.samba.org/ftp/ccache/${MASON_NAME}-${MASON_VERSION}.tar.bz2 \
        6a53c7eff8e5f8ada65db5de155821f7deee19fd

    mason_extract_tar_bz2

    export MASON_BUILD_PATH=${MASON_ROOT}/.build/${MASON_NAME}-${MASON_VERSION}
}

function mason_compile {
    # Add optimization flags since CFLAGS overrides the default (-g -O2)
    export CFLAGS="${CFLAGS} -O3 -DNDEBUG"
    ./configure \
        --prefix=${MASON_PREFIX} \
        ${MASON_HOST_ARG} \
        --with-bundled-zlib
    make V=1 -j${MASON_CONCURRENCY}
    make install
}

function mason_ldflags {
    :
}

function mason_cflags {
    :
}

function mason_clean {
    make clean
}

mason_run "$@"

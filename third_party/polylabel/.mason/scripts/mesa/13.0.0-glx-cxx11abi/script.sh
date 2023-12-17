#!/usr/bin/env bash

LIB_VERSION=13.0.0

MASON_NAME=mesa
MASON_VERSION=${LIB_VERSION}-glx-cxx11abi
MASON_LIB_FILE=lib/libGL.so
MASON_PKGCONFIG_FILE=lib/pkgconfig/gl.pc

. ${MASON_DIR}/mason.sh

function mason_load_source {
    mason_download \
        https://mesa.freedesktop.org/archive/${LIB_VERSION}/mesa-${LIB_VERSION}.tar.gz \
        bba4f687bc0b0066961424dd0ae2ca053ffc1fcb

    mason_extract_tar_gz

    export MASON_BUILD_PATH=${MASON_ROOT}/.build/mesa-${LIB_VERSION}
}

function mason_prepare_compile {
    true
}

function mason_compile {
    ./configure \
        --prefix=${MASON_PREFIX} \
        ${MASON_HOST_ARG} \
        --disable-egl \
        --disable-dri \
        --disable-dri3 \
        --disable-driglx-direct \
        --disable-osmesa \
        --disable-gallium-osmesa \
        --enable-opengl \
        --enable-gles1 \
        --enable-gles2 \
        --enable-glx=gallium-xlib \
        --enable-glx-tls \
        --enable-gallium-llvm \
        --enable-texture-float \
        --enable-llvm-shared-libs \
        --with-gallium-drivers=swrast \
        --with-llvm-prefix=/usr/lib/llvm-3.8 \
        --with-sha1=libcrypto

    make
    make install
}

function mason_cflags {
    echo -I${MASON_PREFIX}/include
}

function mason_ldflags {
    echo $(`mason_pkgconfig` --libs)
}

function mason_clean {
    make clean
}

mason_run "$@"

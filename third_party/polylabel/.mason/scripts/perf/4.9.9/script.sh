#!/usr/bin/env bash

MASON_NAME=perf
MASON_VERSION=4.9.9
MASON_LIB_FILE=bin/perf

. ${MASON_DIR}/mason.sh

function mason_load_source {
    # https://www.kernel.org/
    # https://git.kernel.org/cgit/linux/kernel/git/stable/linux-stable.git/log/tools/perf
    mason_download \
        https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-${MASON_VERSION}.tar.xz \
        b45b464dbf36c360fbeb5d73c4648b5f14d92fc9

    mason_extract_tar_xz

    export MASON_BUILD_PATH=${MASON_ROOT}/.build/linux-${MASON_VERSION}
}

function mason_prepare_compile {
    ${MASON_DIR}/mason install zlib 1.2.8
    MASON_ZLIB=$(${MASON_DIR}/mason prefix zlib 1.2.8)
    ${MASON_DIR}/mason install xz 5.2.3
    MASON_XZ=$(${MASON_DIR}/mason prefix xz 5.2.3)
    ${MASON_DIR}/mason install binutils 2.27
    MASON_BINUTILS=$(${MASON_DIR}/mason prefix binutils 2.27)
    ${MASON_DIR}/mason install slang 2.3.1
    MASON_SLANG=$(${MASON_DIR}/mason prefix slang 2.3.1)
    ${MASON_DIR}/mason install bzip2 1.0.6
    MASON_BZIP2=$(${MASON_DIR}/mason prefix bzip2 1.0.6)
    ${MASON_DIR}/mason install elfutils 0.168
    MASON_ELFUTILS=$(${MASON_DIR}/mason prefix elfutils 0.168)
    EXTRA_CFLAGS="-m64 -I${MASON_SLANG}/include -I${MASON_ZLIB}/include  -I${MASON_XZ}/include -I${MASON_BINUTILS}/include -I${MASON_BZIP2}/include -I${MASON_ELFUTILS}/include"
    EXTRA_LDFLAGS="-L${MASON_BZIP2}/lib -L${MASON_ZLIB}/lib -L${MASON_XZ}/lib -L${MASON_SLANG}/lib  -L${MASON_ELFUTILS}/lib -L${MASON_BINUTILS}/lib"
}

# https://perf.wiki.kernel.org/index.php/Jolsa_Howto_Install_Sources
# https://askubuntu.com/questions/50145/how-to-install-perf-monitoring-tool/306683
# https://www.spinics.net/lists/linux-perf-users/msg03040.html
# https://software.intel.com/en-us/articles/linux-perf-for-intel-vtune-Amplifier-XE
function mason_compile {
    cd tools/perf
    # hack to enable libdw since I could not get the feature checks to work to autodetect libdw correctly
    patch -N -p1 < ${MASON_DIR}/scripts/${MASON_NAME}/${MASON_VERSION}/patch.diff
    # we set NO_LIBUNWIND since libdw is used from elfutils which is faster: https://lwn.net/Articles/579508/
    # note: LIBELF is needed for symbols + node --perf_basic_prof_only_functions
    make \
      LIBDW_LDFLAGS="-L${MASON_ELFUTILS}/lib -Wl,--start-group -lelf -lebl -lz -llzma -lbz2" \
      LIBDW_CFLAGS="-I${MASON_ELFUTILS}/include" \
      V=1 VF=1 \
      prefix=${MASON_PREFIX} \
      NO_LIBNUMA=1 \
      NO_LIBAUDIT=1 \
      NO_LIBUNWIND=1 \
      NO_BIONIC=1 \
      NO_BACKTRACE=1 \
      NO_LIBCRYPTO=1 \
      NO_LIBPERL=1 \
      NO_GTK2=1 \
      LDFLAGS="${EXTRA_LDFLAGS}" \
      NO_LIBPYTHON=1 \
      WERROR=0 \
      EXTRA_CFLAGS="${EXTRA_CFLAGS}" \
      install
}

function mason_cflags {
    :
}

function mason_ldflags {
    :
}

function mason_static_libs {
    :
}

mason_run "$@"

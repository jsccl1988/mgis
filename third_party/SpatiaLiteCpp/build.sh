#!/bin/bash

set -eu

# ----------------------------------------------------------------------
# FILE: build.sh
# DESCRIPTION: Script to build SpatiaLiteC++ on a Linux system.
# REQUIREMENTS: CMake, lcov, genhtml
# ----------------------------------------------------------------------

# Default argument values
SELF=$0
BUILDCOV=OFF
BUILDDIST=OFF
BUILDDOC=OFF
BUILDDYN=OFF
BUILDEXS=OFF
BUILDTEST=OFF
BUILDMODE=Release
BUILDDIR=build/linux
CMAKEARGS=
GENERATOR="Unix Makefiles"

# Define directories
SRCDIR=$(readlink -f $(dirname ${BASH_SOURCE[0]}))
DOCSDIR=$SRCDIR/docs/api
RESDIR=$SRCDIR/docs/resources
DOXYCSS=customdoxygen.css

# Display usage
__usage()
{
    echo "Usage: $0 [-h] [-a] [-b] [-c <string>] [-d] [-e] [-g <cmake generator>]"
    echo "                  [-m <Release|Debug>] [-o <directory>] [-t] [-x]"
    echo "    -h = Display this usage"
    echo "    -a = Build API documentation"
    echo "    -b = Build release distributions"
    echo "    -c = Additional CMake arguments. Get concatenated and passed to CMake."
    echo "    -d = Build dynamic library otherwise static"
    echo "    -e = Build examples project"
    echo "    -g = Build generator (Default: \"$GENERATOR\")"
    echo "    -m = Build mode (Default: $BUILDMODE)"
    echo "    -o = Build directory (Default: $BUILDDIR)"
    echo "    -t = Build test project (will build examples and Debug mode)"
    echo "    -x = Build test coverage (will build tests)"
}

# Parse arguments
USERDIR=
while getopts :habc:deg:m:o:tx flag ; do
    case $flag in
        h) __usage; exit 0; ;;
        a) BUILDDOC=ON; ;;
        b) BUILDDIST=ON; ;;
        c) CMAKEARGS="$CMAKEARGS $OPTARG"; ;;
        d) BUILDDYN=ON; ;;
        e) BUILDEXS=ON; ;;
        g) GENERATOR=$OPTARG; ;;
        m) BUILDMODE=$OPTARG; ;;
        o) USERDIR=$OPTARG; ;;
        t) BUILDTEST=ON; ;;
        x) BUILDCOV=ON; ;;
        *) echo "Invalid option: -$OPTARG" >&2; exit 1; ;;
    esac
done
shift $((OPTIND-1))

# Build documentation

if [ "$BUILDDOC" == "ON" ]; then
    if [ -d "$DOCSDIR" ]; then
        rm -rf $DOCSDIR
    fi
    mkdir $DOCSDIR
    #cp -r $RESDIR/* $DOCSDIR/
    doxygen Doxyfile
    #cp $RESDIR/$DOXYCSS $DOCSDIR/$DOXYCSS
    exit 0
fi

if [ "$BUILDDIST" == "ON" ]; then
    ./$SELF -o "build/linux-static" -m Debug
    ./$SELF -o "build/linux-static" -m Release
    ./$SELF -o "build/linux-shared" -m Debug -d
    ./$SELF -o "build/linux-shared" -m Release -d
    exit 0
fi

if [ "$BUILDCOV" == "ON" ]; then
    BUILDTEST=ON
fi

if [ "$BUILDTEST" == "ON" ]; then
    BUILDEXS=ON
    BUILDMODE=Debug
fi

# Add build mode as subfolder unless user gave explicit directory
if [[ -n "$USERDIR" ]]; then
    BUILDDIR=$USERDIR
fi
BUILDDIR=$BUILDDIR/$BUILDMODE

# Generate, build, and install
mkdir -p $BUILDDIR
pushd $BUILDDIR
cmake -DSPATIALITECPP_BUILD_DYNAMIC=$BUILDDYN \
      -DSPATIALITECPP_BUILD_EXAMPLES=$BUILDEXS \
      -DSPATIALITECPP_BUILD_TEST=$BUILDTEST \
      -DCMAKE_BUILD_TYPE=$BUILDMODE \
      -G"$GENERATOR" \
      $CMAKEARGS \
      $SRCDIR

TARGET=package
if [ "$BUILDTEST" == "ON" ]; then
    TARGET=install
fi
cmake --build . \
      --config $BUILDMODE \
      --target $TARGET

# Run coverage
if [ $? -eq 0 ] && [ "$BUILDCOV" == "ON" ]; then
    pushd test
    ctest
    popd
    lcov --capture --directory src -base-directory $SRCDIR --no-external --output-file coverage.info.tmp
    lcov --remove coverage.info.tmp $SRCDIR/ext/\* --output-file coverage.info
    genhtml coverage.info --output-directory coverage
    rm coverage.info*
fi

popd

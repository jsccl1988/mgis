#!/usr/bin/env bash

# NOTE: use the ./utils/new_boost.sh script to create new versions

export MASON_VERSION=1.63.0
export BOOST_VERSION=${MASON_VERSION//./_}
export BOOST_TOOLSET="clang"
export BOOST_TOOLSET_CXX="clang++"
export BOOST_ARCH="x86"
export BOOST_SHASUM=5c5cf0fd35a5950ed9e00ba54153df47747803f9
# special override to ensure each library shares the cached download
export MASON_DOWNLOAD_SLUG="boost-${MASON_VERSION}"

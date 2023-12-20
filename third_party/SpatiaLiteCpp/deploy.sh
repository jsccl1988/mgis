#!/bin/bash

set -eu

# ----------------------------------------------------------------------
# FILE: deploy.sh
# DESCRIPTION: Deploy documentation to gh-pages remote branch
# REQUIREMENTS: build.sh
# ----------------------------------------------------------------------

# Define branches and directories
REPO_BRANCH="gh-pages"
REPO_WORKDIR="build/gh-pages"

# Define github parameters
GIT_EMAIL=`git config user.email`
GIT_REPO=`git config remote.origin.url`
GIT_SHA=`git rev-parse --short --verify HEAD`
GIT_USER=`git config user.name`

# Clone the gh-pages branch
mkdir -p $REPO_WORKDIR
git clone $GIT_REPO $REPO_WORKDIR
pushd $REPO_WORKDIR
git checkout $REPO_BRANCH || git checkout --orphan $REPO_BRANCH
popd

# Delete contents and hidden files
rm -rf $REPO_WORKDIR/** || exit 1
find $REPO_WORKDIR -iname ".*" -type f -delete || exit 1

# Build documentation
./build.sh -a
cp -r docs/. $REPO_WORKDIR/

# Commit and push updates
pushd $REPO_WORKDIR
if test -z "$(git status --porcelain)"; then
    echo "No updates to $REPO_BRANCH. Exiting."
else
    git add .
    git commit -m "Deploy to GitHub Pages: ${GIT_SHA}"
    git push $GIT_REPO $REPO_BRANCH
fi
popd

# Remove working directory
rm -rf $REPO_WORKDIR

#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 3 ]; then
  echo "Usage: build-hamlib-macos.sh BRANCH DEPLOYMENT_TARGET PREFIX" >&2
  exit 2
fi

branch="$1"
deployment_target="$2"
prefix="$3"

git clone --depth 1 --branch "$branch" \
  https://github.com/Hamlib/Hamlib.git hamlib-src
cd hamlib-src
./bootstrap
# -headerpad_max_install_names leaves room in the Mach-O header for the
# install names and rpaths the bundling step rewrites later. Without it that
# step dies on Hamlib's own utilities:
#
#   install_name_tool: changing install names or rpaths can't be redone for:
#   .../MacOS/rigctl-wsjtx because larger updated load commands do not fit
#
# build-macos.yml passes the same flag to the WSJT-X CMake build, so WSJT-X's
# binaries always had the padding; rigctl-wsjtx is built here, by autotools,
# and was the one binary in the bundle without it.
./configure \
  --prefix="$prefix" \
  --disable-shared --enable-static \
  --without-cxx-binding \
  CFLAGS="-mmacosx-version-min=${deployment_target}" \
  LDFLAGS="-mmacosx-version-min=${deployment_target} -Wl,-headerpad_max_install_names"
make -j"$(sysctl -n hw.ncpu)"
make install

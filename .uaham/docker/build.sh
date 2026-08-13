#!/usr/bin/env bash
#
# Build WSJT-X inside the toolchain image (see Dockerfile beside this script).
#
#   .uaham/docker/build.sh              # configure if needed, then build
#   .uaham/docker/build.sh --fresh      # throw the build directory away first
#   .uaham/docker/build.sh --tests      # build and run the ctest suite
#
# The build directory lives on the host at build/linux, so an incremental build
# after one edited file takes seconds. It is deliberately outside the source
# tree that CMake reads, and build/ was added to .gitignore for it.
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMAGE="${UAHAM_BUILD_IMAGE:-uaham/wsjtx-build}"
BUILD_DIR="build/linux"

fresh=0
tests=0
for arg in "$@"; do
  case "$arg" in
    --fresh) fresh=1 ;;
    --tests) tests=1 ;;
    *) echo "unknown option: $arg" >&2; exit 2 ;;
  esac
done

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
  echo "==> building toolchain image $IMAGE (first time only, ~15 min)"
  docker build -t "$IMAGE" "$REPO_ROOT/.uaham/docker"
fi

# Run as the invoking user so the build output is not left owned by root —
# otherwise the next `git status` on the host reports files it cannot stat.
run_in_container () {
  docker run --rm \
    -v "$REPO_ROOT:/src" \
    -u "$(id -u):$(id -g)" \
    -e HOME=/tmp \
    -w /src \
    "$IMAGE" bash -c "$1"
}

if [ "$fresh" = 1 ]; then
  echo "==> removing $BUILD_DIR"
  rm -rf "${REPO_ROOT:?}/$BUILD_DIR"
fi

# WSJT_BUILD_TESTS is off unless asked for: the suite needs pFUnit, which is
# another long compile, and the CI runs it on every push anyway.
if [ ! -f "$REPO_ROOT/$BUILD_DIR/CMakeCache.txt" ]; then
  echo "==> configuring"
  run_in_container "cmake -S /src -B /src/$BUILD_DIR \
      -DCMAKE_PREFIX_PATH=/opt/hamlib \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
      -DCMAKE_BUILD_TYPE=Release \
      -DWSJT_SKIP_MANPAGES=ON \
      -DWSJT_GENERATE_DOCS=OFF \
      -DWSJT_BUILD_TESTS=$([ "$tests" = 1 ] && echo ON || echo OFF) \
      -Wno-dev"
fi

echo "==> building"
run_in_container "cmake --build /src/$BUILD_DIR -j\$(nproc)"

if [ "$tests" = 1 ]; then
  echo "==> ctest"
  run_in_container "cd /src/$BUILD_DIR && QT_QPA_PLATFORM=offscreen ctest --output-on-failure"
fi

echo
echo "==> binaries in $BUILD_DIR:"
ls -lh "$REPO_ROOT/$BUILD_DIR/wsjtx" "$REPO_ROOT/$BUILD_DIR/jt9" 2>/dev/null || true

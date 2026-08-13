# Reproducible Linux build environment for the UaHamAward fork of WSJT-X.
#
# This is a copy of what .github/workflows/build-linux.yml does on the CI
# runner, frozen into an image so a developer gets the same compiler, the same
# Qt and — the part that actually matters — the same Hamlib. Building against a
# distribution's libhamlib is not equivalent: upstream pins 4.7.0 and links it
# statically, and a rig backend that differs between a local build and the
# released binary is a bug nobody can reproduce.
#
# The image carries only the toolchain. The source tree is mounted at build
# time, so editing a file on the host is enough — no rebuild of this image.
#
#   docker build -t uaham/wsjtx-build .uaham/docker
#   .uaham/docker/build.sh
#
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# The package list is lifted verbatim from build-linux.yml so the two cannot
# drift apart silently; git, ca-certificates and xvfb are the only additions,
# needed to fetch Hamlib and to run the Qt test suite without a display.
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake gfortran \
      libfftw3-dev libboost-all-dev \
      qtbase5-dev qttools5-dev qtmultimedia5-dev libqt5serialport5-dev \
      libqt5sql5-sqlite libqt5websockets5-dev \
      libqt5multimedia5-plugins \
      libusb-1.0-0-dev libudev-dev libreadline-dev portaudio19-dev \
      autoconf automake libtool pkg-config \
      texinfo \
      dpkg-dev fakeroot \
      asciidoctor \
      rpm \
      git ca-certificates xvfb \
 && rm -rf /var/lib/apt/lists/*

# Hamlib, built exactly as the workflow builds it: static, no C++ binding, and
# with the section flags that let the linker drop the backends WSJT-X never
# calls. Baked into the image rather than fetched per build — it is a quarter
# of an hour of compilation that never changes between our builds.
ARG HAMLIB_BRANCH=4.7.0
RUN git clone --depth 1 --branch "${HAMLIB_BRANCH}" \
      https://github.com/Hamlib/Hamlib.git /tmp/hamlib-src \
 && cd /tmp/hamlib-src \
 && ./bootstrap \
 && ./configure \
      --prefix=/opt/hamlib \
      --disable-shared --enable-static \
      --without-cxx-binding \
      CFLAGS="-g -O2 -fPIC -fdata-sections -ffunction-sections" \
      LDFLAGS="-Wl,--gc-sections" \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /tmp/hamlib-src

WORKDIR /src

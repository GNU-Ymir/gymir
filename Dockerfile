# syntax=docker/dockerfile:1

# Builds gyc-<GCC_MAJOR_VERSION>_<GYC_VERSION_NUMBER>_amd64.deb - the GCC-based Ymir compiler -
# from this checkout of the ymir frontend (gcc/ymir in a full GCC source tree), bundled with the
# midgard standard library. Adapted from GNU-Ymir/CD_suite's amd64/deb/jobs/{clone_gcc,
# bootstrap_build} Dockerfiles, collapsed into one file and pointed at a local checkout instead
# of re-cloning this repo from GitHub. Nothing is built from source for the *compiler that does
# the compiling*: the previous gyc/gyllir release is downloaded as a prebuilt .deb, exactly like
# GNU-Ymir/bootstrap's own Dockerfile does. All build-args below are read from YMIR_VERSION at
# the repo root by `.github/workflows/release.yml` - that file is the single source of truth,
# copied from ymir/bootstrap/YMIR_VERSION by hand at release time (nothing here bumps a version).
#
# To build locally:
#   . ./YMIR_VERSION
#   GCC_MAJOR_VERSION="${GCC_VERSION%%.*}"
#   docker build \
#     --build-arg GCC_VERSION="$GCC_VERSION" \
#     --build-arg GCC_MAJOR_VERSION="$GCC_MAJOR_VERSION" \
#     --build-arg COMPILER_MAJOR_VERSION="$GCC_MAJOR_VERSION" \
#     --build-arg GYC_RELEASE_TAG="$YMIR_BOOTSTRAP_VERSION" \
#     --build-arg GYC_ASSET="gyc-${GCC_MAJOR_VERSION}_${YMIR_BOOTSTRAP_VERSION}_amd64.deb" \
#     --build-arg GYLLIR_RELEASE_TAG="$GYLLIR_VERSION" \
#     --build-arg GYLLIR_ASSET="gyllir_${GYLLIR_VERSION}_amd64.deb" \
#     --build-arg BOOTSTRAP_RELEASE_TAG="$GYC_VERSION" \
#     --build-arg MIDGARD_RELEASE_TAG="$MIDGARD_VERSION" \
#     --build-arg GYC_VERSION_NUMBER="$GYC_VERSION" \
#     --target export --output type=local,dest=out .

ARG UBUNTU_VERSION="26.04"
ARG ARCH="amd64"

# ---------------------------------------------------------------------------
# clone_gcc: bare shallow clone of the upstream GCC tree, cached independently of every
# version knob below it (mirrors CD_suite's jobs/clone_gcc/Dockerfile).
# ---------------------------------------------------------------------------
FROM ubuntu:${UBUNTU_VERSION} AS clone_gcc

RUN apt-get update && apt-get install -y --no-install-recommends \
        sudo pkg-config git build-essential software-properties-common aspcud unzip curl wget \
        gcc g++ flex autoconf automake libtool cmake patchelf libdwarf-dev \
        gcc-multilib g++-multilib libgc-dev libgmp-dev libbfd-dev zlib1g-dev gdc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /gcc
RUN git clone --depth=1 git://gcc.gnu.org/git/gcc.git gcc-src

# ---------------------------------------------------------------------------
# toolchain: install the previous gyc/gyllir release .deb's - this is the compiler that builds
# the new ymirc (bootstrap) source into ymir1, and the tool that drives that build (see
# Make-lang.in's `ymir/libymirc.a` target, which shells out to `gyllir build --release`).
# ---------------------------------------------------------------------------
FROM clone_gcc AS toolchain

ARG GCC_MAJOR_VERSION
ARG COMPILER_MAJOR_VERSION
ARG GYC_RELEASE_TAG
ARG GYC_ASSET
ARG GYLLIR_RELEASE_TAG
ARG GYLLIR_ASSET

RUN test -n "$GYC_RELEASE_TAG" && test -n "$GYC_ASSET" || \
    (echo "GYC_RELEASE_TAG and GYC_ASSET build-args are required - see YMIR_VERSION" >&2 && exit 1)
RUN test -n "$GYLLIR_RELEASE_TAG" && test -n "$GYLLIR_ASSET" || \
    (echo "GYLLIR_RELEASE_TAG and GYLLIR_ASSET build-args are required - see YMIR_VERSION" >&2 && exit 1)

RUN curl -fsSL -o /tmp/gyc.deb \
        "https://github.com/GNU-Ymir/gymir/releases/download/${GYC_RELEASE_TAG}/${GYC_ASSET}" \
    && curl -fsSL -o /tmp/gyllir.deb \
        "https://github.com/GNU-Ymir/Gyllir/releases/download/${GYLLIR_RELEASE_TAG}/${GYLLIR_ASSET}" \
    && apt-get update \
    && apt-get install -y --no-install-recommends /tmp/gyc.deb /tmp/gyllir.deb \
    && rm -f /tmp/gyc.deb /tmp/gyllir.deb \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update \
    && apt-get install -y --no-install-recommends gcc-${COMPILER_MAJOR_VERSION} g++-${COMPILER_MAJOR_VERSION} \
    && rm -rf /var/lib/apt/lists/*

RUN gyc --version
RUN gyllir -h || true

# ---------------------------------------------------------------------------
# fetch_gcc_version: check out the target upstream GCC release, merge in this checkout as the
# ymir frontend, and pull in the matching self-hosted bootstrap (ymirc) source.
# ---------------------------------------------------------------------------
FROM toolchain AS fetch_gcc_version

ARG GCC_VERSION
ARG BOOTSTRAP_RELEASE_TAG
ARG MIDGARD_RELEASE_TAG

RUN test -n "$BOOTSTRAP_RELEASE_TAG" || \
    (echo "BOOTSTRAP_RELEASE_TAG build-arg is required - see YMIR_VERSION's GYC_VERSION" >&2 && exit 1)

WORKDIR /gcc/gcc-src
RUN git fetch origin releases/gcc-${GCC_VERSION}:refs/releases/gcc-${GCC_VERSION} --depth=1
RUN git switch refs/releases/gcc-${GCC_VERSION} --detach

WORKDIR /gcc/gcc-src/gcc/ymir
COPY . .

RUN grep -qF 'ymir/gycspec.o: $(CORETYPES_H) $(PLUGIN_HEADERS) $(INSN_ATTR_H)' Make-lang.in || sed -i '/^CFLAGS-ymir/a ymir/gycspec.o: $(CORETYPES_H) $(PLUGIN_HEADERS) $(INSN_ATTR_H)' Make-lang.in
RUN touch lang.opt.urls

# bootstrap/ is gitignored in this checkout (it's a separate repo, GNU-Ymir/bootstrap) - fetch
# it fresh at the tag this release is being cut under. This tag must already exist on bootstrap
# (gymir and bootstrap are released in lockstep); fail loudly rather than silently building the
# wrong self-hosted source if it doesn't.
RUN git clone https://github.com/GNU-Ymir/bootstrap.git

WORKDIR /gcc/gcc-src/gcc/ymir/bootstrap
RUN git fetch --tags \
    && git switch "${BOOTSTRAP_RELEASE_TAG}" --detach || \
    (echo "::error::github.com/GNU-Ymir/bootstrap has no tag ${BOOTSTRAP_RELEASE_TAG} - tag that release on bootstrap before releasing gymir" >&2 && exit 1)

# Keep the compiled ymirc's baked-in include-path constant (MIDGARD_VERSION in this checkout's
# own YMIR_VERSION, read by Make-lang.in via `sed` at `make` time) in sync with the midgard
# release this build actually pairs with (MIDGARD_RELEASE_TAG, checked out below in
# build_midgard) - the tag's own committed YMIR_VERSION would otherwise be free to drift from
# what this repo's root YMIR_VERSION intends.
RUN if [ -f YMIR_VERSION ]; then \
		grep -q '^MIDGARD_VERSION=' YMIR_VERSION \
			&& sed -i "s/^MIDGARD_VERSION=.*/MIDGARD_VERSION=${MIDGARD_RELEASE_TAG}/" YMIR_VERSION \
			|| echo "MIDGARD_VERSION=${MIDGARD_RELEASE_TAG}" >> YMIR_VERSION; \
	fi

WORKDIR /gcc/gcc-src/
RUN ./contrib/download_prerequisites

# ---------------------------------------------------------------------------
# configure / make: build gyc (and, via Make-lang.in's ymir/libymirc.a target, the bootstrap
# source) against the just-installed previous gyc/gyllir.
# ---------------------------------------------------------------------------
FROM fetch_gcc_version AS configure

ARG GCC_MAJOR_VERSION
ARG COMPILER_MAJOR_VERSION

WORKDIR /gcc/gcc-build
RUN CC=gcc-${COMPILER_MAJOR_VERSION} CXX=g++-${COMPILER_MAJOR_VERSION} ../gcc-src/configure --enable-languages=c,d,ymir --with-gcc-major-version-only --program-suffix=-${GCC_MAJOR_VERSION} --prefix=/usr --program-prefix=x86_64-linux-gnu- --libexecdir=/usr/libexec --libdir=/usr/lib --with-sysroot=/ --with-arch-directory=amd64 --enable-multiarch --with-arch-32=i686 --with-abi=m64 --with-multilib-list=m32,m64,mx32 --enable-multilib --enable-checking=release --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --disable-bootstrap

FROM configure AS make
WORKDIR /gcc/gcc-build
RUN make -j$(( $(nproc) - 1 ))
RUN make install DESTDIR=/gcc/gcc-install

# ---------------------------------------------------------------------------
# build_first_deb: stage the raw gyc binaries into a throwaway .deb, just so build_midgard below
# can install it (and its Depends:) in a clean image rather than dragging the whole gcc-build
# tree along.
# ---------------------------------------------------------------------------
FROM make AS build_first_deb

ARG GCC_VERSION
ARG GCC_MAJOR_VERSION

WORKDIR /gcc
RUN mkdir -p gcc-bin/DEBIAN gcc-bin/usr/bin \
    gcc-bin/usr/lib/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION} \
    gcc-bin/usr/libexec/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION}

RUN cp /gcc/gcc-install/usr/bin/x86_64-linux-gnu-gyc-${GCC_MAJOR_VERSION} /gcc/gcc-bin/usr/bin/
RUN cp /gcc/gcc-install/usr/libexec/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION}/ymir1 /gcc/gcc-bin/usr/libexec/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION}/ymir1

WORKDIR /gcc/gcc-bin/usr/bin
RUN ln -s x86_64-linux-gnu-gyc-${GCC_MAJOR_VERSION} gyc-${GCC_MAJOR_VERSION} \
    && ln -s gyc-${GCC_MAJOR_VERSION} gyc

WORKDIR /gcc/gcc-bin/DEBIAN
COPY packaging/control.in /gcc/gcc-bin/DEBIAN/control
RUN sed -i "s/@GCC_MAJOR_VERSION@/${GCC_MAJOR_VERSION}/g; s/@GCC_VERSION@/${GCC_VERSION}/g" /gcc/gcc-bin/DEBIAN/control

WORKDIR /gcc
RUN dpkg --build gcc-bin

# ---------------------------------------------------------------------------
# build_midgard: fresh image, install the gyc-only .deb from above, build midgard against it and
# bundle it in, produce the final gyc-<major>_<GYC_VERSION_NUMBER>_amd64.deb.
# ---------------------------------------------------------------------------
FROM ubuntu:${UBUNTU_VERSION} AS build_midgard

ARG GCC_VERSION
ARG GCC_MAJOR_VERSION
ARG GYC_VERSION_NUMBER
ARG MIDGARD_RELEASE_TAG
ARG ARCH

RUN test -n "$GYC_VERSION_NUMBER" || \
    (echo "GYC_VERSION_NUMBER build-arg is required - see YMIR_VERSION's GYC_VERSION" >&2 && exit 1)

WORKDIR /gcc
RUN apt-get update && apt-get install -y --no-install-recommends \
        gcc-${GCC_MAJOR_VERSION} g++-${GCC_MAJOR_VERSION} flex autoconf automake libtool cmake \
        patchelf libdwarf-dev git libgc-dev \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p gcc-bin/DEBIAN gcc-bin/usr/bin \
    gcc-bin/usr/lib/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION} \
    gcc-bin/usr/libexec/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION}
COPY --from=build_first_deb gcc/gcc-install/usr/bin/x86_64-linux-gnu-gyc-${GCC_MAJOR_VERSION} gcc-bin/usr/bin/
COPY --from=build_first_deb gcc/gcc-install/usr/libexec/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION}/ymir1 gcc-bin/usr/libexec/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION}/ymir1

COPY --from=build_first_deb /gcc/gcc-bin.deb .
RUN apt-get update && apt-get install -y --no-install-recommends ./gcc-bin.deb && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/GNU-Ymir/yruntime.git
WORKDIR /gcc/yruntime
RUN git fetch --all --tags
RUN git checkout "${MIDGARD_RELEASE_TAG}"

# midgard's own CMakeLists.txt names the built libs/install dir after the major.minor of
# YMIR_BOOTSTRAP_VERSION in this checkout's YMIR_VERSION file - not after MIDGARD_RELEASE_TAG
# itself, and the two can drift. Read the same file CMake reads instead of re-deriving it.
RUN sed -nE 's/^YMIR_BOOTSTRAP_VERSION=([0-9]+\.[0-9]+).*/\1/p' YMIR_VERSION > /tmp/midgard_short_version
RUN test -s /tmp/midgard_short_version || (echo "could not derive MIDGARD_SHORT_VERSION from $(pwd)/YMIR_VERSION" >&2 && exit 1)

RUN mkdir .build
WORKDIR /gcc/yruntime/.build
RUN touch __lib_test_.o __lib_debug_.o __lib_release_.o
RUN cmake ..
RUN make
RUN make install DESTDIR=/gcc/gcc-bin

WORKDIR /gcc/yruntime/midgard
RUN MIDGARD_SHORT_VERSION="$(cat /tmp/midgard_short_version)" \
	&& mkdir -p /gcc/gcc-bin/usr/libexec/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION}/include/ymir/$MIDGARD_SHORT_VERSION \
	&& cp -r * /gcc/gcc-bin/usr/libexec/gcc/x86_64-linux-gnu/${GCC_MAJOR_VERSION}/include/ymir/$MIDGARD_SHORT_VERSION

WORKDIR /gcc/gcc-bin/usr/lib
RUN MIDGARD_SHORT_VERSION="$(cat /tmp/midgard_short_version)" \
	&& ln -s libgymidgard-debug_$MIDGARD_SHORT_VERSION.a libgymidgard-debug.a \
	&& ln -s libgymidgard-tests_$MIDGARD_SHORT_VERSION.a libgymidgard-tests.a \
	&& ln -s libgymidgard-release_$MIDGARD_SHORT_VERSION.a libgymidgard-release.a

WORKDIR /gcc/gcc-bin/DEBIAN
COPY packaging/control.in /gcc/gcc-bin/DEBIAN/control
RUN sed -i "s/@GCC_MAJOR_VERSION@/${GCC_MAJOR_VERSION}/g; s/@GCC_VERSION@/${GYC_VERSION_NUMBER}/g" /gcc/gcc-bin/DEBIAN/control

WORKDIR /gcc/gcc-bin/usr/bin
RUN ln -s x86_64-linux-gnu-gyc-${GCC_MAJOR_VERSION} gyc-${GCC_MAJOR_VERSION} \
    && ln -s gyc-${GCC_MAJOR_VERSION} gyc

WORKDIR /gcc
RUN dpkg --build gcc-bin
RUN mv gcc-bin.deb /gyc-${GCC_MAJOR_VERSION}_${GYC_VERSION_NUMBER}_${ARCH}.deb

# ---------------------------------------------------------------------------
# export: nothing but the produced .deb, for a cheap `docker create` + `docker cp`.
# ---------------------------------------------------------------------------
FROM scratch AS export

ARG GCC_MAJOR_VERSION
ARG GYC_VERSION_NUMBER
ARG ARCH

COPY --from=build_midgard /gyc-${GCC_MAJOR_VERSION}_${GYC_VERSION_NUMBER}_${ARCH}.deb /
CMD ["true"]

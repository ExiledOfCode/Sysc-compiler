# base image
FROM maxxing/compiler-dev

# some arguments
ARG KOOPA_REPO_URL=https://github.com/pku-minic/koopa.git
ARG KOOPAC=koopac.tar.gz
ARG SYSYRT_REPO_URL=https://github.com/pku-minic/sysy-runtime-lib.git
ARG AUTOTEST=autotest
ARG INSTALL_DIR=/opt
ARG LIB_INSTALL_DIR=${INSTALL_DIR}/l
ARG INC_INSTALL_DIR=${INSTALL_DIR}/include
ARG BIN_INSTALL_DIR=${INSTALL_DIR}/bin

# setup APT mirror
RUN sed -i "s@http://.*archive.ubuntu.com@http://mirrors.tuna.tsinghua.edu.cn@g" /etc/apt/sources.list && \
  sed -i "s@http://.*security.ubuntu.com@http://mirrors.tuna.tsinghua.edu.cn@g" /etc/apt/sources.list

# setup ca-certificates for https mirror
RUN apt update 
RUN apt upgrade ca-certificates --fix-missing -y

# setup APT mirror for arm64, armhf, PowerPC, ppc64el, s390x
RUN cat <<'EOF' > /etc/apt/sources.list
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ focal main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ focal-updates main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ focal-backports main restricted universe multiverse
deb http://ports.ubuntu.com/ubuntu-ports/ focal-security main restricted universe multiverse
EOF

# install LLVM's APT
RUN apt update && \
  DEBIAN_FRONTEND="noninteractive" apt install -y wget gnupg && \
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
  echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal main" >> /etc/apt/sources.list && \
  echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-12 main" >> /etc/apt/sources.list && \
  echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-13 main" >> /etc/apt/sources.list

# install necessary packages
RUN apt update && DEBIAN_FRONTEND="noninteractive" apt install -y \
  git flex bison python3 \
  make cmake \
  qemu-user-static \
  clang-13 lldb-13 lld-13

# setup LLVM toolchain
WORKDIR /root
COPY update-alternatives-clang.sh .
RUN bash update-alternatives-clang.sh 13 100 && \
  rm update-alternatives-clang.sh

# install Rust toolchain
RUN wget -O - https://sh.rustup.rs | sh -s -- -y && \
  mkdir -p ~/.cargo && \
  echo "[source.crates-io]" > ~/.cargo/config && \
  echo "replace-with = 'tuna'" >> ~/.cargo/config && \
  echo "[source.tuna]" >> ~/.cargo/config && \
  echo "registry = \"sparse+https://mirrors.tuna.tsinghua.edu.cn/crates.io-index/\"" >> ~/.cargo/config
ENV RUSTUP_UPDATE_ROOT=https://mirrors.tuna.tsinghua.edu.cn/rustup/rustup
ENV RUSTUP_DIST_SERVER=https://mirrors.tuna.tsinghua.edu.cn/rustup
ENV PATH="/root/.cargo/bin:${PATH}"

# setup install directories
RUN mkdir -p ${LIB_INSTALL_DIR} ${BIN_INSTALL_DIR}
ENV CDE_LIBRARY_PATH="${LIB_INSTALL_DIR}"
ENV CDE_INCLUDE_PATH="${INC_INSTALL_DIR}"
ENV PATH="${BIN_INSTALL_DIR}:${PATH}"

# install libkoopa
WORKDIR ${LIB_INSTALL_DIR}
RUN git clone --single-branch --depth 1 ${KOOPA_REPO_URL} koopa && \
  cargo build --release --manifest-path koopa/crates/libkoopa/Cargo.toml && \
  mkdir -p native && \
  cp koopa/crates/libkoopa/target/release/libkoopa.a native && \
  mkdir -p ${INC_INSTALL_DIR} && \
  cp -R koopa/crates/libkoopa/include/* ${INC_INSTALL_DIR}/. && \
  rm -rf koopa /root/.cargo/registry

# install koopac
WORKDIR ${INSTALL_DIR}
COPY ${KOOPAC} .
RUN mkdir kpc && tar xzf ${KOOPAC} -C kpc && \
  cargo install --path kpc --root ${INSTALL_DIR} --no-track && \
  rm -rf ${KOOPAC} kpc /root/.cargo/registry

# install SysY runtime library
WORKDIR ${LIB_INSTALL_DIR}
RUN git clone --single-branch --depth 1 ${SYSYRT_REPO_URL} sysyrt && \
  make -C sysyrt libsysy && \
  mkdir -p native && cp sysyrt/build/libsysy.a native && \
  make NO_LIBC=1 \
  ADD_CFLAGS="-target riscv32-unknown-linux-elf -march=rv32im -mabi=ilp32" \
  -C sysyrt clean libsysy && \
  mkdir -p riscv32 && cp sysyrt/build/libsysy.a riscv32 && \
  rm -rf sysyrt


# compiler development environment image
FROM compiler-dev-base as compiler-dev

# some arguments
ARG TEST_CASES_REPO_URL=https://github.com/pku-minic/compiler-dev-test-cases.git

# install autotest and test cases
WORKDIR ${BIN_INSTALL_DIR}
COPY ${AUTOTEST} .
RUN git clone --single-branch --depth 1 ${TEST_CASES_REPO_URL} cd-test-cases && \
  make LIB_DIR=${LIB_INSTALL_DIR}/native INSTALL_DIR=${BIN_INSTALL_DIR}/testcases \
  -C cd-test-cases install -j`nproc` && \
  rm -rf cd-test-cases

WORKDIR /root
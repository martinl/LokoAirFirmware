ARG VARIANT="focal"
FROM mcr.microsoft.com/vscode/devcontainers/base:0-${VARIANT}
ARG VARIANT

ENV REFRESHED_AT 2023-02-15

# build commands to build and tag image (from root project directory):
# !!! DONT FORGET TO CHANGE TAG (VERSION) !!!
# docker login registry.gitlab.com -u dkrasutski
# <type your gitlab password>
# docker build -t registry.gitlab.com/dkrasutski/loko_air/loko:X.Y -f ./.devcontainer/Dockerfile .
# docker push registry.gitlab.com/dkrasutski/loko_air/loko:X.Y
# Manually build the Docker image: docker build -f .devcontainer/Dockerfile .

#Install CLang keys
# RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
# RUN echo "\n\n"\
#     "deb http://apt.llvm.org/focal/ llvm-toolchain-focal main\n"\
#     "deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal main\n"\
#     "# 16\n"\
#     "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-16 main\n"\
#     "deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal-16 main\n"\
#     "# 15\n"\
#     "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main\n"\
#     "deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main\n" >> /etc/apt/sources.list

RUN apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends software-properties-common \
    && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && add-apt-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main" \
    && apt-get -y install \
    python3-dev \
    python3-pip \
    git \
    bzr \
    lib32z1 \
    cmake \
    ninja-build \
    gdb \
    protobuf-compiler \
    automake \
    autoconf \
    usbutils \
    make \
    libncurses5 \
    build-essential \
    libtool \
    gcc-multilib \
    g++-multilib \
    bc \
    lcov \
    unzip \
    device-tree-compiler \
    tcl-dev \
    ccache \
    clang-tidy-15 \
    clang-format-15 \
    && apt-get remove -y software-properties-common \
    && rm -rf /var/lib/apt/lists/*

# COPY ./.devcontainer/llvm.sh /opt/llvm.sh
# RUN /opt/llvm.sh

# Setup symlinks for clang tools
RUN ln -s /usr/bin/clang-tidy-15 /usr/bin/clang-tidy \
    && ln -s /usr/bin/clang-format-15 /usr/bin/clang-format


# Update PIP
RUN pip3 install --no-cache-dir --upgrade pip
RUN pip3 install commitizen pycryptodome kconfiglib pre-commit


#CppUTest setup
RUN git clone https://github.com/cpputest/cpputest.git /opt/cpputest -b v4.0 --recursive --shallow-submodules --depth 1 \
    && cd /opt/cpputest \
    && autoreconf . -i \
    && ./configure "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
    && make tdd -j$(nproc) \
    && find . -maxdepth 1 ! \( -name '.' -o -name 'lib' -o -name 'include' -o -name 'scripts' \) -exec rm -rf {} +
ENV CPPUTEST_HOME="/opt/cpputest" \
    PATH="/opt/cpputest/scripts:${PATH}"

#CppUMockGen setup
# RUN mkdir -p /opt/cppumockgen \
#     && wget -P /opt/cppumockgen/ https://github.com/jgonzalezdr/CppUMockGen/releases/download/v0.4/cppumockgen_0.4-22_amd64.deb \
#     && apt-get update \
#     && apt-get install /opt/cppumockgen/cppumockgen_0.4-22_amd64.deb -y \
#     && rm -rf /var/lib/apt/lists/* \
#     && rm -rf /opt/cppumockgen

# Set up a tools dev directory
WORKDIR /usr

ENV GCC_ARM_VERSION="gcc-arm-none-eabi-10.3-2021.07" \
    GCC_ARM_RELEASE_TIME="10.3-2021.07"

# Download ARM GNU toolchain
RUN wget -q --show-progress --progress=bar:force:noscroll --no-check-certificate --output-document=gnu_gcc_arm.tar.bz2 https://developer.arm.com/-/media/Files/downloads/gnu-rm/${GCC_ARM_RELEASE_TIME}/${GCC_ARM_VERSION}-x86_64-linux.tar.bz2 \
    && tar -xf gnu_gcc_arm.tar.bz2 \
    && rm gnu_gcc_arm.tar.bz2


ENV GCC_PATH /usr/${GCC_ARM_VERSION}
ENV PATH /usr/${GCC_ARM_VERSION}/bin:$PATH


RUN echo "__bash_prompt() {\n" \
    "    local userpart='\`export XIT=\$? " \
    "        && [ ! -z \"\${GITHUB_USER}\" ] && echo -n \"\[\033[0;32m\]@\${GITHUB_USER} \" || echo -n \"\[\033[0;32m\]\u \" " \
    "        && [ \"\$XIT\" -ne \"0\" ] && echo -n \"\[\033[1;31m\]➜\" || echo -n \"\[\033[0m\]➜\"\`' \n" \
    "    local gitbranch='\`" \
    "        if [ \"\$(git config --get codespaces-theme.hide-status 2>/dev/null)\" != 1 ]; then " \
    "            export BRANCH=\$(git symbolic-ref --short HEAD 2>/dev/null || git rev-parse --short HEAD 2>/dev/null); " \
    "            if [ \"\${BRANCH}\" != \"\" ]; then " \
    "                echo -n \"\[\033[0;36m\](\[\033[1;31m\]\${BRANCH}\" " \
    "                && if git ls-files --error-unmatch -m --directory --no-empty-directory -o --exclude-standard \":/*\" > /dev/null 2>&1; then " \
    "                        echo -n \" \[\033[1;33m\]✗\"; " \
    "                fi " \
    "                && echo -n \"\[\033[0;36m\]) \";" \
    "            fi; " \
    "        fi\`'\n" \
    "    local lightblue='\[\033[1;34m\]'\n" \
    "    local removecolor='\[\033[0m\]'\n" \
    "    PS1=\"\${userpart} \${lightblue}\w \${gitbranch}\${removecolor}\\$ \"\n" \
    "    unset -f __bash_prompt\n" \
    "}\n" \
    "__bash_prompt" \ >> ~/.bashrc

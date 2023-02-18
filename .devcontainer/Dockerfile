FROM mcr.microsoft.com/devcontainers/cpp:0-ubuntu-22.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.22.2"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>


# [Optional] Uncomment this section to install additional packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
     && apt-get -y install --no-install-recommends software-properties-common ninja-build

# Install llvm
# COPY tools/llvm.sh /tmp/
# RUN chmod +x /tmp/llvm.sh && /tmp/llvm.sh
# RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-14 100
# RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-14 100
# RUN update-alternatives --install /usr/bin/clang cc /usr/bin/clang-14 100
# RUN update-alternatives --install /usr/bin/clang++ c++ /usr/bin/clang++-14 100
# RUN update-alternatives --install /usr/bin/lldb lldb /usr/bin/lldb-14 100

#RUN useradd -rm -d /home/vscode -s /bin/bash -g root -G sudo -u 1001 vscode
USER vscode
WORKDIR /home/vscode
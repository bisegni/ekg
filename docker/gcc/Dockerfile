ARG GCC_VERSION=12.2.0-bullseye
FROM gcc:$GCC_VERSION
LABEL maintainer="bisegni@slac.stanford.edu"

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.25.3"

RUN apt-get update && \
    apt-get install -y tclsh

COPY ./.devcontainer/reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
     && apt-get -y install --no-install-recommends software-properties-common ninja-build

COPY src /opt/app/src
COPY test /opt/app/test
COPY CMakeLists.txt /opt/app/

RUN mkdir opt/app/build && cd opt/app/build && cmake .. && make install

# syntax=docker/dockerfile:1.6

########################################
# Builder image
########################################
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive \
    IFCOPENSHELL_VERSION=0.8.0 \
    IFCOPENSHELL_ROOT=/opt/ifcopenshell

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    wget \
    unzip \
    python3 \
    python3-pip \
    openjdk-17-jdk \
    swig \
    pkg-config \
    libboost-all-dev \
    libeigen3-dev \
    libgl1-mesa-dev \
    libxi-dev \
    libxmu-dev \
    libx11-dev \
    libtbb-dev \
    libgmp-dev \
    libmpfr-dev \
    libcgal-dev \
    libpng-dev \
    libfreetype6-dev \
    libssl-dev \
    libxml2-dev \
    libhdf5-dev \
    nlohmann-json3-dev \
    opencollada-dev \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y --no-install-recommends \
    libocct-data-exchange-dev \
    libocct-foundation-dev \
    libocct-modeling-algorithms-dev \
    libocct-modeling-data-dev \
    libocct-ocaf-dev \
    libocct-visualization-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp
RUN git clone --branch v${IFCOPENSHELL_VERSION} --recurse-submodules --depth 1 --shallow-submodules \
    https://github.com/IfcOpenShell/IfcOpenShell.git ifcopenshell

WORKDIR /tmp/ifcopenshell/cmake
RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${IFCOPENSHELL_ROOT} \
    -DBUILD_IFCGEOM=1 \
    -DBUILD_IFCPYTHON=0 \
    -DBUILD_CONVERT=1 \
    -DBUILD_GEOMSERVER=0 \
    -DBUILD_EXAMPLES=0 \
    -DGLTF_SUPPORT=1 \
    -DCOLLADA_SUPPORT=0 \
    -DIFCXML_SUPPORT=1 \
    -DUSE_OCCT_STATIC=0 \
    -GNinja
RUN cmake --build build
RUN cmake --install build

ENV PATH="${IFCOPENSHELL_ROOT}/bin:${PATH}"

WORKDIR /app
COPY . .

# Generate JNI headers before building the native library
RUN javac -h native/include src/main/java/com/example/ifc/IfcOpenShellBridge.java

RUN cmake -S native -B native/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DIFCOPENSHELL_ROOT=${IFCOPENSHELL_ROOT} \
    -GNinja \
    && cmake --build native/build

RUN ./scripts/download-gradle-wrapper.sh \
    && ./gradlew --no-daemon clean shadowJar

########################################
# Runtime image
########################################
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive \
    IFCOPENSHELL_ROOT=/opt/ifcopenshell \
    DATA_DIR=/data

RUN apt-get update && apt-get install -y --no-install-recommends \
    openjdk-17-jre-headless \
    libgomp1 \
    libtbb12 \
    libgmp10 \
    libmpfr6 \
    libeigen3-dev \
    libboost-filesystem1.83.0 \
    libboost-system1.83.0 \
    libboost-program-options1.83.0 \
    libboost-thread1.83.0 \
    libocct-data-exchange-dev \
    libocct-foundation-dev \
    libocct-modeling-algorithms-dev \
    libocct-modeling-data-dev \
    libocct-ocaf-dev \
    libocct-visualization-dev \
    libopencollada0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder ${IFCOPENSHELL_ROOT} ${IFCOPENSHELL_ROOT}
COPY --from=builder /app/native/build/libifcglb_jni.so /usr/lib/libifcglb_jni.so
COPY --from=builder /app/build/libs/ifc-glb-service-all.jar /app/app.jar

ENV LD_LIBRARY_PATH="${IFCOPENSHELL_ROOT}/lib:/usr/lib/x86_64-linux-gnu:/usr/lib"

WORKDIR /app

RUN mkdir -p ${DATA_DIR}/in ${DATA_DIR}/out

EXPOSE 8080

ENTRYPOINT ["java", "-jar", "/app/app.jar"]

# syntax=docker/dockerfile:1.6

########################################
# Builder image
########################################
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    curl \
    unzip \
    openjdk-17-jdk \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir -p native/build

# Generate JNI headers
RUN javac -h native/include src/main/java/com/example/ifc/IfcOpenShellBridge.java

RUN cmake -S native -B native/build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build native/build --config Release

RUN ./scripts/download-gradle-wrapper.sh \
    && ./gradlew --no-daemon clean shadowJar

########################################
# Runtime image
########################################
FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    openjdk-17-jre-headless \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/libs/ifc-glb-service-all.jar /app/app.jar
COPY --from=builder /app/native/build/libifcglb_jni.so /usr/lib/libifcglb_jni.so

ENV LD_LIBRARY_PATH=/usr/lib
ENV DATA_DIR=/data

RUN mkdir -p /data/in /data/out

EXPOSE 8080

ENTRYPOINT ["java", "-jar", "/app/app.jar"]

# IFC to GLB Conversion Service

This project provides a Kotlin (Ktor) web service that accepts IFC uploads and converts them to GLB using an in-process JNI bridge. The service bundles a native integration of the IfcOpenShell geometry toolkit and performs the conversion without spawning external command line tools.

## Features

- **Single REST endpoint** `POST /convert` for uploading IFC files using `multipart/form-data`.
- Conversion is delegated to a JNI bridge (`libifcglb_jni.so`) that uses the IfcOpenShell C++ API and its built-in glTF serializer.
- Geometry options (units, tolerance, vertex welding, inclusion of properties, and level of detail) are configurable per request.
- Dockerfile and docker-compose configuration for containerised builds based on Ubuntu 24.04 that compile IfcOpenShell 0.8.0 and its dependencies from source.
- Optional `GET /health` endpoint for service health checks.

## Project structure

```
.
├── Dockerfile
├── docker-compose.yml
├── build.gradle.kts
├── settings.gradle.kts
├── gradlew
├── gradle/
├── native/
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── IfcToGltfConverter.h
│   │   └── ifcglb_jni.h
│   └── src/
│       ├── IfcToGltfConverter.cpp
│       └── ifcglb_jni.cpp
└── src/
    └── main/
        ├── java/com/example/ifc/IfcOpenShellBridge.java
        └── kotlin/com/example/ifc/
            ├── Application.kt
            ├── ConvertRoute.kt
            ├── ConvertService.kt
            └── Models.kt
```

## Build and run

> **Note:** The Gradle wrapper JAR is not committed. Run the helper script below or execute `gradle wrapper` locally to populate `gradle/wrapper/gradle-wrapper.jar` before building.

### 1. Download Gradle wrapper (if required)

```bash
./scripts/download-gradle-wrapper.sh
```

Alternatively, install Gradle locally and run `gradle wrapper` inside the repository.

### 2. Build with Docker

```bash
docker-compose build
```

The Docker build compiles OpenCascade, IfcOpenShell 0.8.0 (with glTF support), the JNI library, and the Ktor service inside the Ubuntu 24.04 builder stage image.

### 3. Start the service

```bash
docker-compose up
```

The API will be available at `http://localhost:8080`.

### 4. Convert an IFC file

```bash
curl -F "file=@/absolute/path/to/model.ifc" \
     "http://localhost:8080/convert?lod=medium&triangulation_tolerance=0.001"
```

The converted GLB artefact is written to `./data/out/<uuid>.glb` on the host.

## Environment variables

| Variable        | Default | Description                                |
|-----------------|---------|--------------------------------------------|
| `MAX_UPLOAD_MB` | `1024`  | Maximum upload size in megabytes            |
| `DATA_DIR`      | `/data` | Base directory for input/output subfolders |
| `IN_DIR`        | `/data/in` | Directory for temporary IFC uploads     |
| `OUT_DIR`       | `/data/out` | Directory for GLB outputs              |
| `LOG_LEVEL`     | `INFO`  | Logback logging level                      |

## Development notes

- The native converter now uses IfcOpenShell's glTF serializer to emit binary glTF (GLB) assets. The implementation is designed to work across multiple IfcOpenShell versions by probing available APIs at compile time.
- `native/CMakeLists.txt` resolves IfcOpenShell and OpenCascade libraries automatically. Override `IFCOPENSHELL_ROOT` when building locally to point at a custom installation.
- The Dockerfile demonstrates a complete toolchain build suitable for production containers. When iterating locally, ensure the same dependencies are available on the host before running `cmake`.

## Testing

Unit tests can be added under `src/test/kotlin`. At the moment there are no automated tests bundled with the repository.

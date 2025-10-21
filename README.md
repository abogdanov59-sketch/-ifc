# IFC to GLB Conversion Service

This project provides a Kotlin (Ktor) web service that accepts IFC uploads and converts them to GLB using an in-process JNI bridge. The native layer is intentionally lightweight in this repository and produces a placeholder GLB artefact. Integrating the full IfcOpenShell toolchain requires updating the native build to link against the IfcOpenShell and OCCT libraries inside the Docker image.

## Features

- **Single REST endpoint** `POST /convert` for uploading IFC files using `multipart/form-data`.
- Conversion is delegated to a JNI bridge (`libifcglb_jni.so`).
- Native converter writes a minimal GLB file to the configured `/data/out` directory.
- Dockerfile and docker-compose configuration for containerised builds.
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

- The native converter currently writes a placeholder GLB structure. Replace the implementation in `native/src/IfcToGltfConverter.cpp` with logic that leverages IfcOpenShell's geometry API and tinygltf for production use.
- Update `native/CMakeLists.txt` to locate and link against IfcOpenShell and OCCT libraries when integrating the full toolchain.
- The Dockerfile contains the high-level steps required for compiling IfcOpenShell from source; additional dependencies may be necessary.

## Testing

Unit tests can be added under `src/test/kotlin`. At the moment there are no automated tests bundled with the repository.

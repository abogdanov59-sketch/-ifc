#include "IfcToGltfConverter.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>

#include <ifcparse/IfcFile.h>

#if __has_include(<ifcgeom_schema_agnostic/IteratorSettings.h>)
#include <ifcgeom_schema_agnostic/IteratorSettings.h>
#include <ifcgeom_schema_agnostic/IfcGeomIterator.h>
#include <ifcgeom_schema_agnostic/Kernel.h>
#include <ifcgeom_schema_agnostic/serializers/Serializer.h>
#include <ifcgeom_schema_agnostic/serializers/SerializerFactory.h>
#include <ifcgeom_schema_agnostic/serializers/SerializerSettings.h>
#define IFCGEOM_NS IfcGeom::schema_agnostic
#elif __has_include(<ifcgeom/IteratorSettings.h>)
#include <ifcgeom/IteratorSettings.h>
#include <ifcgeom/IfcGeomIterator.h>
#include <ifcgeom/IfcGeomKernel.h>
#include <ifcgeom/IfcGeomSerializer.h>
#include <ifcgeom/IfcGeomSerializerFactory.h>
#include <ifcgeom/IfcGeomSerializerSettings.h>
#define IFCGEOM_NS IfcGeom
#else
#error "IfcOpenShell geometry headers not found. Ensure IfcOpenShell development files are installed."
#endif

#if __has_include(<ifcgeom_schema_agnostic/serializers/gltf/GltfSerializerSettings.h>)
#include <ifcgeom_schema_agnostic/serializers/gltf/GltfSerializerSettings.h>
#elif __has_include(<ifcgeom/serializers/gltf/GltfSerializerSettings.h>)
#include <ifcgeom/serializers/gltf/GltfSerializerSettings.h>
#endif

namespace {

IFCGEOM_NS::IteratorSettings::LOD resolveLod(const std::string &lod) {
#if defined(IFCGEOM_NS::IteratorSettings::LOD)
    if (lod == "low") {
        return IFCGEOM_NS::IteratorSettings::LOD::Low;
    }
    if (lod == "high") {
        return IFCGEOM_NS::IteratorSettings::LOD::High;
    }
    return IFCGEOM_NS::IteratorSettings::LOD::Medium;
#elif defined(IFCGEOM_NS::IteratorSettings::LOD_LOW)
    if (lod == "low") {
        return IFCGEOM_NS::IteratorSettings::LOD_LOW;
    }
    if (lod == "high") {
        return IFCGEOM_NS::IteratorSettings::LOD_HIGH;
    }
    return IFCGEOM_NS::IteratorSettings::LOD_MEDIUM;
#else
    (void)lod;
    return IFCGEOM_NS::IteratorSettings::LOD_MEDIUM;
#endif
}

void configureIteratorSettings(IFCGEOM_NS::IteratorSettings &settings, const ConverterOptions &options) {
    settings.set(IFCGEOM_NS::IteratorSettings::USE_WORLD_COORDS, true);
    settings.set(IFCGEOM_NS::IteratorSettings::APPLY_DEFAULT_MATERIALS, true);
    settings.set(IFCGEOM_NS::IteratorSettings::DISABLE_OPENING_SUBTRACTIONS, false);
    settings.set(IFCGEOM_NS::IteratorSettings::WELD_VERTICES, options.weldVertices);
    settings.set(IFCGEOM_NS::IteratorSettings::SEW_SHELLS, options.weldVertices);
    settings.set(IFCGEOM_NS::IteratorSettings::RETAIN_TRIANGULATION, true);
    settings.set(IFCGEOM_NS::IteratorSettings::GENERATE_UVS, true);
    settings.set(IFCGEOM_NS::IteratorSettings::ENABLE_TRIANGULATION_TOLERANCE, true);
    settings.set(IFCGEOM_NS::IteratorSettings::TRIANGULATION_TOLERANCE, options.triangulationTolerance);

#if defined(IFCGEOM_NS::IteratorSettings::LEVEL_OF_DETAIL)
    settings.set(IFCGEOM_NS::IteratorSettings::LEVEL_OF_DETAIL, resolveLod(options.lod));
#elif defined(IFCGEOM_NS::IteratorSettings::LOD_MEDIUM)
    settings.set(IFCGEOM_NS::IteratorSettings::LEVEL_OF_DETAIL, resolveLod(options.lod));
#endif
}

#if defined(IFCGEOM_NS::GltfSerializerSettings)
IFCGEOM_NS::GltfSerializerSettings buildGltfSettings(const ConverterOptions &options, const std::string &outputPath) {
    IFCGEOM_NS::GltfSerializerSettings gltfSettings;
    gltfSettings.set_output_path(outputPath);
    gltfSettings.set_embed_images(true);
    gltfSettings.set_embed_buffers(true);
    gltfSettings.set_include_properties(options.includeProperties);
    gltfSettings.set_export_binary(true);
    gltfSettings.set_units(options.units);
    return gltfSettings;
}
#endif

template <typename Iterator>
auto initialiseIterator(Iterator &iterator, int) -> decltype(iterator.initialize(), bool()) {
    return iterator.initialize();
}

template <typename Iterator>
bool initialiseIterator(Iterator &, long) {
    return true;
}

template <typename Iterator>
auto iteratorHasNext(Iterator &iterator, int) -> decltype(iterator.hasNext(), bool()) {
    return iterator.hasNext();
}

template <typename Iterator>
auto iteratorHasNext(Iterator &iterator, long) -> decltype(iterator.more(), bool()) {
    return iterator.more();
}

template <typename Iterator>
auto iteratorNext(Iterator &iterator, int) -> decltype(iterator.next(), void()) {
    iterator.next();
}

template <typename Iterator>
auto iteratorNext(Iterator &iterator, long) -> decltype(iterator.step(), void()) {
    iterator.step();
}

template <typename Iterator>
auto iteratorGeometry(Iterator &iterator, int) -> decltype(iterator.get()) {
    return iterator.get();
}

template <typename Iterator>
auto iteratorGeometry(Iterator &iterator, long) -> decltype(iterator.value()) {
    return iterator.value();
}

template <typename Geometry>
auto rawGeometryPointer(Geometry &geometry, int) -> decltype(geometry.get()) {
    return geometry.get();
}

template <typename Geometry>
Geometry *rawGeometryPointer(Geometry *geometry, long) {
    return geometry;
}

template <typename Serializer>
auto serializerOpen(Serializer &serializer, const std::string &path, int) -> decltype(serializer.open(path), void()) {
    serializer.open(path);
}

template <typename Serializer>
auto serializerOpen(Serializer &serializer, const std::string &path, long) -> decltype(serializer.Open(path), void()) {
    serializer.Open(path);
}

template <typename Serializer>
auto serializerClose(Serializer &serializer, int) -> decltype(serializer.close(), void()) {
    serializer.close();
}

template <typename Serializer>
auto serializerClose(Serializer &serializer, long) -> decltype(serializer.Close(), void()) {
    serializer.Close();
}

template <typename Serializer, typename Geometry>
auto serializerWrite(Serializer &serializer, Geometry *geometry, int) -> decltype(serializer.write(geometry), void()) {
    serializer.write(geometry);
}

template <typename Serializer, typename Geometry>
auto serializerWrite(Serializer &serializer, Geometry *geometry, long) -> decltype(serializer.Write(geometry), void()) {
    serializer.Write(geometry);
}

} // namespace

int IfcToGltfConverter::convert(const std::string &inputIfc, const std::string &outputGlb, const ConverterOptions &options) {
    if (!std::filesystem::exists(inputIfc)) {
        std::cerr << "Input IFC file does not exist: " << inputIfc << std::endl;
        return 1;
    }

    IfcParse::IfcFile ifcFile;
    try {
        if (!ifcFile.Init(inputIfc)) {
            std::cerr << "Failed to open IFC model: " << inputIfc << std::endl;
            return 2;
        }
    } catch (const std::exception &ex) {
        std::cerr << "Exception while loading IFC: " << ex.what() << std::endl;
        return 2;
    }

    try {
        std::filesystem::create_directories(std::filesystem::path(outputGlb).parent_path());
    } catch (const std::exception &ex) {
        std::cerr << "Failed to ensure output directory: " << ex.what() << std::endl;
        return 4;
    }

    IFCGEOM_NS::IteratorSettings iteratorSettings;
    configureIteratorSettings(iteratorSettings, options);

#if defined(IFCGEOM_NS::Kernel)
    IFCGEOM_NS::Kernel kernel;
#elif defined(IFCGEOM_NS::GeomKernel)
    IFCGEOM_NS::GeomKernel kernel;
#else
    auto kernel = IFCGEOM_NS::Kernel::create();
#endif

    std::unique_ptr<IFCGEOM_NS::Serializer> serializer;

#if defined(IFCGEOM_NS::SerializerFactory)
    try {
#if defined(IFCGEOM_NS::GltfSerializerSettings)
        auto gltfSettings = buildGltfSettings(options, outputGlb);
        serializer.reset(IFCGEOM_NS::SerializerFactory::create("glb", iteratorSettings, gltfSettings));
#else
        serializer.reset(IFCGEOM_NS::SerializerFactory::create("glb", iteratorSettings, outputGlb));
#endif
    } catch (const std::exception &ex) {
        std::cerr << "Failed to create glTF serializer: " << ex.what() << std::endl;
        return 5;
    }
#elif defined(IFCGEOM_NS::createSerializer)
    serializer.reset(IFCGEOM_NS::createSerializer("glb", iteratorSettings, outputGlb));
#else
#error "Unable to create IfcOpenShell glTF serializer. Update converter implementation for the installed IfcOpenShell version."
#endif

    if (!serializer) {
        std::cerr << "IfcOpenShell glTF serializer unavailable" << std::endl;
        return 5;
    }

    try {
        serializerOpen(*serializer, outputGlb, 0);
    } catch (const std::exception &ex) {
        std::cerr << "Failed to initialise serializer: " << ex.what() << std::endl;
        return 4;
    }

    try {
        IFCGEOM_NS::IfcGeomIterator iterator(&ifcFile, &kernel, iteratorSettings);
        initialiseIterator(iterator, 0);
        while (iteratorHasNext(iterator, 0)) {
            auto geometryHolder = iteratorGeometry(iterator, 0);
            if (!geometryHolder) {
                iteratorNext(iterator, 0);
                continue;
            }
            auto *rawGeometry = rawGeometryPointer(geometryHolder, 0L);
            serializerWrite(*serializer, rawGeometry, 0);
            iteratorNext(iterator, 0);
        }
        serializerClose(*serializer, 0);
    } catch (const std::exception &ex) {
        std::cerr << "Geometry conversion failed: " << ex.what() << std::endl;
        return 3;
    }

    if (!std::filesystem::exists(outputGlb)) {
        std::cerr << "GLB output not produced: " << outputGlb << std::endl;
        return 4;
    }

    return 0;
}


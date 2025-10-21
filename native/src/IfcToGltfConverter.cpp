#include "IfcToGltfConverter.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
constexpr const char *kMagic = "glTF";
constexpr uint32_t kVersion = 2;
constexpr uint32_t kJsonChunkType = 0x4E4F534A; // JSON
}

int IfcToGltfConverter::convert(const std::string &inputIfc, const std::string &outputGlb, const ConverterOptions &options) {
    (void)options; // Options are currently not used in the stub implementation.

    if (!std::filesystem::exists(inputIfc)) {
        std::cerr << "Input IFC file does not exist: " << inputIfc << std::endl;
        return 1;
    }

    try {
        std::filesystem::create_directories(std::filesystem::path(outputGlb).parent_path());
    } catch (const std::exception &ex) {
        std::cerr << "Failed to ensure output directory: " << ex.what() << std::endl;
        return 4;
    }

    // Minimal glTF JSON payload.
    std::string json = R"({"asset":{"version":"2.0","generator":"IfcToGltfConverterStub"},"scene":0,"scenes":[{"nodes":[]}],"nodes":[]})";
    while (json.size() % 4 != 0) {
        json.push_back(' ');
    }

    const uint32_t jsonLength = static_cast<uint32_t>(json.size());
    const uint32_t totalLength = 12 + 8 + jsonLength;

    std::ofstream output(outputGlb, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        std::cerr << "Failed to open output file: " << outputGlb << std::endl;
        return 4;
    }

    output.write(kMagic, 4);
    output.write(reinterpret_cast<const char *>(&kVersion), sizeof(uint32_t));
    output.write(reinterpret_cast<const char *>(&totalLength), sizeof(uint32_t));
    output.write(reinterpret_cast<const char *>(&jsonLength), sizeof(uint32_t));
    output.write(reinterpret_cast<const char *>(&kJsonChunkType), sizeof(uint32_t));
    output.write(json.data(), static_cast<std::streamsize>(json.size()));

    if (!output.good()) {
        std::cerr << "Failed to write GLB output file: " << outputGlb << std::endl;
        return 4;
    }

    return 0;
}


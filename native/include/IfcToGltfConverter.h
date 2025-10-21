#pragma once

#include <string>

struct ConverterOptions {
    std::string units = "meter";
    double triangulationTolerance = 0.001;
    bool weldVertices = true;
    bool includeProperties = false;
    std::string lod = "medium";
};

class IfcToGltfConverter {
public:
    int convert(const std::string &inputIfc, const std::string &outputGlb, const ConverterOptions &options);
};


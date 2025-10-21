#include "ifcglb_jni.h"
#include "IfcToGltfConverter.h"

#include <jni.h>

#include <cctype>
#include <exception>
#include <iostream>
#include <string>

namespace {

std::string jstringToString(JNIEnv *env, jstring value) {
    if (value == nullptr) {
        return {};
    }
    const char *utf = env->GetStringUTFChars(value, nullptr);
    if (!utf) {
        return {};
    }
    std::string result(utf);
    env->ReleaseStringUTFChars(value, utf);
    return result;
}

ConverterOptions parseOptions(const std::string &json) {
    ConverterOptions options;
    if (json.empty()) {
        return options;
    }

    auto extractString = [&](const std::string &key) -> std::string {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return {};
        pos = json.find(':', pos);
        if (pos == std::string::npos) return {};
        pos = json.find('"', pos);
        if (pos == std::string::npos) return {};
        auto end = json.find('"', pos + 1);
        if (end == std::string::npos) return {};
        return json.substr(pos + 1, end - pos - 1);
    };

    auto extractNumber = [&](const std::string &key, double fallback) -> double {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return fallback;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return fallback;
        auto end = json.find_first_of(",}\n\r", pos + 1);
        auto token = json.substr(pos + 1, end - (pos + 1));
        try {
            return std::stod(token);
        } catch (...) {
            return fallback;
        }
    };

    auto extractBool = [&](const std::string &key, bool fallback) -> bool {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return fallback;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return fallback;
        auto end = json.find_first_of(",}\n\r", pos + 1);
        auto token = json.substr(pos + 1, end - (pos + 1));
        std::string lowered;
        lowered.reserve(token.size());
        for (char ch : token) {
            lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        if (lowered.find("true") != std::string::npos || lowered.find('1') != std::string::npos)
            return true;
        if (lowered.find("false") != std::string::npos || lowered.find('0') != std::string::npos)
            return false;
        return fallback;
    };

    auto units = extractString("units");
    if (!units.empty()) {
        options.units = units;
    }

    options.triangulationTolerance = extractNumber("triangulationTolerance", options.triangulationTolerance);
    options.triangulationTolerance = extractNumber("triangulation_tolerance", options.triangulationTolerance);

    options.weldVertices = extractBool("weldVertices", options.weldVertices);
    options.includeProperties = extractBool("includeProperties", options.includeProperties);
    auto lod = extractString("lod");
    if (!lod.empty()) {
        options.lod = lod;
    }

    return options;
}

}

JNIEXPORT jint JNICALL Java_com_example_ifc_IfcOpenShellBridge_convertIfcToGlb(
    JNIEnv *env,
    jclass /*clazz*/,
    jstring jInputPath,
    jstring jOutputPath,
    jstring jOptionsJson) {
    try {
        const std::string inputPath = jstringToString(env, jInputPath);
        const std::string outputPath = jstringToString(env, jOutputPath);
        const std::string optionsJson = jstringToString(env, jOptionsJson);

        ConverterOptions options = parseOptions(optionsJson);

        IfcToGltfConverter converter;
        return converter.convert(inputPath, outputPath, options);
    } catch (const std::exception &ex) {
        std::cerr << "Exception in JNI bridge: " << ex.what() << std::endl;
        return 100;
    } catch (...) {
        std::cerr << "Unknown exception in JNI bridge" << std::endl;
        return 100;
    }
}


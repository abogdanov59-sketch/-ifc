cmake_minimum_required(VERSION 3.20)
project(ifcglb_jni LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(OpenCASCADE REQUIRED COMPONENTS
    TKernel
    TKMath
    TKBRep
    TKGeomBase
    TKGeomAlgo
    TKService
    TKV3d
    TKMesh
    TKShHealing
    TKSTEP
    TKSTEP209
    TKSTEPAttr
    TKSTEPBase
)

find_path(IFCOPENSHELL_INCLUDE_DIR
    NAMES ifcparse/IfcFile.h
    HINTS
        $ENV{IFCOPENSHELL_ROOT}/include
        /usr/local/include
        /usr/include
)

find_library(IFCOPENSHELL_C_LIBRARY
    NAMES ifcopenshell-c ifcopenshell
    HINTS
        $ENV{IFCOPENSHELL_ROOT}/lib
        /usr/local/lib
        /usr/lib
        /usr/lib/x86_64-linux-gnu
)

find_library(IFCOPENSHELL_GEOM_LIBRARY
    NAMES ifcgeom_schema_agnostic ifcgeom
    HINTS
        $ENV{IFCOPENSHELL_ROOT}/lib
        /usr/local/lib
        /usr/lib
        /usr/lib/x86_64-linux-gnu
)

find_library(IFCOPENSHELL_GLTFSERIALIZER_LIBRARY
    NAMES ifcglTF
    HINTS
        $ENV{IFCOPENSHELL_ROOT}/lib
        /usr/local/lib
        /usr/lib
        /usr/lib/x86_64-linux-gnu
)

if (NOT IFCOPENSHELL_INCLUDE_DIR OR NOT IFCOPENSHELL_C_LIBRARY OR NOT IFCOPENSHELL_GEOM_LIBRARY)
    message(FATAL_ERROR "IfcOpenShell headers or libraries were not found. Set IFCOPENSHELL_ROOT or provide explicit paths.")
endif()

set(IFCOPENSHELL_LIBRARIES
    ${IFCOPENSHELL_C_LIBRARY}
    ${IFCOPENSHELL_GEOM_LIBRARY}
)

if (IFCOPENSHELL_GLTFSERIALIZER_LIBRARY)
    list(APPEND IFCOPENSHELL_LIBRARIES ${IFCOPENSHELL_GLTFSERIALIZER_LIBRARY})
endif()

add_library(ifcglb_jni SHARED
    src/ifcglb_jni.cpp
    src/IfcToGltfConverter.cpp
)

target_include_directories(ifcglb_jni
    PRIVATE
        include
        ${IFCOPENSHELL_INCLUDE_DIR}
)

target_link_libraries(ifcglb_jni
    PRIVATE
        ${IFCOPENSHELL_LIBRARIES}
        OpenCASCADE::TKernel
        OpenCASCADE::TKMath
        OpenCASCADE::TKBRep
        OpenCASCADE::TKGeomBase
        OpenCASCADE::TKGeomAlgo
        OpenCASCADE::TKService
        OpenCASCADE::TKV3d
        OpenCASCADE::TKMesh
        OpenCASCADE::TKShHealing
        OpenCASCADE::TKSTEP
        OpenCASCADE::TKSTEP209
        OpenCASCADE::TKSTEPAttr
        OpenCASCADE::TKSTEPBase
)

install(TARGETS ifcglb_jni DESTINATION lib)

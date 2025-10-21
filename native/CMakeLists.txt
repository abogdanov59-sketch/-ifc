cmake_minimum_required(VERSION 3.16)
project(ifcglb_jni LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(ifcglb_jni SHARED
    src/ifcglb_jni.cpp
    src/IfcToGltfConverter.cpp
)

target_include_directories(ifcglb_jni
    PRIVATE
        include
)

# Placeholder for IfcOpenShell and OCCT dependencies. Actual paths should be
# configured via CMake options or environment variables when available.
if (DEFINED IFCOPENSHELL_INCLUDE_DIRS)
    target_include_directories(ifcglb_jni PRIVATE ${IFCOPENSHELL_INCLUDE_DIRS})
endif()

if (DEFINED IFCOPENSHELL_LIBRARIES)
    target_link_libraries(ifcglb_jni PRIVATE ${IFCOPENSHELL_LIBRARIES})
endif()

if (DEFINED OCCT_LIBRARIES)
    target_link_libraries(ifcglb_jni PRIVATE ${OCCT_LIBRARIES})
endif()

install(TARGETS ifcglb_jni DESTINATION lib)

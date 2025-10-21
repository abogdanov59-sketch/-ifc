#pragma once

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Java_com_example_ifc_IfcOpenShellBridge_convertIfcToGlb(
    JNIEnv *env,
    jclass clazz,
    jstring jInputPath,
    jstring jOutputPath,
    jstring jOptionsJson);

#ifdef __cplusplus
}
#endif


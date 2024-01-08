#include <iostream>
#include <string>
#include <jni.h>
#include "preload.h"

jfieldID getField(JNIEnv* env, const std::string& className, const std::string& fieldName, const std::string& fieldType) {
    jclass cls = env->FindClass(className.c_str());
    if (!cls) {
        std::cerr << "Class '" << className << "' does not exist" << std::endl;
        return nullptr;
    }

    jfieldID fieldID = env->GetFieldID(cls, fieldName.c_str(), fieldType.c_str());
    if (!fieldID) {
        std::cerr << "Class '" << className << "' does not have field '" << fieldName << "' of type '" << fieldType << "'" << std::endl;
        return nullptr;
    }

    return fieldID;
}

CanvasFields canvasFields = {};
MatrixFields matrixFields = {};
FrameCanvasFields frameCanvasFields = {};

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR; // JNI version not supported.
    }

    canvasFields.nativePtr = getField(env, "net/frozenbit/ledmatrix/driver/Canvas",  "nativePtr", "J");
    if (!canvasFields.nativePtr) {
        return JNI_ERR;
    }

    matrixFields.nativePtr = getField(env, "net/frozenbit/ledmatrix/driver/RGBMatrix",  "nativePtr", "J");
    if (!matrixFields.nativePtr) {
        return JNI_ERR;
    }

    frameCanvasFields.nativePtr = getField(env, "net/frozenbit/ledmatrix/driver/FrameCanvas",  "nativePtr", "J");
    if (!frameCanvasFields.nativePtr) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
}

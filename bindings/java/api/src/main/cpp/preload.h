#ifndef PRELOAD_H
#define PRELOAD_H

#include <iostream>
#include <string>
#include <jni.h>

jfieldID getField(JNIEnv* env, const std::string& className, const std::string& fieldName, const std::string& fieldType);

struct CanvasFields {
  jfieldID nativePtr;
};
extern CanvasFields canvasFields;

struct MatrixFields {
  jfieldID nativePtr;
};
extern MatrixFields matrixFields;

struct FrameCanvasFields {
  jfieldID nativePtr;
};
extern FrameCanvasFields frameCanvasFields;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved);

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved);

#endif

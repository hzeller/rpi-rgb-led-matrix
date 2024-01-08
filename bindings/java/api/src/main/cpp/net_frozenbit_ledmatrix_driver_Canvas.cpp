#include <iostream>
#include <jni.h>
#include "net_frozenbit_ledmatrix_driver_Canvas.h"
#include "canvas.h"
#include "preload.h"

rgb_matrix::Canvas* GetCanvas(JNIEnv *env, jobject obj) {
    jlong ptr = env->GetLongField(obj, canvasFields.nativePtr);
    return reinterpret_cast<rgb_matrix::Canvas *>(ptr);
}

JNIEXPORT jint JNICALL Java_net_frozenbit_ledmatrix_driver_Canvas_getWidth
  (JNIEnv *env, jobject obj) {
      rgb_matrix::Canvas *canvas = GetCanvas(env, obj);
      return canvas->width();
}

JNIEXPORT jint JNICALL Java_net_frozenbit_ledmatrix_driver_Canvas_getHeight
  (JNIEnv *env, jobject obj) {
      rgb_matrix::Canvas *canvas = GetCanvas(env, obj);
      return canvas->height();
}

JNIEXPORT void JNICALL Java_net_frozenbit_ledmatrix_driver_Canvas_setPixel
  (JNIEnv *env, jobject obj, jint x, jint y, jint red, jint green, jint blue) {
      rgb_matrix::Canvas *canvas = GetCanvas(env, obj);
      canvas->SetPixel(x, y, red, green, blue);
}

JNIEXPORT void JNICALL Java_net_frozenbit_ledmatrix_driver_Canvas_clear
  (JNIEnv *env, jobject obj) {
      rgb_matrix::Canvas *canvas = GetCanvas(env, obj);
      canvas->Clear();
}

JNIEXPORT void JNICALL Java_net_frozenbit_ledmatrix_driver_Canvas_fill
  (JNIEnv *env, jobject obj, jint red, jint green, jint blue) {
      rgb_matrix::Canvas *canvas = GetCanvas(env, obj);
      canvas->Fill(red, green, blue);
}

#include "net_frozenbit_ledmatrix_driver_FrameCanvas.h"
#include <jni.h>
#include <led-matrix.h>
#include "preload.h"

rgb_matrix::FrameCanvas* GetFrameCanvas(JNIEnv *env, jobject obj) {
    jlong ptr = env->GetLongField(obj, frameCanvasFields.nativePtr);
    return reinterpret_cast<rgb_matrix::FrameCanvas *>(ptr);
}

// Implementing setPwmBits
JNIEXPORT jboolean JNICALL Java_net_frozenbit_ledmatrix_driver_FrameCanvas_setPwmBits
  (JNIEnv *env, jobject obj, jint pwmBits) {
    rgb_matrix::FrameCanvas *canvas = GetFrameCanvas(env, obj);
    return static_cast<jboolean>(canvas->SetPWMBits(static_cast<uint8_t>(pwmBits)));
}

// Implementing getPwmBits
JNIEXPORT jint JNICALL Java_net_frozenbit_ledmatrix_driver_FrameCanvas_getPwmBits
  (JNIEnv *env, jobject obj) {
    rgb_matrix::FrameCanvas *canvas = GetFrameCanvas(env, obj);
    return static_cast<jint>(canvas->pwmbits());
}

// Implementing setLuminanceCorrect
JNIEXPORT void JNICALL Java_net_frozenbit_ledmatrix_driver_FrameCanvas_setLuminanceCorrect
  (JNIEnv *env, jobject obj, jboolean luminanceCorrect) {
    rgb_matrix::FrameCanvas *canvas = GetFrameCanvas(env, obj);
    canvas->set_luminance_correct(static_cast<bool>(luminanceCorrect));
}

// Implementing isLuminanceCorrect
JNIEXPORT jboolean JNICALL Java_net_frozenbit_ledmatrix_driver_FrameCanvas_isLuminanceCorrect
  (JNIEnv *env, jobject obj) {
    rgb_matrix::FrameCanvas *canvas = GetFrameCanvas(env, obj);
    return static_cast<jboolean>(canvas->luminance_correct());
}

// Implementing setBrightness
JNIEXPORT void JNICALL Java_net_frozenbit_ledmatrix_driver_FrameCanvas_setBrightness
  (JNIEnv *env, jobject obj, jint brightness) {
    rgb_matrix::FrameCanvas *canvas = GetFrameCanvas(env, obj);
    canvas->SetBrightness(static_cast<uint8_t>(brightness));
}

// Implementing getBrightness
JNIEXPORT jint JNICALL Java_net_frozenbit_ledmatrix_driver_FrameCanvas_getBrightness
  (JNIEnv *env, jobject obj) {
    rgb_matrix::FrameCanvas *canvas = GetFrameCanvas(env, obj);
    return static_cast<jint>(canvas->brightness());
}

// Implementing copyFrom
JNIEXPORT void JNICALL Java_net_frozenbit_ledmatrix_driver_FrameCanvas_copyFrom
  (JNIEnv *env, jobject obj, jobject other) {
    rgb_matrix::FrameCanvas *canvas = GetFrameCanvas(env, obj);
    rgb_matrix::FrameCanvas *otherCanvas = GetFrameCanvas(env, other);
    canvas->CopyFrom(*otherCanvas);
}

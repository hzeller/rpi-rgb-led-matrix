#include <iostream>
#include <jni.h>
#include <led-matrix.h>
#include "net_frozenbit_ledmatrix_driver_RGBMatrix.h"
#include "preload.h"

int unboxInteger(JNIEnv *env, jobject integerObject) {
    jclass integerClass = env->FindClass("java/lang/Integer");
    jmethodID intValue = env->GetMethodID(integerClass, "intValue", "()I");
    return env->CallIntMethod(integerObject, intValue);
}

bool unboxBoolean(JNIEnv *env, jobject booleanObject) {
    jclass booleanClass = env->FindClass("java/lang/Boolean");
    jmethodID booleanValue = env->GetMethodID(booleanClass, "booleanValue", "()Z");
    return env->CallBooleanMethod(booleanObject, booleanValue);
}

const char* getStringChars(JNIEnv *env, jobject jstrObj) {
    jstring jstr = (jstring) jstrObj;
    return jstr ? env->GetStringUTFChars(jstr, NULL) : NULL;
}

rgb_matrix::RGBMatrix* GetRGBMatrix(JNIEnv *env, jobject obj) {
    jlong ptr = env->GetLongField(obj, matrixFields.nativePtr);
    return reinterpret_cast<rgb_matrix::RGBMatrix *>(ptr);
}

#define HANDLE_OPTION(cpp_name, optionsObject, java_name, type, unboxer) { \
    jclass optionsClass = env->GetObjectClass(optionsObject);\
    jfieldID fid = env->GetFieldID(optionsClass, java_name, "Ljava/lang/" type ";"); \
    if (fid) {\
      jobject obj = env->GetObjectField(optionsObject, fid); \
      if (obj) cpp_name = unboxer(env, obj);\
    } else {\
      std::cerr << "WARN: Options object is missing field '" java_name "', ignoring..." << std::endl; \
    }\
}

JNIEXPORT jobject JNICALL Java_net_frozenbit_ledmatrix_driver_RGBMatrix_createFromOptions
(JNIEnv *env, jclass rgbMatrixClass, jobject optionsObject, jobject runtimeOptions) {
    // Return null if optionsObject is null
    if (optionsObject == nullptr) {
        return nullptr;
    }


    // Populate the RGBMatrix::Options struct
    rgb_matrix::RGBMatrix::Options cOptions;

    HANDLE_OPTION(cOptions.hardware_mapping, optionsObject, "hardwareMapping", "String", getStringChars)
    HANDLE_OPTION(cOptions.rows, optionsObject, "rows", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.cols, optionsObject, "cols", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.chain_length, optionsObject, "chainLength", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.parallel, optionsObject, "parallel", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.pwm_bits, optionsObject, "pwmBits", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.pwm_lsb_nanoseconds, optionsObject, "pwmLsbNanoseconds", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.pwm_dither_bits, optionsObject, "pwmDitherBits", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.brightness, optionsObject, "brightness", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.scan_mode, optionsObject, "scanMode", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.row_address_type, optionsObject, "rowAddressType", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.multiplexing, optionsObject, "multiplexing", "Integer", unboxInteger)
    HANDLE_OPTION(cOptions.disable_hardware_pulsing, optionsObject, "disableHardwarePulsing", "Boolean", unboxBoolean)
    HANDLE_OPTION(cOptions.show_refresh_rate, optionsObject, "showRefreshRate", "Boolean", unboxBoolean)
    HANDLE_OPTION(cOptions.inverse_colors, optionsObject, "inverseColors", "Boolean", unboxBoolean)
    HANDLE_OPTION(cOptions.led_rgb_sequence, optionsObject, "ledRgbSequence", "String", getStringChars)
    HANDLE_OPTION(cOptions.pixel_mapper_config, optionsObject, "pixelMapperConfig", "String", getStringChars)
    HANDLE_OPTION(cOptions.panel_type, optionsObject, "panelType", "String", getStringChars)
    HANDLE_OPTION(cOptions.limit_refresh_rate_hz, optionsObject, "limitRefreshRateHz", "Integer", unboxInteger)

    // Populate the RGBMatrix::Options struct
    rgb_matrix::RuntimeOptions cRuntimeOptions;
    if (runtimeOptions) {
        HANDLE_OPTION(cRuntimeOptions.gpio_slowdown, runtimeOptions, "gpioSlowdown", "Integer", unboxInteger);
        HANDLE_OPTION(cRuntimeOptions.daemon, runtimeOptions, "daemon", "Integer", unboxInteger);
        HANDLE_OPTION(cRuntimeOptions.drop_privileges, runtimeOptions, "dropPrivileges", "Integer", unboxInteger);
        HANDLE_OPTION(cRuntimeOptions.do_gpio_init, runtimeOptions, "doGpioInit", "Boolean", unboxBoolean);
        HANDLE_OPTION(cRuntimeOptions.drop_priv_user, runtimeOptions, "dropPrivUser", "String", getStringChars);
        HANDLE_OPTION(cRuntimeOptions.drop_priv_group, runtimeOptions, "dropPrivGroup", "String", getStringChars);
    }

    // Create the RGBMatrix instance using the populated options struct
    rgb_matrix::RGBMatrix *matrix = rgb_matrix::RGBMatrix::CreateFromOptions(cOptions, cRuntimeOptions);

    // Convert the native pointer to a long for Java
    jlong matrixPtr = reinterpret_cast<jlong>(matrix);

    // Create and return the new RGBMatrix Java object with the native pointer
    jmethodID constructor = env->GetMethodID(rgbMatrixClass, "<init>", "(J)V");
    jobject newMatrixObject = env->NewObject(rgbMatrixClass, constructor, matrixPtr);

    return newMatrixObject;
}

// JNI method for RGBMatrix.delete()
JNIEXPORT void JNICALL Java_net_frozenbit_ledmatrix_driver_RGBMatrix_delete
  (JNIEnv *env, jobject obj) {

    rgb_matrix::RGBMatrix* matrix = GetRGBMatrix(env, obj);

    delete matrix;

    // set the nativeMatrixPtr to 0 in the Java object to prevent reuse
    env->SetLongField(obj, matrixFields.nativePtr, (jlong)0);
}


/*
 * Class:     net_frozenbit_ledmatrix_driver_RGBMatrix
 * Method:    createFrameCanvas
 * Signature: ()Lnet/frozenbit/ledmatrix/driver/FrameCanvas;
 */
JNIEXPORT jobject JNICALL Java_net_frozenbit_ledmatrix_driver_RGBMatrix_createFrameCanvas
  (JNIEnv *env, jobject obj) {
      rgb_matrix::RGBMatrix* matrix = GetRGBMatrix(env, obj);

      // Convert the native pointer to a long for Java
      jlong canvasPtr = reinterpret_cast<jlong>(matrix->CreateFrameCanvas());

      // Create and return the new RGBMatrix Java object with the native pointer
      jclass frameCanvasClass = env->FindClass("net/frozenbit/ledmatrix/driver/FrameCanvas");
      jmethodID constructor = env->GetMethodID(frameCanvasClass, "<init>", "(J)V");
      return env->NewObject(frameCanvasClass, constructor, canvasPtr);
}

/*
 * Class:     net_frozenbit_ledmatrix_driver_RGBMatrix
 * Method:    swapOnVSync
 * Signature: (Lnet/frozenbit/ledmatrix/driver/FrameCanvas;I)Lnet/frozenbit/ledmatrix/driver/FrameCanvas;
 */
JNIEXPORT void JNICALL Java_net_frozenbit_ledmatrix_driver_RGBMatrix_swapOnVSync
  (JNIEnv *env, jobject matrixWrapper, jobject nextFrameCanvas, jint frameRateFraction) {
      rgb_matrix::RGBMatrix* matrix = GetRGBMatrix(env, matrixWrapper);

      jlong ptr = env->GetLongField(nextFrameCanvas, frameCanvasFields.nativePtr);
      rgb_matrix::FrameCanvas *nextFrame = reinterpret_cast<rgb_matrix::FrameCanvas *>(ptr);

      matrix->SwapOnVSync(nextFrame, frameRateFraction);
}

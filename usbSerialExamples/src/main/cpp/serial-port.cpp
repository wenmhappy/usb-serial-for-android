#include <jni.h>

//
// Created by wenmhappy on 2024/12/10.
// 演示如何通过c函数向usb串口发送数据以及
// 接收usb串口返回的数据
//
#include <thread>
#include <mutex>
#include <condition_variable>
#include "usb-serial.h"

#include "android/log.h"
static const char *TAG="serial_port";
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO,  TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##args)

#define BUF_SIZE 8096
static char buf[BUF_SIZE];
static int data_len;
static std::mutex mtx;
static std::condition_variable cv;

static JNIEnv *g_env = nullptr;
static jobject g_obj = nullptr;

// java 对象 TerminalFragment 的 receiveNative方法的id
static jmethodID receiveNative;

extern "C"
void send_respose_to_java(char *data, int len) {
    LOGD("send_respose_to_java %d bytes\n", len);

    // 不要在该函数中直接处理数据
    {
        std::lock_guard<std::mutex> lock(mtx);
        memcpy(buf, data, len);
        data_len = len;
    }
    cv.notify_one();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hoho_android_usbserial_examples_TerminalFragment_sendNative(JNIEnv *env, jobject thiz,
                                                                     jbyteArray data) {
    // TODO: implement sendNative()
    jsize length = env->GetArrayLength(data);
    jbyte *bytes = env->GetByteArrayElements(data, NULL);

    usb_send((char *)bytes, (int )length);
    env->ReleaseByteArrayElements(data, bytes, 0);

}

static bool quit;

extern "C"
JNIEXPORT void JNICALL
Java_com_hoho_android_usbserial_examples_TerminalFragment_initNative(JNIEnv *env, jobject thiz) {
    // TODO: implement initNative()
    g_env = env;
    g_obj = thiz;
    quit = false;

    jclass cls = env->GetObjectClass(thiz);
    receiveNative = env->GetMethodID(cls,"receiveNative","([B)V");

    usb_set_receive_callback(send_respose_to_java);

    // 等待处理 usb串口返回的数据
    while (true) {
        std::unique_lock<std::mutex> locker(mtx);
        cv.wait(locker/*, []{ return !buffer.empty(); }*/);

        if (quit)
            break;

        jbyteArray array = g_env->NewByteArray(data_len);
        g_env->SetByteArrayRegion(array, 0, data_len, (const jbyte *) buf);

        // 将数据返回到 java对象中以便更新 ui
        g_env->CallVoidMethod(g_obj, receiveNative, array);
        g_env->DeleteLocalRef(array);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hoho_android_usbserial_examples_TerminalFragment_destroyNative(JNIEnv *env, jobject thiz) {
    // TODO: implement destroyNative()
    quit = true;
    cv.notify_one();
}
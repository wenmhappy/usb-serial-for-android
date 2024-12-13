#include <jni.h>

//
// Created by wenmhappy on 2024/12/11.
// 接收来自其他 c代码的数据，将其发送至java中，再由java发送至usb串口；
// 接收 usb串口经由 java 传来的数据，并通过回调函数发往其他 c代码中
//
#include <thread>
#include <mutex>
#include <condition_variable>

typedef void (*ReceiveCallback)(char *data, int len);
ReceiveCallback receiveCallback = nullptr;

static JNIEnv *g_env;
static jobject g_obj;

// java 对象UsbSerialWrapper 的send方法的id
static jmethodID send;

#define BUF_SIZE (32 * 1024)
static char buf[BUF_SIZE];
static int data_len;
static std::mutex mtx;
static std::condition_variable cv;

extern "C" {
    // 发送数据到 usb串口
void usb_send(char *data, int len) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        memcpy(buf, data, len);
        data_len = len;
    }
    cv.notify_one();  // 通知 java线程处理数据
}

// 注册处理usb串口返回数据的回调
void usb_set_receive_callback(ReceiveCallback callback) {
    receiveCallback = callback;
}

}

static bool quit = false;

// 处理来自c/c++代码发送至usb串口的数据
static void handle_sent_data() {
    while (true) {
        std::unique_lock<std::mutex> locker(mtx);
        cv.wait(locker/*, []{ return !buffer.empty(); }*/);

        if (quit)
            break;

        jbyteArray array = g_env->NewByteArray(data_len);
        g_env->SetByteArrayRegion(array, 0, data_len, (const jbyte *) buf);

        // 把数据发送到 java中
        g_env->CallVoidMethod(g_obj, send, array);
        g_env->DeleteLocalRef(array);

    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hoho_android_usbserial_wrapper_UsbSerialWrapper_sendToNative(JNIEnv *env, jobject thiz,
                                                                   jbyteArray data) {
    // TODO: implement sendToNative()
    if (!receiveCallback) {
        return;
    }

    jsize length = env->GetArrayLength(data);
    jbyte *bytes = env->GetByteArrayElements(data, NULL);

    receiveCallback((char *)bytes, (int)length);

    env->ReleaseByteArrayElements(data, bytes, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hoho_android_usbserial_wrapper_UsbSerialWrapper_initNative(JNIEnv *env, jobject thiz) {
    // TODO: implement initNative()
    g_env = env;
    g_obj = thiz;
    quit = false;

    jclass cls = env->GetObjectClass(thiz);
    // 开启终端窗口，执行 javap -s '.\usbSerialForAndroid\build\intermediates\javac\debug\classes\com\hoho\android\
    // usbserial\util\UsbSerialWrapper.class' 查看方法签名
    send = env->GetMethodID(cls,"send","([B)V");

    handle_sent_data();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_hoho_android_usbserial_wrapper_UsbSerialWrapper_destroyNative(JNIEnv *env, jobject thiz) {
    // TODO: implement destroyNative()
    quit = true;
    cv.notify_one();
}
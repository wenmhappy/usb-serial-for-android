#include <jni.h>

//
// Created by wenmhappy on 2024/12/11.
//
typedef void (*ReceiveCallback)(char *data, int len);
ReceiveCallback receiveCallback = nullptr;

JNIEnv *g_env = nullptr;
jobject g_obj = nullptr;

// java 对象UsbSerialWrapper 的send方法的id
jmethodID send;

extern "C" {
void usb_send(char *data, int len) {
    jbyteArray array = g_env->NewByteArray(len);
    g_env->SetByteArrayRegion(array, 0, len, (const jbyte *) data);

    g_env->CallVoidMethod(g_obj, send, array);
}

void usb_set_receive_callback(ReceiveCallback callback) {
    receiveCallback = callback;
}

}

extern "C"
JNIEXPORT void JNICALL
Java_com_hoho_android_usbserial_util_UsbSerialWrapper_sendToNative(JNIEnv *env, jobject thiz,
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
Java_com_hoho_android_usbserial_util_UsbSerialWrapper_initNative(JNIEnv *env, jobject thiz) {
    // TODO: implement initNative()
    g_env = env;
    g_obj = thiz;

    jclass cls = env->GetObjectClass(thiz);
    // 开启终端窗口，执行 javap -s '.\usbSerialForAndroid\build\intermediates\javac\debug\classes\com\hoho\android\
    // usbserial\util\UsbSerialWrapper.class' 查看方法签名
    send = env->GetMethodID(cls,"send","([B)V");
}
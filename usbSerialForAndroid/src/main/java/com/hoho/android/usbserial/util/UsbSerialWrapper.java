package com.hoho.android.usbserial.util;

public class UsbSerialWrapper {
    public native void sendToNative(byte[] data);
    public native void initNative();

    public void send(byte[] data) {
        try {
            Thread.sleep(1000);
            sendToNative(data);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    static {
        System.loadLibrary("usb-serial");
    }
}

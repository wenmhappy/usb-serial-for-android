package com.hoho.android.usbserial.examples;

import android.util.Log;

public class UsbSerialWrapper implements Runnable {
    private final String TAG = "UsbSerialWrapper";

    public UsbSerialWrapper() {

    }
    public native void sendToNative(byte[] data);
    private native void initNative();

    public void send(byte[] data) {
        Log.d(TAG, "sending data");
        /*
        try {
            Thread.sleep(1000);
            sendToNative(data);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }*/
        new Thread(() -> {
            sendToNative(data);
        }).start();
    }

    public void connect() {
        new Thread(this).start();
    }

    static {
        System.loadLibrary("usb-serial");
    }

    @Override
    public void run() {
        initNative();
    }
}

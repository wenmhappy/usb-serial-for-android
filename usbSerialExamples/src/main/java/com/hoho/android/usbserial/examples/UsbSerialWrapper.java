package com.hoho.android.usbserial.examples;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.util.Log;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;

public class UsbSerialWrapper implements Runnable {
    private final String TAG = "UsbSerialWrapper";

    public enum UsbPermission { Unknown, Requested, Granted, Denied }
    public static final int WRITE_WAIT_MILLIS = 2000;
    public static final int READ_WAIT_MILLIS = 2000;

    public static final String INTENT_ACTION_GRANT_USB = "UsbSerialWrapper.GRANT_USB";
    public int deviceId, portNum, baudRate;
    public boolean withIoManager;

    private Context context;
    private SerialInputOutputManager usbIoManager;
    private UsbSerialPort usbSerialPort;
    private boolean connected = false;
    private UsbPermission usbPermission = UsbPermission.Unknown;

    private SerialInputOutputManager.Listener listener;

    public UsbSerialWrapper(Context context, SerialInputOutputManager.Listener listener) {
        this.context = context;
        this.listener = listener;
    }

    public native void sendToNative(byte[] data);
    private native void initNative();

    public void send(byte[] data) {
        Log.d(TAG, "sending data");

        if (!connected) {
            return;
        }

        if (usbSerialPort != null) {
            try {
                usbSerialPort.write(data, WRITE_WAIT_MILLIS);
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        } else {
            new Thread(() -> {
                sendToNative(data);
            }).start();
        }
    }

    public UsbSerialPort getUsbSerialPort() {
        return usbSerialPort;
    }

    public UsbPermission getPermission() {
        return usbPermission;
    }

    public void setPermission(UsbPermission permission) {
        usbPermission = permission;
    }

    public boolean isConnected() { return connected; }

    public void connect() throws Exception {
        UsbDevice device = null;
        UsbManager usbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        for(UsbDevice v : usbManager.getDeviceList().values())
            if(v.getDeviceId() == deviceId)
                device = v;
        if(device == null) {
            throw new Exception("connection failed: device not found");
        }
        UsbSerialDriver driver = UsbSerialProber.getDefaultProber().probeDevice(device);
        if(driver == null) {
            driver = CustomProber.getCustomProber().probeDevice(device);
        }
        if(driver == null) {
            throw new Exception("connection failed: no driver for device");
        }
        if(driver.getPorts().size() < portNum) {
            throw new Exception("connection failed: not enough ports at device");
        }
        usbSerialPort = driver.getPorts().get(portNum);
        UsbDeviceConnection usbConnection = usbManager.openDevice(driver.getDevice());
        if(usbConnection == null && usbPermission == UsbPermission.Unknown && !usbManager.hasPermission(driver.getDevice())) {
            usbPermission = UsbPermission.Requested;
            int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? PendingIntent.FLAG_MUTABLE : 0;
            Intent intent = new Intent(INTENT_ACTION_GRANT_USB);
            intent.setPackage(context.getPackageName());
            PendingIntent usbPermissionIntent = PendingIntent.getBroadcast(context, 0, intent, flags);
            usbManager.requestPermission(driver.getDevice(), usbPermissionIntent);
            return;
        }
        if(usbConnection == null) {
            if (!usbManager.hasPermission(driver.getDevice()))
                throw new Exception("connection failed: permission denied");
            else
                throw new Exception("connection failed: open failed");
        }

        try {
            usbSerialPort.open(usbConnection);
            try{
                usbSerialPort.setParameters(baudRate, 8, 1, UsbSerialPort.PARITY_NONE);
            }catch (UnsupportedOperationException e){
                throw new Exception("unsupport setparameters");
            }
            if(withIoManager) {
                usbIoManager = new SerialInputOutputManager(usbSerialPort, listener);
                usbIoManager.start();
            }

            connected = true;

        } catch (Exception e) {
            disconnect();
            throw new Exception("connection failed: " + e.getMessage());
        }

        new Thread(this).start();
    }

    public void disconnect() {
        connected = false;

        if(usbIoManager != null) {
            usbIoManager.setListener(null);
            usbIoManager.stop();
        }
        usbIoManager = null;
        try {
            usbSerialPort.close();
        } catch (IOException ignored) {}
        usbSerialPort = null;
    }

    static {
        System.loadLibrary("usb-serial");
    }

    @Override
    public void run() {
        initNative();
    }
}

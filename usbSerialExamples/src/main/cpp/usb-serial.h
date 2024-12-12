//
// Created by wenmhappy on 2024/12/11.
//

#ifndef USB_SERIAL_FOR_ANDROID_USB_SERIAL_H
#define USB_SERIAL_FOR_ANDROID_USB_SERIAL_H

extern "C" {
typedef void (*ReceiveCallback)(char *data, int len);

// 发送数据到 usb串口
void usb_send(char *data, int len);

// 注册处理usb串口返回数据的回调
void usb_set_receive_callback(ReceiveCallback callback);

}
#endif //USB_SERIAL_FOR_ANDROID_USB_SERIAL_H

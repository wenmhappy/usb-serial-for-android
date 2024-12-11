//
// Created by wenmhappy on 2024/12/11.
//

#ifndef USB_SERIAL_FOR_ANDROID_USB_SERIAL_H
#define USB_SERIAL_FOR_ANDROID_USB_SERIAL_H

extern "C" {
typedef void (*ReceiveCallback)(char *data, int len);

void usb_send(char *data, int len);

void usb_set_receive_callback(ReceiveCallback callback);

}
#endif //USB_SERIAL_FOR_ANDROID_USB_SERIAL_H

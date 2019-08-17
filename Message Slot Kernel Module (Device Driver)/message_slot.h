#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H
#include <linux/ioctl.h>

#define MAJOR_NUM 244

// Set the message of the device driver
#define MSG_SLOT_CHANNEL   _IOW(MAJOR_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FILE_NAME "simple_message_slot"
#define SUCCESS 0
#define MAX_BUFFER 128






#endif
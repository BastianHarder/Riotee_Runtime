#ifndef __SNAPSHOT_HANDLER_H_
#define __SNAPSHOT_HANDLER_H_

#include <stdint.h>

//Define the size of snapshots to be received from max2769 in bytes
//Snapshot size depends on sampling frequency, snapshot duration and adc resolution
#define SNAPSHOT_SIZE_BYTES 6138    // 4092000 Hz x 0.012s / 8 = 6138 Bytes

//Global buffer to store snapshot
uint8_t snapshot_buf[SNAPSHOT_SIZE_BYTES];

#endif /* __SNAPSHOT_HANDLER_H_ */
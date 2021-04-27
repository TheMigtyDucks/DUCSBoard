/*
 * common defines and macros used across the project 
 */
#ifndef DEFINES_H
#define DEFINES_H

// Serial Print Options
#define DEBUG 1
#define TEST 1

// Dev Options
#define DISABLE_BLE 0
#define USE_READCNT 0
#define MIME_BLE 0

// Mutex Macros
// for portMUTEX_TYPE
#define LOCK_M(x) portENTER_CRITICAL(x)
#define UNLOCK_M(x) portEXIT_CRITICAL(x)
// for semaphore mutexes
#define LOCK_S(x) xSemaphoreTake(x, portMAX_DELAY)
#define UNLOCK_S(x) xSemaphoreGive(x)

 // Conversion constants
#define S_MS_CONV 1000
#define S_US_CONV 1000000

// BLE Info
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// Default App Settings
#define REC_FREQ 60 // in seconds
//NOTE: advertise freq MUST be longer than the timeout time
#define BLE_ADV_FREQ 30 // in seconds
#define BLE_TIMEOUT 10 // in seconds
#define BLE_BUFF_LEN 512 // in bytes

// Sensors
#define NUM_SENSORS 4

#endif /* DEFINES_H */

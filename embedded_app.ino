/************
 * Includes *
 ************/
 // std library
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <SPIFFS.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// project files
#include "defines.h"
#include "print.h"
#include "state_machines.h"
#include "thread_fns.h"
#include "hw_timers.h"

/******************************
 * State Machine Declarations *
 ******************************/
 //TODO: Do these need special memory attributes to talk about where to store them?
// master state machine - begin as SETUP
volatile enum master_state_t master_state = M_SETUP;
// sensor thread state machine
volatile enum sensor_state_t sensor_state = S_INIT;
// BLE communication state machine
volatile enum ble_state_t ble_state = B_INIT;
// all state machine states

/*****************
 * State Mutexes *
 *****************/
// NOTE: No mutexes needed as the states are only ever acceced within their threads

/****************
 * Task Handles *
 ****************/
// NOTE: _th stands for "task handle"
TaskHandle_t master_thread_th;
TaskHandle_t sensor_thread_th;
TaskHandle_t ble_thread_th;

 /*******************
  * Hardware Timers *
  *******************/
esp_timer_handle_t sensor_timer;
esp_timer_handle_t ble_timer;

/***************************
 * Timer Binary Semaphores *
 ***************************/ 
// NOTE _b_sem stands for "binary semaphore"
volatile SemaphoreHandle_t sensor_timer_b_sem;
volatile SemaphoreHandle_t ble_timer_b_sem;
volatile SemaphoreHandle_t timeout_timer_b_sem;

/**********************************
 * Notification Binary Semaphores *
 **********************************/ 
volatile SemaphoreHandle_t sensor_wake_b_sem;
volatile SemaphoreHandle_t sensor_done_b_sem;
volatile SemaphoreHandle_t ble_wake_b_sem;
volatile SemaphoreHandle_t ble_done_b_sem;
volatile SemaphoreHandle_t ble_ctrl_start_b_sem;
volatile SemaphoreHandle_t ble_ctrl_stop_b_sem;
volatile SemaphoreHandle_t master_in_ctrl_b_sem;

volatile SemaphoreHandle_t ble_dev_conn_b_sem;
volatile SemaphoreHandle_t ble_dev_disconn_b_sem;
volatile SemaphoreHandle_t ble_cmd_rec_b_sem;

/*******************************
 * Mutual Exclusion Semaphores *
 *******************************/
// NOTE: _s_mux stands for "semaphore mutex"
volatile SemaphoreHandle_t data_s_mux;

/****************************
 * App Command Buffer Mutex *
 ****************************/
portMUX_TYPE acb_mux = portMUX_INITIALIZER_UNLOCKED;

/****************
 * Serial Mutex *
 ****************/
//NOTE: This is only used if TEST or DEBUG is running
#if DEBUG || TEST
portMUX_TYPE serial_mux = portMUX_INITIALIZER_UNLOCKED;
#endif

/*********
 * Error *
 ********/
esp_err_t errRc;

/**********************************
 * BLE Server, RX, & TX Variables *
 **********************************/
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
uint8_t *tx_data_buff = new uint8_t[BLE_BUFF_LEN];
std::string app_cmd_buff;

/***************************
 * App Control Information *
 ***************************/
// NOTE: in milliseconds
const TickType_t yeildDelay = 1 / portTICK_PERIOD_MS;
// NOTE: In microseconds
uint32_t rec_freq = REC_FREQ * S_US_CONV;
uint32_t ble_adv_freq = BLE_ADV_FREQ * S_US_CONV;
uint32_t ble_timeout = BLE_TIMEOUT * S_US_CONV;


/***************************
 * SETUP on Board Power-UP *
 ***************************/
void setup() {

  // enable serial if testing or debugging
  #if DEBUG || TEST || MIME_BLE
    Serial.begin(115200);
    delay(1000);
  #endif

  // initialize hw timers
  TPRINT_TSK("Initializing HW Timers\n");
  timer_init(&sensor_timer,on_sensor_timer,"Sensor Timer");
  timer_init(&ble_timer,on_ble_timer,"BLE Timer");

  // initialize timer semaphores
  sensor_timer_b_sem = xSemaphoreCreateBinary();
  ble_timer_b_sem = xSemaphoreCreateBinary();
  timeout_timer_b_sem = xSemaphoreCreateBinary();

  // init notify semaphores
  sensor_wake_b_sem = xSemaphoreCreateBinary();
  sensor_done_b_sem = xSemaphoreCreateBinary();
  ble_wake_b_sem = xSemaphoreCreateBinary();
  ble_done_b_sem = xSemaphoreCreateBinary();
  ble_ctrl_start_b_sem = xSemaphoreCreateBinary();
  ble_ctrl_stop_b_sem = xSemaphoreCreateBinary();
  master_in_ctrl_b_sem = xSemaphoreCreateBinary();

  ble_dev_conn_b_sem = xSemaphoreCreateBinary();
  ble_dev_disconn_b_sem = xSemaphoreCreateBinary();
  ble_cmd_rec_b_sem = xSemaphoreCreateBinary();

  // init mutex semaphores
  data_s_mux = xSemaphoreCreateMutex();

  // init SPIFFS
  SPIFFS.begin();

  // initialize threads. Each thread will automatically suspend until setup is complete
  TPRINT_TSK("Initializing Tasks\n");
  // initialized as priority 2 to allow timer task to prempt any thread
  xTaskCreate(master_thread, "Master Thread", 2048, NULL, 2, &master_thread_th);
  xTaskCreate(sensor_thread, "Sensor Thread", 2048, NULL, 2, &sensor_thread_th);
  xTaskCreate(ble_thread, "BLE Thread", 3072, NULL, 2, &ble_thread_th);


  // finish SETUP
  TPRINT_TSK("FINISHING SETUP\n\n");
  
  // Lauch Master Thread
  vTaskResume(master_thread_th);
}

void loop() {
  // suspend main loop as it is never used.
  // All functionality is handled within each individual task
  vTaskSuspend(NULL);  
}

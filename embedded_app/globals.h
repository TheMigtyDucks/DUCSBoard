#ifndef GLOBALS_H
#define GLOBALS_H

#include "state_machines.h"

/*
 * This is a file of all variables that are created/defined
 * in the embedded_app sketch but need to be accessed by all
 * of the files in the project
 */

// Task Handles
 extern TaskHandle_t master_thread_th;
 extern TaskHandle_t sensor_thread_th;
 extern TaskHandle_t ble_thread_th;

// State Machines
extern volatile enum master_state_t master_state;
extern volatile enum sensor_state_t sensor_state;
extern volatile enum ble_state_t ble_state;

// State Machine Mutexes
// NONE

// Hardware Timers
extern esp_timer_handle_t sensor_timer;
extern esp_timer_handle_t ble_timer;

// Timer Binary Semaphores
extern volatile SemaphoreHandle_t sensor_timer_b_sem;
extern volatile SemaphoreHandle_t ble_timer_b_sem;
extern volatile SemaphoreHandle_t timeout_timer_b_sem;


// Notification Binary Semaphores *
extern volatile SemaphoreHandle_t sensor_wake_b_sem;
extern volatile SemaphoreHandle_t sensor_done_b_sem;
extern volatile SemaphoreHandle_t ble_wake_b_sem;
extern volatile SemaphoreHandle_t ble_done_b_sem;
extern volatile SemaphoreHandle_t ble_ctrl_start_b_sem;
extern volatile SemaphoreHandle_t ble_ctrl_stop_b_sem;
extern volatile SemaphoreHandle_t master_in_ctrl_b_sem;

extern volatile SemaphoreHandle_t ble_dev_conn_b_sem;
extern volatile SemaphoreHandle_t ble_dev_disconn_b_sem;
extern volatile SemaphoreHandle_t ble_cmd_rec_b_sem;

//Mutual Exclusion Semaphores
extern volatile SemaphoreHandle_t data_s_mux;

extern esp_err_t errRc;

// App Command Buffer Mutex
extern portMUX_TYPE acb_mux;

// BLE Server, RX, & TX Variables
// DEFINED IN BLE_THREAD.CPP

// App Control Information
extern const TickType_t yeildDelay;
extern uint32_t rec_freq;
extern uint32_t ble_adv_freq;
extern uint32_t ble_timeout;

#endif /* GLOBALS_H */

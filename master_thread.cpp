#include <Arduino.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include "defines.h"
#include "globals.h"
#include "print.h"
#include "state_machines.h"
#include "hw_timers.h"

#define BLESTAT()  PLOCK(); TPRINT("  CTRLR Status: "); \
    TPRINTLN(esp_bt_controller_get_status()); FLUSH(); PUNLOCK();
#define BDDSTAT() PLOCK(); TPRINT("  BLUEDRD Status: "); \
    TPRINTLN(esp_bluedroid_get_status()); FLUSH(); PUNLOCK();

static inline void set_master_state(master_state_t state){
  master_state = state;
}

static inline void en_ble() {
  TPRINT_TSK("MT: BLE EN\n");
  //esp_bt_controller_enable(ESP_BT_MODE_BTDM);
  //esp_bluedroid_enable();
}

static inline void dis_ble() {
  TPRINT_TSK("MT: BLE DIS\n");
  //esp_bluedroid_disable();
  //esp_bt_controller_disable();
}

const TickType_t yeildDelay = 1 / portTICK_PERIOD_MS;

/*
 * function for the master state machine
 */
void master_thread(void *params) {
  TPRINT_TSK("Master State Machine Launched\n");
  
  while(1){
    
    switch(master_state) {
      case M_SETUP:
        // suspend task because waiting for setup to continue
        TPRINT_TSK("Suspending Master Thread\n");
        vTaskSuspend(NULL);


        TPRINT_TSK("MT: SETUP\n");
        // after set-up complete, activate BLE to get user commands
        #if DISABLE_BLE
        timer_start(&sensor_timer, rec_freq);
        xSemaphoreGive(sensor_wake_b_sem);
        TPRINT_TSK("MT: SENSOR\n");
        set_master_state(M_SENSOR);
        #else
        // NORMAL OPERATION
        set_master_state(M_WAIT4CONN);
        #endif
        
        break;
      case M_WAIT4CONN:
        TPRINT_TSK("MT: WAIT4CONN\n");
        // activate BLE thread to begin polling for connection
        xSemaphoreGive(ble_wake_b_sem);

        // wait for initial connection
        xSemaphoreTake(ble_done_b_sem, portMAX_DELAY);

        TPRINT_TSK("MT: WAIT4CONN -> BLE\n");
        set_master_state(M_BLE);

        break;
      case M_SLEEP:
        //TPRINT_TSK("MT: Sleeping\n");
        TPRINT_TSK("\n");
        
        esp_sleep_enable_timer_wakeup(get_time_to_next_alarm());
        esp_light_sleep_start();
        
        TPRINT_TSK("MT: Wake\n");
        if(xSemaphoreTake(ble_timer_b_sem, 0) == pdTRUE) {
          //en_ble();
          TPRINT_TSK("MT: SLEEP -> BLE\n");
          xSemaphoreGive(ble_wake_b_sem);
          set_master_state(M_BLE);
        }else if(xSemaphoreTake(sensor_timer_b_sem, 0) == pdTRUE ) {
          TPRINT_TSK("MT: SLEEP -> SENSOR\n");
          xSemaphoreGive(sensor_wake_b_sem);
          set_master_state(M_SENSOR);
        }              
        break;
      case M_BLE:

        while(1) {
          // If sensor time went off too
          if(xSemaphoreTake(sensor_timer_b_sem, 0) == pdTRUE ) {
            TPRINT_TSK("MT: BLE -> BOTH\n");
            xSemaphoreGive(sensor_wake_b_sem);
            set_master_state(M_BOTH);
            break;
          // User wants to control params
          } else if (xSemaphoreTake(ble_ctrl_start_b_sem, 0) == pdTRUE) {
            TPRINT_TSK("MT: BLE -> TOCTRL\n");
            xSemaphoreGive(sensor_done_b_sem); // cheat to let TOCTRL know sensor is not reading
            set_master_state(M_TOCTRL);
            break;
          // BLE Done
          } else if (xSemaphoreTake(ble_done_b_sem, 0) == pdTRUE) {
            //dis_ble();
            TPRINT_TSK("MT: BLE -> SLEEP\n");
            set_master_state(M_SLEEP);
            break;
          }else { // yeild rest of time
            vTaskDelay(yeildDelay);
          }
        }
        break;
      case M_SENSOR:
        
        while(1) {
          // If ble timer went off too
          if(xSemaphoreTake(ble_timer_b_sem, 0) == pdTRUE ) {
            //en_ble();
            TPRINT_TSK("MT: SENSOR -> BOTH\n");
            xSemaphoreGive(ble_wake_b_sem);
            set_master_state(M_BOTH);
            break;
          // Sensor Done
          } else if (xSemaphoreTake(sensor_done_b_sem, 0) == pdTRUE) {
            TPRINT_TSK("MT: SENSOR -> SLEEP\n");
            set_master_state(M_SLEEP);
            break;
          } else { // yeild rest of time
            vTaskDelay(yeildDelay);
          }
        }
        break;
      case M_BOTH:

      while(1) {
          // If sensor Done
          if (xSemaphoreTake(sensor_done_b_sem, 0) == pdTRUE) {
            TPRINT_TSK("MT: BOTH -> BLE\n");
            set_master_state(M_BLE);
            break;
          // BLE Done
          } else if (xSemaphoreTake(ble_done_b_sem, 0) == pdTRUE) {
            //dis_ble();
            TPRINT_TSK("MT: BOTH -> SENSOR\n");
            set_master_state(M_SENSOR);
            break;
          } else if (xSemaphoreTake(ble_ctrl_start_b_sem, 0) == pdTRUE) {
            TPRINT_TSK("MT: BOTH -> TOCTRL\n");
            set_master_state(M_TOCTRL);
            break;
          } else { // yeild rest of time
            vTaskDelay(yeildDelay);
          }
        }
        break;

      case M_TOCTRL:
        //TPRINT_TSK("MT: TOCTRL\n");
        // disable sensor timer
        timer_stop(&sensor_timer);
        // poll in case timer went of before being disabled
        xSemaphoreTake(sensor_timer_b_sem, 0);

        // make sure sensor is sleeping
        xSemaphoreTake(sensor_done_b_sem, portMAX_DELAY);

        // enter control state
        TPRINT_TSK("MT: TOCTRL -> CTRL\n");
        set_master_state(M_CTRL);
        break;
      case M_CTRL:
        //TPRINT_TSK("MT: CTRL\n");
        // send BLE to CTRL state
        xSemaphoreGive(master_in_ctrl_b_sem);


        // wait for BLE to finish CTRL
        xSemaphoreTake(ble_ctrl_stop_b_sem, portMAX_DELAY);

        //start sensor timer
        timer_start(&sensor_timer, rec_freq);

        // change state
        TPRINT_TSK("MT: CTRL -> BOTH\n");
        set_master_state(M_BOTH);

        // trigger sensor read
        xSemaphoreGive(sensor_wake_b_sem);
        break;
    }
  }
}

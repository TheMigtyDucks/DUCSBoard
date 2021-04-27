#include <Arduino.h>
#include <SPIFFS.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "defines.h"
#include "globals.h"
#include "print.h"
#include "state_machines.h"
#include "hw_timers.h"

#define BLESTAT()  PLOCK(); TPRINT("  CTRLR Status: "); \
    TPRINTLN(esp_bt_controller_get_status()); FLUSH(); PUNLOCK();
#define BDDSTAT() PLOCK(); TPRINT("  BLUEDRD Status: "); \
    TPRINTLN(esp_bluedroid_get_status()); FLUSH(); PUNLOCK();

// external variables
extern BLEServer *pServer;
extern BLECharacteristic *pTxCharacteristic;
extern uint8_t *tx_data_buff;
extern std::string app_cmd_buff;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    xSemaphoreGive(ble_dev_conn_b_sem);
  };

  void onDisconnect(BLEServer* pServer) {
    xSemaphoreGive(ble_dev_disconn_b_sem);
  }
};

// class for handling anything that's sent over BLE
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    LOCK_M(&acb_mux);
    app_cmd_buff = pCharacteristic->getValue();
    //TPRINTLN_TSK(app_cmd_buff.c_str());
    //TPRINTLN_TSK(app_cmd_buff.length());
    UNLOCK_M(&acb_mux);
    // notify thread
    xSemaphoreGive(ble_cmd_rec_b_sem);
  }
};

// must hold data_s_mux before calling
void sendDataUART() {

  // delete once spiffs is implemented
  #if 0
  TPRINTLN_TSK("BT: Data Transmitting");
  vTaskDelay(10*S_MS_CONV);
  TPRINTLN_TSK("BT: Data Transfer Done");
  #else
  int buffLenTemp;
  // Read the file
  File s_data = SPIFFS.open("/data.csv");
  if (!s_data) {
    TPRINT_TSK("BT: Did not open file\n");
  }
  TPRINT_TSK("BT: Transmitting Data\n");
  /*#if MIME_BLE
  // writes data to serial port
  if (s_data.available()) {
    PLOCK();
    while (s_data.available()) {
      TWRITE(s_data.read());
    }
    FLUSH();
    PUNLOCK();
  }*/
  //#else
  while (s_data.available()) {
    for (int i = 0; i < BLE_BUFF_LEN; i++)
    {
      if (s_data.available()) {
        tx_data_buff[i] = s_data.read();
        
        }
      else {
        buffLenTemp = i;
        break;
        }
    }
    // i is number of bytes written into tx_data_buff
    pTxCharacteristic->setValue(tx_data_buff, buffLenTemp);
    pTxCharacteristic->notify();
    
    vTaskDelay(10); // bluetooth stack will go into congestion, if too many packets are sent
  }
  s_data.close();
  TPRINT_TSK("BT: Data Sent\n");
  //#endif // MIME_BLE
  #endif // to delete if
}

static inline void set_ble_state(ble_state_t state){
  ble_state = state;
}

/*
 * function for the ble state machine
 */
void ble_thread(void *params) {
  esp_timer_handle_t timeout_timer;
  BLEService *pService;
  BLECharacteristic *pRxCharacteristic;
  int val;

  #if TEST
  char serial_data[65];
  int bytes_avail;
  #endif;

  TPRINT_TSK("BLE State Machine Launched\n");

  while (1) {

    switch (ble_state) {
      // Initialization
      case B_INIT:
          // suspend task
          TPRINT_TSK("Suspending BLE Thread\n");          
          xSemaphoreTake(ble_wake_b_sem, portMAX_DELAY);

          // initialize BT Server
          // Create the BLE Device         
          BLEDevice::init("DUCC Board");

          // Create the BLE Server
          pServer = BLEDevice::createServer();
          pServer->setCallbacks(new MyServerCallbacks());

          // Create the BLE Service 
          pService = pServer->createService(SERVICE_UUID);

          // Create a BLE Characteristic
          pTxCharacteristic = pService->createCharacteristic(
                            CHARACTERISTIC_UUID_TX,
                            BLECharacteristic::PROPERTY_NOTIFY
                          );
                              
          pTxCharacteristic->addDescriptor(new BLE2902());

          pRxCharacteristic = pService->createCharacteristic(
                              CHARACTERISTIC_UUID_RX,
                              BLECharacteristic::PROPERTY_WRITE
                            );

          pRxCharacteristic->setCallbacks(new MyCallbacks());

          // Start the service
          pService->start();

          // create timout timer
          timer_init(&timeout_timer,on_timeout_timer, "Timeout Timer");

          TPRINT_TSK("BT: INIT Complete\n");
          TPRINT_TSK("BT: Esablishing Initial Connection\n");

          // estabilsh initial connection
          #if MIME_BLE
          while (1) {
            // temporary code to mime connection via arduino serial monitor

            bytes_avail = 0;
            serial_data[0] = 0;

            if ((bytes_avail = Serial.available()) > 0) {
              //TPRINTLN_TSK(bytes_avail);
              Serial.readBytes(serial_data, bytes_avail);
              serial_data[bytes_avail-1] = 0; // set transmitted newline to NULL
              /*
              PLOCK();
              TPRINT("BT: CMD - "); TPRINTLN(serial_data);
              FLUSH();
              PUNLOCK();
              */
            }

            if(serial_data[0] == 'C') {
              // use ble_done_b_sem to tell master in conn
              xSemaphoreGive(ble_done_b_sem);
              set_ble_state(B_CONN);
              break;
            }
            vTaskDelay(yeildDelay); // allow idle to trigger the watchdog
          }
          #else
          // start advertising
          pServer->getAdvertising()->start();

          // wait for conn to be made
          xSemaphoreTake(ble_dev_conn_b_sem, portMAX_DELAY);
          // use ble_done_b_sem to tell master in conn
          xSemaphoreGive(ble_done_b_sem);
          set_ble_state(B_CONN);

          #endif
        break;
      // Sleeping
      case B_SLEEP:
        TPRINT_TSK("BT: SLEEP\n");
        
        xSemaphoreTake(ble_wake_b_sem, portMAX_DELAY);

        //TPRINT_TSK("BT: Waking-up\n");
        set_ble_state(B_ADV);
        break;
      // Advertise BLE Connection
      case B_ADV:
        TPRINT_TSK("BT: Advertising\n");

        // clear semaphore in case there
        xSemaphoreTake(timeout_timer_b_sem, 0);
        // start timeout timer
        timer_start_once(&timeout_timer, ble_timeout);

        #if MIME_BLE
        // code to mime a serial connection if dev op selected
        while (1) {
          bytes_avail = 0;
          serial_data[0] = 0;

          if ((bytes_avail = Serial.available()) > 0) {
            //TPRINTLN_TSK(bytes_avail);
            Serial.readBytes(serial_data, bytes_avail);
            serial_data[bytes_avail-1] = 0; // set transmitted newline to NULL
            /*
            PLOCK();
            TPRINT("BT: CMD - "); TPRINTLN(serial_data);
            FLUSH();
            PUNLOCK();
            */
          }

          if(serial_data[0] == 'C') {
            // stop timeout timer
            timer_stop(&timeout_timer);
            // clear semaphore if triggered
            xSemaphoreTake(timeout_timer_b_sem, 0);
            set_ble_state(B_CONN);
            break;
          } else {}

          if (xSemaphoreTake(timeout_timer_b_sem, 0) == pdTRUE) {
            xSemaphoreGive(ble_done_b_sem);            
            set_ble_state(B_SLEEP);
            break;
          }
          vTaskDelay(yeildDelay); // allow idle to trigger the watchdog
        }
        #else
        // Actual advertise code
        pServer->getAdvertising()->start();

        while (1) {
          // check if conn made
          if (xSemaphoreTake(ble_dev_conn_b_sem,0) == pdTRUE) {
            // stop timeout timer
            timer_stop(&timeout_timer);
            // clear semaphore if triggered
            xSemaphoreTake(timeout_timer_b_sem,0);
            set_ble_state(B_CONN);
            break;
          } else if (xSemaphoreTake(timeout_timer_b_sem, 0) == pdTRUE) {
            // stop advertising
            pServer->getAdvertising()->stop();
            // Notify master and sleep
            xSemaphoreGive(ble_done_b_sem);            
            set_ble_state(B_SLEEP);
            break;
          }
          vTaskDelay(yeildDelay); // allow idle to trigger the watchdog
        }
        #endif
        break;
      case B_CONN:
        TPRINT_TSK("BT: CONN\n");

        // disable timer because we got a connection
        timer_stop(&ble_timer);
        // clear in case triggered
        xSemaphoreTake(ble_timer_b_sem,0);

        // Process BLE commands
        #if MIME_BLE
        // temporary code to mime connection via arduino serial monitor
        while (1) {
          bytes_avail = 0;
          serial_data[0] = 0;

          if ((bytes_avail = Serial.available()) > 0) {
            Serial.readBytes(serial_data, bytes_avail);
            serial_data[bytes_avail-1] = 0; // set transmitted newline to NULL
            /*
            PLOCK();
            TPRINT("BT: CMD - "); TPRINTLN(serial_data);
            FLUSH();
            PUNLOCK();
            */
          }

          if(serial_data[0] == 'X') {
            set_ble_state(B_TX);
            break;
          } else if(serial_data[0] == 'R') {
            xSemaphoreGive(ble_ctrl_start_b_sem);
            set_ble_state(B_CTRL);
            break;
          } else if(serial_data[0] == 'Q') {
            timer_start(&ble_timer,ble_adv_freq);
            xSemaphoreGive(ble_done_b_sem);      
            set_ble_state(B_SLEEP);
            break;
          }
          vTaskDelay(yeildDelay); // allow idle to trigger
        }
        #else
        // Actual Conn code
        while (1) {
          // dev disconnects
          if(xSemaphoreTake(ble_dev_disconn_b_sem, 0) == pdTRUE) {
            timer_start(&ble_timer,ble_adv_freq);
            xSemaphoreGive(ble_done_b_sem);      
            set_ble_state(B_SLEEP);
            break;
          // cmd available
          } else if (xSemaphoreTake(ble_cmd_rec_b_sem,0) == pdTRUE) {
            // read from app_cmd_buff and process code. Make sure to use
            // LOCK_M(&acb_mux) and UNLOCK_M(&acb_mux) to ensure no races
              
            LOCK_M(&acb_mux);
            if ((app_cmd_buff.length() >= 2) && (app_cmd_buff[0] == '!')) {
              switch (app_cmd_buff[1]) {
                case 'X':
                  UNLOCK_M(&acb_mux);
                  set_ble_state(B_TX); // need to import this function from BLE_Final
                  goto EXIT_CONN;
                  break;
                case 'L':
                  UNLOCK_M(&acb_mux);
                  xSemaphoreGive(ble_ctrl_start_b_sem);
                  set_ble_state(B_CTRL);
                  goto EXIT_CONN;
                  break;
              }
            }
            UNLOCK_M(&acb_mux);
          }
          vTaskDelay(yeildDelay); // allow idle to trigger the watchdog
        }
EXIT_CONN:
        #endif
        break;
      // NOTE: Assumes will always have a connection through the entire \
               transfer and no packets are dropped
      case B_TX:
        // transmit data
        LOCK_S(data_s_mux);
        TPRINT_TSK("BT: B_TX\n");
        sendDataUART();
        UNLOCK_S(data_s_mux);

        set_ble_state(B_CONN);
        break;
      case B_CTRL:
        // recieve updated op params and update data structures
        
        // sleep until master is in CTRL State
        xSemaphoreTake(master_in_ctrl_b_sem, portMAX_DELAY);
        TPRINT_TSK("BT: B_CTRL\n");

        // update the things
        #if MIME_BLE
        // temporary code to mime ble conn
        while(1) {
          bytes_avail = 0;
          serial_data[0] = 0;

          if ((bytes_avail = Serial.available()) > 0) {
            Serial.readBytes(serial_data, bytes_avail);
            serial_data[bytes_avail-1] = 0; // set transmitted newline to NULL
            /*
            PLOCK();
            TPRINT("BT: CMD - "); TPRINTLN(serial_data);
            FLUSH();
            PUNLOCK();
            */
          }

          if(serial_data[0] == 'Q') {
            // exit loop
            break;
          } else if (serial_data[0] != 0) {
            PLOCK();
            TPRINT("BT: CMD - "); TPRINTLN(serial_data);
            FLUSH();
            PUNLOCK();
          }

          vTaskDelay(yeildDelay); // allow idle to trigger the watchdog
        }
        #else
        // Actual CTRL logic
        while (1) {
          // if accidental disconnect
          if(xSemaphoreTake(ble_dev_disconn_b_sem, 0) == pdTRUE) {
            TPRINT_TSK("\nBT: DISCONN IN CTRL - Please Recconnect\n");
            // advertise
            pServer->getAdvertising()->start();
            xSemaphoreTake(ble_dev_conn_b_sem,portMAX_DELAY);
            TPRINT_TSK("BT: RECONNECTED\n\n");

          // cmd available
          } else if (xSemaphoreTake(ble_cmd_rec_b_sem,0) == pdTRUE) {
            // read from app_cmd_buff and change settings. Make sure to use
            // LOCK_M(&acb_mux) and UNLOCK_M(&acb_mux) to ensure no race conditions
              
            LOCK_M(&acb_mux);
            if ((app_cmd_buff.length() >= 2) && (app_cmd_buff[0] == '!')) {
              switch (app_cmd_buff[1]) {
                // recording freqency
                case 'R':
                  val = atoi(app_cmd_buff.substr(2).c_str());
                  if (val > 0) {
                    rec_freq = val * S_US_CONV;
                    PLOCK();TPRINT("Set Rec Freq: ");TPRINTLN(rec_freq / S_US_CONV); \
                    FLUSH();PUNLOCK();
                  }
                  break;
                // enable sensor
                case 'E':
                  val = atoi(app_cmd_buff.substr(2).c_str());
                  if (val > 0) {
                    // TODO add code for enabling sensor in val

                  }
                  break;
                // disable sensor
                case 'D':
                val = atoi(app_cmd_buff.substr(2).c_str());
                  if (val > 0) {
                    // TODO add code for disabling sensor in val
                    
                  }
                  break;
                // Begin capture
                case 'B':
                  UNLOCK_M(&acb_mux);
                  goto EXIT_CTRL;
                  break;
              }
            }
            UNLOCK_M(&acb_mux);
          }
          vTaskDelay(yeildDelay); // allow idle to trigger the watchdog
        }


        #endif


        // tell Master done w/ control
EXIT_CTRL:
        xSemaphoreGive(ble_ctrl_stop_b_sem);
        

        set_ble_state(B_CONN);
        break;
    }
  }
}

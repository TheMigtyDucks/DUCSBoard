#include <SEN0189.h>
#include <sensor.h>

#include <Arduino.h>
#include <list>
#include <string>
#include <SPIFFS.h>
#include "defines.h"
#include "globals.h"
#include "print.h"
#include "state_machines.h"

#if USE_READCNT
uint32_t readcnt;
uint32_t temp;
#endif

// Battery Measurement
#define VBATPIN 35 
float measuredvbat;


//TODO: Delete
uint32_t readcnt;

static inline void set_sensor_state(sensor_state_t state){
  sensor_state = state;
}

// TODO init based on app input
Sensor * sensors[NUM_SENSORS];
// create buffer
float read_buff[NUM_SENSORS];

void sensor_thread(void *params) {
  TPRINT_TSK("Sensor State Machine Launched\n");

  while(1) {
    
    switch (sensor_state) {
      case S_INIT:
        {
        // suspend task
        TPRINT_TSK("Suspending Sensor Thread\n");          
        xSemaphoreTake(sensor_wake_b_sem, portMAX_DELAY);

        #if USE_READCNT
        readcnt = 0;
        #endif

        sensors[0] = Sensor::create("pH", A3, "pH");
        sensors[1] = Sensor::create("DS18B20", 13, "Temp");
        sensors[2] = Sensor::create("SEN0189", A4, "Turbidity");
        sensors[3] = Sensor::create("TDS", A5, "TDS");
        sensors[3]->calibrate(55); // TDS of 55ppm
        LOCK_S(data_s_mux);
        File s_data = SPIFFS.open("/data.csv", FILE_APPEND);
        if(! s_data) {
            TPRINT_TSK("ST: File no open 1\n");
        }
        s_data.printf("pH(pH),Temp(F),Turbidity(V),TDS(ppm),VBat(V),Time(ms)\n");
        s_data.close();
        UNLOCK_S(data_s_mux);
        
        TPRINT_TSK("ST: List made");

        // once resumed
        set_sensor_state(S_READ);
        TPRINT_TSK("ST: INIT Complete\n");
        }
        break;
      case S_SLEEP:
        {
          TPRINT_TSK("ST: SLEEP\n");
  
          xSemaphoreTake(sensor_wake_b_sem, portMAX_DELAY);
  
          //TPRINT_TSK("ST: Waking-up\n");
          set_sensor_state(S_READ);
        }
        break;
      case S_READ:
        {
          TPRINT_TSK("ST: Reading from Sensors");
          for(int i=0; i<NUM_SENSORS; i++) {
                read_buff[i] = sensors[i]->read();
          }
          measuredvbat = analogRead(VBATPIN);
          measuredvbat *= 2;    // we divided by 2, so multiply back
          measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
          measuredvbat /= 4096; // convert to voltage
          measuredvbat += 0.2; // compensate for error
          TPRINT_TSK(measuredvbat);
          set_sensor_state(S_PERSIST);
        }
        break;
      /*
      // STATE REMOVED
      case S_WAIT4TX:
        set_sensor_state(S_PERSIST);
        break;
      */
      case S_PERSIST: 
        {
          LOCK_S(data_s_mux);
          TPRINT_TSK("ST: PERSIST\n");
          File s_data = SPIFFS.open("/data.csv", FILE_APPEND);
          if(! s_data) {
            TPRINT_TSK("ST: File no open\n");
          }
          for(int i=0; i<NUM_SENSORS; i++) {
            s_data.printf("%f,", read_buff[i]);
          }
          s_data.printf("%f,", measuredvbat);
          s_data.printf("%lu\n", millis());
          s_data.close();
          delay(1 * S_MS_CONV);
          UNLOCK_S(data_s_mux);
  
          set_sensor_state(S_SLEEP);
          TPRINT_TSK("ST: Resuming Master Thread\n");
          xSemaphoreGive(sensor_done_b_sem);
        }
        break;
    }
  }
}

#include <Arduino.h>

#include "defines.h"
#include "globals.h"
#include "hw_timers.h"


void timer_init(esp_timer_handle_t *timer,void (*onTimer)(void *), const char *name) {
  // set create args
  esp_timer_create_args_t timer_args = {
    onTimer,         // callback
    NULL,             // callback args
    ESP_TIMER_TASK,   // dispatch_method. change to ESP_TIMER_ISR if want to run as interrupt
    name              // timer name
  };

  esp_timer_create(&timer_args, timer);
}

void timer_delete(esp_timer_handle_t *timer) {
  esp_timer_delete(*timer);
}

void on_sensor_timer(void *args) {
  // Notify Master thread that it is time to read some sensors
  xSemaphoreGive(sensor_timer_b_sem);
  //xSemaphoreGiveFromISR(sensor_timer_b_sem, NULL); // if from ISR
}

void on_ble_timer(void * args) {
  xSemaphoreGive(ble_timer_b_sem);
  //xSemaphoreGiveFromISR(ble_timer_b_sem, NULL); // if from ISR 
}

void on_timeout_timer(void *args) {
  xSemaphoreGive(timeout_timer_b_sem);
  //xSemaphoreGiveFromISR(timeout_timer_b_sem, NULL); // if from ISR
}

void timer_start(esp_timer_handle_t *timer, uint64_t period) {
  esp_timer_start_periodic(*timer, period);
}

void timer_start_once(esp_timer_handle_t *timer, uint64_t timeout_us) {
  esp_timer_start_once(*timer, timeout_us);
}

void timer_stop(esp_timer_handle_t *timer) {
  esp_timer_stop(*timer);
}

uint64_t get_time_to_next_alarm() {
  return esp_timer_get_next_alarm() - esp_timer_get_time();
}

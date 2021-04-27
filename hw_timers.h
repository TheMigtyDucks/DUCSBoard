#ifndef HW_TIMERS_H
#define HW_TIMERS_H

// External Variables

// Variables

// Function Definitions
void timer_init(esp_timer_handle_t *,void (*onTimer)(void *), const char *);
void timer_delete(esp_timer_handle_t *);
void on_sensor_timer(void *); // add IRAM_ATTR if created as ISR
void on_ble_timer(void *); // add IRAM_ATTR if created as ISR
void on_timeout_timer(void *); // add IRAM_ATTR if created as ISR
void timer_start(esp_timer_handle_t *,uint64_t); // start a periodic timer
void timer_start_once(esp_timer_handle_t *, uint64_t); // start a one-shot timer
void timer_stop(esp_timer_handle_t *);
uint64_t get_time_to_next_alarm(); // get the time remaining until next timer event in us



#endif /* HW_TIMERS_H */

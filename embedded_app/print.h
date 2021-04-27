// inline functions and defines for printing
#ifndef PRINT_H
#define PRINT_H

#include "defines.h"

//serial debug print definitions
#if TEST || DEBUG
extern portMUX_TYPE serial_mux;
#define FLUSH() Serial.flush()
#define PLOCK() portENTER_CRITICAL(&serial_mux)
#define PUNLOCK() portEXIT_CRITICAL(&serial_mux)
#else
#define FLUSH() do{}while(0)
#define PLOCK() do{}while(0)
#define PUNLOCK() do{}while(0)
#endif

#if DEBUG
#define DPSYSTIME() Serial.print('[');Serial.print(xTaskGetTickCount());Serial.print(']')
#define DBPRINT(s) Serial.print(s)
#define DBPRINTLN(s) Serial.println(s)
#define DBWRITE(s) Serial.write(s)
// a task safe print
#define DPRINT_TSK(s) PLOCK(); Serial.print(s); Serial.flush(); PUNLOCK()
#define DPRINTLN_TSK(s) PLOCK(); Serial.println(s); Serial.flush(); PUNLOCK()
#else
#define DPSYSTIME() do{}while(0)
#define DBPRINT(s) do{}while(0)
#define DBPRINTLN(s) do{}while(0)
#define DBWRITE(s) do{}while(0)
#define DPRINT_TSK(s) do{}while(0)
#define DPRINTLN_TSK(s) do{}while(0)
#endif

#if TEST
#define TPSYSTIME() Serial.print('[');Serial.print(xTaskGetTickCount());Serial.print(']')
#define TPRINT(s) Serial.print(s)
#define TPRINTLN(s) Serial.println(s)
#define TWRITE(s) Serial.write(s)
// a task safe print
#define TPRINT_TSK(s) PLOCK(); Serial.print(s); Serial.flush(); PUNLOCK()
#define TPRINTLN_TSK(s) PLOCK(); Serial.println(s); Serial.flush(); PUNLOCK()
#else
#define TPSYSTIME() do{}while(0)
#define TPRINT(s) do{}while(0)
#define TPRINTLN(s) do{}while(0)
#define TWRITE(s) do{}while(0)
#define TPRINT_TSK(s) do{}while(0)
#define TPRINTLN_TSK(s) do{}while(0)
#endif


#endif /* PRINT_H */

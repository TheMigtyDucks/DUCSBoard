/*
 * Typedefs for all the state machines used in the embedded application
 */

 #ifndef STATE_MACHINES_H
 #define STATE_MACHINES_H
 // NOTE: Each enum value has "{M,S,B}_" prepended to the name 
 // to prevent naming collisions between different state machines
enum master_state_t {
  M_SETUP,
  M_WAIT4CONN,
  M_SLEEP,
  M_BLE,
  M_SENSOR,
  M_BOTH,
  M_TOCTRL,
  M_CTRL
};

enum sensor_state_t {
  S_INIT,
  S_SLEEP,
  S_READ,
  // S_WAIT4TX, // NOT NEEDED
  S_PERSIST
};

enum ble_state_t {
  B_INIT,
  B_SLEEP,  
  B_ADV,    // advertise
  B_CONN,   // connected
  B_TX,     // transmit
  B_CTRL    // control
};

#endif /* STATE_MACHINES_H */

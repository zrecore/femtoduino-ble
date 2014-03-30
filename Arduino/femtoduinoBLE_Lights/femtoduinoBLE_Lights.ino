#include <SoftwareSerial.h>
#include "BGLib.h"


// Uncomment to see Debugging stuff in the Serial
// window. NOTE: When running the FemtoduinoBLE without 
// a Serial connection to your computer, do not wait for the 
// Serial object to become available, as (obviously) it won't.
// (Not connected to a USB port, so it's obviously not there, right?)

#define DEBUG

// ================================================================
// BLE STATE TRACKING (UNIVERSAL TO JUST ABOUT ANY BLE PROJECT)
// ================================================================

// BLE state machine definitions
#define BLE_STATE_STANDBY           0
#define BLE_STATE_SCANNING          1
#define BLE_STATE_ADVERTISING       2
#define BLE_STATE_CONNECTING        3
#define BLE_STATE_CONNECTED_MASTER  4
#define BLE_STATE_CONNECTED_SLAVE   5

// BLE state/link status tracker
uint8_t ble_state = BLE_STATE_STANDBY;
uint8_t ble_encrypted = 0;  // 0 = not encrypted, otherwise = encrypted
uint8_t ble_bonding = 0xFF; // 0xFF = no bonding, otherwise = bonding handle

// Pins
#define LED_PIN 15
#define BLE_WAKEUP_PIN 5
#define BLE_RESET_PIN 6

#define RGB_LED_PIN1 9
#define RGB_LED_PIN2 10
#define RGB_LED_PIN3 11
//#define RGB_LED_PIN4 12

#define GATT_HANDLE_C_RX_DATA 17
#define GATT_HANDLE_C_TX_DATA 20


// Let's talk to our BLE module using these ports as RX/TX
SoftwareSerial bleSerialPort(8, 3); // D8 - RX, D3 - TX

BGLib ble113((HardwareSerial *) &bleSerialPort, 0, 1);
#define BGAPI_GET_RESPONSE(v, dType) dType *v = (dType *)ble113.getLastRXPayload()


/**
 * Based on Jeff Rowberg's BGLib_stub_slave example. Modified to work 
 * with the FemtoduinoBLE, revision 7 or higher.
 */
void setup() {
  // Let's setup our status LED.
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // We will need to set up the BLE control pins.
  // (BLE Reset, BLE wake-up)
  pinMode(BLE_RESET_PIN, OUTPUT);
  digitalWrite(BLE_RESET_PIN, HIGH);
  
  pinMode(BLE_WAKEUP_PIN, OUTPUT);
  digitalWrite(BLE_WAKEUP_PIN, LOW);
  
  pinMode(RGB_LED_PIN1, OUTPUT);
  pinMode(RGB_LED_PIN2, OUTPUT);
  pinMode(RGB_LED_PIN3, OUTPUT);
  //pinMode(RGB_LED_PIN4, OUTPUT);
  
  digitalWrite(RGB_LED_PIN1, LOW);
  digitalWrite(RGB_LED_PIN2, LOW);
  digitalWrite(RGB_LED_PIN3, LOW);
  //digitalWrite(RGB_LED_PIN4, LOW);
  
  // Great! Let's set up our internal status handlers, so we 
  // can blink our status LED accordingly!
  ble113.onBusy = onBusy;
  ble113.onIdle = onIdle;
  ble113.onTimeout = onTimeout;
  
  ble113.onBeforeTXCommand = onBeforeTXCommand;
  ble113.onTXCommandComplete = onTXCommandComplete;
  
  ble113.ble_evt_system_boot = femtoSystemBoot;
  ble113.ble_evt_connection_status = femtoConnectionStatus;
  ble113.ble_evt_connection_disconnected = femtoConnectionDisconnect;
  ble113.ble_evt_attributes_value = femtoAttributesValue;
  
#ifdef DEBUG
  Serial.begin(38400);
  while(!Serial);
#endif
  // open BLE software serial port
  bleSerialPort.begin(38400);
  
  // reset module (maybe not necessary for your application)
  digitalWrite(BLE_RESET_PIN, LOW);
  delay(5); // wait 5ms
  digitalWrite(BLE_RESET_PIN, HIGH);
  
}

void loop() {
  // Keep polling for new data from BLE
  ble113.checkActivity();
  
  // blink FemtoduinoBLE Data LED based on state:
  //  - solid = STANDBY
  //  - 1 pulse per second = ADVERTISING
  //  - 2 pulses per second = CONNECTED_SLAVE
  //  - 3 pulses per second = CONNECTED_SLAVE w/ encryption
  uint16_t slice = millis() % 1000;
  if (ble_state == BLE_STATE_STANDBY) {

    digitalWrite(LED_PIN, HIGH);
  } else if (ble_state == BLE_STATE_ADVERTISING) {

    digitalWrite(LED_PIN, slice < 100);
  } else if (ble_state == BLE_STATE_CONNECTED_SLAVE) {

      if(!ble_encrypted) {
        digitalWrite(LED_PIN, slice < 100 || (slice > 200 && slice < 300));
      } else {
        digitalWrite(LED_PIN, slice < 100 || (slice > 200 && slice < 300) || (slice > 400 && slice < 500));
      }
  }
}

void femtoSystemBoot(const ble_msg_system_boot_evt_t *msg) {
  
#ifdef DEBUG
        Serial.print("###\tsystem_boot: { ");
        Serial.print("major: "); Serial.print(msg -> major, HEX);
        Serial.print(", minor: "); Serial.print(msg -> minor, HEX);
        Serial.print(", patch: "); Serial.print(msg -> patch, HEX);
        Serial.print(", build: "); Serial.print(msg -> build, HEX);
        Serial.print(", ll_version: "); Serial.print(msg -> ll_version, HEX);
        Serial.print(", protocol_version: "); Serial.print(msg -> protocol_version, HEX);
        Serial.print(", hw: "); Serial.print(msg -> hw, HEX);
        Serial.println(" }");
#endif
  // System boot means module is in standby state.
  
  // set advertisement interval to 200-300ms, use all 
  // advertisement channels. Note: min/max parameters are
  // in units of 625 uSec.
  
  ble113.ble_cmd_gap_set_adv_parameters(320, 480, 7);
  while(ble113.checkActivity(1000));
  
  // USE THE FOLLOWING TO LET THE BLE STACK HANDLE YOUR ADVERTISEMENT PACKETS
  // start advertising general discoverable /undirected connectable
  //ble113.ble_cmd_gap_set_mode(BGLIB_GAP_GENERAL_DISCOVERABLE, BGLIB_GAP_UNDIRECTED_CONNECTABLE);
  //while(ble113.checkActivity(1000));
  
  // USE THE FOLLOWING TO HANDLE YOUR OWN CUSTOM ADVERTISEMENT PACKETS
  // build custom advertisement data
  // default BLE stack value: 0201061107e4ba94c3c9b7cdb09b487a438ae55a19
  uint8 adv_data[] = {
    
      0x02,                                     // Field length
      
      BGLIB_GAP_AD_TYPE_FLAGS,                  // Field type (0x01)
      0x06, // data (0x02 | 0x04 = 0x06, general discoverable + BLE only, no BR+EDR) 
        
      0x11,                                     // Field length
      
      BGLIB_GAP_AD_TYPE_SERVICES_128BIT_ALL,    // Field Type (0x07)
      
      0xe4, 0xba, 0x94, 0xc3, 
      0xc9, 0xb7, 0xcd, 0xb0, 
      0x9b, 0x48, 0x7a, 0x43, 
      0x8a, 0xe5, 0x5a, 0x19
  };
  
  ble113.ble_cmd_gap_set_adv_data(0, 0x15, adv_data);
  while(ble113.checkActivity(1000));
  
  // build custom scan response data (i.e. the Device Name value)
  // default BLE stack value: 140942474c69622055314131502033382e344e4657
  uint8 sr_data[] = {
    
    0x14,                                       // Field length
    
    BGLIB_GAP_AD_TYPE_LOCALNAME_COMPLETE,       // Field type
    
    // Set the name to "FemtoduinoBLE 00:00:00"
       'F', 'e', 'm', 't', 'o', 'd', 'u', 'i', 'n', 'o', 'B', 'L', 'E', ' ', '0', '0', ':', '0', '0', ':', '0', '0'
    //  1    2    3    4    5    6    7    8    9    10   11   12   13  14   15   16   17   18   19   20   21   22
  };
  
  // get the BLE MAC address
  ble113.ble_cmd_system_address_get();
  while(ble113.checkActivity(1000));
  BGAPI_GET_RESPONSE(r0, ble_msg_system_address_get_rsp_t);
  
  // assign last three bytes of MAC address to add packet friendly name (instead of 00:00:00 above)
  sr_data[16] = (r0 -> address.addr[2] / 0x10) + 48 + ((r0 -> address.addr[2] / 0x10) / 10 * 7); // MAC byte 4 10's digit
  sr_data[17] = (r0 -> address.addr[2] & 0xF)  + 48 + ((r0 -> address.addr[2] & 0xF ) / 10 * 7); // MAC byte 4 1's digit
  sr_data[18] = (r0 -> address.addr[1] / 0x10) + 48 + ((r0 -> address.addr[1] / 0x10) / 10 * 7); // MAC byte 5 10's digit
  sr_data[19] = (r0 -> address.addr[1] & 0xF)  + 48 + ((r0 -> address.addr[1] & 0xF ) / 10 * 7); // MAC byte 5 1's digit
  sr_data[20] = (r0 -> address.addr[0] / 0x10) + 48 + ((r0 -> address.addr[0] / 0x10) / 10 * 7); // MAC byte 6 10's digit
  sr_data[21] = (r0 -> address.addr[0] & 0xF)  + 48 + ((r0 -> address.addr[0] & 0xF ) / 10 * 7); // MAC byte 6 1's digit
  
  // set custom scan respnose data (i.e. The Device Name value)
  ble113.ble_cmd_gap_set_adv_data(1, 0x15, sr_data);
  while(ble113.checkActivity(1000));
  
  // put the module into "discoverable/connectable" mode
  // (with user-defined advertisement data)
  ble113.ble_cmd_gap_set_mode(BGLIB_GAP_USER_DATA, BGLIB_GAP_UNDIRECTED_CONNECTABLE);
  while (ble113.checkActivity(1000));
  
  // set state to ADVERTISING
  ble_state = BLE_STATE_ADVERTISING;

}

void femtoConnectionStatus(const ble_msg_connection_status_evt_t * msg) {
#ifdef DEBUG
        Serial.print("###\tconnection_status: { ");
        Serial.print("connection: "); Serial.print(msg -> connection, HEX);
        Serial.print(", flags: "); Serial.print(msg -> flags, HEX);
        Serial.print(", address: ");
        // this is a "bd_addr" data type, which is a 6-byte uint8_t array
        for (uint8_t i = 0; i < 6; i++) {
            if (msg -> address.addr[i] < 16) Serial.write('0');
            Serial.print(msg -> address.addr[i], HEX);
        }
        Serial.print(", address_type: "); Serial.print(msg -> address_type, HEX);
        Serial.print(", conn_interval: "); Serial.print(msg -> conn_interval, HEX);
        Serial.print(", timeout: "); Serial.print(msg -> timeout, HEX);
        Serial.print(", latency: "); Serial.print(msg -> latency, HEX);
        Serial.print(", bonding: "); Serial.print(msg -> bonding, HEX);
        Serial.println(" }");
#endif  
  /**
   * "flags" bit description:
   * - bit 0: connection_connnected
   *          Indicates the connection exists to a remote device.
   * - bit 1: connection_encrypted
   *          Indicates the connection is encrypted.
   * - bit 2: connection_completed
   *          Indicates that a new connection has been created.
   * - bit 3: connection_parameters_change
   *          Indicates that connection parameters have changed, and is 
   *          set when parameters change due to a link layer operation.
   */
   
   // Check to see if a new connection has been established.
   if ((msg -> flags & 0x05) == 0x05) {
     // track state change based on the last known state
     if (ble_state == BLE_STATE_ADVERTISING) {
         ble_state = BLE_STATE_CONNECTED_SLAVE;
     } else {
         ble_state = BLE_STATE_CONNECTED_MASTER;
     }
   }
   
   // update "encrypted" status
   ble_encrypted = msg -> flags & 0x02;
   
   // update "bonded" status
   ble_bonding = msg -> bonding;
}

void femtoConnectionDisconnect(const struct ble_msg_connection_disconnected_evt_t *msg) {
#ifdef DEBUG
        Serial.print("###\tconnection_disconnect: { ");
        Serial.print("connection: "); Serial.print(msg -> connection, HEX);
        Serial.print(", reason: "); Serial.print(msg -> reason, HEX);
        Serial.println(" }");
#endif
  ble113.ble_cmd_gap_set_mode(BGLIB_GAP_USER_DATA, BGLIB_GAP_UNDIRECTED_CONNECTABLE);
  while(ble113.checkActivity(1000));
  
  // set state to ADVERTISING
  ble_state = BLE_STATE_ADVERTISING;
  
  // clear "encrypted" and "bonding" info
  ble_encrypted = 0;
  ble_bonding = 0xFF;
}

void femtoAttributesValue(const struct ble_msg_attributes_value_evt_t *msg) {
  /**
   * @todo Use the msg -> value
   */
   #ifdef DEBUG
   Serial.print("###\tattributes_value: { ");
   Serial.print("connection: "); Serial.print(msg -> connection, HEX);
   Serial.print(", reason: "); Serial.print(msg -> reason, HEX);
   Serial.print(", handle: "); Serial.print(msg -> handle, HEX);
   Serial.print(", offset: "); Serial.print(msg -> offset, HEX);
   Serial.print(", value_len: "); Serial.print(msg -> value.len, HEX);
   Serial.print(", value_data: ");
   // this is a "uint8array" data type, which is a length byte and a uint8_t* pointer
   for (uint8_t i = 0; i < msg -> value.len; i++) {
       if (msg -> value.data[i] < 16) Serial.write('0');
       Serial.print(msg -> value.data[i], HEX);
   }
   Serial.println(" }");
   
   #endif
   
   // Check for data written to "c_rx_data" handle
   if (msg -> handle == GATT_HANDLE_C_RX_DATA ) {
     Serial.println("Got C_RX_DATA handle");
     if (msg -> value.len > 0) {
       
       // set pins 9, 10, 11, and 12 to four lower-most bits of first byte of RX data
       // (nice for controlling RGB LED or something)
       int intVal = (int) msg -> value.data[0];
       
       #ifdef DEBUG
         Serial.print("RX Data:");
         Serial.println(intVal);
       #endif
       
       digitalWrite(RGB_LED_PIN1, intVal & 0x01);
       digitalWrite(RGB_LED_PIN2, intVal & 0x02);
       digitalWrite(RGB_LED_PIN3, intVal & 0x04);
//       digitalWrite(RGB_LED_PIN4, intVal & 0x08);
     }
   }
}


// Event handlers

void onBusy() {
  digitalWrite(LED_PIN, HIGH);
}

void onIdle() {
  digitalWrite(LED_PIN, LOW);
}

void onTimeout() {
    digitalWrite(BLE_RESET_PIN, LOW);
    delay(5);
    digitalWrite(BLE_RESET_PIN, HIGH);
}

void onBeforeTXCommand() {
  // wake module up
  digitalWrite(BLE_WAKEUP_PIN, HIGH);
  
  // wait for "hardware_io_port_status" event to come throgh, and parse it (and otherwise ignore it)
  uint8_t *last;
  while(1) {
    ble113.checkActivity();
    last = ble113.getLastEvent();
    if (last[0] == 0x07 && last[1] == 0x00) break;
  }
  
  // give a bit of a gab between parsing the wake-up event and allowing the 
  // command to go out
  delayMicroseconds(1000);
}

void onTXCommandComplete() {
  // allow module to return to sleep
  digitalWrite(BLE_WAKEUP_PIN, LOW);
}


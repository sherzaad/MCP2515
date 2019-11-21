/* This is for use with Sparkfun's CAN Bus Shield: https://www.sparkfun.com/products/10039 */

/*
 * Filter Options
 * 6 Standard CAN IDs
 * 6 Extended CAN IDs
 * 4 Standard CAN IDs + 2 Extended CAN IDs
 * 2 Standard CAN IDs + 4 Extended CAN IDs 
 *  
 */

#include "mcp2515.h"

MCP2515 CAN_Ch1(10, 2);

const uint16_t filter_std_id[3] = {0x525, 0x002, 0x7D4};
const uint32_t filter_ex_id[1] = {0x18DAF1D1};

void setup() {

  //Initialise Baudrate for board serial interface
  Serial.begin(115200);

  while (!Serial) {
    ; //wait for serial port to connect
  }

  //Initialise MCP2515 CAN controller at the specified speed
  if (CAN_Ch1.CAN_init(CANSPEED_500kBPS))
    Serial.println("CAN1 Init ok");
  else
    Serial.println("Can't Init CAN1");

  //Initialise MCP2515 CAN controller filter (a maximum of 6 CAN IDs can be added to filter)
  if (CAN_Ch1.init_filter(filter_std_id, 3, filter_ex_id, 1))
    Serial.println("Hardware filter Initialised.");
  else
    Serial.println("Can't Init Hardware Filter");

}

void loop() {
  tCAN message;

  if (CAN_Ch1.check_message()) {
    if (CAN_Ch1.get_message(&message)) {
      ;
      Serial.print("CAN1 ID: ");
      if (message.header.ide) Serial.print(message.id.ex, HEX);
      else Serial.print(message.id.std, HEX);
      Serial.print(", ");
      Serial.print("Data: ");
      for (int i = 0; i < message.header.dlc; i++) {
        Serial.print(message.data[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
    }
  }
}

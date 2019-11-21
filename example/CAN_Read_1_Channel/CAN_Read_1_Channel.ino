/* This is for use with Sparkfun's CAN Bus Shield: https://www.sparkfun.com/products/10039 */

#include "mcp2515.h"

MCP2515 CAN_Ch1(10, 2);
void setup() {

  //Initialise Baudrate for board serial interface
  Serial.begin(115200);

  while (!Serial) {
    ; //wait for serial port to connect
  }

  //Initialise CH1 MCP2515 CAN controller at the specified speed
  if (CAN_Ch1.CAN_init(CANSPEED_500kBPS))
    Serial.println("CAN1 Init ok");
  else
    Serial.println("Can't Init CAN1");

  Serial.println("READY");
  
}

void loop() {
  tCAN message;

  if (CAN_Ch1.check_message()) {
    if (CAN_Ch1.get_message(&message)) {
      Serial.print("CAN_1 ID: 0x");
      if(message.header.ide)  Serial.print(message.id.ex, HEX); //extended CAN ID
      else Serial.print(message.id.std, HEX); //standard CAN ID
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

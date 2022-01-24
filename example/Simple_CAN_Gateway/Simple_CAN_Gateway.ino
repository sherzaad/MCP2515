/* This is for use with Sparkfun's CAN Bus Shield: https://www.sparkfun.com/products/10039 */

#include "mcp2515.h"

MCP2515 CAN_Ch1(10, 2);
MCP2515 CAN_Ch2(4, 3);

uint8_t ch = 1;
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

  //Initialise CH1 MCP2515 CAN controller at the specified speed
  if (CAN_Ch2.CAN_init(CANSPEED_500kBPS)){
	CAN_Ch1.set_mode(MODE_NORMAL); //set MCP2515 CAN controller into NORMAL mode (init default mode is LISTEN_ONLY)
    Serial.println("CAN2 Init ok");
  }	
  else
    Serial.println("Can't Init CAN2");

  Serial.println("READY");
}

void loop() {
tCAN message;

  if (ch == 1) {
    if (CAN_Ch1.check_message()) {
      if (CAN_Ch1.get_message(&message)) {
        CAN_Ch2.send_message(&message);
      }
    }
  }
  else {
    if (CAN_Ch2.check_message()) {
      if (CAN_Ch2.get_message(&message)) {
        CAN_Ch1.send_message(&message);
      }
    }
  }

  ch ^= 0x03; //XOR allows toggle between CH1 and CH2
}

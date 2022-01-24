/* This is for use with Sparkfun's CAN Bus Shield: https://www.sparkfun.com/products/10039 */
#include "mcp2515.h"

MCP2515 CAN_Ch1(10, 2);

tCAN message_std;
tCAN message_ex;
uint8_t cnt = 0;

void setup() {
uint8_t std_data[7] ={0x01,0x02,0x03,0x04,0x05,0x06,cnt};
uint8_t ex_data[8] ={0x02,0x22,cnt,0xFF,0xFF,0xFF,0xFF,0xFF};

  //Initialise Baudrate for board serial interface
  Serial.begin(115200);

  while (!Serial) {
    ; //wait for serial port to connect
  }
  
  //define CAN standard message
  message_std.id.std = 0x284;
  message_std.header.rtr = 0;
  message_std.header.ide = 0;
  message_std.header.dlc = 7;
  memcpy(message_std.data,std_data,sizeof(std_data));

  //define CAN extended message
  message_ex.id.ex = 0x18CAD1F1;
  message_ex.header.rtr = 0;
  message_ex.header.ide = 1;
  message_ex.header.dlc = 8;
  memcpy(message_ex.data,ex_data,sizeof(ex_data));

  //Initialise CH1 MCP2515 CAN controller at the specified speed
  if (CAN_Ch1.CAN_init(CANSPEED_500kBPS)){
	CAN_Ch1.set_mode(MODE_NORMAL); //set MCP2515 CAN controller into NORMAL mode (init default if LISTEN_ONLY)
    Serial.println("CAN1 Init ok");
  }
  else
    Serial.println("Can't Init CAN1");

  Serial.println("READY");

}

void loop() {

  message_std.data[6] = cnt;
  CAN_Ch1.send_message(&message_std);

  //tx ex id every 400ms
  if ((cnt % 4) == 3) {
    message_ex.data[2] = cnt;
    CAN_Ch1.send_message(&message_ex);
  }

  cnt = (cnt + 1) % 255;

  delay(100);
}

#include <mcp2515.h>

MCP2515 CAN_Ch1(10, 2);
MCP2515 CAN_Ch2(4, 3);
tCAN tx_msg;
tCAN tx_msg_ch2;
uint8_t cnt = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //define CAN message for ch1
  tx_msg.id.std = 0x284; //Standard CAN ID
  tx_msg.header.rtr = 0;
  tx_msg.header.ide = 0;
  tx_msg.header.dlc = 8;
  tx_msg.data[0] = 0x01;
  tx_msg.data[1] = 0x02;
  tx_msg.data[2] = 0x03;
  tx_msg.data[3] = 0x04;
  tx_msg.data[4] = 0x05;
  tx_msg.data[5] = 0x06;
  tx_msg.data[6] = 0x07;
  tx_msg.data[7] = cnt;

  //define CAN message for ch2
  tx_msg_ch2.id.ex = 0x18DAD1F1; //extended CAN ID
  tx_msg_ch2.header.rtr = 0;
  tx_msg_ch2.header.ide = 1;  //enable extended CAN ID Tx
  tx_msg_ch2.header.dlc = 8;
  tx_msg_ch2.data[0] = 0x01;
  tx_msg_ch2.data[1] = 0x02;
  tx_msg_ch2.data[2] = 0x03;
  tx_msg_ch2.data[3] = 0x04;
  tx_msg_ch2.data[4] = 0x05;
  tx_msg_ch2.data[5] = 0x06;
  tx_msg_ch2.data[6] = cnt;
  tx_msg_ch2.data[7] = 0x0A;

  //Initialise CH1 MCP2515 CAN controller at the specified speed
  if (CAN_Ch1.CAN_init(CANSPEED_500kBPS)){
	CAN_Ch1.set_mode(MODE_NORMAL); //set MCP2515 CAN controller into NORMAL mode (init default mode is LISTEN_ONLY)
    Serial.println("CAN1 Init ok");
  }
  else
    Serial.println("Can't Init CAN1");

  //Initialise CH1 MCP2515 CAN controller at the specified speed
  if (CAN_Ch2.CAN_init(CANSPEED_500kBPS)){
	CAN_Ch2.set_mode(MODE_NORMAL); //set MCP2515 CAN controller into NORMAL mode (init default mode is LISTEN_ONLY)
    Serial.println("CAN1 Init ok");
  }
  else
    Serial.println("Can't Init CAN2");

  Serial.println("READY");
}

void loop() {
  tx_msg.data[7] = cnt;
  tx_msg_ch2.data[6] = cnt;

  CAN_Ch1.send_message(&tx_msg);

  CAN_Ch2.send_message(&tx_msg_ch2);

  cnt = (cnt + 1) % 255;

  delay(100);
}

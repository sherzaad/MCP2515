/* This is for use with Sparkfun's CAN Bus Shield: https://www.sparkfun.com/products/10039 */

#include "mcp2515_defs.h"
#include "mcp2515.h"
#include "SPI.h"

//Calling of parameterized constructor with array of object.
//channel_1: cs_pin=10, intr_pin=2
//channel_2: cs_pin=4, intr_pin=3

MCP2515 CAN_Channels[2]={{10,2}, {4,3}} ;

//MCP2515 CAN_Ch1(10, 2);
//MCP2515 CAN_Ch2(4, 3);

uint8_t ch_select =1;

void setup() {

  //Initialise Baudrate for board serial interface
  Serial.begin(115200);

  delay(500);

  Serial.println("Init Start");

  //Initialise CH1 MCP2515 CAN controller at the specified speed
  if (CAN_Channels[0].CAN_init(CANSPEED_500kBPS))
    Serial.println("CAN1 Init ok");
  else {
    Serial.println("Can't Init CAN1");
  }

  delay(500);

  //Initialise CH2 MCP2515 CAN controller at the specified speed
  if (CAN_Channels[1].CAN_init(CANSPEED_500kBPS))
    Serial.println("CAN2 Init ok");
  else {
    Serial.print("Can't Init CAN2. ");
  }
  
}

void loop(){ 
tCAN message;
 
  if (CAN_Channels[ch_select].check_message()){ 
    if (CAN_Channels[ch_select].get_message(&message)){
      Serial.print("CAN_");
      Serial.print(ch_select);
      Serial.print(" ID: ");
      if(message.header.ide)  Serial.print(message.id.ex, HEX); //extended CAN ID
      else Serial.print(message.id.std, HEX); //standard CAN ID
      Serial.print(", ");
      Serial.print("Data: ");
      for(int i=0;i<message.header.dlc;i++){
        Serial.print(message.data[i],HEX);
        Serial.print(" ");
      }
      Serial.println("");
    }
  }

  ch_select^=0x01; //XOR allows toggle between CH1 and CH2
}

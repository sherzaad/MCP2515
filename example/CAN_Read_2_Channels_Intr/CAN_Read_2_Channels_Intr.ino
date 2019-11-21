/* This is for use with Sparkfun's CAN Bus Shield: https://www.sparkfun.com/products/10039 */
#include "mcp2515.h"

MCP2515 CAN_Ch1(10, 2);
MCP2515 CAN_Ch2(4, 3);

//ch1 and ch2 circular buffer variables
#define MAX_SIZE 25
volatile tCAN ch1_messages[MAX_SIZE - 1], ch2_messages[MAX_SIZE - 1];
volatile uint8_t ch1_front = 0, ch2_front = 0, ch1_rear = MAX_SIZE, ch2_rear = MAX_SIZE;

void intr_pin2() {
  if (ch1_rear == MAX_SIZE) ch1_rear = 0;

  CAN_Ch1.get_message(&ch1_messages[ch1_rear]);

  ++ch1_rear;
}

void intr_pin3() {
  if (ch2_rear == MAX_SIZE) ch2_rear = 0;

  CAN_Ch2.get_message(&ch2_messages[ch2_rear]);

  ++ch2_rear;
}

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
  if (CAN_Ch2.CAN_init(CANSPEED_500kBPS))
    Serial.println("CAN2 Init ok");
  else
    Serial.println("Can't Init CAN2");

  Serial.println("READY");

  //enable interrupts
  //setup interrupt on pin 2 for CH1 data ready detection
  attachInterrupt(digitalPinToInterrupt(2), intr_pin2, LOW);
  //setup interrupt on pin 3 for CH2 data ready detection
  attachInterrupt(digitalPinToInterrupt(3), intr_pin3, LOW);
}

void loop() {

  if (ch1_rear != MAX_SIZE && ch1_front < ch1_rear + 1) {
    if (ch1_messages[ch1_front].header.ide) {
      String str = "CAN1 ID: " + String(ch1_messages[ch1_front].id.ex, HEX) + ", Data: "; //extended CAN ID
      Serial.print(str);
    }
    else {
      String str = "CAN1 ID: " + String(ch1_messages[ch1_front].id.std, HEX) + ", Data: ";//standard CAN ID
      Serial.print(str);
    }


    for (uint8_t i = 0; i < ch1_messages[ch1_front].header.dlc; i++) {
      Serial.print(String(ch1_messages[ch1_front].data[i], HEX) + " ");
    }
    Serial.println("");

    ++ch1_front;

    if (ch1_front == MAX_SIZE) ch1_front = 0;
  }

  if (ch2_rear != MAX_SIZE && ch2_front < ch2_rear + 1) {
    if (ch2_messages[ch2_front].header.ide) {
      String str = "CAN1 ID: " + String(ch2_messages[ch2_front].id.ex, HEX) + ", Data: "; //extended CAN ID
      Serial.print(str);
    }
    else {
      String str = "CAN1 ID: " + String(ch2_messages[ch2_front].id.std, HEX) + ", Data: ";//standard CAN ID
      Serial.print(str);
    }
    for (uint8_t i = 0; i < ch2_messages[ch2_front].header.dlc; i++) {
      Serial.print(String(ch2_messages[ch2_front].data[i], HEX) + " ");
    }
    Serial.println("");

    ++ch2_front;

    if (ch2_front == MAX_SIZE) ch2_front = 0;
  }
}

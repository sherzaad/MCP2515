/* This is for use with Sparkfun's CAN Bus Shield: https://www.sparkfun.com/products/10039 */

#include "mcp2515.h"

MCP2515 CAN_Ch1(10, 2);
MCP2515 CAN_Ch2(4, 3);

const uint8_t msg_5A9_MASK[8] = {0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //set to '0' bits to be modified in MASK, bits passed through set as '1'.
const uint8_t msg_5A9[8] = {0x00, 0x4E, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00}; //bits passed through set as '0' in modified message

const uint8_t msg_5B3_mod_bytes[2] = {0x03, 0xBC};
const uint8_t msg_5B3_dlc = 8;
uint8_t *msg_5B3_arrayPtr[msg_5B3_dlc];

tCAN message; //CAN message container for all received and transmitted messages

uint8_t ch = 1;

void setup() {

  //Initialise Baudrate for board serial interface
  Serial.begin(115200);

  while (!Serial) {
    ;
  }

  //Initialise CH1 MCP2515 CAN controller at the specified speed
  if (CAN_Ch1.CAN_init(CANSPEED_500kBPS))
    Serial.println("CAN1 Init ok");
  else
    Serial.println("Can't Init CAN1");

  CAN_Ch1.set_mode(MODE_NORMAL);

  //Initialise CH2 MCP2515 CAN controller at the specified speed
  if (CAN_Ch2.CAN_init(CANSPEED_500kBPS))
    Serial.println("CAN2 Init ok");
  else
    Serial.println("Can't Init CAN2");

  CAN_Ch2.set_mode(MODE_NORMAL);

  //initialise pointer array to contain modded CAN message data
  msg_5B3_arrayPtr[0] = &message.data[0];
  msg_5B3_arrayPtr[1] = &message.data[1];
  msg_5B3_arrayPtr[2] = &message.data[2];
  msg_5B3_arrayPtr[3] = &msg_5B3_mod_bytes[0];
  msg_5B3_arrayPtr[4] = &msg_5B3_mod_bytes[1];
  msg_5B3_arrayPtr[5] = &message.data[5];
  msg_5B3_arrayPtr[6] = &message.data[6];
  msg_5B3_arrayPtr[7] = &message.data[7];

  Serial.println("READY");

}

void loop() {
uint64_t *i_ptr, *o_ptr, *mask_ptr; //uint64_t selected as data type since max array size here is 8.
  //for smaller arrays, may be possible other data type (uint32_t, uint16_t) depending on array size

  if (ch == 1) {
    if (CAN_Ch1.check_message()) {
      if (CAN_Ch1.get_message(&message)) {

        switch (message.id.std) {
          case 0x5A9:
            //assign received 'data' address to i_ptr. pointer casting required since using different data types
            i_ptr = reinterpret_cast<uint64_t>(&message.data[0]);
            //assign msg_5A9_MASK address to mask_ptr. pointer casting required since using different data types
            mask_ptr = reinterpret_cast<uint64_t>(msg_5A9_MASK);
            //assign msg_5A9 address to mask_ptr. pointer casting required since using different data types
            o_ptr = reinterpret_cast<uint64_t>(msg_5A9);
            //data operation to fix values of desired bits.
            *i_ptr = (*i_ptr & *mask_ptr) | *o_ptr;
            //output to other channel
            CAN_Ch2.send_message(&message);
            break;

          case 0x50D:
            //easier to just set the array element since only one byte involved here! :)
            message.data[0] = 0x64;
            CAN_Ch2.send_message(&message);
            break;

          case 0x5B3:
            //and yet another fancy method of modding your CAN message data
            CAN_Ch2.send_message(&message, msg_5B3_arrayPtr);
            break;

          default:
            CAN_Ch2.send_message(&message);
        }
      }
    }
  }
  else {
    if (CAN_Ch2.check_message()) {
      if (CAN_Ch2.get_message(&message)) {

        switch (message.id.std) {
          case 0x5A9:
            i_ptr = reinterpret_cast<uint64_t>(&message.data[0]);
            mask_ptr = reinterpret_cast<uint64_t>(msg_5A9_MASK);
            o_ptr = reinterpret_cast<uint64_t>(msg_5A9);
            *i_ptr = (*i_ptr & *mask_ptr) | *o_ptr;
            CAN_Ch1.send_message(&message);
            break;

          case 0x50D:
            message.data[0] = 0x64;
            CAN_Ch1.send_message(&message);
            break;

          case 0x5B3:
            CAN_Ch1.send_message(&message, msg_5B3_arrayPtr);
            break;

          default:
            CAN_Ch1.send_message(&message);
        }
      }
    }
  }

  ch ^= 0x03; //XOR allows toggle between CH1 and CH2
}

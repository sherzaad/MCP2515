# MCP2515
MCP2515 Arduino IDE compatible library for Standard CAN communication

Supports 8MHz and 16MHz CAN Sheilds.

All examples are for 16MHz CAN sheild.

To use this library for other CAN speeds, refer to mcp2515_defs.h file to identify with CFG1, CFG2, and CFG3 DEFINEs to apply when calling 'mcp2515.CAN_init(cfg1, cfg2,cfg3)' to initialise your sheild.

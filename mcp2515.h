/*
mcp2515.h - Library for mcp2015 Arduino CAN shield
ver1.1 created by Sherzaad Dinah

Revision History
ver1.0 - Newly created
ver1.1 - added tCAN modify_message(tCAN *message, uint16_t byte_arr[]) inline function. Allows bytes within received CAN message to be modified
*/
#ifndef	MCP2515_H
#define	MCP2515_H

#include <Arduino.h>
#include <mcp2515_defs.h>
#include <SPI.h>

#pragma pack(1) // To force compiler to use 1 byte packaging. enforces stricter packaging

#define BIT_CHK(x,y) (((x)&(1<<(y)))>0? 1:0) //returns value selected bit '0' or '1'

typedef struct
{
	union { 
	    uint32_t ex;
		struct{
			uint16_t std;
			uint16_t stdh;
		}__attribute__((packed)); //avoids structure padding 
    } id;
	struct {
		uint8_t rtr : 1;
		uint8_t ide : 1; //IDE: IDentifier Extension bit - is '1' for extended frame format with 29-bit identifiers 		
		uint8_t dlc : 4;
	}__attribute__((packed))header; //avoids structure padding
	uint8_t data[8];
} tCAN;

class MCP2515{

public:
	uint8_t get_message(tCAN *message);
	
	uint8_t send_message(tCAN *message, uint8_t *alt_data[ ]=nullptr);
	
	uint8_t CAN_init(uint8_t cnf1, uint8_t cnf2 = (1<<BTLMODE)|(1<<PHSEG11), uint8_t cnf3 = (1<<PHSEG21));
	
	uint8_t init_filter(uint16_t *Std_CAN_id_Arr, uint8_t std_arr_size, uint32_t *Ex_CAN_id_Arr, uint8_t ex_arr_size);
	
	uint8_t init_filter(uint16_t *Std_CAN_id_Arr, uint8_t std_arr_size, uint8_t reg_select=0);
	
	uint8_t init_filter(uint32_t *Ex_CAN_id_Arr, uint8_t ex_arr_size, uint8_t reg_select=0);
	
	uint8_t enable_filter();
	
	uint8_t disable_filter();
	
	uint8_t set_filter_mask(uint8_t mask_reg, uint32_t mask);
	
	uint8_t set_filter_mask(uint8_t mask_reg, uint16_t mask);

	inline void set_mode(uint8_t mode)
	{
		/*five modes of operation available
			1. Configuration mode (MODE_CONFIG)
			2. Normal mode (MODE_NORMAL)
			3. Sleep mode (MODE_SLEEP)
			4. Listen-Only mode (MODE_LISTENONLY)
			5. Loopback mode (MODE_LOOPBACK)
		*/
		
		
		#ifdef HW_Mode
			#undef HW_Mode
		#endif
		
		#define HW_Mode
		mode_chip = mode;
		
		bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), mode_chip);
		
	}
	
	inline void RESET_CS()
	{
		digitalWrite(cs_pin,LOW);
	}
	
	inline void SET_CS()
	{
		digitalWrite(cs_pin,HIGH);	
	}
	
	inline uint8_t check_message()
	{
		return !digitalRead(intr_pin);
	}
	
	inline void bit_modify(uint8_t adress, uint8_t mask, uint8_t data)
	{ 
	  
	  RESET_CS();
	  
	  SPI.transfer(SPI_BIT_MODIFY);
	  SPI.transfer(adress);
	  SPI.transfer(mask);
	  SPI.transfer(data);
	  
	  SET_CS();
	}

	inline void write_register( uint8_t adress, uint8_t data)
	{
	  
	  RESET_CS();
	  
	  SPI.transfer(SPI_WRITE);
	  SPI.transfer(adress);
	  SPI.transfer(data);
	  
	  SET_CS();
	}
	
	inline void write_register( uint8_t adress, uint8_t *data, uint8_t n)
	{
	  
	  RESET_CS();
	  
	  SPI.transfer(SPI_WRITE);
	  SPI.transfer(adress);
	  for(uint8_t i=0;i<n;++i){
		SPI.transfer(*(data+i));
	  }
	  
	  SET_CS();
	}
	
	inline uint8_t read_register(uint8_t adress)
	{
	  uint8_t data;
	  
	  RESET_CS();
	  
	  SPI.transfer(SPI_READ);
	  SPI.transfer(adress);
	  
	  data = SPI.transfer(0xff);  
	  
	  SET_CS();
	  
	  return data;
	}

	inline uint8_t read_status(uint8_t type)
	{
	  uint8_t data;
	  
	  RESET_CS();
	  
	  SPI.transfer(type);
	  data = SPI.transfer(0xff);
	  
	  SET_CS();
	  
	  return data;
	}

	//returns Tx buffer status and message Tx error status
	inline uint8_t Tx_buffer_Status()
	{
	  /*Bits defined  in mcp2515_defs.h for TXBnCTRL (n = 0, 1, 2)
		ABTF	Message Aborted Flag bit (1: Err, 0: OK)
		MLOA	Message Lost Arbitration bit (1: Err, 0: OK)
		TXERR	Transmission Error Detected bit (1: Err, 0: OK)
		TXREQ	Message Transmit Request bit (1: pending Tx, 0: Tx complete)
		TXP1	Transmit Buffer Priority bits[1:0]
		TXP0	
	  */
	  
	  uint8_t r_status = read_status(TXB0CTRL);
	  
	  r_status |= read_status(TXB1CTRL);
	  r_status |= read_status(TXB2CTRL);
	  
	  return r_status;
	}
	
	MCP2515(uint8_t cs,uint8_t intr)
	{
		cs_pin = cs;
		intr_pin = intr;
	}
	
	~MCP2515(){
		#ifdef SPI_Init
			#undef SPI_Init
		#endif
		SPI.end();
	}

private:
	uint8_t cs_pin;
	uint8_t intr_pin;
	uint8_t mode_chip;

};

#endif

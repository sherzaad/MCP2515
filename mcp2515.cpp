/*
mcp2515.ccp - Library for mcp2015 Arduino CAN shield
ver1.0 created by Sherzaad Dinah
*/

#include <mcp2515.h>

uint8_t MCP2515::get_message(tCAN *message)
{
uint8_t r_status, addr, length;
  // read status
  r_status = read_status(SPI_RX_STATUS);

  //SPI.begin();
  
  if (BIT_CHK(r_status,6)) {
    // message in buffer 0
    addr = SPI_READ_RX;
  }
  else if (BIT_CHK(r_status,7)) {
    // message in buffer 1
    addr = SPI_READ_RX | 0x04;
  }
  else {
    // Error: no message available
    return 0;
  }

  RESET_CS();
  SPI.transfer(addr);
  
  message->header.rtr = BIT_CHK(r_status, 3);
  message->header.ide = BIT_CHK(r_status, 4); 
  
  // read ex-id
  if(message->header.ide){
	message->id.stdh  = (uint16_t) SPI.transfer(0xff) << 5;
	uint8_t temp = SPI.transfer(0xff);
	message->id.stdh |= (((temp >> 3)&0xFC)|(temp&0x03));
	message->id.std = (uint16_t) SPI.transfer(0xff) << 8;
	message->id.std |= SPI.transfer(0xff);
  }
  else{
	// read std-id
	message->id.std  = (uint16_t) SPI.transfer(0xff) << 3;
	message->id.std |=            SPI.transfer(0xff) >> 5;
	SPI.transfer(0xff);
	SPI.transfer(0xff);
  }
  
  // read DLC
  length = SPI.transfer(0xff) & 0x0f;
  
  message->header.dlc = length;
  
  // read data
  for (uint8_t t=0;t<length;t++) {
    message->data[t] = SPI.transfer(0xff);
  }
  SET_CS();
  
  // clear interrupt flag
  if (BIT_CHK(r_status, 6)) {
    bit_modify(CANINTF, (1<<RX0IF),0);
  }
  else {
    bit_modify(CANINTF, (1<<RX1IF), 0);
  }
  
  return (r_status & 0x07) + 1;
}

uint8_t MCP2515::send_message(tCAN *message, uint8_t *alt_data[ ]=nullptr)
{
uint8_t r_status, addr, length;

  r_status = read_status(SPI_READ_STATUS);
  
  /* Statusbyte:
   *
   * Bit  Function
   *  2 TXB0CNTRL.TXREQ
   *  4 TXB1CNTRL.TXREQ
   *  6 TXB2CNTRL.TXREQ
   */

  //SPI.begin();
  
  if (!(BIT_CHK(r_status, 2))) {
    addr = 0x00;
  }
  else if (!(BIT_CHK(r_status, 4))) {
    addr = 0x02;
  } 
  else if (!(BIT_CHK(r_status, 6))) {
    addr = 0x04;
  }
  else {
    // all buffer used => could not send message
    return 0;
  }
  
  RESET_CS();
  SPI.transfer(SPI_WRITE_TX | addr);


  //send ex-id
  if(message->header.ide){
	SPI.transfer(message->id.stdh >> 5);
	uint8_t temp = ((message->id.stdh << 3)&0xE0)|((message->id.stdh)&0x03)|0x08; //)0x08 : EXIDE=1
	SPI.transfer(temp);
	SPI.transfer(message->id.std >> 8);
	SPI.transfer(message->id.std);
  }
  else{
	// send std-id
	  SPI.transfer(message->id.std >> 3);
	  SPI.transfer(message->id.std << 5);
	  SPI.transfer(0);
	  SPI.transfer(0);
  }  

  
  length = message->header.dlc & 0x0f;
  
  if (message->header.rtr) {
    // a rtr-frame has a length, but contains no data
    SPI.transfer((1<<RTR) | length);
  }
  else {
    // set message length
    SPI.transfer(length);
    
    // data
	if(alt_data==nullptr){
		for (uint8_t t=0;t<length;t++) {
			SPI.transfer(message->data[t]);
		}
	}
	else{
		for (uint8_t t=0;t<length;t++) {
			SPI.transfer(*alt_data[t]);
		}	
	}
  }
  SET_CS();
  
  //_delay_us(1);
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"); //nanosecond delay (4 * 1/freq clk)
												 //small delay to allow CS switching during SET/RESET
  // send message
  RESET_CS();
  addr = (addr == 0) ? 1 : addr;
  SPI.transfer(SPI_RTS | addr);
  SET_CS();
  
  return addr;
}

uint8_t MCP2515::CAN_init(uint8_t cnf1, uint8_t cnf2, uint8_t cnf3)
{ 
	//initialise IOs and SPI comms
	pinMode(cs_pin,OUTPUT);   //MCP2515_CS
	digitalWrite(cs_pin,HIGH);//Deselect Chip
	pinMode(intr_pin,INPUT);     //MCP2515_INT
	
	#ifndef SPI_Init
		#define SPI_Init
		//SPI clock speed:10MHz, Data Shift:MSB First, Data Clock Idle: SPI_MODE0
		SPI.beginTransaction(SPISettings(10000000,MSBFIRST,SPI_MODE0)); 
		SPI.begin();
	#endif
  
  // reset MCP2515 by software reset.
  // After this he is in configuration mode.
  RESET_CS();
  SPI.transfer(SPI_RESET);
  SET_CS();
  
  //1ms delay to wait until the MCP2515 has restarted
  delay(1);
  
  // load CNF1..3 Register
  RESET_CS();
  SPI.transfer(SPI_WRITE);
  SPI.transfer(CNF3);
 
  SPI.transfer(cnf3);   // Bitrate 250 kbps at 16 MHz
  SPI.transfer(cnf2);

  SPI.transfer(cnf1);

  // activate interrupts
  SPI.transfer((1<<RX1IE)|(1<<RX0IE));
  SET_CS();
  
  // test if we could read back the value => is the chip accessible?
  if (read_register(CNF1) != cnf1) {
    return 0;
  }
  
  // deaktivate the RXnBF Pins (High Impedance State)
  write_register(BFPCTRL, 0);
  
  // set TXnRTS as inputs
  write_register(TXRTSCTRL, 0);
  
  // turn off filters => receive any message
  write_register(RXB0CTRL, (1<<RXM1)|(1<<RXM0));
  write_register(RXB1CTRL, (1<<RXM1)|(1<<RXM0));
  
  // reset device to MODE_LISTENONLY
  write_register(CANCTRL, MODE_LISTENONLY);
  
  return 1;
}

uint8_t MCP2515::set_filter_mask(uint8_t mask_reg, uint16_t mask)
{
  // config mode
  if (read_register(CANCTRL) != MODE_CONFIG){
	write_register(CANCTRL, MODE_CONFIG);
	if (read_register(CANCTRL) != MODE_CONFIG) return 0;
  }
  
  uint8_t data[4] = {(mask >> 3),(mask << 5),0,0};
  
  write_register(mask_reg, data, 4); 
  
  #ifdef HW_Mode
	//reset device initial set mode
	write_register(CANCTRL, mode_chip);
  #else
    //reset device to MODE_LISTENONLY
    write_register(CANCTRL, MODE_LISTENONLY);
  #endif
  
  return 1;
}

uint8_t MCP2515::set_filter_mask(uint8_t mask_reg, uint32_t mask)
{
union { 
	uint32_t ex;
	struct{
		uint16_t std;
		uint16_t stdh;
	}__attribute__((packed));
} id;

  // config mode
  if (read_register(CANCTRL) != MODE_CONFIG){
	write_register(CANCTRL, MODE_CONFIG);
	if (read_register(CANCTRL) != MODE_CONFIG) return 0;
  }

  id.ex = mask;
  
  uint8_t data[4] = {(id.stdh >> 5),((id.stdh << 3)&0xE0)|(id.stdh&0x03),(id.std >> 8),id.std};
  
  write_register(mask_reg, data, 4); 
  
  #ifdef HW_Mode
	//reset device initial set mode
	write_register(CANCTRL, mode_chip);
  #else
    //reset device to MODE_LISTENONLY
    write_register(CANCTRL, MODE_LISTENONLY);
  #endif
  
  return 1;
}

uint8_t MCP2515::enable_filter()
{
uint8_t rxbnctrl;

  // config mode
  if (read_register(CANCTRL) != MODE_CONFIG){
	write_register(CANCTRL, MODE_CONFIG);
	if (read_register(CANCTRL) != MODE_CONFIG) return 0;
  }
  
  
  // turn on filters => Receive all valid messages using either standard or extended identifiers that meet filter criteria
  rxbnctrl = (~((1<<RXM1)|(1<<RXM0))) & read_register(RXB0CTRL);
	
  write_register(RXB0CTRL,rxbnctrl);
	
  rxbnctrl = (~((1<<RXM1)|(1<<RXM0))) & read_register(RXB1CTRL);
	
  write_register(RXB1CTRL,rxbnctrl);
	
  #ifdef HW_Mode
	//reset device initial set mode
	write_register(CANCTRL, mode_chip);
  #else
    //reset device to MODE_LISTENONLY
    write_register(CANCTRL, MODE_LISTENONLY);
  #endif
  
  return 1;
}

uint8_t MCP2515::disable_filter()
{
  // config mode
  if (read_register(CANCTRL) != MODE_CONFIG){
	write_register(CANCTRL, MODE_CONFIG);
	if (read_register(CANCTRL) != MODE_CONFIG) return 0;
  }
  
  // turn off filters => receive any message
  write_register(RXB0CTRL, (1<<RXM1)|(1<<RXM0));
  write_register(RXB1CTRL, (1<<RXM1)|(1<<RXM0));
  
  // reset device to normal mode
  write_register(CANCTRL, 0);
  
  return 1;
}

uint8_t MCP2515::init_filter(uint16_t *Std_CAN_id_Arr, uint8_t std_arr_size, uint32_t *Ex_CAN_id_Arr, uint8_t ex_arr_size)
{
uint8_t val;

	if(std_arr_size+ex_arr_size>6) return 0;
	
	if(std_arr_size>2){
		val = init_filter(Std_CAN_id_Arr, std_arr_size, 4); //use 2nd mask filter for std msg 
		val &= init_filter(Ex_CAN_id_Arr, ex_arr_size, 1);	 //use 1st mask filter for ext msg
	}
	else{
		val = init_filter(Std_CAN_id_Arr, std_arr_size, 1); //use 1st mask filter for std msg
		val &= init_filter(Ex_CAN_id_Arr, ex_arr_size, 4);	 //use 2nd mask filter for ext msg	
	}
	
	return val;
}

uint8_t MCP2515::init_filter(uint16_t *Std_CAN_id_Arr, uint8_t std_arr_size, uint8_t reg_select)
{
uint8_t filter_regs[8] = {RXM0SIDH, RXF0SIDH, RXF1SIDH, RXM1SIDH, RXF2SIDH, RXF3SIDH, RXF4SIDH, RXF5SIDH};
uint8_t u_bound, rxbnctrl, i=0, j=0;

	// config mode
    write_register(CANCTRL, MODE_CONFIG);
	
    if (read_register(CANCTRL) != MODE_CONFIG) return 0;
	else if(std_arr_size>6) return 0;
	else{
		uint8_t data[4] = {0xFF,0xFF,0xFF,0xFF};
		
		//init RX mask registers. 11-bit mask: 0x7FF, 29-bit mask: 0x1FFFFFFF <----No masking applied to filter CAN Ids	
		write_register(filter_regs[((reg_select==1) ? 3: 0)], data, 4);
		
		if(reg_select>0){
			data[2]=0x00;
			data[3]=0x00;
		}
		
		write_register(filter_regs[((reg_select==1) ? 0: 3)], data, 4);
	}
	
	u_bound = ((reg_select==1) ? 3: 8);
	
	for(i=reg_select;i<u_bound;++i){
		uint16_t std = *(Std_CAN_id_Arr+((j<std_arr_size) ? j: 0));
		uint8_t data[4] = {(std >> 3),(std<< 5),0,0};
		write_register(filter_regs[i], data, 4);
		++j;	
	}
	
	// turn on filters => Receive all valid messages using either standard or extended identifiers that meet filter criteria
	rxbnctrl = ((~((1<<RXM1)|(1<<RXM0))) & read_register(RXB0CTRL))|(1<<BUKT);
	
	write_register(RXB0CTRL,rxbnctrl);
	
	rxbnctrl = (~((1<<RXM1)|(1<<RXM0))) & read_register(RXB1CTRL);
	
	write_register(RXB1CTRL,rxbnctrl);
	
  #ifdef HW_Mode
	//reset device initial set mode
	write_register(CANCTRL, mode_chip);
  #else
    //reset device to MODE_LISTENONLY
    write_register(CANCTRL, MODE_LISTENONLY);
  #endif
	
	return 1;
}

uint8_t MCP2515::init_filter(uint32_t *Ex_CAN_id_Arr, uint8_t ex_arr_size, uint8_t reg_select)
{
union { 
	uint32_t ex;
	struct{
		uint16_t std;
		uint16_t stdh;
	}__attribute__((packed));
} id;

uint8_t filter_regs[8] = {RXM0SIDH, RXF0SIDH, RXF1SIDH, RXM1SIDH, RXF2SIDH, RXF3SIDH, RXF4SIDH, RXF5SIDH};
uint8_t u_bound, rxbnctrl, i=0, k=0;

	// config mode
    write_register(CANCTRL, MODE_CONFIG);
	
    if (read_register(CANCTRL) != MODE_CONFIG) return 0;
	else if(ex_arr_size>6) return 0;
	else{
		uint8_t data[4] = {0xFF,0xFF,0xFF,0xFF};
		
		//init RX mask registers. 11-bit mask: 0x7FF, 29-bit mask: 0x1FFFFFFF <----No masking applied to filter CAN Ids	
		write_register(filter_regs[((reg_select==1) ? 0: 3)], data, 4);
		
		if(reg_select>0){
			data[2]=0x00;
			data[3]=0x00;
		}
		
		write_register(filter_regs[((reg_select==1) ? 3: 0)], data, 4);
	}
	
	u_bound = ((reg_select==1) ? 3: 8);
	
	for(i=reg_select;i<u_bound;++i){
		id.ex = *(Ex_CAN_id_Arr+((k<ex_arr_size) ? k: 0));
		uint8_t data[4] = {(id.stdh >> 5),(((id.stdh << 3)&0xE0)|(id.stdh&0x03)|0x08),(id.std >> 8),id.std};
		write_register(filter_regs[i], data, 4);			
		++k;		
	}

	// turn on filters => Receive all valid messages using either standard or extended identifiers that meet filter criteria
	rxbnctrl = ((~((1<<RXM1)|(1<<RXM0))) & read_register(RXB0CTRL))|(1<<BUKT);
	
	write_register(RXB0CTRL,rxbnctrl);
	
	rxbnctrl = (~((1<<RXM1)|(1<<RXM0))) & read_register(RXB1CTRL);
	
	write_register(RXB1CTRL,rxbnctrl);
	
  #ifdef HW_Mode
	//reset device initial set mode
	write_register(CANCTRL, mode_chip);
  #else
    //reset device to MODE_LISTENONLY
    write_register(CANCTRL, MODE_LISTENONLY);
  #endif
	
	return 1;
}	


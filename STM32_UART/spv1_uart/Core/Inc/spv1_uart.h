/*
 * spv1_uart.h
 *
 *  Created on: Oct 4, 2022
 *      Author: Sümeyye
 */

#ifndef INC_SPV1_UART_H_
#define INC_SPV1_UART_H_

#include <stdint.h>
#include "main.h"
#include "uart_driver.h"


//Commands
#define CMD_ACTIVATE_ALL           0x83
#define CMD_SEEK_FOR_TAG           0X82
#define CMD_ACTIVATE_IDLE          0x84
#define CMD_HALT                   0x93
#define CMD_AUTHENTICATE           0x85
#define CMD_READ_BLOCK             0x86
#define CMD_WRITE_BLOCK            0x89
#define CMD_WRITE_BLOCK_4_BYTE     0x8B
#define CMD_READ_VALUE_BLOCK       0x87
#define CMD_WRITE_VALUE_BLOCK      0x8A
#define CMD_INCREMENT_VALUE_BLOCK  0x8D
#define CMD_DECREMENT_VALUE_BLOCK  0x8E

//RXSTATE
#define RXSTATE_SOF            0
#define RXSTATE_ADDRESS        1
#define RXSTATE_DATACOUNT_MSB  2
#define RXSTATE_DATACOUNT_LSB  3
#define RXSTATE_COMMAND        4
#define RXSTATE_DATA           5
#define RXSTATE_CHECKSUM       6

//MIN-MAX VALUE
#define MAX_TIMEOUT      5   //5ms
#define MAX_DATA_COUNT  528

#define SERIAL_HEADER_BYTE   0xFF

//FLAGS
#define SERIAL_FLAG_RX_IN_PROGRESS  0x01
#define SERIAL_FLAG_COMMAND_READY   0x02

typedef struct {
uint8_t RxState;
uint16_t ReceivedDataCount;
uint8_t rxFlag;
uint16_t RxSerialTimeOut;
uint8_t txFlag;
uint8_t reserved;
} tsSerial ;

typedef struct {
uint8_t HeaderByte;
uint8_t DataLength ;
uint8_t NodeAddress;
uint8_t CommandByte;
uint8_t Checksum;
uint8_t RxDataBuffer[MAX_DATA_COUNT];
} tsSPV1RxFrame;

extern volatile tsSerial Serial;
extern volatile tsSPV1RxFrame RxFrame;

extern volatile UART_HandleTypeDef huart1;
extern volatile UART_HandleTypeDef huart2;

extern uint8_t SerialCommand;

extern  uint8_t  rx_data[MAX_DATA_COUNT];
extern  uint8_t  Buf_Activate_All[5] ;
extern  uint8_t  Buf_Seek_For_Tag[5];
extern  uint8_t  Buf_Activate_Idle[5];
extern  uint8_t  Buf_Halt[5];
extern  uint8_t  Buf_Authenticate[7];
extern  uint8_t  Buf_Read_Block[6];
extern  uint8_t  Write_Block[16];
extern  uint8_t  Buf_Write_Block[22];
extern  uint8_t  Write_Block_4_Byte[4];
extern  uint8_t  Buf_Write_Block_4_Byte[10];
extern  uint8_t  Buf_Read_Value_Block[6];
extern  uint8_t  Write_Value_Block [4];
extern  uint8_t  Buf_Write_Value_Block[10];
extern  uint8_t  Increment_Value_Block[4];
extern  uint8_t  Buf_Increment_Value_Block [10];
extern  uint8_t  Decrement_Value_Block[4];
extern  uint8_t  Buf_Decrement_Value_Block[10];

extern uint8_t activate_all_checksum;
extern uint8_t seek_for_tag_checksum;
extern uint8_t activate_idle_checksum;
extern uint8_t halt_checksum;
extern uint8_t authenticate_checksum;
extern uint8_t read_block_checksum ;
extern uint8_t write_block_checksum;
extern uint8_t write_block_4_byte_checksum;
extern uint8_t read_value_block_checksum;
extern uint8_t write_value_block_checksum;
extern uint8_t increment_value_block_checksum;
extern uint8_t decrement_value_block_checksum;

extern uint8_t authenticate_block_no;
extern uint8_t read_block_no;
extern uint8_t write_block_no;
extern uint8_t write_block_4_byte_no;
extern uint8_t read_value_block_no;
extern uint8_t write_value_block_no;
extern uint8_t increment_value_block_no;
extern uint8_t decrement_value_block_no;


extern uint8_t SerialMaxDataLength ;

void comp_spv1_serial_init() ;
void spv1_uart_rx_event(uint8_t rx_data[MAX_DATA_COUNT]);
void Tx_Frame (uint8_t SerialCommand);
//void spv1_tx_transmit ();


void func_activate_all();
void func_seek_for_tag ();
void func_activate_idle();
void func_halt();
void func_authenticate();
void func_read_block ();
void func_write_block();
void func_read_value_block();
void func_write_value_block ();
void func_write_block_4_byte();
void func_increment_value_block();
void func_decrement_value_block();



#endif /* INC_SPV1_UART_H_ */

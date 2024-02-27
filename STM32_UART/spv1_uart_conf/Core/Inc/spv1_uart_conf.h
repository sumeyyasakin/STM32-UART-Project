/*
 * spv1_uart_conf.h
 *
 *  Created on: Oct 4, 2022
 *      Author: Sümeyye
 */

#ifndef INC_SPV1_UART_CONF_H_
#define INC_SPV1_UART_CONF_H_

#include <stdint.h>
#include "main.h"
#include "uart_driver.h"


//Commands
#define CMD_RESET                  0x80
#define CMD_GET_FIRMWARE_VERSION   0x81
#define CMD_ANTENNA_POWER          0x90
#define CMD_READ_INPUT             0x91
#define CMD_WRITE_OUTPUT           0x92
#define CMD_SET_BAUD_RATE          0x94
#define CMD_GET_I2C_ADDRESS        0X9C
#define CMD_POLL_BUFFER            0xB0
#define CMD_GET_APP_CONFIG         0xD8

//RXSTATE
#define RXSTATE_SOF            0
#define RXSTATE_ADDRESS        1
#define RXSTATE_DATACOUNT_MSB  2
#define RXSTATE_DATACOUNT_LSB  3
#define RXSTATE_COMMAND        4
#define RXSTATE_DATA           5
#define RXSTATE_CHECKSUM       6

//MIN-MAX VALUE
#define MAX_TIMEOUT   5  //5ms
#define MAX_DATA_COUNT  528

#define SERIAL_HEADER_BYTE  0xFF

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
} tsSerial;

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
extern uint8_t SerialMaxDataLength ;

extern uint8_t rx_data[MAX_DATA_COUNT];
extern uint8_t Buf_Reset[5];
extern uint8_t Buf_Get_Firmware_Version[5];
extern uint8_t Buf_Antenna_Power[6];
extern uint8_t Buf_Read_Input[5];
extern uint8_t Buf_Write_Output[6];
extern uint8_t Buf_Set_BaudRate[6];
extern uint8_t Buf_Get_I2C_Address[5];
extern uint8_t Buf_Poll_Buffer[5];
extern uint8_t Buf_Get_App_Config[6];


extern uint8_t reset_checksum;
extern uint8_t get_firmware_version_checksum;
extern uint8_t antenna_power_checksum;
extern uint8_t read_input_checksum;
extern uint8_t write_output_checksum;
extern uint8_t set_baudrate_checksum;
extern uint8_t get_i2c_address_checksum;
extern uint8_t poll_buffer_checksum;
extern uint8_t get_app_config_checksum;

extern uint8_t antenna_power_status;
extern uint8_t write_output_status;
extern uint8_t set_baudrate_status;

void comp_spv1_serial_init() ;
void spv1_uart_rx_event(uint8_t rx_data[MAX_DATA_COUNT]);
void Tx_Frame (uint8_t SerialCommand);

void func_reset();
void func_get_firmware_version();
void func_antenna_power();
void func_read_input ();
void func_write_output ();
void func_set_baudrate();
void func_get_I2C_address();
void func_poll_buffer();
void func_get_app_config();




#endif /* INC_SPV1_UART_CONF_H_ */

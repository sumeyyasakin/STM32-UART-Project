/*
 * spv1_uart_conf.c
 *
 *  Created on: Oct 4, 2022
 *      Author: Sümeyye
 */

#include "spv1_uart_conf.h"
//#include "uart_driver.h"

volatile tsSerial Serial;
volatile tsSPV1RxFrame RxFrame;


uint8_t SerialMaxDataLength ;
uint8_t rx_data[MAX_DATA_COUNT];

uint8_t SerialCommand = 0x81;

uint8_t antenna_power_status = 0x02;    //0x00 for switch off , 0x01 for switch on , 0x02 for reset

uint8_t write_output_status = 0x02;
/*0x00 = Output1: Low, Output2: Low
0x01 = Output1: High, Output2: Low
0x02 = Output1: Low, Output2: High
0x03 = Output1: High, Output2: High*/

uint8_t set_baudrate_status = 0x01;
/*New Baud Rate:
0x00 = 9600bps
0x01 = 19200 bps
0x02 = 38400bps
0x03 = 57600 bps
0x04 = 115200bps*/


void comp_spv1_serial_init()
{
    RxFrame.NodeAddress = 0x00;
  	Serial.RxState = RXSTATE_SOF;
  	RxFrame.Checksum = 0;
  	Serial.ReceivedDataCount = 0;
  	Serial.rxFlag = 0 ;
}

 void spv1_uart_rx_event (uint8_t rx_data[MAX_DATA_COUNT])
 {
     Serial.RxSerialTimeOut = MAX_TIMEOUT;
     Serial.rxFlag = SERIAL_FLAG_RX_IN_PROGRESS;

      switch (Serial.RxState)
      {
            case RXSTATE_SOF:
            {
                if(rx_data == SERIAL_HEADER_BYTE)
                {
                     RxFrame.HeaderByte = rx_data;
                     RxFrame.Checksum = 0;
                     Serial.ReceivedDataCount = 0;
                     Serial.RxState = RXSTATE_ADDRESS;
                }
             break;
            }

            case RXSTATE_ADDRESS:
            {
                 if((rx_data == 0) || (rx_data == RxFrame.NodeAddress))
                 {
                 	 RxFrame.NodeAddress = rx_data;
                 	 RxFrame.Checksum = rx_data;
                      Serial.RxState = RXSTATE_DATACOUNT_LSB;
                 }
                 else
                 {
                    Serial.RxState = RXSTATE_SOF;
                 }

             break;
            }
            case RXSTATE_DATACOUNT_LSB:
            {
         	   RxFrame.DataLength = rx_data;
                if((RxFrame.DataLength > MAX_DATA_COUNT + 1) ||(RxFrame.DataLength == 0))
                {
                  Serial.RxState = RXSTATE_SOF;
                }
                else
                {
             	 RxFrame.Checksum += rx_data;
                  Serial.RxState = RXSTATE_COMMAND;
                }
              break;
             }
            case RXSTATE_COMMAND:
            {

         	   RxFrame.CommandByte = rx_data;
                Serial.ReceivedDataCount =0;
                RxFrame.Checksum += rx_data;

                if (RxFrame.DataLength == 1)
                {
                  Serial.RxState = RXSTATE_CHECKSUM;
                }
                else
                {
                   Serial.RxState = RXSTATE_DATA;
                }
              break ;
             }
             case RXSTATE_DATA:
             {
             	RxFrame.RxDataBuffer[Serial.ReceivedDataCount ++]= rx_data;
             	RxFrame.Checksum += rx_data;

                if(Serial.ReceivedDataCount == RxFrame.DataLength-1)
                {

                  Serial.RxState = RXSTATE_CHECKSUM;
                }

              break;
             }
             case RXSTATE_CHECKSUM:
             {
                  if (RxFrame.Checksum == rx_data)
                  {
                    Serial.RxState = RXSTATE_SOF;
                    Serial.rxFlag = SERIAL_FLAG_COMMAND_READY;
                    spv1_uart_command_parser(RxFrame.CommandByte);

                  }
                  else
                  {
                    Serial.RxState = RXSTATE_SOF;
                  }

              break;
             }

             default:
             {
                Serial.RxState = RXSTATE_SOF;

              break ;
             }
        }
}


void Tx_Frame (uint8_t SerialCommand)
{
	switch(SerialCommand)
	{
	  case CMD_RESET:
	  {
		  //CmdReset
		  uint8_t Buf_Reset[5];
		  uint8_t reset_checksum = 0;

		  Buf_Reset[0] = 0xFF;
		  Buf_Reset[1] = 0x00;
		  Buf_Reset[2] = 0x01;
		  Buf_Reset[3] = CMD_RESET;

		  for(int i = 1 ; i < sizeof(Buf_Reset) - 1 ; i++)
		  {
			  reset_checksum = Buf_Reset[i] + reset_checksum;
		  }

		  Buf_Reset[4] = reset_checksum;
		  UART_driver_send_data(Buf_Reset , sizeof(Buf_Reset));
	      reset_checksum = 0;

		break;
	  }

	  case CMD_GET_FIRMWARE_VERSION:
	  {
		  //CmdGetFirmwareVersion
		   uint8_t Buf_Get_Firmware_Version[5];
		   uint8_t get_firmware_version_checksum = 0;

		   Buf_Get_Firmware_Version[0] = 0xFF;
		   Buf_Get_Firmware_Version[1] = 0x00;
		   Buf_Get_Firmware_Version[2] = 0x01;
		   Buf_Get_Firmware_Version[3] = CMD_GET_FIRMWARE_VERSION;

		   for(int i = 1 ; i < sizeof(Buf_Get_Firmware_Version) - 1 ; i++)
		   {
		  	  get_firmware_version_checksum = Buf_Get_Firmware_Version[i] + get_firmware_version_checksum;
		   }

		   Buf_Get_Firmware_Version[4] = get_firmware_version_checksum;
		   UART_driver_send_data(Buf_Get_Firmware_Version , sizeof(Buf_Get_Firmware_Version));
		   get_firmware_version_checksum = 0;

        break;
	  }

	  case CMD_ANTENNA_POWER:
	  {
		  //CmdAntennaPower
		  uint8_t Buf_Antenna_Power[6];
		  uint8_t antenna_power_checksum = 0;

		  Buf_Antenna_Power[0] = 0xFF;
		  Buf_Antenna_Power[1] = 0x00;
		  Buf_Antenna_Power[2] = 0x02;
		  Buf_Antenna_Power[3] = CMD_ANTENNA_POWER;
		  Buf_Antenna_Power[4] = antenna_power_status;

		  for(int i = 1 ; i < sizeof(Buf_Antenna_Power) - 1 ; i++)
		  {
			  antenna_power_checksum = Buf_Antenna_Power[i] + antenna_power_checksum;
		  }

		  Buf_Antenna_Power[5] = antenna_power_checksum;
		  UART_driver_send_data(Buf_Antenna_Power, sizeof(Buf_Antenna_Power));
		  antenna_power_checksum = 0;

		 break;
 	  }

	  case CMD_READ_INPUT:
	  {
		  //ReadInput
		  uint8_t Buf_Read_Input[5];
		  uint8_t read_input_checksum = 0;

		  Buf_Read_Input[0] = 0xFF;
		  Buf_Read_Input[1] = 0x00;
		  Buf_Read_Input[2] = 0x01;
		  Buf_Read_Input[3] = CMD_READ_INPUT;

		  for(int i = 1 ; i < sizeof(Buf_Read_Input) - 1 ; i++)
		  {
			  read_input_checksum = Buf_Read_Input[i] + read_input_checksum;
		  }

		  Buf_Read_Input[4] = read_input_checksum;
		  UART_driver_send_data(Buf_Read_Input, sizeof(Buf_Read_Input));
		  read_input_checksum = 0;

		 break;
	  }

	  case CMD_WRITE_OUTPUT:
	  {
		  //CmdWriteOutput
		  uint8_t Buf_Write_Output[6];
		  uint8_t write_output_checksum = 0;

		  Buf_Write_Output[0] = 0xFF;
		  Buf_Write_Output[1] = 0x00;
	      Buf_Write_Output[2] = 0x02;
	      Buf_Write_Output[3] = CMD_WRITE_OUTPUT;
	      Buf_Write_Output[4] = write_output_status;

	      for(int i = 1 ; i < sizeof(Buf_Write_Output) - 1 ; i++)
	      {
	    	  write_output_checksum = Buf_Write_Output[i] + write_output_checksum;
	      }

	      Buf_Write_Output[5] = write_output_checksum;
	      UART_driver_send_data(Buf_Write_Output, sizeof(Buf_Write_Output));
	      write_output_checksum = 0;

	      break;
	  }

	  case CMD_SET_BAUD_RATE:
	  {
		  //CmdSetBaudRate
		  uint8_t Buf_Set_BaudRate[6];
		  uint8_t set_baudrate_checksum = 0;

		  Buf_Set_BaudRate[0] = 0xFF;
		  Buf_Set_BaudRate[1] = 0x00;
		  Buf_Set_BaudRate[2] = 0x02;
		  Buf_Set_BaudRate[3] = CMD_SET_BAUD_RATE;
		  Buf_Set_BaudRate[4] = set_baudrate_status;

		  for(int i = 1 ; i < sizeof(Buf_Set_BaudRate) - 1 ; i++)
		  {
			  set_baudrate_checksum = Buf_Set_BaudRate[i] + set_baudrate_checksum;
		  }

		  Buf_Set_BaudRate[5] = set_baudrate_checksum;
		  UART_driver_send_data(Buf_Set_BaudRate, sizeof(Buf_Set_BaudRate));
		  set_baudrate_checksum = 0;

		break;
	  }

	  case CMD_GET_I2C_ADDRESS:
	  {
		  //CmdGetI2CAddress
		  uint8_t Buf_Get_I2C_Address[5];
		  uint8_t get_i2c_address_checksum = 0;

		  Buf_Get_I2C_Address[0] = 0xFF;
		  Buf_Get_I2C_Address[1] = 0x00;
		  Buf_Get_I2C_Address[2] = 0x01;
		  Buf_Get_I2C_Address[3] = CMD_GET_I2C_ADDRESS;

		  for(int i = 1 ; i < sizeof(Buf_Get_I2C_Address) - 1 ; i++)
		  {
			  get_i2c_address_checksum = Buf_Get_I2C_Address[i] + get_i2c_address_checksum;
		  }

		  Buf_Get_I2C_Address[4] = get_i2c_address_checksum;
		  UART_driver_send_data(Buf_Get_I2C_Address, sizeof(Buf_Get_I2C_Address));
		  get_i2c_address_checksum = 0;

		  break;

	   }

	  case CMD_POLL_BUFFER:
	  {
		  //CmdPollBuffer(RS485)
		  uint8_t Buf_Poll_Buffer[5];
		  uint8_t poll_buffer_checksum = 0;

		  Buf_Poll_Buffer[0] = 0xFF;
		  Buf_Poll_Buffer[1] = 0x00;
		  Buf_Poll_Buffer[2] = 0x01;
		  Buf_Poll_Buffer[3] = CMD_POLL_BUFFER;

		  for(int i = 1 ; i < sizeof(Buf_Poll_Buffer) - 1 ; i++)
		  {
			  poll_buffer_checksum = Buf_Poll_Buffer[i] + poll_buffer_checksum;
		  }

		  Buf_Poll_Buffer[4] = poll_buffer_checksum;
		  UART_driver_send_data(Buf_Poll_Buffer, sizeof(Buf_Poll_Buffer));
		  poll_buffer_checksum = 0;

		  break;
	  }

	  case CMD_GET_APP_CONFIG:
	  {
		  //CmdGetAppConfig
		  uint8_t Buf_Get_App_Config[6];
		  uint8_t get_app_config_checksum = 0;

		  Buf_Get_App_Config[0] = 0xFF;
		  Buf_Get_App_Config[1] = 0x00;
		  Buf_Get_App_Config[2] = 0x02;
		  Buf_Get_App_Config[3] = CMD_GET_APP_CONFIG;
		  Buf_Get_App_Config[4] = 0x01;

		  for(int i = 1 ; i < sizeof(Buf_Get_App_Config) - 1 ; i++)
		  {
			  get_app_config_checksum = Buf_Get_App_Config[i] + get_app_config_checksum;
		  }

		  Buf_Get_App_Config[5] = get_app_config_checksum;
		  UART_driver_send_data(Buf_Get_App_Config, sizeof(Buf_Get_App_Config));
		  get_app_config_checksum = 0;

		break;
	  }

	}
}



void func_reset()
{
 	for(int j = 0 ; j < RxFrame.DataLength - 1 ; j++)
 	{
 	    printf("%c\n", RxFrame.RxDataBuffer[j]);
 	}
}

void func_get_firmware_version()
{

	printf("%\n", RxFrame.RxDataBuffer[0]);
	 for(int j = 0 ; j < RxFrame.DataLength - 1 ; j++)
	 {
	  	printf("%c\n", RxFrame.RxDataBuffer[j]);

	 }
}

void func_antenna_power()
{
	 if(RxFrame.RxDataBuffer[0] == 0x00)
	 {
		 printf("%s\n", "RF field is switched Off..");
	 }

	 if(RxFrame.RxDataBuffer[0] == 0x01)
	 {
		 printf("%s\n" , "RF field is switched on.");
	 }

	 if(RxFrame.RxDataBuffer[0] == 0x02)
	 {
		 printf("%s\n" ,"RF field was reset.");
	 }
}

void func_read_input ()
{
	/*for (int i = 0 ; i < RxFrame.DataLength ; i++)
	{
		printf("%x\n" , RxFrame.RxDataBuffer[i]);
	}*/
	if(RxFrame.RxDataBuffer[0] & 1)
	{
		printf("%s\n" , "IN1 = High");
	}
	else
	{
		printf("%s\n" , "IN1 = Low");
	}
	if(RxFrame.RxDataBuffer[0] & 2)
	{
		printf("%s\n" , "IN2 = High");
	}
	else
    {
		printf("%s\n" , "IN2 = Low");
	}

}

void func_write_output ()
{
	if(RxFrame.RxDataBuffer[0] & 1)
	{
		printf("%s\n","Output 1: High");
	}
	else
	{
		printf("%s\n", "Output 1: Low");
	}
	if(RxFrame.RxDataBuffer[0] & 2)
	{
		printf("%s\n" ,"Output 2: High");
	}
	else
	{
		printf("%s\n","Output 2: Low");
	}
}

void func_set_baudrate()
{
	if(RxFrame.RxDataBuffer[0] == 0x4C)
	{
		printf("%s\n" , "0x4C (‘L’):Operation is successful. Baud rate is changed.");
	}

	if(RxFrame.RxDataBuffer[0] == 0x4E)
	{
	   printf("%s\n" , "0x4E (‘N’):Operation is failed.(Response returns with the old baud rate)");
	}

	if(Buf_Set_BaudRate[4] == 0x00)
	{
	  printf("%s\n" , "Set new baud rate to 9600bps");
	}

	if(Buf_Set_BaudRate[4] == 0x01)
	{
	  printf("%s\n" , "Set new baud rate to 19200bps");
	}

	if(Buf_Set_BaudRate[4] == 0x02)
	{
	  printf("%s\n" , "Set new baud rate to 38400bps");
	}

	if(Buf_Set_BaudRate[4] == 0x03)
	{
	  printf("%s\n" , "Set new baud rate to 57600bps");
	}

	if(Buf_Set_BaudRate[4] == 0x04)
	{
	  printf("%s\n" , "Set new baud rate to 115200bps");
	}

}

void func_get_I2C_address()
{
	printf("%s %x\n", "I2C Slave Addres of the reader is", RxFrame.RxDataBuffer[0]);
}


void func_poll_buffer()
{
	if(RxFrame.RxDataBuffer[0] == 0x4E)
	{
		printf("%s\n", "Status Code: 0x4E(‘N’)No Card UID found in polling buffer");
	}

	else if ((RxFrame.DataLength == 6) || (RxFrame.DataLength == 9))
	{
		int uidlength = RxFrame.DataLength - 2 ;
		printf("%x %s\n" , uidlength , "-byte Card UID found in the polling buffer");
		printf("%s\n" , ", UID is");

		for(int i = 0 ; i < uidlength ; i++)
		{
			printf("%x\n" , RxFrame.RxDataBuffer[uidlength - i]);
		}
		printf("%s %x\n", "Tag_Type is ", RxFrame.RxDataBuffer[0]);

	}
}

void func_get_app_config()
{
	if(RxFrame.RxDataBuffer[0] == 0x02)
	{
		printf("%s\n", "Id = 2");
	}

	if(RxFrame.RxDataBuffer[1] & 1)
	{
		printf("%s\n" , "Auto Mode: Enabled");
	}
	else
	{
		printf("%s\n", "Auto Mode: Disabled");
	}

	if(RxFrame.RxDataBuffer[1] & 2)
	{
		printf("%s\n", "SendFirmwareVersionOnStartup: True");
	}
	else
	{
		printf("%s\n", "SendFirmwareVersionOnStartup: False");
	}

	if(RxFrame.RxDataBuffer[1] & 4)
	{
		printf("%s\n", "SeekForTagOnStartUp: True");
	}
	else
	{
		printf("%s\n" ,"SeekForTagOnStartUp: False");
	}

	if(RxFrame.RxDataBuffer[1] & 8)
	{
		printf("%s\n","RS485: Enabled");
	}
	else
	{
		printf("%s\n", "RS485: Disabled");
	}

	if(RxFrame.RxDataBuffer[1] & 0b00010000)
	{
		printf("%s\n", "RS485 Mode: Only Master");
	}
	else
	{
		printf("%s\n", "RS485 Mode: Bidirectional");
	}

	if(RxFrame.RxDataBuffer[1] & 0b00100000)
	{
		printf("%s\n", "I2C: Enabled");
	}
	else
	{
		printf("%s\n", "I2C: Disabled");
	}

	if(RxFrame.RxDataBuffer[1] & 0b01000000)
	{
		printf("%s\n", "Beep OnStartup: Enabled");
	}
	else
	{
		printf("%s\n", "Beep OnStartup: Disabled");
	}

	if(RxFrame.RxDataBuffer[1] & 0b10000000)
	{
		printf("%s\n", "Beep OnSeekForTagFound: Enabled");
	}
	else
	{
		printf("%s\n", "Beep OnSeekForTagFound: Disabled");
	}

	if(RxFrame.RxDataBuffer[2] == 0x01)
	{
		printf("%s %x\n", "Serial Node Adress: ", RxFrame.RxDataBuffer[2]);
	}

	if(RxFrame.RxDataBuffer[3] & 1)
	{
		printf("%s\n" ,"Auto Mode Output: ASCII");
	}
	else
	{
		printf("%s\n", "Auto Mode Output: Protocol");
	}

	if(RxFrame.RxDataBuffer[3] & 2)
	{
		printf("%s\n", "ASCII Reverse Card UID: True");
	}
	else
	{
		printf("%s\n" ,"ASCII Reverse Card UID: False");
	}

	if(RxFrame.RxDataBuffer[3] & 4)
	{
		printf("%s\n" ,"AutoBeepOnTagFound: True");
	}
	else
	{
		printf("%s\n" ,"AutoBeepOnTagFound: False");
	}

	if(RxFrame.RxDataBuffer[3] & 8)
	{
		printf("%s\n" ,"ASCII CR: True");
	}
	else
	{
		printf("%s\n" ,"ASCII CR: False");
	}

	if(RxFrame.RxDataBuffer[3] & 0b00010000)
	{
		printf("%s\n" ,"ASCII LF: True");
	}
	else
	{
		printf("%s\n" ,"ASCII LF: False");
	}

	if(RxFrame.RxDataBuffer[4] == 0x00)
	{
		printf("%s %x %s\n", "Baudrate" , RxFrame.RxDataBuffer[4], " (9600bps)");
	}
	if(RxFrame.RxDataBuffer[4] == 0x01)
	{
		printf("%s %x %s\n", "Baudrate" , RxFrame.RxDataBuffer[4], " (19200bps)");
	}
	if(RxFrame.RxDataBuffer[4] == 0x02)
	{
		printf("%s %x %s\n", "Baudrate" , RxFrame.RxDataBuffer[4], " (38400bps)");
	}
	if(RxFrame.RxDataBuffer[4] == 0x03)
	{
		printf("%s %x %s\n", "Baudrate" , RxFrame.RxDataBuffer[4], " (57600bps)");
	}
	if(RxFrame.RxDataBuffer[4] == 0x04)
	{
		printf("%s %x %s\n", "Baudrate" , RxFrame.RxDataBuffer[4], " (115200bps)");
	}
	if(RxFrame.RxDataBuffer[5] == 0x42)
	{
		printf("%s %x\n", "I2C Adress: ", RxFrame.RxDataBuffer[5]);
	}
	if(RxFrame.RxDataBuffer[6] == 0x00)
	{
		printf("%s %x\n", "UserData1: ", RxFrame.RxDataBuffer[6]);
	}
	if(RxFrame.RxDataBuffer[7] == 0x00)
    {
			printf("%s %x\n", "UserData2: ", RxFrame.RxDataBuffer[7]);
	}

	printf("%s\n", "ASCII Header(prefix) chars: ");
	for(int i = 8 ; i < 11 ; i++)
	{
		printf("%x\n", RxFrame.RxDataBuffer[i]);
	}
	printf("%s\n", "ASCII Footer(suffix) chars: ");
	for(int k = 11 ; k < 14 ; k++)
	{
	   printf("%x\n", RxFrame.RxDataBuffer[k]);
	}

	if(RxFrame.RxDataBuffer[14] & 1)
	{
		printf("%s\n","Pinout Connection: TypeB");
	}
	else
	{
		printf("%s\n","Pinout Connection: TypeA");
	}
	if(RxFrame.RxDataBuffer[14] & 2)
	{
		printf("%s\n" , "Buzzer Type: PWM Buzzer");
	}
	else
	{
		printf("%s\n" , "Buzzer Type: DC Buzzer");
	}
	if((RxFrame.RxDataBuffer[14] & 0b00001100) == (0b00000000))
	{
		printf("%s\n" , "Visual Effect: EFFECT_STD_AUTO_FLASHING");
	}
	if((RxFrame.RxDataBuffer[14] & 0b00001100) == (0b00000100))
    {
		printf("%s\n" , "Visual Effect: SREAD No Flashing");
	}
	if((RxFrame.RxDataBuffer[14] & 0b00001100) == (0b00001000))
	{
	    printf("%s\n" , "Visual Effect: TAGF Single Pulse/SREAD Flashing");
	}
	if((RxFrame.RxDataBuffer[14] & 0b00001100) == (0b00001100))
	{
		printf("%s\n" , "No Effect");
	}
	if(RxFrame.RxDataBuffer[14] & 0b00010000)
	{
		printf("%s\n" , "BeepOnActivateAll: True");
	}
	else
	{
		printf("%s\n" , "BeepOnActivateAll: False");
	}

	printf("%s\n", "Firmware Version: ");
	for( int j = 15 ; j < RxFrame.DataLength ; j++)
	{
		printf("%c\n" , RxFrame.RxDataBuffer[j]);
	}
}




 void spv1_uart_command_parser(uint8_t Command)
 {
	switch(Command)
	{
	   case CMD_RESET:
	   func_reset();
	   break;

	   case CMD_GET_FIRMWARE_VERSION:
	   func_get_firmware_version();
	   break;

	   case CMD_ANTENNA_POWER:
	   func_antenna_power();
	   break;

	   case CMD_READ_INPUT:
	   func_read_input();
	   break;

	   case CMD_WRITE_OUTPUT:
	   func_write_output();
	   break;

	   case CMD_GET_I2C_ADDRESS:
	   func_get_I2C_address();
	   break;

	   case CMD_POLL_BUFFER:
	   func_poll_buffer();
	   break;

	   case CMD_GET_APP_CONFIG:
	   func_get_app_config();
	   break;

	   default :
	   {
		  //UnknownCommand();
		  break;
	   }
	 }
 }




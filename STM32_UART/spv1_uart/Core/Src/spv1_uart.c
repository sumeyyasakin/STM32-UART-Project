/*
 * spv1_uart.c
 *
 *  Created on: Oct 4, 2022
 *      Author: Sümeyye
 */

#include "spv1_uart.h"
//#include "uart_driver.h"

volatile tsSerial Serial;
volatile tsSPV1RxFrame RxFrame;


uint8_t SerialMaxDataLength ;
uint8_t rx_data[MAX_DATA_COUNT];

uint8_t SerialCommand = 0x93;

uint8_t authenticate_block_no = 0x04;

uint8_t read_block_no = 0x04;

uint8_t write_block_no = 0x04;
uint8_t Write_Block [16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

uint8_t write_block_4_byte_no = 0x04;
uint8_t Write_Block_4_Byte[4] = {0x10, 0x11, 0x12, 0x13};

uint8_t read_value_block_no = 0x04;

uint8_t write_value_block_no = 0x04;
uint8_t Write_Value_Block [4] = {0x0C, 0x00, 0x00, 0x00};


uint8_t Increment_Value_Block[4] = {0x02, 0x00, 0x00, 0x00};
uint8_t increment_value_block_no = 0x04;

uint8_t  Decrement_Value_Block[4] = {0x02, 0x00, 0x00, 0x00};
uint8_t  decrement_value_block_no = 0x04;


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
                   uart_command_parser(RxFrame.CommandByte);

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
	  case CMD_ACTIVATE_ALL:
	  {
		//Cmd Activate All
		uint8_t Buf_Activate_All[5];
		uint8_t activate_all_checksum = 0;

	    Buf_Activate_All[0] = 0xFF;
	    Buf_Activate_All[1] = 0x00;
	    Buf_Activate_All[2] = 0x01;
	    Buf_Activate_All[3]	= CMD_ACTIVATE_ALL;

	    for(int i = 1 ; i < sizeof(Buf_Activate_All) - 1 ; i++)
	  	{
	    	activate_all_checksum = Buf_Activate_All[i] + activate_all_checksum;
	  	}
	    Buf_Activate_All[4] = activate_all_checksum;

        UART_driver_send_data(Buf_Activate_All, sizeof(Buf_Activate_All));
		activate_all_checksum = 0;

	    break;
	  }

      case CMD_SEEK_FOR_TAG:
	  {
		  //Cmd Seek For Tag
		  uint8_t Buf_Seek_For_Tag[5];
		  uint8_t seek_for_tag_checksum = 0;

		  Buf_Seek_For_Tag[0] = 0xFF;
		  Buf_Seek_For_Tag[1] = 0x00;
		  Buf_Seek_For_Tag[2] = 0x01;
		  Buf_Seek_For_Tag[3] = CMD_SEEK_FOR_TAG;

		  for(int i = 1 ; i < sizeof(Buf_Seek_For_Tag) - 1 ; i++)
		  {
			  seek_for_tag_checksum = Buf_Seek_For_Tag[i] + seek_for_tag_checksum;
		  }

		  Buf_Seek_For_Tag[4] = seek_for_tag_checksum;
		  UART_driver_send_data(Buf_Seek_For_Tag, sizeof(Buf_Seek_For_Tag));
		  seek_for_tag_checksum = 0 ;

		 break;
	  }

	  case CMD_ACTIVATE_IDLE:
	  {
		  //Cmd Activate Idle
		  uint8_t Buf_Activate_Idle[5];
		  uint8_t activate_idle_checksum = 0;

		  Buf_Activate_Idle[0] = 0xFF;
		  Buf_Activate_Idle[1] = 0x00;
		  Buf_Activate_Idle[2] = 0x01;
		  Buf_Activate_Idle[3] = CMD_ACTIVATE_IDLE;

		  for(int i = 1 ; i < sizeof(Buf_Activate_Idle)- 1 ; i++)
		  {
		     activate_idle_checksum = Buf_Activate_Idle[i] + activate_idle_checksum ;
		  }

		  Buf_Activate_Idle[4] = activate_idle_checksum;
		  UART_driver_send_data(Buf_Activate_Idle, sizeof(Buf_Activate_Idle));
		  activate_idle_checksum = 0 ;

		break;
	  }
	  case CMD_HALT:
	  {
		  //Cmd Halt Command
		  uint8_t Buf_Halt[5];
		  uint8_t halt_checksum = 0;

		  Buf_Halt[0] = 0xFF;
		  Buf_Halt[1] = 0x00;
		  Buf_Halt[2] = 0x01;
		  Buf_Halt[3] = CMD_HALT;

		  for(int i = 1 ; i < sizeof(Buf_Halt) - 1 ; i++)
		  {
		  	  halt_checksum = Buf_Halt[i] + halt_checksum;
		  }

          Buf_Halt[4] = halt_checksum;
		  UART_driver_send_data(Buf_Halt, sizeof(Buf_Halt));
		  halt_checksum = 0 ;

		break;
	  }
	  case CMD_AUTHENTICATE:
	  {
		  //Cmd Authenticate
		  uint8_t Buf_Authenticate[7];
		  uint8_t authenticate_checksum = 0;

		  Serial.txFlag = 0;
		  Tx_Frame(CMD_ACTIVATE_ALL);

		  while(!Serial.txFlag)
		  {
		  }
		  //AUTHENTICATE for Block4,5,6,7
		  Buf_Authenticate[0] = 0xFF;
		  Buf_Authenticate[1] = 0x00;
		  Buf_Authenticate[2] = 0x03;
		  Buf_Authenticate[3] = CMD_AUTHENTICATE;
		  Buf_Authenticate[4] = authenticate_block_no;
		  Buf_Authenticate[5] = 0xFF;

		  for(int i = 1 ; i < sizeof(Buf_Authenticate) - 1 ; i++)
		  {
			  authenticate_checksum = Buf_Authenticate[i] + authenticate_checksum;
		  }

		  Buf_Authenticate[6] = authenticate_checksum;
		  UART_driver_send_data(Buf_Authenticate, sizeof(Buf_Authenticate));

		  Serial.txFlag = 0;
		  authenticate_checksum = 0;
       break;
	  }

	  case CMD_READ_BLOCK:
	  {
		  //Cmd Read Block Command
		  uint8_t Buf_Read_Block[6];
		  uint8_t read_block_checksum = 0;

		  Tx_Frame(CMD_AUTHENTICATE);

		  while (!Serial.txFlag)
		  {
		  }

		  Buf_Read_Block[0] = 0xFF;
		  Buf_Read_Block[1] = 0x00;
		  Buf_Read_Block[2] = 0x02;
		  Buf_Read_Block[3] = CMD_READ_BLOCK;
		  Buf_Read_Block[4] = read_block_no;

		  for(int i = 1 ; i < sizeof(Buf_Read_Block) - 1 ; i++)
		  {
			read_block_checksum = Buf_Read_Block[i] + read_block_checksum;
		  }

		  Buf_Read_Block[5] = read_block_checksum;
          UART_driver_send_data(Buf_Read_Block, sizeof(Buf_Read_Block));
          Serial.txFlag = 0;
          read_block_checksum = 0;

		 break;
	  }

	  case CMD_WRITE_BLOCK:
	  {
		  //Cmd Write Block Command
		  uint8_t Buf_Write_Block[22];
		  uint8_t write_block_checksum = 0;

		  Tx_Frame(CMD_AUTHENTICATE);

		  while(!Serial.txFlag)
		  {
		  }

		  Buf_Write_Block[0] = 0xFF;
	      Buf_Write_Block[1] = 0x00;
	      Buf_Write_Block[2] = 0x12;
	      Buf_Write_Block[3] = CMD_WRITE_BLOCK;
	      Buf_Write_Block[4] = write_block_no;

	      memcpy(&Buf_Write_Block[5], &Write_Block ,sizeof(Write_Block));

	      for(int i = 1 ; i < sizeof(Buf_Write_Block) - 1 ; i++)
	      {
	        write_block_checksum = Buf_Write_Block[i] + write_block_checksum;
	      }

	      Buf_Write_Block[21] = write_block_checksum;
	      UART_driver_send_data(Buf_Write_Block, sizeof(Buf_Write_Block));
	      Serial.txFlag = 0;
	      write_block_checksum = 0;

	    break;
	  }

	  case CMD_WRITE_BLOCK_4_BYTE:
	  {
		 //Cmd Write Block 4 Byte
		 uint8_t Buf_Write_Block_4_Byte[10];
		 uint8_t write_block_4_byte_checksum = 0;

		 Tx_Frame(CMD_AUTHENTICATE);

		 while(!Serial.txFlag)
		 {
		 }
		 Buf_Write_Block_4_Byte[0] = 0xFF;
		 Buf_Write_Block_4_Byte[1] = 0x00;
		 Buf_Write_Block_4_Byte[2] = 0x06;
		 Buf_Write_Block_4_Byte[3] = CMD_WRITE_BLOCK_4_BYTE;
		 Buf_Write_Block_4_Byte[4] = write_block_4_byte_no;

		 memcpy(&Buf_Write_Block_4_Byte[5], &Write_Block_4_Byte ,sizeof(Write_Block_4_Byte));

		 for(int i = 1; i < sizeof(Buf_Write_Block_4_Byte) - 1; i++ )
		 {
			write_block_4_byte_checksum = Buf_Write_Block_4_Byte[i] + write_block_4_byte_checksum;
		 }
		 Buf_Write_Block_4_Byte[9] = write_block_4_byte_checksum;
		 UART_driver_send_data(Buf_Write_Block_4_Byte, sizeof(Buf_Write_Block_4_Byte));

		 Serial.txFlag = 0;
		 write_block_4_byte_checksum = 0 ;

		break;
	  }

	  case CMD_WRITE_VALUE_BLOCK:
	  {
		  //Cmd Write Value Block Command
		  uint8_t Buf_Write_Value_Block[10];
		  uint8_t write_value_block_checksum = 0;

		  Tx_Frame(CMD_AUTHENTICATE);

		  while(!Serial.txFlag)
		  {
		  }

		  Buf_Write_Value_Block[0] = 0xFF;
		  Buf_Write_Value_Block[1] = 0x00;
		  Buf_Write_Value_Block[2] = 0x06;
		  Buf_Write_Value_Block[3] = CMD_WRITE_VALUE_BLOCK;
		  Buf_Write_Value_Block[4] = write_value_block_no;

		  memcpy(&Buf_Write_Value_Block[5], &Write_Value_Block ,sizeof(Write_Value_Block));

		  for(int i = 1 ; i < sizeof(Buf_Write_Value_Block) - 1 ; i++)
		  {
			write_value_block_checksum = Buf_Write_Value_Block[i] + write_value_block_checksum;
		  }

		  Buf_Write_Value_Block[9] = write_value_block_checksum;
		  UART_driver_send_data(Buf_Write_Value_Block, sizeof(Buf_Write_Value_Block));

		  Serial.txFlag = 0;
		  write_value_block_checksum = 0;

		break;
	  }

	  case CMD_READ_VALUE_BLOCK:
	  {
		  //Cmd Read Value Block Command
		  uint8_t Buf_Read_Value_Block[6];
		  uint8_t read_value_block_checksum = 0;

		  Tx_Frame(CMD_WRITE_VALUE_BLOCK);

          while(!Serial.txFlag)
          {
          }
		  Buf_Read_Value_Block[0] = 0xFF;
		  Buf_Read_Value_Block[1] = 0x00;
		  Buf_Read_Value_Block[2] = 0x02;
		  Buf_Read_Value_Block[3] = CMD_READ_VALUE_BLOCK;
		  Buf_Read_Value_Block[4] = read_value_block_no;

		  for(int i = 1 ; i < sizeof(Buf_Read_Value_Block) - 1 ; i++)
		  {
		    read_value_block_checksum = Buf_Read_Value_Block[i] + read_value_block_checksum;
		  }

		  Buf_Read_Value_Block[5] = read_value_block_checksum;
		  UART_driver_send_data(Buf_Read_Value_Block, sizeof(Buf_Read_Value_Block));

		  Serial.txFlag = 0;
		  read_value_block_checksum = 0;

	   break;
     }

	 case CMD_INCREMENT_VALUE_BLOCK:
	 {
		 //Cmd Increment Value Block Command
		 uint8_t Buf_Increment_Value_Block [10];
		 uint8_t increment_value_block_checksum = 0;

		 Tx_Frame(CMD_AUTHENTICATE);

		 while(!Serial.txFlag)
		 {
		 }

		 Buf_Increment_Value_Block[0] = 0xFF;
		 Buf_Increment_Value_Block[1] = 0x00;
		 Buf_Increment_Value_Block[2] = 0x06;
		 Buf_Increment_Value_Block[3] = CMD_INCREMENT_VALUE_BLOCK;
		 Buf_Increment_Value_Block[4] = increment_value_block_no;

		 memcpy(&Buf_Increment_Value_Block[5], &Increment_Value_Block ,sizeof(Increment_Value_Block));

		 for(int i = 1 ; i < sizeof(Buf_Increment_Value_Block) - 1 ; i++)
		 {
			 increment_value_block_checksum = Buf_Increment_Value_Block[i] + increment_value_block_checksum;
		 }

		 Buf_Increment_Value_Block[9] = increment_value_block_checksum;

		 UART_driver_send_data(Buf_Increment_Value_Block, sizeof(Buf_Increment_Value_Block));
		 Serial.txFlag = 0;
		 increment_value_block_checksum = 0;

		break;
	 }

	 case CMD_DECREMENT_VALUE_BLOCK:
	 {
		 //Cmd Decrement Value Block Command
		 uint8_t  Buf_Decrement_Value_Block[10];
		 uint8_t  decrement_value_block_checksum = 0;

		 Tx_Frame(CMD_AUTHENTICATE);

	     while(!Serial.txFlag)
	     {
	     }

	     Buf_Decrement_Value_Block[0] = 0xFF;
	     Buf_Decrement_Value_Block[1] = 0x00;
	     Buf_Decrement_Value_Block[2] = 0x06;
	     Buf_Decrement_Value_Block[3] = CMD_DECREMENT_VALUE_BLOCK;
	     Buf_Decrement_Value_Block[4] = decrement_value_block_no;

	     memcpy(&Buf_Decrement_Value_Block[5], &Decrement_Value_Block ,sizeof(Decrement_Value_Block));

	     for(int i = 1 ; i < sizeof(Buf_Decrement_Value_Block) - 1 ; i++)
	     {
	    	decrement_value_block_checksum = Buf_Decrement_Value_Block[i] + decrement_value_block_checksum;
	     }

	     Buf_Decrement_Value_Block[9] = decrement_value_block_checksum;
	     UART_driver_send_data(Buf_Decrement_Value_Block, sizeof(Buf_Decrement_Value_Block));

	     Serial.txFlag = 0;
	     decrement_value_block_checksum = 0;

	    break;

	 }
  }
}




void func_activate_all()
{

	  if (RxFrame.RxDataBuffer[0] == 0x4E)
	  {
		 printf("%s\n","No Tag");
		 //return 0;
		 Serial.txFlag = 1 ;
	  }

	  else if ((RxFrame.DataLength == 6) ||(RxFrame.DataLength ==9))
	  {
		printf(" %s %x\n", "Tag_Type: " ,RxFrame.RxDataBuffer[0]);
	    int uidlength = RxFrame.DataLength - 2;
		printf("%s %x\n", "UID Length:", uidlength);
		printf("%s\n","UID: ");

		for (int i = 0; i< uidlength; i++)
		{
	       printf("%x\n", RxFrame.RxDataBuffer[uidlength-i]);
		}

		//return 1;
		Serial.txFlag = 1 ;
	  }

}


void func_seek_for_tag ()
{

	if(RxFrame.RxDataBuffer[0] == 0x4C)
	{
	   printf("%s\n", "0x4C (‘L’) : Operation is successful. Card UID will be reported as soon as detected.");
	}

	if(RxFrame.RxDataBuffer[0] == 0x46)
	{
	   printf("%s\n", "0x46 (‘F’) : Operation failed. CmdSeekForTag cannot be used if Auto Mode is enabled");
	}

	if(RxFrame.RxDataBuffer[0] == 0x55)
	{
	   printf("%s\n", "0x55 (‘U’) : Operation failed. The Antenna power was switched off. CmdSeekForTag cannot run.");
	}

	else if ((RxFrame.DataLength == 6) ||(RxFrame.DataLength ==9))
	{
	   printf(" %s %x\n", "Tag_Type: " ,RxFrame.RxDataBuffer[0]);
	   int uidlength = RxFrame.DataLength - 2;
	   printf("%s %x\n", "UID Length:", uidlength);
	   printf("%s\n","UID: ");

	   for (int i = 0; i< uidlength; i++)
	   {
	     printf("%x\n", RxFrame.RxDataBuffer[uidlength-i]);
	   }
	}

}


void func_activate_idle()
{
   if (RxFrame.RxDataBuffer[0] == 0x4E)
   {
      printf("%s\n","No Tag");
   }

   if (RxFrame.RxDataBuffer[0] == 0x55)
   {
     printf("%s\n","Operation failed. The Antenna power was switched off.");
   }

   else if ((RxFrame.DataLength == 6) ||(RxFrame.DataLength == 9))
   {

     printf(" %s %x\n", "Tag_Type: " ,RxFrame.RxDataBuffer[0]);
     int uidlength = RxFrame.DataLength - 2;
     printf("%s %x\n", "UID Length:", uidlength);

     printf("%s\n","UID: ");

     for (int i = 0; i< uidlength; i++)
     {
       printf("%x\n", RxFrame.RxDataBuffer[uidlength-i]);
     }

   }

}

void func_halt()
{
     if(RxFrame.Checksum == 0xE1)
 	 {
 	 	printf("%s\n","Status Code: 0x4C(‘L’). Halt command is executed successfully.");
 	 }

 	 if(RxFrame.Checksum == 0xEA)
 	 {
 	    printf("%s\n","Status Code: 0x55(‘U’). Operation failed. The Antenna power was switched off.");
 	 }

}


void func_authenticate()
{

	printf("%x %x %x %x\n",RxFrame.HeaderByte,RxFrame.NodeAddress,RxFrame.DataLength,RxFrame.CommandByte);

    for(int k=0 ; k< RxFrame.DataLength-1; k++)
    {
      printf("%x\n",RxFrame.RxDataBuffer[k] );
    }

    printf("%x\n", RxFrame.Checksum);

    Serial.txFlag = 1;

}


void func_read_block ()
{

    printf("%s %x\n", "Mifare Card Block no: " ,RxFrame.RxDataBuffer[0]);
    printf("%x %x %x %x\n",RxFrame.HeaderByte,RxFrame.NodeAddress,RxFrame.DataLength, RxFrame.CommandByte);

    for(int i = 0; i< RxFrame.DataLength - 1; i++)
 	{
 	   printf("%x\n", RxFrame.RxDataBuffer[i]);
 	}

    printf("%x\n", RxFrame.Checksum);

 	if(RxFrame.RxDataBuffer[0] == 0x4E)
 	{
 	  printf("%s\n", "0x4E ('N'):No Tag or operation failed.");
 	}

 	if(RxFrame.RxDataBuffer[0] == 0x46)
 	{
 	  printf("%s\n", "0x46 ('F'): No Tag or operation failed.");
 	}

 	if(RxFrame.RxDataBuffer[0] == 0x55)
 	{
 	  printf("%s\n", "0x55 ('U'): Operation failed. The Antenna power was switched off.");
 	}

}

void func_write_block()
{

     printf("%s %x\n", "Mifare Card Block no: ", RxFrame.RxDataBuffer[0]);
     printf("%x %x %x %x\n",RxFrame.HeaderByte,RxFrame.NodeAddress,RxFrame.DataLength,RxFrame.CommandByte);

     for(int i=0 ; i < RxFrame.DataLength-1; i++)
     {
        printf("%x\n", RxFrame.RxDataBuffer[i]);
     }

     printf("%x\n", RxFrame.Checksum);

     if(RxFrame.RxDataBuffer[0] == 0x4E)
     {
        printf("%s\n", "0x4E ('N'):No Tag or operation failed.");
     }

     if(RxFrame.RxDataBuffer[0] == 0x46)
     {
        printf("%s\n", "0x46 ('F'): No Tag or operation failed.");
     }

     if(RxFrame.RxDataBuffer[0] == 0x55)
     {
        printf("%s\n", "0x55 ('U'): Read after write failed.");
     }

     if(RxFrame.RxDataBuffer[0] == 0x58)
     {
       printf("%s\n", "0x58 ('X'): Unable to read after write.");
     }

}

void func_write_block_4_byte()
{

	printf("%s %x\n", "Mifare Ultralight Page No: " , RxFrame.RxDataBuffer[0]);
	printf("%x %x %x %x\n",RxFrame.HeaderByte,RxFrame.NodeAddress, RxFrame.DataLength, RxFrame.CommandByte);

	for(int i = 0 ; i < RxFrame.DataLength - 1 ; i++)
	{
	   printf("%x\n", RxFrame.RxDataBuffer[i]);
	}

	printf("%x\n", RxFrame.Checksum);

	if(RxFrame.RxDataBuffer[0] == 0x4E)
    {
	   printf("%s\n", "0x4E ('N'):No Tag or operation failed.");
	}

	if(RxFrame.RxDataBuffer[0] == 0x46)
	{
	   printf("%s\n","0x46 ('F'): No Tag or operation failed.");
	}

	if(RxFrame.RxDataBuffer[0] == 0x55)
	{
	   printf("%s\n", "0x55 ('U'): Read after write failed ");
	}

	if(RxFrame.RxDataBuffer[0] == 0x58)
	{
	   printf("%s\n", "Unable to read after write");
	}

}


void func_read_value_block()
{

	printf("%s %d\n", "The value is ", RxFrame.RxDataBuffer[1]);
    printf("%s %x\n", "Mifare Card Bloc k no = ", RxFrame.RxDataBuffer[0]);

    printf("%x %x %x %x\n",RxFrame.HeaderByte,RxFrame.NodeAddress,RxFrame.DataLength,RxFrame.CommandByte);

    for(int i = 0; i< RxFrame.DataLength-1 ; i++)
	{
	   printf("%x\n", RxFrame.RxDataBuffer[i]);
	}

	printf("%x\n", RxFrame.Checksum);

	if(RxFrame.RxDataBuffer[0] == 0x4E)
	{
	   printf("%s\n" ,"0x4E (‘N’): No Tag or operation failed");
	}

	if(RxFrame.RxDataBuffer[0] == 0x46)
	{
	   printf("%s\n", "0x46 (‘F’): No Tag or operation failed");
	}

    if(RxFrame.RxDataBuffer[0] == 0x49)
	{
	   printf("%s\n","0x49 (‘I’): Invalid Value Block. The block was not in the proper value format.");
	}

}

void func_write_value_block ()
{

	printf("%s %x\n", "Mifare Card Block No: " ,RxFrame.RxDataBuffer[0]);
	printf("%x %x %x %x\n",RxFrame.HeaderByte, RxFrame.NodeAddress, RxFrame.DataLength, RxFrame.CommandByte);

    for(int i=0; i < RxFrame.DataLength-1 ;i++)
    {
	   printf("%x\n", RxFrame.RxDataBuffer[i]);
    }
	printf("%x\n", RxFrame.Checksum);

	if(RxFrame.RxDataBuffer[0] == 0x4E)
	{
	   printf("%s\n" ,"0x4E (‘N’): No Tag or operation failed");
	}

    if(RxFrame.RxDataBuffer[0] == 0x46)
	{
	   printf("%s\n", "0x46 (‘F’): No Tag or operation failed");
	}

	if(RxFrame.RxDataBuffer[0] == 0x55)
	{
	   printf("%s\n", "0x55 ('U'): Read after write failed ");
	}

	if(RxFrame.RxDataBuffer[0] == 0x58)
	{
	   printf("%s\n", "Unable to read after write ");
	}

	Serial.txFlag = 1;
}


void  func_increment_value_block()
{

	printf("%s %x\n", "Mifare Card Block no: " ,RxFrame.RxDataBuffer[0]);
    printf("%x %x %x %x\n",RxFrame.HeaderByte, RxFrame.NodeAddress,RxFrame.DataLength, RxFrame.CommandByte);

    for(int i = 0 ; i < RxFrame.DataLength - 1 ; i++)
    {
       printf("%x\n" , RxFrame.RxDataBuffer[i]);
    }

    printf("%x\n", RxFrame.Checksum);

   if(RxFrame.RxDataBuffer[0] == 0x4E)
   {
      printf("%s\n" ,"0x4E ('N'):No Tag or operation failed.");
   }

   if(RxFrame.RxDataBuffer[0] == 0x46)
   {
      printf("%s\n", "0x46 ('F'): No Tag or operation failed.");
   }

   if(RxFrame.RxDataBuffer[0] == 0x49)
   {
     printf("%s\n" ,"0x49 ('I'): Invalid Value Block. The block was not in the proper value format when read back.");
   }

}


void  func_decrement_value_block()
{

	printf("%s %x\n", "Mifare Card Block no: " , RxFrame.RxDataBuffer[0]);
	printf("%x %x %x %x\n", RxFrame.HeaderByte, RxFrame.NodeAddress, RxFrame.DataLength, RxFrame.CommandByte);

	for(int i = 0 ; i < RxFrame.DataLength -1 ; i++)
	{
	   printf("%x\n" , RxFrame.RxDataBuffer[i]);
	}

	printf("%x\n", RxFrame.Checksum);

	if(RxFrame.RxDataBuffer[0] == 0x4E)
	{
	   printf("%s\n", "0x4E ('N'): No Tag or operation failed.");
	}

	if(RxFrame.RxDataBuffer[0] == 0x46)
	{
	   printf("%s\n", "0x46 ('F'): No Tag or operation failed.");
	}

	if(RxFrame.RxDataBuffer[0] == 0x49)
	{
	  printf("%s\n", "0x49 ('I'): Invalid Value Block. The block was not in the proper value format when read back.");
	}

}


void uart_command_parser(uint8_t Command)
{
	switch (Command)
	{
	  case CMD_ACTIVATE_ALL:
	  func_activate_all();
	  break;

	  case CMD_SEEK_FOR_TAG:
	  func_seek_for_tag();
	  break;

	  case CMD_ACTIVATE_IDLE:
	  func_activate_idle();
	  break;

	  case CMD_HALT:
	  func_halt();
	  break;

	  case CMD_AUTHENTICATE:
	  func_authenticate();
	  break;

	  case CMD_READ_BLOCK:
	  func_read_block();
	  break;

	  case CMD_WRITE_BLOCK:
	  func_write_block();
	  break;

	  case CMD_WRITE_BLOCK_4_BYTE:
	  func_write_block_4_byte();
	  break;

	  case CMD_READ_VALUE_BLOCK:
	  func_read_value_block();
	  break;

	  case CMD_WRITE_VALUE_BLOCK:
	  func_write_value_block();
	  break;

	  case CMD_INCREMENT_VALUE_BLOCK:
	  func_increment_value_block();
	  break;

	  case CMD_DECREMENT_VALUE_BLOCK:
	  func_decrement_value_block();
	  break;


	  default :
	  {
		//UnknownCommand();
		break;
	  }
	}
}




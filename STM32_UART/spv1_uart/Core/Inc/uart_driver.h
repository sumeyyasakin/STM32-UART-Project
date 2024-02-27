/*
 * uart_driver.h
 *
 *  Created on: 11 Eki 2022
 *      Author: Sümeyye
 */

#ifndef INC_UART_DRIVER_H_
#define INC_UART_DRIVER_H_

#include "stm32l0xx_hal.h"


/**
  * @brief UART Initialization Function:
  * 	   Initializes UART with the following settings:
  * 	   Word Length = 8 Bits
      	   Stop Bit = One Stop bit
      	   Parity = None
           BaudRate = 19200 baud
           Hardware flow control disabled (RTS and CTS signals)
  * @param void
  * @retval void
  */

void UART_Init(void);

void UART_driver_send_data (uint8_t* Buffer , uint8_t Size);

#endif /* INC_UART_DRIVER_H_ */

#pragma once

#ifdef IRVINE_BOARD_V2_0
#define BOARD_MISO_PIN (15)
#define BOARD_MOSI_PIN (13)
#define BOARD_SCK_PIN (2)
#define BOARD_SD_CS_PIN (32)

#define BOARD_CAN_TX_PIN (33)
#define BOARD_CAN_RX_PIN (25)
#define BOARD_CAN_SE_PIN (4)

#define BOARD_MODEM_TXD_PIN (19)
#define BOARD_MODEM_RXD_PIN (18)
#define BOARD_MODEM_PWR_PIN (21)
#define BOARD_MODEM_STATUS_PIN (22)
#define BOARD_MODEM_RESET_PIN (16)
#define BOARD_MODEM_RI_PIN (17)
#define BOARD_MODEM_DTR_PIN (5)

#define CONSOLE_UART_BAUD 115200
#endif


#ifdef IRVINE_BOARD_V2_1
#define BOARD_MISO_PIN (15)
#define BOARD_MOSI_PIN (2)
#define BOARD_SCK_PIN (13)
#define BOARD_SD_CS_PIN (32)

#define BOARD_CAN_TX_PIN (33)
#define BOARD_CAN_RX_PIN (25)
#define BOARD_CAN_SE_PIN (4)

#define BOARD_BUZZER_PIN (26)

#define BOARD_ACCELERATOR_INT_PIN (36)

#define BOARD_VCC_ADC_PIN (35)

#define BOARD_MODEM_TXD_PIN (19)
#define BOARD_MODEM_RXD_PIN (18)
#define BOARD_MODEM_PWR_PIN (21)
#define BOARD_MODEM_STATUS_PIN (22)
#define BOARD_MODEM_RESET_PIN (16)
#define BOARD_MODEM_RI_PIN (17)
#define BOARD_MODEM_DTR_PIN (5)

#define CONSOLE_UART_BAUD 115200
#endif


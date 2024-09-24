#include "A7672x.h"
#include <string.h>

#define STR_W_LEN(x) x, sizeof(x)

A7672x::A7672x(uart_port_t uart, const int txPin, const int rxPin) : uartPort(uart)
{
    ESP_ERROR_CHECK(uart_set_pin(uart, txPin, rxPin, -1, -1));

    uartConfig.baud_rate = baudrate;
    uartConfig.data_bits = UART_DATA_8_BITS;
    uartConfig.parity = UART_PARITY_DISABLE;
    uartConfig.stop_bits = UART_STOP_BITS_1;
    uartConfig.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
}

bool A7672x::init()
{
    return ESP_OK == uart_param_config(uartPort, &uartConfig);
}

void A7672x::loop()
{
    uint8_t byte;
    if (uart_read_bytes(uartPort, &byte, 1, 0) > 0)
    {
        switch (parserState)
        {
        case A7672xParserState::COMMAND:
            if (byte != '\r')
            {
                if (commandBufferPos < sizeof(commandBuffer))
                {
                    commandBuffer[commandBufferPos++] = byte;
                }
            }
            else
            {
                commandBuffer[commandBufferPos] = '\0';
                parseCommand();
            }
            break;
        }
    }
}

bool A7672x::setBaudrate(const int baudrate)
{
    this->baudrate = baudrate;
}

void A7672x::parseCommand()
{
    if (!strcmp(commandBuffer, "OK"))
    {
        if (activeCommand.result)
            *activeCommand.result = CommandResults::OK;
        activeCommand = {};
    }
    else if (!strcmp(commandBuffer, "ERROR"))
    {
        if (activeCommand.result)
            *activeCommand.result = CommandResults::ERROR;
        activeCommand = {};
    }
    else if (commandBuffer[0] == '+')
    {
        /* Notification */
        char *command = &commandBuffer[1];
        if (!strncmp(command, STR_W_LEN("CPIN:")))
        {
            parseCPIN(&command[sizeof("CPIN:")] + 1u);
        }
        else if (!strncmp(command, STR_W_LEN("ICCID:")))
        {
            parseICCID(&command[sizeof("ICCID:")] + 1u);
        }
    }
}

void A7672x::parseCPIN(const char *str)
{
}
void A7672x::parseICCID(const char *str)
{
}

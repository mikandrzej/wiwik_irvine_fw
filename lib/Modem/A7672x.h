#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/uart.h>

const int uartRxBufSize = 6144;
const int uartTxBufSize = 1024;

class A7672x;

struct A7672xCommand
{
    A7672x::Commands command;
    volatile A7672x::CommandResults *result;
};

enum class A7672xParserState
{
    COMMAND,
    BINARY
};

class A7672x
{
public:
    A7672x(uart_port_t uart, const int txPin, const int rxPin);
    bool init();
    void loop();
    bool setBaudrate(const int baudrate);

    enum class Commands
    {
        NONE,
        AT,
        CGMM,
        CPIN,
        ATI,
        GSN,
        COCCID
    };

    enum class CommandResults
    {
        NONE,
        OK,
        ERROR,
        TIMEOUT
    };

private:
    char commandBuffer[128];
    int commandBufferPos = 0;
    uart_port_t uartPort;
    int baudrate = 115200;
    xQueueHandle uartQueue;
    uart_config_t uartConfig;
    A7672xParserState parserState;
    A7672xCommand activeCommand;

    void parseCommand();
    void parseCPIN(const char * str);
    void parseICCID(const char *str);
};
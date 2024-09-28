#include "ModemEngine.h"

ModemEngine::ModemEngine(HardwareSerial &serial) : serial(serial)
{
}

void ModemEngine::loop()
{
    while (serial.available())
    {
        char chr = serial.read();
        cmdRxBuf[cmdRxBufPos] = chr;

        if (cmdRxBufPos < cmdRxBuf.size() - 1)
            cmdRxBufPos++;

        if (chr == '\n')
        {
            parseReceivedLine();
        }
    }
}

void ModemEngine::registerCommand(AtCommand &command)
{
    registeredCommands.push_back(&command);
}

void ModemEngine::parseReceivedLine()
{
    // Check if the first character is '+', indicating an AT response
    if (cmdRxBuf[0] == '+')
    {
        // Iterate over all registered commands
        for (auto *command : registeredCommands)
        {
            const std::string &cmdStr = command->getCommand();
            size_t cmdLength = cmdStr.size();

            // Ensure the response is long enough to contain the command
            if (cmdRxBufPos > cmdLength + 1) // +1 for the '+' character
            {
                // Compare characters after '+' with the command
                if (std::equal(cmdStr.begin(), cmdStr.end(), cmdRxBuf.begin() + 1))
                {
                    // Pass the remaining response to parseResponse after the ": " part
                    std::vector<char> response(cmdRxBuf.begin(), cmdRxBuf.begin() + cmdRxBufPos);
                    command->parseResponse(response);
                    return; // Found the matching command, exit the function
                }
            }
        }
    }
}

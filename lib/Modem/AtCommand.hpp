#pragma once

#include <Arduino.h>
#include <string>

class AtCommand
{
protected:
    std::string command;
    uint32_t timeout;

public:
    AtCommand(const std::string &command, uint32_t timeout);
    const std::string &getCommand();
    virtual bool parseResponse(const std::vector<char> &response) = 0;
    virtual void parseData(const std::vector<char> &data) = 0;
    virtual const std::string getQuery() = 0;
    virtual const std::string getQueryCommand() = 0;
    virtual const std::string getExecuteCommand() = 0;
    virtual const std::string getSetCommand() = 0;
    uint32_t getTimeout();
    ~AtCommand();
};

AtCommand::AtCommand(const std::string &command, uint32_t timeout) : command(command), timeout(timeout)
{
}

uint32_t AtCommand::getTimeout()
{
    return timeout;
}

AtCommand::~AtCommand()
{
}

const std::string &AtCommand::getCommand()
{
    return command;
}

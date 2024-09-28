#pragma once

#include <Arduino.h>
#include "AtCommand.hpp"
#include <vector>
#include <algorithm> // For std::equal

class AtStandardCommand : public AtCommand
{
public:
    AtStandardCommand(const std::string &command, uint32_t timeout);
    bool parseResponse(const std::vector<char> &response) override;
    const std::string getQueryCommand() override;
    const std::string getExecuteCommand() override;
    const std::string getSetCommand() override;
    virtual const std::vector<char> getSetParams() = 0;
    virtual void parseData(std::vector<char> &data) {};
    virtual void parseResponseData(std::vector<char> &data) {};
};

inline AtStandardCommand::AtStandardCommand(const std::string &command, uint32_t timeout) : AtCommand(command, timeout)
{
}

bool AtStandardCommand::parseResponse(const std::vector<char> &response)
{
    // Check if the response is long enough to contain the expected command and ': '
    size_t commandSize = command.size();
    size_t expectedPrefixSize = 1 + commandSize + 2; // "+<command>: "

    if (response.size() < expectedPrefixSize)
    {
        return false;
    }

    // Check if response starts with "+<command>: "
    const std::string expectedPrefix = "+" + std::string(command.begin(), command.end()) + ": ";
    if (!std::equal(expectedPrefix.begin(), expectedPrefix.end(), response.begin()))
    {
        return false;
    }

    // Extract the data part after "+<command>: "
    std::vector<char> data(response.begin() + expectedPrefixSize, response.end());

    // Convert the data to the format needed by parseResponseData
    // Assuming parseResponseData can work with std::vector<char> or modify as needed
    parseResponseData(data);

    return true;
}

const std::string AtStandardCommand::getQueryCommand()
{
    std::string resp;
    resp.reserve(3 + command.size() + 1);

    resp.append("AT+");
    resp.append(command);
    resp.push_back('?');

    return resp;
}

const std::string AtStandardCommand::getExecuteCommand()
{
    std::string resp;
    resp.reserve(3 + command.size());

    resp.append("AT+");
    resp.append(command);

    return resp;
}

const std::string AtStandardCommand::getSetCommand()
{
    std::string resp;
    resp.reserve(3 + command.size() + 1);

    resp.append("AT+");
    resp.append(command);
    resp.push_back('=');

    return resp;
}

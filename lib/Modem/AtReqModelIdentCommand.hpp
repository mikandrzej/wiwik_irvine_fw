#pragma once

#include <Arduino.h>
#include "AtStandardCommand.hpp"

using AtCgmmResponseHandler = void (*)(const std::vector<char> &);

class AtReqModelIdentCommand : private AtStandardCommand
{
public:
    AtReqModelIdentCommand(AtCgmmResponseHandler responseHandler);
    void parseData(const std::vector<char> &data) override;

private:
    AtCgmmResponseHandler responseHandler;
};

AtReqModelIdentCommand::AtReqModelIdentCommand(AtCgmmResponseHandler responseHandler)
{
}

inline AtReqModelIdentCommand::AtReqModelIdentCommand(AtCgmmResponseHandler responseHandler)
    : AtStandardCommand("CGMM", 100u)
{
}

inline void AtReqModelIdentCommand::parseData(const std::vector<char> &data)
{
    if (responseHandler)
        responseHandler(data);
}

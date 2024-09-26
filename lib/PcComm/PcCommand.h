#pragma once

#include <string.h>

using PcCommandCallback = std::function<void(char *)>;

class PcCommand
{
public:
    PcCommand(const char *command, PcCommandCallback callback) : command(command), callback(callback) { commandLen = strlen(command); };
    const char *getCommand() { return command; };
    size_t getCommandLen() { return commandLen; };
    PcCommandCallback getCallback() { return callback; };

private:
    const char *command;
    size_t commandLen;
    PcCommandCallback callback;
};

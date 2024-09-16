#pragma once

class Updater
{
public:
    bool updateTrigger(const char *server, const char *path);
};

extern Updater updater;
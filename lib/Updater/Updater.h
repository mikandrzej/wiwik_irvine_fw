#pragma once

class Updater
{
public:
    bool updateTrigger(const char *url);
};

extern Updater updater;
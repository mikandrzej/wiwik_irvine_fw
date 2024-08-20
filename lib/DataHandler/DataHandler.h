#pragma once

#include <stdint.h>

#include <DataLoggable.h>

class DataHandler
{
public:
    static void handleData(DataLoggable &data);
};

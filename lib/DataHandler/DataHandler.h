#pragma once

#include <stdint.h>

#include <DataLoggable.h>
#include <memory>

class DataHandler
{
public:
    static void handleData(DataLoggable &data);
};

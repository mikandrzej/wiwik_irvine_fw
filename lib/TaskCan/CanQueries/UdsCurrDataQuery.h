#pragma once

#include "../CanQuery.h"
#include "IrvineConfiguration.h"

#define UDS_SERVICE_CURR_DATA 1u

#pragma pack(push, 1)
using UdsFrame = struct
{
    uint8_t additionalBytes;
    uint8_t service;
    uint8_t pid;
    uint8_t data[5u];
};
#pragma pack(pop)

class UdsCurrDataQuery : public CanQuery
{
public:
    UdsCurrDataQuery(uint8_t pid);

    bool messageReceived(uint32_t id, bool extd, bool rtr, uint8_t dataLen, uint8_t *data);

    virtual bool parseReceivedValue(uint8_t dataLen, uint8_t *data) = 0;
    void sendQuery();
    void intervalElapsed() override;

protected:
    uint32_t queryId = 0u;
    uint8_t paddingChar;
    bool padding;
    uint8_t pid;
    uint32_t responseId;

};
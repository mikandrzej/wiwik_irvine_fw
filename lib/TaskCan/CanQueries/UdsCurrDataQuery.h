#pragma once

#include "CanQuery.h"
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

    void sendQuery();

protected:
    virtual bool parseCurrDataResponse(uint8_t dataLen, uint8_t *data) = 0;

    uint32_t queryId = 0u;
    uint8_t paddingChar;
    bool padding;
    uint8_t pid;
    bool extd;
    uint32_t responseId;

private:
    bool parseReceivedFrame(twai_message_t &msg) override;
};
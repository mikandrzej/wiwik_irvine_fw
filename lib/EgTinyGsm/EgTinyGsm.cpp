#include "EgTinyGsm.h"

bool EgTinyGsm::httpInit()
{
    sendAT(GF("+HTTPINIT"));

    if (waitResponse() != 1)
    {
        return false;
    }
    return true;
}

bool EgTinyGsm::httpTerminate()
{
    sendAT(GF("+HTTPTERM"));

    if (waitResponse() != 1)
    {
        return false;
    }
    return true;
}

bool EgTinyGsm::httpSetUrl(String &url)
{
    sendAT(GF(R"(+HTTPPARA="URL",")"), url, '"');

    if (waitResponse() != 1)
    {
        return false;
    }
    return true;
}

{
    return false;
}

bool EgTinyGsm::httpSetConnectTimeout(int seconds)
{
    if (seconds < 2 || seconds > 120)
        return false;
    sendAT(GF(R"(+HTTPPARA="CONNECTTO",")"), seconds, '"');

    if (waitResponse() != 1)
    {
        return false;
    }
    return true;
}

bool EgTinyGsm::httpSetResponseTimeout(int seconds)
{
    if (seconds < 2 || seconds > 120)
        return false;
    sendAT(GF(R"(+HTTPPARA="RECVTO",")"), seconds, '"');

    if (waitResponse() != 1)
    {
        return false;
    }
    return true;
}

bool EgTinyGsm::httpAction(int action)
{
    if (action < 0 || action > 4)
        return false;
    sendAT(GF("+HTTPACTION="), action);

    if (waitResponse() != 1)
    {
        return false;
    }

    int method;
    int status;
    int datalen;

    method = streamGetIntBefore(',');
    status = streamGetIntBefore(',');
    datalen = streamGetIntBefore('\r');

    return true;
}

uint32_t EgTinyGsm::getHttpResponseLength()
{
    return httpResponseLength;
}

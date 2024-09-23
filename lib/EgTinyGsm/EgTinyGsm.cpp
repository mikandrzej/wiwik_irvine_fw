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

bool EgTinyGsm::httpSetUrl(String &&url)
{
    sendAT(GF(R"(+HTTPPARA="URL",")"), url, '"');

    if (waitResponse() != 1)
    {
        return false;
    }
    return true;
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

int EgTinyGsm::httpAction(int action)
{
    if (action < 0 || action > 4)
        return -1;
    sendAT(GF("+HTTPACTION="), action);

    if (waitResponse() != 1)
    {
        return -2;
    }

    if (waitResponse(60000, GF("+HTTPACTION:")) != 1)
    {
        return -3;
    }

    int method;
    int status;
    int datalen;

    method = streamGetIntBefore(',');
    status = streamGetIntBefore(',');
    datalen = streamGetInt32Before('\r');

    httpResponseLength = datalen;

    return status;
}

int EgTinyGsm::getHttpResponseLength()
{
    return httpResponseLength;
}

int EgTinyGsm::getHttpResponse(int length, uint8_t *buf)
{
    sendAT(GF("+HTTPREAD="), length);
    if (waitResponse(100) != 1)
    {
        return -1;
    }

    // stream.setTimeout(100);

    int totalReadLength = 0;
    int readLength;
    while (1)
    {
        if (waitResponse(100, GF("+HTTPREAD:")) != 1)
        {
            return -2;
        }
        readLength = streamGetInt32Before('\r');
        stream.read(); // skip newline character
        if (!readLength)
            break;
        if (stream.readBytes(&buf[totalReadLength], readLength) != readLength)
        {
            return -3; // read error
        }
        totalReadLength += readLength;
    }

    streamSkipUntil('\r');

    return totalReadLength;
}

bool EgTinyGsm::saveHttpResponseToFile(String &&filename)
{
    sendAT(GF("+HTTPREADFILE="), filename);
    if (waitResponse(60000) != 1)
    {
        return false;
    }
    return true;
}

int EgTinyGsm::readFile(String &&filename, int length, uint8_t *buf)
{
    return 0;
}

int32_t EgTinyGsm::streamGetInt32Before(char lastChar)
{
    char buf[20];
    size_t bytesRead = stream.readBytesUntil(
        lastChar, buf, static_cast<size_t>(20));
    // if we read 7 or more bytes, it's an overflow
    if (bytesRead && bytesRead < 20)
    {
        buf[bytesRead] = '\0';
        int32_t res = atoi(buf);
        return res;
    }

    return -9999;
}
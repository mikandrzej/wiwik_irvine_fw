#pragma once

#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>

#define HTTP_ACTION_GET 0
#define HTTP_ACTION_POST 1
#define HTTP_ACTION_HEAD 2
#define HTTP_ACTION_DELETE 3
#define HTTP_ACTION_PUT 4

class EgTinyGsm : public TinyGsmSim7600
{
public:
  explicit EgTinyGsm(Stream &stream) : TinyGsmSim7600(stream)
  {
  }

  bool httpInit();
  bool httpTerminate();
  bool httpSetUrl(String &url);
  bool httpSetConnectTimeout(int seconds);
  bool httpSetResponseTimeout(int seconds);
  bool httpAction(int action = HTTP_ACTION_GET);
  uint32_t getHttpResponseLength();

protected:
  bool httpOpened = false;
  uint32_t httpResponseLength = 0u;
};
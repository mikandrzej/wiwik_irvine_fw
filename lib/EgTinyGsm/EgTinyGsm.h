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
  bool httpSetUrl(String &&url);
  bool httpSetConnectTimeout(int seconds);
  bool httpSetResponseTimeout(int seconds);
  int httpAction(int action = HTTP_ACTION_GET);
  int getHttpResponseLength();
  int getHttpResponse(int length, uint8_t *buf);
  bool saveHttpResponseToFile(String &&filename);
  int readFile(String &&filename, int length, uint8_t *buf);

protected:
  bool httpOpened = false;
  int httpResponseLength = -1;

  int32_t streamGetInt32Before(char lastChar);
};
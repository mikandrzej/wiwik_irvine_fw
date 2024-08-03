#pragma once

#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>

class EgTinyGsm : public TinyGsmSim7600 {
 public:
  explicit EgTinyGsm(Stream& stream) : TinyGsmSim7600(stream) {
  }
};
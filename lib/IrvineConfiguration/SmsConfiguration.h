#pragma once

#define SMS_MNG_ALLOWED_NUMBERS_LEN 5u

class SmsConfiguration
{
public:
    char mngAllowedNumbers[SMS_MNG_ALLOWED_NUMBERS_LEN][15u];
};

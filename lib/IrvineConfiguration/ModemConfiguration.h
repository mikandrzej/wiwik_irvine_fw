#pragma once

#define MODEM_OPERATORS_WHITELIST_LEN 10u
#define MODEM_OPERATORS_BLACKLIST_LEN 10u

class ModemConfiguration
{
public:
    char apn[50u] = "";
    char apnUsername[50u] = "";
    char apnPassword[50u] = "";
    char pin[9u] = "";
    bool nationalRoaming = false;
    bool internationalRoaming = false;
    char operatorsWhitelist[MODEM_OPERATORS_WHITELIST_LEN][10u] = {0};
    char operatorsBlacklist[MODEM_OPERATORS_BLACKLIST_LEN][10u] = {0};
    uint32_t reportInterval = 0u;
};
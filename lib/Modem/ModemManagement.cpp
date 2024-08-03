#include <ModemManagement.h>
#include <Logger.h>
#include <EgTinyGsm.h>
#include <IrvineConfiguration.h>

const char MODULE[] = "MODEM_MNG";

#define MODEM_PWR_PIN 21
#define MODEM_RESET_PIN 16

ModemManagement modemManagement;
extern EgTinyGsm modem;

ModemManagement::ModemManagement()
{
    semaphore = xSemaphoreCreateMutex();
}

void ModemManagement::begin()
{
    logger.logPrintF(LogSeverity::INFO, MODULE, "Initialized");
}

void ModemManagement::loop()
{
    switch (state)
    {
    case ModemManagementState::UNINITIALIZED:
        pinMode(MODEM_RESET_PIN, OUTPUT);
        pinMode(MODEM_PWR_PIN, OUTPUT);
        digitalWrite(MODEM_RESET_PIN, HIGH);
        digitalWrite(MODEM_PWR_PIN, HIGH);
        state = ModemManagementState::POWER_OFF;
        break;

    case ModemManagementState::POWER_OFF:
    {
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem reset");
        digitalWrite(MODEM_PWR_PIN, HIGH);
        digitalWrite(MODEM_RESET_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(3000));
        digitalWrite(MODEM_RESET_PIN, LOW);

        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem power up");
        vTaskDelay(pdMS_TO_TICKS(300));
        digitalWrite(MODEM_PWR_PIN, LOW);

        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem power up delay");
        vTaskDelay(pdMS_TO_TICKS(20000));
        // todo wait for AT

        // todo obtain modem data
        if (!modem.init())
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "Modem initialization failed");
            state = ModemManagementState::INFINITE_LOOP;
            break;
        }
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem initialized");

        modemName = modem.getModemName();
        logger.logPrintF(LogSeverity::INFO, MODULE, "Modem name: %s", modemName.c_str());
        modemInfo = modem.getModemInfo();
        logger.logPrintF(LogSeverity::INFO, MODULE, "Modem info: %s", modemInfo.c_str());

        SimStatus sim_status = modem.getSimStatus(2000);
        if (sim_status == SIM_LOCKED)
        {
            if (modem.simUnlock(irvineConfiguration.modem.pin))
            {
                logger.logPrintF(LogSeverity::INFO, MODULE, "SIM unclocked");
            }
            else
            {
                logger.logPrintF(LogSeverity::ERROR, MODULE, "SIM unlock error");
                state = ModemManagementState::INFINITE_LOOP;
                break;
            }
        }
        else if (sim_status == SIM_READY)
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "SIM lock disabled");
        }
        else
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "SIM error");
            state = ModemManagementState::INFINITE_LOOP;
            break;
        }

        simCcid = modem.getSimCCID();
        simImsi = modem.getIMSI();
        logger.logPrintF(LogSeverity::INFO, MODULE, "SIM CCID: %s, SIM IMSI: %s", simCcid.c_str(), simImsi.c_str());

        xSemaphoreGive(semaphore);

        state = ModemManagementState::APN_DISCONNECTED;
        break;
    }
    case ModemManagementState::APN_DISCONNECTED:
        if (xSemaphoreTake(modemManagement.semaphore, (TickType_t)10) == pdTRUE)
        {
            if (modem.isNetworkConnected())
            {
                if (modem.gprsConnect(irvineConfiguration.modem.apn,
                                      irvineConfiguration.modem.apnUsername,
                                      irvineConfiguration.modem.apnPassword))
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "APN %s:%s/%s connected",
                                     irvineConfiguration.modem.apn,
                                     irvineConfiguration.modem.apnUsername,
                                     irvineConfiguration.modem.apnPassword);
                    state = ModemManagementState::APN_CONNECTED;
                }
            }
            xSemaphoreGive(modemManagement.semaphore);
        }
        break;

    case ModemManagementState::APN_CONNECTED:
        if (xSemaphoreTake(modemManagement.semaphore, (TickType_t)10) == pdTRUE)
        {
            if (!modem.isGprsConnected())
            {
                state = ModemManagementState::APN_DISCONNECTED;
            }
            xSemaphoreGive(modemManagement.semaphore);
        }
        break;

    case ModemManagementState::INFINITE_LOOP:
        break;
    }
}

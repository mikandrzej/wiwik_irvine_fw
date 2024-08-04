#include <ModemManagement.h>
#include <Logger.h>
#include <EgTinyGsm.h>
#include <IrvineConfiguration.h>
#include <TimeLib.h>
#include <Device.h>

const char MODULE[] = "MODEM_MNG";

#define SerialAT Serial1
#define MODEM_UART_BAUD 115200
#define MODEM_PIN_TX 19
#define MODEM_PIN_RX 18

#define MODEM_PWR_PIN 21
#define MODEM_RESET_PIN 16

ModemManagement modemManagement;
EgTinyGsm modem(SerialAT);

SemaphoreHandle_t modemSemaphore = NULL;

ModemManagement::ModemManagement()
{
}

void ModemManagement::begin()
{
    modemSemaphore = xSemaphoreCreateMutex();
    xSemaphoreTake(modemSemaphore, portMAX_DELAY);
}

void ModemManagement::loop()
{
    switch (state)
    {
    case ModemManagementState::UNINITIALIZED:
        SerialAT.begin(MODEM_UART_BAUD, SERIAL_8N1, MODEM_PIN_RX, MODEM_PIN_TX);

        pinMode(MODEM_RESET_PIN, OUTPUT);
        pinMode(MODEM_PWR_PIN, OUTPUT);
        digitalWrite(MODEM_RESET_PIN, HIGH);
        digitalWrite(MODEM_PWR_PIN, LOW);
        state = ModemManagementState::POWER_OFF;
        break;

    case ModemManagementState::POWER_OFF:
    {
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem reset");
        digitalWrite(MODEM_PWR_PIN, HIGH);
        digitalWrite(MODEM_RESET_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(MODEM_RESET_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));
        digitalWrite(MODEM_RESET_PIN, LOW);

        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem power up");
        digitalWrite(MODEM_PWR_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(MODEM_PWR_PIN, LOW);

        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem power up delay");
        vTaskDelay(pdMS_TO_TICKS(5000));
        // todo wait for AT

        // todo obtain modem data
        if (!modem.init())
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "Modem initialization failed");
            break;
        }
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem initialized");

        modemName = modem.getModemName();
        logger.logPrintF(LogSeverity::INFO, MODULE, "Modem name: %s", modemName.c_str());
        modemInfo = modem.getModemInfo();
        logger.logPrintF(LogSeverity::INFO, MODULE, "Modem info: %s", modemInfo.c_str());

        SimStatus sim_status = modem.getSimStatus(10000);
        if (sim_status == SIM_LOCKED)
        {
            if (modem.simUnlock(irvineConfiguration.modem.pin))
            {
                sim_status = modem.getSimStatus(10000);
                if (sim_status == SIM_READY)
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "SIM unlocked");
                    state = ModemManagementState::INFINITE_LOOP;
                }
            }
            else
            {
                logger.logPrintF(LogSeverity::ERROR, MODULE, "SIM unlock error");
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

        vTaskDelay(pdMS_TO_TICKS(3000));

        simCcid = modem.getSimCCID();
        simImsi = modem.getIMSI();
        logger.logPrintF(LogSeverity::INFO, MODULE, "SIM CCID: %s, SIM IMSI: %s", simCcid.c_str(), simImsi.c_str());

        int yearNetwork;
        int monthNetwork;
        int dayNetwork;
        int hourNetwork;
        int minuteNetwork;
        int secondNetwork;
        float timezoneNetwork;
        if (true == modem.getNetworkTime(
                        &yearNetwork,
                        &monthNetwork,
                        &dayNetwork,
                        &hourNetwork,
                        &minuteNetwork,
                        &secondNetwork,
                        &timezoneNetwork))
        {

            tmElements_t tm;

            tm.Day = dayNetwork;
            tm.Month = monthNetwork;
            tm.Year = yearNetwork - 1970;
            tm.Hour = hourNetwork;
            tm.Minute = minuteNetwork;
            tm.Second = secondNetwork;

            uint64_t unixTimestamp = makeTime(tm);

            // offset timezone
            unixTimestamp -= (int)timezoneNetwork * 60 * 60;
            device.updateGsmTime(unixTimestamp);

            logger.logPrintF(LogSeverity::DEBUG, MODULE, "time %d-%d-%d %d:%d:%d timezone %f unix:%llu",
                             yearNetwork,
                             monthNetwork,
                             dayNetwork,
                             hourNetwork,
                             minuteNetwork,
                             secondNetwork,
                             timezoneNetwork,
                             unixTimestamp);
        }

        xSemaphoreGive(modemSemaphore);

        state = ModemManagementState::APN_DISCONNECTED;
        break;
    }
    case ModemManagementState::APN_DISCONNECTED:
        if (xSemaphoreTake(modemSemaphore, portMAX_DELAY) == pdTRUE)
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
                    connected = true;
                }
            }
            xSemaphoreGive(modemSemaphore);
        }
        break;

    case ModemManagementState::APN_CONNECTED:
        if (xSemaphoreTake(modemSemaphore, portMAX_DELAY) == pdTRUE)
        {
            if (!modem.isGprsConnected())
            {
                connected = false;
                logger.logPrintF(LogSeverity::INFO, MODULE, "APN disconnected");
                state = ModemManagementState::APN_DISCONNECTED;
            }
            xSemaphoreGive(modemSemaphore);
        }
        break;

    case ModemManagementState::INFINITE_LOOP:
        break;
    }
}

bool ModemManagement::isConnected()
{
    return connected;
}

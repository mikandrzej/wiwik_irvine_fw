#include "Updater.h"
#include <Logger.h>
#include <EgTinyGsm.h>
#include <Update.h>
#include <IrvineConfiguration.h>

char MODULE[] = "UPDATER";

Updater updater;

// bool Updater::updateTrigger(const char *server, const char *path)
// {
//     logger.logPrintF(LogSeverity::INFO, MODULE, "Update started with url: %s and path: %s", server, path);

//     httpClient.connect(server, 80);

//     // Start the OTA update
//     int err = httpClient.get(path);
//     if (err)
//     {
//         logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP GET failed with code: %d", err);
//         httpClient.stop();
//         return false;
//     }

//     int httpCode = httpClient.responseStatusCode();
//     logger.logPrintF(LogSeverity::INFO, MODULE, "HTTP GET with response status code: %d", httpCode);
//     if (httpCode != 200)
//     {
//         httpClient.stop();
//         return false;
//     }

//     int length = httpClient.contentLength();
//     logger.logPrintF(LogSeverity::DEBUG, MODULE, "HTTP Update content length %d", length);

//     if (!length)
//     {
//         logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Content length 0");
//         httpClient.stop();
//         return false;
//     }

//     if (!Update.begin(length, U_FLASH))
//     {
//         const char *err_str = Update.errorString();
//         logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Update Error %s", err_str);
//         return false;
//     }

//     if ((Update.writeStream(httpClient) != length) || (!Update.end()))
//     {
//         const char *err_str = Update.errorString();
//         logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Update Error %s", err_str);
//         return false;
//     }

//     logger.logPrintF(LogSeverity::INFO, MODULE, "HTTP Update Complete");

//     return true;
// }

void Updater::loop()
{
    switch (updateState)
    {
    case UpdateState::TRIGGERED:
        currentAddress = 0u;
        updateState = UpdateState::IN_PROGRESS;
        break;

    case UpdateState::IN_PROGRESS:
    {
        int remainingBytes = newFileLen - currentAddress;
        int chunkLength = remainingBytes > sizeof(buf) ? sizeof(buf) : remainingBytes;

        int bytesRead = modem.getHttpResponse(chunkLength, buf);
        if (bytesRead <= 0)
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Read failed: %d", bytesRead);
            sendUpdateStatus("HTTP Read failed");
            updateState = UpdateState::IDLE;
        }

        if (bytesRead != chunkLength)
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Read length incorrect. Expexted %d was %d", chunkLength, bytesRead);
            sendUpdateStatus("HTTP length incorrect");
            Update.abort();
            updateState = UpdateState::IDLE;
        }

        size_t bytesWritten = Update.write(buf, bytesRead);
        if (bytesRead != bytesWritten)
        {
            sendUpdateStatus("Update write failed");
            logger.logPrintF(LogSeverity::ERROR, MODULE, "Update write length incorrect. Expexted %d was %d. Error: %s", bytesRead, bytesWritten, Update.errorString());
            Update.abort();
            updateState = UpdateState::IDLE;
        }

        currentAddress += bytesWritten;
        float progress = ((float)currentAddress * 100.0f) / (float)newFileLen;
        logger.logPrintF(LogSeverity::INFO, MODULE, "Update progress: %.2f%%", progress);
        sendUpdateProgress(progress);

        if (currentAddress >= newFileLen)
        {
            updateState = UpdateState::FINISHING;
        }
    }
    break;

    case UpdateState::FINISHING:
        if (!Update.end())
        {
            String errText = "Update error: " + String(Update.errorString());
            sendUpdateStatus(errText.c_str());
            const char *err_str = Update.errorString();
            logger.logPrintF(LogSeverity::ERROR, MODULE, "Update Error %s", err_str);
        }
        else
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "Update Succesfull");
            sendUpdateStatus("Success");
        }
        modem.httpTerminate();

        sendUpdateStatus("Module restarting...");
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Module restarting...");
        resetTimestamp = millis();
        updateState = UpdateState::BEFORE_RESET;
        break;

    case UpdateState::BEFORE_RESET:
        if (millis() - resetTimestamp > 5000)
        {

            delay(5000);

            ESP.restart();
        }
        break;
    }
}

bool Updater::updateTrigger(const char *url)
{
    bool result = false;

    do
    {
        if (updateState != UpdateState::IDLE)
        {
            sendUpdateStatus("Update in progress");
            logger.logPrintF(LogSeverity::WARNING, MODULE, "Update in progress");
            break;
        }

        modem.httpTerminate();
        if (!modem.httpInit())
        {
            sendUpdateStatus("HTTP_init_fail");
            logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP_init fail");
            break;
        }

        if (!modem.httpSetUrl(String(url)))
        {
            modem.httpTerminate();

            sendUpdateStatus("SET_URL_FAIL");
            logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP_setUrl fail");
            break;
        }

        int status = modem.httpAction(HTTP_ACTION_GET);
        if (status != 200)
        {
            modem.httpTerminate();

            sendUpdateStatus("HTTP_GET_FAIL");
            logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP_action fail, status: %d", status);
            break;
        }
        logger.logPrintF(LogSeverity::INFO, MODULE, "HTTP_action status: %d", status);

        int length = modem.getHttpResponseLength();
        logger.logPrintF(LogSeverity::INFO, MODULE, "HTTP_reponseLength: %d", length);

        // Update.abort();
        if (!Update.begin(length))
        {
            modem.httpTerminate();

            const char *err_str = Update.errorString();
            logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Update Error %s", err_str);
            String errText = "Update error: " + String(Update.errorString());
            sendUpdateStatus(errText.c_str());
            break;
        }

        newFileLen = length;
        logger.logPrintF(LogSeverity::INFO, MODULE, "Update triggerred");
        sendUpdateStatus("Update triggerred");

        updateState = UpdateState::TRIGGERED;

    } while (false);

    return true;
}

void Updater::sendUpdateProgress(const float progress)
{
    sprintf(mqttTxItem.msg, R"({"progress":%.2f})", progress);
    sprintf(mqttTxItem.topic, "irvine/%s/update/progress", irvineConfiguration.device.deviceId);
    modemManagement.mqttPublish(mqttTxItem);
}

void Updater::sendUpdateStatus(const char *status)
{
    sprintf(mqttTxItem.msg, R"({"status":"%s"})", status);
    sprintf(mqttTxItem.topic, "irvine/%s/update/status", irvineConfiguration.device.deviceId);
    modemManagement.mqttPublish(mqttTxItem);
}

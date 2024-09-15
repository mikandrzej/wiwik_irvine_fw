#include "Updater.h"
#include <Logger.h>
#include <EgTinyGsm.h>
#include <ArduinoHttpClient.h>
#include <Update.h>
#include <ModemManagement.h>

char MODULE[] = "UPDATER";

Updater updater;

bool Updater::updateTrigger(const char *url)
{
    logger.logPrintF(LogSeverity::INFO, MODULE, "Update started with url: %s", url);

    HttpClient http(updateClient, url);
    http.connect(url, 80);

    // Start the OTA update
    int err = http.get(url);
    if (err)
    {
        http.stop();
        return false;
    }

    int httpCode = http.responseStatusCode();
    if (httpCode != 200)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP GET failed with code: %d", httpCode);
        http.stop();
        return false;
    }

    int length = http.contentLength();
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "HTTP Update content length %d", length);

    if (!length)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Content length 0");
        http.stop();
        return false;
    }

    if ((Update.writeStream(http) != length) || (!Update.end()))
    {
        const char *err_str = Update.errorString();
        logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Update Error %s", err_str);
        return false;
    }

    return true;
}
#include "Updater.h"
#include <Logger.h>
#include <EgTinyGsm.h>
#include <Update.h>
#include <ModemManagement.h>

char MODULE[] = "UPDATER";

Updater updater;

bool Updater::updateTrigger(const char *server, const char *path)
{
    logger.logPrintF(LogSeverity::INFO, MODULE, "Update started with url: %s and path: %s", server, path);

    httpClient.connect(server, 80);

    // Start the OTA update
    int err = httpClient.get(path);
    if (err)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP GET failed with code: %d", err);
        httpClient.stop();
        return false;
    }

    int httpCode = httpClient.responseStatusCode();
    logger.logPrintF(LogSeverity::INFO, MODULE, "HTTP GET with response status code: %d", httpCode);
    if (httpCode != 200)
    {
        httpClient.stop();
        return false;
    }

    int length = httpClient.contentLength();
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "HTTP Update content length %d", length);

    if (!length)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Content length 0");
        httpClient.stop();
        return false;
    }

    if (!Update.begin(length, U_FLASH))
    {
        const char *err_str = Update.errorString();
        logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Update Error %s", err_str);
        return false;
    }

    if ((Update.writeStream(httpClient) != length) || (!Update.end()))
    {
        const char *err_str = Update.errorString();
        logger.logPrintF(LogSeverity::ERROR, MODULE, "HTTP Update Error %s", err_str);
        return false;
    }

    logger.logPrintF(LogSeverity::INFO, MODULE, "HTTP Update Complete");

    return true;
}
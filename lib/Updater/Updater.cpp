#include "Updater.h"
#include <Logger.h>
#include <EgTinyGsm.h>
#include <Update.h>
#include <IrvineConfiguration.h>
#include <TimeCounter.h>

char MODULE[] = "UPDATER";

Updater updater;

void Updater::loop()
{
    // todo statuses
    switch (updateState)
    {
    case UpdateState::TRIGGERED:
    {
        modem.httpTerminate();
        if (!modem.httpInit())
        {
            updateError("HTTP init fail");
            updateState = UpdateState::IDLE;
            break;
        }

        if (!modem.httpSetUrl(String(url)))
        {
            updateError("HTTP set URL fail");
            updateState = UpdateState::IDLE;
            break;
        }

        int status = modem.httpAction(HTTP_ACTION_GET);
        if (status != 200)
        {
            updateError("HTTP GET chunk list fail, code: %d", status);
            updateState = UpdateState::IDLE;
            break;
        }
        updateStatus("HTTP Get chunks OK");

        updateState = UpdateState::OBTAIN_CHUNK_LIST;
    }
    break;

    case UpdateState::OBTAIN_CHUNK_LIST:
    {
        bool stop = false;
        int length = modem.getHttpResponseLength();
        updateStatus("HTTP Get chunks length = %d", length);

        if (length >= sizeof(buf))
        {
            updateError("Chunk list file: %u, exceed %u", length, sizeof(buf));
            updateState = UpdateState::IDLE;
            break;
        }

        int bytesRead = modem.getHttpResponse(length, buf);
        if (bytesRead != length)
        {
            updateError("Chunk list HTTP Read failed - response: %d , expected: %d", bytesRead, length);
            logger.logPrintF(LogSeverity::WARNING, MODULE, "Invalid length of http response: %d , expected: %d", bytesRead, length);
            updateState = UpdateState::IDLE;
            break;
        }

        char *pch;
        pch = strtok((char *)buf, "\t \r\n");

        // Get total length
        totalLength = atoi(pch);

        pch = strtok(NULL, "\t \r\n");

        uint8_t k = 0u;
        while (pch != NULL)
        {
            size_t filenameLength = strlen(pch);
            if (filenameLength == 0u)
            {
                logger.logPrintF(LogSeverity::WARNING, MODULE, "Empty filename");
                continue;
            }
            if (filenameLength >= MAX_FILENAME_LEN)
            {
                updateError("Chunk filename too long: %u", filenameLength);
                updateState = UpdateState::IDLE;
                stop = true;
                break;
            }
            strncpy(chunkFiles[k], pch, MAX_FILENAME_LEN);
            k++;

            if (k > MAX_CHUNKS)
            {
                updateError("Too much chunks");
                updateState = UpdateState::IDLE;
                stop = true;
                break;
            }

            pch = strtok(NULL, "\t \r\n");
        }
        if (stop)
        {
            break;
        }

        if (k == 0)
        {
            updateError("Empty chunk list");
            updateState = UpdateState::IDLE;
            break;
        }

        chunkCnt = k;

        for (uint8_t t = 0u; t < k; t++)
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "Update chunk %u: %s", t, chunkFiles[t]);
        }

        Update.end();
        if (!Update.begin(totalLength))
        {
            modem.httpTerminate();

            const char *err_str = Update.errorString();
            updateError("Update error: %s, total length: %u", Update.errorString(), totalLength);
            updateState = UpdateState::IDLE;
            break;
        }

        updateStatus("Update started");

        currentChunk = 0u;
        currentUpdatePosition = 0u;
        updateState = UpdateState::GET_CHUNK;
    }
    break;

    case UpdateState::GET_CHUNK:
    {
        TIME_COUNT_BEGIN
        char chunkUrl[100];

        if (!buildChunkUrl(this->url, chunkFiles[currentChunk], chunkUrl))
        {
            updateError("Build chunk URL failed for: %s", chunkFiles[currentChunk]);
            updateState = UpdateState::IDLE;
            break;
        }

        logger.logPrintF(LogSeverity::DEBUG, MODULE, "chunk url: %s", chunkUrl);

        if (!modem.httpSetUrl(String(chunkUrl)))
        {
            updateError("HTTP set URL for chunk %u fail", currentChunk);
            updateState = UpdateState::IDLE;
            break;
        }

        int status = modem.httpAction(HTTP_ACTION_GET);
        if (status != 200)
        {
            updateError("HTTP GET for chunk %u fail", currentChunk);
            updateState = UpdateState::IDLE;
            break;
        }

        updateStatus("HTTP Get chunk %u OK", currentChunk);

        chunkLength = modem.getHttpResponseLength();
        if (chunkLength < 0)
        {
            updateError("Chunk %u read error", currentChunk);
            updateState = UpdateState::IDLE;
            break;
        }
        updateStatus("HTTP chunk %u length = %d", currentChunk, chunkLength);

        currentChunkPosition = 0;

        updateState = UpdateState::WRITE_CHUNK;
        TIME_COUNT_END("Chunk GET")
    }
    break;

    case UpdateState::WRITE_CHUNK:
    {
        int remainingBytes = chunkLength - currentChunkPosition;
        int stepLength = remainingBytes > sizeof(buf) ? sizeof(buf) : remainingBytes;

        int bytesRead = modem.getHttpResponse(stepLength, buf);
        if (bytesRead <= 0)
        {
            updateError("Chunk %u read error %d", bytesRead);
            Update.abort();
            updateState = UpdateState::IDLE;
            break;
        }

        if (bytesRead != stepLength)
        {
            updateError("HTTP Read length incorrect. Expexted %d was %d", chunkLength, bytesRead);
            Update.abort();
            updateState = UpdateState::IDLE;
            break;
        }

        TIME_COUNT_BEGIN
        size_t bytesWritten = Update.write(buf, bytesRead);
        if (bytesRead != bytesWritten)
        {
            updateError("Update write failed: %s", Update.errorString());
            Update.abort();
            updateState = UpdateState::IDLE;
            break;
        }

        currentChunkPosition += bytesWritten;
        currentUpdatePosition += bytesWritten;
        TIME_COUNT_END("Chunk write")

        float progress = ((float)currentUpdatePosition * 100.0f) / (float)totalLength;
        updateProgress(progress);

        if (currentChunkPosition >= chunkLength)
        {
            if (currentUpdatePosition >= totalLength)
            {
                updateState = UpdateState::FINISHING;
            }
            else
            {
                currentChunk++;
                updateState = UpdateState::GET_CHUNK;
            }
        }
    }
    break;

    case UpdateState::FINISHING:
        modem.httpTerminate();
        if (!Update.end())
        {
            updateError("Update end failed: %s", Update.errorString());
            Update.abort();
            updateState = UpdateState::IDLE;
        }
        else
        {
            updateStatus("Success");
            updateState = UpdateState::BEFORE_RESET;
            updateStatus("Module restarting...");
            resetTimestamp = millis();
        }

        break;

    case UpdateState::BEFORE_RESET:
        if (millis() - resetTimestamp > 5000)
        {
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
            updateStatus("Update in progress");
            logger.logPrintF(LogSeverity::WARNING, MODULE, "Update in progress");
            break;
        }

        strncpy(this->url, url, sizeof(this->url));
        updateState = UpdateState::TRIGGERED;

        result = true;
    } while (false);

    return result;
}

void Updater::updateProgress(const float progress)
{
    char val[10];
    sprintf(val, "%.1f", progress);

    logger.logPrintF(LogSeverity::INFO, MODULE, "Update progress: %s", val);

    sprintf(mqttTxItem.msg, R"({"progress":%s})", val);
    sprintf(mqttTxItem.topic, "irvine/%s/update/progress", irvineConfiguration.device.deviceId);
    modemManagement.mqttPublish(mqttTxItem);
}

void Updater::updateStatus(const char *format, ...)
{
    char status[256]; // Buffer for formatted status message
    va_list args;

    // Start processing the variable arguments
    va_start(args, format);

    // Format the status string
    vsnprintf(status, sizeof(status), format, args);

    // End processing the variable arguments
    va_end(args);

    // Log the status message using the logger
    logger.logPrintF(LogSeverity::INFO, MODULE, status);

    // Use the formatted status in the MQTT message
    sprintf(mqttTxItem.msg, R"({"status":"%s"})", status);
    sprintf(mqttTxItem.topic, "irvine/%s/update/status", irvineConfiguration.device.deviceId);

    // Publish the message using MQTT
    if (!modemManagement.mqttPublish(mqttTxItem))
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "MQTT Publish failed for status message");
    }
}

void Updater::updateError(const char *format, ...)
{
    char status[256]; // Buffer for formatted status message
    va_list args;

    // Start processing the variable arguments
    va_start(args, format);

    // Format the status string
    vsnprintf(status, sizeof(status), format, args);

    // End processing the variable arguments
    va_end(args);

    // Log the status message using the logger
    logger.logPrintF(LogSeverity::ERROR, MODULE, status);

    // Use the formatted status in the MQTT message
    sprintf(mqttTxItem.msg, R"({"status":"%s"})", status);
    sprintf(mqttTxItem.topic, "irvine/%s/update/status", irvineConfiguration.device.deviceId);

    // Publish the message using MQTT
    if (!modemManagement.mqttPublish(mqttTxItem))
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "MQTT Publish failed for status message");
    }
}

bool Updater::buildChunkUrl(const char *origin, const char *chunkFilename, char *url)
{
    if (!origin || !chunkFilename || !url)
    {
        return false;
    }

    // Find the last occurrence of '/' in the origin URL
    const char *lastSlash = strrchr(origin, '/');

    if (lastSlash)
    {
        // Copy the origin up to the last '/'
        size_t baseLength = lastSlash - origin + 1;
        strncpy(url, origin, baseLength);
        url[baseLength] = '\0'; // Null-terminate the string
    }
    else
    {
        return false;
    }

    // Concatenate the chunk filename
    strncat(url, chunkFilename, sizeof(url) - strlen(url) - 1);
    return true;
}
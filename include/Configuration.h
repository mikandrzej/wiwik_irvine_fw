#pragma once

class Configuration
{
public:
    void initSource();
    void readConfig();

private:
    bool readReportInterval();

private:
    bool m_sourceInit = false;
    int m_reportInterval = -1;

    
private:
    const String m_config_path = "/config/";
    const String m_report_interval_path = m_config_path + "report_interval.txt";
};

extern Configuration configuration;
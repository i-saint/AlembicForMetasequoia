#pragma once
#include "MQWidget.h"

class mqabcRecorderPlugin;

class mqabcRecorderWindow : public MQWindow
{
public:
    mqabcRecorderWindow(mqabcRecorderPlugin* plugin, MQWindowBase& parent);
    void LogInfo(const char *message);

private:
    mqabcRecorderPlugin* m_plugin = nullptr;
};

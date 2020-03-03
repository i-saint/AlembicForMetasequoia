#pragma once
#include "MQWidget.h"

class mqabcPlugin;

class mqabcSettingsDlg : public MQWindow
{
public:
    mqabcSettingsDlg(mqabcPlugin* plugin, MQWindowBase& parent);
    void LogInfo(const char *message);

private:
    mqabcPlugin* m_plugin = nullptr;
};

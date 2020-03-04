#pragma once
#include "MQWidget.h"

class mqabcPlayerPlugin;

class mqabcPlayerWindow : public MQWindow
{
public:
    mqabcPlayerWindow(mqabcPlayerPlugin* plugin, MQWindowBase& parent);

    BOOL OnHide(MQWidgetBase* sender, MQDocument doc);
    BOOL OnOpenClicked(MQWidgetBase* sender, MQDocument doc);

    void LogInfo(const char *message);

private:
    mqabcPlayerPlugin* m_plugin = nullptr;

    MQSlider* m_slider_interval = nullptr;
    MQButton* m_button_open = nullptr;
    MQLabel* m_log = nullptr;
};

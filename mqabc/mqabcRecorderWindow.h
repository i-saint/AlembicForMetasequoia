#pragma once
#include "MQWidget.h"

class mqabcRecorderPlugin;

class mqabcRecorderWindow : public MQWindow
{
public:
    mqabcRecorderWindow(mqabcRecorderPlugin* plugin, MQWindowBase& parent);

    BOOL OnHide(MQWidgetBase* sender, MQDocument doc);
    BOOL OnIntervalChange(MQWidgetBase* sender, MQDocument doc);
    BOOL OnIntervalSlide(MQWidgetBase* sender, MQDocument doc);
    BOOL OnScaleChange(MQWidgetBase* sender, MQDocument doc);
    BOOL OnRecordingClicked(MQWidgetBase* sender, MQDocument doc);

    void LogInfo(const char *message);

private:
    mqabcRecorderPlugin* m_plugin = nullptr;

    MQEdit* m_edit_interval = nullptr;
    MQSlider* m_slider_interval = nullptr;
    MQEdit* m_edit_scale = nullptr;
    MQCheckBox* m_check_freeze_mirror = nullptr;
    MQCheckBox* m_check_freeze_lathe = nullptr;
    MQCheckBox* m_check_freeze_subdiv = nullptr;
    MQButton* m_button_recording = nullptr;
    MQLabel* m_log = nullptr;
};

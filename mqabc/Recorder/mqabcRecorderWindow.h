#pragma once
#include "MQWidget.h"

class mqabcRecorderPlugin;

class mqabcRecorderWindow : public MQWindow
{
public:
    mqabcRecorderWindow(mqabcRecorderPlugin* plugin, MQWindowBase& parent);

    BOOL OnShow(MQWidgetBase* sender, MQDocument doc);
    BOOL OnHide(MQWidgetBase* sender, MQDocument doc);
    BOOL OnIntervalChange(MQWidgetBase* sender, MQDocument doc);
    BOOL OnScaleChange(MQWidgetBase* sender, MQDocument doc);
    BOOL OnSettingsUpdate(MQWidgetBase* sender, MQDocument doc);
    BOOL OnRecordingClicked(MQWidgetBase* sender, MQDocument doc);
#ifdef mqabcDebug
    BOOL OnDebugClicked(MQWidgetBase* sender, MQDocument doc);
#endif // mqabcDebug

    void SyncSettings();
    void LogInfo(const char *message);

private:
    mqabcRecorderPlugin* m_plugin = nullptr;

    MQFrame* m_frame_settings = nullptr;
    MQEdit* m_edit_interval = nullptr;
    MQEdit* m_edit_scale = nullptr;
    MQCheckBox* m_check_mirror = nullptr;
    MQCheckBox* m_check_lathe = nullptr;
    MQCheckBox* m_check_subdiv = nullptr;
    MQCheckBox* m_check_normals = nullptr;
    MQCheckBox* m_check_colors = nullptr;
    MQCheckBox* m_check_mids = nullptr;
    MQButton* m_button_recording = nullptr;
    MQMemo* m_log = nullptr;
#ifdef mqabcDebug
    MQButton* m_button_debug = nullptr;
#endif // mqabcDebug
};

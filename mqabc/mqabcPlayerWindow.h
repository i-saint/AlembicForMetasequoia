#pragma once
#include "MQWidget.h"

class mqabcPlayerPlugin;

class mqabcPlayerWindow : public MQWindow
{
public:
    mqabcPlayerWindow(mqabcPlayerPlugin* plugin, MQWindowBase& parent);

    BOOL OnShow(MQWidgetBase* sender, MQDocument doc);
    BOOL OnHide(MQWidgetBase* sender, MQDocument doc);
    BOOL OnOpenClicked(MQWidgetBase* sender, MQDocument doc);
    BOOL OnSampleEdit(MQWidgetBase* sender, MQDocument doc);
    BOOL OnSampleSlide(MQWidgetBase* sender, MQDocument doc);
    BOOL OnSettingsUpdate(MQWidgetBase* sender, MQDocument doc);

    void SyncSettings();
    void LogInfo(const char *message);

private:
    mqabcPlayerPlugin* m_plugin = nullptr;

    MQFrame* m_frame_open = nullptr;
    MQButton* m_button_open = nullptr;

    MQFrame* m_frame_play = nullptr;
    MQEdit* m_edit_sample = nullptr;
    MQSlider* m_slider_sample = nullptr;

    MQEdit* m_edit_scale = nullptr;
    MQCheckBox* m_check_flip_faces = nullptr;
    MQCheckBox* m_check_flip_x = nullptr;
    MQCheckBox* m_check_flip_yz = nullptr;

    MQMemo* m_log = nullptr;
};

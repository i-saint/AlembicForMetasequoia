#pragma once
#include "MQWidget.h"

class mqabcPlayerPlugin;

class mqabcPlayerWindow : public MQWindow
{
public:
    mqabcPlayerWindow(mqabcPlayerPlugin* plugin, MQWindowBase& parent);

    BOOL OnHide(MQWidgetBase* sender, MQDocument doc);
    BOOL OnOpenClicked(MQWidgetBase* sender, MQDocument doc);
    BOOL OnTimeEdit(MQWidgetBase* sender, MQDocument doc);
    BOOL OnTimeSlide(MQWidgetBase* sender, MQDocument doc);

    void LogInfo(const char *message);

private:
    mqabcPlayerPlugin* m_plugin = nullptr;

    MQFrame* m_frame_open = nullptr;
    MQFrame* m_frame_time = nullptr;
    MQEdit* m_edit_time = nullptr;
    MQSlider* m_slider_time = nullptr;
    MQButton* m_button_open = nullptr;
    MQMemo* m_log = nullptr;
};

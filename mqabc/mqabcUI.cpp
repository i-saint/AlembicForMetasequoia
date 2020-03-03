#include "pch.h"
#include "mqabcUI.h"

mqabcSettingsDlg::mqabcSettingsDlg(mqabcPlugin* plugin, MQWindowBase& parent)
    : MQWindow(parent)
{
    setlocale(LC_ALL, "");

    m_plugin = plugin;

    SetTitle(L"Alembic For Metasequoia");
    SetOutSpace(0.4);
}

void mqabcSettingsDlg::LogInfo(const char* message)
{
    // todo
}

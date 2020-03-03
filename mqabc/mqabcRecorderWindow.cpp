#include "pch.h"
#include "mqabcRecorderWindow.h"

mqabcRecorderWindow::mqabcRecorderWindow(mqabcRecorderPlugin* plugin, MQWindowBase& parent)
    : MQWindow(parent)
{
    setlocale(LC_ALL, "");

    m_plugin = plugin;

    SetTitle(L"Alembic For Metasequoia");
    SetOutSpace(0.4);
}

void mqabcRecorderWindow::LogInfo(const char* message)
{
    // todo
}

#include "pch.h"
#include "mqabc.h"
#include "mqabcRecorderPlugin.h"
#include "mqabcRecorderWindow.h"

mqabcRecorderWindow::mqabcRecorderWindow(mqabcRecorderPlugin* plugin, MQWindowBase& parent)
    : MQWindow(parent)
{
    setlocale(LC_ALL, "");

    m_plugin = plugin;

    SetTitle(L"Alembic Recorder");
    SetOutSpace(0.4);

    double outer_margin = 0.2;
    double inner_margin = 0.1;

    {
        MQFrame* vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        CreateLabel(vf, L"Capture Interval (second):");
        m_slider_interval = CreateSlider(vf);
        m_slider_interval->SetMin(0.0f);
        m_slider_interval->SetMax(180.0f);
        m_slider_interval->SetPosition(m_plugin->GetInterval());
        m_slider_interval->AddChangedEvent(this, &mqabcRecorderWindow::OnIntervalChange);
    }

    {
        MQFrame* vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        m_button_recording = CreateButton(vf, L"Start Recording");
        m_button_recording->AddClickEvent(this, &mqabcRecorderWindow::OnRecordingClicked);
    }

    {
        MQFrame* vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        std::string plugin_version = "Plugin Version: " mqabcVersionString;
        CreateLabel(vf, mu::ToWCS(plugin_version));
    }
    {
        MQFrame* vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        m_log = CreateLabel(vf, mu::ToWCS(""));
    }
    this->AddHideEvent(this, &mqabcRecorderWindow::OnHide);
}

BOOL mqabcRecorderWindow::OnHide(MQWidgetBase* sender, MQDocument doc)
{
    m_plugin->CloseABC();
    m_plugin->WindowClose();
    return 0;
}

BOOL mqabcRecorderWindow::OnIntervalChange(MQWidgetBase* sender, MQDocument doc)
{
    m_plugin->SetInterval(m_slider_interval->GetPosition());
    return 0;
}

BOOL mqabcRecorderWindow::OnRecordingClicked(MQWidgetBase* sender, MQDocument doc)
{
    if (!m_plugin->IsArchiveOpened()) {
        MQSaveFileDialog dlg(*this);
        dlg.AddFilter(L"Alembic file (*.abc)|*.abc");
        dlg.SetDefaultExt(L"abc");
        if (dlg.Execute()) {
            auto path = dlg.GetFileName();
            if (m_plugin->OpenABC(mu::ToMBS(path))) {
                m_button_recording->SetText(L"Stop Recording");
            }
        }
    }
    else {
        if (m_plugin->CloseABC()) {
            m_button_recording->SetText(L"Start Recording");
        }
    }

    return 0;
}

void mqabcRecorderWindow::LogInfo(const char* message)
{
    // todo
}

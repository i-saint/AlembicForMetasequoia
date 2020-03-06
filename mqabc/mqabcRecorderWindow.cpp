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

        {
            MQFrame* hf = CreateHorizontalFrame(vf);
            CreateLabel(hf, L"Capture Interval (second):");

            m_edit_interval = CreateEdit(hf);
            m_edit_interval->SetNumeric(MQEdit::NUMERIC_DOUBLE);
            m_edit_interval->AddChangedEvent(this, &mqabcRecorderWindow::OnIntervalChange);

            m_slider_interval = CreateSlider(vf);
            m_slider_interval->SetMin(0.0f);
            m_slider_interval->SetMax(180.0f);
            m_slider_interval->AddChangedEvent(this, &mqabcRecorderWindow::OnIntervalSlide);
        }
        {
            MQFrame* hf = CreateHorizontalFrame(vf);
            CreateLabel(hf, L"Scale Factor");
            m_edit_scale = CreateEdit(hf);
            m_edit_scale->SetNumeric(MQEdit::NUMERIC_DOUBLE);
            m_edit_scale->AddChangedEvent(this, &mqabcRecorderWindow::OnScaleChange);
        }

        m_check_freeze_mirror = CreateCheckBox(vf, L"Freeze Mirror");
        m_check_freeze_mirror->AddChangedEvent(this, &mqabcRecorderWindow::OnFreezeChange);

        m_check_freeze_lathe = CreateCheckBox(vf, L"Freeze Lathe");
        m_check_freeze_lathe->AddChangedEvent(this, &mqabcRecorderWindow::OnFreezeChange);

        m_check_freeze_subdiv = CreateCheckBox(vf, L"Freeze Subdiv");
        m_check_freeze_subdiv->AddChangedEvent(this, &mqabcRecorderWindow::OnFreezeChange);
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

#ifdef mqabcDebug
    {
        MQFrame* vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        m_button_debug = CreateButton(vf, L"Debug");
        m_button_debug->AddClickEvent(this, &mqabcRecorderWindow::OnDebugClicked);
    }
#endif // mqabcDebug

    this->AddHideEvent(this, &mqabcRecorderWindow::OnHide);

    SyncSettings();
}

BOOL mqabcRecorderWindow::OnHide(MQWidgetBase* sender, MQDocument doc)
{
    m_plugin->CloseABC();
    m_plugin->WindowClose();
    return 0;
}

BOOL mqabcRecorderWindow::OnIntervalChange(MQWidgetBase* sender, MQDocument doc)
{
    auto str = mu::ToMBS(m_edit_interval->GetText());
    auto value = std::atof(str.c_str());
    m_plugin->SetInterval(value);
    m_slider_interval->SetPosition(value);
    return 0;
}

BOOL mqabcRecorderWindow::OnIntervalSlide(MQWidgetBase* sender, MQDocument doc)
{
    double value = m_slider_interval->GetPosition();
    m_plugin->SetInterval(value);

    const size_t buf_len = 128;
    wchar_t buf[buf_len];
    swprintf(buf, buf_len, L"%.2lf", m_plugin->GetInterval());
    m_edit_interval->SetText(buf);
    return 0;
}

BOOL mqabcRecorderWindow::OnScaleChange(MQWidgetBase* sender, MQDocument doc)
{
    auto str = mu::ToMBS(m_edit_scale->GetText());
    auto value = std::atof(str.c_str());
    if (value != 0.0) {
        auto& settings = m_plugin->GetSettings();
        settings.scale_factor = (float)value;
    }
    return 0;
}

BOOL mqabcRecorderWindow::OnFreezeChange(MQWidgetBase* sender, MQDocument doc)
{
    auto& settings = m_plugin->GetSettings();
    settings.freeze_mirror = m_check_freeze_mirror->GetChecked();
    settings.freeze_lathe = m_check_freeze_lathe->GetChecked();
    settings.freeze_subdiv = m_check_freeze_subdiv->GetChecked();
    return 0;
}

BOOL mqabcRecorderWindow::OnRecordingClicked(MQWidgetBase* sender, MQDocument doc)
{
    if (!m_plugin->IsArchiveOpened()) {
        // show file open directory

        MQSaveFileDialog dlg(*this);
        dlg.AddFilter(L"Alembic file (*.abc)|*.abc");
        dlg.SetDefaultExt(L"abc");

        auto& mqo_path = m_plugin->GetMQOPath();
        auto dir = mu::GetDirectory(mqo_path.c_str());
        auto filename = mu::GetFilename_NoExtension(mqo_path.c_str());
        if (filename.empty())
            filename = "Untitled";
        filename += ".abc";

        if (!dir.empty())
            dlg.SetInitialDir(mu::ToWCS(dir));
        dlg.SetFileName(mu::ToWCS(filename));

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

#ifdef mqabcDebug
BOOL mqabcRecorderWindow::OnDebugClicked(MQWidgetBase* sender, MQDocument doc)
{
    m_plugin->DbgDoSomething();
    return 0;
}
#endif // mqabcDebug

void mqabcRecorderWindow::SyncSettings()
{
    const size_t buf_len = 128;
    wchar_t buf[buf_len];
    auto& settings = m_plugin->GetSettings();

    swprintf(buf, buf_len, L"%.2lf", m_plugin->GetInterval());
    m_edit_interval->SetText(buf);
    m_slider_interval->SetPosition(m_plugin->GetInterval());

    swprintf(buf, buf_len, L"%.3f", settings.scale_factor);
    m_edit_scale->SetText(buf);

    m_check_freeze_mirror->SetChecked(settings.freeze_mirror);
    m_check_freeze_lathe->SetChecked(settings.freeze_lathe);
    m_check_freeze_subdiv->SetChecked(settings.freeze_subdiv);
}

void mqabcRecorderWindow::LogInfo(const char* message)
{
    // todo
}

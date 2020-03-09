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
        m_frame_settings = vf;

        {
            MQFrame* hf = CreateHorizontalFrame(vf);
            CreateLabel(hf, L"Scale Factor");
            m_edit_scale = CreateEdit(hf);
            m_edit_scale->SetNumeric(MQEdit::NUMERIC_DOUBLE);
            m_edit_scale->AddChangedEvent(this, &mqabcRecorderWindow::OnScaleChange);
        }

        m_check_normals = CreateCheckBox(vf, L"Capture Normals");
        m_check_normals->AddChangedEvent(this, &mqabcRecorderWindow::OnSettingsUpdate);

        m_check_colors = CreateCheckBox(vf, L"Capture Vertex Colors");
        m_check_colors->AddChangedEvent(this, &mqabcRecorderWindow::OnSettingsUpdate);

        m_check_mids = CreateCheckBox(vf, L"Capture Material IDs");
        m_check_mids->AddChangedEvent(this, &mqabcRecorderWindow::OnSettingsUpdate);

        m_check_mirror = CreateCheckBox(vf, L"Freeze Mirror");
        m_check_mirror->AddChangedEvent(this, &mqabcRecorderWindow::OnSettingsUpdate);

        m_check_lathe = CreateCheckBox(vf, L"Freeze Lathe");
        m_check_lathe->AddChangedEvent(this, &mqabcRecorderWindow::OnSettingsUpdate);

        m_check_subdiv = CreateCheckBox(vf, L"Freeze Subdiv");
        m_check_subdiv->AddChangedEvent(this, &mqabcRecorderWindow::OnSettingsUpdate);
    }

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
        }

        m_button_recording = CreateButton(vf, L"Start Recording");
        m_button_recording->AddClickEvent(this, &mqabcRecorderWindow::OnRecordingClicked);
    }

    {
        MQFrame* vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        m_log = CreateMemo(vf);
        m_log->SetHorzBarStatus(MQMemo::SCROLLBAR_OFF);
        m_log->SetVertBarStatus(MQMemo::SCROLLBAR_OFF);

        std::string plugin_version = "Plugin Version: " mqabcVersionString;
        CreateLabel(vf, mu::ToWCS(plugin_version));
    }

//#ifdef mqabcDebug
//    {
//        MQFrame* vf = CreateVerticalFrame(this);
//        vf->SetOutSpace(outer_margin);
//        vf->SetInSpace(inner_margin);
//
//        m_button_debug = CreateButton(vf, L"Debug");
//        m_button_debug->AddClickEvent(this, &mqabcRecorderWindow::OnDebugClicked);
//    }
//#endif // mqabcDebug

    this->AddShowEvent(this, &mqabcRecorderWindow::OnShow);
    this->AddHideEvent(this, &mqabcRecorderWindow::OnHide);
}

BOOL mqabcRecorderWindow::OnShow(MQWidgetBase* sender, MQDocument doc)
{
    SyncSettings();
    return 0;
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

BOOL mqabcRecorderWindow::OnSettingsUpdate(MQWidgetBase* sender, MQDocument doc)
{
    auto& settings = m_plugin->GetSettings();
    settings.freeze_mirror = m_check_mirror->GetChecked();
    settings.freeze_lathe = m_check_lathe->GetChecked();
    settings.freeze_subdiv = m_check_subdiv->GetChecked();
    settings.capture_normals = m_check_normals->GetChecked();
    settings.capture_colors = m_check_colors->GetChecked();
    settings.capture_material_ids = m_check_mids->GetChecked();
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
                m_plugin->CaptureFrame(doc);
            }
        }
    }
    else {
        if (m_plugin->CloseABC()) {
        }
    }
    SyncSettings();

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

    swprintf(buf, buf_len, L"%.3f", settings.scale_factor);
    m_edit_scale->SetText(buf);

    m_check_mirror->SetChecked(settings.freeze_mirror);
    m_check_lathe->SetChecked(settings.freeze_lathe);
    m_check_subdiv->SetChecked(settings.freeze_subdiv);

    m_check_normals->SetChecked(settings.capture_normals);
    m_check_colors->SetChecked(settings.capture_colors);
    m_check_mids->SetChecked(settings.capture_material_ids);

    if (m_plugin->IsRecording()) {
        SetBackColor(MQCanvasColor(255, 0, 0));
        m_button_recording->SetText(L"Stop Recording");
        m_frame_settings->SetEnabled(false);
    }
    else {
        SetBackColor(GetDefaultBackColor());
        m_button_recording->SetText(L"Start Recording");
        m_frame_settings->SetEnabled(true);
    }
}

void mqabcRecorderWindow::LogInfo(const char* message)
{
    if (m_log) {
        m_log->SetText(mu::ToWCS(message));
    }
}

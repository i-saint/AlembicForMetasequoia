#include "pch.h"
#include "mqabc.h"
#include "mqabcPlayerPlugin.h"
#include "mqabcPlayerWindow.h"

mqabcPlayerWindow::mqabcPlayerWindow(mqabcPlayerPlugin* plugin, MQWindowBase& parent)
    : MQWindow(parent)
{
    setlocale(LC_ALL, "");

    m_plugin = plugin;

    SetTitle(L"Alembic Player");
    SetOutSpace(0.4);

    double outer_margin = 0.2;
    double inner_margin = 0.1;


    {
        MQFrame* vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);
        m_frame_open = vf;

        m_button_open = CreateButton(vf, L"Open");
        m_button_open->AddClickEvent(this, &mqabcPlayerWindow::OnOpenClicked);
    }
    {
        MQFrame* vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);
        vf->SetVisible(false);
        m_frame_time = vf;

        MQFrame* hf = CreateHorizontalFrame(vf);
        CreateLabel(hf, L"Capture Interval (second):");

        m_edit_sample = CreateEdit(hf);
        m_edit_sample->SetNumeric(MQEdit::NUMERIC_INT);
        m_edit_sample->SetText(L"0");
        m_edit_sample->AddChangedEvent(this, &mqabcPlayerWindow::OnSampleEdit);

        m_slider_sample = CreateSlider(vf);
        m_slider_sample->AddChangingEvent(this, &mqabcPlayerWindow::OnSampleSlide);
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
    this->AddHideEvent(this, &mqabcPlayerWindow::OnHide);
}

BOOL mqabcPlayerWindow::OnHide(MQWidgetBase* sender, MQDocument doc)
{
    m_plugin->CloseABC();
    m_plugin->WindowClose();

    m_frame_open->SetVisible(true);
    m_frame_time->SetVisible(false);
    return 0;
}

BOOL mqabcPlayerWindow::OnOpenClicked(MQWidgetBase* sender, MQDocument doc)
{
    if (!m_plugin->IsArchiveOpened()) {
        MQOpenFileDialog dlg(*this);
        dlg.AddFilter(L"Alembic file (*.abc)|*.abc");
        dlg.SetDefaultExt(L"abc");
        if (dlg.Execute()) {
            auto path = dlg.GetFileName();
            if (m_plugin->OpenABC(mu::ToMBS(path))) {
                m_slider_sample->SetMin(0);
                m_slider_sample->SetMax((double)m_plugin->GetSampleCount());
                m_slider_sample->SetPosition(0);

                m_frame_open->SetVisible(false);
                m_frame_time->SetVisible(true);
                m_plugin->Seek(0);
            }
        }
    }
    else {
    }

    return 0;
}

BOOL mqabcPlayerWindow::OnSampleEdit(MQWidgetBase* sender, MQDocument doc)
{
    auto str = mu::ToMBS(m_edit_sample->GetText());
    auto value = std::atoi(str.c_str());
    m_slider_sample->SetPosition(value);

    m_plugin->Seek(value);
    return 0;
}

BOOL mqabcPlayerWindow::OnSampleSlide(MQWidgetBase* sender, MQDocument doc)
{
    const size_t buf_len = 128;
    wchar_t buf[buf_len];
    auto value = (int)m_slider_sample->GetPosition();
    swprintf(buf, buf_len, L"%d", (int)value);
    m_edit_sample->SetText(buf);

    m_plugin->Seek(value);
    return 0;
}

void mqabcPlayerWindow::LogInfo(const char* message)
{
    // todo
}

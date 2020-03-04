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

        m_button_open = CreateButton(vf, L"Open");
        m_button_open->AddClickEvent(this, &mqabcPlayerWindow::OnOpenClicked);
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
    this->AddHideEvent(this, &mqabcPlayerWindow::OnHide);
}

BOOL mqabcPlayerWindow::OnHide(MQWidgetBase* sender, MQDocument doc)
{
    m_plugin->CloseABC();
    m_plugin->WindowClose();
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
                 // todo
            }
        }
    }
    else {
    }

    return 0;
}

void mqabcPlayerWindow::LogInfo(const char* message)
{
    // todo
}

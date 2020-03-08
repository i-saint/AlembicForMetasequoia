#include "pch.h"
#include "mqabc.h"
#include "mqabcPlayerPlugin.h"
#include "mqabcPlayerWindow.h"

static mqabcPlayerPlugin g_plugin;

// Constructor
// コンストラクタ
mqabcPlayerPlugin::mqabcPlayerPlugin()
{
}

// Destructor
// デストラクタ
mqabcPlayerPlugin::~mqabcPlayerPlugin()
{
}

#if defined(__APPLE__) || defined(__linux__)
// Create a new plugin class for another document.
// 別のドキュメントのための新しいプラグインクラスを作成する。
MQBasePlugin* mqabcPlayerPlugin::CreateNewPlugin()
{
    return new mqabcPlayerPlugin();
}
#endif

//---------------------------------------------------------------------------
//  GetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::GetPlugInID(DWORD *Product, DWORD *ID)
{
    // プロダクト名(制作者名)とIDを、全部で64bitの値として返す
    // 値は他と重複しないようなランダムなもので良い
    *Product = mqabcPluginProduct;
    *ID = mqabcPlayerPluginID;
}

//---------------------------------------------------------------------------
//  GetPlugInName
//    プラグイン名を返す。
//    この関数は[プラグインについて]表示時に呼び出される。
//---------------------------------------------------------------------------
const char *mqabcPlayerPlugin::GetPlugInName(void)
{
    return "Alembic Player (version " mqabcVersionString ") " mqabcCopyRight;
}

//---------------------------------------------------------------------------
//  EnumString
//    ポップアップメニューに表示される文字列を返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
#if MQPLUGIN_VERSION >= 0x0470
const wchar_t *mqabcPlayerPlugin::EnumString(void)
{
    return L"Alembic Player";
}
#else
const char *mqabcPlayerPlugin::EnumString(void)
{
    return "Alembic Player";
}
#endif



//---------------------------------------------------------------------------
//  EnumSubCommand
//    サブコマンド前を列挙
//---------------------------------------------------------------------------
const char *mqabcPlayerPlugin::EnumSubCommand(int index)
{
    return NULL;
}

//---------------------------------------------------------------------------
//  GetSubCommandString
//    サブコマンドの文字列を列挙
//---------------------------------------------------------------------------
const wchar_t *mqabcPlayerPlugin::GetSubCommandString(int index)
{
    return NULL;
}

//---------------------------------------------------------------------------
//  Initialize
//    アプリケーションの初期化
//---------------------------------------------------------------------------
BOOL mqabcPlayerPlugin::Initialize()
{
    if (!m_window) {
        auto parent = MQWindow::GetMainWindow();
        m_window = new mqabcPlayerWindow(this, parent);
    }
    return TRUE;
}

//---------------------------------------------------------------------------
//  Exit
//    アプリケーションの終了
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::Exit()
{
    if (m_window) {
        delete m_window;
        m_window = nullptr;
    }
    CloseABC();
}

//---------------------------------------------------------------------------
//  Activate
//    表示・非表示切り替え要求
//---------------------------------------------------------------------------
BOOL mqabcPlayerPlugin::Activate(MQDocument doc, BOOL flag)
{
    if (!m_window) {
        return FALSE;
    }

    bool active = flag ? true : false;
    m_window->SetVisible(active);
    return active;
}

//---------------------------------------------------------------------------
//  IsActivated
//    表示・非表示状態の返答
//---------------------------------------------------------------------------
BOOL mqabcPlayerPlugin::IsActivated(MQDocument doc)
{
    if (!m_window) {
        return FALSE;
    }
    return m_window->GetVisible();
}

//---------------------------------------------------------------------------
//  OnMinimize
//    ウインドウの最小化への返答
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnMinimize(MQDocument doc, BOOL flag)
{
}

//---------------------------------------------------------------------------
//  OnReceiveUserMessage
//    プラグイン独自のメッセージの受け取り
//---------------------------------------------------------------------------
int mqabcPlayerPlugin::OnReceiveUserMessage(MQDocument doc, DWORD src_product, DWORD src_id, const char *description, void *message)
{
    return 0;
}

//---------------------------------------------------------------------------
//  OnSubCommand
//    A message for calling a sub comand
//    サブコマンドの呼び出し
//---------------------------------------------------------------------------
BOOL mqabcPlayerPlugin::OnSubCommand(MQDocument doc, int index)
{
    return FALSE;
}

//---------------------------------------------------------------------------
//  OnDraw
//    描画時の処理
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnDraw(MQDocument doc, MQScene scene, int width, int height)
{
}


//---------------------------------------------------------------------------
//  OnNewDocument
//    ドキュメント初期化時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnNewDocument(MQDocument doc, const char *filename, NEW_DOCUMENT_PARAM& param)
{
}

//---------------------------------------------------------------------------
//  OnEndDocument
//    ドキュメント終了時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnEndDocument(MQDocument doc)
{
    CloseABC();
}

//---------------------------------------------------------------------------
//  OnSaveDocument
//    ドキュメント保存時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnSaveDocument(MQDocument doc, const char *filename, SAVE_DOCUMENT_PARAM& param)
{
}

//---------------------------------------------------------------------------
//  OnUndo
//    アンドゥ実行時
//---------------------------------------------------------------------------
BOOL mqabcPlayerPlugin::OnUndo(MQDocument doc, int undo_state)
{
    return TRUE;
}

//---------------------------------------------------------------------------
//  OnRedo
//    リドゥ実行時
//---------------------------------------------------------------------------
BOOL mqabcPlayerPlugin::OnRedo(MQDocument doc, int redo_state)
{
    return TRUE;
}

//---------------------------------------------------------------------------
//  OnUpdateUndo
//    アンドゥ状態更新時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnUpdateUndo(MQDocument doc, int undo_state, int undo_size)
{
}

//---------------------------------------------------------------------------
//  OnObjectModified
//    オブジェクトの編集時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnObjectModified(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnObjectSelected
//    オブジェクトの選択状態の変更時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnObjectSelected(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnUpdateObjectList
//    カレントオブジェクトの変更時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnUpdateObjectList(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnMaterialModified
//    マテリアルのパラメータ変更時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnMaterialModified(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnUpdateMaterialList
//    カレントマテリアルの変更時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnUpdateMaterialList(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnUpdateScene
//    シーン情報の変更時
//---------------------------------------------------------------------------
void mqabcPlayerPlugin::OnUpdateScene(MQDocument doc, MQScene scene)
{
}

//---------------------------------------------------------------------------
//  ExecuteCallback
//    コールバックに対する実装部
//---------------------------------------------------------------------------
bool mqabcPlayerPlugin::ExecuteCallback(MQDocument doc, void *option)
{
    CallbackInfo *info = (CallbackInfo*)option;
    return ((*this).*info->proc)(doc);
}

// コールバックの呼び出し
void mqabcPlayerPlugin::Execute(ExecuteCallbackProc proc)
{
    CallbackInfo info;
    info.proc = proc;
    BeginCallback(&info);
}


bool mqabcPlayerPlugin::OpenABC(const std::string& path)
{
    try
    {
        // Abc::IArchive doesn't accept wide string path. so create file stream with wide string path and pass it.
        // (VisualC++'s std::ifstream accepts wide string)
#ifdef WIN32
        auto wpath = mu::ToWCS(path);
        m_stream.open(wpath.c_str(), std::ios::in | std::ios::binary);
#else
        m_stream.open(path.c_str(), std::ios::in | std::ios::binary);
#endif
        if (!m_stream.is_open())
            return false;

        std::vector< std::istream*> streams{ &m_stream };
        Alembic::AbcCoreOgawa::ReadArchive archive_reader(streams);
        m_archive = Abc::IArchive(archive_reader(path), Abc::kWrapExisting, Abc::ErrorHandler::kThrowPolicy);
        m_abc_path = path;

        m_top_node = new TopNode(m_archive.getTop());
        ConstructTree(m_top_node);

        for (auto mesh : m_mesh_nodes)
            m_sample_count = std::max(m_sample_count, (int64_t)mesh->sample_count);
    }
    catch (Alembic::Util::Exception e)
    {
        //LogInfo("Failed (%s)", e.what());
        return false;
    }

    return true;
}

bool mqabcPlayerPlugin::CloseABC()
{
    if (!m_archive)
        return false;

    m_top_node = nullptr;
    m_mesh_nodes.clear();
    m_nodes.clear();

    m_archive.reset();
    m_stream.close();
    m_abc_path.clear();

    m_sample_count = m_sample_index = 0;
    m_mqobj_id = 0;

    return true;
}

void mqabcPlayerPlugin::ConstructTree(Node* n)
{
    m_nodes.push_back(NodePtr(n));

    auto& abc = n->abcobj;
    size_t nchildren = abc.getNumChildren();
    for (size_t ci = 0; ci < nchildren; ++ci) {
        auto cabc = abc.getChild(ci);

        const auto& metadata = cabc.getMetaData();
        Node* c = nullptr;
        if (AbcGeom::IXformSchema::matches(metadata)) {
            c = new XformNode(n, cabc);
        }
        else if (AbcGeom::IPolyMeshSchema::matches(metadata)) {
            c = new MeshNode(n, cabc);
            m_mesh_nodes.push_back((MeshNode*)c);
        }
        else {
            c = new Node(n, cabc);
        }
        ConstructTree(c);
    }
}

void mqabcPlayerPlugin::Seek(int64_t i)
{
    if (!m_archive)
        return;

    m_sample_index = i;
    Execute(&mqabcPlayerPlugin::DoSeek);
}

void mqabcPlayerPlugin::Refresh()
{
    if (!m_archive)
        return;

    Execute(&mqabcPlayerPlugin::DoSeek);
}

bool mqabcPlayerPlugin::DoSeek(MQDocument doc)
{
    // read abc
    m_top_node->update(m_sample_index);
    mu::parallel_for_each(m_mesh_nodes.begin(), m_mesh_nodes.end(), [this](MeshNode* n) {
        n->convert(m_settings);
    });

    // build merged mesh
    m_mesh_merged.clear();
    for (auto n : m_mesh_nodes)
        m_mesh_merged.merge(n->mesh);
    m_mesh_merged.clearInvalidComponent();


    // update mq object
    auto obj = doc->GetObjectFromUniqueID(m_mqobj_id);
    if (!obj) {
        obj = MQ_CreateObject();
        doc->AddObject(obj);
        m_mqobj_id = obj->GetUniqueID();

        auto name = mu::GetFilename_NoExtension(m_abc_path.c_str());
        obj->SetName(name.c_str());
    }
    obj->Clear();

    // add points
    auto& data = m_mesh_merged;
    for (auto& p : data.points)
        obj->AddVertex((MQPoint&)p);

    // add faces
    {
        size_t ii = 0;
        for (auto c : data.counts) {
            obj->AddFace(c, &data.indices[ii]);
            ii += c;
        }
    }

    // todo

    return true;
}

mqabcPlayerSettings& mqabcPlayerPlugin::GetSettings()
{
    return m_settings;
}

bool mqabcPlayerPlugin::IsArchiveOpened() const
{
    return m_archive;
}

int64_t mqabcPlayerPlugin::GetSampleCount() const
{
    return m_sample_count;
}

void mqabcPlayerPlugin::LogInfo(const char *message)
{
    if (m_window)
        m_window->LogInfo(message);
}


MQBasePlugin* GetPluginClass()
{
    return &g_plugin;
}

void msmqLogInfo(const char *message)
{
    g_plugin.LogInfo(message);
}

#ifdef _WIN32
//---------------------------------------------------------------------------
//  DllMain
//---------------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE hInstance,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        HRESULT hRes = ::CoInitialize(NULL);

        return SUCCEEDED(hRes);
    }

    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        ::CoUninitialize();
    }

    return TRUE;
}
#endif



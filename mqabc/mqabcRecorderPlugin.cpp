#include "pch.h"
#include "mqabc.h"
#include "mqabcRecorderPlugin.h"
#include "mqabcRecorderWindow.h"

static mqabcRecorderPlugin g_plugin;

// Constructor
// コンストラクタ
mqabcRecorderPlugin::mqabcRecorderPlugin()
{
}

// Destructor
// デストラクタ
mqabcRecorderPlugin::~mqabcRecorderPlugin()
{
}

#if defined(__APPLE__) || defined(__linux__)
// Create a new plugin class for another document.
// 別のドキュメントのための新しいプラグインクラスを作成する。
MQBasePlugin* mqabcRecorderPlugin::CreateNewPlugin()
{
    return new mqabcRecorderPlugin();
}
#endif

//---------------------------------------------------------------------------
//  GetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::GetPlugInID(DWORD *Product, DWORD *ID)
{
    // プロダクト名(制作者名)とIDを、全部で64bitの値として返す
    // 値は他と重複しないようなランダムなもので良い
    *Product = mqabcPluginProduct;
    *ID = mqabcRecorderPluginID;
}

//---------------------------------------------------------------------------
//  GetPlugInName
//    プラグイン名を返す。
//    この関数は[プラグインについて]表示時に呼び出される。
//---------------------------------------------------------------------------
const char *mqabcRecorderPlugin::GetPlugInName(void)
{
    return "Alembic Recorder (version " mqabcVersionString ") " mqabcCopyRight;
}

//---------------------------------------------------------------------------
//  EnumString
//    ポップアップメニューに表示される文字列を返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
#if MQPLUGIN_VERSION >= 0x0470
const wchar_t *mqabcRecorderPlugin::EnumString(void)
{
    return L"Alembic Recorder";
}
#else
const char *mqabcRecorderPlugin::EnumString(void)
{
    return "Alembic Recorder";
}
#endif



//---------------------------------------------------------------------------
//  EnumSubCommand
//    サブコマンド前を列挙
//---------------------------------------------------------------------------
const char *mqabcRecorderPlugin::EnumSubCommand(int index)
{
    return NULL;
}

//---------------------------------------------------------------------------
//  GetSubCommandString
//    サブコマンドの文字列を列挙
//---------------------------------------------------------------------------
const wchar_t *mqabcRecorderPlugin::GetSubCommandString(int index)
{
    return NULL;
}

//---------------------------------------------------------------------------
//  Initialize
//    アプリケーションの初期化
//---------------------------------------------------------------------------
BOOL mqabcRecorderPlugin::Initialize()
{
    if (!m_window) {
        auto parent = MQWindow::GetMainWindow();
        m_window = new mqabcRecorderWindow(this, parent);
    }
    return TRUE;
}

//---------------------------------------------------------------------------
//  Exit
//    アプリケーションの終了
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::Exit()
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
BOOL mqabcRecorderPlugin::Activate(MQDocument doc, BOOL flag)
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
BOOL mqabcRecorderPlugin::IsActivated(MQDocument doc)
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
void mqabcRecorderPlugin::OnMinimize(MQDocument doc, BOOL flag)
{
}

//---------------------------------------------------------------------------
//  OnReceiveUserMessage
//    プラグイン独自のメッセージの受け取り
//---------------------------------------------------------------------------
int mqabcRecorderPlugin::OnReceiveUserMessage(MQDocument doc, DWORD src_product, DWORD src_id, const char *description, void *message)
{
    return 0;
}

//---------------------------------------------------------------------------
//  OnSubCommand
//    A message for calling a sub comand
//    サブコマンドの呼び出し
//---------------------------------------------------------------------------
BOOL mqabcRecorderPlugin::OnSubCommand(MQDocument doc, int index)
{
    return FALSE;
}

//---------------------------------------------------------------------------
//  OnDraw
//    描画時の処理
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnDraw(MQDocument doc, MQScene scene, int width, int height)
{
    Flush();
}


//---------------------------------------------------------------------------
//  OnNewDocument
//    ドキュメント初期化時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnNewDocument(MQDocument doc, const char *filename, NEW_DOCUMENT_PARAM& param)
{
    m_mqo_path = filename ? filename : "";
    MarkSceneDirty();
}

//---------------------------------------------------------------------------
//  OnEndDocument
//    ドキュメント終了時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnEndDocument(MQDocument doc)
{
    m_mqo_path.clear();
    CloseABC();
}

//---------------------------------------------------------------------------
//  OnSaveDocument
//    ドキュメント保存時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnSaveDocument(MQDocument doc, const char *filename, SAVE_DOCUMENT_PARAM& param)
{
    m_mqo_path = filename ? filename : "";
}

//---------------------------------------------------------------------------
//  OnUndo
//    アンドゥ実行時
//---------------------------------------------------------------------------
BOOL mqabcRecorderPlugin::OnUndo(MQDocument doc, int undo_state)
{
    MarkSceneDirty();
    return TRUE;
}

//---------------------------------------------------------------------------
//  OnRedo
//    リドゥ実行時
//---------------------------------------------------------------------------
BOOL mqabcRecorderPlugin::OnRedo(MQDocument doc, int redo_state)
{
    MarkSceneDirty();
    return TRUE;
}

//---------------------------------------------------------------------------
//  OnUpdateUndo
//    アンドゥ状態更新時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnUpdateUndo(MQDocument doc, int undo_state, int undo_size)
{
    MarkSceneDirty();
}

//---------------------------------------------------------------------------
//  OnObjectModified
//    オブジェクトの編集時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnObjectModified(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnObjectSelected
//    オブジェクトの選択状態の変更時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnObjectSelected(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnUpdateObjectList
//    カレントオブジェクトの変更時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnUpdateObjectList(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnMaterialModified
//    マテリアルのパラメータ変更時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnMaterialModified(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnUpdateMaterialList
//    カレントマテリアルの変更時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnUpdateMaterialList(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnUpdateScene
//    シーン情報の変更時
//---------------------------------------------------------------------------
void mqabcRecorderPlugin::OnUpdateScene(MQDocument doc, MQScene scene)
{
}

//---------------------------------------------------------------------------
//  ExecuteCallback
//    コールバックに対する実装部
//---------------------------------------------------------------------------
bool mqabcRecorderPlugin::ExecuteCallback(MQDocument doc, void *option)
{
    CallbackInfo *info = (CallbackInfo*)option;
    return ((*this).*info->proc)(doc);
}

// コールバックの呼び出し
void mqabcRecorderPlugin::Execute(ExecuteCallbackProc proc)
{
    CallbackInfo info;
    info.proc = proc;
    BeginCallback(&info);
}


bool mqabcRecorderPlugin::OpenABC(const std::string& path)
{
    try
    {
        m_archive = Abc::OArchive(Alembic::AbcCoreOgawa::WriteArchive(), path);
        m_abc_path = path;
    }
    catch (Alembic::Util::Exception e)
    {
        //LogInfo("Failed (%s)", e.what());
        return false;
    }

    // add dummy time sampling
    auto ts = Abc::TimeSampling(Abc::chrono_t(1.0f / 30.0f), Abc::chrono_t(0.0));
    auto tsi = m_archive.addTimeSampling(ts);

    // create nodes
    auto node_name = mu::GetFilename_NoExtension(m_mqo_path.c_str());
    if (node_name.empty())
        node_name = mu::GetFilename_NoExtension(m_abc_path.c_str());
    if (node_name.empty())
        node_name = "Untitled";

    m_root_node.reset(new AbcGeom::OObject(m_archive, AbcGeom::kTop, tsi));
    m_xform_node.reset(new AbcGeom::OXform(*m_root_node, node_name, tsi));
    m_mesh_node.reset(new AbcGeom::OPolyMesh(*m_xform_node, node_name + "_Mesh", tsi));

    auto props = m_mesh_node->getSchema().getArbGeomParams();
    m_colors_param = AbcGeom::OC4fGeomParam(props, "rgba", false, AbcGeom::GeometryScope::kFacevaryingScope, 1, tsi);
    m_recording = true;

    return true;
}

bool mqabcRecorderPlugin::CloseABC()
{
    if (!m_archive)
        return false;

    if (m_task_write.valid())
        m_task_write.wait();

    auto ts = Abc::TimeSampling(Abc::TimeSamplingType(Abc::TimeSamplingType::kAcyclic), m_timeline);
    *m_archive.getTimeSampling(1) = ts;

    m_colors_param.reset();
    m_mesh_node.reset();
    m_xform_node.reset();
    m_root_node.reset();
    m_archive.reset(); // flush archive

    m_start_time = m_last_flush = 0;
    m_timeline.clear();
    m_abc_path.clear();
    m_recording = false;

    return true;
}

const std::string& mqabcRecorderPlugin::GetMQOPath() const
{
    return m_mqo_path;
}
const std::string& mqabcRecorderPlugin::GetABCPath() const
{
    return m_abc_path;
}

bool mqabcRecorderPlugin::IsArchiveOpened() const
{
    return m_archive;
}

bool mqabcRecorderPlugin::IsRecording() const
{
    return m_recording && m_archive;
}
void mqabcRecorderPlugin::SetRecording(bool v)
{
    m_recording = v;
}

void mqabcRecorderPlugin::SetInterval(double v)
{
    m_interval = mu::S2NS(v);
}
double mqabcRecorderPlugin::GetInterval() const
{
    return mu::NS2Sd(m_interval);
}

void mqabcRecorderPlugin::SetScaleFactor(float v)
{
    m_scale_factor = v;
}
float mqabcRecorderPlugin::GetScaleFactor()
{
    return m_scale_factor;
}
void mqabcRecorderPlugin::SetTimeScale(float v)
{
    m_time_scale = v;
}
float mqabcRecorderPlugin::GetTimeScale()
{
    return m_time_scale;
}


void mqabcRecorderPlugin::LogInfo(const char *message)
{
    if (m_window)
        m_window->LogInfo(message);
}

void mqabcRecorderPlugin::MarkSceneDirty()
{
    m_dirty = true;
}

void mqabcRecorderPlugin::Flush()
{
    if (!IsRecording() || !m_dirty)
        return;

    auto t = mu::Now();
    if (t - m_last_flush < m_interval)
        return;

    m_dirty = false;
    m_last_flush = t;
    if (m_start_time == 0)
        m_start_time = t;

    Execute(&mqabcRecorderPlugin::CaptureFrame);
}

bool mqabcRecorderPlugin::CaptureFrame(MQDocument doc)
{
    if (m_task_write.valid())
        m_task_write.wait();

    // do extract mesh data
    int nobjects = doc->GetObjectCount();
    m_obj_records.resize(nobjects);
    for (int oi = 0; oi < nobjects; ++oi) {
        auto& rec = m_obj_records[oi];
        rec.mqdocument = doc;
        rec.mqobject = doc->GetObject(oi);
    }

    mu::parallel_for(0, nobjects, [this](int oi) {
        ExtractMeshData(m_obj_records[oi]);
    });

    // flush abc
    auto abctime = mu::NS2Sd(m_last_flush - m_start_time);
    m_task_write = std::async(std::launch::async, [this, abctime]() { FlushABC(abctime); });

    return true;
}

void mqabcRecorderPlugin::ExtractMeshData(ObjectRecord& rec)
{
    int nfaces = rec.mqobject->GetFaceCount();
    int npoints = rec.mqobject->GetVertexCount();
    int nindices = 0;
    for (int fi = 0; fi < nfaces; ++fi)
        nindices += rec.mqobject->GetFacePointCount(fi);

    auto& dst = rec.mesh;
    dst.resize(npoints, nindices, nfaces);

    auto obj = rec.mqobject;
    auto dst_points = dst.points.data();
    auto dst_normals = dst.normals.data();
    auto dst_uv = dst.uv.data();
    auto dst_colors = dst.colors.data();
    auto dst_mids = dst.material_ids.data();
    auto dst_counts = dst.counts.data();
    auto dst_indices = dst.indices.data();

    rec.name = obj->GetName();

    // points
    obj->GetVertexArray((MQPoint*)dst_points);
    mu::Scale(dst_points, m_scale_factor, npoints);

    for (int fi = 0; fi < nfaces; ++fi) {
        // counts
        int count = obj->GetFacePointCount(fi);
        dst_counts[fi] = count;

        // material IDs
        dst_mids[fi] = obj->GetFaceMaterial(fi);

        // indices
        obj->GetFacePointArray(fi, dst_indices);
        dst_indices += count;

        // uv
        obj->GetFaceCoordinateArray(fi, (MQCoordinate*)dst_uv);
        dst_uv += count;

        for (int ci = 0; ci < count; ++ci) {
            // vertex color
            *(dst_colors++) = mu::Color32ToFloat4(obj->GetFaceVertexColor(fi, ci));

#if MQPLUGIN_VERSION >= 0x0460
            // normal
            BYTE flags;
            obj->GetFaceVertexNormal(fi, ci, flags, (MQPoint&)*(dst_normals++));
#endif
        }
    }
}

void mqabcRecorderPlugin::FlushABC(abcChrono t)
{
    // make merged mesh
    m_mesh_merged.clear();
    for (auto& rec : m_obj_records)
        m_mesh_merged.merge(rec.mesh);


    // write to abc
    const auto& data = m_mesh_merged;

    m_mesh_sample.setFaceIndices(Abc::Int32ArraySample(data.indices.cdata(), data.indices.size()));
    m_mesh_sample.setFaceCounts(Abc::Int32ArraySample(data.counts.cdata(), data.counts.size()));
    m_mesh_sample.setPositions(Abc::P3fArraySample((const abcV3*)data.points.cdata(), data.points.size()));

    m_sample_normals.setVals(Abc::V3fArraySample((const abcV3*)data.normals.cdata(), data.normals.size()));
    m_mesh_sample.setNormals(m_sample_normals);

    m_sample_uv.setVals(Abc::V2fArraySample((const abcV2*)data.uv.cdata(), data.uv.size()));
    m_mesh_sample.setUVs(m_sample_uv);

    m_mesh_node->getSchema().set(m_mesh_sample);

    m_sample_colors.setVals(Abc::C4fArraySample((const abcC4*)data.colors.cdata(), data.colors.size()));
    m_colors_param.set(m_sample_colors);


    m_xform_node->getSchema().set(m_xform_sample);

    m_timeline.push_back(t * m_time_scale);
}


MQBasePlugin* GetPluginClass()
{
    return &g_plugin;
}

void msmqLogInfo(const char *message)
{
    g_plugin.LogInfo(message);
}

#ifdef mqabcDebug
void mqabcRecorderPlugin::DbgDoSomething()
{
    Execute(&mqabcRecorderPlugin::DbgDoSomethingImpl);
}

bool mqabcRecorderPlugin::DbgDoSomethingImpl(MQDocument doc)
{
    return false;
}
#endif // mqabcDebug


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

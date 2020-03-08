#include "pch.h"
#include "mqabc.h"
#include "mqabcRecorderPlugin.h"
#include "mqabcRecorderWindow.h"


bool mqabcRecorderPlugin::OpenABC(const std::string& path)
{
    try
    {
        m_archive = Abc::OArchive(Alembic::AbcCoreOgawa::WriteArchive(), path);
        m_abc_path = path;
    }
    catch (Alembic::Util::Exception e)
    {
        LogInfo("failed to open %s\nexception: %s", path.c_str(), e.what());
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

    auto geom_params = m_mesh_node->getSchema().getArbGeomParams();
    if (m_settings.capture_colors) {
        m_colors_param = AbcGeom::OC4fGeomParam(geom_params, mqabcVertexColorPropName, false, AbcGeom::GeometryScope::kFacevaryingScope, 1, tsi);
    }
    if (m_settings.capture_material_ids) {
        m_mids_param = AbcGeom::OInt32ArrayProperty(geom_params, mqabcMaterialIDPropName, tsi);
    }
    m_timeline.reserve(1024);
    m_recording = true;

    CaptureFrame();

    LogInfo("succeeded to open %s\nrecording started", m_abc_path.c_str());
    return true;
}

bool mqabcRecorderPlugin::CloseABC()
{
    if (!m_archive)
        return false;

    WaitFlush();

    if (m_settings.keep_time) {
        auto ts = Abc::TimeSampling(Abc::TimeSamplingType(Abc::TimeSamplingType::kAcyclic), m_timeline);
        *m_archive.getTimeSampling(1) = ts;
    }

    m_colors_param.reset();
    m_mids_param.reset();
    m_mesh_node.reset();
    m_xform_node.reset();
    m_root_node.reset();
    m_archive.reset(); // flush archive

    m_start_time = m_last_flush = 0;
    m_timeline.clear();
    m_abc_path.clear();
    m_recording = false;

    LogInfo("recording finished");
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

mqabcRecorderSettings& mqabcRecorderPlugin::GetSettings()
{
    return m_settings;
}


void mqabcRecorderPlugin::MarkSceneDirty()
{
    m_dirty = true;
}

void mqabcRecorderPlugin::CaptureFrame()
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

    Execute(&mqabcRecorderPlugin::DoCaptureFrame);
}

bool mqabcRecorderPlugin::DoCaptureFrame(MQDocument doc)
{
    WaitFlush();

    // prepare
    int nobjects = doc->GetObjectCount();
    m_obj_records.resize(nobjects);
    for (int oi = 0; oi < nobjects; ++oi) {
        auto& rec = m_obj_records[oi];
        rec.mqdocument = doc;

        auto obj = doc->GetObject(oi);
        if ((obj->GetMirrorType() != MQOBJECT_MIRROR_NONE && m_settings.freeze_mirror) ||
            (obj->GetLatheType() != MQOBJECT_LATHE_NONE && m_settings.freeze_lathe) ||
            (obj->GetPatchType() != MQOBJECT_PATCH_NONE && m_settings.freeze_subdiv))
        {
            rec.mqobject = obj->Clone();

            DWORD freeze_flags = 0;
            if (m_settings.freeze_mirror)
                freeze_flags |= MQOBJECT_FREEZE_MIRROR;
            if (m_settings.freeze_lathe)
                freeze_flags |= MQOBJECT_FREEZE_LATHE;
            if (m_settings.freeze_subdiv)
                freeze_flags |= MQOBJECT_FREEZE_PATCH;
            rec.mqobject->Freeze(freeze_flags);

            rec.need_release = true;
        }
        else {
            rec.mqobject = obj;
        }
    }

    // extract mesh data
    mu::parallel_for(0, nobjects, [this](int oi) {
        ExtractMeshData(m_obj_records[oi]);
    });

    auto abctime = mu::NS2Sd(m_last_flush - m_start_time) * m_time_scale;
    m_timeline.push_back(abctime);

    // flush abc async
    m_task_write = std::async(std::launch::async, [this, abctime]() { FlushABC(); });

    // log
    int total_vertices = 0;
    int total_faces = 0;
    for (auto& rec : m_obj_records) {
        total_vertices += (int)rec.mesh.points.size();
        total_faces += (int)rec.mesh.counts.size();
    }
    LogInfo("frame %d: %d vertices, %d faces",
        (int)(m_timeline.size() - 1), total_vertices, total_faces);

    return true;
}

void mqabcRecorderPlugin::ExtractMeshData(ObjectRecord& rec)
{
    auto obj = rec.mqobject;
    int nfaces = obj->GetFaceCount();
    int npoints = obj->GetVertexCount();
    int nindices = 0;
    for (int fi = 0; fi < nfaces; ++fi)
        nindices += obj->GetFacePointCount(fi);

    auto& dst = rec.mesh;
    dst.resize(npoints, nindices, nfaces);

    auto dst_points = dst.points.data();
    auto dst_normals = dst.normals.data();
    auto dst_uvs = dst.uvs.data();
    auto dst_colors = dst.colors.data();
    auto dst_mids = dst.material_ids.data();
    auto dst_counts = dst.counts.data();
    auto dst_indices = dst.indices.data();

    rec.name = obj->GetName();

    // points
    obj->GetVertexArray((MQPoint*)dst_points);
    mu::Scale(dst_points, m_scale_factor, npoints);

    int fc = 0; // 'actual' face count
    for (int fi = 0; fi < nfaces; ++fi) {
        // counts
        // GetFacePointCount() may return 0 for unknown reason. skip it.
        int count = obj->GetFacePointCount(fi);
        if (count == 0)
            continue;
        dst_counts[fc] = count;

        // material IDs
        dst_mids[fc] = obj->GetFaceMaterial(fi);

        // indices
        obj->GetFacePointArray(fi, dst_indices);
        dst_indices += count;

        // uv
        obj->GetFaceCoordinateArray(fi, (MQCoordinate*)dst_uvs);
        dst_uvs += count;

        for (int ci = 0; ci < count; ++ci) {
            // vertex color
            *(dst_colors++) = mu::Color32ToFloat4(obj->GetFaceVertexColor(fi, ci));

#if MQPLUGIN_VERSION >= 0x0460
            // normal
            BYTE flags;
            obj->GetFaceVertexNormal(fi, ci, flags, (MQPoint&)*(dst_normals++));
#endif
        }

        ++fc;
    }
    mu::InvertV(dst.uvs.data(), dst.uvs.size());

    if (nfaces != fc) {
        // refit
        dst.counts.resize(fc);
        dst.material_ids.resize(fc);
    }
}

void mqabcRecorderPlugin::FlushABC()
{
    // make merged mesh
    m_mesh_merged.clear();
    for (auto& rec : m_obj_records)
        m_mesh_merged.merge(rec.mesh);

    // write to abc
    const auto& data = m_mesh_merged;

    m_mesh_sample.reset();
    m_mesh_sample.setFaceIndices(Abc::Int32ArraySample(data.indices.cdata(), data.indices.size()));
    m_mesh_sample.setFaceCounts(Abc::Int32ArraySample(data.counts.cdata(), data.counts.size()));
    m_mesh_sample.setPositions(Abc::P3fArraySample((const abcV3*)data.points.cdata(), data.points.size()));
#if MQPLUGIN_VERSION >= 0x0460
    if (m_settings.capture_normals) {
        m_sample_normals.setVals(Abc::V3fArraySample((const abcV3*)data.normals.cdata(), data.normals.size()));
        m_mesh_sample.setNormals(m_sample_normals);
    }
#endif
    if (m_settings.capture_uvs) {
        m_sample_uv.setVals(Abc::V2fArraySample((const abcV2*)data.uvs.cdata(), data.uvs.size()));
        m_mesh_sample.setUVs(m_sample_uv);
    }
    m_mesh_node->getSchema().set(m_mesh_sample);

    if (m_settings.capture_colors) {
        m_sample_colors.setVals(Abc::C4fArraySample((const abcC4*)data.colors.cdata(), data.colors.size()));
        m_colors_param.set(m_sample_colors);
    }
    if (m_settings.capture_material_ids) {
        m_mids_param.set(AbcGeom::Int32ArraySample(data.material_ids.cdata(), data.material_ids.size()));
    }

    m_xform_node->getSchema().set(m_xform_sample);
}

void mqabcRecorderPlugin::WaitFlush()
{
    if (!m_task_write.valid())
        return;

    m_task_write.wait();
    m_task_write = {};

    for (auto& rec : m_obj_records) {
        if (rec.need_release) {
            rec.need_release = false;
            rec.mqobject->DeleteThis();
            rec.mqobject = nullptr;
        }
    }

}


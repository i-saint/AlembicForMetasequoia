#include "pch.h"
#include "mqabc.h"
#include "mqabcPlayerPlugin.h"


mqabcPlayerPlugin::Node::Node(Node* p, Abc::IObject abc)
    : parent(p)
    , abcobj(abc)
{
    if (parent)
        parent->children.push_back(this);
}

mqabcPlayerPlugin::Node::~Node()
{
}

mqabcPlayerPlugin::Node::Type mqabcPlayerPlugin::Node::getType() const
{
    return Type::Unknown;
}

void mqabcPlayerPlugin::Node::update(int64_t si)
{
    for (auto child : children)
        child->update(si);
}

template<class NodeT>
NodeT* mqabcPlayerPlugin::Node::findParent()
{
    for (Node* p = parent; p != nullptr; p = p->parent) {
        if (p->getType() == NodeT::node_type)
            return (NodeT*)p;
    }
    return nullptr;
}


mqabcPlayerPlugin::TopNode::TopNode(Abc::IObject abc)
    : super(nullptr, abc)
{
}

mqabcPlayerPlugin::Node::Type mqabcPlayerPlugin::TopNode::getType() const
{
    return Type::Top;
}


mqabcPlayerPlugin::XformNode::XformNode(Node* p, Abc::IObject abc)
    : super(p, abc)
{
    auto so = AbcGeom::IXform(abc, Abc::kWrapExisting);
    schema = so.getSchema();
    parent_xform = findParent<XformNode>();
}

mqabcPlayerPlugin::Node::Type mqabcPlayerPlugin::XformNode::getType() const
{
    return Type::Xform;
}

void mqabcPlayerPlugin::XformNode::update(int64_t si)
{
    Abc::ISampleSelector iss(si);
    AbcGeom::XformSample sample;
    schema.get(sample, iss);

    auto matd = sample.getMatrix();
    local_matrix.assign((double4x4&)matd);

    if (parent_xform)
        global_matrix = local_matrix * parent_xform->global_matrix;
    else
        global_matrix = local_matrix;

    super::update(si);
}


mqabcPlayerPlugin::MeshNode::MeshNode(Node* p, Abc::IObject abc)
    : super(p, abc)
{
    auto so = AbcGeom::IPolyMesh(abc, Abc::kWrapExisting);
    schema = so.getSchema();
    parent_xform = findParent<XformNode>();
    sample_count = schema.getNumSamples();

    auto geom_params = schema.getArbGeomParams();
    if (geom_params.valid())
    {
        // find vertex color and material ids params
        size_t num_geom_params = geom_params.getNumProperties();
        for (size_t i = 0; i < num_geom_params; ++i)
        {
            auto& header = geom_params.getPropertyHeader(i);

            // vertex color
            if (AbcGeom::IC4fGeomParam::matches(header))
                rgba_param = AbcGeom::IC4fGeomParam(geom_params, header.getName());
            if (AbcGeom::IC3fGeomParam::matches(header))
                rgb_param = AbcGeom::IC3fGeomParam(geom_params, header.getName());

            // material ids
            if (AbcGeom::IInt32ArrayProperty::matches(header) && header.getName() == mqabcMaterialIDPropName)
                mids_prop = AbcGeom::IInt32ArrayProperty(geom_params, header.getName());
        }
    }
}

mqabcPlayerPlugin::Node::Type mqabcPlayerPlugin::MeshNode::getType() const
{
    return Type::PolyMesh;
}


void mqabcPlayerPlugin::MeshNode::update(int64_t si)
{
    Abc::ISampleSelector iss(si);

    mesh.clear();

    {
        AbcGeom::IPolyMeshSchema::Sample sample;
        schema.get(sample, iss);

        auto counts = sample.getFaceCounts();
        mesh.counts.assign(counts->get(), counts->size());

        auto indices = sample.getFaceIndices();
        mesh.indices.assign(indices->get(), indices->size());

        auto points = sample.getPositions();
        mesh.points.assign((float3*)points->get(), points->size());
    }

    auto get_index_params = [this, iss](auto& abcparam, auto& dst) -> bool {
        if (!abcparam.valid() || abcparam.getNumSamples() == 0)
            return false;

        using src_t = std::remove_reference_t<decltype(abcparam)>::value_type;
        using dst_t = std::remove_reference_t<decltype(dst)>::value_type;
        static_assert(sizeof(src_t) == sizeof(dst_t), "data size mismatch");

        auto sample = abcparam.getExpandedValue(iss);
        const auto& values = *sample.getVals();

        size_t nindices = mesh.indices.size();
        const auto* src = (const dst_t*)values.get();
        if (values.size() == nindices) {
            dst.assign(src, values.size());
            return true;
        }
        else if (values.size() == mesh.points.size()) {
            const auto* indices = mesh.indices.cdata();
            dst.resize_discard(nindices);
            for (size_t i = 0; i < nindices; ++i)
                dst[i] = src[indices[i]];
            return true;
        }
        else {
            return false;
        }
    };

    // normals
    get_index_params(schema.getNormalsParam(), mesh.normals);

    // uvs
    get_index_params(schema.getUVsParam(), mesh.uvs);
    mu::InvertV(mesh.uvs.data(), mesh.uvs.size());

    // colors
    if (!get_index_params(rgba_param, mesh.colors)) {
        // convert rgb to rgba if the param exists
        if (get_index_params(rgb_param, tmp_rgb)) {
            size_t size = tmp_rgb.size();
            mesh.colors.resize_discard(size);
            auto src = tmp_rgb.cdata();
            auto dst = mesh.colors.data();
            for (size_t ii = 0; ii < size; ++ii)
                *(dst++) = to_vec4(*(src++), 1.0f);
        }
    }

    // material ids
    if (mids_prop.valid()) {
        Abc::Int32ArraySamplePtr sp;
        mids_prop.get(sp, iss);
        mesh.material_ids.assign(sp->get(), sp->size());
    }

    // validate
    mesh.clearInvalidComponent();

    super::update(si);
}

void mqabcPlayerPlugin::MeshNode::convert(const mqabcPlayerSettings& settings)
{
    if (parent_xform) {
        mesh.transform(parent_xform->global_matrix);
    }

    if (settings.scale_factor != 1.0f) {
        mu::Scale(mesh.points.data(), settings.scale_factor, mesh.points.size());
    }
    if (settings.flip_x) {
        mu::InvertX(mesh.points.data(), mesh.points.size());
        mu::InvertX(mesh.normals.data(), mesh.normals.size());
    }
    if (settings.flip_yz) {
        auto convert = [this](auto& v) { return flip_z(swap_yz(v)); };

        for (auto& v : mesh.points) v = convert(v);
        for (auto& v : mesh.normals) v = convert(v);
    }
    if (settings.flip_faces) {
        size_t nfaces = mesh.counts.size();
        int* indices = mesh.indices.data();
        int* counts = mesh.counts.data();
        for (size_t fi = 0; fi < nfaces; ++fi) {
            int c = *counts;
            std::reverse(indices, indices + c);
            indices += c;
            ++counts;
        }
    }
}



bool mqabcPlayerPlugin::OpenABC(const std::string& path)
{
    try
    {
        // Abc::IArchive doesn't accept wide string path. so create file stream with wide string path and pass it.
        // (VisualC++'s std::ifstream accepts wide string)
        m_stream.reset(new std::fstream());
#ifdef WIN32
        auto wpath = mu::ToWCS(path);
        m_stream->open(wpath.c_str(), std::ios::in | std::ios::binary);
#else
        m_stream->open(path.c_str(), std::ios::in | std::ios::binary);
#endif
        if (!m_stream->is_open()) {
            CloseABC();
            return false;
        }

        std::vector< std::istream*> streams{ m_stream.get() };
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
        CloseABC();
        return false;
    }

    return true;
}

bool mqabcPlayerPlugin::CloseABC()
{
    m_top_node = nullptr;
    m_mesh_nodes.clear();
    m_nodes.clear();

    m_archive.reset();
    m_stream.reset();
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


    auto& data = m_mesh_merged;
    int nfaces = (int)data.counts.size();
    int npoints = (int)data.points.size();
    int nindices = (int)data.indices.size();

    // points
    {
        for (auto& p : data.points)
            obj->AddVertex((MQPoint&)p);
    }

    // faces
    {
        int ii = 0;
        auto* indices = (int*)data.indices.cdata();
        for (auto c : data.counts) {
            obj->AddFace(c, indices);
            indices += c;
        }
    }

    // uvs
    if (!data.uvs.empty()) {
        auto* uvs = (MQCoordinate*)data.uvs.cdata();
        for (int fi = 0; fi < nfaces; ++fi) {
            int c = data.counts[fi];
            obj->SetFaceCoordinateArray(fi, uvs);
            uvs += c;
        }
    }

    // normals
#if MQPLUGIN_VERSION >= 0x0460
    if (!data.normals.empty()) {
        auto* normals = (const MQPoint*)data.normals.cdata();
        for (int fi = 0; fi < nfaces; ++fi) {
            int c = data.counts[fi];
            for (int ci = 0; ci < c; ++ci)
                obj->SetFaceVertexNormal(fi, ci, MQOBJECT_NORMAL_NONE, normals[ci]);
            normals += c;
        }
    }
#endif

    // colors
    if (!data.colors.empty()) {
        auto* colors = data.colors.cdata();
        for (int fi = 0; fi < nfaces; ++fi) {
            int c = data.counts[fi];
            for (int ci = 0; ci < c; ++ci)
                obj->SetFaceVertexColor(fi, ci, mu::Float4ToColor32(colors[ci]));
            colors += c;
        }
    }

    // material ids
    if (!data.material_ids.empty()) {
        auto* material_ids = data.material_ids.cdata();
        for (int fi = 0; fi < nfaces; ++fi)
            obj->SetFaceMaterial(fi, material_ids[fi]);
    }

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

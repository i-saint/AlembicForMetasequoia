#include "pch.h"
#include "mqabc.h"
#include "mqabcPlayerPlugin.h"


mqabcPlayerPlugin::Node::Node(Node* p, Abc::IObject abc)
    : parent(p)
    , abcobj(abc)
{
    name = abc.getName();
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
    auto so = AbcGeom::IXform(abc);
    schema = so.getSchema();
    parent_xform = findParent<XformNode>();
}

mqabcPlayerPlugin::Node::Type mqabcPlayerPlugin::XformNode::getType() const
{
    return Type::Xform;
}

void mqabcPlayerPlugin::XformNode::update(int64_t si)
{
    if (schema.valid()) {
        Abc::ISampleSelector iss(si);
        AbcGeom::XformSample sample;
        schema.get(sample, iss);

        auto matd = sample.getMatrix();
        local_matrix.assign((double4x4&)matd);

        if (parent_xform)
            global_matrix = local_matrix * parent_xform->global_matrix;
        else
            global_matrix = local_matrix;
    }

    super::update(si);
}


mqabcPlayerPlugin::MeshNode::MeshNode(Node* p, Abc::IObject abc)
    : super(p, abc)
{
    auto so = AbcGeom::IPolyMesh(abc);
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
    updateMeshData(si);
    super::update(si);
}

void mqabcPlayerPlugin::MeshNode::updateMeshData(int64_t si)
{
    mesh.clear();

    Abc::ISampleSelector iss(si);
    AbcGeom::IPolyMeshSchema::Sample sample;
    schema.get(sample, iss);
    if (!sample.valid())
        return;

    auto counts = sample.getFaceCounts();
    mesh.counts.assign(counts->get(), counts->size());

    auto indices = sample.getFaceIndices();
    mesh.indices.assign(indices->get(), indices->size());

    auto points = sample.getPositions();
    mesh.points.assign((float3*)points->get(), points->size());


    auto get_index_params = [this, iss](auto abcparam, auto& dst) -> bool {
        if (!abcparam.valid() || abcparam.getNumSamples() == 0)
            return false;

        using src_t = typename std::remove_reference_t<decltype(abcparam)>::value_type;
        using dst_t = typename std::remove_reference_t<decltype(dst)>::value_type;
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
}

void mqabcPlayerPlugin::MeshNode::convert(const mqabcPlayerSettings& settings)
{
    if (parent_xform)
        mesh.applyTransform(parent_xform->global_matrix);
    mesh.applyScale(settings.scale_factor);
    if (settings.flip_x)
        mesh.flipX();
    if (settings.flip_yz)
        mesh.flipYZ();
    if (settings.flip_faces)
        mesh.flipFaces();
}


mqabcPlayerPlugin::MaterialNode::MaterialNode(Node* p, Abc::IObject abc)
    : super(p, abc)
{
    auto so = AbcMaterial::IMaterial(abc);
    schema = so.getSchema();

    // parse properties
    std::vector<std::string> shaders;
    schema.getShaderTypesForTarget(mqabcMtlTarget, shaders);
    if (!shaders.empty()) {
        auto params = schema.getShaderParameters(mqabcMtlTarget, shaders.front());

        auto find_param = [this, &params](const char* name, auto& dst) -> bool {
            using prop_t = typename std::remove_reference_t<decltype(dst)>;

            auto header = params.getPropertyHeader(name);
            if (header && prop_t::matches(*header)) {
                dst = prop_t(params, header->getName());
                return true;
            }
            return false;
        };

        find_param(mqabcMtlUseVertexColor, use_vertex_color_prop);
        find_param(mqabcMtlDoubleSided, double_sided_prop);
        find_param(mqabcMtlDiffuseColor, color_prop);
        find_param(mqabcMtlDiffuse, diffuse_prop);
        find_param(mqabcMtlAlpha, alpha_prop);
        find_param(mqabcMtlAmbientColor, ambient_prop);
        find_param(mqabcMtlSpecularColor, specular_prop);
        find_param(mqabcMtlEmissionColor, emission_prop);
    }

    if (valid()) {
        // assume all properties are constant and so get values at this point.

        material.name = abc.getName();

        const auto& shader = shaders.front();
#define Case(MQ, ABC) if (shader == ABC) material.shader = MQ;
        Case(MQMATERIAL_SHADER_CLASSIC, mqabcMtlShaderClassic)
            Case(MQMATERIAL_SHADER_CONSTANT, mqabcMtlShaderConstant)
            Case(MQMATERIAL_SHADER_LAMBERT, mqabcMtlShaderLambert)
            Case(MQMATERIAL_SHADER_PHONG, mqabcMtlShaderPhong)
            Case(MQMATERIAL_SHADER_BLINN, mqabcMtlShaderBlinn)
            Case(MQMATERIAL_SHADER_HLSL, mqabcMtlShaderHLSL)
#undef Case

        Abc::ISampleSelector iss(int64_t(0));
        static_assert(sizeof(bool) == sizeof(Abc::bool_t), "");
        use_vertex_color_prop.get((Abc::bool_t&)material.use_vertex_color, iss);
        double_sided_prop.get((Abc::bool_t&)material.double_sided, iss);
        color_prop.get((abcC3&)material.color, iss);
        diffuse_prop.get(material.diffuse, iss);
        alpha_prop.get(material.alpha, iss);
        ambient_prop.get((abcC3&)material.ambient_color, iss);
        specular_prop.get((abcC3&)material.specular_color, iss);
        emission_prop.get((abcC3&)material.emission_color, iss);
    }
}

mqabcPlayerPlugin::Node::Type mqabcPlayerPlugin::MaterialNode::getType() const
{
    return Type::Material;
}

void mqabcPlayerPlugin::MaterialNode::update(int64_t si)
{
    // nothing to do for now
}

bool mqabcPlayerPlugin::MaterialNode::valid() const
{
    return
        use_vertex_color_prop.valid() &&
        double_sided_prop.valid();
        color_prop.valid() &&
        diffuse_prop.valid() &&
        alpha_prop.valid() &&
        ambient_prop.valid() &&
        specular_prop.valid() &&
        emission_prop.valid();
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
    }
    catch (Alembic::Util::Exception e)
    {
        CloseABC();

#ifdef mqabcEnableHDF5
        try
        {
            m_archive = Abc::IArchive(AbcCoreHDF5::ReadArchive(), path);
        }
        catch (Alembic::Util::Exception e2)
        {
            CloseABC();
            LogInfo(
                "failed to open %s\n"
                "it may not an alembic file"
                , path.c_str());
        }
#else
        LogInfo(
            "failed to open %s\n"
            "it may not an alembic file or not in Ogawa format (HDF5 is not supported)"
            , path.c_str());
#endif
    }

    if (m_archive.valid()) {
        try {
            m_abc_path = path;
            m_top_node = new TopNode(m_archive.getTop());
            ConstructTree(m_top_node);

            for (auto mesh : m_mesh_nodes)
                m_sample_count = std::max(m_sample_count, (int64_t)mesh->sample_count);
        }
        catch (Alembic::Util::Exception e3) {
            CloseABC();
            LogInfo(
                "failed to read %s\n"
                "it seems an alembic file but probably broken"
                , path.c_str());
        }
    }

    if (!m_archive.valid())
        return false;

    return true;
}

bool mqabcPlayerPlugin::CloseABC()
{
    m_top_node = nullptr;
    m_mesh_nodes.clear();
    m_material_nodes.clear();
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
            auto mn = new MeshNode(n, cabc);
            m_mesh_nodes.push_back(mn);
            c = mn;
        }
        else if (AbcMaterial::IMaterialSchema::matches(metadata)) {
            auto mn = new MaterialNode(n, cabc);
            if (mn->valid())
                m_material_nodes.push_back(mn);
            c = mn;
        }
        else {
            c = new Node(n, cabc);
        }
        ConstructTree(c);
    }
}

void mqabcPlayerPlugin::ImportMaterials(MQDocument doc)
{
    int nmaterials = (int)m_material_nodes.size();
    for (int mi = 0; mi < nmaterials; ++mi) {
        auto& src= m_material_nodes[mi]->material;
        MQMaterial mqmat = nullptr;
        if (mi < doc->GetMaterialCount()) {
            mqmat = doc->GetMaterial(mi);
            mqmat->SetName(src.name.c_str());
        }
        else {
            mqmat = MQ_CreateMaterial();
            mqmat->SetName(src.name.c_str());
            doc->AddMaterial(mqmat);
        }
        mqmat->SetShader(src.shader);
        mqmat->SetVertexColor(src.use_vertex_color ? MQMATERIAL_VERTEXCOLOR_DIFFUSE : MQMATERIAL_VERTEXCOLOR_DISABLE);
        mqmat->SetDoubleSided(src.double_sided);
        mqmat->SetColor((MQColor&)src.color);
        mqmat->SetDiffuse(src.diffuse);
        mqmat->SetAlpha(src.alpha);
        mqmat->SetAmbientColor((MQColor&)src.ambient_color);
        mqmat->SetSpecularColor((MQColor&)src.specular_color);
        mqmat->SetEmissionColor((MQColor&)src.emission_color);
    }
}

void mqabcPlayerPlugin::Seek(MQDocument doc, int64_t i)
{
    if (!m_archive)
        return;

    m_sample_index = i;

    // read abc
    m_top_node->update(i);
    mu::parallel_for_each(m_mesh_nodes.begin(), m_mesh_nodes.end(), [this](MeshNode* n) {
        n->convert(m_settings);
    });

    // build merged mesh
    auto& mesh = m_mesh_merged;
    mesh.clear();
    for (auto n : m_mesh_nodes)
        mesh.merge(n->mesh);
    mesh.clearInvalidComponent();

    // reserve materials
    {
        int mi = 0;
        int nmaterials = mesh.getMaxMaterialID();
        while (doc->GetMaterialCount() <= nmaterials) {
            const size_t buf_len = 128;
            wchar_t buf[buf_len];
            swprintf(buf, buf_len, L"abcmat%d", mi++);

            auto mat = MQ_CreateMaterial();
            mat->SetName(buf);

            doc->AddMaterial(mat);
        }
    }


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


    int nfaces = (int)mesh.counts.size();

    // points
    {
        for (auto& p : mesh.points)
            obj->AddVertex((MQPoint&)p);
    }

    // faces
    {
        int ii = 0;
        auto* data = (int*)mesh.indices.cdata();
        for (auto c : mesh.counts) {
            obj->AddFace(c, data);
            data += c;
        }
    }

    // uvs
    if (!mesh.uvs.empty()) {
        auto* data = (MQCoordinate*)mesh.uvs.cdata();
        for (int fi = 0; fi < nfaces; ++fi) {
            int c = mesh.counts[fi];
            obj->SetFaceCoordinateArray(fi, data);
            data += c;
        }
    }

    // normals
#if MQPLUGIN_VERSION >= 0x0460
    if (!mesh.normals.empty()) {
        auto* data = (const MQPoint*)mesh.normals.cdata();
        for (int fi = 0; fi < nfaces; ++fi) {
            int c = mesh.counts[fi];
            for (int ci = 0; ci < c; ++ci)
                obj->SetFaceVertexNormal(fi, ci, MQOBJECT_NORMAL_NONE, data[ci]);
            data += c;
        }
    }
#endif

    // colors
    if (!mesh.colors.empty()) {
        auto* data = mesh.colors.cdata();
        for (int fi = 0; fi < nfaces; ++fi) {
            int c = mesh.counts[fi];
            for (int ci = 0; ci < c; ++ci)
                obj->SetFaceVertexColor(fi, ci, mu::Float4ToColor32(data[ci]));
            data += c;
        }
    }

    // material ids
    if (!mesh.material_ids.empty()) {
        auto* data = mesh.material_ids.cdata();
        for (int fi = 0; fi < nfaces; ++fi)
            obj->SetFaceMaterial(fi, data[fi]);
    }

    // repaint
    MQ_RefreshView(nullptr);
}

void mqabcPlayerPlugin::Refresh(MQDocument doc)
{
    Seek(doc, m_sample_index);
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

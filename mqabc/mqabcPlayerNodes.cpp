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

void mqabcPlayerPlugin::Node::update(abcChrono time)
{
    for (auto child : children)
        child->update(time);
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

void mqabcPlayerPlugin::XformNode::update(abcChrono time)
{
    Abc::ISampleSelector iss(time);
    AbcGeom::XformSample sample;
    schema.get(sample, iss);

    auto matd = sample.getMatrix();
    local_matrix.assign((double4x4&)matd);

    if (parent_xform)
        global_matrix = parent_xform->global_matrix * local_matrix;
    else
        global_matrix = local_matrix;

    super::update(time);
}


mqabcPlayerPlugin::MeshNode::MeshNode(Node* p, Abc::IObject abc)
    : super(p, abc)
{
    auto so = AbcGeom::IPolyMesh(abc, Abc::kWrapExisting);
    schema = so.getSchema();
    parent_xform = findParent<XformNode>();

    auto geom_params = schema.getArbGeomParams();
}

mqabcPlayerPlugin::Node::Type mqabcPlayerPlugin::MeshNode::getType() const
{
    return Type::PolyMesh;
}


void mqabcPlayerPlugin::MeshNode::update(abcChrono time)
{
    Abc::ISampleSelector iss(time);

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

    auto get_param_type = [this, iss](auto& abcparam, auto& dst) {
        if (abcparam.getNumSamples() == 0)
            return;

        using src_t = std::remove_reference_t<decltype(abcparam)>::value_type;
        using dst_t = std::remove_reference_t<decltype(dst)>::value_type;
        static_assert(sizeof(src_t) == sizeof(dst_t), "data size mismatch");

        auto sample = abcparam.getExpandedValue(iss);
        const auto& values = *sample.getVals();

        size_t nindices = mesh.indices.size();
        const auto* src = (const dst_t*)values.get();
        if (values.size() == nindices) {
            dst.assign(src, values.size());
        }
        else if (values.size() == mesh.points.size()) {
            const auto* indices = mesh.indices.cdata();
            dst.resize_discard(nindices);
            for (size_t i = 0; i < nindices; ++i)
                dst[i] = src[indices[i]];
        }
    };

    get_param_type(schema.getNormalsParam(), mesh.normals);
    get_param_type(schema.getUVsParam(), mesh.uvs);

    mesh.clearInvalidComponent();

    super::update(time);
}

void mqabcPlayerPlugin::MeshNode::applyTransform()
{
    if (parent_xform)
        mesh.transform(parent_xform->global_matrix);
}

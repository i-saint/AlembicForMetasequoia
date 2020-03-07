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
}

mqabcPlayerPlugin::Node::Type mqabcPlayerPlugin::XformNode::getType() const
{
    return Type::Xform;
}

void mqabcPlayerPlugin::XformNode::update(abcChrono time)
{
    // todo

    super::update(time);
}


mqabcPlayerPlugin::MeshNode::MeshNode(Node* p, Abc::IObject abc)
    : super(p, abc)
{
}

mqabcPlayerPlugin::Node::Type mqabcPlayerPlugin::MeshNode::getType() const
{
    return Type::PolyMesh;
}

void mqabcPlayerPlugin::MeshNode::update(abcChrono time)
{
    // todo

    super::update(time);
}

void mqabcPlayerPlugin::MeshNode::applyTransform()
{
    // find xform
    XformNode* xform = nullptr;
    for (Node* p = parent; p != nullptr; p = p->parent) {
        if (p->getType() == Node::Type::Xform) {
            xform = (XformNode*)p;
            break;
        }
    }

    // apply transform
    if (xform)
        mesh.transform(xform->global_matrix);
}

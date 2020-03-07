#include "pch.h"
#include "mqabcMesh.h"

void mqabcMesh::clear()
{
    points.clear();
    normals.clear();
    uvs.clear();
    colors.clear();
    material_ids.clear();
    counts.clear();
    indices.clear();
}

void mqabcMesh::resize(int npoints, int nindices, int nfaces)
{
    points.resize_discard(npoints);
    normals.resize_discard(nindices);
    uvs.resize_discard(nindices);
    colors.resize_discard(nindices);
    material_ids.resize_discard(nfaces);
    counts.resize_discard(nfaces);
    indices.resize_discard(nindices);
}

void mqabcMesh::transform(const float4x4 v)
{
    mu::MulPoints(v, points.cdata(), points.data(), points.size());
    mu::MulVectors(v, normals.cdata(), normals.data(), normals.size());
}

void mqabcMesh::merge(const mqabcMesh& v)
{
    auto append = [](auto& dst, const auto& src) {
        dst.insert(dst.end(), src.cdata(), src.cdata() + src.size());
    };

    int vertex_offset = (int)points.size();
    int index_offset = (int)indices.size();

    append(points, v.points);
    append(normals, v.normals);
    append(uvs, v.uvs);
    append(colors, v.colors);
    append(material_ids, v.material_ids);
    append(counts, v.counts);
    append(indices, v.indices);

    if (vertex_offset > 0) {
        int index_end = index_offset + (int)v.indices.size();
        for (int ii = index_offset; ii < index_end; ++ii)
            indices[ii] += vertex_offset;
    }
}

void mqabcMesh::clearInvalidComponent()
{
    auto nindices = indices.size();
    auto nfaces = counts.size();

    if (!normals.empty() && normals.size() != nindices)
        normals.clear();
    if (!uvs.empty() && uvs.size() != nindices)
        uvs.clear();
    if (!colors.empty() && colors.size() != nindices)
        colors.clear();
    if (!material_ids.empty() && material_ids.size() != nfaces)
        material_ids.clear();
}

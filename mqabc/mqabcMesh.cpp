#include "pch.h"
#include "mqabcMesh.h"

void mqabcMesh::resize(int npoints, int nindices, int nfaces)
{
    points.resize_discard(npoints);
    normals.resize_discard(nindices);
    uv.resize_discard(nindices);
    colors.resize_discard(nindices);
    material_ids.resize_discard(nfaces);
    counts.resize_discard(nfaces);
    indices.resize_discard(nindices);
}

void mqabcMesh::transform(const float4x4 v)
{
}

#pragma once
#include "mqabc.h"


class mqabcMesh
{
public:
    std::string name;
    RawVector<float3> points;    // per-vertex
    RawVector<float3> normals;   // per-index
    RawVector<float2> uv;        // 
    RawVector<float4> colors;    // 
    RawVector<int> material_ids; // per-face
    RawVector<int> counts;       // 
    RawVector<int> indices;

    void clear();
    void resize(int npoints, int nindices, int nfaces);
    void transform(const float4x4 v);
    void merge(const mqabcMesh& other);
};

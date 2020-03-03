#pragma once
#include "mqabc.h"


class mqabcMesh
{
public:
    std::string name;
    RawVector<float3> points;
    RawVector<float3> normals;
    RawVector<float2> uv;
    RawVector<float4> colors;
    RawVector<int> material_ids;
};

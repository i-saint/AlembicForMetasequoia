#pragma once

#define mqabcVersion        0.1.0
#define mqabcVersionString  "0.1.0"

#define mqabcPluginProduct      0x493ADF11
#define mqabcRecorderPluginID   0xB1CC9999
#define mqabcPlayerPluginID     0xB1CC999A

#ifdef _WIN32
    #define mqabcAPI extern "C" __declspec(dllexport)
#else
    #define mqabcAPI extern "C" 
#endif

#include "MeshUtils/MeshUtils.h"
using mu::float2;
using mu::float3;
using mu::float4;
using mu::float2x2;
using mu::float3x3;
using mu::float4x4;

using namespace Alembic;
using abcV2 = Imath::V2f;
using abcV3 = Imath::V3f;
using abcV3i = Imath::V3i;
using abcV3d = Imath::V3d;
using abcV4 = Imath::V4f;
using abcC3 = Imath::C3f;
using abcC4 = Imath::C4f;
using abcM44 = Imath::M44f;
using abcM44d = Imath::M44d;
using abcBox = Imath::Box3f;
using abcBoxd = Imath::Box3d;
using abcChrono = Abc::chrono_t;
using abcSampleSelector = Abc::ISampleSelector;

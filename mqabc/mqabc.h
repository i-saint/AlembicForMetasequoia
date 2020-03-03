#pragma once

#define mqabcVersion 0.1.0
#define mqabcVersionString "0.1.0"

#define MQPluginProduct 0x493ADF11
#define MQPluginID 0xB1CC9999

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

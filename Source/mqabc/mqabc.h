#pragma once

#define mqabcVersion        0.9.0
#define mqabcVersionString  "0.9.0"
#define mqabcCopyRight      "Copyright(C) 2020, i-saint"

#define mqabcPluginProduct      0x493ADF11
#define mqabcRecorderPluginID   0xB1CC9999
#define mqabcPlayerPluginID     0xB1CC999A

#define mqabcVertexColorPropName    "C"
#define mqabcMaterialIDPropName     "MID"

#define mqabcMtlTarget          "metasequoia"
#define mqabcMtlShaderClassic   "classic"
#define mqabcMtlShaderConstant  "constant"
#define mqabcMtlShaderLambert   "lambert"
#define mqabcMtlShaderPhong     "phong"
#define mqabcMtlShaderBlinn     "blinn"
#define mqabcMtlShaderHLSL      "hlsl"

#define mqabcMtlUseVertexColor  ".useVertexColor"
#define mqabcMtlDoubleSided     ".doubleSided"
#define mqabcMtlDiffuseColor    ".diffuseColor"
#define mqabcMtlDiffuse         ".diffuse"
#define mqabcMtlAlpha           ".alpha"
#define mqabcMtlAmbientColor    ".ambientColor"
#define mqabcMtlSpecularColor   ".specularColor"
#define mqabcMtlEmissionColor   ".emissionColor"

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
using mu::double4x4;

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

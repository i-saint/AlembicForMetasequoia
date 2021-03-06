#pragma once

#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #define _WINDOWS
    #include <windows.h>
    #include <d3d11.h>
    #include <d3d11_4.h>
    #include <d3d12.h>
    #include <dxgi1_4.h>
    #include <dxgiformat.h>
    #include <dxcapi.h>
    #include <comdef.h>
#endif // _WIN32

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <fstream>
#include <future>

#define mqabcEnableHDF5

#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreOgawa/All.h>
#ifdef mqabcEnableHDF5
    #include <Alembic/AbcCoreHDF5/All.h>
#endif
#include <Alembic/Abc/ErrorHandler.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcMaterial/All.h>

#include "mqsdk/sdk_Include.h"

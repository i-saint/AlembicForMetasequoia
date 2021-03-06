#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "mqabc.h"

#ifdef _WIN32
    #pragma comment(lib, "Half-2_4.lib")
    #pragma comment(lib, "Iex-2_4.lib")
    #pragma comment(lib, "IexMath-2_4.lib")
    #pragma comment(lib, "Imath-2_4.lib")
    #pragma comment(lib, "Alembic.lib")
    #ifdef mqabcEnableHDF5
        #pragma comment(lib, "libhdf5.lib")
        #pragma comment(lib, "libszip.lib")
        #pragma comment(lib, "zlib.lib")
    #endif
#endif // _WIN32

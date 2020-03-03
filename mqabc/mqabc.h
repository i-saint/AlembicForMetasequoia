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

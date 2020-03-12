#include "pch.h"
#include "Test.h"
using namespace mu;

TestCase(Test_PipeStream)
{
    const char cmd_ls[] =
#ifdef _WIN32
        "dir";
#else
        "ls";
#endif

    {
        // text mode
        std::cout << "text mode:" << std::endl;
        PipeStream pstream;
        if (pstream.open(cmd_ls, std::ios::in)) {
            std::string line;
            while (std::getline(pstream, line))
                std::cout << line << std::endl;
        }
    }
    {
        // binary mode
        std::cout << "binary mode:" << std::endl;
        PipeStream pstream;
        if (pstream.open(cmd_ls, std::ios::in | std::ios::binary)) {
            std::vector<char> buf;
            buf.resize(1024 * 16);
            pstream.read(buf.data(), buf.size());
            buf.resize(pstream.gcount());
            std::cout.write(buf.data(), buf.size());
        }
    }
}

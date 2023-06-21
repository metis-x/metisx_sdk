#include "echo_test.hpp"

#include "metisx_api.h"

#if _SIM_
int hostAppPrintTest(int argc, const char *argv[])
#else
int main(int argc, const char *argv[])
#endif
{

    if (argc <= 1)
    {
        std::cout << "need elf file name" << std::endl;
        return 0;
    }

    int testCount = std::stoi(argv[1]);
    const char* filename = argv[2];
        
    // input setup
    size_t argType = static_cast<size_t>(metisx::api::wrapper::ArgType::Input);
    void* arg = metisx::api::wrapper::mxMallocDecl(sizeof(void*), testCount, argType);

    metisx::api::wrapper::mxMap(filename, arg);

    metisx::api::wrapper::mxFreeDecl(arg);

    return 0;
}
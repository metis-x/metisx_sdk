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
    auto arg = metisx::api::make_metisx<void*>(testCount, metisx::api::ArgType::Input);

    metisx::api::mxMap(filename, arg);
    
    return 0;
}
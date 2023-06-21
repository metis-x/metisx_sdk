#include "sort_test.hpp"

#include "metisx_api.h"

#if _SIM_
int hostAppSortTest(int argc, const char *argv[])
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
    
    int size = 100;
    
    // input setup
    size_t argType = static_cast<size_t>(metisx::api::wrapper::ArgType::Input) | static_cast<size_t>(metisx::api::wrapper::ArgType::Output);
    void* arg = metisx::api::wrapper::mxMallocDecl(sizeof(int) * size, testCount, argType);

    int (*arr)[100] = (int(*)[100])arg;
    for(int i = 0 ; i < testCount; i++)
    {
        for(int j = 0; j < size; j++)
        {
            arr[i][j] = size - j;
        }
    }

    metisx::api::wrapper::mxMap(filename, arg, size);
    
    // verify
    for(int i = 0 ; i < testCount; i++)
    {
        for(int j = 0; j < size; j++)
        {
            assert(arr[i][j] == j + 1);
        }
    }
    metisx::api::wrapper::mxFreeDecl(arg);

    return 0;
}
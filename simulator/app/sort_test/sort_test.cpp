#include "sort_test.hpp"

#include "metisx_api.h"
#include "metisx_alveo_sim.hpp"

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
    auto arr = metisx::api::make_metisx<int[100]>(testCount, metisx::api::ArgType::Both);

    for(int i = 0 ; i < testCount; i++)
    {
        for(int j = 0; j < size; j++)
        {
            arr[i][j] = size - j;
        }
    }

    metisx::api::mxMap(filename, arr, size);
    
    // verify
    for(int i = 0 ; i < testCount; i++)
    {
        for(int j = 0; j < size; j++)
        {
            assert(arr[i][j] == j + 1);
        }
    }

    return 0;
}
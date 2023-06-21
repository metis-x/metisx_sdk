#include <string>
#include "metisx_alveo_sim.hpp"
#include "echo_test.hpp"
#include "sort_test.hpp"

void printTest()
{
    int testCount = 8;
    const char *testargv[] = {"host", std::to_string(testCount).c_str(), "../../../mu/out/mu_print.os"};
    int testargc = sizeof(testargv)/sizeof(char*);

    metisx::sim::alveoSimulatorRun((metisx::sim::MainFunc)hostAppPrintTest, testargc, (char **)testargv);
}

void sortTest()
{
    int testCount = 100;
    const char *testargv[] = {"host", std::to_string(testCount).c_str(), "../../../mu/out/mu_sort.os"};
    int testargc = sizeof(testargv)/sizeof(char*);

    metisx::sim::alveoSimulatorRun((metisx::sim::MainFunc)hostAppSortTest, testargc, (char **)testargv);
}

int main(int argc, char *argv[])
{
    printTest();

    sortTest();

    return 0;
}
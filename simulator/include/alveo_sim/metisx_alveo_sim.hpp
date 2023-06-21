#pragma once

#include <stdint.h>

namespace metisx
{
namespace sim
{

using MainFunc   = int (*)(uint32_t, char**);

void alveoSimulatorRun(MainFunc func, uint32_t argc, char** argv);

void enableMuDebugger(uint64_t coreId, uint64_t clusterId, uint64_t muId);

} // namespace sim
} // namespace metisx
#pragma once
#include <stdint.h>
namespace metisx
{

namespace api
{
enum class ExecuteType : int
{
    Async,
    Sync,
};

enum class Debug : uint32_t
{
    Disable,
    Enable,
};

}
}
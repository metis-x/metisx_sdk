#ifndef _CORE_H_
#define _CORE_H_

#ifdef __cplusplus
    #ifdef _MU_
        #include <stdint.h>
        #include <stddef.h>
    #else
        #include <cstdint>
        #include <cstddef>
    #endif
#else
#include <stdint.h>
#endif

#ifndef _MU_
#include <assert.h>
#endif
#include "sys_const.h"
#include "sys_macro.h"
#include "sys_config.h"
#include "sys_memory_map.h"
#include "sys_structure.h"
#include "sys_defs.h"

#endif // _CORE_H_
#pragma once

#include <stdint.h>
#include "sys.h"
#include "mu_const.h"

#define SRAM_SECTION        __attribute__((section (".fast_data")))
#define DRAM_SECTION        __attribute__((section (".data")))
#define DRAM_BSS_SECTION    __attribute__((section (".bss")))
#define STAT_SECTION        __attribute__((section (".stat")))
#define EXT_CODE_SECTION    __attribute__((section (".text_lib")))


#if _SIM_
#include "sim/src/sim.h"
#else
#include "mu_intrinsic.h"
#include <string.h>
#endif
#include "mu_structures.h"
#include "mu_alloc.h"
#include "mu_loader.h"
#include "mu_printf.h"
#include "mu_macro.h"

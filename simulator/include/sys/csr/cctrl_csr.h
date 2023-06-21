#pragma once
// ===========================================================================================================
// Company      : MetisX
// File         : cctrl_csr.h
// Author       : kbshin
// Created      : Tue Feb  7 10:45:47 2023
// ===========================================================================================================

static const uint32_t CCTRL_CSR_MON_SVN_REV_OFFSET                     = 0x0;
static const uint32_t CCTRL_CSR_MON_CLST_BMP_OFFSET                    = 0x8;
static const uint32_t CCTRL_CSR_MON_CFG0_OFFSET                        = 0x10;
static const uint32_t CCTRL_CSR_MON_CFG1_OFFSET                        = 0x18;
static const uint32_t CCTRL_CSR_NUM_MU_PER_CLST_3_0_OFFSET             = 0x80;
static const uint32_t CCTRL_CSR_NUM_MU_PER_CLST_7_4_OFFSET             = 0x88;
static const uint32_t CCTRL_CSR_NUM_MU_PER_CLST_11_8_OFFSET            = 0x90;
static const uint32_t CCTRL_CSR_NUM_MU_PER_CLST_15_12_OFFSET           = 0x98;
static const uint32_t CCTRL_CSR_NUM_MU_PER_CLST_19_16_OFFSET           = 0xa0;
static const uint32_t CCTRL_CSR_NUM_MU_PER_CLST_23_20_OFFSET           = 0xa8;
static const uint32_t CCTRL_CSR_NUM_MU_PER_CLST_27_24_OFFSET           = 0xb0;
static const uint32_t CCTRL_CSR_NUM_MU_PER_CLST_31_28_OFFSET           = 0xb8;
static const uint32_t CCTRL_CSR_DUMMY_REG0_OFFSET                      = 0x100;
static const uint32_t CCTRL_CSR_DUMMY_REG1_OFFSET                      = 0x108;
static const uint32_t CCTRL_CSR_PMON_CLEAR_OFFSET                      = 0x200;
static const uint32_t CCTRL_CSR_PMON_CLEAR_CYCLE_OFFSET                = 0x208;
static const uint32_t CCTRL_CSR_PMON_CYCLE_CNT_OFFSET                  = 0x210;


typedef union
{
	uint64_t u64;
} CctrlCsrMonSvnRev_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_sub0_clst : 8;
		uint64_t r_sub1_clst : 8;
		uint64_t r_sub2_clst : 8;
		uint64_t r_sub3_clst : 8;
	};
} CctrlCsrMonClstBmp_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t ddr_capaticy : 1;
		uint64_t : 3;
		uint64_t num_mts_core : 4;
		uint64_t : 8;
		uint64_t sim_cfg : 1;
		uint64_t : 3;
		uint64_t adm_per_sub : 4;
		uint64_t : 8;
		uint64_t : 32;
	};
} CctrlCsrMonCfg0_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_num_per_clst : 8;
		uint64_t r_mu_freq : 8;
		uint64_t r_dpe_freq : 8;
	};
} CctrlCsrMonCfg1_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_clst0 : 8;
		uint64_t r_clst1 : 8;
		uint64_t r_clst2 : 8;
		uint64_t r_clst3 : 8;
	};
} CctrlCsrNumMuPerClst30_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_clst4 : 8;
		uint64_t r_clst5 : 8;
		uint64_t r_clst6 : 8;
		uint64_t r_clst7 : 8;
	};
} CctrlCsrNumMuPerClst74_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_clst8 : 8;
		uint64_t r_clst9 : 8;
		uint64_t r_clst10 : 8;
		uint64_t r_clst11 : 8;
	};
} CctrlCsrNumMuPerClst118_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_clst12 : 8;
		uint64_t r_clst13 : 8;
		uint64_t r_clst14 : 8;
		uint64_t r_clst15 : 8;
	};
} CctrlCsrNumMuPerClst1512_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_clst16 : 8;
		uint64_t r_clst17 : 8;
		uint64_t r_clst18 : 8;
		uint64_t r_clst19 : 8;
	};
} CctrlCsrNumMuPerClst1916_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_clst20 : 8;
		uint64_t r_clst21 : 8;
		uint64_t r_clst22 : 8;
		uint64_t r_clst23 : 8;
	};
} CctrlCsrNumMuPerClst2320_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_clst24 : 8;
		uint64_t r_clst25 : 8;
		uint64_t r_clst26 : 8;
		uint64_t r_clst27 : 8;
	};
} CctrlCsrNumMuPerClst2724_t;

typedef union
{
	uint64_t u64;

	struct
	{
		uint64_t r_clst28 : 8;
		uint64_t r_clst29 : 8;
		uint64_t r_clst30 : 8;
		uint64_t r_clst31 : 8;
	};
} CctrlCsrNumMuPerClst3128_t;

typedef union
{
	uint64_t u64;
} CctrlCsrDummyReg0_t;

typedef union
{
	uint64_t u64;
} CctrlCsrDummyReg1_t;

typedef union
{
	uint64_t u64;
} CctrlCsrPmonClear_t;

typedef union
{
	uint64_t u64;
} CctrlCsrPmonClearCycle_t;

typedef union
{
	uint64_t u64;
} CctrlCsrPmonCycleCnt_t;

typedef union
{
	uint64_t u64[67];

	struct
	{
		CctrlCsrMonSvnRev_t CctrlCsrMonSvnRev;                       // CCTRL_CSR_MON_SVN_REV_OFFSET (0x0)
		CctrlCsrMonClstBmp_t CctrlCsrMonClstBmp;                     // CCTRL_CSR_MON_CLST_BMP_OFFSET (0x8)
		CctrlCsrMonCfg0_t CctrlCsrMonCfg0;                           // CCTRL_CSR_MON_CFG0_OFFSET (0x10)
		CctrlCsrMonCfg1_t CctrlCsrMonCfg1;                           // CCTRL_CSR_MON_CFG1_OFFSET (0x18)
		uint64_t RSVD0[12];
		CctrlCsrNumMuPerClst30_t CctrlCsrNumMuPerClst30;             // CCTRL_CSR_NUM_MU_PER_CLST_3_0_OFFSET (0x80)
		CctrlCsrNumMuPerClst74_t CctrlCsrNumMuPerClst74;             // CCTRL_CSR_NUM_MU_PER_CLST_7_4_OFFSET (0x88)
		CctrlCsrNumMuPerClst118_t CctrlCsrNumMuPerClst118;           // CCTRL_CSR_NUM_MU_PER_CLST_11_8_OFFSET (0x90)
		CctrlCsrNumMuPerClst1512_t CctrlCsrNumMuPerClst1512;         // CCTRL_CSR_NUM_MU_PER_CLST_15_12_OFFSET (0x98)
		CctrlCsrNumMuPerClst1916_t CctrlCsrNumMuPerClst1916;         // CCTRL_CSR_NUM_MU_PER_CLST_19_16_OFFSET (0xa0)
		CctrlCsrNumMuPerClst2320_t CctrlCsrNumMuPerClst2320;         // CCTRL_CSR_NUM_MU_PER_CLST_23_20_OFFSET (0xa8)
		CctrlCsrNumMuPerClst2724_t CctrlCsrNumMuPerClst2724;         // CCTRL_CSR_NUM_MU_PER_CLST_27_24_OFFSET (0xb0)
		CctrlCsrNumMuPerClst3128_t CctrlCsrNumMuPerClst3128;         // CCTRL_CSR_NUM_MU_PER_CLST_31_28_OFFSET (0xb8)
		uint64_t RSVD1[8];
		CctrlCsrDummyReg0_t CctrlCsrDummyReg0;                       // CCTRL_CSR_DUMMY_REG0_OFFSET (0x100)
		CctrlCsrDummyReg1_t CctrlCsrDummyReg1;                       // CCTRL_CSR_DUMMY_REG1_OFFSET (0x108)
		uint64_t RSVD2[30];
		CctrlCsrPmonClear_t CctrlCsrPmonClear;                       // CCTRL_CSR_PMON_CLEAR_OFFSET (0x200)
		CctrlCsrPmonClearCycle_t CctrlCsrPmonClearCycle;             // CCTRL_CSR_PMON_CLEAR_CYCLE_OFFSET (0x208)
		CctrlCsrPmonCycleCnt_t CctrlCsrPmonCycleCnt;                 // CCTRL_CSR_PMON_CYCLE_CNT_OFFSET (0x210)
	};
} CctrlCsr_t;

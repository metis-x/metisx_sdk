# disassembler.py

#import
from Font import *

# ISA
TYPE_R = 0
TYPE_I1 = 1
TYPE_I2 = 2
TYPE_L = 3
ISA = {}

ISA[ 0b1000 << 6 | TYPE_R << 4 | 0x4 << 1 ] = {"name" : "SLL", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "NOT_USED"}
ISA[ 0b1001 << 6 | TYPE_R << 4 | 0x4 << 1 ] = {"name" : "SRL", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "NOT_USED"}
ISA[ 0b1011 << 6 | TYPE_R << 4 | 0x4 << 1 ] = {"name" : "SRA", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "NOT_USED"}
ISA[ 0b0000 << 6 | TYPE_R << 4 | 0x4 << 1 ] = {"name" : "SLLW", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "NOT_USED"}
ISA[ 0b0001 << 6 | TYPE_R << 4 | 0x4 << 1 ] = {"name" : "SRLW", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "NOT_USED"}
ISA[ 0b0011 << 6 | TYPE_R << 4 | 0x4 << 1 ] = {"name" : "SRAW", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "NOT_USED"}
ISA[ 0b1000 << 6 | TYPE_R << 4 | 0x0 << 1 ] = {"name" : "XOR", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "NOT_USED"}
ISA[ 0b1001 << 6 | TYPE_R << 4 | 0x0 << 1 ] = {"name" : "OR", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "NOT_USED"}
ISA[ 0b1010 << 6 | TYPE_R << 4 | 0x0 << 1 ] = {"name" : "AND", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "NOT_USED"}
ISA[ 0b1100 << 6 | TYPE_R << 4 | 0x0 << 1 ] = {"name" : "ADD", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "NOT_USED"}
ISA[ 0b1101 << 6 | TYPE_R << 4 | 0x0 << 1 ] = {"name" : "SUB", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "NOT_USED"}
ISA[ 0b0100 << 6 | TYPE_R << 4 | 0x0 << 1 ] = {"name" : "ADDW", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "NOT_USED"}
ISA[ 0b0101 << 6 | TYPE_R << 4 | 0x0 << 1 ] = {"name" : "SUBW", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "NOT_USED"}
ISA[ 0b1110 << 6 | TYPE_R << 4 | 0x0 << 1 ] = {"name" : "SLT", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "NOT_USED"}
ISA[ 0b1111 << 6 | TYPE_R << 4 | 0x0 << 1 ] = {"name" : "SLTU", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "NOT_USED"}

ISA[ 0b0000 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "MUL",  "delay" : 13, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b1000 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "DIV",  "delay" : 13, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b1001 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "DIVU",  "delay" : 13, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b1010 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "REM",  "delay" : 13, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b1011 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "REMU",  "delay" : 13, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}

ISA[ 0b1100 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "DIVW", "delay" : 22, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b1101 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "DIVUW", "delay" : 22, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b1110 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "REMW", "delay" : 22, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b1111 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "REMUW", "delay" : 22, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}

ISA[ 0b0001 << 6 | TYPE_R << 4 | 0x1 << 1 ] = {"name" : "DPER", "delay" : 1, "GROUP" : ["DPE", "COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b0100 << 6 | TYPE_R << 4 | 0x1 << 1 ] = {"name" : "MCPY", "delay" : 1, "GROUP" : ["DPE", "COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b0101 << 6 | TYPE_R << 4 | 0x1 << 1 ] = {"name" : "MSET", "delay" : 1, "GROUP" : ["DPE", "COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b0000 << 6 | TYPE_R << 4 | 0x1 << 1 ] = {"name" : "DPES", "delay" : 1, "GROUP" : ["DPE", "COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}

ISA[ 0b0111 << 6 | TYPE_R << 4 | 0x5 << 1 ] = {"name" : "MDB", "delay" : 1, "GROUP" : ["DPE", "COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b0000 << 6 | TYPE_R << 4 | 0x6 << 1 ] = {"name" : "FIFOP", "delay" : 1, "GROUP" : ["COP"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b0001 << 6 | TYPE_R << 4 | 0x6 << 1 ] = {"name" : "FIFOR", "delay" : 1, "GROUP" : ["COP"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b0010 << 6 | TYPE_R << 4 | 0x6 << 1 ] = {"name" : "LCHR", "delay" : 1, "GROUP" : ["COP"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b0100 << 6 | TYPE_R << 4 | 0x6 << 1 ] = {"name" : "FIFOS", "delay" : 1, "GROUP" : ["COP"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b0101 << 6 | TYPE_R << 4 | 0x6 << 1 ] = {"name" : "TERM", "delay" : 1, "GROUP" : ["COP"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b0110 << 6 | TYPE_R << 4 | 0x6 << 1 ] = {"name" : "SYNC", "delay" : 1, "GROUP" : ["COP"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"}
ISA[ 0b1000 << 6 | TYPE_R << 4 | 0x6 << 1 ] = {"name" : "CBUSRD", "delay" : 100, "IMM" : "NOT_USED"}
ISA[ 0b1001 << 6 | TYPE_R << 4 | 0x6 << 1 ] = {"name" : "CBUSWR", "delay" : 100, "IMM" : "NOT_USED"}

ISA[ 0b1000 << 6 | TYPE_I1 << 4 | 0x4 << 1 ] = {"name" : "SLLI", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "UNSIGNED"}
ISA[ 0b1001 << 6 | TYPE_I1 << 4 | 0x4 << 1 ] = {"name" : "SRLI", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "UNSIGNED"}
ISA[ 0b1011 << 6 | TYPE_I1 << 4 | 0x4 << 1 ] = {"name" : "SRAI", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "SIGNED"}
ISA[ 0b0000 << 6 | TYPE_I1 << 4 | 0x4 << 1 ] = {"name" : "SLLIW", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "UNSIGNED"}
ISA[ 0b0001 << 6 | TYPE_I1 << 4 | 0x4 << 1 ] = {"name" : "SRLIW", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "UNSIGNED"}
ISA[ 0b0011 << 6 | TYPE_I1 << 4 | 0x4 << 1 ] = {"name" : "SRAIW", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "SIGNED"}
ISA[ 0b1000 << 6 | TYPE_I1 << 4 | 0x0 << 1 ] = {"name" : "XORI", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "SIGNED"}
ISA[ 0b1001 << 6 | TYPE_I1 << 4 | 0x0 << 1 ] = {"name" : "ORI", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "SIGNED"}
ISA[ 0b1010 << 6 | TYPE_I1 << 4 | 0x0 << 1 ] = {"name" : "ANDI","delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "SIGNED"}
ISA[ 0b1100 << 6 | TYPE_I1 << 4 | 0x0 << 1 ] = {"name" : "ADDI", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "SIGNED"}
ISA[ 0b0100 << 6 | TYPE_I1 << 4 | 0x0 << 1 ] = {"name" : "ADDIW", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "SIGNED"}
ISA[ 0b1110 << 6 | TYPE_I1 << 4 | 0x0 << 1 ] = {"name" : "SLTI", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "SIGNED"}
ISA[ 0b1111 << 6 | TYPE_I1 << 4 | 0x0 << 1 ] = {"name" : "SLTIU", "delay" : 1, "GROUP" : ["_ARITH"], "TYPE" : "ARITH", "IMM" : "SIGNED"}
ISA[ 0b1000 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LB", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"} # 1
ISA[ 0b1001 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LH", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"} # 2
ISA[ 0b1010 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LW", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"} # 4
ISA[ 0b1011 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LD", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"} # 8
ISA[ 0b1100 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LQ", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"} # 16
ISA[ 0b0000 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LBU", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}  # 1
ISA[ 0b0001 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LHU", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}  # 2
ISA[ 0b0010 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LWU", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}  # 4
ISA[ 0b0011 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LDU", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}  # 8
ISA[ 0b0100 << 6 | TYPE_I1 << 4 | 0x1 << 1 ] = {"name" : "LQU", "delay" : 5, "GROUP": ["LD", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}  # 16
ISA[ 0b0000 << 6 | TYPE_I1 << 4 | 0x2 << 1 ] = {"name" : "JALR", "delay" : 4, "GROUP" : ["_BRANCH"], "TYPE" : "BRANCH", "IMM" : "SIGNED"}

ISA[ 0b0000 << 6 | TYPE_I1 << 4 | 0x5 << 1 ] = {"name" : "CTZ", "delay" : 8, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"} 
ISA[ 0b0100 << 6 | TYPE_I1 << 4 | 0x5 << 1 ] = {"name" : "REV", "delay" : 8, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "UNSIGNED"} 
ISA[ 0b1000 << 6 | TYPE_I1 << 4 | 0x5 << 1 ] = {"name" : "POPC", "delay" : 8, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "NOT_USED"} 
ISA[ 0b0001 << 6 | TYPE_I1 << 4 | 0x5 << 1 ] = {"name" : "ASPC", "delay" : 8, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "UNSIGNED"}
ISA[ 0b0101 << 6 | TYPE_I1 << 4 | 0x5 << 1 ] = {"name" : "LSPC", "delay" : 8, "GROUP": ["COP", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "UNSIGNED"}
ISA[ 0b1000 << 6 | TYPE_I1 << 4 | 0x7 << 1 ] = {"name" : "CSRGET","delay" : 2, "IMM" : "UNSIGNED"}
ISA[ 0b0001 << 6 | TYPE_I1 << 4 | 0x7 << 1 ] = {"name" : "CSRSET","delay" : 2, "IMM" : "UNSIGNED"}
ISA[ 0b0010 << 6 | TYPE_I1 << 4 | 0x7 << 1 ] = {"name" : "CSRCLR","delay" : 2, "IMM" : "UNSIGNED"}
ISA[ 0b0110 << 6 | TYPE_I1 << 4 | 0x7 << 1 ] = {"name" : "CSRWAIT","delay" : 2, "IMM" : "UNSIGNED"}
ISA[ 0b0101 << 6 | TYPE_I1 << 4 | 0x7 << 1 ] = {"name" : "DBREAK","delay" : 2, "IMM" : "NOT_USED"}
ISA[ 0b0111 << 6 | TYPE_I1 << 4 | 0x7 << 1 ] = {"name" : "SCALL","delay" : 1, "IMM" : "NOT_USED"}

ISA[ 0b1000 << 6 | TYPE_I2 << 4 | 0x1 << 1 ] = {"name" : "SB", "delay" : 1, "GROUP": ["ST", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}
ISA[ 0b1001 << 6 | TYPE_I2 << 4 | 0x1 << 1 ] = {"name" : "SH", "delay" : 1, "GROUP": ["ST", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}
ISA[ 0b1010 << 6 | TYPE_I2 << 4 | 0x1 << 1 ] = {"name" : "SW", "delay" : 1, "GROUP": ["ST", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}
ISA[ 0b1011 << 6 | TYPE_I2 << 4 | 0x1 << 1 ] = {"name" : "SD", "delay" : 1, "GROUP": ["ST", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}
ISA[ 0b1100 << 6 | TYPE_I2 << 4 | 0x1 << 1 ] = {"name" : "SQ", "delay" : 1, "GROUP": ["ST", "_EXTERNAL"], "TYPE" : "EXTERNAL", "IMM" : "SIGNED"}

ISA[ 0b0000 << 6 | TYPE_I2 << 4 | 0x7 << 1 ] = {"name" : "DMB", "delay" : 100, "IMM" : "NOT_USED"}
ISA[ 0b0001 << 6 | TYPE_I2 << 4 | 0x7 << 1 ] = {"name" : "CINV", "delay" : 100, "IMM" : "UNSIGNED"}
ISA[ 0b0010 << 6 | TYPE_I2 << 4 | 0x7 << 1 ] = {"name" : "CFLS", "delay" : 100, "IMM" : "UNSIGNED"}
ISA[ 0b0000 << 6 | TYPE_I2 << 4 | 0x2 << 1 ] = {"name" : "BEQ", "delay" : 4, "GROUP" : ["_BRANCH"], "TYPE" : "BRANCH", "IMM" : "SIGNED"}
ISA[ 0b0100 << 6 | TYPE_I2 << 4 | 0x2 << 1 ] = {"name" : "BNE", "delay" : 4, "GROUP" : ["_BRANCH"], "TYPE" : "BRANCH", "IMM" : "SIGNED"}
ISA[ 0b1110 << 6 | TYPE_I2 << 4 | 0x2 << 1 ] = {"name" : "BLT", "delay" : 4, "GROUP" : ["_BRANCH"], "TYPE" : "BRANCH", "IMM" : "SIGNED"}
ISA[ 0b1010 << 6 | TYPE_I2 << 4 | 0x2 << 1 ] = {"name" : "BGE", "delay" : 4, "GROUP" : ["_BRANCH"], "TYPE" : "BRANCH", "IMM" : "SIGNED"}
ISA[ 0b1111 << 6 | TYPE_I2 << 4 | 0x2 << 1 ] = {"name" : "BLTU", "delay" : 4, "GROUP" : ["_BRANCH"], "TYPE" : "BRANCH", "IMM" : "UNSIGNED"}
ISA[ 0b1011 << 6 | TYPE_I2 << 4 | 0x2 << 1 ] = {"name" : "BGEU", "delay" : 4, "GROUP" : ["_BRANCH"], "TYPE" : "BRANCH", "IMM" : "UNSIGNED"}

ISA[ 0b0001 << 6 | TYPE_L << 4 | 0x2 << 1 ] = {"name" : "JAL", "delay" : 4, "GROUP" : ["_BRANCH"], "TYPE" : "BRANCH", "IMM" : "SIGNED"}
ISA[ 0b0000 << 6 | TYPE_L << 4 | 0x3 << 1 ] = {"name" : "LUI", "delay" : 1, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "SIGNED"}
ISA[ 0b0001 << 6 | TYPE_L << 4 | 0x3 << 1 ] = {"name" : "AUIPC", "delay" : 2, "GROUP" : ["_SHIFT"], "TYPE" : "SHIFT", "IMM" : "SIGNED"}

#Reg
regString = [
                "zero",
                "sp",
                "ra",
                "s0",
                "s1",
                "s2",
                "t0",
                "t1",
                "t2",
                "t3",
                "t4",
                "t5",
                "a0",
                "a1",
                "a2",
                "a3",
                "a4",
                "a5",
                "a6",
                "a7",
                "s3",
                "s4",
                "s5",
                "s6",
                "s7",
                "s8",
                "s9",
                "s10",
                "s11",
                "s12",
                "t6",
                "t7"]

def bitMask(bitCount):
    return ((1 << bitCount) - 1)

def getBitField(value, bitOffset, bitCount):
    return ((value >> bitOffset) & bitMask(bitCount))

class MuIsa:
    def __init__(self, binaryCode):
        self.binaryCode = binaryCode
        self.instruction = self.getInstruction()
        self.immBitCount = 0

    def getInstruction(self):
        INSTRUCTION_BIT_OFFSET = 0
        INSTRUCTION_BIT_COUNT = 10
        return getBitField(self.binaryCode, INSTRUCTION_BIT_OFFSET, INSTRUCTION_BIT_COUNT)

    def isValid(self):
        if self.instruction in ISA:
            return True
        else:
            return False

    def getType(binaryCode):
        TYPE_BIT_OFFSET = 4
        TYPE_BIT_COUNT = 2
        return getBitField(binaryCode, TYPE_BIT_OFFSET, TYPE_BIT_COUNT)

    def getSignedImm(self):
        if 0 != (self.imm >> (self.immBitCount - 1)):
            return -((1 << self.immBitCount) - self.imm)
        else:
            return self.imm

    def getInstructionName(self):
        return (ISA[self.instruction]["name"])

    def getArguments(self):
        ...

    def getImm(self):
        if ISA[self.instruction]["IMM"] == "SIGNED":
            return str(self.getSignedImm())
        else:
            return str(self.imm)

    def getAssembly(self):
        return (self.getInstructionName(), self.getArguments())

class MuIsaRtype(MuIsa):
    def __init__(self, binaryCode):
        super().__init__(binaryCode)
        RS2_BIT_OFFSET = 17
        RS2_BIT_COUNT = 5
        RS1_BIT_OFFSET = RS2_BIT_OFFSET + RS2_BIT_COUNT
        RS1_BIT_COUNT = 5
        RD_BIT_OFFSET = RS1_BIT_OFFSET + RS1_BIT_COUNT
        RD_BIT_COUNT = 5

        self.rd = getBitField(binaryCode, RD_BIT_OFFSET, RD_BIT_COUNT)
        self.rs1 = getBitField(binaryCode, RS1_BIT_OFFSET, RS1_BIT_COUNT)
        self.rs2 = getBitField(binaryCode, RS2_BIT_OFFSET, RS2_BIT_COUNT)

    def getArguments(self):
        return (regString[self.rd], regString[self.rs1], regString[self.rs2])

class MuIsaI1type(MuIsa):
    def __init__(self, binaryCode):
        super().__init__(binaryCode)
        IMM_BIT_OFFSET = 10
        IMM_BIT_COUNT = 12
        RS1_BIT_OFFSET = IMM_BIT_OFFSET + IMM_BIT_COUNT
        RS1_BIT_COUNT = 5
        RD_BIT_OFFSET = RS1_BIT_OFFSET + RS1_BIT_COUNT
        RD_BIT_COUNT = 5

        self.rd = getBitField(binaryCode, RD_BIT_OFFSET, RD_BIT_COUNT)
        self.rs1 = getBitField(binaryCode, RS1_BIT_OFFSET, RS1_BIT_COUNT)
        self.imm = getBitField(binaryCode, IMM_BIT_OFFSET, IMM_BIT_COUNT)
        self.immBitCount = IMM_BIT_COUNT

    def getArguments(self):
        return (regString[self.rd], regString[self.rs1], self.getImm())

class MuIsaI2type(MuIsa):
    def __init__(self, binaryCode):
        super().__init__(binaryCode)
        IMM6_0_BIT_OFFSET = 10
        IMM6_0_BIT_COUNT = 7
        RS2_BIT_OFFSET = IMM6_0_BIT_OFFSET + IMM6_0_BIT_COUNT
        RS2_BIT_COUNT = 5
        RS1_BIT_OFFSET = RS2_BIT_OFFSET + RS2_BIT_COUNT
        RS1_BIT_COUNT = 5
        IMM11_7_BIT_OFFSET = RS1_BIT_OFFSET + RS1_BIT_COUNT
        IMM11_7_BIT_COUNT = 5
        IMM_BIT_COUNT = IMM6_0_BIT_COUNT + IMM11_7_BIT_COUNT

        self.rs2 = getBitField(binaryCode, RS2_BIT_OFFSET, RS2_BIT_COUNT)
        self.rs1 = getBitField(binaryCode, RS1_BIT_OFFSET, RS1_BIT_COUNT)
        imm0_6 = getBitField(binaryCode, IMM6_0_BIT_OFFSET, IMM6_0_BIT_COUNT)
        imm11_7 = getBitField(binaryCode, IMM11_7_BIT_OFFSET, IMM11_7_BIT_COUNT)
        self.imm = ((imm11_7 << 7) + imm0_6)
        self.immBitCount = IMM_BIT_COUNT

    def getArguments(self):
        return (regString[self.rs1], regString[self.rs2], self.getImm())

class MuIsaLtype(MuIsa):
    def __init__(self, binaryCode):
        super().__init__(binaryCode)
        IMM_BIT_OFFSET = 7
        IMM_BIT_COUNT = 20
        RD_BIT_OFFSET = IMM_BIT_OFFSET + IMM_BIT_COUNT
        RD_BIT_COUNT = 5

        self.instruction = self.getInstruction()
        self.rd = getBitField(binaryCode, RD_BIT_OFFSET, RD_BIT_COUNT)
        self.imm = getBitField(binaryCode, IMM_BIT_OFFSET, IMM_BIT_COUNT)
        self.immBitCount = IMM_BIT_COUNT

    def getInstruction(self):
        INSTRUCTION_BIT_OFFSET = 0
        INSTRUCTION_BIT_COUNT = 7
        return getBitField(self.binaryCode, INSTRUCTION_BIT_OFFSET, INSTRUCTION_BIT_COUNT)

    def getArguments(self):
        return (regString[self.rd], self.getImm())

isaTypeArray = [MuIsaRtype, MuIsaI1type, MuIsaI2type, MuIsaLtype]

def disassemble(binaryCode):
    ## input
    #print('\033[90m  (0x{0:08X})\033[0m'.format(binaryCode), end="\t")
    isaType = MuIsa.getType(binaryCode)

    muIsa = isaTypeArray[isaType](binaryCode)

    assemblyCode = None
    if muIsa.isValid():
        assemblyCode = muIsa.getAssembly()

    return assemblyCode

def _printAssemblyCode(assemblyCode):
    print(f'{Font.green(assemblyCode[0])}', end="\t")

    for i in assemblyCode[1]:
        print(i, end="\t")
    print("")

def printDisassemble(binaryCode):
    assemblyCode = disassemble(binaryCode)
    if None != assemblyCode:
        _printAssemblyCode(assemblyCode)
    else:
        print(Font.bright_black(f'0x{binaryCode:X} is invalid instruction'))

def getInstructionName(binaryCode):
    assemblyCode = disassemble(binaryCode)
    if None != assemblyCode:
        return assemblyCode[0]
    else:
        return '-'

def main():
#example 1
#binaryCode |0x283f_ff10_6000_021e|0x6b4f_ff10_6940_fe18|
    printDisassemble(0x6940fe18)
    printDisassemble(0x6b4fff10)
    printDisassemble(0x6000021e)
    printDisassemble(0x283fff10)
#example 2
    printDisassemble(0x631a0300) #    1c54: 00 03 1a 63  	add	a0, a0, a1
    printDisassemble(0x087f8310) #      d0: 10 83 7f 08  	addi	sp, sp, -32 I1
    printDisassemble(0x1041e2d2) #    1394: d2 e2 41 10  	ld	ra, 120(sp)
    printDisassemble(0x031e5ae4) #     4a0: e4 5a 1e 03  	bgeu	a0, a3, 0x4cc
    printDisassemble(0x71800036) #     1fc: 36 00 80 71  	lui	a2, 196608

#example 3
    print(getInstructionName(0x031e5ae4))
    print(getInstructionName(0x71800036))

#exception test
    printDisassemble(0xdeaddead)
    print(getInstructionName(0xdeaddead))

# Functions
if __name__ == '__main__':
    main()
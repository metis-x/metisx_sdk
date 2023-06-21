/*Reference Site : https://blog.dasomoli.org/linker-linker-scripts-ld/ */
/*
OUTPUT_FORMAT("elf64-metis", "elf64-metis", "elf64-metis")
*/
OUTPUT_ARCH(metis)
ENTRY(_start)

MEMORY
{
  TEXT(rx)        : o  = 0x0,        LENGTH = 128K
  TEXT_LIB(rx)    : o  = 0x20000,    LENGTH = 128K
  DRAM_CTX(rw!x)  : o  = 0x10000000, LENGTH = 0x4000    /*16K */
  DRAM(rw!x)      : o  = 0x10004000, LENGTH = 0x3FC000  /*4M - 16K*/
  SRAM(rw!x)      : o  = 0x30000000, LENGTH = 16K
}


SECTIONS
{
  .text : ALIGN(8)
  {
    *(.text)
    *(.text.*)
  }  >TEXT AT>TEXT

  .text_lib : ALIGN(8)
  {
    *(.text_lib)
     _textEnd = .;
  }  >TEXT_LIB AT>TEXT_LIB

  .sram_ctx : ALIGN(8)
  {
    _sramDataStart = .;
    *(.stat) ALIGN(0x20)
    *(.fast_rodata)
    *(.fast_data) 
    *(.sdata*)    
    . = ALIGN(8);
    _sramDataEnd = .; 
  } >SRAM /*AT>TEXT*/

  .dram_ctx : ALIGN(8)
  {
    _dramReadOnlyStart = .;
    *(.rodata*)
    *(.lib_rodata*)
    *(.lib_rwdata*)  
  
    . = ALIGN(8);
    _dramReadOnlyEnd = .;

    _dramDataStart = .;
    *(.data*)
    . = ALIGN(8);
    _dramDataEnd = .;
  }  >DRAM_CTX /*AT>TEXT*/
    
  .sram : ALIGN(8)
  {   
    _sramBssStart = .;
    *(.fast_bss)
    *(.sbss*)
    _sramBssEnd = .;
  }  >SRAM AT>SRAM
  
  .dram : ALIGN(8)
  {       
    *(.PRINT_BUF) ALIGN(0x100)       
    _dramBssStart = .;    
    *(.bss*)        
    _dramBssEnd = .;
    *(COMMON)    
    *(.uninitialize)
    _dramMallocStart = .; 
  } >DRAM AT>DRAM

  PROVIDE (__heap_start = 0x10700000);
  PROVIDE (__heap_end   = 0x10800000);
}
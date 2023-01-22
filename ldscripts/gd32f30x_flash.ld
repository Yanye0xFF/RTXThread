/*
**  File        : gd32f103_flash.ld

Attention：CortexM4 should be linked with v7e-m+xx, xx - depending on floating point configuration. 
           CortexM4 implement single-precision only VFPv4 instructions with 16 double-word registers
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* 2048 bytes stack for crt and RTX5 kernel */
_system_stack_size = 0x800;

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 256K 
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 48K
}

/* Define output sections */
SECTIONS
{
  __link_addr = ORIGIN(FLASH);

  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH
  
  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text.Reset_Handler)    
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.gnu.linkonce.t*)
    
    /* section information for finsh shell */
    . = ALIGN(4);
    __fsymtab_start = .;
    KEEP(*(FSymTab))
    __fsymtab_end = .;

    /* section information for initial. */
    . = ALIGN(4);
    __rt_init_start = .;
    KEEP(*(SORT(.rti_fn*)))
    __rt_init_end = .;

    . = ALIGN(4);
    
    /* for c++ */
    PROVIDE(__ctors_start__ = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE(__ctors_end__ = .);
    
    /* */
    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH


   .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
    .ARM : {
    __exidx_start = .;
      *(.ARM.exidx*)
      __exidx_end = .;
    } >FLASH

  /* used by the startup to initialize data */
  _sidata = .;

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.gnu.linkonce.d*)
    
    /* for c++ */
    PROVIDE(__dtors_start__ = .);
    KEEP(*(SORT(.dtors.*)))
    KEEP(*(.dtors))
    PROVIDE(__dtors_end__ = .);
    
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM
  
  .stack : 
  {
    . = ALIGN(4);
    _sstack = .;
    . = . + _system_stack_size;
    . = ALIGN(4);
    _estack = .;
  } >RAM


  /* Uninitialized data section */
  __bss_start = .;
  .bss :
  {
    . = ALIGN(4);
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;
    
    *(.bss)
    *(.bss.*)
    *(COMMON)  /* -fno-common */
    
    . = ALIGN(4);
    /* This is used by the startup in order to initialize the .bss secion */
    _ebss = . ;
    
    *(.bss.init)
  } > RAM
  __bss_end = .;
  _end = .;
  
  PROVIDE ( end = _ebss );
  PROVIDE ( _end = _ebss );

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
    /* DWARF debug sections.
     * Symbols in the DWARF debugging sections are relative to the beginning
     * of the section so we begin them at 0.  */
    /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
}

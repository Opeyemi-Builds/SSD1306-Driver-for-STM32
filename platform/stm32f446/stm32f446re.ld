/* Linker script for STM32F446RE -- WRITTEN BY ME.
 * Tells the linker WHERE things live in the chip's memory map. */
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);   /* stack grows down from the top of RAM */

SECTIONS
{
    /* The vector table MUST be first: the core reads the initial stack
     * pointer and reset vector from the very start of flash. */
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH

    .text : {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* .data: initialised globals. Stored in FLASH, but must live in RAM at
     * run time, so startup code copies it. _sidata = its flash address. */
    _sidata = LOADADDR(.data);
    .data : {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* .bss: zero-initialised globals. Startup code clears it. */
    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /DISCARD/ : { *(.ARM.exidx*) *(.comment) *(.note*) }
}

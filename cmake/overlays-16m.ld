/*--------------------------------------------------------------------------------
    ** Example linker script for ARM9 overlays **
--------------------------------------------------------------------------------*/

MEMORY {
    /* Placeholder LMA for overlays */
    ovlmem : ORIGIN = 0xb0000000, LENGTH = 0x10000000
}

PHDRS {
    /* Program header for the overlay table */
    ovltable  PT_LOAD FLAGS(0x200007);

    /* Program headers for each overlay (in order) */
    ovl_dsi PT_LOAD FLAGS(0x200007);
}

/* Overlays in main RAM */
SECTIONS {

    PROVIDE_HIDDEN( __ovlarea_main_start = . );
    PROVIDE_HIDDEN( __ovlarea_main_end = . );

    /* ======== Overlay 'dsi' ======== */

    . = __ovlarea_main_start;

    .ovl.dsi ALIGN(32) : {
        EXCLUDE_FILE(*.a *libnds9*.a *libcalico_ds9*.a *16m.o) *(.text .text.* .gnu.linkonce.t.* .glue_7t .glue_7 .vfp11_veneer .v4_bx)
        EXCLUDE_FILE(*.a *libnds9*.a *libcalico_ds9*.a *16m.o) *(.rodata .rodata1 .rodata.* .gnu.linkonce.r.*)
        EXCLUDE_FILE(*.a *libnds9*.a *libcalico_ds9*.a *16m.o) *(.data .data1 .data.* .gnu.linkonce.d.*)
        . = ALIGN(4);

        PROVIDE_HIDDEN( __ovl_ctor_start_dsi = . );
        KEEP( EXCLUDE_FILE(*.a *libnds9*.a *libcalico_ds9*.a *16m.o) *(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*) .init_array .ctors) )
        PROVIDE_HIDDEN( __ovl_ctor_end_dsi = . );
        KEEP( EXCLUDE_FILE(*.a *libnds9*.a *libcalico_ds9*.a *16m.o) *(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*) .fini_array .dtors) )
        PROVIDE_HIDDEN( __ovl_dtor_end_dsi = . );
    } AT>ovlmem :ovl_dsi = 0xff

    .ovl.dsi.bss . (NOLOAD) : {
        EXCLUDE_FILE(*.a *libnds9*.a *libcalico_ds9*.a *boot_16m.o) *(.bss .bss.* .dynbss .gnu.linkonce.b.* COMMON)
        . = ALIGN(4);
    } AT>ovlmem :ovl_dsi

    PROVIDE_HIDDEN( __ovl_dsi_end = . );
    __ovlarea_main_end = MAX(__ovlarea_main_end, .);

}
INSERT AFTER .twl.bss;

SECTIONS {

    .ovltable 0 : {
        /* Static ctors/dtors for overlay 'dsi' */
        LONG(__ovl_ctor_start_dsi)
        LONG(__ovl_ctor_end_dsi)
        LONG(__ovl_dtor_end_dsi)

    } AT>ovlmem :ovltable

}


SEARCH_DIR(.)
GROUP(-lgcc -lc_nano -lnosys)

MEMORY
{
  MBR_SD (rx) :         ORIGIN = 0x00000000, LENGTH = 0x26000
  FLASH (rx) :          ORIGIN = 0x00026000, LENGTH = 0x34000
  APP_INFO (rx) :       ORIGIN = 0x0005c000, LENGTH = 0x01000

  SWAP (rx) :           ORIGIN = 0x0005d000, LENGTH = 0x00000
  FACTORY_CONFIG (rx) : ORIGIN = 0x0005d000, LENGTH = 0x01000
  CONFIG1 (rx) :        ORIGIN = 0x0005e000, LENGTH = 0x01000
  CONFIG2 (rx) :        ORIGIN = 0x0005f000, LENGTH = 0x01000

/* RAM */
  SOFTDEVRAM (rwx) :    ORIGIN = 0x20000000, LENGTH = 0x02000
  RAM (rwx) :           ORIGIN = 0x20002000, LENGTH = 0x0dd00
  NVRAM (rw) :          ORIGIN = 0x2000fd00, LENGTH = 0x00100
  SHAREDRAM (rw) :      ORIGIN = 0x2000fe00, LENGTH = 0x00200
}

REGION_ALIAS("CERTIFICATE", FACTORY_CONFIG);

__sharedram_start__ = ORIGIN(SHAREDRAM);
__sharedram_end__ = ORIGIN(SHAREDRAM) + LENGTH(SHAREDRAM);

SECTIONS
{
  .bl_app_shared_ram (NOLOAD) :
  {
    PROVIDE(__start_bl_app_shared_ram = .);
    KEEP(*(.bl_app_shared_ram_nodestate))
    PROVIDE(__bl_app_shared_ram = .);
  } > SHAREDRAM
  
  .no_erase (NOLOAD) :
  {
    . = ALIGN(4);
    *(.no_erase*);
  } > NVRAM
}


INCLUDE "nrf52_mira.ld"


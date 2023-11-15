/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  MBR_SD (rx) :         ORIGIN = 0x00000000, LENGTH = 0x27000
  FLASH (rx) :          ORIGIN = 0x00027000, LENGTH = 0x59000
  FACTORY_CONFIG (rx) :    ORIGIN = 0x0007f000, LENGTH = 0x01000

  SOFTDEVRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x02300
  RAM (rwx) :        ORIGIN = 0x20002300, LENGTH = 0x1dd00
}

REGION_ALIAS("CERTIFICATE", FACTORY_CONFIG);

/* Load MBR and softdevice */
EXTERN(nrf_mbr_sd)
SECTIONS
{
    .text.mbr_sd :
    {
        KEEP(*(.nrf_mbr_sd*))
    } > MBR_SD
}

/* Add symbols to refer to certificate page */
__CertificateStart = ORIGIN(FACTORY_CONFIG);
__CertificateEnd = ORIGIN(FACTORY_CONFIG) + LENGTH(FACTORY_CONFIG);

INCLUDE "nrf_common.ld"

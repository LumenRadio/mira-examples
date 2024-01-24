
SEARCH_DIR(.)
GROUP(-lgcc -lc_nano -lnosys)

MEMORY
{
/* Active region for code */
  MBR_SD (rx) :      ORIGIN = 0x00000000, LENGTH = 0x26000
  FLASH (rx) :       ORIGIN = 0x00026000, LENGTH = 0x27000
  
/* For temporary storage of new firmware */
  SWAP (rx) :        ORIGIN = 0x0004D000, LENGTH = 0x27000

/* Two configuration areas (App data) */
  CONFIG2 (rx) :     ORIGIN = 0x00074000, LENGTH = 0x1000
  CONFIG1 (rx) :     ORIGIN = 0x00075000, LENGTH = 0x1000
  
/* Storage for device certificate (App data) */
  FACTORY_CONFIG (rx) : ORIGIN = 0x00076000, LENGTH = 0x1000

/* Bootloader, these values MUST match the values in:
 * bootloader/pca10056_ble/armgcc/secure_bootloader_gcc_nrf52.ld
 */
  /* FLASH in the bootloader script: */
  BOOTLOADER (rx) :  ORIGIN = 0x00077000, LENGTH = 0x7000
  /* mbr_params_page in the bootloader script: */
  MBRSETTINGS (rx) : ORIGIN = 0x0007e000, LENGTH = 0x1000
  /* bootloader_settings_page in the bootloader script: */
  BLSETTINGS (rx) :  ORIGIN = 0x0007f000, LENGTH = 0x1000

/* RAM */
  SOFTDEVRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x22f0
  RAM (rwx) :        ORIGIN = 0x200022f0, LENGTH = 0xdd10
}

REGION_ALIAS("CERTIFICATE", FACTORY_CONFIG);

__ApplicationStart = ORIGIN(FLASH);
__ApplicationEnd = ORIGIN(FLASH) + LENGTH(FLASH);

__BlSettingsStart = ORIGIN(BLSETTINGS);
__BlSettingsEnd = ORIGIN(BLSETTINGS) + LENGTH(BLSETTINGS);

/* Swap start/end not required, they are defined in nrf52_mira.ld */

INCLUDE "nrf52_mira.ld"

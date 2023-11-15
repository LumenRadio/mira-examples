
SEARCH_DIR(.)
GROUP(-lgcc -lc_nano -lnosys)

MEMORY
{
/* Active region for code */
  MBR_SD (rx) :      ORIGIN = 0x00000000, LENGTH = 0x027000 - 0x000000
  FLASH (rx) :       ORIGIN = 0x00027000, LENGTH = 0x07e000 - 0x027000
  
/* For temporary storage of new firmware */
  SWAP (rx) :        ORIGIN = 0x0007e000, LENGTH = 0x0f4000 - 0x07e000

/* Two configuration areas (App data) */
  CONFIG2 (rx) :     ORIGIN = 0x000f4000, LENGTH = 0x0f5000 - 0x0f4000
  CONFIG1 (rx) :     ORIGIN = 0x000f5000, LENGTH = 0x0f6000 - 0x0f5000
  
/* Storage for device certificate (App data) */
  FACTORY_CONFIG (rx) : ORIGIN = 0x000f6000, LENGTH = 0x0f7000 - 0x0f6000

/* Bootloader, these values MUST match the values in:
 * bootloader/pca10056_ble/armgcc/secure_bootloader_gcc_nrf52.ld
 */
  /* FLASH in the bootloader script: */
  BOOTLOADER (rx) :  ORIGIN = 0x000f7000, LENGTH = 0x0fe000 - 0x0f7000
  /* mbr_params_page in the bootloader script: */
  MBRSETTINGS (rx) : ORIGIN = 0x000ff000, LENGTH = 0x0ff000 - 0x0fe000
  /* bootloader_settings_page in the bootloader script: */
  BLSETTINGS (rx) :  ORIGIN = 0x000ff000, LENGTH = 0x100000 - 0x0ff000

/* RAM */
  SOFTDEVRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x20002300 - 0x20000000
  RAM (rwx) :        ORIGIN = 0x20002300, LENGTH = 0x20040000 - 0x20002300
}

REGION_ALIAS("CERTIFICATE", FACTORY_CONFIG);

__ApplicationStart = ORIGIN(FLASH);
__ApplicationEnd = ORIGIN(FLASH) + LENGTH(FLASH);

__BlSettingsStart = ORIGIN(BLSETTINGS);
__BlSettingsEnd = ORIGIN(BLSETTINGS) + LENGTH(BLSETTINGS);

/* Swap start/end not required, they are defined in nrf52_mira.ld */

INCLUDE "nrf52_mira.ld"

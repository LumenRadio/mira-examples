/* Overrides for parameters in bootloaders sdk_config.h */

/* Turn off default behaviour of entering bootloader on GPIO */
#define NRF_BL_DFU_ENTER_METHOD_BUTTON 0

/* Turn on entering bootloader via buttonless settings parameter */
#define NRF_BL_DFU_ENTER_METHOD_BUTTONLESS 1

/* Only allow dual-bank updates, firmwares too large to accomodate this will be rejected */
#define NRF_DFU_FORCE_DUAL_BANK_APP_UPDATES 1

/* Allow the bootloader settings page to be updated from firmware, this is critical for FOTA! */
#define NRF_DFU_SETTINGS_ALLOW_UPDATE_FROM_APP 1

/* Allow downgrades */
#define NRF_DFU_APP_DOWNGRADE_PREVENTION 0

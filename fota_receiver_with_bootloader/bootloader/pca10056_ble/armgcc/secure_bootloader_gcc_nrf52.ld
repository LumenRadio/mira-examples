/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) :                        ORIGIN = 0x0f7000, LENGTH = 0x0fe000 - 0x0f7000
  /* With nRF5-SDK 17.1, these values are hardcoded in:
   * components/libraries/bootloader/dfu/nrf_dfu_types.h
   */
  mbr_params_page (r) :               ORIGIN = 0x0fe000, LENGTH = 0x0ff000 - 0x0fe000
  bootloader_settings_page (r) :      ORIGIN = 0x0ff000, LENGTH = 0x100000 - 0x0ff000

  RAM (rwx) :                         ORIGIN = 0x200057b8, LENGTH = 0x3a848

  uicr_mbr_params_page (r) :          ORIGIN = 0x10001018, LENGTH = 0x4
  uicr_bootloader_start_address (r) : ORIGIN = 0x10001014, LENGTH = 0x4
}

SECTIONS
{
  .mbr_params_page(NOLOAD) :
  {
    PROVIDE(__start_mbr_params_page = .);
    KEEP(*(SORT(.mbr_params_page*)))
    PROVIDE(__stop_mbr_params_page = .);
  } > mbr_params_page
  .bootloader_settings_page(NOLOAD) :
  {
    PROVIDE(__start_bootloader_settings_page = .);
    KEEP(*(SORT(.bootloader_settings_page*)))
    PROVIDE(__stop_bootloader_settings_page = .);
  } > bootloader_settings_page
  .uicr_mbr_params_page :
  {
    PROVIDE(__start_uicr_mbr_params_page = .);
    KEEP(*(SORT(.uicr_mbr_params_page*)))
    PROVIDE(__stop_uicr_mbr_params_page = .);
  } > uicr_mbr_params_page
  .uicr_bootloader_start_address :
  {
    PROVIDE(__start_uicr_bootloader_start_address = .);
    KEEP(*(SORT(.uicr_bootloader_start_address*)))
    PROVIDE(__stop_uicr_bootloader_start_address = .);
  } > uicr_bootloader_start_address
}

SECTIONS
{
  . = ALIGN(4);
  .mem_section_dummy_ram :
  {
  }
  .log_dynamic_data :
  {
    PROVIDE(__start_log_dynamic_data = .);
    KEEP(*(SORT(.log_dynamic_data*)))
    PROVIDE(__stop_log_dynamic_data = .);
  } > RAM
  .log_filter_data :
  {
    PROVIDE(__start_log_filter_data = .);
    KEEP(*(SORT(.log_filter_data*)))
    PROVIDE(__stop_log_filter_data = .);
  } > RAM
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM

} INSERT AFTER .data;

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }
  .crypto_data :
  {
    PROVIDE(__start_crypto_data = .);
    KEEP(*(SORT(.crypto_data*)))
    PROVIDE(__stop_crypto_data = .);
  } > FLASH
    .nrf_queue :
  {
    PROVIDE(__start_nrf_queue = .);
    KEEP(*(.nrf_queue))
    PROVIDE(__stop_nrf_queue = .);
  } > FLASH
  .dfu_trans :
  {
    PROVIDE(__start_dfu_trans = .);
    KEEP(*(SORT(.dfu_trans*)))
    PROVIDE(__stop_dfu_trans = .);
  } > FLASH
    .svc_data :
  {
    PROVIDE(__start_svc_data = .);
    KEEP(*(.svc_data))
    PROVIDE(__stop_svc_data = .);
  } > FLASH
  .log_const_data :
  {
    PROVIDE(__start_log_const_data = .);
    KEEP(*(SORT(.log_const_data*)))
    PROVIDE(__stop_log_const_data = .);
  } > FLASH
    .nrf_balloc :
  {
    PROVIDE(__start_nrf_balloc = .);
    KEEP(*(.nrf_balloc))
    PROVIDE(__stop_nrf_balloc = .);
  } > FLASH
  .sdh_ble_observers :
  {
    PROVIDE(__start_sdh_ble_observers = .);
    KEEP(*(SORT(.sdh_ble_observers*)))
    PROVIDE(__stop_sdh_ble_observers = .);
  } > FLASH
  .log_backends :
  {
    PROVIDE(__start_log_backends = .);
    KEEP(*(SORT(.log_backends*)))
    PROVIDE(__stop_log_backends = .);
  } > FLASH
  .sdh_stack_observers :
  {
    PROVIDE(__start_sdh_stack_observers = .);
    KEEP(*(SORT(.sdh_stack_observers*)))
    PROVIDE(__stop_sdh_stack_observers = .);
  } > FLASH
  .sdh_req_observers :
  {
    PROVIDE(__start_sdh_req_observers = .);
    KEEP(*(SORT(.sdh_req_observers*)))
    PROVIDE(__stop_sdh_req_observers = .);
  } > FLASH
  .sdh_state_observers :
  {
    PROVIDE(__start_sdh_state_observers = .);
    KEEP(*(SORT(.sdh_state_observers*)))
    PROVIDE(__stop_sdh_state_observers = .);
  } > FLASH
  .sdh_soc_observers :
  {
    PROVIDE(__start_sdh_soc_observers = .);
    KEEP(*(SORT(.sdh_soc_observers*)))
    PROVIDE(__stop_sdh_soc_observers = .);
  } > FLASH

} INSERT AFTER .text

INCLUDE "nrf_common.ld"

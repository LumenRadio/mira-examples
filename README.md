# mira-examples

Repository containing Mira example applications.  
Contains both standalone examples and examples showcasing available toolkits in [mira-toolkit](https://github.com/LumenRadio/mira-toolkit) repository.

## General
To build the examples as is, place libmira inside vendor/.  
For examples using a mira toolkit, place the toolkit repository in vendor/.

## Dependencies

### Libmira
To be able to run any of the examples provided in this repository, `libmira` is required.

### Mira toolkit
Some of the examples requires Mira toolkit to be present in `vendor/``

### nRF5-SDK
To build examples using nRF5-SDK, [download nRF5-SDK from Nordic Semiconductor](https://www.nordicsemi.com/Products/Development-software/nrf5-sdk). Current supported version is 17.1.0.  
Note, the original directory has a name similar to `nRF5_SDK_17.1.0_ddde560` and has to be renamed to `nrf5-sdk`.  
Then build the example as described in the example's README.md.

## Examples available

| Name                                                  | Description                                                                                       |
| ---                                                   | ---                                                                                               |
| [ADC example](adc/README.md) | How to use the ADC driver |
| [BLE example](ble/README.md) | Concurrent BLE |
| [BLE beacon example](ble_beacon/README.md) | Example sending BLE beacon |
| [Blink example](blink/README.md) | GPIO blinking LEDs |
| [Blink sync client](blink_sync_client/README.md) | Synchronized blinking of LEDs |
| [Blink sync server](blink_sync_server/README.md) | Synchronized blinking of LEDs |
| [Broadcast example](broadcast/README.md)     | Showcase of [mtk_broadcast](https://github.com/LumenRadio/mira-toolkit/tree/main/mtk_broadcast)   |
| [Custom module example](custom_module/README.md) | How to register a custom module configuration |
| [Flash write example](examples/flash_write/README.md) | ---                 |
| [FOTA receiver](fota_receiver/README.md) | Demo of FOTA reception process |
| [FOTA receiver with bootloader](fota_receiver_with_bootloader/README.md) | FOTA interaction with bootloader |
| [FOTA receiver with driver](fota_receiver_with_driver/README.md) | FOTA receiver with custom storage driver |
| [FOTA sender](fota_sender/README.md) | Demo of FOTA sending process |
| [FOTA sender with driver](fota_sender_with_driver/README.md)| FOTA sender with custom storage driver |
| [License validation example](license_validation/README.md) | Use of `mira_license_*` API |
| [Monitoring example](monitoring/README.md) | Collection of network stats |
| [Network extender](network_extender/README.md) | Application agnostic range extender |
| [Network receiver example](network_receiver/README.md) | Minimal root node |
| [Network sender example](network_sender/README.md) | Minimal mesh node |
| [NFC example](nfc/README.md) | How to use NFC driver |
| [RF-slots BLE beacon example](rf_slots_ble_beacon/README.md) | How to use rf-slots API to send BLE beacons |
| [SPI master example](spi_master/README.md) | How to use SPI driver |
| [Stdout example](stdout/README.md) | Redirecting stdout |
| [UART over USB](usb_uart/README.md) | UART over USB |

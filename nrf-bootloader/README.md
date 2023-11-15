## nRF52840 bootloader

This is a combination of the nRF5 SDK 17's bootloaders for DFU over USB and DFU over BLE.

It has been modified to work with the MiraUSB's memory layout for
the Mira Gateway, Mira Packet Sniffer and Mira Network Extender.

There is also a work around added for handling the watchdog timeout.

The timeout from DFU mode has been disabled,
change `NRF_BL_DFU_INACTIVITY_TIMEOUT_MS` in `sdk_config.h` to enable it again.

### How to build
```sh
make SDKDIR=../../nrf5-sdk keys
make SDKDIR=../../nrf5-sdk
```

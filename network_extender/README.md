# Network extender example

This is an example of a network extender. The extender can be configured
using the serial interface.

## Support for DFU packages

There is support to create DFU packages for this example:
```bash
make dfu
```
To create the DFU package a private key (private.key) is generated,
which is used to sign the firmware image. A public key (dfu_public_key.c)
corresponding to the private key is also generated. The DFU Bootloader
should be built with this public key for the validation to be successful.

## DFU package for MiraUSB

LumenRadio provides a DFU package of the network extender example that
can be used on the MiraUSB. The DFU package can be found in the build
directory of this example.

### Steps to set up MiraUSB as network extender

1. Put MiraUSB module into DFU mode.

    The `reset_to_dfu.py` script available in the tools directory can
    used for this purpose:

    ``` bash
    reset_to_dfu.py -d /dev/[YOUR_MIRAUSB_SERIAL_DEVICE]
    ```

2. Upgrade the firmware with DFU (USB) using nrfutil (download from Nordic Semiconductor):

    ```bash
    nrfutil dfu usb-serial -p /dev/[YOUR_MIRAUSB_SERIAL_DEVICE] -pkg network_extender-mira_usb-nrf52840ble-os.dfu
    ```

3. Configure the MiraUSB network extender with the desired credentials using
   the `mira_network_extender_configuration.py` script provided.

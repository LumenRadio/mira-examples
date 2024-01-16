# Example of FOTA transfer with Nordic's Secure Bootloader

This example shows a complete FOTA process including upgrading
of the application (without the softdevice) by a bootloader.

This example only works on nRF52840.

The bootloader in this project is Nordic's Secure Bootloader with BLE
support.

The source code for the rest of the Secure Bootloader is in the nRF5-SDK.

Nordic has more documentation about the bootloader here:
https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.1.0/lib_bootloader_modules.html


## The process
The Mira FOTA process in detail can be found at [Mira FOTA documentation](https://docs.lumenrad.io/miraos/latest/api/common/mira_fota.html).

After the FOTA transfer is complete the application takes over and does the following:

1. The update process `fota_upgrade_process` in the application writes an updated `nrf_dfu_settings_t` to the bootloader settings flash page, and then resets.

2. After restart, the nRF52 chip enters the bootloader, it notices that
a new firmware version is available, and copies it to the application
area in flash. Once copied, the node starts the new application.


## Build

The application and the bootloader both use the nRF5-SDK. Download it first,
then set the environment variable `SDK_ROOT` to its path:
```sh
export SDK_ROOT=/path/to/sdk
```


### Build everything

The default make target builds the application, the settings for
the bootloader and the bootloader. A private/public key-pair is also 
created, if missing.

Just run:
```sh
make
```

## Flash the mesh node

Run the mesh node by flashing:
1. MBR + SoftDevice + Application
2. Bootloader
3. Bootloader settings
4. Certificate (NOTICE the address in the link script)

This is done by running:
```sh
SER=683123456
DEVID=$(mira_license.py get_device_id -s $SER)

# This installs the MBR, Softdevice, the application, the bootloader and the settings page:
make install.$SER

# The Mira License.
mira_license.py license -s $SER -I $DEVID.lic -i $DEVID -a 0xf6000 -l 4096
```

Reset the card and start the application by running:
```sh
nrfjprog -s $SER -r
```


## Test an upgrade via FOTA

Start by flashing a node and make sure it starts up by looking
at log output coming from the card's serial port.

Start up the Mira Gateway and make sure the mesh node joins the network.

Remove the comment before `#define NEW_VERSION` in `fota_receiver.c` and run
`make bin`. That creates a `0.bin` file.

Copy `0.bin` to the Mira Gateway's `firmwares` folder.

The Mira Gateway then writes something like
`FOTA: Loaded slot 0, version 158`, when it has loaded the file.
After that it is ready to be distributed.

Wait until you have a valid image at the mesh node, the node will
show a log message about it.
The transfer of this firmware takes roughly 1 minute at rate 0.
Please mind that the mesh node polls the parent at an interval of ~5 minutes.
When the image is received and is concluded to be a new version,
the node will trigger a firmware update.

The new version of the application starts up with the added print
statement: `THIS IS A NEW VERSION!`.

## Security
The first time the application builds, a private/public key-pair is created
to secure the updates. Make sure the private key (`private.key`) is kept secure.

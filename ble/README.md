## BLE example
Showcase of running BLE alongside running Mira communication.

Only nRF52 series are supported.

To test BLE together with Mira communication one need to have a root node configured also. The example network_receiver can be used together with this example.

### How to build

To be able to build this example one need to have downloaded the nrf5-sdk from Nordic Semiconductor. It is recommended to put nrf5-sdk in vendor/ and it is required to rename the folder to nrf5-sdk. It is also possible specify the location of nrf5-sdk when building by providing `SDKDIR` when running make.

To build the examples, in this directory run:
```
make TARGET=<target>
```
The example assumes that libmira and nrf5-sdk is placed in vendor/, to specify another path run:
```
make LIBDIR=<path-to-libmira> SDKDIR=<path-to-nrf5-sdk>  TARGET=<target>
```

To flash after building, add `flashall` or `flash.<programmer serial number>` to the make command:
```
make TARGET=<target> flashall
``````
or
```
make LIBDIR=<path-to-libmira> SDKDIR=<path-to-nrf5-sdk> TARGET=<target> flashall
```

### How to use

Download the "nRF blinky" app from the app store/google play and use
it to interact with the devboard.

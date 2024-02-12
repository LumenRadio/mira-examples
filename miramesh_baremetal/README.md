## MiraMesh baremetal
Runs MiraMesh without MiraOS or any other RTOS. The example is configured to have the same functionality as the `network_receiver` example.

### How to build
To be able to build this example one need to have downloaded the nrf5-sdk from Nordic Semiconductors. It is recommended to put nrf5-sdk in vendor/ and it is required to rename the folder to nrf5-sdk. It is also possible specify the location of nrf5-sdk when building by providing `SDKDIR` when running make.

To build the example, in this directory run:
```
make all
```
The example assumes that libmira and nrf5-sdk is placed in vendor/, to specify another path run:
```
make MIRA_LIB=<path-to-libmira> SDKDIR=<path-to-nrf5-sdk> TARGET=<target>
```

To flash after building, add `flash` to the make command:
```
make flash
``````
or
```
make MIRA_LIB=<path-to-libmira> SDKDIR=<path-to-nrf5-sdk> flash
```

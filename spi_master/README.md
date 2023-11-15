## SPI master example
MiraOS SPI driver supports being an SPI master. This example shows how to setup a node as an SPI master and then send and receive data.

Only nRF52 series are supported.

The example can be used by just using one node by connecting the MISO pin to the MOSI pin. The communication will then loopback into the device and the example will log both sent and received data.

MISO pin is by default port 0, pin 25 and MOSI pin is by default port 0, pin 24.

SCK pin is by default port 0, pin 23 and SS_PIN is by default port 0, pin 22.

### How to build
To build the example, in this directory run:
```
make TARGET=<target>
```
The example assumes that libmira is placed in vendor/, to specify another path run:
```
make LIBDIR=<path-to-libmira> TARGET=<target>
```

To flash after building, add `flashall` or `flash.<programmer serial number>` to the make command:
```
make TARGET=<target> flashall
``````
or
```
make LIBDIR=<path-to-libmira> TARGET=<target> flashall
```
## USB UART example
This example shows how to define a I/O method to Mira that uses the USB peripheral of the nRF52840. The I/O method can then be used as an normal POSIX file descriptor. 

Only nRF52840 is supported.

This example uses the nRF52840's USB peripheral to send UART data over USB. The file descriptor can then be used with `dprintf()` to print to the defined file descriptor.

To receive the UART over USB communication on your computer, connect the nRF USB port of the nRF52840 development kit to your computer and then connect a serial terminal to the USB device.

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
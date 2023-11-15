## FOTA sender with custom driver
This example is similar to [FOTA sender][../fota_sender/README.md] but it is extended to use a custom driver for read, write, erase and get size. This is useful if using a external storage.

However, this example mocks the driver by using a buffer in RAM to simulate the external storage.

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
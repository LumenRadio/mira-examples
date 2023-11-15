## Near Field Communication (NFC) example
This example shows how to use the NFC driver included in Mira.

The example will emulate a NFC tag that presents itself as a NFC tag type 4. The type 4 stores data as files, and it will emulate a NDEF file that can be read and written to.

By using a NFC tag reader on another device that supports NFC, it is possible to read and write to the tag emulated by the device running the example.

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
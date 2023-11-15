## BLE beacon example
This example sends a static Eddystone-URL payload at an interval.

Only nRF52 series are supported.

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

### How to use
Use any BLE beacon scanner on a bluetooth capable device to catch and read the beacon.
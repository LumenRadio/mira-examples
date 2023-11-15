## Blink synchronization client example
Uses Mira's precise time synchronization to toggle a LED on different developement boards simultaneously.

This example requires one development board to be running the `blink_sync_server`.

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
## Monitoring example
This example shows how one can collect network diagnostics per node and forward them to a root or gateway.

The format and description of what is sent in the message can be found in `monitoring.h`.

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
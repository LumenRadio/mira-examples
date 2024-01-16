## RF-slots BLE beacon example
RF-slots is the low level scheduler for the radio used in Mira, this allows multiple protocols to co-exist on the same device.

This example shows how to setup a rf-slot running at regular intervals. The slots can then be used for sending custom radio packets. In this example the rf-slot is used for sending BLE beacon packets at regular intervals.

_NOTE: Using the rf-slots API can result in breaking radio regulations_

_For more information read our [rf-slots documentation](https://docs.lumenrad.io/miraos/latest/api/common/net/mira_net_rf_slots.html)_

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

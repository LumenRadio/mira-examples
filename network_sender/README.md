## Network sender example
This example acts as an mesh node and it sends messages to the root in the network, the `network_receiever` example can be used as a root.

There can exist multiple nodes in the same network running the `network_sender` example, and messages sent from all messages will be received and printed on the node running `network_receiver`.

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
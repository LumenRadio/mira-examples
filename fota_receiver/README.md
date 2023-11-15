## FOTA receiver
Example of how firmware over the air works in Mira.

This example showcase the use of the built in FOTA capabilites in Mira. It requires running one development board using the `fota_sender` example and one or more development boards running `fota_receiver` example.

The node running `fota_sender` will generate a mock firmware that will be used to do a FOTA transfer to the `fota_reciever` nodes. When the `fota_sender` have generated the mock firmware the `fota_receiver` nodes will be able to request it. This happens automatically behind the scenes.

Nodes running `fota_sender` will log events during the duration of the transfer. It will log when the transfer started, when each individual part of the firmware is requested, when transfer gets aborted and when it is finished along with how many retries had to be done.

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
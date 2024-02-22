## Bulk data collection example
Example showcasing the tool mtk_bulk_data_collection provided in mira-toolkit.


### Principle

This example implements two nodes: Sender and Receiver (root).

```
Sender
======
Generate data ---> Signal new data

Requested for sub-packets ---> Send requested sub-packets at desired pace

Receiver
========

                                     Signaled!
                                        |
                                        v
                              Request all sub-packets
                                        |
                                        v
    +---------------------------------> o <-----------------+
   /                                    /\                   \
  /                 +------------------+  \                   \
  |                 v                      \                   \
  \           Sub-packet received           \                   \
   +------no--- All received?                \                   \
                   yes |                      v                   \
                       v                  time-out                 |
              Print and Quit    Max re-tries done? --yes--> Abort  |
                                        | no                       |
                                        v                         /
                                 Request missing sub-packets ----+
```

Sender fakes a bulk packet ready for sending every `PACKET_GENERATION_PERIOD_S`
(see `sender/bulk_data_collection_sender.c`) and notifies receiver with a signal message
(see `mtk_bdc_signal.[ch]` in the tool). This signal message includes the number of
sub-packets which constitutes the bulk packet, as well as an ID number for the
bulk packet.

Receiver listens to incoming signal messages, and replies with a request to send
the bulk packet. In this request, receiver includes a bit mask showing which
sub-packets to send, as well as the requested packet ID number and at which at
period the send sub-packets.

Sender starts sending sub-packets to receiver. It paces sub-packet transmissions
according to the requested period, in order to avoid saturating transmission
queues.

Receiver keeps track of all received sub-packets, as long as more are missing or
until a new packet ready notification arrives. Once all sub-packets are
received, receiver displays the bulk packet.

If sub-packets stop arriving before the whole bulk packet is transmitted,
receiver sends a new request, but only for the sub-packets that it lacks. This
happens on a time-out on sub-packet reception. The time-out value depends on the
requested sub-packet period.

### How to build
To build the example, in either the sender or receiver directory run:
```
make TARGET=<target>
```
The example assumes that libmira is placed in the same folder as mira-examples, to specify another path run:
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

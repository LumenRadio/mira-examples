# Network wide broadcast example

This application demonstrates how one can use the Mira toolkit mtk_broadcast to do network wide broadcast.

Mira version 2.4.0 or later is required.

## Examples
This example contains two application examples, one for a root and one that is mesh.  

### Root
The root initializes the broadcast functionality and the broadcast of the data itself.  
Data to be broadcasted is updated at a defined interval and is then broadcasted to all nodes in the network.  

### Mesh
Mesh nodes will register a listener that will listen for the broadcasted packets from the root, and when  
it receives the data to be broadcasted it will automatically re-broadcast the received data.  
  
The mesh nodes are also periodically sending unicast messages to the root, containing `"Hello Network"`.

### Dependecies
This example is dependent of the Mira toolkit `mtk_broadcast`.

### Building and flashing
To build the examples, enter either `root/` or `mesh/` and run:
If `mira-toolkit` and libmira is in `vendor/` simply run:
```
make TARGET=<target>
```
Otherwise change the path in the Makefiles or run:
```
make LIBDIR=<path-to-libmira> MTKDIR=<path-to-mira-toolkit> TARGET=<target>
```

To flash after building, add `flashall` or `flash.<programmer serial number>` to the make command:
```
make TARGET=<target> flashall
``````
or
```
make LIBDIR=<path-to-libmira> MTKDIR=<path-to-mira-toolkit> TARGET=<target> flashall
```
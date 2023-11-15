## Analog-to-digital converter (ADC) example
This example showcases the use of the ADC peripheral driver provided with MiraOS.

Only nRF52 series are supported.

The input voltage is measured on port 0, pin 4. 
It then uses a internal reference voltage of 3.6 V to calculate the voltage on port 0, pin 4.

### Building and flashing
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
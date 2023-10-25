# mira-examples

Repository containing Mira example applications.  
Contains both standalone examples and examples showcasing available toolkits in [mira-toolkit](https://github.com/LumenRadio/mira-toolkit) repository.

## General
To build the examples as is, place libmira inside vendor/.  
For examples using a mira toolkit, place the toolkit repository in vendor/.

## Dependencies

### Libmira
To be able to run any of the examples provided in this repository, `libmira` is required.

### Mira toolkit
Some of the examples requires Mira toolkit to be present in `vendor/``

### nRF5-SDK
To build examples using nRF5-SDK, [download nRF5-SDK from Nordic Semiconductor](https://www.nordicsemi.com/Products/Development-software/nrf5-sdk). Current supported version is 17.1.0.  
Note, the original directory has a name similar to `nRF5_SDK_17.1.0_ddde560` and has to be renamed to `nrf5-sdk`.  
Then build the example as described in the example's README.md.

## Examples available

| Name                                                  | Description                                                                                       |
| ---                                                   | ---                                                                                               |
| [Broadcast example](examples/broadcast/README.md)     | Showcase of [mtk_broadcast](https://github.com/LumenRadio/mira-toolkit/tree/main/mtk_broadcast)   |

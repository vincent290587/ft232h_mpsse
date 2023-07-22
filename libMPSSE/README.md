<p align="center">
  <h1 align="center">libMPSSE</h1>
  <p align="center">
    FTDI libMPSSE v0.6
  </p>
</p>

## Getting started
To get started with the libMPSSE library you will need a couple pieces of software and a piece of hardware.
-   [CMake](https://cmake.org/download/)
-   [FTDI D2xx driver](https://ftdichip.com/drivers/d2xx-drivers/)
-   [MPSSE capable FTDI device](https://ftdichip.com/product-category/products/)

### Building the libMPSSE static library
To build a static library that can be linked against in your application. The static library can be found in the newly created **lib** folder.
```bash
$ mkdir -p build
$ cd build
$ cmake ..
$ make -j8
```

### Building the sample applications
Once you have compiled the static library, you can generate a sample application which shows the connected FTDI devices with compatible MPSSE modules. The resulting sample binary can be found in the newly created **output** folder.

```bash
$ cd example/SPI
$ mkdir -p build
# Debug build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
# OR
# Release build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j8
```

### Integrating the libMPSSE static library
To integrate the libMPSSE.a static lib, you will need to link your application executable against the generated **.a** file, and include the **inc** folder.
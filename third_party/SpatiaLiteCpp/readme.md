
# SpatiaLiteC++

SpatiaLiteC++ is a C++ wrapper for the SpatiaLite library to provide RAII management.

## Goal

The main goal of SpatiaLiteC++ is to provide a simple and clean
encapsulation of the dynamically allocated data structures in
[SpatiaLite](http://www.gaia-gis.it/gaia-sins/). It is not a complete C++
wrapper to the SpatiaLite C API but only to those data structures that were
dynamically allocated with malloc or one of the various gaiaAlloc methods in the
SpatiaLite. Additionally, SpatiaLiteC++ uses
[SQLite++](https://github.com/SRombauts/SQLiteCpp) to provide RAII management to
the underlying SQLite database used by SpatiaLite. SpatiaLiteC++ aims to provide
complete memory management of all SpatiaLite data structures and minimize
memory leaks.

## Documentation

The following help is available at the SpatiaLiteC++
[website](http://dpmcmlxxvi.github.io/SpatiaLiteCpp):

- [Introduction](http://dpmcmlxxvi.github.io/SpatiaLiteCpp/web/)
- [Quick Start](http://dpmcmlxxvi.github.io/SpatiaLiteCpp/web/start.html)
- [Examples](http://dpmcmlxxvi.github.io/SpatiaLiteCpp/web/demos.html)
- [API](http://dpmcmlxxvi.github.io/SpatiaLiteCpp/api/)

## License

Copyright (c) 2015 Daniel Pulido [dpmcmlxxvi@gmail.com][email]

SpatiaLiteC++ is released under the [MIT License][license]

[email]: mailto:dpmcmlxxvi@gmail.com
[license]: http://opensource.org/licenses/MIT

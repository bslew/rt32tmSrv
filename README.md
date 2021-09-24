# General

This package implements UT1 time distribution for 32-m radio telescope 
in Piwnice (Poland). It also stores actual values of polar angles and 
leap seconds count.

The time is distributed via UDP datagrams with dUT1 corrections.

## Comments
Dedicated scripts can be used to provide 
the actual data for earth orientation
fetched from 
https://datacenter.iers.org/data/latestVersion/6_BULLETIN_A_V2013_016.txt
or 
https://cddis.nasa.gov/archive/products/


# Requirements

Libraries required:
 - spdlog
 - yaml-cpp
 - boost-filesystem, boost-program-options

# BUILD
```sh
cmake . && make
```

#AUTHOR
Bartosz Lew [<bartosz.lew@protonmail.com>](bartosz.lew@protonmail.com)



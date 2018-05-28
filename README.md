# pymeshb

[libMeshb](https://github.com/LoicMarechal/libMeshb) Python 2&3 wrapper.

# Overview

The **pymeshb** Python module provides read/write functions
to handle the \*.meshb file format. It is simply a Python wrapper
of [libMeshb](https://github.com/LoicMarechal/libMeshb).

# Installation

## From the Python Package Index

Please make sure that [numpy](http://www.numpy.org) is installed by typing

    pip install -U numpy

pymeshb is [available from the Python Package
Index](https://pypi.org/project/pymeshb/), so simply type

    pip install -U pymeshb

to install or upgrade.

## From sources

    pip install -U numpy
    git clone https://github.com/jvanharen/pymeshb
    cd pymeshb
    git submodule init
    git submodule update
    python setup.py install

# Usage

```Python
import pymeshb
msh = pymeshb.read('in.meshb')
pymeshb.write(msh, 'out.meshb')
```

The read function simply returns a dictionary whose the keys
are the keywords of the libmeshb library.

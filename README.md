# pymeshb
[libMeshb](https://github.com/LoicMarechal/libMeshb) Python wrapper.

# Overview
The **pymeshb** Python module provides read/write functions
to handle the *.meshb file format. It is simply a Python wrapper
of [libMeshb](https://github.com/LoicMarechal/libMeshb).

# Install
Simply do:
`pip install pymeshb`

# Usage
```Python
import pymeshb
msh = pymeshb.read('in.meshb')
pymeshb.write(msh, 'out.meshb')
```

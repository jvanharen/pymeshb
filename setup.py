import os, numpy
from distutils.core import setup, Extension

libmeshb_path = os.path.join('libmeshb',    'sources')
libmeshb_c = os.path.join(libmeshb_path, 'libmeshb7.c')
libmeshb_h = os.path.join(libmeshb_path, 'libmeshb7.h')

pymeshb = Extension('pymeshb', ['libmeshb_wrap.c', libmeshb_c], include_dirs=[
                    libmeshb_path, numpy.get_include()])
setup(name='pymeshb', ext_modules=[pymeshb], version='1.0', description='LibMeshb Python wrapper',
      author='Julien Vanharen', author_email='julien.vanharen@inria.fr')
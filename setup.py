import os
import numpy
import setuptools
from distutils.core import Extension


def readme():
    with open('README.md') as f:
        return f.read()


libmeshb_path = os.path.join('libmeshb', 'sources')
libmeshb_c = os.path.join(libmeshb_path, 'libmeshb7.c')
libmeshb_h = os.path.join(libmeshb_path, 'libmeshb7.h')

pymeshb = Extension('pymeshb', ['libmeshb_wrap.c', libmeshb_c], depends=[libmeshb_h], include_dirs=[
                    libmeshb_path, numpy.get_include()], extra_compile_args=["-DTRANSMESH"], )

setuptools.setup(name='pymeshb',
                 version='0.0.3',
                 url='https://github.com/jvanharen/pymeshb',
                 author='Julien Vanharen',
                 author_email='julien.vanharen@inria.fr',
                 description='LibMeshb Python wrapper to read/write *.mesh[b]/*.sol[b] file.',
                 long_description=readme(),
                 long_description_content_type='text/markdown',
                 ext_modules=[pymeshb],
                 packages=setuptools.find_packages(),
                 install_requires=['numpy>=1.13.3'],
                 classifiers=["License :: OSI Approved :: MIT License",
                              'Operating System :: OS Independent',
                              'Programming Language :: Python',
                              'Programming Language :: Python :: 2',
                              'Programming Language :: Python :: 3',
                              'Topic :: Scientific/Engineering']
                 )

import os
import numpy
from setuptools import setup, Extension, find_packages


def readme():
    with open('README.md') as f:
        return f.read()


libmeshb_path = os.path.join('libmeshb', 'sources')
libmeshb_c = os.path.join(libmeshb_path, 'libmeshb7.c')

pymeshb = Extension('pymeshb', ['libmeshb_wrap.c', libmeshb_c], include_dirs=[
                    libmeshb_path, numpy.get_include()], extra_compile_args=["-DTRANSMESH"], )

setup(name='pymeshb',
      version='0.0.4',
      url='https://github.com/jvanharen/pymeshb',
      author='Julien Vanharen',
      author_email='julien.vanharen@inria.fr',
      description='LibMeshb Python wrapper to read/write *.mesh[b]/*.sol[b] file.',
      long_description=readme(),
      long_description_content_type='text/markdown',
      ext_modules=[pymeshb],
      packages=find_packages(),
      install_requires=['numpy>=1.13.3'],
      classifiers=["License :: OSI Approved :: MIT License",
                   'Operating System :: OS Independent',
                   'Programming Language :: Python',
                   'Programming Language :: Python :: 2',
                   'Programming Language :: Python :: 3',
                   'Topic :: Scientific/Engineering']
      )

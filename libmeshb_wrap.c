#include "Python.h"
#include "libmeshb7.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

static PyObject *pymeshb_read(PyObject *self, PyObject *args) {
  return Py_BuildValue("i", 1);
}

static PyObject *pymeshb_write(PyObject *self, PyObject *args) {
  return Py_BuildValue("i", 1);
}

// --- Module method table --- //
static PyMethodDef Methods[] = {
    {"read", pymeshb_read, METH_VARARGS, "Read *.meshb file"},
    {"write", pymeshb_write, METH_VARARGS, "Write *.meshb file"},
    {NULL, NULL, 0, NULL}};

// --- Module initialization --- //
#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef pymeshb = {PyModuleDef_HEAD_INIT, "pymeshb",
                                     "LibMeshb Python wrapper", -1, Methods};
#endif

PyMODINIT_FUNC
#if PY_MAJOR_VERSION >= 3
PyInit_pymeshb(void) {
#else
initpymeshb(void) {
#endif
  import_array();
#if PY_MAJOR_VERSION >= 3
  return PyModule_Create(&pymeshb);
#else
  (void)Py_InitModule("pymeshb", Methods);
#endif
}

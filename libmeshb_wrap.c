#include "Python.h"
#include "libmeshb7.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

static PyObject *pymeshb_read(PyObject *self, PyObject *args) {
  // --- C variables declaration. --- //
  char *Inp, str[GmfStrSiz];
  int i, j, Ver, Dim, KwdCod, NmbInt, NmbDbl, StrSiz;
  int64_t FID, IntTab[GmfMaxTyp], NmbElt, *CIntTab;
  double DblTab[GmfMaxTyp], *CDblTab;

  // --- Python variables declaration. --- //
  npy_intp NpyIntTabDim[2], NpyDblTabDim[2];
  PyObject *NpyIntTab, *NpyDblTab, *lst;
  PyObject *out = PyDict_New();

  // --- Get file name. --- //
  if (!PyArg_ParseTuple(args, "s", &Inp))
    return NULL;

  // --- Open file. --- //
  if (!(FID = GmfOpenMesh(Inp, GmfRead, &Ver, &Dim)))
    return NULL;

  // --- Loop on all keywords for a generic reading. --- //
  for (KwdCod = 1; KwdCod <= GmfMaxKwd; KwdCod++) {
    NmbElt = GmfStatKwd(FID, KwdCod);
    if (NmbElt > 0) {
      GmfGotoKwd(FID, KwdCod);
      GmfGetLinTab(FID, KwdCod, IntTab, &NmbInt, DblTab, &NmbDbl, str, &StrSiz);
      if (NmbInt > 0 || NmbDbl > 0) {
        CIntTab = malloc(NmbElt * NmbInt * sizeof(int64_t));
        CDblTab = malloc(NmbElt * NmbDbl * sizeof(double));
        for (i = 0; i < NmbInt; i++)
          CIntTab[i] = IntTab[i];
        for (i = 0; i < NmbDbl; i++)
          CDblTab[i] = DblTab[i];
        for (i = 2; i <= NmbElt; i++) {
          // --- Reading each line in FID. --- //
          GmfGetLinTab(FID, KwdCod, IntTab, &NmbInt, DblTab, &NmbDbl, str,
                       &StrSiz);
          for (j = 0; j < NmbInt; j++)
            CIntTab[(i - 1) * NmbInt + j] = IntTab[j];
          for (j = 0; j < NmbDbl; j++)
            CDblTab[(i - 1) * NmbDbl + j] = DblTab[j];
        }

        // --- Send C arrays to Python Numpy arrays. --- //
        NpyIntTabDim[0] = NpyDblTabDim[0] = NmbElt;
        NpyIntTabDim[1] = NmbInt;
        NpyDblTabDim[1] = NmbDbl;

        NpyIntTab =
            PyArray_SimpleNewFromData(2, NpyIntTabDim, NPY_INT64, CIntTab);
        PyArray_ENABLEFLAGS((PyArrayObject *)NpyIntTab, NPY_ARRAY_OWNDATA);
        NpyDblTab =
            PyArray_SimpleNewFromData(2, NpyDblTabDim, NPY_DOUBLE, CDblTab);
        PyArray_ENABLEFLAGS((PyArrayObject *)NpyDblTab, NPY_ARRAY_OWNDATA);

        // --- Make a list if there are int64 and double arrays. --- //
        if (NmbDbl > 0 && NmbInt > 0) {
          lst = PyList_New(0);
          PyList_Append(lst, NpyDblTab);
          PyList_Append(lst, NpyIntTab);
          PyDict_SetItemString(out, GmfKwdFmt[KwdCod][0], lst);
          // --- Allow the Python garbage collector to free this list. --- //
          Py_DECREF(lst);
        }
        if (NmbDbl == 0)
          PyDict_SetItemString(out, GmfKwdFmt[KwdCod][0], NpyIntTab);
        if (NmbInt == 0)
          PyDict_SetItemString(out, GmfKwdFmt[KwdCod][0], NpyDblTab);

        // --- Allow the Python garbage collector to free these arrays. --- //
        Py_DECREF(NpyIntTab);
        Py_DECREF(NpyDblTab);
      }
    }
  }

  // --- Close mesh. ---
  GmfCloseMesh(FID);

  // --- Return a dictionary. --- //
  // -- The keys are the keywords of the libmeshb library. --- //
  return out;
}

static PyObject *pymeshb_write(PyObject *self, PyObject *args) {
  return Py_BuildValue("i", 1);
}

// --- Module methods table --- //
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

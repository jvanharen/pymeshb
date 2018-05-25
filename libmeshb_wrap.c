#include "Python.h"
#include "libmeshb7.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#define SizVec(Dim) (Dim)
#define SizMat(Dim) (Dim == 2 ? 4 : 9)
#define SizSymMat(Dim) (Dim == 2 ? 3 : 6)

static PyObject *pymeshb_read_msh(char *);
static PyObject *pymeshb_read_sol(char *);
static PyObject *pymeshb_write_msh(PyObject *, char *, int);
static PyObject *pymeshb_write_sol(PyObject *, char *, int);

// ########################################################################## //
// ###### GENERIC FUNCTIONS ################################################# //
// ########################################################################## //
static PyObject *pymeshb_read(PyObject *self, PyObject *args) {
  char *Inp;
  PyObject *out;

  // --- Get file name. --- //
  if (!PyArg_ParseTuple(args, "s", &Inp))
    return NULL;

  if (strstr(Inp, ".mesh"))
    out = pymeshb_read_msh(Inp);
  else if (strstr(Inp, ".sol"))
    out = pymeshb_read_sol(Inp);
  else {
    PyErr_SetString(
        PyExc_RuntimeError,
        "The provided file name does not contain .mesh[b] or .sol[b].");
    return NULL;
  }

  return out;
}

static PyObject *pymeshb_write(PyObject *self, PyObject *args,
                               PyObject *kwargs) {
  static char *kwlist[] = {"in", "out", "version", NULL};
  int Ver = 3;
  PyObject *in;
  char *Out;

  // --- Get the dictionary and the output file name. --- //
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Os|i", kwlist, &in, &Out,
                                   &Ver))
    return NULL;

  if (strstr(Out, ".mesh"))
    pymeshb_write_msh(in, Out, Ver);
  else if (strstr(Out, ".sol"))
    pymeshb_write_sol(in, Out, Ver);
  else {
    PyErr_SetString(
        PyExc_RuntimeError,
        "The provided file name does not contain .mesh[b] or .sol[b].");
    return NULL;
  }
  return Py_BuildValue("i", 1);
}

// ########################################################################## //
// ###### MESH AND SOLUTION READERS ######################################### //
// ########################################################################## //
static PyObject *pymeshb_read_msh(char *Inp) {
  // --- C variables declaration. --- //
  char str[GmfStrSiz];
  int i, j, Ver, Dim, KwdCod, NmbInt, NmbDbl, StrSiz;
  int64_t FID, IntTab[GmfMaxTyp], NmbElt, *CIntTab;
  double DblTab[GmfMaxTyp], *CDblTab;

  // --- Python variables declaration. --- //
  npy_intp NpyIntTabDim[2], NpyDblTabDim[2];
  PyObject *NpyIntTab, *NpyDblTab, *lst;
  PyObject *out = PyDict_New();

  // --- Open file. --- //
  if (!(FID = GmfOpenMesh(Inp, GmfRead, &Ver, &Dim))) {
    PyErr_SetString(PyExc_RuntimeError, "The file does not exist.");
    return NULL;
  }

  // --- Set dimension. --- //
  PyObject *pyDim = Py_BuildValue("i", Dim);
  PyDict_SetItemString(out, "Dimension", pyDim);
  Py_DECREF(pyDim);

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

static PyObject *pymeshb_read_sol(char *Inp) {
  // --- C variables declaration. --- //
  const char *KwdFmt;
  char str[GmfStrSiz];
  int SolSiz, NmbTyp, TypTab[GmfMaxTyp];
  int i, j, k, Ver, Dim, KwdCod, NmbInt, NmbDbl, StrSiz;
  int64_t FID, IntTab[GmfMaxTyp], NmbSol, *CIntTab, idxSol;
  double DblTab[GmfMaxTyp], *CDblTab, *PtrSol[GmfMaxTyp];

  // --- Python variables declaration. --- //
  npy_intp NpySolTabDim[2];
  PyObject *NpySolTab[GmfMaxTyp];
  npy_intp NpyIntTabDim[2], NpyDblTabDim[2];
  PyObject *NpyIntTab, *NpyDblTab, *lst;
  PyObject *out = PyDict_New();

  // --- Open file. --- //
  if (!(FID = GmfOpenMesh(Inp, GmfRead, &Ver, &Dim))) {
    PyErr_SetString(PyExc_RuntimeError, "The file does not exist.");
    return NULL;
  }

  // --- Set dimension. --- //
  PyObject *pyDim = Py_BuildValue("i", Dim);
  PyDict_SetItemString(out, "Dimension", pyDim);
  Py_DECREF(pyDim);

  // --- Loop on all keywords for a generic reading. --- //
  for (KwdCod = 1; KwdCod <= GmfMaxKwd; KwdCod++) {
    KwdFmt = GmfKwdFmt[KwdCod][2];
    NmbSol = GmfStatKwd(FID, KwdCod, &NmbTyp, &SolSiz, TypTab);
    if (NmbSol > 0) {
      GmfGotoKwd(FID, KwdCod);
      GmfGetLinTab(FID, KwdCod, IntTab, &NmbInt, DblTab, &NmbDbl, str, &StrSiz);
      if (!strcmp(KwdFmt, "sr") || !strcmp(KwdFmt, "hr")) {
        idxSol = 0;
        for (j = 0; j < NmbTyp; j++) {
          switch (TypTab[j]) {
          case GmfSca:
            PtrSol[j] = malloc(NmbSol * 1 * sizeof(double));
            PtrSol[j][0] = DblTab[idxSol++];
            break;
          case GmfVec:
            PtrSol[j] = malloc(NmbSol * Dim * sizeof(double));
            for (k = 0; k < Dim; k++)
              PtrSol[j][k] = DblTab[idxSol++];
            break;
          case GmfSymMat:
            PtrSol[j] = malloc(NmbSol * (Dim * (Dim + 1)) / 2 * sizeof(double));
            for (k = 0; k < (Dim * (Dim + 1)) / 2; k++)
              PtrSol[j][k] = DblTab[idxSol++];
            break;
          case GmfMat:
            PtrSol[j] = malloc(NmbSol * Dim * Dim * sizeof(double));
            for (k = 0; k < Dim * Dim; k++)
              PtrSol[j][k] = DblTab[idxSol++];
            break;
          }
        }

        for (i = 2; i <= NmbSol; i++) {
          // --- Reading each line in FID. --- //
          GmfGetLinTab(FID, KwdCod, IntTab, &NmbInt, DblTab, &NmbDbl, str,
                       &StrSiz);
          idxSol = 0;
          for (j = 0; j < NmbTyp; j++) {
            switch (TypTab[j]) {
            case GmfSca:
              PtrSol[j][i - 1] = DblTab[idxSol++];
              break;
            case GmfVec:
              for (k = 0; k < Dim; k++)
                PtrSol[j][(i - 1) * Dim + k] = DblTab[idxSol++];
              break;
            case GmfSymMat:
              for (k = 0; k < (Dim * (Dim + 1)) / 2; k++)
                PtrSol[j][(i - 1) * (Dim * (Dim + 1)) / 2 + k] =
                    DblTab[idxSol++];
              break;
            case GmfMat:
              for (k = 0; k < Dim * Dim; k++)
                PtrSol[j][(i - 1) * Dim * Dim + k] = DblTab[idxSol++];
              break;
            }
          }
        }

        // --- Send C arrays to Python Numpy arrays. --- //
        NpySolTabDim[0] = NmbSol;
        for (j = 0; j < NmbTyp; j++) {
          switch (TypTab[j]) {
          case GmfSca:
            NpySolTabDim[1] = 1;
            NpySolTab[j] = PyArray_SimpleNewFromData(1, NpySolTabDim,
                                                     NPY_DOUBLE, PtrSol[j]);
            PyArray_ENABLEFLAGS((PyArrayObject *)NpySolTab[j],
                                NPY_ARRAY_OWNDATA);
            break;
          case GmfVec:
            NpySolTabDim[1] = Dim;
            NpySolTab[j] = PyArray_SimpleNewFromData(2, NpySolTabDim,
                                                     NPY_DOUBLE, PtrSol[j]);
            PyArray_ENABLEFLAGS((PyArrayObject *)NpySolTab[j],
                                NPY_ARRAY_OWNDATA);
            break;
          case GmfSymMat:
            NpySolTabDim[1] = (Dim * (Dim + 1)) / 2;
            NpySolTab[j] = PyArray_SimpleNewFromData(2, NpySolTabDim,
                                                     NPY_DOUBLE, PtrSol[j]);
            PyArray_ENABLEFLAGS((PyArrayObject *)NpySolTab[j],
                                NPY_ARRAY_OWNDATA);
            break;
          case GmfMat:
            NpySolTabDim[1] = Dim * Dim;
            NpySolTab[j] = PyArray_SimpleNewFromData(2, NpySolTabDim,
                                                     NPY_DOUBLE, PtrSol[j]);
            PyArray_ENABLEFLAGS((PyArrayObject *)NpySolTab[j],
                                NPY_ARRAY_OWNDATA);
            break;
          }
        }

        lst = PyList_New(0);
        for (j = 0; j < NmbTyp; j++)
          PyList_Append(lst, NpySolTab[j]);
        PyDict_SetItemString(out, GmfKwdFmt[KwdCod][0], lst);
        // --- Allow the Python garbage collector to free this list. --- //
        Py_DECREF(lst);

        // --- Allow the Python garbage collector to free these arrays. ---
        for (j = 0; j < NmbTyp; j++)
          Py_DECREF(NpySolTab[j]);
      } else {
        if (NmbInt > 0 || NmbDbl > 0) {
          CIntTab = malloc(NmbSol * NmbInt * sizeof(int64_t));
          CDblTab = malloc(NmbSol * NmbDbl * sizeof(double));
          for (i = 0; i < NmbInt; i++)
            CIntTab[i] = IntTab[i];
          for (i = 0; i < NmbDbl; i++)
            CDblTab[i] = DblTab[i];
          for (i = 2; i <= NmbSol; i++) {
            // --- Reading each line in FID. --- //
            GmfGetLinTab(FID, KwdCod, IntTab, &NmbInt, DblTab, &NmbDbl, str,
                         &StrSiz);
            for (j = 0; j < NmbInt; j++)
              CIntTab[(i - 1) * NmbInt + j] = IntTab[j];
            for (j = 0; j < NmbDbl; j++)
              CDblTab[(i - 1) * NmbDbl + j] = DblTab[j];
          }

          // --- Send C arrays to Python Numpy arrays. --- //
          NpyIntTabDim[0] = NpyDblTabDim[0] = NmbSol;
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
  }

  // --- Close solution. ---
  GmfCloseMesh(FID);

  // --- Return a dictionary. --- //
  // -- The keys are the keywords of the libmeshb library. --- //
  return out;
}

// ########################################################################## //
// ###### MESH AND SOLUTION WRITERS ######################################### //
// ########################################################################## //
static PyObject *pymeshb_write_msh(PyObject *in, char *Out, int Ver) {
  // --- C variables declaration. --- //
  int i, typ, Dim, KwdCod;
  int64_t FID, NmbElt;

  // --- Python variables declaration. --- //
  PyObject *val;
  npy_intp *NpyIntTabDim, *NpyDblTabDim;
  PyArrayObject *NpyIntTab, *NpyDblTab;

  // --- Open file. --- //
  Dim = (int)PyLong_AsLong(PyDict_GetItemString(in, "Dimension"));
  if (!(FID = GmfOpenMesh(Out, GmfWrite, Ver, Dim))) {
    PyErr_SetString(PyExc_RuntimeError, "The file does not exist.");
    return NULL;
  }

  // --- Loop on all keywords for a generic writing. --- //
  for (KwdCod = 1; KwdCod <= GmfMaxKwd; KwdCod++) {
    val = PyDict_GetItemString(in, GmfKwdFmt[KwdCod][0]);
    if (val != NULL) {
      if (PyList_Check(val)) {
        NpyDblTab = (PyArrayObject *)PyArray_FROM_OTF(
            PyList_GetItem(val, 0), NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
        NpyDblTabDim = PyArray_DIMS(NpyDblTab);
        NpyIntTab = (PyArrayObject *)PyArray_FROM_OTF(
            PyList_GetItem(val, 1), NPY_INT64, NPY_ARRAY_IN_ARRAY);
        NpyIntTabDim = PyArray_DIMS(NpyIntTab);

        if (NpyDblTabDim[0] != NpyIntTabDim[0]) {
          PyErr_SetString(PyExc_RuntimeError, "Double array dimensions and Int "
                                              "array dimensions are "
                                              "different.");
          return NULL;
        } else
          NmbElt = NpyDblTabDim[0];
        GmfSetKwd(FID, KwdCod, NmbElt);
        for (i = 0; i < NmbElt; i++) {
          // --- Writing each line in FID. --- //
          GmfSetLinTab(FID, KwdCod, PyArray_GETPTR2(NpyIntTab, i, 0),
                       PyArray_GETPTR2(NpyDblTab, i, 0), NULL);
        }

        // --- Allow the Python garbage collector to free these arrays. --- //
        Py_DECREF(NpyIntTab);
        Py_DECREF(NpyDblTab);
        Py_DECREF(NpyIntTabDim);
        Py_DECREF(NpyDblTabDim);
      }
      if (PyArray_Check(val)) {
        typ = PyArray_TYPE((PyArrayObject *)val);
        switch (typ) {
        case NPY_INT64:
          NpyIntTab = (PyArrayObject *)PyArray_FROM_OTF(val, NPY_INT64,
                                                        NPY_ARRAY_IN_ARRAY);
          NpyIntTabDim = PyArray_DIMS(NpyIntTab);
          NmbElt = NpyIntTabDim[0];
          GmfSetKwd(FID, KwdCod, NmbElt);
          for (i = 0; i < NmbElt; i++) {
            // --- Writing each line in FID. --- //
            GmfSetLinTab(FID, KwdCod, PyArray_GETPTR2(NpyIntTab, i, 0), NULL,
                         NULL);
          }
          // --- Allow the Python garbage collector to free these arrays. --- //
          Py_DECREF(NpyIntTab);
          Py_DECREF(NpyIntTabDim);
          break;
        case NPY_DOUBLE:
          NpyDblTab = (PyArrayObject *)PyArray_FROM_OTF(val, NPY_DOUBLE,
                                                        NPY_ARRAY_IN_ARRAY);
          NpyDblTabDim = PyArray_DIMS(NpyDblTab);
          NmbElt = NpyDblTabDim[0];
          GmfSetKwd(FID, KwdCod, NmbElt);
          for (i = 0; i < NmbElt; i++) {
            // --- Writing each line in FID. --- //
            GmfSetLinTab(FID, KwdCod, NULL, PyArray_GETPTR2(NpyDblTab, i, 0),
                         NULL);
          }
          // --- Allow the Python garbage collector to free these arrays. --- //
          Py_DECREF(NpyDblTab);
          Py_DECREF(NpyDblTabDim);
          break;
        default:
          PyErr_SetString(PyExc_RuntimeError, "Unknown type of array.");
          return NULL;
        }
      }
    }
  }

  // --- Close mesh. ---
  GmfCloseMesh(FID);

  // --- Return 1 for success. --- //
  return Py_BuildValue("i", 1);
}

static PyObject *pymeshb_write_sol(PyObject *in, char *Out, int Ver) {
  // --- C variables declaration. --- //
  const char *KwdFmt;
  int i, j, k, Dim, KwdCod, NmbTyp, PtrSolNDim[GmfMaxTyp], TypTab[GmfMaxTyp];
  int SolBufIdx, Siz, typ;
  int64_t FID, NmbElt;
  double SolBuf[GmfMaxTyp];

  // --- Python variables declaration. --- //
  PyObject *val;
  npy_intp *NpyPtrSolDim[GmfMaxTyp];
  npy_intp *NpyIntTabDim, *NpyDblTabDim;
  PyArrayObject *NpyIntTab, *NpyDblTab;
  PyArrayObject *NpyPtrSol[GmfMaxTyp];

  // --- Open file. --- //
  Dim = (int)PyLong_AsLong(PyDict_GetItemString(in, "Dimension"));
  if (!(FID = GmfOpenMesh(Out, GmfWrite, Ver, Dim))) {
    PyErr_SetString(PyExc_RuntimeError, "The file does not exist.");
    return NULL;
  }

  // --- Loop on all keywords for a generic writing. --- //
  for (KwdCod = 1; KwdCod <= GmfMaxKwd; KwdCod++) {
    val = PyDict_GetItemString(in, GmfKwdFmt[KwdCod][0]);
    if (val != NULL) {
      KwdFmt = GmfKwdFmt[KwdCod][2];
      if (!strcmp(KwdFmt, "sr") || !strcmp(KwdFmt, "hr")) {
        if (PyList_Check(val)) {
          NmbTyp = PyList_Size(val);
          for (i = 0; i < NmbTyp; i++) {
            NpyPtrSol[i] = (PyArrayObject *)PyList_GetItem(val, i);
            PtrSolNDim[i] = PyArray_NDIM(NpyPtrSol[i]);
            NpyPtrSolDim[i] = PyArray_DIMS(NpyPtrSol[i]);
            if (PtrSolNDim[i] == 1)
              TypTab[i] = GmfSca;
            else {
              if (NpyPtrSolDim[i][1] == SizVec(Dim))
                TypTab[i] = GmfVec;
              else if (NpyPtrSolDim[i][1] == SizSymMat(Dim))
                TypTab[i] = GmfSymMat;
              else if (NpyPtrSolDim[i][1] == SizMat(Dim))
                TypTab[i] = GmfMat;
              else {
                PyErr_SetString(PyExc_RuntimeError,
                                "Unknown type of solution.");
                return NULL;
              }
            }
          }
        }

        NmbElt = NpyPtrSolDim[0][0];
        for (i = 0; i < NmbTyp; i++) {
          if (NmbElt != NpyPtrSolDim[i][0]) {
            PyErr_SetString(
                PyExc_RuntimeError,
                "The number of elements for each solution field is different.");
            return NULL;
          }
        }

        GmfSetKwd(FID, KwdCod, NmbElt, NmbTyp, TypTab);
        for (i = 0; i < NmbElt; i++) {
          SolBufIdx = 0;
          for (j = 0; j < NmbTyp; j++) {
            if (PtrSolNDim[j] == 1)
              SolBuf[SolBufIdx++] =
                  *((double *)PyArray_GETPTR1(NpyPtrSol[j], i));
            else {
              switch (TypTab[j]) {
              case GmfVec:
                Siz = SizVec(Dim);
                break;
              case GmfSymMat:
                Siz = SizSymMat(Dim);
                break;
              case GmfMat:
                Siz = SizMat(Dim);
                break;
              }
              for (k = 0; k < Siz; k++) {
                SolBuf[SolBufIdx++] =
                    *((double *)PyArray_GETPTR2(NpyPtrSol[j], i, k));
              }
            }
          }
          // --- Writing each line in FID. --- //
          GmfSetLinTab(FID, KwdCod, NULL, SolBuf, NULL);
        }
      } else {
        if (PyList_Check(val)) {
          NpyDblTab = (PyArrayObject *)PyArray_FROM_OTF(
              PyList_GetItem(val, 0), NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
          NpyDblTabDim = PyArray_DIMS(NpyDblTab);
          NpyIntTab = (PyArrayObject *)PyArray_FROM_OTF(
              PyList_GetItem(val, 1), NPY_INT64, NPY_ARRAY_IN_ARRAY);
          NpyIntTabDim = PyArray_DIMS(NpyIntTab);

          if (NpyDblTabDim[0] != NpyIntTabDim[0]) {
            PyErr_SetString(PyExc_RuntimeError,
                            "Double array dimensions and Int "
                            "array dimensions are "
                            "different.");
            return NULL;
          } else
            NmbElt = NpyDblTabDim[0];
          GmfSetKwd(FID, KwdCod, NmbElt);
          for (i = 0; i < NmbElt; i++) {
            // --- Writing each line in FID. --- //
            GmfSetLinTab(FID, KwdCod, PyArray_GETPTR2(NpyIntTab, i, 0),
                         PyArray_GETPTR2(NpyDblTab, i, 0), NULL);
          }

          // --- Allow the Python garbage collector to free these arrays. --- //
          Py_DECREF(NpyIntTab);
          Py_DECREF(NpyDblTab);
          Py_DECREF(NpyIntTabDim);
          Py_DECREF(NpyDblTabDim);
        }
        if (PyArray_Check(val)) {
          typ = PyArray_TYPE((PyArrayObject *)val);
          switch (typ) {
          case NPY_INT64:
            NpyIntTab = (PyArrayObject *)PyArray_FROM_OTF(val, NPY_INT64,
                                                          NPY_ARRAY_IN_ARRAY);
            NpyIntTabDim = PyArray_DIMS(NpyIntTab);
            NmbElt = NpyIntTabDim[0];
            GmfSetKwd(FID, KwdCod, NmbElt);
            for (i = 0; i < NmbElt; i++) {
              // --- Writing each line in FID. --- //
              GmfSetLinTab(FID, KwdCod, PyArray_GETPTR2(NpyIntTab, i, 0), NULL,
                           NULL);
            }
            // --- Allow the Python garbage collector to free these arrays. ---
            Py_DECREF(NpyIntTab);
            break;
          case NPY_DOUBLE:
            NpyDblTab = (PyArrayObject *)PyArray_FROM_OTF(val, NPY_DOUBLE,
                                                          NPY_ARRAY_IN_ARRAY);
            NpyDblTabDim = PyArray_DIMS(NpyDblTab);
            NmbElt = NpyDblTabDim[0];
            GmfSetKwd(FID, KwdCod, NmbElt);
            for (i = 0; i < NmbElt; i++) {
              // --- Writing each line in FID. --- //
              GmfSetLinTab(FID, KwdCod, NULL, PyArray_GETPTR2(NpyDblTab, i, 0),
                           NULL);
            }
            // --- Allow the Python garbage collector to free these arrays. ---
            Py_DECREF(NpyDblTab);
            break;
          default:
            PyErr_SetString(PyExc_RuntimeError, "Unknown type of array.");
            return NULL;
          }
        }
      }
    }
  }

  // --- Close mesh. ---
  GmfCloseMesh(FID);

  // --- Return 1 for success. --- //
  return Py_BuildValue("i", 1);
}

static char pymeshb_docstring[] =
    "LibMeshb Python wrapper to read/write *.mesh[b]/*.sol[b] file.";
static char read_docstring[] =
    "read('file.mesh[b]') or read('file.sol[b]'):\n"
    "Read a mesh file or a solution file.\n"
    "Return a dictionary based on the keys of the libmeshb library.\n";

static char write_docstring[] =
    "write('file.mesh[b]') or write('file.sol[b]'):\n"
    "Write a mesh file or a solution file.\n";

// --- Module methods table --- //
static PyMethodDef Methods[] = {
    {"read", pymeshb_read, METH_VARARGS, read_docstring},
    {"write", (PyCFunction)pymeshb_write, METH_VARARGS | METH_KEYWORDS,
     write_docstring},
    {NULL, NULL, 0, NULL}};

// --- Module initialization --- //
#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef pymeshb = {PyModuleDef_HEAD_INIT, "pymeshb",
                                     pymeshb_docstring, -1, Methods};
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
  (void)Py_InitModule3("pymeshb", Methods, pymeshb_docstring);
#endif
}

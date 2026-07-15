/* createrepo_c - Library of routines for manipulation with repodata
 * Copyright (C) 2012-2013  Tomas Mlcoch
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#ifndef CR_MODULESTATE_H
#define CR_MODULESTATE_H

#include <Python.h>

typedef struct {
    PyObject *error;
    PyTypeObject *ContentStat_Type;
    PyTypeObject *CrFile_Type;
    PyTypeObject *Metadata_Type;
    PyTypeObject *MetadataLocation_Type;
    PyTypeObject *Package_Type;
    PyTypeObject *PkgIterator_Type;
    PyTypeObject *Repomd_Type;
    PyTypeObject *RepomdRecord_Type;
    PyTypeObject *Sqlite_Type;
    PyTypeObject *UpdateCollection_Type;
    PyTypeObject *UpdateCollectionModule_Type;
    PyTypeObject *UpdateCollectionPackage_Type;
    PyTypeObject *UpdateInfo_Type;
    PyTypeObject *UpdateRecord_Type;
    PyTypeObject *UpdateReference_Type;
    PyTypeObject *XmlFile_Type;
} cr_module_state;

extern struct PyModuleDef createrepo_c_module_def;

static inline cr_module_state *
get_cr_module_state(PyObject *module)
{
    return (cr_module_state *)PyModule_GetState(module);
}

static inline cr_module_state *
get_cr_module_state_global(void)
{
    PyObject *name = PyUnicode_FromString("_createrepo_c");
    if (!name)
        return NULL;
    PyObject *module = PyImport_GetModule(name);
    Py_DECREF(name);
    if (!module) {
        PyErr_Clear();
        name = PyUnicode_FromString("createrepo_c._createrepo_c");
        if (!name)
            return NULL;
        module = PyImport_GetModule(name);
        Py_DECREF(name);
        if (!module)
            return NULL;
    }
    cr_module_state *state = get_cr_module_state(module);
    Py_DECREF(module);
    return state;
}

#endif

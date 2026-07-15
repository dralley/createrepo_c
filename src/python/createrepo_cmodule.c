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

#include <Python.h>
#include "src/createrepo_c.h"

#include "modulestate.h"
#include "checksum-py.h"
#include "compression_wrapper-py.h"
#include "contentstat-py.h"
#include "exception-py.h"
#include "load_metadata-py.h"
#include "locate_metadata-py.h"
#include "misc-py.h"
#include "package-py.h"
#include "parsepkg-py.h"
#include "repomd-py.h"
#include "repomdrecord-py.h"
#include "sqlite-py.h"
#include "updatecollection-py.h"
#include "updatecollectionmodule-py.h"
#include "updatecollectionpackage-py.h"
#include "updateinfo-py.h"
#include "updaterecord-py.h"
#include "updatereference-py.h"
#include "xml_dump-py.h"
#include "xml_file-py.h"
#include "xml_parser-py.h"

static struct PyMethodDef createrepo_c_methods[] = {
    {"package_from_rpm",        (PyCFunction)py_package_from_rpm,
        METH_VARARGS | METH_KEYWORDS, package_from_rpm__doc__},
    {"xml_from_rpm",            (PyCFunction)py_xml_from_rpm,
        METH_VARARGS | METH_KEYWORDS, xml_from_rpm__doc__},
    {"xml_dump_primary",        (PyCFunction)py_xml_dump_primary,
        METH_VARARGS, xml_dump_primary__doc__},
    {"xml_dump_filelists",      (PyCFunction)py_xml_dump_filelists,
        METH_VARARGS, xml_dump_filelists__doc__},
    {"xml_dump_filelists_ext",  (PyCFunction)py_xml_dump_filelists_ext,
        METH_VARARGS, xml_dump_filelists_ext__doc__},
    {"xml_dump_other",          (PyCFunction)py_xml_dump_other,
        METH_VARARGS, xml_dump_other__doc__},
    {"xml_dump_updaterecord",    (PyCFunction)py_xml_dump_updaterecord,
        METH_VARARGS, xml_dump_updaterecord__doc__},
    {"xml_dump",                (PyCFunction)py_xml_dump,
        METH_VARARGS, xml_dump__doc__},
    {"xml_parse_primary",       (PyCFunction)py_xml_parse_primary,
        METH_VARARGS, xml_parse_primary__doc__},
    {"xml_parse_primary_snippet",(PyCFunction)py_xml_parse_primary_snippet,
        METH_VARARGS, xml_parse_primary_snippet__doc__},
    {"xml_parse_filelists",     (PyCFunction)py_xml_parse_filelists,
        METH_VARARGS, xml_parse_filelists__doc__},
    {"xml_parse_filelists_snippet",(PyCFunction)py_xml_parse_filelists_snippet,
        METH_VARARGS, xml_parse_filelists_snippet__doc__},
    {"xml_parse_other",         (PyCFunction)py_xml_parse_other,
        METH_VARARGS, xml_parse_other__doc__},
    {"xml_parse_other_snippet",(PyCFunction)py_xml_parse_other_snippet,
        METH_VARARGS, xml_parse_other_snippet__doc__},
    {"xml_parse_repomd",        (PyCFunction)py_xml_parse_repomd,
        METH_VARARGS, xml_parse_repomd__doc__},
    {"xml_parse_updateinfo",    (PyCFunction)py_xml_parse_updateinfo,
        METH_VARARGS, xml_parse_updateinfo__doc__},
    {"checksum_name_str",       (PyCFunction)py_checksum_name_str,
        METH_VARARGS, checksum_name_str__doc__},
    {"checksum_type",           (PyCFunction)py_checksum_type,
        METH_VARARGS, checksum_type__doc__},
    {"compress_file_with_stat", (PyCFunction)py_compress_file_with_stat,
        METH_VARARGS, compress_file_with_stat__doc__},
    {"decompress_file_with_stat",(PyCFunction)py_decompress_file_with_stat,
        METH_VARARGS, decompress_file_with_stat__doc__},
    {"compression_suffix",      (PyCFunction)py_compression_suffix,
        METH_VARARGS, compression_suffix__doc__},
    {"detect_compression",      (PyCFunction)py_detect_compression,
        METH_VARARGS, detect_compression__doc__},
    {"compression_type",        (PyCFunction)py_compression_type,
        METH_VARARGS, compression_type__doc__},
    {NULL, NULL, 0, NULL} /* sentinel */
};

static int
createrepo_c_add_type(PyObject *module, PyType_Spec *spec, const char *name,
                      PyTypeObject **out)
{
    PyObject *type = PyType_FromModuleAndSpec(module, spec, NULL);
    if (!type)
        return -1;
    *out = (PyTypeObject *)type;
    if (PyModule_AddObjectRef(module, name, type) < 0) {
        Py_DECREF(type);
        *out = NULL;
        return -1;
    }
    return 0;
}

static int
createrepo_c_exec(PyObject *m)
{
    cr_module_state *state = get_cr_module_state(m);

    /* Exceptions */
    if (!init_exceptions())
        return -1;
    if (PyModule_AddObjectRef(m, "CreaterepoCError", CrErr_Exception) < 0)
        return -1;

    /* Types */
    if (createrepo_c_add_type(m, &ContentStat_Type_spec, "ContentStat",
                              &state->ContentStat_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &CrFile_Type_spec, "CrFile",
                              &state->CrFile_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &Package_Type_spec, "Package",
                              &state->Package_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &Metadata_Type_spec, "Metadata",
                              &state->Metadata_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &MetadataLocation_Type_spec, "MetadataLocation",
                              &state->MetadataLocation_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &Repomd_Type_spec, "Repomd",
                              &state->Repomd_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &RepomdRecord_Type_spec, "RepomdRecord",
                              &state->RepomdRecord_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &Sqlite_Type_spec, "Sqlite",
                              &state->Sqlite_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &UpdateCollection_Type_spec, "UpdateCollection",
                              &state->UpdateCollection_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &UpdateCollectionModule_Type_spec, "UpdateCollectionModule",
                              &state->UpdateCollectionModule_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &UpdateCollectionPackage_Type_spec, "UpdateCollectionPackage",
                              &state->UpdateCollectionPackage_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &UpdateInfo_Type_spec, "UpdateInfo",
                              &state->UpdateInfo_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &UpdateRecord_Type_spec, "UpdateRecord",
                              &state->UpdateRecord_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &UpdateReference_Type_spec, "UpdateReference",
                              &state->UpdateReference_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &XmlFile_Type_spec, "XmlFile",
                              &state->XmlFile_Type) < 0)
        return -1;
    if (createrepo_c_add_type(m, &PkgIterator_Type_spec, "PkgIterator",
                              &state->PkgIterator_Type) < 0)
        return -1;

    /* Createrepo init */

    cr_xml_dump_init();
    cr_package_parser_init();

    Py_AtExit(cr_xml_dump_cleanup);
    Py_AtExit(cr_package_parser_cleanup);

    /* Module constants */

    /* Version */
    PyModule_AddIntConstant(m, "VERSION_MAJOR", CR_VERSION_MAJOR);
    PyModule_AddIntConstant(m, "VERSION_MINOR", CR_VERSION_MINOR);
    PyModule_AddIntConstant(m, "VERSION_PATCH", CR_VERSION_PATCH);

    /* Checksum types */
    PyModule_AddIntConstant(m, "CHECKSUM_UNKNOWN", CR_CHECKSUM_UNKNOWN);
#ifdef WITH_LEGACY_HASHES
    PyModule_AddIntConstant(m, "MD5", CR_CHECKSUM_MD5);
    PyModule_AddIntConstant(m, "SHA", CR_CHECKSUM_SHA);
    PyModule_AddIntConstant(m, "SHA1", CR_CHECKSUM_SHA1);
#endif
    PyModule_AddIntConstant(m, "SHA224", CR_CHECKSUM_SHA224);
    PyModule_AddIntConstant(m, "SHA256", CR_CHECKSUM_SHA256);
    PyModule_AddIntConstant(m, "SHA384", CR_CHECKSUM_SHA384);
    PyModule_AddIntConstant(m, "SHA512", CR_CHECKSUM_SHA512);

    /* File open modes */
    PyModule_AddIntConstant(m, "MODE_READ", CR_CW_MODE_READ);
    PyModule_AddIntConstant(m, "MODE_WRITE", CR_CW_MODE_WRITE);

    /* Compression types */
    PyModule_AddIntConstant(m, "AUTO_DETECT_COMPRESSION", CR_CW_AUTO_DETECT_COMPRESSION);
    PyModule_AddIntConstant(m, "UNKNOWN_COMPRESSION", CR_CW_UNKNOWN_COMPRESSION);
    PyModule_AddIntConstant(m, "NO_COMPRESSION", CR_CW_NO_COMPRESSION);
    PyModule_AddIntConstant(m, "GZ_COMPRESSION", CR_CW_GZ_COMPRESSION);
    PyModule_AddIntConstant(m, "BZ2_COMPRESSION", CR_CW_BZ2_COMPRESSION);
    PyModule_AddIntConstant(m, "XZ_COMPRESSION", CR_CW_XZ_COMPRESSION);
    PyModule_AddIntConstant(m, "ZCK_COMPRESSION", CR_CW_ZCK_COMPRESSION);
    PyModule_AddIntConstant(m, "ZSTD_COMPRESSION", CR_CW_ZSTD_COMPRESSION);

    /* Zchunk support */
#ifdef WITH_ZCHUNK
    PyModule_AddIntConstant(m, "HAS_ZCK", 1);
#else
    PyModule_AddIntConstant(m, "HAS_ZCK", 0);
#endif // WITH_ZCHUNK

    /* Load Metadata key values */
    PyModule_AddIntConstant(m, "HT_KEY_DEFAULT", CR_HT_KEY_DEFAULT);
    PyModule_AddIntConstant(m, "HT_KEY_HASH", CR_HT_KEY_HASH);
    PyModule_AddIntConstant(m, "HT_KEY_NAME", CR_HT_KEY_NAME);
    PyModule_AddIntConstant(m, "HT_KEY_FILENAME", CR_HT_KEY_FILENAME);

    /* Load Metadata key dup action */
    PyModule_AddIntConstant(m, "HT_DUPACT_KEEPFIRST", CR_HT_DUPACT_KEEPFIRST);
    PyModule_AddIntConstant(m, "HT_DUPACT_REMOVEALL", CR_HT_DUPACT_REMOVEALL);

    /* Sqlite DB types */
    PyModule_AddIntConstant(m, "DB_PRIMARY", CR_DB_PRIMARY);
    PyModule_AddIntConstant(m, "DB_FILELISTS", CR_DB_FILELISTS);
    PyModule_AddIntConstant(m, "DB_FILELISTS_EXT", CR_DB_FILELISTS_EXT);
    PyModule_AddIntConstant(m, "DB_OTHER", CR_DB_OTHER);

    /* XmlFile types */
    PyModule_AddIntConstant(m, "XMLFILE_PRIMARY", CR_XMLFILE_PRIMARY);
    PyModule_AddIntConstant(m, "XMLFILE_FILELISTS", CR_XMLFILE_FILELISTS);
    PyModule_AddIntConstant(m, "XMLFILE_FILELISTS_EXT", CR_XMLFILE_FILELISTS_EXT);
    PyModule_AddIntConstant(m, "XMLFILE_OTHER", CR_XMLFILE_OTHER);
    PyModule_AddIntConstant(m, "XMLFILE_PRESTODELTA", CR_XMLFILE_PRESTODELTA);
    PyModule_AddIntConstant(m, "XMLFILE_UPDATEINFO", CR_XMLFILE_UPDATEINFO);

    /* XmlParser types */
    PyModule_AddIntConstant(m, "XML_WARNING_UNKNOWNTAG", CR_XML_WARNING_UNKNOWNTAG);
    PyModule_AddIntConstant(m, "XML_WARNING_MISSINGATTR", CR_XML_WARNING_MISSINGATTR);
    PyModule_AddIntConstant(m, "XML_WARNING_UNKNOWNVAL", CR_XML_WARNING_UNKNOWNVAL);
    PyModule_AddIntConstant(m, "XML_WARNING_BADATTRVAL", CR_XML_WARNING_BADATTRVAL);

    /* Header Reading flags */
    PyModule_AddIntConstant(m, "HDRR_NONE", CR_HDRR_NONE);
    PyModule_AddIntConstant(m, "HDRR_LOADHDRID", CR_HDRR_LOADHDRID);
    PyModule_AddIntConstant(m, "HDRR_LOADSIGNATURES", CR_HDRR_LOADSIGNATURES);
    PyModule_AddIntConstant(m, "HDRR_NOFILEDIGESTS", CR_HDRR_NOFILEDIGESTS);

    return 0;
}

static int
createrepo_c_traverse(PyObject *m, visitproc visit, void *arg)
{
    cr_module_state *state = get_cr_module_state(m);
    Py_VISIT(state->error);
    Py_VISIT(state->ContentStat_Type);
    Py_VISIT(state->CrFile_Type);
    Py_VISIT(state->Metadata_Type);
    Py_VISIT(state->MetadataLocation_Type);
    Py_VISIT(state->Package_Type);
    Py_VISIT(state->PkgIterator_Type);
    Py_VISIT(state->Repomd_Type);
    Py_VISIT(state->RepomdRecord_Type);
    Py_VISIT(state->Sqlite_Type);
    Py_VISIT(state->UpdateCollection_Type);
    Py_VISIT(state->UpdateCollectionModule_Type);
    Py_VISIT(state->UpdateCollectionPackage_Type);
    Py_VISIT(state->UpdateInfo_Type);
    Py_VISIT(state->UpdateRecord_Type);
    Py_VISIT(state->UpdateReference_Type);
    Py_VISIT(state->XmlFile_Type);
    return 0;
}

static int
createrepo_c_clear(PyObject *m)
{
    cr_module_state *state = get_cr_module_state(m);
    Py_CLEAR(state->error);
    Py_CLEAR(state->ContentStat_Type);
    Py_CLEAR(state->CrFile_Type);
    Py_CLEAR(state->Metadata_Type);
    Py_CLEAR(state->MetadataLocation_Type);
    Py_CLEAR(state->Package_Type);
    Py_CLEAR(state->PkgIterator_Type);
    Py_CLEAR(state->Repomd_Type);
    Py_CLEAR(state->RepomdRecord_Type);
    Py_CLEAR(state->Sqlite_Type);
    Py_CLEAR(state->UpdateCollection_Type);
    Py_CLEAR(state->UpdateCollectionModule_Type);
    Py_CLEAR(state->UpdateCollectionPackage_Type);
    Py_CLEAR(state->UpdateInfo_Type);
    Py_CLEAR(state->UpdateRecord_Type);
    Py_CLEAR(state->UpdateReference_Type);
    Py_CLEAR(state->XmlFile_Type);
    return 0;
}

static void
createrepo_c_free(void *m)
{
    createrepo_c_clear((PyObject *)m);
}

static PyModuleDef_Slot createrepo_c_slots[] = {
    {Py_mod_exec, createrepo_c_exec},
    {0, NULL}
};

struct PyModuleDef createrepo_c_module_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_createrepo_c",
    .m_size = sizeof(cr_module_state),
    .m_methods = createrepo_c_methods,
    .m_slots = createrepo_c_slots,
    .m_traverse = createrepo_c_traverse,
    .m_clear = createrepo_c_clear,
    .m_free = createrepo_c_free,
};

PyMODINIT_FUNC
PyInit__createrepo_c(void)
{
    return PyModuleDef_Init(&createrepo_c_module_def);
}

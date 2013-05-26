/* createrepo_c - Library of routines for manipulation with repodata
 * Copyright (C) 2013  Tomas Mlcoch
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

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include "fixtures.h"
#include "createrepo/error.h"
#include "createrepo/package.h"
#include "createrepo/misc.h"
#include "createrepo/xml_parser.h"

// Callbacks

static int
pkgcb(cr_Package *pkg, void *cbdata, GError **err)
{
    g_assert(pkg);
    g_assert(!err || *err == NULL);
    if (cbdata) *((int *)cbdata) += 1;
    cr_package_free(pkg);
    return CRE_OK;
}

static int
pkgcb_interrupt(cr_Package *pkg, void *cbdata, GError **err)
{
    g_assert(pkg);
    g_assert(!err || *err == NULL);
    if (cbdata) *((int *)cbdata) += 1;
    cr_package_free(pkg);
    return 1;
}

static int
newpkgcb_skip_fake_bash(cr_Package **pkg,
                        const char *pkgId,
                        const char *name,
                        const char *arch,
                        void *cbdata,
                        GError **err)
{
    CR_UNUSED(pkgId);
    CR_UNUSED(arch);
    CR_UNUSED(cbdata);

    g_assert(pkg != NULL);
    g_assert(*pkg == NULL);
    g_assert(pkgId != NULL);
    g_assert(!err || *err == NULL);

    if (!g_strcmp0(name, "fake_bash"))
        return CRE_OK;

    *pkg = cr_package_new();
    return CRE_OK;
}

static int
newpkgcb_interrupt(cr_Package **pkg,
                   const char *pkgId,
                   const char *name,
                   const char *arch,
                   void *cbdata,
                   GError **err)
{
    CR_UNUSED(pkgId);
    CR_UNUSED(arch);
    CR_UNUSED(cbdata);

    g_assert(pkg != NULL);
    g_assert(*pkg == NULL);
    g_assert(pkgId != NULL);
    g_assert(!err || *err == NULL);

    return 1;
}

// Tests

static void
test_cr_xml_parse_filelists_00(void)
{
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_REPO_00_FILELISTS, NULL, NULL,
                                     pkgcb, NULL, NULL, &tmp_err);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
}

static void
test_cr_xml_parse_filelists_01(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_REPO_01_FILELISTS, NULL, NULL,
                                     pkgcb, &parsed, NULL, &tmp_err);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
    g_assert_cmpint(parsed, ==, 1);
}

static void
test_cr_xml_parse_filelists_02(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_REPO_02_FILELISTS, NULL, NULL,
                                     pkgcb, &parsed, NULL, &tmp_err);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
    g_assert_cmpint(parsed, ==, 2);
}

static void
test_cr_xml_parse_filelists_unknown_element_00(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_MRF_UE_FIL_00, NULL, NULL,
                                     pkgcb, &parsed, NULL, &tmp_err);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
    g_assert_cmpint(parsed, ==, 2);
}

static void
test_cr_xml_parse_filelists_unknown_element_01(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_MRF_UE_FIL_01, NULL, NULL,
                                     pkgcb, &parsed, NULL, &tmp_err);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
    g_assert_cmpint(parsed, ==, 1);
}

static void
test_cr_xml_parse_filelists_unknown_element_02(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_MRF_UE_FIL_02, NULL, NULL,
                                     pkgcb, &parsed, NULL, &tmp_err);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
    g_assert_cmpint(parsed, ==, 2);
}

static void
test_cr_xml_parse_filelists_no_pkgid(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_MRF_NO_PKGID_FIL, NULL, NULL,
                                     pkgcb, &parsed, NULL, &tmp_err);
    g_assert(tmp_err != NULL);
    g_error_free(tmp_err);
    g_assert_cmpint(ret, ==, CRE_BADXMLFILELISTS);
}

static void
test_cr_xml_parse_filelists_skip_fake_bash_00(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_MRF_UE_FIL_00,
                                     newpkgcb_skip_fake_bash, NULL,
                                     pkgcb, &parsed, NULL, &tmp_err);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
    g_assert_cmpint(parsed, ==, 1);
}

static void
test_cr_xml_parse_filelists_skip_fake_bash_01(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_MRF_UE_FIL_01,
                                     newpkgcb_skip_fake_bash, NULL,
                                     pkgcb, &parsed, NULL, &tmp_err);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
    g_assert_cmpint(parsed, ==, 0);
}

static void
test_cr_xml_parse_filelists_pkgcb_interrupt(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_REPO_02_FILELISTS, NULL, NULL,
                                     pkgcb_interrupt, &parsed, NULL, &tmp_err);
    g_assert(tmp_err != NULL);
    g_error_free(tmp_err);
    g_assert_cmpint(ret, ==, CRE_CBINTERRUPTED);
    g_assert_cmpint(parsed, ==, 1);
}

static void
test_cr_xml_parse_filelists_newpkgcb_interrupt(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_REPO_02_FILELISTS,
                                     newpkgcb_interrupt, NULL,
                                     pkgcb, &parsed,  NULL, &tmp_err);
    g_assert(tmp_err != NULL);
    g_error_free(tmp_err);
    g_assert_cmpint(ret, ==, CRE_CBINTERRUPTED);
    g_assert_cmpint(parsed, ==, 0);
}

static void
test_cr_xml_parse_filelists_bad_file_type_00(void)
{
    int parsed = 0;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_MRF_BAD_TYPE_FIL,
                                     NULL, NULL,
                                     pkgcb, &parsed, NULL, &tmp_err);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
    g_assert_cmpint(parsed, ==, 2);
}

static void
test_cr_xml_parse_filelists_bad_file_type_01(void)
{
    int parsed = 0;
    char *msgs = NULL;
    GError *tmp_err = NULL;
    int ret = cr_xml_parse_filelists(TEST_MRF_BAD_TYPE_FIL,
                                     NULL, NULL,
                                     pkgcb, &parsed, &msgs, &tmp_err);
    g_assert(msgs != NULL);
    g_assert_cmpstr(msgs, ==, "Unknown file type \"foo\";");
    g_free(msgs);
    g_assert(tmp_err == NULL);
    g_assert_cmpint(ret, ==, CRE_OK);
    g_assert_cmpint(parsed, ==, 2);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_00",
                    test_cr_xml_parse_filelists_00);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_01",
                    test_cr_xml_parse_filelists_01);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_02",
                    test_cr_xml_parse_filelists_02);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_unknown_element_00",
                    test_cr_xml_parse_filelists_unknown_element_00);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_unknown_element_01",
                    test_cr_xml_parse_filelists_unknown_element_01);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_unknown_element_02",
                    test_cr_xml_parse_filelists_unknown_element_02);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_no_pgkid",
                    test_cr_xml_parse_filelists_no_pkgid);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_skip_fake_bash_00",
                    test_cr_xml_parse_filelists_skip_fake_bash_00);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_skip_fake_bash_01",
                    test_cr_xml_parse_filelists_skip_fake_bash_01);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_pkgcb_interrupt",
                    test_cr_xml_parse_filelists_pkgcb_interrupt);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_newpkgcb_interrupt",
                    test_cr_xml_parse_filelists_newpkgcb_interrupt);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_bad_file_type_00",
                    test_cr_xml_parse_filelists_bad_file_type_00);
    g_test_add_func("/xml_parser_filelists/test_cr_xml_parse_filelists_bad_file_type_01",
                    test_cr_xml_parse_filelists_bad_file_type_01);

    return g_test_run();
}
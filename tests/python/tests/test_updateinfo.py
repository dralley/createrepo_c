import unittest
import shutil
import tempfile
import os.path
import createrepo_c as cr

from fixtures import *

class TestCaseUpdateInfo(unittest.TestCase):

    def test_updateinfo_setters(self):
        ui = cr.UpdateInfo()
        self.assertTrue(ui)

        self.assertEqual(ui.updates, [])

        rec = cr.UpdateRecord()
        rec.fromstr = "from"
        rec.status = "status"
        rec.type = "type"
        rec.version = "version"
        rec.id = "id"
        rec.title = "title"
        rec.issued_date = "issued_date"
        rec.updated_date = "updated_date"
        rec.rights = "rights"
        rec.release = "release"
        rec.pushcount = "pushcount"
        rec.severity = "severity"
        rec.summary = "summary"
        rec.description = "description"
        rec.solution = "solution"

        ui.append(rec)

        self.assertEqual(len(ui.updates), 1)
        rec = ui.updates[0]

        self.assertEqual(rec.fromstr, "from")
        self.assertEqual(rec.status, "status")
        self.assertEqual(rec.type, "type")
        self.assertEqual(rec.version, "version")
        self.assertEqual(rec.id, "id")
        self.assertEqual(rec.title, "title")
        self.assertEqual(rec.issued_date, "issued_date")
        self.assertEqual(rec.updated_date, "updated_date")
        self.assertEqual(rec.rights, "rights")
        self.assertEqual(rec.release, "release")
        self.assertEqual(rec.pushcount, "pushcount")
        self.assertEqual(rec.severity, "severity")
        self.assertEqual(rec.summary, "summary")
        self.assertEqual(rec.description, "description")
        self.assertEqual(rec.solution, "solution")
        self.assertEqual(len(rec.references), 0)
        self.assertEqual(len(rec.collections), 0)

    def test_updateinfo_xml_dump_01(self):
        ui = cr.UpdateInfo()
        xml = ui.xml_dump()

        self.assertEqual(xml,
        """<?xml version="1.0" encoding="UTF-8"?>\n<updates/>\n""")

    def test_updateinfo_xml_dump_02(self):
        ui = cr.UpdateInfo()
        xml = ui.xml_dump()

        rec = cr.UpdateRecord()
        rec.fromstr = "from"
        rec.status = "status"
        rec.type = "type"
        rec.version = "version"
        rec.id = "id"
        rec.title = "title"
        rec.issued_date = "issued_date"
        rec.updated_date = "updated_date"
        rec.rights = "rights"
        rec.release = "release"
        rec.pushcount = "pushcount"
        rec.severity = "severity"
        rec.summary = "summary"
        rec.description = "description"
        rec.solution = "solution"

        ui.append(rec)
        xml = ui.xml_dump()

        self.assertEqual(xml,
"""<?xml version="1.0" encoding="UTF-8"?>
<updates>
  <update from="from" status="status" type="type" version="version">
    <id>id</id>
    <title>title</title>
    <issued date="issued_date"/>
    <updated date="updated_date"/>
    <rights>rights</rights>
    <release>release</release>
    <pushcount>pushcount</pushcount>
    <severity>severity</severity>
    <summary>summary</summary>
    <description>description</description>
    <solution>solution</solution>
    <references/>
    <pkglist/>
  </update>
</updates>
""")

    def test_updateinfo_xml_dump_03(self):

        pkg = cr.UpdateCollectionPackage()
        pkg.name = "foo"
        pkg.version = "1.2"
        pkg.release = "3"
        pkg.epoch = "0"
        pkg.arch = "x86"
        pkg.src = "foo.src.rpm"
        pkg.filename = "foo.rpm"
        pkg.sum = "abcdef"
        pkg.sum_type = cr.SHA1
        pkg.reboot_suggested = True

        col = cr.UpdateCollection()
        col.shortname = "short name"
        col.name = "long name"
        col.append(pkg)

        ref = cr.UpdateReference()
        ref.href = "href"
        ref.id = "id"
        ref.type = "type"
        ref.title = "title"

        rec = cr.UpdateRecord()
        rec.fromstr = "from"
        rec.status = "status"
        rec.type = "type"
        rec.version = "version"
        rec.id = "id"
        rec.title = "title"
        rec.issued_date = "issued_date"
        rec.updated_date = "updated_date"
        rec.rights = "rights"
        rec.release = "release"
        rec.pushcount = "pushcount"
        rec.severity = "severity"
        rec.summary = "summary"
        rec.description = "description"
        rec.solution = "solution"
        rec.append_collection(col)
        rec.append_reference(ref)

        ui = cr.UpdateInfo()
        ui.append(rec)
        xml = ui.xml_dump()

        self.assertEqual(xml,
"""<?xml version="1.0" encoding="UTF-8"?>
<updates>
  <update from="from" status="status" type="type" version="version">
    <id>id</id>
    <title>title</title>
    <issued date="issued_date"/>
    <updated date="updated_date"/>
    <rights>rights</rights>
    <release>release</release>
    <pushcount>pushcount</pushcount>
    <severity>severity</severity>
    <summary>summary</summary>
    <description>description</description>
    <solution>solution</solution>
    <references>
      <reference href="href" id="id" type="type" title="title"/>
    </references>
    <pkglist>
      <collection short="short name">
        <name>long name</name>
        <package name="foo" version="1.2" relase="3" epoch="0" arch="x86" src="foo.src.rpm">
          <filename>foo.rpm</filename>
          <sum type="sha1">abcdef</sum>
          <reboot_suggested/>
        </package>
      </collection>
    </pkglist>
  </update>
</updates>
""")
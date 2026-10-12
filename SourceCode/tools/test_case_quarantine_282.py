#!/usr/bin/env python3
"""Offline regression for SUMMIT 282. Runs on synthetic data only."""
import hashlib
import json
import os
import io
from contextlib import redirect_stdout, redirect_stderr
import tempfile
import unittest
from pathlib import Path

from quarantine_case_directory_282 import EXPECTED, EXPECTED_SMD, EXPECTED_INX, MANIFEST, main


class QuarantineTests282(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.repo = Path(self.tmp.name)
        (self.repo / 'SourceCode').mkdir()
        (self.repo / 'bugfixes').mkdir()
        self.upper = self.repo / 'cliente/char/tmABCD'
        self.lower = self.repo / 'cliente/char/tmabcd'
        self.upper.mkdir(parents=True)
        self.lower.mkdir()
        for i, name in enumerate(sorted(EXPECTED)):
            data = f'model {i} test-content\x00'.encode()
            (self.upper / name).write_bytes(data)
            (self.lower / name).write_bytes(data)
        self.b = self.repo / 'bugfixes' / 'CASE_282_BACKUP'
        self.before = {f: hashlib.sha256((self.upper / f).read_bytes()).hexdigest() for f in EXPECTED}

    def call(self, operation, *extra):
        out = io.StringIO()
        with redirect_stdout(out), redirect_stderr(out):
            code = main([operation, '--repo', str(self.repo), *extra])
        return code, out.getvalue()

    def inspect_ok(self):
        return self.call('inspect')

    def apply(self):
        return self.call('apply','--backup-dir',str(self.b),'--confirm-game-closed')

    def rollback(self):
        return self.call('rollback','--backup-dir',str(self.b),'--confirm-rollback')

    def test_complete_inspect_apply_rollback(self):
        status, out = self.inspect_ok()
        self.assertEqual(status,0,out)
        self.assertIn('client_writes=0',out)
        status,out = self.apply()
        self.assertEqual(status,0,out)
        self.assertFalse(self.lower.exists())
        self.assertTrue((self.b / 'tmabcd').is_dir())
        self.assertTrue((self.b / MANIFEST).is_file())
        self.assertEqual(self.before,{f:hashlib.sha256((self.upper/f).read_bytes()).hexdigest() for f in EXPECTED})
        status,out=self.rollback()
        self.assertEqual(status,0,out)
        self.assertTrue(self.lower.is_dir())
        self.assertFalse((self.b / 'tmabcd').exists())
        self.assertEqual(self.before,{f:hashlib.sha256((self.upper/f).read_bytes()).hexdigest() for f in EXPECTED})

    def test_48_exact_required_no_extra(self):
        (self.lower / 'extra.BMP').write_bytes(b'asset')
        status,out=self.apply()
        self.assertEqual(status,2)
        self.assertTrue(self.lower.is_dir())
        self.assertFalse(self.b.exists())

    def test_subdir_and_hidden_extra_refused(self):
        (self.lower/'nested').mkdir()
        self.assertEqual(self.apply()[0],2)
        (self.lower/'nested').rmdir()
        (self.lower/'.hidden').write_bytes(b'1')
        self.assertEqual(self.apply()[0],2)

    def test_symlink_file_refused(self):
        n=sorted(EXPECTED)[0]
        (self.lower / n).unlink()
        (self.lower / n).symlink_to(self.upper/n)
        self.assertEqual(self.inspect_ok()[0],2)
        self.assertEqual(self.apply()[0],2)

    def test_different_hash_refused(self):
        (self.lower / sorted(EXPECTED)[0]).write_bytes(b'wrong model')
        self.assertEqual(self.apply()[0],2)
        self.assertFalse(self.b.exists())

    def test_missing_model_refused(self):
        (self.lower / sorted(EXPECTED)[0]).unlink()
        self.assertEqual(self.apply()[0],2)

    def test_24_inx_included_and_reported(self):
        self.assertEqual(len(EXPECTED_SMD), 24)
        self.assertEqual(len(EXPECTED_INX), 24)
        status, out = self.inspect_ok()
        self.assertEqual(status, 0, out)
        self.assertIn('pairs=48 identical=48', out)

    def test_inx_missing_in_upper_refused_and_reported(self):
        name=sorted(EXPECTED_INX)[0]
        (self.upper/name).unlink()
        status,out=self.inspect_ok()
        self.assertEqual(status,2)
        self.assertIn('missing_in_upper:'+name,out)
        self.assertEqual(self.apply()[0],2)

    def test_inx_different_hash_refused_and_reported(self):
        name=sorted(EXPECTED_INX)[0]
        (self.upper/name).write_bytes(b'changed inx')
        status,out=self.inspect_ok()
        self.assertEqual(status,2)
        self.assertIn('hash_differs:'+name,out)
        self.assertEqual(self.apply()[0],2)

    def test_inx_changed_in_quarantine_prevents_rollback(self):
        self.assertEqual(self.apply()[0],0)
        (self.b/'tmabcd'/sorted(EXPECTED_INX)[0]).write_bytes(b'changed after quarantine')
        self.assertEqual(self.rollback()[0],2)
        self.assertFalse(self.lower.exists())

    def test_inx_added_only_to_lower_fails(self):
        name=sorted(EXPECTED_INX)[0]
        (self.upper/name).unlink()
        self.assertEqual(self.inspect_ok()[0],2)
        self.assertEqual(self.apply()[0],2)

    def test_legacy_282_backup_can_still_be_rolled_back(self):
        # An actual V1 backup (only the 24 known SMDs) can be restored without
        # weakening the V2 requirement of 48 for any NEW quarantine.
        for name in EXPECTED_INX:
            (self.lower/name).unlink()
        self.b.mkdir()
        saved={name:hashlib.sha256((self.lower/name).read_bytes()).hexdigest()
               for name in EXPECTED_SMD}
        manifest={'format':'SUMMIT_CASE_QUARANTINE_282_V1',
                  'client_rel':'cliente/char/tmabcd',
                  'preserved_rel':'cliente/char/tmABCD',
                  'files':saved,'status':'quarantined'}
        (self.b/MANIFEST).write_text(json.dumps(manifest))
        os.rename(self.lower, self.b/'tmabcd')
        status,out=self.rollback()
        self.assertEqual(status,0,out)
        self.assertTrue(self.lower.is_dir())
        self.assertEqual(len(list(self.lower.iterdir())),24)

    def test_legacy_282_manifest_cannot_authorize_new_48_file_archive(self):
        self.assertEqual(self.apply()[0],0)
        m=self.b/MANIFEST
        raw=json.loads(m.read_text())
        raw['format']='SUMMIT_CASE_QUARANTINE_282_V1'
        m.write_text(json.dumps(raw))
        self.assertEqual(self.rollback()[0],2)
        self.assertFalse(self.lower.exists())


    def test_multiple_inx_issues_reported_at_once(self):
        names=sorted(EXPECTED_INX)
        (self.upper/names[0]).unlink()
        (self.upper/names[1]).write_bytes(b'mismatch')
        code,out=self.inspect_ok()
        self.assertEqual(code,2)
        self.assertIn(names[0],out)
        self.assertIn(names[1],out)
        self.assertFalse(self.b.exists())


    def test_no_apply_without_confirmation(self):
        status,out=self.call('apply','--backup-dir',str(self.b))
        self.assertEqual(status,2)
        self.assertTrue(self.lower.is_dir())

    def test_no_rollback_without_confirmation(self):
        self.assertEqual(self.apply()[0],0)
        status,out=self.call('rollback','--backup-dir',str(self.b))
        self.assertEqual(status,2)
        self.assertFalse(self.lower.exists())

    def test_no_overwrite_existing_backup(self):
        self.b.mkdir()
        self.assertEqual(self.apply()[0],2)
        self.assertTrue(self.lower.is_dir())

    def test_rollback_refuses_modified_archive(self):
        self.assertEqual(self.apply()[0],0)
        (self.b/'tmabcd'/sorted(EXPECTED)[0]).write_bytes(b'tampered')
        self.assertEqual(self.rollback()[0],2)
        self.assertFalse(self.lower.exists())

    def test_rollback_refuses_occupied_destination(self):
        self.assertEqual(self.apply()[0],0)
        self.lower.mkdir()
        self.assertEqual(self.rollback()[0],2)
        self.assertTrue((self.b/'tmabcd').is_dir())

    def test_rollback_refuses_upper_changed(self):
        self.assertEqual(self.apply()[0],0)
        (self.upper/sorted(EXPECTED)[0]).write_bytes(b'changed after apply')
        self.assertEqual(self.rollback()[0],2)

    def test_refuses_case_alias_third_directory(self):
        (self.repo/'cliente/char/TmAbCd').mkdir()
        self.assertEqual(self.apply()[0],2)

    def test_backup_traversal_and_wrong_location_refused(self):
        for location in ('bugfixes/../cliente/unsafe', '../outside', 'cliente/unsafe'):
            status,out=self.call('apply','--backup-dir',location,'--confirm-game-closed')
            self.assertEqual(status,2, location+' '+out)
            self.assertTrue(self.lower.is_dir())

    def test_backup_symlink_refused(self):
        (self.repo/'bugfixes/CASE_282_BACKUP').symlink_to(self.repo/'cliente')
        self.assertEqual(self.apply()[0],2)
        self.assertTrue(self.lower.is_dir())

    def test_rollback_after_apply_does_not_overwrite_other_files(self):
        extra=self.upper/'unrelated.dds'
        extra.write_bytes(b'must stay')
        self.assertEqual(self.apply()[0],0)
        self.assertEqual(self.rollback()[0],0)
        self.assertEqual(extra.read_bytes(), b'must stay')


if __name__ == '__main__':
    unittest.main(verbosity=2)

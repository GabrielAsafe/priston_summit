#!/usr/bin/env python3
"""PATCH 280: read-only, unique casefold aliases must not authorize installation."""
from __future__ import annotations

import contextlib
import hashlib
import io
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import deploy_native_model_stage_278 as deploy
from test_native_model_stage_267 import StageGateTests


class HistoricalCasefoldTests280(unittest.TestCase):
    def setUp(self):
        self.fixture = StageGateTests('test_complete_staging_passes')
        self.fixture.setUp()
        self.addCleanup(self.fixture.doCleanups)
        self.client = self.fixture.client
        self.stage = self.fixture.out
        self.server = self.fixture.server
        self.cache = self.fixture.cache_path

    def test_unique_uppercase_file_is_read_and_hashes_match(self):
        expected = self.cache.read_bytes()
        alias = self.client / 'Effect' / 'TEST.SMD'
        alias.write_bytes(expected)
        self.server.write_bytes(b'rebuilt server identity')
        totals, rows = deploy.compare(self.client, self.server, self.stage, archival_readonly=True)
        self.assertEqual(totals, {'SAME': 1, 'DIFFERENT': 0, 'MISSING': 0})
        self.assertEqual(rows[0]['existing_path'], 'Effect/TEST.SMD')
        self.assertTrue(rows[0]['case_variant'])
        self.assertEqual(rows[0]['existing_sha256'], hashlib.sha256(expected).hexdigest())
        self.assertFalse((self.client / 'Effect/test.smd').exists())

    def test_unique_uppercase_file_is_reported_different_without_writes(self):
        alias = self.client / 'Effect' / 'TEST.SMD'
        alias.write_bytes(b'old cache bytes')
        before = alias.stat().st_mtime_ns, alias.read_bytes()
        totals, rows = deploy.compare(self.client, self.server, self.stage, archival_readonly=True)
        self.assertEqual(totals['DIFFERENT'], 1)
        self.assertTrue(rows[0]['case_variant'])
        self.assertEqual((alias.stat().st_mtime_ns, alias.read_bytes()), before)

    def test_true_leaf_casefold_collision_reported_but_never_selected(self):
        folder = self.client / 'Effect'
        (folder / 'test.smd').write_bytes(b'file 1')
        (folder / 'TEST.SMD').write_bytes(b'file 2')
        # PATCH 281: read-only historical compare reports AMBIGUOUS instead
        # of aborting all other rows. Strict lookup and deployment still refuse.
        totals, rows = deploy.compare(self.client, self.server, self.stage,
                                      archival_readonly=True)
        self.assertEqual(totals, {'SAME': 0, 'DIFFERENT': 0, 'MISSING': 0})
        self.assertEqual(rows[0]['state'], 'AMBIGUOUS')
        self.assertIsNone(rows[0]['existing_sha256'])
        with self.assertRaisesRegex(ValueError, 'case-insensitive target path collision'):
            deploy._historical_existing_target(self.client, 'Effect/test.smd')
        with self.assertRaisesRegex(ValueError, 'case-insensitive target path collision'):
            deploy.compare(self.client, self.server, self.stage)

    def test_symlink_alias_refused_even_if_unique(self):
        alias = self.client / 'Effect' / 'TEST.SMD'
        alias.symlink_to(self.client / 'Game.exe')
        with self.assertRaisesRegex(ValueError, 'symlink'):
            deploy.compare(self.client, self.server, self.stage, archival_readonly=True)

    def test_missing_remains_missing(self):
        totals, rows = deploy.compare(self.client, self.server, self.stage, archival_readonly=True)
        self.assertEqual(totals['MISSING'], 1)
        self.assertFalse(rows[0]['case_variant'])
        self.assertIsNone(rows[0]['existing_path'])

    def test_readonly_parent_case_variant(self):
        # Source ASE remains where 267 expects it; test directory resolution
        # without mutating that source tree.
        p = self.client / 'char' / 'Tmabcd'
        p.mkdir(parents=True)
        binary = p / 'Mb03BT.SMB'
        binary.write_bytes(b'safe-cache')
        target, alias = deploy._historical_existing_target(self.client,
                                                            'char/tmABCD/Mb03bt.smb')
        self.assertEqual(target, binary)
        self.assertTrue(alias)
        self.assertEqual(target.read_bytes(), b'safe-cache')
        self.assertFalse((self.client / 'char/tmABCD').exists())

    def test_two_similar_parent_spellings_refused(self):
        for folder in ('Tmabcd', 'tmabcd'):
            (self.client / 'char' / folder).mkdir(parents=True)
        with self.assertRaisesRegex(ValueError, 'case-insensitive target path collision'):
            deploy._historical_existing_target(self.client, 'char/tmABCD/file.smb')

    def test_parent_alias_symlink_refused(self):
        (self.client / 'char').mkdir()
        (self.client / 'char' / 'TMABCD').symlink_to(self.client / 'Effect', target_is_directory=True)
        with self.assertRaisesRegex(ValueError, 'symlink'):
            deploy._historical_existing_target(self.client, 'char/tmABCD/file.smb')

    def test_deployment_still_refuses_same_unique_alias(self):
        (self.client / 'Effect' / 'TEST.SMD').write_bytes(b'old')
        plan = self.client.parent / 'plan280.json'
        with self.assertRaisesRegex(ValueError, 'case-insensitive target path collision'):
            deploy.make_plan(self.client, self.server, self.stage, None, plan,
                             ['Effect/test.ase'], False)
        self.assertFalse(plan.exists())
        with self.assertRaisesRegex(ValueError, 'case-insensitive target path collision'):
            deploy.compare(self.client, self.server, self.stage)

    def test_cli_indicates_case_variant_and_no_deployment(self):
        (self.client / 'Effect/TEST.SMD').write_bytes(b'old')
        args = ['deploy_native_model_stage_278.py', 'compare', '--client', str(self.client),
                '--server', str(self.server), '--out', str(self.stage),
                '--archival-readonly', '--details']
        with mock.patch.object(sys, 'argv', args):
            with contextlib.redirect_stdout(io.StringIO()) as out:
                self.assertEqual(deploy.main(), 0)
        result = out.getvalue()
        self.assertIn('NATIVE_DEPLOY_280=ARCHIVAL_COMPARE', result)
        self.assertIn('case_variants=1', result)
        self.assertIn('deployment_allowed=0 client_writes=0', result)
        self.assertIn('case_variant=Effect/TEST.SMD', result)


if __name__ == '__main__':
    unittest.main(verbosity=2)

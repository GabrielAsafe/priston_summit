#!/usr/bin/env python3
"""PATCH 281: historical audit reports genuine casefold collisions, no deployment."""
from __future__ import annotations

import contextlib
import io
from pathlib import Path
import sys
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import deploy_native_model_stage_278 as deploy
from test_native_model_stage_267 import StageGateTests


class AmbiguousArchive281(unittest.TestCase):
    def setUp(self):
        self.fixture = StageGateTests('test_complete_staging_passes')
        self.fixture.setUp()
        self.addCleanup(self.fixture.doCleanups)
        self.client = self.fixture.client
        self.stage = self.fixture.out
        self.server = self.fixture.server

    def test_two_leaf_aliases_classified_ambiguous_without_hashing_either(self):
        parent = self.client / 'Effect'
        (parent / 'test.smd').write_bytes(b'original wrong')
        (parent / 'TEST.SMD').write_bytes(b'other wrong')
        original_hash = deploy._hash
        def guarded_hash(path):
            if path.parent == parent and path.suffix.lower() == '.smd':
                raise AssertionError('hashed ambiguous client bytes')
            return original_hash(path)
        with mock.patch.object(deploy, '_hash', side_effect=guarded_hash):
            totals, rows = deploy.compare(self.client, self.server, self.stage, archival_readonly=True)
        self.assertEqual(totals, {'SAME': 0, 'DIFFERENT': 0, 'MISSING': 0})
        self.assertEqual(rows[0]['state'], 'AMBIGUOUS')
        self.assertEqual(rows[0]['ambiguous_candidates'], ['Effect/TEST.SMD', 'Effect/test.smd'])
        self.assertIsNone(rows[0]['existing_path'])
        self.assertIsNone(rows[0]['existing_sha256'])

    def test_two_parent_aliases_are_not_mislabelled_missing(self):
        a = self.client / 'char'
        a.mkdir()
        (a/'tmABCD').mkdir()
        (a/'TMABCD').mkdir()
        target, changed, candidates = deploy._historical_inspect_target(self.client,
                                              'char/tmABCD/MfhB03bt.smb')
        self.assertIsNone(target)
        self.assertFalse(changed)
        self.assertEqual(candidates, ['char/TMABCD', 'char/tmABCD'])

    def test_cli_reports_ambiguity_and_no_deploy(self):
        p = self.client / 'Effect'
        (p/'test.smd').write_bytes(b'a')
        (p/'TEST.SMD').write_bytes(b'b')
        args = ['deploy_native_model_stage_278.py', 'compare', '--client', str(self.client),
                '--server', str(self.server), '--out', str(self.stage),
                '--archival-readonly', '--details']
        with mock.patch.object(sys, 'argv', args), contextlib.redirect_stdout(io.StringIO()) as out:
            self.assertEqual(deploy.main(), 0)
        self.assertIn('ambiguous=1', out.getvalue())
        self.assertIn('AMBIGUOUS', out.getvalue())
        self.assertIn('candidates=Effect/TEST.SMD,Effect/test.smd', out.getvalue())
        self.assertIn('deployment_allowed=0 client_writes=0', out.getvalue())

    def test_strict_plan_and_compare_still_refuse(self):
        p = self.client / 'Effect'
        (p/'test.smd').write_bytes(b'a')
        (p/'TEST.SMD').write_bytes(b'b')
        with self.assertRaisesRegex(ValueError, 'case-insensitive target path collision'):
            deploy.compare(self.client, self.server, self.stage)
        plan = self.client.parent/'plan281.json'
        with self.assertRaisesRegex(ValueError, 'case-insensitive target path collision'):
            deploy.make_plan(self.client, self.server, self.stage, None, plan,
                             ['Effect/test.ase'], False)
        self.assertFalse(plan.exists())

    def test_symlink_in_collision_still_fail_closed(self):
        p = self.client / 'Effect'
        (p/'test.smd').write_bytes(b'a')
        (p/'TEST.SMD').symlink_to(self.client/'Game.exe')
        with self.assertRaisesRegex(ValueError, 'symlink'):
            deploy.compare(self.client, self.server, self.stage, archival_readonly=True)

    def test_unique_alias_unchanged(self):
        p = self.client/'Effect'/'TEST.SMD'
        p.write_bytes(self.fixture.binary)
        totals, rows = deploy.compare(self.client, self.server, self.stage, archival_readonly=True)
        self.assertEqual(totals['SAME'], 1)
        self.assertEqual(rows[0]['state'], 'SAME')
        self.assertEqual(rows[0]['ambiguous_candidates'], [])
        self.assertTrue(rows[0]['case_variant'])

    def test_absent_target_still_missing(self):
        totals, rows = deploy.compare(self.client, self.server, self.stage, archival_readonly=True)
        self.assertEqual(totals['MISSING'], 1)
        self.assertEqual(rows[0]['state'], 'MISSING')
        self.assertEqual(rows[0]['ambiguous_candidates'], [])

    def test_actual_ase_change_rejected_before_ambiguity_inspection(self):
        p = self.client/'Effect'
        (p/'test.smd').write_bytes(b'a')
        (p/'TEST.SMD').write_bytes(b'b')
        self.fixture.ase_path.write_bytes(b'dirty ASE')
        with self.assertRaisesRegex(ValueError, 'source changed|corrupt|invalid live ASE roles'):
            deploy.compare(self.client, self.server, self.stage, archival_readonly=True)


if __name__=='__main__':
    unittest.main(verbosity=2)

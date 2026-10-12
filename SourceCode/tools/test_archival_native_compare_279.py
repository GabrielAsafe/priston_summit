#!/usr/bin/env python3
"""Regression tests for 279: archival compare reads only; plan/apply still gated."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import sys
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_native_model_stage_267 import StageGateTests
import deploy_native_model_stage_278 as deploy
from verify_archived_native_stage_279 import verify_archived_readonly


class ArchiveCompareTests279(unittest.TestCase):
    def setUp(self):
        self.fixture = StageGateTests('test_complete_staging_passes')
        self.fixture.setUp()
        self.addCleanup(self.fixture.doCleanups)
        self.client = self.fixture.client
        self.server = self.fixture.server
        self.stage = self.fixture.out
        self.ase = self.fixture.ase_path
        self.cache = self.fixture.cache_path
        self.manifest = self.stage / 'manifest.json'
        self.root = self.client.parent

    def report(self):
        return verify_archived_readonly(self.client, self.server, self.stage)

    def test_archival_gate_remains_complete_when_identity_changed(self):
        self.server.write_bytes(b'Server.exe new binary')
        with self.assertRaisesRegex(ValueError, 'version differs'):
            deploy.compare(self.client, self.server, self.stage)
        report = self.report()
        self.assertEqual(report['identity_changed'], ['server_sha256'])
        self.assertEqual(report['verified'], 1)
        self.assertFalse(report['deployment_allowed'])
        totals, rows = deploy.compare(self.client, self.server, self.stage, archival_readonly=True)
        self.assertEqual(totals, {'SAME': 0, 'DIFFERENT': 0, 'MISSING': 1})
        self.assertEqual(rows[0]['cache'], 'Effect/test.smd')

    def test_old_cache_comparison_does_not_write_anything(self):
        cache_client = self.client / 'Effect' / 'test.smd'
        cache_client.write_bytes(b'old cached data')
        self.server.write_bytes(b'changed server')
        paths = (self.server, self.client / 'Game.exe', self.ase, self.cache,
                 cache_client, self.manifest)
        before = [(str(p), p.stat().st_mtime_ns, hashlib.sha256(p.read_bytes()).hexdigest()) for p in paths]
        totals, _ = deploy.compare(self.client, self.server, self.stage, archival_readonly=True)
        self.assertEqual(totals['DIFFERENT'], 1)
        self.assertEqual(before, [(str(p), p.stat().st_mtime_ns,
                                   hashlib.sha256(p.read_bytes()).hexdigest()) for p in paths])

    def test_gate267_still_blocks_plan(self):
        self.server.write_bytes(b'new server')
        plan = self.root / 'deployment-plan.json'
        with self.assertRaisesRegex(ValueError, 'version differs'):
            deploy.make_plan(self.client, self.server, self.stage, None, plan,
                             ['Effect/test.ase'], False)
        self.assertFalse(plan.exists())

    def test_archival_compare_recognizes_changed_client_and_converter(self):
        self.client.joinpath('Game.exe').write_bytes(b'new client')
        report = self.report()
        self.assertEqual(report['identity_changed'], ['client_sha256'])
        old = json.loads(self.manifest.read_text())
        old['identity']['converter_tool_sha256'] = '0' * 64
        self.manifest.write_text(json.dumps(old))
        report = self.report()
        self.assertEqual(report['identity_changed'], ['client_sha256', 'converter_tool_sha256'])

    def test_corrupted_model_cannot_be_compared_as_verified(self):
        self.server.write_bytes(b'new server')
        self.cache.write_bytes(self.cache.read_bytes() + b'x')
        with self.assertRaisesRegex(ValueError, 'cache corrupt'):
            deploy.compare(self.client, self.server, self.stage, archival_readonly=True)

    def test_changed_ase_cannot_be_compared_as_verified(self):
        self.ase.write_bytes(b'*GEOMOBJECT {\n *NODE_NAME "DIFFERENT"\n}\n')
        with self.assertRaisesRegex(ValueError, 'source changed'):
            self.report()

    def test_partial_scope_refused(self):
        manifest = json.loads(self.manifest.read_text())
        manifest['scope'] = 'pilot'
        self.manifest.write_text(json.dumps(manifest))
        with self.assertRaisesRegex(ValueError, 'only full'):
            self.report()

    def test_missing_expected_cache_refused(self):
        (self.client / 'Effect' / 'other.ase').write_text('*GEOMOBJECT {\n *NODE_NAME "NEW"\n}\n')
        with self.assertRaisesRegex(ValueError, 'coverage differs'):
            self.report()

    def test_untrusted_extra_file_and_symlink_refused(self):
        (self.stage / 'injected.tmp').write_bytes(b'bad')
        with self.assertRaisesRegex(ValueError, 'unexpected extra'):
            self.report()
        (self.stage / 'injected.tmp').unlink()
        self.cache.unlink()
        self.cache.symlink_to(self.server)
        with self.assertRaisesRegex(ValueError, 'symlink'):
            self.report()

    def test_malformed_identity_rejected(self):
        manifest = json.loads(self.manifest.read_text())
        manifest['identity']['server_sha256'] = 'NOT-A-HASH'
        self.manifest.write_text(json.dumps(manifest))
        with self.assertRaisesRegex(ValueError, 'invalid recorded'):
            self.report()

    def test_server_path_and_stage_inside_client_refused(self):
        unsafe = self.client / 'stage'
        unsafe.mkdir()
        with self.assertRaisesRegex(ValueError, 'outside client'):
            verify_archived_readonly(self.client, self.server, unsafe)


if __name__ == '__main__':
    unittest.main(verbosity=2)

#!/usr/bin/env python3
"""PATCH 263: actual offline orchestrator using fake native binaries, no D3D/Wine."""
from __future__ import annotations
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from cook_native_models_incremental_263 import prepare_incremental, FORMAT
from test_native_model_cook_259 import NativeCookTests259
from audit_model_roles_246 import audit


class IncrementalTests(unittest.TestCase):
    def fixture(self, directory):
        return NativeCookTests259().fixture(directory)

    def test_first_build_then_zero_work_without_publishing(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            out = pathlib.Path(d) / 'models'
            first = prepare_incremental(*a[:4], out, str(a[4]))
            self.assertEqual((first['converted'], first['reused'], first['complete']), (1, 0, True))
            self.assertEqual(first['format'], FORMAT)
            mtime = (out / 'manifest.json').stat().st_mtime_ns
            with mock.patch('cook_native_models_incremental_263.prepare', side_effect=AssertionError('should not recook')):
                again = prepare_incremental(*a[:4], out, str(a[4]))
            self.assertEqual((again['converted'], again['reused']), (0, 1))
            self.assertEqual((out / 'manifest.json').stat().st_mtime_ns, mtime)
            self.assertFalse((a[0] / 'Effect/Test.smd').exists())

    def test_corrupt_binary_regenerates_and_revalidates(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            out = pathlib.Path(d) / 'models'
            prepare_incremental(*a[:4], out, str(a[4]))
            (out / 'Effect/Test.smd').write_bytes(b'invalid old binary')
            again = prepare_incremental(*a[:4], out, str(a[4]))
            self.assertEqual(again['converted'], 1)
            self.assertTrue((out / 'Effect/Test.smd').read_bytes().startswith(b'SMD Model data Ver 0.62'))

    def test_updated_ASE_reconverts_only_changed_source(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            out = pathlib.Path(d) / 'models'
            prepare_incremental(*a[:4], out, str(a[4]))
            ase = a[0] / 'Effect/Test.ASE'
            ase.write_text('*GEOMOBJECT {\n *NODE_NAME "Different"\n}\n')
            document = json.loads(a[2].read_text())
            document['entries'][0]['source_sha256'] = hashlib.sha256(ase.read_bytes()).hexdigest()
            a[2].write_text(json.dumps(document))
            audit(a[0], a[2], a[3])
            again = prepare_incremental(*a[:4], out, str(a[4]))
            self.assertEqual(again['converted'], 1)
            self.assertEqual(again['artifacts'][0]['source_sha256'], document['entries'][0]['source_sha256'])

    def test_failure_preserves_existing_good_tree(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            out = pathlib.Path(d) / 'models'
            prepare_incremental(*a[:4], out, str(a[4]))
            prior = {p.relative_to(out).as_posix(): p.read_bytes() for p in out.rglob('*') if p.is_file()}
            a[1].write_bytes(b'changed_server_binary')
            with mock.patch('cook_native_models_incremental_263.prepare', side_effect=ValueError('native crash')):
                with self.assertRaisesRegex(ValueError, 'native crash'):
                    prepare_incremental(*a[:4], out, str(a[4]))
            after = {p.relative_to(out).as_posix(): p.read_bytes() for p in out.rglob('*') if p.is_file()}
            self.assertEqual(prior, after)

    def test_binary_identity_invalidates_cached_models(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            out = pathlib.Path(d) / 'models'
            prepare_incremental(*a[:4], out, str(a[4]))
            a[1].write_bytes(b'updated_server_compiler')
            again = prepare_incremental(*a[:4], out, str(a[4]))
            self.assertEqual(again['converted'], 1)
            self.assertEqual(again['reused'], 0)

    def test_no_symlink_or_untrusted_previous_manifest(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            out = pathlib.Path(d) / 'models'
            prepare_incremental(*a[:4], out, str(a[4]))
            old = json.loads((out / 'manifest.json').read_text())
            (out / 'manifest.json').write_text('{}')
            with self.assertRaisesRegex(ValueError, '263 incremental manifest'):
                prepare_incremental(*a[:4], out, str(a[4]))
            (out / 'manifest.json').write_text(json.dumps(old))
            binpath = out / 'Effect/Test.smd'
            binpath.unlink()
            binpath.symlink_to(a[0] / 'Effect/Test.ASE')
            with self.assertRaisesRegex(ValueError, 'symlink'):
                prepare_incremental(*a[:4], out, str(a[4]))

    def test_pilot_reuses_complete_unchanged_set(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            out = pathlib.Path(d) / 'models'
            prepare_incremental(*a[:4], out, str(a[4]))
            result = prepare_incremental(*a[:4], out, str(a[4]), 'Effect/Test.ASE')
            self.assertEqual((result['reused'], result['converted'], result['complete']), (1, 0, True))
            a[1].write_bytes(b'new ABI')
            with self.assertRaisesRegex(ValueError, 'full pass required'):
                prepare_incremental(*a[:4], out, str(a[4]), 'Effect/Test.ASE')


    def test_auto_refresh_audits_and_only_one_changed_source(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            other = a[0] / 'Effect/Another.ASE'
            other.write_text('*GEOMOBJECT {\n *NODE_NAME "Second"\n}\n')
            out = pathlib.Path(d) / 'models'
            def run():
                return subprocess.run([sys.executable, str(ROOT / 'tools/cook_native_models_incremental_263.py'),
                                       '--client', str(a[0]), '--server', str(a[1]),
                                       '--out', str(out), '--wine', str(a[4])],
                                      capture_output=True, text=True)
            first = run()
            self.assertEqual(first.returncode, 0, first.stderr)
            self.assertIn('converted=2', first.stdout)
            second = run()
            self.assertEqual(second.returncode, 0, second.stderr)
            self.assertIn('converted=0 reused=2', second.stdout)
            other.write_text('*GEOMOBJECT {\n *NODE_NAME "Revised"\n}\n')
            third = run()
            self.assertEqual(third.returncode, 0, third.stderr)
            self.assertIn('converted=1 reused=1', third.stdout)
            self.assertFalse((a[0] / 'Effect/Another.smd').exists())

    def test_paths_and_no_generated_content(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            with self.assertRaisesRegex(ValueError, 'outside client'):
                prepare_incremental(*a[:4], a[0] / 'models', str(a[4]))
            with self.assertRaisesRegex(ValueError, 'canonical'):
                prepare_incremental(*a[:4], pathlib.Path(d) / 'SourceCode' / 'Content' / 'canonical' / 'out', str(a[4]))
            self.assertFalse((a[0] / 'Effect/Test.smd').exists())


if __name__ == '__main__':
    unittest.main(verbosity=2)

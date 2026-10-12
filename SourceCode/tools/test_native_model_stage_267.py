#!/usr/bin/env python3
"""Run real offline staging verifier against isolated synthetic ASE catalogs."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cook_native_models_259 import MAGIC
from cook_native_models_incremental_263 import FORMAT, _hash
from verify_native_model_stage_267 import verify_staging


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


class StageGateTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        root = Path(self.tmp.name)
        self.client = root / 'cliente'
        self.client.mkdir()
        (self.client / 'Game.exe').write_bytes(b'test game identity')
        self.server = root / 'Servidor' / 'Server.exe'
        self.server.parent.mkdir()
        self.server.write_bytes(b'test server identity')
        self.ase_path = self.client / 'Effect' / 'test.ase'
        self.ase_path.parent.mkdir()
        self.ase_path.write_text('*GEOMOBJECT {\n *NODE_NAME "MESH"\n}\n', encoding='cp1252')
        self.out = root / 'stage'
        self.out.mkdir()
        self.cache_path = self.out / 'Effect' / 'test.smd'
        self.cache_path.parent.mkdir()
        self.binary = MAGIC + (b'\0' * 90)
        self.cache_path.write_bytes(self.binary)
        self.make_manifest()

    def make_manifest(self, **changes):
        row = {'source': 'Effect/test.ase', 'type': 'smd', 'cache': 'Effect/test.smd',
               'bytes': len(self.binary), 'sha256': digest(self.binary),
               'source_sha256': _hash(self.ase_path), 'native_readback': True,
               'client_readback': True}
        manifest = {'format': FORMAT,
                    'identity': {'client_sha256': _hash(self.client/'Game.exe'),
                                 'server_sha256': _hash(self.server),
                                 'converter_tool_sha256': _hash(Path(__file__).with_name('cook_native_models_259.py'))},
                    'scope': 'all', 'artifacts': [row], 'count': 1,
                    'converted': 1, 'reused': 0, 'complete': True}
        manifest.update(changes)
        (self.out / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')

    def check(self, allow_partial=False):
        return verify_staging(self.client, self.server, self.out, allow_partial)

    def test_complete_staging_passes(self):
        result = self.check()
        self.assertEqual((result['verified'], result['expected'], result['missing']), (1, 1, 0))
        self.assertTrue(result['complete'])

    def test_no_writes_to_client_or_cache(self):
        old = [(p, p.stat().st_mtime_ns, _hash(p)) for p in (self.ase_path, self.cache_path, self.out/'manifest.json')]
        self.check()
        self.assertEqual(old, [(p, p.stat().st_mtime_ns, _hash(p)) for p, _, _ in old])

    def test_pilot_partial_blocked_by_default(self):
        extra = self.client / 'Effect' / 'another.ase'
        extra.write_text('*GEOMOBJECT {\n *NODE_NAME "SECOND"\n}\n', encoding='cp1252')
        self.make_manifest(complete=False, scope='pilot')
        with self.assertRaisesRegex(ValueError, 'partial staging'):
            self.check()
        result = self.check(allow_partial=True)
        self.assertEqual((result['verified'], result['expected'], result['missing']), (1, 2, 1))

    def test_manifest_cannot_claim_complete_with_missing_asset(self):
        (self.client / 'Effect' / 'another.ase').write_text('*GEOMOBJECT {\n *NODE_NAME "A"\n}\n', encoding='cp1252')
        with self.assertRaisesRegex(ValueError, 'completeness claim'):
            self.check(True)

    def test_changed_source_refused(self):
        self.ase_path.write_text('*GEOMOBJECT {\n *NODE_NAME "CHANGED"\n}\n', encoding='cp1252')
        with self.assertRaisesRegex(ValueError, 'source changed'):
            self.check()

    def test_corrupt_binary_refused(self):
        self.cache_path.write_bytes(MAGIC + b'Q' * 90)
        with self.assertRaisesRegex(ValueError, 'cache corrupt'):
            self.check()

    def test_binary_identity_mismatch_refused(self):
        self.server.write_bytes(b'new binary')
        with self.assertRaisesRegex(ValueError, 'version differs'):
            self.check()

    def test_unverified_readback_refused(self):
        data = json.loads((self.out/'manifest.json').read_text())
        data['artifacts'][0]['client_readback'] = False
        (self.out/'manifest.json').write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, 'cache corrupt'):
            self.check()

    def test_extra_untrusted_artifact_refused(self):
        (self.out/'unexpected.smd').write_bytes(self.binary)
        with self.assertRaisesRegex(ValueError, 'unexpected extra file'):
            self.check()

    def test_staging_symlink_refused(self):
        self.cache_path.unlink()
        self.cache_path.symlink_to(self.server)
        with self.assertRaisesRegex(ValueError, 'symlink'):
            self.check()

    def test_staging_inside_client_refused(self):
        dest = self.client/'staging'
        shutil.copytree(self.out, dest)
        with self.assertRaisesRegex(ValueError, 'outside client'):
            verify_staging(self.client, self.server, dest)

    def test_pilot_cannot_claim_full_even_if_all_present(self):
        self.make_manifest(scope='pilot')
        with self.assertRaisesRegex(ValueError, 'scope=all'):
            self.check()


if __name__ == '__main__':
    unittest.main(verbosity=2)

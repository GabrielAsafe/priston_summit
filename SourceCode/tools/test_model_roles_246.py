#!/usr/bin/env python3
"""Portable regression tests for the safe 246 role audit / cache path capacity."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
from audit_model_roles_246 import audit


class RoleTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        root = Path(self.tmp.name)
        self.client = root / 'client'
        self.client.mkdir()
        (self.client / 'Game.exe').write_bytes(b'dummy')
        self.census = root / 'census.json'
        self.result = root / 'roles.json'

    def _create(self, names, content):
        entries = []
        for name in names:
            f = self.client / name
            f.parent.mkdir(parents=True, exist_ok=True)
            f.write_text(content, encoding='cp1252')
            entries.append({'source': name, 'source_sha256': hashlib.sha256(f.read_bytes()).hexdigest(),
                            'cache': {ext: {'path': str(f.with_suffix('.'+ext)), 'status': 'missing'}
                                      for ext in ('smd', 'smb')}})
        self.census.write_text(json.dumps({'format':'SUMMIT_MODEL_CACHE_CENSUS_245_V1',
                            'entries': entries, 'counts': {'sources_ase':len(entries)}}))

    def test_mesh_and_bone_are_different_conversions(self):
        self._create(['Effect/NewEffect/A.ASE'], '*GEOMOBJECT {\n*NODE_NAME "Bip01"\n}\n*GEOMOBJECT {\n*NODE_NAME "Weapon01"\n}\n')
        result = audit(self.client, self.census, self.result)
        self.assertEqual(result['summary']['both'], 1)
        self.assertEqual(result['summary']['needs_smd'], 1)
        self.assertEqual(result['summary']['needs_smb'], 1)
        self.assertEqual(result['entries'][0]['required_cache_types'], ['smd', 'smb'])

    def test_only_mesh_is_not_missing_bone(self):
        self._create(['char/monster/foo.ase'], '*GEOMOBJECT {\n*NODE_NAME "Body"\n}')
        result = audit(self.client, self.census, self.result)
        self.assertEqual(result['summary']['mesh_only'], 1)
        self.assertEqual(result['summary']['needs_smb'], 0)

    def test_only_bone_is_not_missing_mesh(self):
        self._create(['char/monster/foo.ase'], '*GEOMOBJECT {\n*NODE_NAME "Bip Pelvis"\n}')
        result = audit(self.client, self.census, self.result)
        self.assertEqual(result['summary']['bone_only'], 1)
        self.assertEqual(result['summary']['needs_smd'], 0)

    def test_changed_source_fails_without_writes(self):
        self._create(['char/monster/foo.ase'], '*GEOMOBJECT {\n*NODE_NAME "foo"\n}')
        (self.client / 'char/monster/foo.ase').write_text('tampered')
        with self.assertRaisesRegex(ValueError, 'changed'):
            audit(self.client, self.census, self.result)
        self.assertFalse(self.result.exists())

    def test_out_in_client_fails(self):
        self._create(['char/monster/foo.ase'], '*GEOMOBJECT {\n*NODE_NAME "foo"\n}')
        with self.assertRaisesRegex(ValueError, 'output must be outside'):
            audit(self.client, self.census, self.client/'output.json')

    def test_long_name_is_reported_and_safe_path_buffer_updated(self):
        self._create(['image/Sinimage/Effect/SkillEffect/ShieldStrike/SheildStrike.ASE'], '*GEOMOBJECT {\n*NODE_NAME "Body"\n}')
        result = audit(self.client, self.census, self.result)
        self.assertEqual(result['summary']['legacy_cache_path_over_63'], 0) # exactly 63 chars still fits
        base = Path(__file__).resolve().parents[1] / 'Shared/rendering/legacy_3d'
        self.assertIn('thread_local char szChangeFileExt[512];', (base / 'smRead3d_shared.inl').read_text())
        active = (base / 'smRead3d_legacy_client.inl').read_text()
        self.assertIn('thread_local char szChangeFileExt[512];', active)
        self.assertIn('summit::io::ReplaceFileExtension(', active)

    def test_symlink_source_fails(self):
        self._create(['char/monster/foo.ase'], '*GEOMOBJECT {\n*NODE_NAME "foo"\n}')
        original = self.client/'char/monster/foo.ase'
        moved = original.with_name('real.ase')
        original.rename(moved)
        original.symlink_to(moved)
        with self.assertRaisesRegex(ValueError, 'symlink'):
            audit(self.client, self.census, self.result)


if __name__ == '__main__':
    unittest.main(verbosity=2)

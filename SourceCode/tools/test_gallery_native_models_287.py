#!/usr/bin/env python3
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
import gallery_native_models_287 as g


class GalleryTests287(unittest.TestCase):
    def make_stage(self, root: Path, modify=None):
        stage = root / 'staging'
        stage.mkdir()
        rows = []
        for i in range(134):
            ext = '.smb' if i % 4 == 0 else '.smd'
            name = f'char/monster/model_{i:03}{ext}'
            p = stage / name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(f'binary-{i}'.encode())
            rows.append({'cache': name, 'source': f'char/monster/model_{i:03}.ASE',
                         'sha256': g.digest(p)})
        if modify: modify(stage, rows)
        (stage / 'manifest.json').write_text(json.dumps({'complete': True, 'artifacts': rows}))
        return stage

    def test_134_manifest_and_document(self):
        with tempfile.TemporaryDirectory() as t:
            _, rows = g.validate_manifest(self.make_stage(Path(t)))
            self.assertEqual(len(rows), 134)
            self.assertEqual(g.checklist(rows).count('| [ ] OK'), 134)
            h = g.header(rows, 'SUMMIT_NATIVE_MODELS_INCREMENTAL_263')
            self.assertEqual(h.count('..\\\\bugfixes\\\\SUMMIT_NATIVE_MODELS_INCREMENTAL_263'), 134)
            self.assertIn("GetAsyncKeyState('9')", h)
            self.assertIn("GetAsyncKeyState('0')", h)
            self.assertNotIn('GetAsyncKeyState(VK_NEXT)', h)
            self.assertNotIn('GetAsyncKeyState(VK_PRIOR)', h)
            self.assertIn('smReadModel_Bip(', h)
            self.assertIn('smReadModel(', h)
            self.assertNotIn('smASE_Read(', h)
            self.assertNotIn('LOGS\\', h)

    def test_changed_binary_fails_closed(self):
        with tempfile.TemporaryDirectory() as t:
            stage = self.make_stage(Path(t))
            (stage / 'char/monster/model_050.smd').write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError, 'hash differs'):
                g.validate_manifest(stage)

    def test_missing_binary_fails_closed(self):
        with tempfile.TemporaryDirectory() as t:
            stage = self.make_stage(Path(t))
            (stage / 'char/monster/model_050.smd').unlink()
            with self.assertRaisesRegex(ValueError, 'missing/untrusted'):
                g.validate_manifest(stage)

    def test_extra_catalog_number_rejected(self):
        with tempfile.TemporaryDirectory() as t:
            stage = self.make_stage(Path(t))
            m = json.loads((stage / 'manifest.json').read_text())
            m['artifacts'].pop()
            (stage / 'manifest.json').write_text(json.dumps(m))
            with self.assertRaisesRegex(ValueError, '134-artifact'):
                g.validate_manifest(stage)

    def test_case_collision_rejected(self):
        with tempfile.TemporaryDirectory() as t:
            def mutate(stage, rows):
                row = dict(rows[1]); row['cache'] = rows[0]['cache'].upper(); rows[1] = row
            stage = self.make_stage(Path(t), mutate)
            with self.assertRaisesRegex(ValueError, 'duplicate case-insensitive'):
                g.validate_manifest(stage)

    def test_source_patch_and_noop_refusal(self):
        src = Path('/mnt/data/audit286/src/SrcGame/src/platform/windows/application/Winmain.cpp')
        if not src.exists():
            self.skipTest('source fixture unavailable')
        initial = src.read_text()
        patched = g.patched_source(initial)
        self.assertEqual(patched.count(g.MARK), 3)
        self.assertIn('SummitCookedGallery287Draw(', patched)
        self.assertIn('SummitCookedGallery287Labels()', patched)
        with self.assertRaisesRegex(ValueError, 'already inserted'):
            g.patched_source(patched)

    def test_malformed_layout_refused(self):
        with self.assertRaisesRegex(ValueError, 'layout differs'):
            g.patched_source('void smPlayD3D() {}')

    def test_no_git_index_or_canonical_code(self):
        code = Path(g.__file__).read_text()
        self.assertNotIn('git add', code)
        self.assertNotIn('git reset', code)
        self.assertNotIn('canonical/', code)


if __name__ == '__main__':
    unittest.main(verbosity=2)

#!/usr/bin/env python3
"""Regression gates for offline performance restoration and runtime skip paths."""
from __future__ import annotations
import importlib.util
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from PIL import Image

TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))
import cook_textures_dds as cooker
import restore_cooked_runtime_performance_v1 as recovery


class OfflineRecoveryTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='summit_perf_test_')
        self.addCleanup(self.tmp.cleanup)
        self.client = Path(self.tmp.name)
        (self.client / 'image').mkdir()
        (self.client / 'cooked').mkdir()

    def sample(self, rel='field/rock.bmp', colored=(12, 70, 120, 255), material=True):
        source = self.client / rel
        source.parent.mkdir(parents=True, exist_ok=True)
        image = Image.new('RGBA', (128,128), colored)
        if source.suffix.lower() == '.bmp':
            image.convert('RGB').save(source)
        else:
            image.save(source)
        target = self.client / 'cooked' / Path(rel).with_suffix('.dds')
        target.parent.mkdir(parents=True, exist_ok=True)
        semantic = cooker.apply_cook_profile(image, 'material', source.suffix)
        fmt = 'DXT5' if cooker.has_semantic_alpha(semantic) else 'DXT1'
        payload, _ = cooker.encode_dds(semantic, fmt, False)
        target.write_bytes(payload)
        return source, target

    def test_audit_is_read_only_and_apply_copies_only(self):
        source, old = self.sample()
        old_sha = recovery.sha256(old)
        before = {p.relative_to(self.client).as_posix() for p in self.client.rglob('*') if p.is_file()}
        found, counts = recovery.audit(self.client)
        after = {p.relative_to(self.client).as_posix() for p in self.client.rglob('*') if p.is_file()}
        self.assertEqual(before, after)
        self.assertEqual(len(found), 1, counts)
        recovery.publish(self.client, found, counts)
        new = self.client / 'cooked/material/field/rock.bmp.dds'
        self.assertTrue(new.is_file())
        self.assertEqual(recovery.sha256(new), old_sha)
        self.assertEqual(recovery.sha256(old), old_sha)
        again, counts2 = recovery.audit(self.client)
        self.assertEqual(again, [])
        self.assertEqual(counts2['already_present'], 1)

    def test_collision_bmp_tga_fails_closed(self):
        _, old = self.sample('field/shared.bmp')
        second = self.client / 'field/shared.tga'
        second.parent.mkdir(parents=True, exist_ok=True)
        Image.new('RGB', (128,128), (255,0,0)).save(second)
        found, counts = recovery.audit(self.client)
        self.assertFalse(found)
        self.assertEqual(counts['legacy_collision_sources'], 2)

    def test_invalid_and_mismatched_dds_rejected(self):
        _, old = self.sample('field/a.bmp')
        _, other = self.sample('field/b.bmp', colored=(250,5,30,255))
        other.write_bytes(old.read_bytes())
        found, counts = recovery.audit(self.client)
        self.assertEqual(len(found), 1, counts)
        self.assertGreaterEqual(counts['semantic_mismatch_or_unproven'], 1)
        old.write_bytes(b'not DDS')
        found, counts = recovery.audit(self.client)
        self.assertFalse(found)
        self.assertEqual(counts['legacy_dds_invalid_header'], 1)

    def test_dib_and_excluded_families_never_migrate(self):
        self.sample('image/Sinimage/Items/Accessory/icon.bmp')
        self.sample('image/Sinimage/Items/DropItem/ground.tga')
        found, counts = recovery.audit(self.client)
        self.assertFalse(found)
        self.assertEqual(counts['excluded_unproven_family'], 2)

    def test_stale_dds_not_migrated(self):
        src, old = self.sample()
        os.utime(old, (src.stat().st_mtime - 10, src.stat().st_mtime - 10))
        found, counts = recovery.audit(self.client)
        self.assertFalse(found)
        self.assertEqual(counts['legacy_dds_stale'], 1)

    def test_runtime_profile_short_circuit(self):
        cpp = (TOOLS.parent / 'Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl').read_text()
        self.assertIn('static bool SummitMaterialCookedRootReady()', cpp)
        self.assertIn('SummitCookedTexturesEnabled() && SummitMaterialCookedRootReady()', cpp)
        self.assertIn('static bool SummitDibCookedLookupEnabled()', cpp)
        self.assertIn('SummitIsProvenInventoryDibPath(Filename)', cpp)
        self.assertIn('SUMMIT_COOKED_DIB', cpp)


if __name__ == '__main__':
    unittest.main(verbosity=2)

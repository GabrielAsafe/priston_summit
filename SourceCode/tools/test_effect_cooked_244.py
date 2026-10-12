#!/usr/bin/env python3
"""PATCH 244 tests: effect inventory, safety and cooked-first sync path."""
from __future__ import annotations
import argparse
import json
import os
from pathlib import Path
import tempfile
import unittest
import sys

from PIL import Image
import audit_effect_cooked_244 as audit
import cook_all_missing_243 as cooker


class EffectCooked244Tests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.client = self.root / 'cliente'
        self.client.mkdir()
        self.output = self.root / 'report.json'

    def image(self, relative):
        path = self.client / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        Image.new('RGBA', (8, 8), (15, 33, 90, 255)).save(path)
        return path

    def test_effect_images_count_even_when_outside_effect_root(self):
        self.image('Effect/Particle/magic.tga')
        self.image('image/Sinimage/Skill/Burst.bmp')
        self.image('Field/Ricarten/Floor.bmp')
        report = audit.audit(self.client)
        self.assertEqual(report['effect_image_sources'], 2)
        self.assertEqual(report['effect_image_counts']['missing_dds'], 2)
        self.assertEqual(len(report['effect_images']), 2)

    def test_effect_cooked_exists_without_replacing_original(self):
        source = self.image('Effect/Particle/magic.tga')
        relative = 'Effect/Particle/magic.tga'
        target = self.client / 'cooked/material/Effect/Particle/magic.tga.dds'
        target.parent.mkdir(parents=True)
        data, _ = cooker.encode(source, relative, 'material')
        target.write_bytes(data)
        os.utime(target, (source.stat().st_mtime + 3, source.stat().st_mtime + 3))
        report = audit.audit(self.client)
        self.assertEqual(report['effect_image_counts'], {'dds_present': 1})
        self.assertEqual(source.suffix, '.tga')

    def test_effect_scripts_models_are_not_implicitly_cooked(self):
        path = self.client / 'Effect/Particle/Fire.ase'
        path.parent.mkdir(parents=True)
        path.write_text('model')
        script = path.with_suffix('.ini')
        script.write_text('effect=fire')
        report = audit.audit(self.client)
        self.assertEqual(report['effect_image_sources'], 0)
        self.assertEqual(report['effect_nonimages_counts'],
                         {'effect_model': 1, 'effect_script_or_data': 1})

    def test_sky_collisions_not_relabelled_as_attack_effects(self):
        self.image('StartImage/sky/Dsky00.bmp')
        self.image('StartImage/sky/dsky00.bmp')
        report = audit.audit(self.client)
        self.assertEqual(report['effect_image_sources'], 0)
        self.assertEqual(report['global_243_summary']['ambiguous'], 2)

    def test_report_must_not_be_inside_client(self):
        self.image('Effect/Particle/magic.tga')
        self.assertEqual(audit.main(['--client', str(self.client), '--out',
                                     str(self.client / 'report.json')]), 2)
        self.assertFalse((self.client / 'report.json').exists())

    def test_client_source_cpp_244_fast_path(self):
        if not CPP_SOURCE:
            self.skipTest('optional source not supplied')
        source = (CPP_SOURCE / 'Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl').read_text()
        self.assertIn('Summit244TryCookedTextureNow', source)
        self.assertIn('SummitCookedTextureRuntimeEligible(handle->Name)', source)
        self.assertIn('SummitCookedTextureIsFresh(logicalPath, cookedPath)', source)
        self.assertIn('SummitRejectCookedTexture(logicalPath)', source)
        self.assertIn('if (Summit244TryCookedTextureNow(lpTexHandle, assetIndex, MipMap))', source)
        self.assertLess(source.index('if (Summit244TryCookedTextureNow(lpTexHandle, assetIndex, MipMap))'),
                        source.index('fopen_s(&pFile, lpTexHandle->Name, "rb");'))
        self.assertIn('SummitTextureRecordCreateSuccess(handle, info.Width, info.Height)', source)
        # Assert the first-use special effect owner still requests the sync path.
        effect = (CPP_SOURCE / 'Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl').read_bytes().decode('latin-1')
        self.assertIn('AssetLoadV2LoadTextureNow(material.smTexture[0], SMASSET_OWNER_EFFECT, 1)', effect)

CPP_SOURCE = None
if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--source-code', type=Path)
    opts = p.parse_args()
    CPP_SOURCE = opts.source_code
    unittest.main(argv=[sys.argv[0]], verbosity=2)

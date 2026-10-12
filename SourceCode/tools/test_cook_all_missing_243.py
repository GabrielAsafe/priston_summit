#!/usr/bin/env python3
"""PATCH 243 regression: broad coverage, fail closed, no overwrites or client writes on stage."""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from PIL import Image

HERE = Path(__file__).resolve().parent
CMD = HERE / 'cook_all_missing_243.py'


class CookAll243Tests(unittest.TestCase):
    def setUp(self):
        self.context = tempfile.TemporaryDirectory()
        self.addCleanup(self.context.cleanup)
        self.root = Path(self.context.name)
        self.client = self.root / 'cliente'
        self.client.mkdir()
        self.staging = self.root / 'bugfixes' / 'STAGE'
        self.original_manifest = self.client / 'cooked' / 'manifest.json'
        self.original_manifest.parent.mkdir(parents=True)
        self.original_manifest.write_bytes(b'{"legacy": "untouched"}\n')
        self.environ = os.environ.copy()
        self.environ['PYTHONPATH'] = str(HERE) + os.pathsep + str(HERE.parents[1] / 'source' / 'tools') + os.pathsep + self.environ.get('PYTHONPATH','')

    def img(self, name, size=(16, 16), fmt=None):
        file = self.client / name
        file.parent.mkdir(parents=True, exist_ok=True)
        Image.new('RGB', size, (100, 75, 20)).save(file, format=fmt)
        return file

    def run_cli(self, action, expect=0):
        p = subprocess.run([sys.executable, str(CMD), action,
                            '--client', str(self.client), '--staging', str(self.staging)],
                           capture_output=True, text=True, env=self.environ)
        self.assertEqual(p.returncode, expect, p.stdout + '\n' + p.stderr)
        return p

    def test_stage_all_small_and_publish_no_manifest_write(self):
        self.img('Field/Ricarten/tiny.bmp', (2, 2))
        self.img('Effect/particle/a.png', (17, 13))
        self.img('image/Sinimage/Items/WA/item.bmp', (32, 32))
        self.img('StartImage/login/CharacterInfo.tga', (8, 8), 'TGA')
        # Case-only collision: neither source is auto-selected.
        self.img('char/tmABCD/A.bmp')
        self.img('char/tmABCD/a.BMP')
        corrupted = self.client / 'Effect' / 'invalid.bmp'
        corrupted.write_bytes(b'no image')
        self.run_cli('scan')
        scan = json.loads((self.staging / 'scan_243.json').read_text())
        self.assertEqual(scan['counts']['ambiguous'], 2)
        self.assertEqual(scan['counts']['missing'], 5)
        self.run_cli('stage')
        stage = json.loads((self.staging / 'stage_243.json').read_text())
        self.assertEqual(stage['staged'], 4)
        self.assertEqual(stage['failed'], 1)
        self.assertEqual(self.original_manifest.read_bytes(), b'{"legacy": "untouched"}\n')
        self.assertFalse((self.client / 'cooked' / 'material' / 'Field').exists())
        self.run_cli('apply')
        self.assertEqual(self.original_manifest.read_bytes(), b'{"legacy": "untouched"}\n')
        self.assertTrue((self.client / 'cooked' / 'material' / 'Field/Ricarten/tiny.bmp.dds').exists())
        self.assertTrue((self.client / 'cooked' / 'dib' / 'image/Sinimage/Items/WA/item.bmp.dds').exists())
        self.assertFalse((self.client / 'cooked' / 'material' / 'char').exists())
        self.run_cli('apply')
        receipt = json.loads((self.staging / 'receipt_243.json').read_text())
        self.assertEqual(receipt['already_identical'], 4)
        self.assertEqual(receipt['published_this_run'], 0)

    def test_modified_source_aborts_before_any_publish(self):
        path = self.img('Field/A/a.bmp')
        self.img('Field/B/b.bmp')
        self.run_cli('stage')
        Image.new('RGB', (16, 16), (22, 13, 10)).save(path)
        self.run_cli('apply', expect=2)
        self.assertFalse((self.client / 'cooked' / 'material').exists())

    def test_tampered_staging_aborts_before_any_publish(self):
        self.img('Field/A/a.bmp')
        self.img('Field/B/b.bmp')
        self.run_cli('stage')
        target = self.staging / 'payload/material/Field/B/b.bmp.dds'
        with target.open('ab') as f:
            f.write(b'X')
        self.run_cli('apply', expect=2)
        self.assertFalse((self.client / 'cooked' / 'material').exists())

    def test_existing_artifact_is_not_overwritten(self):
        self.img('Field/A/a.bmp')
        self.img('Field/B/b.bmp')
        self.run_cli('stage')
        existing = self.client / 'cooked/material/Field/B/b.bmp.dds'
        existing.parent.mkdir(parents=True)
        existing.write_bytes(b'untrusted existing')
        self.run_cli('apply', expect=2)
        self.assertEqual(existing.read_bytes(), b'untrusted existing')
        self.assertFalse((self.client / 'cooked/material/Field/A/a.bmp.dds').exists())

    def test_symlink_destination_aborts_safely(self):
        self.img('Field/Ricarten/a.bmp')
        self.run_cli('stage')
        outside = self.root / 'other'
        outside.mkdir()
        cooked_material = self.client / 'cooked' / 'material'
        cooked_material.symlink_to(outside, target_is_directory=True)
        self.run_cli('apply', expect=2)
        self.assertEqual(list(outside.iterdir()), [])

    def test_symlink_source_not_followed(self):
        real = self.img('Field/A/a.bmp')
        (self.client / 'Field/A/link.bmp').symlink_to(real)
        self.run_cli('stage')
        stage = json.loads((self.staging / 'stage_243.json').read_text())
        self.assertEqual(stage['staged'], 1)
        self.assertTrue(any(x['reason'] == 'unsafe_symlink_or_nonfile' for x in stage['exclusions']))


if __name__ == '__main__':
    unittest.main(verbosity=2)

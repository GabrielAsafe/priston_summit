#!/usr/bin/env python3
"""Focused CLI regression for profile-aware cooked image identity and alpha."""
from __future__ import annotations

import importlib.util
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("summit_cooker", ROOT / "tools" / "cook_textures_dds.py")
cooker = importlib.util.module_from_spec(SPEC)
assert SPEC.loader
SPEC.loader.exec_module(cooker)


def synthetic_bmp(size: int) -> Image.Image:
    image = Image.new("RGBA", (size, size), (250, 5, 5, 255))
    for x in range(size):
        for y in range(size):
            if x < 4 or y < 4:
                image.putpixel((x, y), (0, 0, 0, 255))
            elif x < 8 or y < 8:
                image.putpixel((x, y), (240, 255, 255, 255))
    return image


class CookedIdentityTest(unittest.TestCase):
    def test_identity_extension_and_profile(self):
        bmp = cooker.cooked_relative_path("image/Sinimage/Items/DropItem/a.bmp", "material")
        tga = cooker.cooked_relative_path("image/Sinimage/Items/DropItem/a.tga", "material")
        dib = cooker.cooked_relative_path("image/Sinimage/Items/DropItem/a.bmp", "dib")
        self.assertNotEqual(bmp, tga)
        self.assertNotEqual(bmp, dib)
        self.assertEqual(bmp.as_posix(), "material/image/Sinimage/Items/DropItem/a.bmp.dds")

    def test_roles(self):
        self.assertEqual(cooker.cook_profile_for(Path("image/Sinimage/Items/WA/itwa101.bmp")), "dib")
        self.assertEqual(cooker.cook_profile_for(Path("image/Sinimage/Items/DropItem/a.bmp")), "material")
        self.assertEqual(cooker.cook_profile_for(Path("Field/forest/a.tga")), "material")

    def test_alpha_profiles_neutral_decode(self):
        with tempfile.TemporaryDirectory() as directory:
            p = Path(directory) / "in.bmp"
            src = synthetic_bmp(64).convert("RGB")
            src.save(p)
            neutral, _ = cooker.open_legacy_image_with_info(p)
            self.assertEqual(neutral.getpixel((0, 0))[3], 255)
            self.assertEqual(neutral.getpixel((5, 5))[3], 255)
            dib = cooker.apply_cook_profile(neutral, "dib", ".bmp")
            material = cooker.apply_cook_profile(neutral, "material", ".bmp")
            self.assertEqual(dib.getpixel((0, 0))[3], 0)
            self.assertEqual(dib.getpixel((5, 5))[3], 255)
            self.assertEqual(material.getpixel((0, 0))[3], 255)
            self.assertEqual(material.getpixel((5, 5))[3], 0)
            self.assertEqual(dib.getpixel((10, 10))[3], 255)
            self.assertEqual(material.getpixel((10, 10))[3], 255)

            authored = Image.new("RGBA", (16, 16), (1, 2, 3, 100))
            self.assertEqual(cooker.apply_cook_profile(authored, "dib", ".tga").getpixel((0, 0))[3], 100)
            self.assertEqual(cooker.apply_cook_profile(authored, "material", ".png").getpixel((0, 0))[3], 100)

    def test_encode_small_and_large_alpha(self):
        for profile, key in [("dib", (0, 0)), ("material", (5, 5))]:
            for size, expected_format in [(64, "A8R8G8B8"), (1024, "DXT5")]:
                with self.subTest(profile=profile, size=size):
                    image = cooker.apply_cook_profile(synthetic_bmp(size), profile, ".bmp")
                    fmt = cooker.select_dds_format(image, lossless=(size < 128))
                    self.assertEqual(fmt, expected_format)
                    encoded, _ = (cooker.encode_argb8888_dds(image) if fmt == "A8R8G8B8"
                                  else cooker.encode_dds(image, fmt, False))
                    with tempfile.TemporaryDirectory() as directory:
                        path = Path(directory) / "one.dds"
                        path.write_bytes(encoded)
                        self.assertEqual(cooker.readable_dds_header(path), (size, size, fmt))
                        with Image.open(path) as decoded:
                            rgba = decoded.convert("RGBA")
                            self.assertLessEqual(rgba.getpixel(key)[3], 8)
                            self.assertGreaterEqual(rgba.getpixel((10, 10))[3], 240)

    def test_casefold_ambiguity_fail_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            files = [root / "a.bmp", root / "A.bmp"]
            for file in files:
                file.write_bytes(b"fake")
            with self.assertRaisesRegex(ValueError, "ambiguous Windows cooked destination"):
                cooker.preflight_casefold_destinations(files, root)

    def test_manifest_and_reuse_invalidation(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "client"
            source = root / "image" / "Sinimage" / "Items" / "WA" / "itwa101.bmp"
            source.parent.mkdir(parents=True)
            synthetic_bmp(64).convert("RGB").save(source)
            cmd = [sys.executable, str(ROOT / "tools" / "cook_textures_dds.py"),
                   "--input", str(root), "--item-icons-only", "--quiet"]
            for expected in ("cooked", "up-to-date", "cooked"):
                run = subprocess.run(cmd, text=True, capture_output=True)
                self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
                manifest = json.loads((root / "cooked" / "manifest.json").read_text())
                row = manifest["entries"][0]
                self.assertEqual(row["status"], expected)
                self.assertEqual(row["cook_profile"], "dib")
                self.assertTrue(row["source_sha256"])
                target = root / "cooked" / row["cooked"]
                self.assertTrue(target.exists())
                if expected == "up-to-date":
                    data = bytearray(target.read_bytes())
                    data[-1] ^= 0xFF
                    target.write_bytes(data)

    def test_patch24_item_icon_policy_preserved(self):
        self.assertTrue(cooker.is_item_icon(Path("image/Sinimage/Items/DropItem/a.tga")))
        self.assertTrue(cooker.inventory_dib_image(Path("image/Sinimage/Items/WA/itwa101.bmp")))
        self.assertFalse(cooker.inventory_dib_image(Path("image/Sinimage/Items/DropItem/a.bmp")))
        self.assertEqual(cooker.item_icon_pixel_format(Image.new("RGBA", (64, 64)), Path("icon.bmp"), 1048576)[0], "A8R8G8B8")
        alpha = Image.new("RGBA", (1024, 1024), (100, 2, 3, 255))
        alpha.putpixel((0, 0), (0, 0, 0, 0))
        self.assertEqual(cooker.item_icon_pixel_format(alpha, Path("icon.bmp"), 1048576)[0], "DXT5")

    def test_focused_item_scope_excludes_unproven_drop_assets(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "client"
            for name in ("WA/itwa101.bmp", "DropItem/itwa101.bmp", "DropItem/itwa101.tga"):
                path = root / "image" / "Sinimage" / "Items" / name
                path.parent.mkdir(parents=True, exist_ok=True)
                synthetic_bmp(64).convert("RGB").save(path, format="BMP")
            args = [sys.executable, str(ROOT / "tools" / "cook_textures_dds.py"), "--input", str(root), "--item-icons-only", "--quiet"]
            proc = subprocess.run(args, text=True, capture_output=True)
            self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
            manifest = json.loads((root / "cooked" / "manifest.json").read_text())
            self.assertEqual(len(manifest["entries"]), 1)
            self.assertEqual(manifest["entries"][0]["source"], "image/Sinimage/Items/WA/itwa101.bmp")

    def test_casefold_cli_no_publish(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ("abwa101.bmp", "ABWA101.bmp"):
                path = root / "image" / "Sinimage" / "Items" / "WA" / name
                path.parent.mkdir(parents=True, exist_ok=True)
                synthetic_bmp(64).convert("RGB").save(path)
            run = subprocess.run([sys.executable, str(ROOT / "tools" / "cook_textures_dds.py"),
                                  "--input", str(root), "--item-icons-only"],
                                 text=True, capture_output=True)
            self.assertEqual(run.returncode, 2)
            self.assertIn("ambiguous Windows cooked destination", run.stderr)
            self.assertFalse((root / "cooked").exists())


if __name__ == "__main__":
    unittest.main(verbosity=2)

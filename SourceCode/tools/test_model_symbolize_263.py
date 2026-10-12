#!/usr/bin/env python3
"""Sanity tests for local-only Wine address resolver."""
import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from resolve_model_cook_crash_263 import resolve

class Tests(unittest.TestCase):
    def test_symbols_from_real_debug_binary(self):
        if not shutil.which('cc') or not shutil.which('nm') or not shutil.which('addr2line'):
            self.skipTest('ELF test toolchain unavailable')
        with tempfile.TemporaryDirectory() as d:
            binary = pathlib.Path(d) / 't'
            src = binary.with_suffix('.c')
            src.write_text('int isolated_test_function(void){return 23;}\nint main(void){return isolated_test_function();}\n')
            subprocess.run(['cc', '-g', '-O0', '-fno-pie', '-no-pie', str(src), '-o', str(binary)],
                           check=True, capture_output=True)
            nm = subprocess.run(['nm', '-n', str(binary)], capture_output=True, text=True, check=True)
            address = next(row.split()[0] for row in nm.stdout.splitlines() if row.endswith(' isolated_test_function'))
            info = resolve(binary, ['0x' + address], shutil.which('addr2line'))
            self.assertIn('isolated_test_function', info[0][1])
            self.assertIn('t.c:', info[0][2])

    def test_rejects_invalid_address(self):
        with tempfile.TemporaryDirectory() as d:
            exe = pathlib.Path(d) / 'exe'
            exe.write_bytes(b'not a binary')
            with self.assertRaisesRegex(ValueError, 'invalid hexadecimal'):
                resolve(exe, ['something_bad'])

if __name__ == '__main__':
    unittest.main(verbosity=2)

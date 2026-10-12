#!/usr/bin/env python3
"""PATCH 272: explicit current Game.exe for offline cook; safe early probe."""
from __future__ import annotations
import hashlib
import pathlib
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

TOOLS = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))
from test_native_model_cook_259 import NativeCookTests259
from cook_native_models_259 import probe_client_verifier, select_client_executable
from cook_native_models_incremental_263 import prepare_incremental
from verify_native_model_stage_267 import verify_staging


class ClientVerifier272(unittest.TestCase):
    def make(self, tmp: str):
        c, s, census, roles, runner = NativeCookTests259().fixture(tmp)
        # The old copy in cliente is kept intact; it is *not* trusted for the override.
        fresh = pathlib.Path(tmp) / 'Área de Trabalho/SourceCode/dist_mingw_client_i686/Game.exe'
        fresh.parent.mkdir(parents=True)
        fresh.write_bytes(b'MZ' + b'\x00' * 30 + b'--summit-verify-model' + b'\0' * 128)
        script = runner.read_text()
        original = "if not p.is_file() or not p.read_bytes().startswith(b'SMD Model data Ver 0.62'):sys.exit(8)"
        self.assertIn(original, script)
        script = script.replace(original, "if p.name == '__summit_offline_probe_272_missing__.smb':sys.exit(2)\n  " + original)
        runner.write_text(script)
        return c, s, census, roles, runner, fresh

    def test_new_executable_selected_and_no_client_writes(self):
        with tempfile.TemporaryDirectory() as d:
            c, s, census, roles, wine, fresh = self.make(d)
            old = (c/'Game.exe').read_bytes()
            out = pathlib.Path(d)/'staged'
            report = prepare_incremental(c, s, census, roles, out, str(wine), None, fresh)
            self.assertEqual(report['converted'], 1)
            self.assertTrue(report['complete'])
            self.assertEqual(report['identity']['client_sha256'], hashlib.sha256(fresh.read_bytes()).hexdigest())
            self.assertEqual((c/'Game.exe').read_bytes(), old)
            self.assertFalse((c/'Effect/Test.smd').exists())
            self.assertEqual(verify_staging(c, s, out, False, fresh)['verified'], 1)
            with self.assertRaisesRegex(ValueError, 'version differs'):
                verify_staging(c, s, out)

    def test_short_probe_refuses_old_binary_before_native_conversion(self):
        with tempfile.TemporaryDirectory() as d:
            c, s, census, roles, wine, fresh = self.make(d)
            fresh.write_bytes(b'MZ--old-game-without-verifier')
            out = pathlib.Path(d)/'staged'
            with self.assertRaisesRegex(ValueError, 'lacks the offline verifier'):
                prepare_incremental(c, s, census, roles, out, str(wine), None, fresh)
            self.assertFalse(out.exists())

    def test_wrong_status_refused(self):
        with tempfile.TemporaryDirectory() as d:
            c, s, census, roles, wine, fresh = self.make(d)
            script = wine.read_text().replace("p.name == '__summit_offline_probe_272_missing__.smb':sys.exit(2)",
                                              "p.name == '__summit_offline_probe_272_missing__.smb':sys.exit(8)")
            wine.write_text(script)
            with self.assertRaisesRegex(ValueError, 'returned rc=8'):
                probe_client_verifier(fresh, str(wine), pathlib.Path(d))

    def test_probe_timeout_is_bounded_and_no_gameplay(self):
        with tempfile.TemporaryDirectory() as d:
            c, s, census, roles, wine, fresh = self.make(d)
            with mock.patch('cook_native_models_259.subprocess.run',
                            side_effect=subprocess.TimeoutExpired(['wine'], 0.01)):
                with self.assertRaisesRegex(ValueError, 'did not exit quickly'):
                    probe_client_verifier(fresh, str(wine), pathlib.Path(d), timeout=1)

    def test_refuses_symlink_and_wrong_name(self):
        with tempfile.TemporaryDirectory() as d:
            c, s, census, roles, wine, fresh = self.make(d)
            link = pathlib.Path(d)/'Game.exe'
            link.symlink_to(fresh)
            with self.assertRaisesRegex(ValueError, 'symlink'):
                select_client_executable(c, link)
            other = pathlib.Path(d)/'NotGame.exe'; other.write_bytes(fresh.read_bytes())
            with self.assertRaisesRegex(ValueError, 'regular Game.exe'):
                select_client_executable(c, other)

    def test_command_line_and_identity_present(self):
        import inspect
        from cook_native_models_259 import prepare
        self.assertIn('client_exe', inspect.signature(prepare).parameters)
        self.assertIn('client_exe', inspect.signature(prepare_incremental).parameters)
        self.assertIn('client_exe', inspect.signature(verify_staging).parameters)
        for name in ('cook_native_models_259.py', 'cook_native_models_incremental_263.py',
                     'verify_native_model_stage_267.py'):
            self.assertIn('--client-exe', (TOOLS / name).read_text())


if __name__ == '__main__':
    unittest.main(verbosity=2)

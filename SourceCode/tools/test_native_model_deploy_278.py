#!/usr/bin/env python3
"""Portable tests: a real 263 manifest, hash comparisons, explicit deployment/rollback."""
import hashlib
import json
import pathlib
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import deploy_native_model_stage_278 as deploy


def hash_bytes(b):
    return hashlib.sha256(b).hexdigest()


class DeployTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='summit 278 Área ')
        self.addCleanup(self.tmp.cleanup)
        self.root = pathlib.Path(self.tmp.name)
        self.client = self.root / 'cliente'
        self.client.mkdir()
        self.exe = self.client / 'Game.exe'
        self.exe.write_bytes(b'fake Game.exe')
        self.server = self.root / 'Servidor'
        self.server.mkdir()
        self.bin = self.server / 'Server.exe'
        self.bin.write_bytes(b'fake Server.exe')
        self.stage = self.root / 'bugfixes' / 'STAGE'
        self.stage.mkdir(parents=True)
        self.rows = {}
        for name, data in [('char/mon/M01.smb', b'SMD Model data Ver 0.62---bone'),
                           ('Effect/E01.smd', b'SMD Model data Ver 0.62---mesh')]:
            p = self.stage / name
            p.parent.mkdir(parents=True)
            p.write_bytes(data)
            self.rows[name] = {'cache': name, 'source': name[:-3] + 'ASE',
                               'type': name[-3:], 'sha256': hash_bytes(data), 'source_sha256': '0' * 64, 'bytes': len(data), 'native_readback': True, 'client_readback': True}
        self.manifest = {'identity': {'test': 'fixed'}, 'format': 'SUMMIT_NATIVE_MODEL_INCREMENTAL_263_V1',
                         'complete': True, 'scope': 'all', 'count': 2, 'artifacts': list(self.rows.values())}
        (self.stage / 'manifest.json').write_text(json.dumps(self.manifest))
        self.original = self.client / 'char/mon/M01.smb'
        self.original.parent.mkdir(parents=True)
        self.original.write_bytes(b'old cache')
        self.plan_file = self.root / 'bugfixes' / 'review.json'
        self.backup = self.root / 'bugfixes' / 'backup'
        self.patch = mock.patch.object(deploy, 'verify_staging', return_value={'verified': 2, 'expected': 2,
                                                                              'missing': 0, 'complete': True})
        self.patch.start()
        self.addCleanup(self.patch.stop)

    def plan(self, all_models=False):
        return deploy.make_plan(self.client, self.bin, self.stage, self.exe, self.plan_file,
                                [] if all_models else ['char/mon/M01.ASE'], all_models)

    def test_compare_read_only_diff_and_missing(self):
        before = hash_bytes(self.original.read_bytes())
        summary, rows = deploy.compare(self.client, self.bin, self.stage, self.exe)
        self.assertEqual(summary, {'SAME': 0, 'DIFFERENT': 1, 'MISSING': 1})
        self.assertEqual(before, hash_bytes(self.original.read_bytes()))
        self.assertFalse(self.backup.exists())
        self.assertEqual(len(rows), 2)

    def test_plan_then_apply_then_rollback_restores_original(self):
        result = self.plan()
        self.assertEqual(len(result['artifacts']), 1)
        self.assertEqual(self.original.read_bytes(), b'old cache')
        applied = deploy.apply(self.plan_file, self.backup, False)
        self.assertEqual(applied['deployed'], 1)
        self.assertEqual(self.original.read_bytes(), (self.stage / 'char/mon/M01.smb').read_bytes())
        self.assertTrue((self.backup / 'original/char/mon/M01.smb').is_file())
        self.assertEqual(deploy.rollback(self.backup)['restored'], 1)
        self.assertEqual(self.original.read_bytes(), b'old cache')
        self.assertTrue(deploy.rollback(self.backup)['already'])

    def test_full_catalog_requires_second_explicit_ack(self):
        self.plan(all_models=True)
        with self.assertRaisesRegex(ValueError, 'full catalog'):
            deploy.apply(self.plan_file, self.backup, False)
        self.assertFalse(self.backup.exists())
        result = deploy.apply(self.plan_file, self.backup, True)
        self.assertEqual(result['deployed'], 2)
        self.assertTrue((self.client / 'Effect/E01.smd').is_file())
        self.assertEqual(deploy.rollback(self.backup)['restored'], 2)
        self.assertFalse((self.client / 'Effect/E01.smd').exists())
        self.assertEqual(self.original.read_bytes(), b'old cache')

    def test_plan_refuses_changed_client_before_any_write(self):
        self.plan()
        self.original.write_bytes(b'new user edit')
        with self.assertRaisesRegex(ValueError, 'changed since plan'):
            deploy.apply(self.plan_file, self.backup, False)
        self.assertFalse(self.backup.exists())

    def test_refuses_modified_staging_after_plan(self):
        self.plan()
        (self.stage / 'manifest.json').write_text('{}')
        with self.assertRaises((ValueError, KeyError)):
            deploy.apply(self.plan_file, self.backup, False)
        self.assertEqual(self.original.read_bytes(), b'old cache')

    def test_failure_during_second_atomic_copy_rolls_back_first(self):
        self.plan(all_models=True)
        original_copy = deploy._atomic_copy
        count = 0
        def flaky(*args, **kwargs):
            nonlocal count
            count += 1
            if count == 2:
                raise OSError('test forced second publication failure')
            return original_copy(*args, **kwargs)
        with mock.patch.object(deploy, '_atomic_copy', side_effect=flaky):
            with self.assertRaisesRegex(OSError, 'forced'):
                deploy.apply(self.plan_file, self.backup, True)
        # Rollback may call _atomic_copy for the old file but the mock's third
        # call succeeds. All selected caches should be in their old state.
        self.assertEqual(self.original.read_bytes(), b'old cache')
        self.assertFalse((self.client / 'Effect/E01.smd').exists())
        self.assertEqual(json.loads((self.backup / 'receipt.json').read_text())['state'], 'ROLLED_BACK')

    def test_external_edits_refuse_rollback_without_overwrite(self):
        self.plan()
        deploy.apply(self.plan_file, self.backup, False)
        self.original.write_bytes(b'edited after apply')
        with self.assertRaisesRegex(ValueError, 'externally edited'):
            deploy.rollback(self.backup)
        self.assertEqual(self.original.read_bytes(), b'edited after apply')

    def test_backup_corruption_refuses_restore(self):
        self.plan()
        deploy.apply(self.plan_file, self.backup, False)
        (self.backup / 'original/char/mon/M01.smb').write_bytes(b'corrupt')
        with self.assertRaisesRegex(ValueError, 'backup missing or corrupt'):
            deploy.rollback(self.backup)

    def test_symlink_cache_and_casefold_collision_refused(self):
        self.original.unlink()
        self.original.symlink_to(self.exe)
        with self.assertRaisesRegex(ValueError, 'symlink'):
            deploy.compare(self.client, self.bin, self.stage, self.exe)
        self.original.unlink()
        wrong = self.client / 'CHAR'
        wrong.mkdir()
        with self.assertRaisesRegex(ValueError, 'case-insensitive'):
            deploy.make_plan(self.client, self.bin, self.stage, self.exe, self.plan_file,
                             ['char/mon/M01.ASE'], False)

    def test_non_client_output_only_and_plan_immutability(self):
        with self.assertRaisesRegex(ValueError, 'protected|client'):
            deploy.make_plan(self.client, self.bin, self.stage, self.exe,
                             self.client / 'plan.json', ['char/mon/M01.ASE'], False)
        self.plan()
        with self.assertRaises((FileExistsError, ValueError)):
            self.plan()
        with self.assertRaisesRegex(ValueError, 'protected|client'):
            deploy.apply(self.plan_file, self.client / 'backups', False)
        self.assertEqual(self.original.read_bytes(), b'old cache')

    def test_unverified_staging_refused(self):
        with mock.patch.object(deploy, 'verify_staging', side_effect=ValueError('gate FAIL_CLOSED')):
            with self.assertRaisesRegex(ValueError, 'gate FAIL_CLOSED'):
                deploy.compare(self.client, self.bin, self.stage, self.exe)

    def test_plan_tampering_source_not_in_manifest_refused(self):
        self.plan()
        plan = json.loads(self.plan_file.read_text())
        plan['artifacts'][0]['cache'] = '../evil.smd'
        self.plan_file.write_text(json.dumps(plan))
        with self.assertRaises(ValueError):
            deploy.apply(self.plan_file, self.backup, False)


class RealGateIntegration(unittest.TestCase):
    def test_actual_267_gate_then_canary_deploy_rollback(self):
        from test_native_model_stage_267 import StageGateTests
        fixture = StageGateTests('test_complete_staging_passes')
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        base = fixture.client.parent
        plan = base / 'canary278.json'
        backup = base / 'canary278-backup'
        digest = (fixture.out / 'Effect/test.smd').read_bytes()
        results, _ = deploy.compare(fixture.client, fixture.server, fixture.out)
        self.assertEqual(results, {'SAME': 0, 'DIFFERENT': 0, 'MISSING': 1})
        made = deploy.make_plan(fixture.client, fixture.server, fixture.out, None, plan,
                                ['Effect/test.ase'], False)
        self.assertEqual(len(made['artifacts']), 1)
        self.assertEqual(deploy.apply(plan, backup, False)['deployed'], 1)
        self.assertEqual((fixture.client / 'Effect/test.smd').read_bytes(), digest)
        self.assertEqual(deploy.rollback(backup)['restored'], 1)
        self.assertFalse((fixture.client / 'Effect/test.smd').exists())


if __name__ == '__main__':
    unittest.main(verbosity=2)

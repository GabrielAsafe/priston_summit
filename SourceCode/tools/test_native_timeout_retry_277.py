#!/usr/bin/env python3
"""PATCH 277: a single bounded, clean retry for truly timed-out Wine jobs."""
from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import cook_native_models_incremental_263 as incremental
from test_native_model_cook_259 import NativeCookTests259


def timed_out(message: str) -> ValueError:
    try:
        raise subprocess.TimeoutExpired(['wine', 'Server.exe'], 180)
    except subprocess.TimeoutExpired as cause:
        try:
            raise ValueError(message) from cause
        except ValueError as outer:
            return outer


class TimeoutRetryTests277(unittest.TestCase):
    def fixture(self, directory):
        return NativeCookTests259().fixture(directory)

    def test_one_native_timeout_recovers_and_stages_verified_binary(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            dest = Path(d) / 'out'
            real_prepare = incremental.prepare
            calls = []
            def once(*args, **kwargs):
                calls.append(Path(args[4]))
                if len(calls) == 1:
                    raise timed_out('native conversion invocation failed: timed out')
                return real_prepare(*args, **kwargs)
            with mock.patch.object(incremental, 'prepare', side_effect=once):
                result = incremental.prepare_incremental(*a[:4], dest, str(a[4]))
            self.assertEqual(len(calls), 2)
            self.assertNotEqual(calls[0], calls[1])
            self.assertTrue(calls[1].name.endswith('_retry277'))
            self.assertEqual((result['converted'], result['complete']), (1, True))
            self.assertTrue((dest / 'Effect/Test.smd').is_file())
            self.assertFalse((a[0] / 'Effect/Test.smd').exists())
            self.assertEqual(json.loads((dest / 'manifest.json').read_text())['artifacts'][0]['client_readback'], True)

    def test_second_timeout_aborts_without_partial_publish(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            dest = Path(d) / 'out'
            with mock.patch.object(incremental, 'prepare', side_effect=lambda *_args, **_kw: (_ for _ in ()).throw(timed_out('native conversion invocation failed')) ) as fn:
                with self.assertRaisesRegex(ValueError, 'native conversion invocation failed'):
                    incremental.prepare_incremental(*a[:4], dest, str(a[4]))
            self.assertEqual(fn.call_count, 2)
            self.assertFalse(dest.exists())

    def test_second_timeout_preserves_old_staging(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            dest = Path(d) / 'out'
            incremental.prepare_incremental(*a[:4], dest, str(a[4]))
            before = {p.relative_to(dest).as_posix(): p.read_bytes() for p in dest.rglob('*') if p.is_file()}
            a[1].write_bytes(b'new server identity')
            with mock.patch.object(incremental, 'prepare', side_effect=lambda *_a, **_kw: (_ for _ in ()).throw(timed_out('client native readback invocation failed'))) as fn:
                with self.assertRaisesRegex(ValueError, 'client native readback invocation failed'):
                    incremental.prepare_incremental(*a[:4], dest, str(a[4]))
            self.assertEqual(fn.call_count, 2)
            after = {p.relative_to(dest).as_posix(): p.read_bytes() for p in dest.rglob('*') if p.is_file()}
            self.assertEqual(before, after)

    def test_parser_failure_never_retries(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            dest = Path(d) / 'out'
            with mock.patch.object(incremental, 'prepare', side_effect=ValueError('native conversion failed: rc=3 ase_parse')) as fn:
                with self.assertRaisesRegex(ValueError, 'ase_parse'):
                    incremental.prepare_incremental(*a[:4], dest, str(a[4]))
            self.assertEqual(fn.call_count, 1)
            self.assertFalse(dest.exists())

    def test_unexpected_scratch_output_refuses_retry(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            dest = Path(d) / 'out'
            def fault(*args, **_kwargs):
                Path(args[4]).mkdir()
                raise timed_out('native conversion invocation failed')
            with mock.patch.object(incremental, 'prepare', side_effect=fault) as fn:
                with self.assertRaisesRegex(ValueError, 'unexpectedly published'):
                    incremental.prepare_incremental(*a[:4], dest, str(a[4]))
            self.assertEqual(fn.call_count, 1)
            self.assertFalse(dest.exists())

    def test_normal_path_no_extra_conversion_and_no_changes_on_noop(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            dest = Path(d) / 'out'
            real = incremental.prepare
            with mock.patch.object(incremental, 'prepare', wraps=real) as fn:
                first = incremental.prepare_incremental(*a[:4], dest, str(a[4]))
            self.assertEqual(fn.call_count, 1)
            self.assertEqual(first['converted'], 1)
            before = (dest / 'manifest.json').stat().st_mtime_ns
            with mock.patch.object(incremental, 'prepare', side_effect=AssertionError('no need to convert')) as fn:
                second = incremental.prepare_incremental(*a[:4], dest, str(a[4]))
            self.assertEqual(fn.call_count, 0)
            self.assertEqual(second['reused'], 1)
            self.assertEqual((dest / 'manifest.json').stat().st_mtime_ns, before)


if __name__ == '__main__':
    unittest.main(verbosity=2)

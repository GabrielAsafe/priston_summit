#!/usr/bin/env python3
"""Focused parser and safety regressions for PATCH 239."""
import tempfile
import unittest
from pathlib import Path
import validate_fog_stage_239 as diag


class FogStage239Tests(unittest.TestCase):
    def test_valid_frame_record(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'f.tsv'
            path.write_text('FRAME_SLOW\tms=123\tfield=3\tstage_pump_us=49000\tgap_us=80000\n')
            rows = diag.parse_log(path, 'FRAME_')
            self.assertEqual(diag.integer(rows[0], 'stage_pump_us'), 49000)
            self.assertEqual(rows[0]['field'], '3')

    def test_valid_upload_record(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 't.tsv'
            path.write_text('UPLOAD_SLOW\tcommit_us=647843\twait_gpu_us=4000\tlogical=Field/A.TGA\tcooked=1\n')
            rows = diag.parse_log(path, 'UPLOAD_SLOW')
            self.assertEqual(rows[0]['logical'], 'Field/A.TGA')
            self.assertEqual(diag.integer(rows[0], 'commit_us'), 647843)

    def test_ignores_unrelated_entries(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'f.tsv'
            path.write_text('NOISE\tvalue=1\nFRAME_120F\tms=1\n')
            self.assertEqual(len(diag.parse_log(path, 'FRAME_')), 1)

    def test_missing_log_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(FileNotFoundError):
                diag.parse_log(Path(directory) / 'missing.tsv', 'FRAME_')

    def test_invalid_numeric_is_safe(self):
        self.assertEqual(diag.integer({'value': 'unknown'}, 'value'), 0)

    def test_source_gates(self):
        diag.source_gates(diag.ROOT)


if __name__ == '__main__':
    unittest.main(verbosity=2)

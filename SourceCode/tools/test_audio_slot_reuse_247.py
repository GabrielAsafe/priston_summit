#!/usr/bin/env python3
"""PATCH 247 source + C++ regression. No Windows, client or network needed."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
CPP = ROOT / 'SrcGame/src/systems/audio/playback/legacy/Dxwav.cpp'
HEADER = ROOT / 'SrcGame/src/systems/audio/playback/legacy/SummitWavSlotReuse247.h'
SMOKE = ROOT / 'tools/tests/wav_slot_reuse_247_smoke.cpp'

class Audio247(unittest.TestCase):
    def test_must_have_wav_slot_reuse(self):
        s = CPP.read_bytes().decode('latin1')
        self.assertIn('summitWavSlots247.Matches(sbufN, FileName, stamp)', s)
        self.assertIn('SUCCEEDED(g_lpSounds[sbufN]->SetCurrentPosition(0))', s)
        self.assertIn('stampBefore.write_time == stampAfter.write_time', s)
        self.assertIn('summitWavSlots247.Remember(dwBuf, FileName, stampAfter)', s)

    def test_all_buffer_releases_invalidate_reuse(self):
        s = CPP.read_bytes().decode('latin1')
        for header, tail in (
            ('BOOL sBufferClear(int sbufN)', 'summitWavSlots247.Forget(sbufN)'),
            ('BOOL CreateSoundBuffer(', 'summitWavSlots247.Forget(dwBuf)'),
            ('void TermDX_Wav2()', 'summitWavSlots247.Clear()'),
            ('void TermDX_Wav()', 'summitWavSlots247.Clear()'),
            ('BOOL InitDX_Wav2(', 'summitWavSlots247.Clear()'),
            ('BOOL InitDX_Wav(', 'summitWavSlots247.Clear()'),
        ):
            self.assertIn(tail, s[s.index(header):s.index(header)+360])

    def test_no_logging_or_environment_gates(self):
        s = CPP.read_bytes().decode('latin1')
        h = HEADER.read_text()
        self.assertNotIn('getenv(', h)
        self.assertNotIn('fprintf(', h)
        self.assertNotIn('SUMMIT_AUDIO_TRACE', s)
        self.assertNotIn('SUMMIT_AUDIO_DIAG', s)

    def test_portable_cpp_smoke(self):
        compiler = shutil.which('g++')
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as d:
            binary = Path(d)/'wav_247_smoke'
            subprocess.run([compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT), str(SMOKE), '-o', str(binary)], check=True)
            proc = subprocess.run([str(binary)], capture_output=True, text=True, check=True)
            self.assertIn('SUMMIT_AUDIO_247_SMOKE=PASS', proc.stdout)

if __name__ == '__main__':
    unittest.main(verbosity=2)

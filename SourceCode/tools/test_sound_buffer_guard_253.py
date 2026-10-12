#!/usr/bin/env python3
import argparse,pathlib,subprocess,tempfile,shutil,unittest
class Sound253(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.root=pathlib.Path(ARGS.source_code)
 def test_wav_replacement_releases_old_and_failed_new(self):
  s=(self.root/'SrcGame/src/systems/audio/playback/legacy/Dxwav.cpp').read_text(encoding='latin1')
  f=s[s.index('BOOL CreateBufferFromWaveFile('):s.index('BOOL ReadDataWavBuf(')]
  self.assertEqual(f.count('if(!ReadData('),3)
  self.assertEqual(f.count('ReleaseSoundBuffer253(g_lpSounds[dwBuf])'),6)
  self.assertIn('fread(&wavHdr1, sizeof(wavHdr1), 1, pFile) != 1',f)
  self.assertIn('fread(&wavHdr2, sizeof(wavHdr2), 1, pFile) != 1',f)
  self.assertIn('if (!dwSize)',f)
 def test_cpp_guard(self):
  compiler=shutil.which('g++')
  if not compiler:self.skipTest('g++ unavailable')
  with tempfile.TemporaryDirectory() as tmp:
   subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-I',str(self.root/'SrcGame/src'),str(self.root/'tools/tests/sound_buffer_guard_253_smoke.cpp'),'-o',tmp+'/t'],check=True)
   subprocess.run([tmp+'/t'],check=True)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--source-code',default='.');ARGS=p.parse_args();unittest.main(argv=['test_sound_buffer_guard_253.py'],verbosity=2)

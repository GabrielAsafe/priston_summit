#!/usr/bin/env python3
import argparse,pathlib,subprocess,tempfile,shutil,unittest
class AudioGuards250(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.root=pathlib.Path(ARGS.source_code)
 def test_active_audio_uses_bounded_paths(self):
  s=(self.root/'SrcGame/src/systems/audio/playback/legacy/Dxwav.cpp').read_text(encoding='latin1')
  b=s[s.index('int  BackMusic_Load('):s.index('void BackMusicPlay(',s.index('int  BackMusic_Load('))]
  self.assertIn('JoinWavPath250(',b);self.assertNotIn('strcat(',b);self.assertIn('_countof(backMusicWav)',b)
  b=s[s.index('BOOL InitDX_Wav2('):s.index('void TermDX_Wav2(',s.index('BOOL InitDX_Wav2('))]
  self.assertIn('JoinWavPath250(',b);self.assertNotIn('strcat(',b)
 def test_locked_audio_buffer_always_unlocks(self):
  s=(self.root/'SrcGame/src/systems/audio/playback/legacy/Dxwav.cpp').read_text(encoding='latin1')
  b=s[s.index('BOOL ReadData('):s.index('BOOL CreateBufferFromWaveFile(')]
  self.assertEqual(b.count('lpDSB->Unlock('),1)
  self.assertIn('return readOk && SUCCEEDED(rval)',b)
  self.assertIn('g_lpSounds[dwBuf] ? TRUE : FALSE',s)
 def test_cpp_bounded_path(self):
  compiler=shutil.which('g++')
  if not compiler:self.skipTest('g++ unavailable')
  with tempfile.TemporaryDirectory() as tmp:
   subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-I',str(self.root/'SrcGame/src'),str(self.root/'tools/tests/wav_path_250_smoke.cpp'),'-o',tmp+'/t'],check=True)
   subprocess.run([tmp+'/t'],check=True)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--source-code',default='.');ARGS=p.parse_args();unittest.main(argv=['test_audio_guards_250.py'],verbosity=2)

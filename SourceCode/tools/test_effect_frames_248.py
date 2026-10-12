#!/usr/bin/env python3
"""Verify active client uses bounded effect frame paths; smoke test optional."""
import argparse, pathlib, subprocess, tempfile, unittest, shutil

class EffectFrames248(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root=pathlib.Path(ARGS.source_code)
    def test_legacy_client_path_generation_replaced(self):
        text=(self.root/'Shared/rendering/effects/legacy_new/HoEffectModel_legacy_client.inl').read_text(encoding='latin1')
        block=text[text.index('void HoEffectTextureNameProperty::Init('):text.index('HoEffectModelInterface::HoEffectModelInterface()')]
        self.assertIn('BuildEffectFrameNames248(',block)
        self.assertNotIn('strcpy(',block)
        self.assertNotIn('wsprintf(',block)
        self.assertIn('m_iAniIndexCount = 0;',block)
        self.assertIn('m_TextureName.size()',block)
    def test_smoke_cpp(self):
        compiler=shutil.which('g++')
        if not compiler:self.skipTest('g++ not installed')
        with tempfile.TemporaryDirectory() as tmp:
            cmd=[compiler,'-std=c++11','-Wall','-Wextra','-Werror','-I',str(self.root/'Shared'),str(self.root/'tools/tests/effect_frames_248_smoke.cpp'),'-o',tmp+'/smoke']
            subprocess.run(cmd,check=True,capture_output=True)
            subprocess.run([tmp+'/smoke'],check=True,capture_output=True)

if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--source-code',default='.')
    ARGS=ap.parse_args(); unittest.main(argv=['test_effect_frames_248.py'],verbosity=2)

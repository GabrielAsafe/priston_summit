#!/usr/bin/env python3
import argparse, pathlib, subprocess, tempfile, shutil, unittest

class EffectKey249(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.root=pathlib.Path(ARGS.source_code)
 def test_paired_bone_identity_in_active_client(self):
  hdr=(self.root/'Shared/rendering/effects/legacy_new/core/HoEffectResMgr.h').read_text(encoding='latin1')
  impl=(self.root/'Shared/rendering/effects/legacy_new/HoEffectResMgrLegacyCommon.inl').read_text(encoding='latin1')
  self.assertIn('std::map<ResMeshKey, HoEffectResMesh *>',hdr)
  self.assertIn('MakeEffectMeshKey249(meshName, boneName)',impl)
  self.assertIn('m_ResMeshMap.find(key)',impl)
  self.assertIn('it->first.first != meshName',impl)
  self.assertNotIn('m_ResMeshMap[meshName]',impl)
 def test_cpp_identity(self):
  compiler=shutil.which('g++')
  if not compiler:self.skipTest('g++ not found')
  with tempfile.TemporaryDirectory() as tmp:
   subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-I',str(self.root/'Shared'),str(self.root/'tools/tests/effect_mesh_key_249_smoke.cpp'),'-o',tmp+'/t'],check=True)
   subprocess.run([tmp+'/t'],check=True)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--source-code',default='.');ARGS=p.parse_args();unittest.main(argv=['test_effect_mesh_key_249.py'],verbosity=2)

#!/usr/bin/env python3
import argparse,pathlib,shutil,subprocess,tempfile,unittest
class AssaCatalog251(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.root=pathlib.Path(ARGS.source_code)
 def test_active_catalog_has_bounded_paths_and_no_logging(self):
  s=(self.root/'Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl').read_text(encoding='latin1')
  block=s[s.index('void LoadResFile('):s.index('void InitAssaRes(')]
  self.assertIn('JoinCatalogPath251',block)
  self.assertIn('AssaResCount + 1 >= MAX_RES_BUFFER',block)
  self.assertIn('break; // release FindFirstFile handle below',block)
  self.assertIn('strlen(wfd.cFileName) >= MAX_RES_NAME',block)
  self.assertIn('strlen(fileDirectoryName) >= MAX_RES_FULL_NAME',block)
  self.assertNotIn('wsprintf(',block)
  self.assertNotIn('fopen(',block)
  self.assertNotIn('mingw_loadresfile_errors.txt',block)
 def test_find_file_cannot_overflow_output(self):
  s=(self.root/'Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl').read_text(encoding='latin1')
  block=s[s.index('BOOL FindFileRecursive('):s.index('#define MAX_RES_NAME')]
  self.assertNotIn('wsprintf(',block)
  self.assertIn('AppendCatalogPath251',block)
 def test_cpp_bounded_paths(self):
  compiler=shutil.which('g++')
  if not compiler:self.skipTest('g++ unavailable')
  with tempfile.TemporaryDirectory() as tmp:
   subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-I',str(self.root/'Shared'),str(self.root/'tools/tests/assa_catalog_path_251_smoke.cpp'),'-o',tmp+'/t'],check=True)
   subprocess.run([tmp+'/t'],check=True)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--source-code',default='.');ARGS=p.parse_args();unittest.main(argv=['test_assa_catalog_251.py'],verbosity=2)

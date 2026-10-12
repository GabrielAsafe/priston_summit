#!/usr/bin/env python3
import argparse,pathlib,shutil,subprocess,tempfile,unittest
class AssaLookup252Tests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.root=pathlib.Path(ARGS.source_code)
 def test_active_client_index_first_match_and_invalidation(self):
  s=(self.root/'Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl').read_text(encoding='latin1')
  self.assertIn('SummitFindAssaRes252(resName)',s)
  self.assertIn('summitAssaLookup252.Rebuild(AssaRes, AssaResCount)',s)
  self.assertIn('stricmp(AssaRes[found].resName, name)',s)
  self.assertIn('void CloseAssaRes()\n{\n    summitAssaLookup252.Invalidate();',s)
  self.assertIn('void InitAssaRes()\n{\n    summitAssaLookup252.Invalidate();',s)
 def test_smoke(self):
  compiler=shutil.which('g++')
  if not compiler:self.skipTest('g++ unavailable')
  with tempfile.TemporaryDirectory() as tmp:
   subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-I',str(self.root/'Shared'),str(self.root/'tools/tests/assa_lookup_252_smoke.cpp'),'-o',tmp+'/t'],check=True)
   subprocess.run([tmp+'/t'],check=True)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--source-code',default='.');ARGS=p.parse_args();unittest.main(argv=['test_assa_lookup_252.py'],verbosity=2)

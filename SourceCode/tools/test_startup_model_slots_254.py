#!/usr/bin/env python3
# PATCH 254R: portable, actionable C++ regression for the unchanged PATCH 254 loader.
import argparse
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

class SlotTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source=(Path(cls.source_root)/'Shared/gameplay/legacy_support/PlaySubStartupModelLegacyCommon.inl').read_bytes().decode('latin-1')

    def test_both_loaders_have_pre_write_capacity_guards(self):
        for start,end in [('int\tLoadOftenMeshPattern(', '#ifdef _CHECK_MESH'),
                          ('int\tLoadOftenMeshPatternCheckSum(', 'int Send_OftPatSumToDataServer(')]:
            section=self.source[self.source.index(start):self.source.index(end, self.source.index(start))]
            self.assertIn('DynTempPatCnt >= TEMP_DYNPAT_MAX',section)
            self.assertLess(section.index('DynTempPatCnt >= TEMP_DYNPAT_MAX'),section.index('DynTempPat[DynTempPatCnt] ='))
            self.assertIn('!szFileName', section)

    def test_actual_model_loader_with_cpp(self):
        begin=self.source.index('int\tLoadOftenMeshPattern(')
        end=self.source.index('#ifdef _CHECK_MESH',begin)
        function=self.source[begin:end]
        prefix='''#include <cassert>
#include <cstring>
#define FALSE 0
#define TRUE 1
#define TEMP_DYNPAT_MAX 2048
struct smDPAT { int UseCount = 0; };
static smDPAT obj;
static int loads = 0;
struct Factory { smDPAT *LoadCharactor(char *s) { ++loads; return (s && s[0] == 'X') ? nullptr : &obj; } } smPattern;
static smDPAT* DynTempPat[TEMP_DYNPAT_MAX] = {};
static int DynTempPatCnt = 0;
'''
        suffix='''int main() {
 char ok[]="valid", bad[]="Xfail", empty[]="";
 assert(LoadOftenMeshPattern(nullptr) == FALSE && loads == 0);
 assert(LoadOftenMeshPattern(empty) == FALSE && loads == 0);
 DynTempPatCnt=-1;
 assert(LoadOftenMeshPattern(ok) == FALSE && loads == 0);
 DynTempPatCnt=TEMP_DYNPAT_MAX;
 assert(LoadOftenMeshPattern(ok) == FALSE && loads == 0);
 DynTempPatCnt=TEMP_DYNPAT_MAX-1;
 assert(LoadOftenMeshPattern(bad) == FALSE && loads == 1);
 assert(DynTempPatCnt == TEMP_DYNPAT_MAX-1);
 assert(LoadOftenMeshPattern(ok) == TRUE && loads == 2);
 assert(DynTempPatCnt == TEMP_DYNPAT_MAX);
 assert(DynTempPat[TEMP_DYNPAT_MAX-1] == &obj && obj.UseCount == 99);
 assert(LoadOftenMeshPattern(ok) == FALSE && loads == 2);
}
'''
        with tempfile.TemporaryDirectory() as d:
            p=Path(d); src=p/'t.cpp'; exe=p/'t'; src.write_text(prefix+function+suffix)
            # Mandatory portable check: do not make the gameplay regression depend
            # on optional ASan/UBSan libraries installed by a Linux distribution.
            compiler = 'g++'
            common = [compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror', '-O1']

            def run_checked(command, label):
                try:
                    result = subprocess.run(command, capture_output=True, text=True,
                                            timeout=60, check=False)
                except (OSError, subprocess.TimeoutExpired) as exc:
                    self.fail(f'{label}: could not run {command!r}: {exc}')
                if result.returncode:
                    self.fail(f'{label}: exit={result.returncode}\n'
                              f'command={command!r}\n'
                              f'stdout:\n{result.stdout[-8000:]}\n'
                              f'stderr:\n{result.stderr[-8000:]}')

            run_checked(common + [str(src), '-o', str(exe)], 'normal C++ compilation')
            run_checked([str(exe)], 'normal C++ loader regression')

            # Optional extra: probe BOTH compiler support and sanitizer runtime.
            # A failed probe is an environmental limitation, not an asset-loader bug.
            probe = p / 'sanitizer_probe.cpp'
            probe_exe = p / 'sanitizer_probe'
            probe.write_text('int main() { return 0; }\n')
            san_flags = ['-fsanitize=address,undefined']
            try:
                supported = subprocess.run(
                    common + san_flags + [str(probe), '-o', str(probe_exe)],
                    capture_output=True, text=True, timeout=60, check=False)
                if supported.returncode == 0:
                    supported = subprocess.run([str(probe_exe)], capture_output=True,
                                               text=True, timeout=60, check=False)
            except (OSError, subprocess.TimeoutExpired) as exc:
                print(f'SANITIZER=SKIPPED reason={type(exc).__name__}', flush=True)
            else:
                if supported.returncode != 0:
                    print('SANITIZER=SKIPPED reason=compiler_or_runtime_unavailable',
                          flush=True)
                else:
                    sanitized = p / 't_sanitized'
                    run_checked(common + san_flags + [str(src), '-o', str(sanitized)],
                                'sanitized C++ compilation')
                    run_checked([str(sanitized)], 'sanitized C++ loader regression')
                    print('SANITIZER=PASS', flush=True)

if __name__=='__main__':
 parser=argparse.ArgumentParser();parser.add_argument('--source-code',default='.');args=parser.parse_args()
 SlotTests.source_root=args.source_code
 unittest.main(argv=['test_startup_model_slots_254'],verbosity=2)

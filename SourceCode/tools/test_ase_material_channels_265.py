#!/usr/bin/env python3
"""Regression tests for ignored ambient/opacity UV channels in real server ASE code."""
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

SRC = Path(__file__).resolve().parents[1] / 'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'
KEYS = ['UVW_U_OFFSET', 'UVW_V_OFFSET', 'UVW_U_TILING', 'UVW_V_TILING', 'UVW_ANGLE']
sys.path.insert(0, str(Path(__file__).resolve().parent))
from inspect_native_ase_265 import inspect, resolve_source


def actual_uv_blocks(source):
    """Extract *actual* branch bodies, preserving their nested braces."""
    result = []
    for key in KEYS:
        needle = f'if(lstrcmp(decode, "*{key}") == 0)'
        start = source.find(needle, source.find('FILE* ReadASE_LIGHTMAP_MATERIAL'))
        if start < 0:
            raise AssertionError('missing ' + key)
        open_at = source.find('{', start)
        depth = 0
        for pos in range(open_at, len(source)):
            if source[pos] == '{':
                depth += 1
            elif source[pos] == '}':
                depth -= 1
                if depth == 0:
                    result.append(source[start:pos+1])
                    break
        else:
            raise AssertionError('unclosed ' + key)
    return '\n'.join(result)


class Tests(unittest.TestCase):
    def test_real_uv_branches_compiled_and_executed(self):
        source = SRC.read_text(encoding='latin1')
        branches = actual_uv_blocks(source)
        code = r'''#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#define FALSE nullptr
struct Material {
    float UVW_U_OFFSET[8] = {};
    float UVW_V_OFFSET[8] = {};
    float UVW_U_TILING[8] = {};
    float UVW_V_TILING[8] = {};
    float UVW_ANGLE[8] = {};
};
Material aseMaterial[2];
int lstrcmp(const char *a, const char *b) { return std::strcmp(a,b); }
char* GetWord(char* q, char* p) {
    while (*p == ' ' || *p == ':') ++p;
    size_t n = 0;
    while (*p && *p != '\n' && *p != ' ') { if(n < 255) q[n++] = *p; ++p; }
    q[n] = 0;
    return p;
}
FILE* runUV(const char* decode, int BitmapDiffuse, int BitmapCnt) {
    char strBuff[256] = {};
    char input[] = " 0.625";
    char *p = input;
    const int curMatrialNum = 0;
    FILE *fp = std::tmpfile();
    if (!fp) std::abort();
    __BRANCHES__
    std::fclose(fp);
    return reinterpret_cast<FILE*>(1);
}
int main() {
    for (const char* key : {"*UVW_U_OFFSET", "*UVW_V_OFFSET", "*UVW_U_TILING", "*UVW_V_TILING", "*UVW_ANGLE"}) {
        if (!runUV(key, 0, -1)) return 1;    // opacity without diffuse: skip safely
        if (!runUV(key, -1, -1)) return 2;   // ambient without diffuse: skip safely
        if (!runUV(key, 0, 0)) return 3;     // opacity must not alter prior diffuse slot
        if (!runUV(key, 1, 0)) return 4;     // diffuse channel written
        if (runUV(key, 1, 8)) return 5;      // invalid index remains fail closed
    }
    if (std::fabs(aseMaterial[0].UVW_U_OFFSET[0] - .625f) > .0001f) return 6;
    if (std::fabs(aseMaterial[0].UVW_ANGLE[0] - .625f) > .0001f) return 7;
    return 0;
}
'''.replace('__BRANCHES__', branches).replace('for (const char* key : {', 'for (const char* key : std::initializer_list<const char*>{').replace('#include <cmath>', '#include <cmath>\n#include <initializer_list>')
        with tempfile.TemporaryDirectory() as td:
            cpp = Path(td)/'material.cpp'
            exe = Path(td)/'material'
            cpp.write_text(code)
            proc = subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', str(cpp), '-o', str(exe)], capture_output=True, text=True)
            self.assertEqual(proc.returncode, 0, proc.stderr)
            proc = subprocess.run([str(exe)], capture_output=True, text=True)
            self.assertEqual(proc.returncode, 0, proc.stdout+'\n'+proc.stderr)

    def test_inspector_structural_report(self):
        data = b'''*MATERIAL_LIST {\n *MATERIAL_COUNT 1\n *MATERIAL 0 {\n *MAP_AMBIENT {\n *UVW_U_OFFSET 0.1\n }\n *MAP_DIFFUSE {\n *BITMAP "a.bmp"\n *UVW_V_OFFSET 0.2\n }\n }\n}\n*GEOMOBJECT {\n *NODE_NAME "test"\n *MESH_NUMFACES 1\n *MESH_FACE 0: A: 0 B: 1 C: 2\n *MATERIAL_REF 0\n}\n'''
        r = inspect(data)
        self.assertEqual((r['geometry'],r['actual_faces'],r['material_count']), (1,1,1))
        self.assertEqual(r['ignored_uv_fields'], 1)
        self.assertEqual(r['issues'], [])
        self.assertEqual(r['nodes'], ['test'])

    def test_inspector_missing_materials(self):
        r = inspect(b'*GEOMOBJECT {\n *MESH_NUMFACES 1\n *MESH_FACE 0: A: 0 B: 1 C: 2\n}')
        self.assertIn('faces_without_material_list', r['issues'])
        self.assertIn('faces_without_material_ref', r['issues'])

    def test_reject_symlinks_and_traversal(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)/'client'; root.mkdir()
            asset = root/'123.ASE'; asset.write_bytes(b'*GEOMOBJECT {}')
            self.assertEqual(resolve_source(root,'123.ASE'), asset)
            for invalid in ('../123.ASE', 'foo\\123.ASE', '/tmp/a.ASE', 'x//123.ASE', 'x/./123.ASE'):
                with self.assertRaises(ValueError): resolve_source(root, invalid)
            (root/'symlink.ASE').symlink_to(asset)
            with self.assertRaises(ValueError): resolve_source(root, 'symlink.ASE')

if __name__ == '__main__':
    unittest.main(verbosity=2)

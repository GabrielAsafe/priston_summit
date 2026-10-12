#!/usr/bin/env python3
"""PATCH 266: executable UV policy + exact server parser branch regression."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT/'Shared/rendering/legacy_3d/ase/ModelUvPolicy266.h'
READER = ROOT/'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'
SERIALIZER = ROOT/'Shared/rendering/legacy_3d/object/LegacyObjectSerialization.h'
import inspect_ase_geometry_uv_266 as inspector


class Tests(unittest.TestCase):
    def test_native_uv_policy_and_real_guard_cpp(self):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler, 'C++ compiler required')
        source = READER.read_text(encoding='latin1')
        geom = source.split('FILE *ReadASE_GEOMOBJECT(',1)[1].split('void  smASE_SetPhysique',1)[0]
        begin = geom.index('TexCnt = smMatPattern[mNum].TextureCounter;')
        end = geom.index('if(bLightMap)\n\t\t\tTexCnt--;',begin)
        actual = geom[begin:end]
        code = '''#include <cassert>
#include <cstddef>
#include "ase/ModelUvPolicy266.h"
struct Material { int TextureCounter; };
Material materials[1]{{1}};
Material *smMatPattern = materials;
int nTVertex=0, TVertexCnt=0, TFaceCnt=0, nFace=1, bLightMap=0;
void *tPoint=nullptr, *tFace=nullptr;
int mNum=0;
int simulate() {
    int TexCnt=0;
    auto failImportWith = [](const char*) {return -100;};
''' + actual + '''\nreturn TexCnt;
}
int main() {
    assert(simulate()==0); // absent UV: keep material, no AddTexLink
    nTVertex=-1; assert(simulate()==0);
    nTVertex=4; TVertexCnt=3; TFaceCnt=1;
    tPoint=(void*)1; tFace=(void*)1;
    assert(simulate()==-100); // incomplete UV rejected
    TVertexCnt=4; assert(simulate()==1); // complete UV mapped
    TFaceCnt=0; assert(simulate()==-100);
    nTVertex=0; TVertexCnt=0; TFaceCnt=1; assert(simulate()==-100);
    nTVertex=4; TVertexCnt=4; TFaceCnt=1;
    using namespace summit_ase_uv266;
    assert(FaceIndicesValid(0,1,3,4));
    assert(!FaceIndicesValid(0,1,4,4));
    assert(!FaceIndicesValid(0,1,static_cast<unsigned>(-1),4));
    assert(!FaceIndicesValid(0,1,3,0));
    bLightMap=1; tPoint=nullptr; assert(simulate()==-100); // no change to lightmap safety
}
'''
        with tempfile.TemporaryDirectory() as td:
            cpp=Path(td)/'test.cpp'; exe=Path(td)/'test';cpp.write_text(code)
            c=subprocess.run([compiler,'-std=c++17','-Wall','-Wextra','-Werror',
                              '-I',str(HEADER.parent.parent),str(cpp),'-o',str(exe)],capture_output=True,text=True)
            self.assertEqual(c.returncode,0,c.stderr)
            r=subprocess.run([str(exe)],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stdout+'\n'+r.stderr)

    def test_face_material_not_lost_and_serialization_allows_null_link(self):
        source = READER.read_text(encoding='latin1')
        geom = source.split('FILE *ReadASE_GEOMOBJECT(',1)[1].split('void  smASE_SetPhysique',1)[0]
        self.assertIn('TexCnt = 0; // no AddTexLink; SetFaceMaterial still runs',geom)
        self.assertIn('obj->SetFaceMaterial(cnt, mNum);',geom)
        self.assertIn('geometry_partial_uv',geom)
        self.assertIn('geometry_uv_face_index',geom)
        self.assertIn('geometry_face_vertex',geom)
        self.assertIn('geometry_incomplete_mesh',geom)
        disk = SERIALIZER.read_text()
        self.assertRegex(disk,r'if\(!pointer\)\s*\{\s*index = -1;\s*return true;')
        self.assertIn('if(index < 0)',disk)
        self.assertIn('token = 0;',disk)

    @staticmethod
    def sample_geometry(name, faces, n_tverts=None, actual_tverts=0, actual_tfaces=0):
        data=(f'*GEOMOBJECT {{\n*NODE_NAME "{name}"\n*MESH_NUMFACES {faces}\n'
              +''.join('*MESH_FACE 0: A:0 B:1 C:2\n' for _ in range(faces)))
        if n_tverts is not None:
            data += f'*MESH_NUMTVERTEX {n_tverts}\n'
        data += '*MESH_TVERT 0 0 0 0\n'*actual_tverts
        data += '*MESH_TFACE 0 0 1 2\n'*actual_tfaces
        return data+'}\n'

    def test_node_by_node_geometry_uv_inventory(self):
        data=(self.sample_geometry('BarePiece',2)
              +self.sample_geometry('TexturedPiece',1,3,3,1)
              +self.sample_geometry('IncompletePiece',1,3,2,1))
        rows=inspector.inspect_geometry(data.encode())
        self.assertEqual([r['uv'] for r in rows],['ABSENT','MAPPED','PARTIAL'])
        self.assertEqual([r['name'] for r in rows],['BarePiece','TexturedPiece','IncompletePiece'])
        self.assertEqual(rows[0]['actual_faces'],2)
        self.assertEqual(rows[1]['actual_tfaces'],1)

    def test_inspector_mapping_channels_and_unbalanced_mesh(self):
        data=b'''*GEOMOBJECT {
*NODE_NAME "a"
*MESH_NUMFACES 1
*MESH_FACE 0: A:0 B:1 C:2
*MESH_NUMTVERTEX 3
*MESH_TVERT 0 0 0 0
*MESH_TVERT 1 0 0 0
*MESH_TVERT 2 0 0 0
*MESH_TFACE 0 0 1 2
*MESH_MAPPINGCHANNEL 2 {
*MESH_NUMTVERTEX 4
*MESH_TVERT 0 0 0 0
}
}
*GEOMOBJECT {
*NODE_NAME "b"
*MESH_NUMFACES 2
*MESH_FACE 0: A:0 B:1 C:2
}
'''
        rows=inspector.inspect_geometry(data)
        self.assertEqual(rows[0]['uv'],'MAPPED')
        self.assertEqual(rows[0]['mapping_channels'],1)
        self.assertEqual(rows[0]['actual_tverts'],3)
        self.assertEqual(rows[1]['uv'],'INVALID_GEOMETRY')

    def test_read_only_inspector_rejects_traversal_and_symlinks(self):
        from inspect_native_ase_265 import resolve_source
        with tempfile.TemporaryDirectory() as td:
            client=Path(td)/'client';client.mkdir()
            a=client/'a.ASE';a.write_bytes(self.sample_geometry('ok',1).encode())
            self.assertEqual(resolve_source(client,'a.ASE'),a)
            (client/'b.ASE').symlink_to(a)
            for name in ('../a.ASE','b.ASE'):
                with self.assertRaises(ValueError): resolve_source(client,name)
            self.assertEqual(inspector.inspect_geometry(a.read_bytes())[0]['uv'],'ABSENT')

if __name__=='__main__':
    unittest.main(verbosity=2)

#!/usr/bin/env python3
"""PATCH 268: run the actual bone policy C++ and verify parser call-site separation."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
READER = ROOT / 'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'
POLICY = ROOT / 'Shared/rendering/legacy_3d/ase/BoneMaterialPolicy268.h'


class BoneAseTests268(unittest.TestCase):
    def test_compiled_real_bone_material_policy(self):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler, 'g++ or clang++ required')
        self.assertTrue(POLICY.is_file())
        test = r'''
#include <cassert>
#include "ase/BoneMaterialPolicy268.h"
struct Skeleton {
    int nFace;
    int mats[4];
    int assignments;
    bool allow;
    explicit Skeleton(int count) : nFace(count), mats{17,17,17,17}, assignments(0), allow(true) {}
    bool SetFaceMaterial(int face, int material) {
        if (!allow || face < 0 || face >= nFace || material != 0) return false;
        mats[face] = material; ++assignments; return true;
    }
};
int main() {
    Skeleton noMesh(0);
    assert(summit_ase_bone268::AssignNeutralFaces(&noMesh, -1));
    assert(noMesh.assignments == 0);
    assert(summit_ase_bone268::AssignNeutralFaces(&noMesh, 0));
    assert(noMesh.assignments == 0);
    Skeleton mesh(2);
    assert(summit_ase_bone268::AssignNeutralFaces(&mesh, 2));
    assert(mesh.assignments == 2);
    assert(mesh.mats[0] == 0 && mesh.mats[1] == 0);
    assert(!summit_ase_bone268::AssignNeutralFaces(&mesh, 3));
    assert(!summit_ase_bone268::AssignNeutralFaces(&mesh, -2));
    assert(!summit_ase_bone268::AssignNeutralFaces<Skeleton>(nullptr, 0));
    mesh.allow = false;
    assert(!summit_ase_bone268::AssignNeutralFaces(&mesh, 2));
}
'''
        with tempfile.TemporaryDirectory() as td:
            code = Path(td) / 'bone.cpp'; exe = Path(td) / 'bone'
            code.write_text(test)
            cc = subprocess.run([compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
                                 '-I', str(POLICY.parent.parent), str(code), '-o', str(exe)],
                                capture_output=True, text=True)
            self.assertEqual(cc.returncode, 0, cc.stdout + cc.stderr)
            run = subprocess.run([str(exe)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)

    def test_compiled_real_bone_reader_branch(self):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        body = READER.read_text(encoding='latin1')
        start = body.index('if (boneOnly && nFace == -1)', body.index('// PATCH 268: skeleton ASE'))
        stop = body.index('} else {', start)
        actual = body[start:stop] + '}\n'
        code = r"""
#include <cassert>
#include <cstring>
#include "ase/BoneMaterialPolicy268.h"
struct Object {
    int nFace;
    int materials[3];
    bool accept;
    int initialized;
    explicit Object(int count) : nFace(count), materials{42,42,42}, accept(true), initialized(0) {}
    void Init(int vertices, int faces, int flags) {
        assert(vertices==0 && faces==0 && flags==0);
        ++initialized; nFace=0;
    }
    bool SetFaceMaterial(int f, int m) {
        if (!accept || f < 0 || f >= nFace || m != 0) return false;
        materials[f] = m; return true;
    }
};
Object* obj = nullptr;
int nFace = -1;
bool boneOnly = false;
const char* failure = nullptr;
int exercise() {
    auto failImportWith = [](const char* reason) { failure = reason; return -3; };
""" + actual + r"""
return 0;
}
int main() {
    Object skeleton(0); obj=&skeleton; boneOnly=true;
    nFace=-1; assert(exercise()==0 && skeleton.initialized==1);
    nFace=0; assert(exercise()==0 && skeleton.initialized==1);
    Object faces(2); obj=&faces; nFace=2;
    assert(exercise()==0 && faces.materials[0]==0 && faces.materials[1]==0);
    nFace=3; assert(exercise()==-3);
    assert(std::strcmp(failure, "bone_geometry_faces")==0);
    failure=nullptr; boneOnly=false; nFace=3;
    assert(exercise()==0 && failure==nullptr);
}
"""
        with tempfile.TemporaryDirectory() as td:
            src=Path(td)/'branch.cpp';exe=Path(td)/'branch';src.write_text(code)
            cc=subprocess.run([compiler,'-std=c++17','-Wall','-Wextra','-Werror',
                               '-I',str(POLICY.parent.parent),str(src),'-o',str(exe)],
                              capture_output=True,text=True)
            self.assertEqual(cc.returncode,0,cc.stdout+cc.stderr)
            run=subprocess.run([str(exe)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)

    def test_only_skeleton_readers_use_bone_mode(self):
        text = READER.read_text(encoding='latin1')
        self.assertIn('#include "ase/BoneMaterialPolicy268.h"', text)
        self.assertIn('bool boneOnly = false', text)
        mesh = text.split('smPAT3D *smASE_Read(char *file', 1)[1].split('smPAT3D *smASE_ReadBone(char *file)', 1)[0]
        bone = text.split('smPAT3D *smASE_ReadBone(char *file)', 1)[1].split('smPAT3D* smASE_TalkReadBone(char *file)', 1)[0]
        talk = text.split('smPAT3D* smASE_TalkReadBone(char *file)', 1)[1].split('smPAT3D *smASE_Read', 1)[0] if text.count('smPAT3D *smASE_Read', 1) else text.split('smPAT3D* smASE_TalkReadBone(char *file)', 1)[1]
        self.assertIn('ReadASE_GEOMOBJECT(fp, obj);', mesh)
        self.assertNotIn('ReadASE_GEOMOBJECT(fp, obj, 0, true);', mesh)
        self.assertIn('ReadASE_GEOMOBJECT(fp, obj, 0, true);', bone)
        self.assertIn('ReadASE_GEOMOBJECT(fp, obj, 0, true);', talk)
        self.assertEqual(text.count('ReadASE_GEOMOBJECT(fp, obj, 0, true);'), 2)

    def test_material_guards_remain_mandatory_for_mesh(self):
        text = READER.read_text(encoding='latin1')
        section = text.split('// PATCH 268: skeleton ASE', 1)[1].split('if(FaceMatrial) delete[] FaceMatrial;', 1)[0]
        self.assertIn('if (boneOnly)', section)
        self.assertIn('obj->Init(0, 0, 0);', section)
        self.assertIn('AssignNeutralFaces(obj, nFace)', section)
        self.assertIn('} else {', section)
        self.assertIn('MatrialRef >= aseMaterialCnt', section)
        self.assertIn('geometry_material_reference', section)
        self.assertIn('geometry_face_material', section)
        self.assertIn('smMatPattern[mNum].TextureCounter', section)
        self.assertIn('geometry_partial_uv', section)
        self.assertIn('geometry_uv_face_index', section)
        self.assertIn('obj->SetFaceMaterial(cnt, mNum);', section)
        self.assertIn('bone_geometry_faces', section)

    def test_no_changes_to_client_parser_or_content(self):
        text = READER.read_text(encoding='latin1')
        self.assertTrue(text.count('geometry_incomplete_mesh') == 1)
        self.assertIn('if ((nVertex < 0) != (nFace < 0)', text)


if __name__ == '__main__':
    unittest.main(verbosity=2)

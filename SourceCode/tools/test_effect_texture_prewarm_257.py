#!/usr/bin/env python3
"""Exercise the actual HoEffect material queue and frame-property methods."""
from pathlib import Path
import importlib.util
import shutil
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]

def previous():
    p=ROOT/'tools/test_effect_mesh_stream_255.py'
    spec=importlib.util.spec_from_file_location('test255',p)
    mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
    return mod

class TexturePrewarm257(unittest.TestCase):
    def test_actual_effect_manager_and_animation_cpp(self):
        if not shutil.which('g++'):
            self.skipTest('g++ not available')
        m=previous()
        stub=m.STUB
        model=(ROOT/'Shared/rendering/effects/legacy_new/HoEffectModel_legacy_client.inl').read_text()
        prop=model[model.index('void HoEffectTextureNameProperty::Init'):model.index('HoEffectModelInterface::HoEffectModelInterface')]
        klass=r'''
class HoEffectTextureNameProperty {
    int m_iAniIndex=0,m_iAniIndexCount=0;
    float m_fAniDelayTime=0.5f,m_fAniTimer=0.f;
    std::vector<std::string> m_TextureName;
public:
    void Init(const std::string&);
    void Init(const std::string&,int,float);
    bool Main(float);
    int GetMatNum();
    void Clear();
};
'''
        cpp=m.CPP
        cpp=cpp.replace('#include <iostream>','#include <iostream>\n#include <vector>\n#include "rendering/effects/legacy_new/SummitEffectFrames248.h"')
        cpp=cpp.replace('static std::atomic<int> reads{0}, bones{0}, published{0}, destroyed{0};',
                        'static std::atomic<int> reads{0}, bones{0}, published{0}, destroyed{0}, textures{0};')
        cpp=cpp.replace('int CreateTextureMaterial(char*,int,int,int,int,int) { return 1; }',
                        'int CreateTextureMaterial(char*,int,int,int,int,int) { return ++textures; }')
        cpp=cpp.replace('int main() {',klass+prop+'\nint main() {',1)
        position='    auto& pool=summit::core::concurrency::JobSystem::Instance();'
        additional=r'''
    auto* tm=HoEffectResManager::GetInstance();
    HoEffectTextureNameProperty property;
    property.Init("image/fx/frame.tga", 5, .05f);
    assert(textures==0); // Init queues; no resource material is allocated.
    tm->PumpResTexturePrewarm(1);
    assert(textures==1);
    tm->PumpResTexturePrewarm(1);
    assert(textures==2);
    tm->PumpResTexturePrewarm(4);
    assert(textures==2); // no duplicate and no global sweep.
    assert(property.GetMatNum()>=0 && textures==2);
    property.Main(.1f); property.Main(.1f);
    tm->PumpResTexturePrewarm(2);
    assert(textures>=3 && textures<=4);
    const int warmed=textures;
    property.Init("image/fx/frame.tga",5,.05f);
    tm->PumpResTexturePrewarm(4);
    assert(textures==warmed); // existing cache identities reused.
    property.Clear();
    tm->RequestResTexturePrewarm("queued/not-released.tga");
    tm->Clear();
    tm->PumpResTexturePrewarm(4);
    assert(textures==warmed); // Clear invalidates queued material work.
    tm->RequestResTexturePrewarm("again.tga");
    tm->PumpResTexturePrewarm(4);
    assert(textures==warmed+1);
    tm->Clear();
    published=0; // isolate the legacy mesh publication assertions.
'''
        cpp=cpp.replace(position,additional+position,1)
        with tempfile.TemporaryDirectory() as td:
            root=Path(td)
            header=root/'systems/rendering/effects/legacy_new/core/HoEffectResMgr.h'
            header.parent.mkdir(parents=True);header.write_text(stub)
            source=root/'test.cpp';source.write_text(cpp)
            cmd=['g++','-std=c++17','-pthread','-Wall','-Wextra','-Werror','-O1',
                 '-I',str(root),'-I',str(ROOT/'SrcGame/src'),'-I',str(ROOT/'Shared'),
                 str(source),str(ROOT/'SrcGame/src/core/concurrency/JobSystem.cpp'),
                 str(ROOT/'SrcGame/src/core/concurrency/SourceAssetLock.cpp'),'-o',str(root/'test')]
            proc=subprocess.run(cmd,capture_output=True,text=True)
            self.assertEqual(proc.returncode,0,'compile error:\n'+proc.stderr)
            execd=subprocess.run([str(root/'test')],capture_output=True,text=True)
            self.assertEqual(execd.returncode,0,'C++ runtime:\n'+execd.stdout+'\n'+execd.stderr)
            self.assertIn('SUMMIT_EFFECT_MESH_STREAM_255_CPP=PASS',execd.stdout)

    def test_frame_pumps_owner_thread_and_bounded(self):
        w=(ROOT/'SrcGame/src/platform/windows/application/Winmain.cpp').read_text(errors='replace')
        o=(ROOT/'SrcGame/src/systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp').read_text(errors='replace')
        self.assertEqual(w.count('PumpResTexturePrewarm(2u)'),2)
        self.assertEqual(o.count('PumpResTexturePrewarm(2u)'),1)
        mgr=(ROOT/'Shared/rendering/effects/legacy_new/HoEffectResMgrLegacyCommon.inl').read_text()
        self.assertIn('if (maxLoads > 4u) maxLoads = 4u;',mgr)
        self.assertIn('m_PrewarmTextureQueue257.clear();',mgr)
        self.assertNotIn('getenv(',mgr[mgr.index('void HoEffectResManager::RequestResTexturePrewarm'):mgr.index('int HoEffectResManager::GetResTexture')])

if __name__=='__main__':unittest.main(verbosity=2)

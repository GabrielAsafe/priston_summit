#!/usr/bin/env python3
"""Compile real ambient/continuous sound paths with synthetic DirectSound calls."""
import shutil,subprocess,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'SrcGame/src/systems/audio/effects/legacy/effectsnd.cpp'

def between(txt,a,b):
    assert a in txt and b in txt
    return txt[txt.index(a):txt.index(b,txt.index(a))]

class SoundBounds258(unittest.TestCase):
    def test_real_legacy_functions_cpp(self):
        if not shutil.which('g++'):self.skipTest('g++ unavailable')
        src=SOURCE.read_text(errors='replace')
        ambient=between(src,'char *szAmbientSound[]', 'struct\tsSKILL_SOUND_FILE')
        effect=between(src,'char *szAmbientSound2[]', 'int\tSkillPlaySound')
        music=between(src,'int\tesPlayEffectBgm(int Num)', 'static int\tFootStepCount')
        code=r'''
#include <cassert>
#include <iostream>
#include <climits>
#include "systems/audio/effects/legacy/SummitSoundTable258.h"
#define TRUE 1
#define FALSE 0
#define FLOATNS 8
#define fONE 256
struct Player { int pX=0,pY=0,pZ=0; } player;
Player* lpCurPlayer=&player;
static int loads=0, plays=0;
static char* requested=nullptr;
int CreateBufferFromWaveFile2(char* file,int) { assert(file); requested=file; ++loads; return TRUE; }
int PlayWav2(int,int,int,int) { ++plays;return TRUE; }
int PlayWav_VolChange(int,int) { return TRUE; }
int StopWav(int) { return TRUE; }
int IsPlayWav(int) { return FALSE; }
int AmbientSound=-1,AmbientSound2=-1,AmbientSoundNum=110,AmbientSoundNum2=111,ContinueSoundNum=112,ContSoundCount=0;
'''+ambient+'\n'+effect+'\n'+music+r'''
int main() {
    using summit::audio::SoundTablePath258;
    assert(SoundTablePath258(szAmbientSound,-1)==nullptr);
    assert(SoundTablePath258(szAmbientSound,INT_MAX)==nullptr);
    assert(SoundTablePath258(szSodSound,INT_MAX)==nullptr);
    assert(SoundTablePath258(szAmbientSound2,0)!=nullptr);
    assert(SoundTablePath258(szAmbientSound2,static_cast<int>(sizeof(szAmbientSound2)/sizeof(*szAmbientSound2))-2)!=nullptr);
    assert(SoundTablePath258(szAmbientSound2,static_cast<int>(sizeof(szAmbientSound2)/sizeof(*szAmbientSound2))-1)==nullptr);
    // No invalid indexes can provoke a DirectSound call.
    assert(esPlayEffectBgm(INT_MAX)==FALSE && loads==0);
    assert(esPlayObjectSound2(INT_MAX,0,0,0,0)==FALSE && loads==0);
    assert(esPlayContSound(INT_MAX)==FALSE);
    assert(esPlayContSound(-1)==FALSE);
    assert(ContSoundPush==0);
    assert(esPlayEffectBgm(1)==TRUE && loads==1);
    // Positive ambient index with positive volume exercises real loader.
    assert(esPlayObjectSound2(1,0,0,0,0)==TRUE && loads==2);
    assert(esPlayContSound(1)==TRUE);
    ContSoundCount=7;
    assert(esContSound()==TRUE && loads==3);
    assert(ContSoundPop==1);
    // Corrupted queue entry gets discarded rather than blocking subsequent sounds.
    ContSoundBuff[ContSoundPush&CONT_SOUND_MASK]=INT_MAX;
    ++ContSoundPush;
    ContSoundCount=15;
    assert(esContSound()==FALSE && loads==3);
    assert(ContSoundPop==2 && ContSoundNum==-1);
    assert(esPlayContSound(2)==TRUE);
    ContSoundCount=23;
    assert(esContSound()==TRUE && loads==4);
    std::cout<<"SUMMIT_SOUND_TABLE_258_CPP=PASS\n";
}
'''
        with tempfile.TemporaryDirectory() as d:
            d=Path(d);cpp=d/'test.cpp';cpp.write_text(code)
            exe=d/'test'
            run=subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-Wno-write-strings','-O1',
                                '-I',str(ROOT/'SrcGame/src'),str(cpp),'-o',str(exe)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,'g++:\n'+run.stderr)
            run=subprocess.run([str(exe)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,'test:\n'+run.stdout+'\n'+run.stderr)
            self.assertIn('SUMMIT_SOUND_TABLE_258_CPP=PASS',run.stdout)

    def test_all_unsafe_lookups_replaced(self):
        t=SOURCE.read_text(errors='replace')
        self.assertNotIn('szAmbientSound[Num]',t)
        self.assertNotIn('szAmbientSound2[AmbNum]',t)
        self.assertNotIn('szSodSound[ContSoundNum]',t)
        self.assertIn('if (!summit::audio::SoundTablePath258(szSodSound, SoundNum))',t)
        self.assertIn('ContSoundPop++;\n\t\t\treturn FALSE;',t)

if __name__=='__main__':unittest.main(verbosity=2)

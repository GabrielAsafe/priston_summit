#include <cassert>
#include <cstring>
#include "systems/audio/playback/legacy/SummitWavPath250.h"
int main(){
 char out[12]={};
 using summit::audio::JoinWavPath250;
 assert(JoinWavPath250("wav/",5,"x.wav",6,out,sizeof(out)));
 assert(std::strcmp(out,"wav/x.wav")==0);
 assert(!JoinWavPath250("wav/",5,"verylongname",13,out,sizeof(out)));
 const char noTerm[3]={'x','y','z'};
 assert(!JoinWavPath250(noTerm,sizeof(noTerm),"a",2,out,sizeof(out)));
 assert(!JoinWavPath250("",1,"",1,out,sizeof(out)));
 assert(!JoinWavPath250("wav/",5,"x.wav",6,out,9));
 assert(JoinWavPath250("",1,"hello",6,out,sizeof(out)));
}

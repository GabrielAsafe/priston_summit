#pragma once
// SUMMIT463A quality policy - server-only native fields; C++11
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <random>
#include <limits>
namespace summit463_quality {
struct Chance {int level,normal,superior,raro,epico;};
static const Chance kTable[]={
  {0,100,0,0,0},
  {10,90,10,0,0},
  {25,70,25,5,0},
  {40,45,35,17,3},
  {50,30,40,25,5},
};
static const int kMin[]={0,2,6,11};
static const int kMax[]={0,5,10,15};
inline int RollTier(int level,std::mt19937_64& rng){
    const Chance* c=&kTable[0];for(const Chance& x:kTable)if(level>=x.level)c=&x;
    std::uniform_int_distribution<int> d(0,99);int r=d(rng);
    if((r-=c->normal)<0)return 0;
    if((r-=c->superior)<0)return 1;
    if((r-=c->raro)<0)return 2;
    return 3;
}
inline int Gain(int value,int percent){
    if(value<=0 || value>32000 || percent<0 || percent>15)return value;
    const std::int64_t extra=(static_cast<std::int64_t>(value)*percent+99)/100;
    if(extra>32767-value)return value;
    return value+static_cast<int>(extra);
}
inline bool IsWeapon(const char* code){return code && code[0]=='W';}
inline bool IsDefence(const char* code){return code && code[0]=='D';}
template<class Item> inline bool Apply(Item& item,const char* code,int level,std::mt19937_64& rng,int& tier){
    tier=0;
    if(!(IsWeapon(code)||IsDefence(code)))return true;
    tier=RollTier(level,rng);if(tier==0)return true;
    std::uniform_int_distribution<int> d(kMin[tier],kMax[tier]);int p=d(rng);
    if(IsWeapon(code)){item.Damage[0]=Gain(item.Damage[0],p);item.Damage[1]=Gain(item.Damage[1],p);}
    else item.Defence=Gain(item.Defence,p);
    static const char* suffix[]={"", " [Superior]", " [Raro]", " [Epico]"};
    const std::size_t cap=sizeof(item.ItemName);
    std::size_t n=0;while(n<cap && item.ItemName[n])++n;
    const std::size_t sl=std::strlen(suffix[tier]);
    if(n>=cap || sl+1>cap-n)return false; // refuse before inventory debit
    std::memcpy(item.ItemName+n,suffix[tier],sl+1);return true;
}
} // namespace summit463_quality

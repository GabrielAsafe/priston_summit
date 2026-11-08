#pragma once
// SUMMIT466A: source of truth for runtime font sizing. C++11 / MinGW i686.
// font_settings_466.ini sits next to client EXE. Restart client after editing.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <string>
#include <algorithm>
#include <cctype>
#ifdef _WIN32
# include <windows.h>
#endif
namespace summit466 {

class FontSettings466 {
public:
    enum Category { Chat=0, Interface=1, Damage=2, Events=3, World=4, Atlas=5, CategoryCount=6 };
    struct Options {
        int global;
        int group[CategoryCount];
        int minimumPx;
        int maximumPx;
        Options():global(125),minimumPx(9),maximumPx(48) {
            for(int i=0;i<CategoryCount;++i)group[i]=100;
            group[Chat]=110;
            group[Damage]=105;
        }
    };
    static Options Parse(const std::string& path) {
        Options result;
        std::ifstream in(path.c_str());
        if(!in)return result;
        std::string line,section;
        while(std::getline(in,line)) {
            const std::size_t comment=line.find_first_of(";#");
            if(comment!=std::string::npos)line.erase(comment);
            trim(line);
            if(line.empty())continue;
            if(line[0]=='[' && line[line.size()-1]==']') {
                section=line.substr(1,line.size()-2);trim(section);lower(section);continue;
            }
            const std::size_t eq=line.find('=');
            if(eq==std::string::npos || section!="fonts")continue;
            std::string key=line.substr(0,eq),value=line.substr(eq+1);
            trim(key);trim(value);lower(key);
            if(value.empty())continue;
            char* end=0;
            const long n=std::strtol(value.c_str(),&end,10);
            if(end==value.c_str() || *end!='\0')continue;
            if(key=="globalscalepercent")result.global=bounded(n,75,200,result.global);
            else if(key=="chatscalepercent")result.group[Chat]=bounded(n,75,200,result.group[Chat]);
            else if(key=="interfacescalepercent")result.group[Interface]=bounded(n,75,200,result.group[Interface]);
            else if(key=="damagescalepercent")result.group[Damage]=bounded(n,75,200,result.group[Damage]);
            else if(key=="eventsscalepercent")result.group[Events]=bounded(n,75,200,result.group[Events]);
            else if(key=="worldscalepercent")result.group[World]=bounded(n,75,200,result.group[World]);
            else if(key=="atlasscalepercent")result.group[Atlas]=bounded(n,75,200,result.group[Atlas]);
            else if(key=="minimumpixels")result.minimumPx=bounded(n,6,24,result.minimumPx);
            else if(key=="maximumpixels")result.maximumPx=bounded(n,24,96,result.maximumPx);
        }
        if(result.minimumPx>result.maximumPx)result.minimumPx=result.maximumPx;
        return result;
    }
    static std::string IniPath() {
#ifdef _WIN32
        char path[4096]={0};
        const DWORD n=GetModuleFileNameA(0,path,static_cast<DWORD>(sizeof(path)));
        if(n && n<sizeof(path)) {
            const std::string exe(path,n);
            const std::string::size_type k=exe.find_last_of("\\/");
            if(k!=std::string::npos)return exe.substr(0,k+1)+"font_settings_466.ini";
        }
#endif
        return "font_settings_466.ini";
    }
    static const Options& Config() {
        static const Options options=Parse(IniPath());
        return options;
    }
    static int Height(int original,Category category) {
        return HeightWith(original,category,Config());
    }
    static int HeightWith(int original,Category category,const Options& o) {
        if(original==0)return 0;
        const bool negative=original<0;
        const long long magnitude=negative?-(long long)original:(long long)original;
        const int px=scale(magnitude,category,o);
        return negative?-px:px;
    }
    static int Leading(int original,Category category) {
        return LeadingWith(original,category,Config());
    }
    static int LeadingWith(int original,Category category,const Options& o) {
        if(original<=0)return original;
        // The old chat had 12px leading around a 15px font. Avoid overlap.
        const int baseline=category==Chat?std::max(original,16):original;
        return scale(baseline,category,o);
    }
    static float AtlasSize(float original,Category category=Atlas) {
        if(!(original>0.0f) || !std::isfinite(original))return original;
        const Options& o=Config();
        const double multiplier=static_cast<double>(o.global)*o.group[category]/10000.0;
        const double changed=static_cast<double>(original)*multiplier;
        return static_cast<float>(std::min(4096.0,std::max(1.0,changed)));
    }
private:
    static int bounded(long x,int lo,int hi,int fallback) {
        return x<lo||x>hi?fallback:static_cast<int>(x);
    }
    static void trim(std::string& s) {
        const std::string::size_type a=s.find_first_not_of(" \t\r\n");
        if(a==std::string::npos){s.clear();return;}
        const std::string::size_type b=s.find_last_not_of(" \t\r\n");
        s=s.substr(a,b-a+1);
    }
    static void lower(std::string& s) {
        std::transform(s.begin(),s.end(),s.begin(),
             [](unsigned char c){return static_cast<char>(std::tolower(c));});
    }
    static int scale(long long n,Category cat,const Options& o) {
        const int category=cat>=Chat && cat<CategoryCount?cat:Interface;
        const double product=static_cast<double>(n)*o.global*o.group[category]/10000.0;
        const double rounded=std::floor(product+0.5);
        return static_cast<int>(std::max(static_cast<double>(o.minimumPx),
                     std::min(static_cast<double>(o.maximumPx),rounded)));
    }
};
}

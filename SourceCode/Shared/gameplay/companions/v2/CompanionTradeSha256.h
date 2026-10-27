#pragma once
// Standalone SHA-256 for binding trade record images and journal receipts.
// No external crypto runtime or filesystem access. No MAC/authentication:
// it MUST NOT be used as a substitute for an authenticated server channel.
#include <array>
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace gameplay { namespace companions { namespace v2 {
using TradeSha256Digest = std::array<std::uint8_t, 32>;

class TradeSha256 {
public:
    static TradeSha256Digest Hash(const std::uint8_t* input, std::size_t length) {
        TradeSha256Digest result{};
        if (!input && length) return result;
        // SHA-256 length field counts bits modulo 2^64. Refuse overflow.
        if (length > UINT64_MAX / 8u) return result;
        std::uint32_t h[8] = {0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,
                              0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};
        const std::size_t blocks = length / 64;
        for (std::size_t i=0;i<blocks;++i) Process(input+i*64,h);
        std::uint8_t tail[128]{};
        const std::size_t remain=length%64;
        if (remain) std::memcpy(tail,input+blocks*64,remain);
        tail[remain]=0x80;
        const std::size_t padding=remain>=56?128:64;
        const std::uint64_t bits=static_cast<std::uint64_t>(length)*8u;
        for (int i=0;i<8;++i)
            tail[padding-8+i]=static_cast<std::uint8_t>(bits>>(56-8*i));
        Process(tail,h);
        if (padding==128) Process(tail+64,h);
        for (int i=0;i<8;++i) for(int b=0;b<4;++b)
            result[i*4+b]=static_cast<std::uint8_t>(h[i]>>(24-b*8));
        return result;
    }
private:
    static std::uint32_t R(std::uint32_t x,int n){return (x>>n)|(x<<(32-n));}
    static void Process(const std::uint8_t* p,std::uint32_t* h) {
        static const std::uint32_t k[64]={
        0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
        0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
        0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
        0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
        0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
        0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
        0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
        0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};
        std::uint32_t w[64]{};
        for(int i=0;i<16;++i) w[i]=(std::uint32_t(p[i*4])<<24)|
            (std::uint32_t(p[i*4+1])<<16)|(std::uint32_t(p[i*4+2])<<8)|p[i*4+3];
        for(int i=16;i<64;++i){
            const auto s0=R(w[i-15],7)^R(w[i-15],18)^(w[i-15]>>3);
            const auto s1=R(w[i-2],17)^R(w[i-2],19)^(w[i-2]>>10);
            w[i]=w[i-16]+s0+w[i-7]+s1;
        }
        auto a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],z=h[7];
        for(int i=0;i<64;++i){
            const auto s1=R(e,6)^R(e,11)^R(e,25);
            const auto t1=z+s1+((e&f)^((~e)&g))+k[i]+w[i];
            const auto s0=R(a,2)^R(a,13)^R(a,22);
            const auto t2=s0+((a&b)^(a&c)^(b&c));
            z=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
        }
        h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=z;
    }
};
}}}

#pragma once
// State only: no item reward until FishNodeForPlayer() succeeds at COLLECT.
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include "gameplay/gathering/v2/GatheringTypes.h"

namespace summit_fishing_378 {
struct Session {
    std::uint32_t nodeId=0;
    int area=-1;
    unsigned stage=0;
    std::uint64_t start=0;
    std::uint64_t changed=0;
};
struct Reply {
    bool success=false;
    bool collect=false;
    std::uint32_t nodeId=0;
    std::string message;
};
inline Reply Advance(const std::string& player, const std::string& action,
                     int area, std::uint64_t now, std::uint32_t nearestNode,
                     bool closeEnough) {
    static std::mutex lock;
    static std::map<std::string,Session> sessions;
    std::lock_guard<std::mutex> hold(lock);
    Reply reply;
    if(player.empty()||player.size()>255||area<0) {reply.message="Jogador invalido.";return reply;}
    Session& state=sessions[player];
    if(state.stage && (now<state.changed || now-state.start>60000 || state.area!=area))state=Session{};
    if(action=="cast") {
        if(!nearestNode||!closeEnough) {reply.message="Aproxima-te do ponto de pesca Sea.";return reply;}
        state=Session{};
        state.nodeId=nearestNode;state.area=area;state.start=now;state.changed=now;state.stage=1;
        reply.success=true;reply.message="CAST confirmado. Aguarda e usa /fish hook.";
        return reply;
    }
    if(!state.stage || !closeEnough || nearestNode!=state.nodeId) {
        reply.message="Sessao expirada ou ponto demasiado longe. /fish cast";return reply;
    }
    const std::uint64_t elapsed=now-state.changed;
    if(action=="hook" && state.stage==1 && elapsed>=500) {
        state.stage=2;state.changed=now;reply.success=true;
        reply.message="HOOK confirmado. Usa /fish reel.";return reply;
    }
    if(action=="reel" && state.stage==2 && elapsed>=350) {
        state.stage=3;state.changed=now;reply.success=true;
        reply.message="REEL confirmado. Usa /fish collect.";return reply;
    }
    if(action=="collect" && state.stage==3 && elapsed>=350) {
        reply.success=true;reply.collect=true;reply.nodeId=state.nodeId;
        reply.message="COLLECT autorizado.";state=Session{};return reply;
    }
    reply.message="Ordem invalida ou acao demasiado rapida: cast > hook > reel > collect.";
    return reply;
}
}

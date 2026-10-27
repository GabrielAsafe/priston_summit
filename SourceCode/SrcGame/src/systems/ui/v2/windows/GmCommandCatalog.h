#pragma once
// SUMMIT 355: catalogo de comandos GM. Sem novos opcodes ou elevacao de privilegios.
#include <array>
#include <cctype>
#include <cstdlib>
#include <string>
#include <vector>

namespace ui::v2::gm {
enum class Arg { None, Secret, Player, OptionalPlayer, Account, Code, OptionalCode,
  Id, OptionalId, Level, Number, Amount, Minutes, OptionalMinutes, Seconds,
  Days, Quantity, OptionalQuantity, OptionalMonsterCount, PotionCount, BossMinute, Monster, Item, Job, Permil, OptionalMode,
  OptionalText, Message };
struct Command {
    const char* token;
    const char* label;
    int level, group;
    std::array<Arg,3> args;
    bool confirm, local, join;
    const char* hint;
};
inline const std::vector<Command>& Catalog() {
    static const std::vector<Command> entries = {
        {"/ativar", "Ativar GM", 0, 0, {{Arg::Secret, Arg::None, Arg::None}}, true, false, false, "Autenticar com senha GM"},
        {"/desativar", "Desativar GM", 0, 0, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Desativar GM (necessita patch 354)"},
        {"/level", "Definir nivel", 2, 0, {{Arg::Level, Arg::None, Arg::None}}, true, true, false, "Definir nivel e EXP (relog para pontos)"},
        {"/Levelup", "Levelup", 2, 0, {{Arg::Level, Arg::None, Arg::None}}, true, true, false, "Alias para alterar nivel e EXP"},
        {"/skiptier", "Liberar tier 5", 2, 0, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Concluir quests 10010-10013"},
        {"/tier5test", "Testar tier 5", 2, 0, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Alias skiptier"},
        {"/reskill", "/reskill", 2, 1, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Recarregar e redistribuir skills"},
        {"/caravan", "/caravan", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Ativar caravana 3 horas"},
        {"/get_guid", "/get_guid", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "GUID e dados da sessao"},
        {"/get_event_level", "/get_event_level", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Nivel de evento do jogador"},
        {"/dm_info", "/dm_info", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Limites de damage e critical"},
        {"/hide", "/hide", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Ocultar GM"},
        {"/show", "/show", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Mostrar GM"},
        {"/user", "/user", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Contar usuarios"},
        {"/near", "/near", 2, 1, {{Arg::Player, Arg::None, Arg::None}}, true, false, false, "Teleportar para jogador"},
        {"/call", "/call", 2, 1, {{Arg::Player, Arg::None, Arg::None}}, true, false, false, "Puxar jogador ate GM"},
        {"/usercode", "/usercode", 2, 1, {{Arg::Code, Arg::None, Arg::None}}, false, false, false, "Consultar codigo interno"},
        {"/username", "/username", 2, 1, {{Arg::Player, Arg::None, Arg::None}}, false, false, false, "Consultar conta, IP e tempo"},
        {"/userid", "/userid", 2, 1, {{Arg::Account, Arg::None, Arg::None}}, false, false, false, "Consultar conta e personagem"},
        {"/sod_view", "/sod_view", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Consultar evento SOD"},
        {"/sod_enter", "/sod_enter", 2, 1, {{Arg::Number, Arg::None, Arg::None}}, true, false, false, "Forcar entrada no SOD"},
        {"/localtime", "/localtime", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Hora local do servidor"},
        {"/get_forceorb", "/get_forceorb", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Dados do Force Orb"},
        {"/get_primitem", "/get_primitem", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Timers Premium"},
        {"/get_currency", "/get_currency", 2, 1, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Dados de moeda"},
        {"/spec", "/spec", 2, 1, {{Arg::Job, Arg::None, Arg::None}}, true, false, false, "Spec temporaria (10min)"},
        {"/itemdaysrevision", "/itemdaysrevision", 2, 1, {{Arg::Days, Arg::None, Arg::None}}, true, false, false, "Revisao de dias -30 a 30"},
        {"/get_potion", "/get_potion", 2, 1, {{Arg::OptionalPlayer, Arg::None, Arg::None}}, false, false, false, "Contagem de pocoes"},
        {"/get_money", "/get_money", 3, 2, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Money da sessao"},
        {"/show_blackuser", "/show_blackuser", 3, 2, {{Arg::OptionalId, Arg::None, Arg::None}}, false, false, false, "Ver blacklist"},
        {"/reset_blackuser", "/reset_blackuser", 3, 2, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Zerar blacklist"},
        {"/free_blackuser", "/free_blackuser", 3, 2, {{Arg::Id, Arg::None, Arg::None}}, true, false, false, "Retirar ID da blacklist"},
        {"/area_monster", "/area_monster", 3, 2, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Monstros na area"},
        {"/agefreeOn", "/agefreeOn", 3, 2, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Ativar aging gratuito"},
        {"/agefreeOff", "/agefreeOff", 3, 2, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Desativar aging gratuito"},
        {"/modomanu", "/modomanu", 3, 2, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Alternar manutencao; kick players"},
        {"/GlobalOFF", "/GlobalOFF", 3, 2, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Desligar chat global"},
        {"/GlobalON", "/GlobalON", 3, 2, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Ligar chat global"},
        {"/monster", "/monster", 3, 2, {{Arg::Monster, Arg::OptionalMonsterCount, Arg::None}}, true, false, false, "Criar monstros perto (ate 20 adicionais)"},
        {"/mymonster", "/mymonster", 3, 2, {{Arg::Monster, Arg::OptionalMonsterCount, Arg::None}}, true, false, false, "Monstros ligados ao GM"},
        {"/area_users", "/area_users", 3, 2, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Usuarios na area"},
        {"/arena_monster", "/arena_monster", 3, 2, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Monstro da arena legacy"},
        {"/where", "/where", 3, 2, {{Arg::Player, Arg::None, Arg::None}}, false, false, false, "Localizar jogador"},
        {"/kick", "/kick", 3, 2, {{Arg::Player, Arg::None, Arg::None}}, true, false, false, "Desconectar jogador"},
        {"/kill", "/kill", 3, 2, {{Arg::Player, Arg::None, Arg::None}}, true, false, false, "Matar jogador"},
        {"/open_warehouse", "/open_warehouse", 3, 2, {{Arg::OptionalPlayer, Arg::OptionalMode, Arg::None}}, true, false, false, "Abrir ou consultar bau de outro jogador"},
        {"/>", "Mensagem broadcast", 3, 2, {{Arg::Message, Arg::None, Arg::None}}, true, false, true, "Broadcast administrativo"},
        {"/>>", "Notice global", 3, 2, {{Arg::Message, Arg::None, Arg::None}}, true, false, true, "Notice global"},
        {"/event_monster", "/event_monster", 4, 3, {{Arg::Monster, Arg::Minutes, Arg::Permil}}, true, false, false, "Monstro de evento na area"},
        {"/start_castle", "/start_castle", 4, 3, {{Arg::OptionalMinutes, Arg::None, Arg::None}}, true, false, false, "Iniciar Bless Castle"},
        {"/end_castle", "/end_castle", 4, 3, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Finalizar Bless Castle"},
        {"/easy_castle", "/easy_castle", 4, 3, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Modo easy castelo"},
        {"/set_castle_time", "/set_castle_time", 4, 3, {{Arg::Minutes, Arg::None, Arg::None}}, true, false, false, "Tempo restante do castelo"},
        {"/set_castle_master", "/set_castle_master", 4, 3, {{Arg::OptionalCode, Arg::None, Arg::None}}, true, false, false, "Definir master do castelo"},
        {"/get_castle_info", "/get_castle_info", 4, 3, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Estado do castelo"},
        {"/get_fountain_info", "/get_fountain_info", 4, 3, {{Arg::None, Arg::None, Arg::None}}, false, false, false, "Estado Soul Fountain"},
        {"/reset_fountain_time", "/reset_fountain_time", 4, 3, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Zerar Soul Fountain"},
        {"/set_castle_tax", "/set_castle_tax", 4, 3, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Aplicar NextTax e skill"},
        {"/shutdownpk", "/shutdownpk", 4, 3, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Desligamento do servidor"},
        {"/@get", "/@get", 4, 3, {{Arg::Item, Arg::None, Arg::None}}, true, false, false, "Criar item para si"},
        {"/inv", "/inv", 4, 3, {{Arg::Player, Arg::Item, Arg::None}}, true, false, false, "Criar item para jogador"},
        {"/setdelaychat", "/setdelaychat", 4, 3, {{Arg::Seconds, Arg::None, Arg::None}}, true, false, false, "Ajustar delay do chat global"},
        {"/Bosstime", "/Bosstime", 4, 3, {{Arg::BossMinute, Arg::None, Arg::None}}, true, false, false, "Minuto configurado do boss"},
        {"/setlevelpt", "/setlevelpt", 4, 3, {{Arg::Number, Arg::None, Arg::None}}, true, false, false, "Diferenca de nivel para party"},
        {"/pegarpots", "/pegarpots", 4, 3, {{Arg::PotionCount, Arg::None, Arg::None}}, true, false, false, "Criar/obter pocoes legacy"},
        {"/givemoney", "/givemoney", 4, 3, {{Arg::Player, Arg::Amount, Arg::None}}, true, false, false, "Entregar money (legacy)"},
        {"/autopromocao", "/autopromocao", 4, 3, {{Arg::Number, Arg::None, Arg::None}}, true, false, false, "Configurar auto promocao"},
        {"/reload", "/reload", 4, 3, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Recarregar configuracoes"},
        {"/rates", "/rates", 4, 3, {{Arg::None, Arg::None, Arg::None}}, true, false, false, "Consultar rates: implementacao atual so consulta"},
        {"/settimelogado", "/settimelogado", 4, 3, {{Arg::Number, Arg::None, Arg::None}}, true, false, false, "Definir tempo logado"},
        {"/settimelogado_coin", "/settimelogado_coin", 4, 3, {{Arg::Number, Arg::None, Arg::None}}, true, false, false, "Tempo logado para coin"},
        {"/setcoinlogado", "/setcoinlogado", 4, 3, {{Arg::Number, Arg::None, Arg::None}}, true, false, false, "Valor coin por tempo"},
        {"/aging", "/aging", 4, 3, {{Arg::Item, Arg::Number, Arg::Job}}, true, false, false, "Criar item aged"},
        {"/getcoins", "/getcoins", 4, 3, {{Arg::Player, Arg::None, Arg::None}}, false, false, false, "Consultar SCoins"},
        {"/addcoins", "/addcoins", 4, 3, {{Arg::Player, Arg::Amount, Arg::None}}, true, false, false, "Adicionar SCoins"},
        {"/removecoins", "/removecoins", 4, 3, {{Arg::Player, Arg::Amount, Arg::None}}, true, false, false, "Remover SCoins"},
        {"/quantidade", "/quantidade", 4, 3, {{Arg::Item, Arg::Quantity, Arg::None}}, true, false, false, "Criar multiplos itens (max 12)"},
        {"/giveexp", "/giveexp", 4, 3, {{Arg::Level, Arg::None, Arg::None}}, true, false, false, "Ajustar EXP inicial"},
        {"/givegold", "/givegold", 4, 3, {{Arg::Amount, Arg::None, Arg::None}}, true, false, false, "Criar gold no chao"},
        {"/init_premiumitem", "/init_premiumitem", 4, 3, {{Arg::Id, Arg::None, Arg::None}}, true, false, false, "Reinicializar premium item"},
    };
    return entries;
}
inline bool Optional(Arg a) {
    return a==Arg::OptionalPlayer || a==Arg::OptionalCode || a==Arg::OptionalId ||
      a==Arg::OptionalMinutes || a==Arg::OptionalQuantity || a==Arg::OptionalMode ||
      a==Arg::OptionalText || a==Arg::OptionalMonsterCount;
}
inline bool Numeric(Arg a) {
    switch (a) { case Arg::Id: case Arg::OptionalId: case Arg::Code: case Arg::OptionalCode:
    case Arg::Level: case Arg::Number: case Arg::Amount: case Arg::Minutes:
    case Arg::OptionalMinutes: case Arg::Seconds: case Arg::Days:
    case Arg::Quantity: case Arg::OptionalQuantity: case Arg::OptionalMonsterCount:
    case Arg::PotionCount: case Arg::BossMinute: case Arg::Permil:
      return true; default: return false; }
}
inline const char* ArgLabel(Arg a) {
    switch(a) {
    case Arg::Secret: return "senha";
    case Arg::Player: case Arg::OptionalPlayer: return "personagem";
    case Arg::Account: return "conta";
    case Arg::Code: case Arg::OptionalCode: return "codigo";
    case Arg::Id: case Arg::OptionalId: return "ID";
    case Arg::Level: return "nivel";
    case Arg::Number: return "valor";
    case Arg::Amount: return "quantidade";
    case Arg::Minutes: case Arg::OptionalMinutes: return "minutos";
    case Arg::Seconds: return "segundos";
    case Arg::Days: return "dias (-30..30)";
    case Arg::Quantity: case Arg::OptionalQuantity: case Arg::OptionalMonsterCount: case Arg::PotionCount: return "quantidade";
    case Arg::BossMinute: return "minuto (0..59)";
    case Arg::Monster: return "monstro";
    case Arg::Item: return "item/codigo";
    case Arg::Job: return "job/spec";
    case Arg::Permil: return "permil";
    case Arg::OptionalMode: return "modo";
    case Arg::OptionalText: return "parametros";
    case Arg::Message: return "mensagem";
    default: return "";
    }
}
// Os comandos, inclusive os de parametro opcional, sao sempre filtrados no cliente
// e continuam sob a validacao de security do servidor. Nenhum comando GM e enviado
// se faltarem dados, houver separadores suspeitos ou o tamanho exceder o chat legado.
inline bool Build(const Command& c, const std::array<std::string,3>& values,
                  std::string& output) {
    output = c.token;
    bool omitted = false;
    for (int i=0;i<3;++i) {
        const Arg arg=c.args[i];
        if (arg==Arg::None) break;
        const auto& v=values[static_cast<size_t>(i)];
        if(v.empty() || v=="(omitir)") {
            if(Optional(arg)) {omitted=true;continue;}
            output.clear(); return false;
        }
        if(omitted || v.size()>74) {output.clear(); return false;}
        for (unsigned char ch : v) {
            if(ch<32 || ch==127 || ch==';' || ch=='`' || ch=='\\') {output.clear(); return false;}
        }
        if(arg==Arg::Secret || arg==Arg::Player || arg==Arg::OptionalPlayer ||
           arg==Arg::Account || arg==Arg::Item || arg==Arg::Job) {
            if(v.find_first_of(" \t\n\r")!=std::string::npos) {output.clear(); return false;}
        }
        if(arg==Arg::Monster && v.find('"')!=std::string::npos) {output.clear();return false;}
        if (Numeric(arg)) {
            if (v.empty()) {output.clear(); return false;}
            for (size_t p=0;p<v.size();++p)
                if (!std::isdigit(static_cast<unsigned char>(v[p])) && !(p==0 && v[p]=='-' && arg==Arg::Days))
                    {output.clear(); return false;}
            const long n=std::strtol(v.c_str(),nullptr,10);
            if(n>1000000000L || n<-1000000000L || (arg==Arg::Days && (n<-30||n>30)) ||
                ((arg==Arg::Quantity || arg==Arg::OptionalQuantity) && (n<1||n>12)) ||
                (arg==Arg::Permil && (n<0||n>1000)) ||
                (arg==Arg::OptionalMonsterCount && (n<0||n>20)) ||
                (arg==Arg::PotionCount && (n<2||n>300)) ||
                (arg==Arg::BossMinute && (n<0||n>59)) || (arg==Arg::Level && n<1))
                {output.clear(); return false;}
        }
        if(arg==Arg::Monster && v.find(' ')!=std::string::npos) {
            if(v.find('"')!=std::string::npos){output.clear();return false;}
            output+=' '+std::string("\"")+v+"\"";
        }
        else if (c.join && i==0) output+=v;
        else output+=' '+v;
        if (output.size()>88) {output.clear(); return false;}
    }
    return true;
}
}

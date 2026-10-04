#include <cstring>
#include <cstdio>
#include "systems/network/protocol/server/opcodes.h"
#include "systems/community/news/News.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "Shared/community/v2/NewsContentService.h"
#include "Shared/community/v2/NewsFeedService.h"
#include "Shared/community/v2/NewsTitleService.h"
#include "systems/community/news/v2/legacy/LegacyNewsNoticeRepository.h"
#include "systems/community/news/v2/legacy/LegacyNewsTitleRepository.h"
#include "modules/File/File.h"
#include "smPacket.h"
#include <string>
#include "content/v1/localization/content/LocalizationRuntime.h"
#include "content/v1/localization/legacy/LocalizationLegacyEncoding.h"
#include "systems/persistence/database/core/Database.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"

extern int OpenBossTimeMin;

namespace
{
bool SummitNewsLegacyFormat(std::string &legacyFormat, const char *key)
{
    return summit::content::localization::GetLocalizationLegacyValue(key, legacyFormat);
}

bool SummitNewsCopyToLegacyBuffer(char *destination, std::size_t capacity, const std::string& message)
{
    if(!destination || capacity == 0 || message.size() >= capacity)
    {
        if(destination && capacity)
            destination[0] = 0;
        return false;
    }
    std::memcpy(destination, message.c_str(), message.size() + 1u);
    return true;
}

bool SummitNewsFormatName(char *destination, std::size_t capacity, const char *key, const char *name)
{
    std::string format;
    std::string message;
    return SummitNewsLegacyFormat(format, key) &&
        gameplay::community::NewsContentService::FormatName(message, format, name) &&
        SummitNewsCopyToLegacyBuffer(destination, capacity, message);
}

bool SummitNewsFormatNumber(char *destination, std::size_t capacity, const char *key, int value)
{
    std::string format;
    std::string message;
    return SummitNewsLegacyFormat(format, key) &&
        gameplay::community::NewsContentService::FormatNumber(message, format, value) &&
        SummitNewsCopyToLegacyBuffer(destination, capacity, message);
}

bool SummitNewsSend(rsPLAYINFO *recipient, const void *data, int size)
{
    if(!recipient || !data || size <= 0)
        return false;
    return gameplay::network::legacy::ServerLegacyPacketSender().Send(
        gameplay::PlayerId(static_cast<std::uint32_t>(recipient->dwObjectSerial)),
        gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}
}


void News::welcome()
{
    if(!player || !player->lpsmSock)
        return;

    TRANS_CHATMESSAGE packet = {};
    if(!SummitNewsFormatName(
        packet.szMessage,
        sizeof(packet.szMessage),
        "news.welcome",
        player->getName()))
        return;

    packet.code = OpCode::OPCODE_WHISPERMESSAGE;
    packet.size = 32 + lstrlen(packet.szMessage);
    packet.dwIP = 3;
    packet.dwObjectSerial = 0;
    (void)SummitNewsSend(player, &packet, packet.size);
}

void News::title()
{
    if(!player || !player->lpsmSock)
        return;

    std::string message;
    if(!gameplay::community::NewsTitleService::Load(
        gameplay::community::legacy::ServerLegacyNewsTitleRepository(), message))
        return;

    TRANS_CHATMESSAGE packet = {};
    packet.code = OpCode::OPCODE_WHISPERMESSAGE;
    strcpy_s(packet.szMessage, sizeof(packet.szMessage), message.c_str());
    packet.size = 32 + lstrlen(packet.szMessage);
    packet.dwIP = 3;
    packet.dwObjectSerial = 0;
    (void)SummitNewsSend(player, &packet, packet.size);
}

void News::msgCoinShop()
{
    if(!player || !player->lpsmSock)
        return;

    TRANS_CHATMESSAGE packet = {};
    if(SummitNewsFormatNumber(
        packet.szMessage,
        sizeof(packet.szMessage),
        "news.coin_shop_balance",
        cCoinShop.GetPlayerCoin(player)))
    {
        packet.code = OpCode::OPCODE_WHISPERMESSAGE;
        packet.size = 32 + lstrlen(packet.szMessage);
        packet.dwIP = 3;
        packet.dwObjectSerial = 0;
        (void)SummitNewsSend(player, &packet, packet.size);
    }

    ZeroMemory(&packet, sizeof(packet));
    if(SummitNewsFormatNumber(
        packet.szMessage,
        sizeof(packet.szMessage),
        "news.time_shop_balance",
        cCoinShop_T.GetPlayerCoin(player)))
    {
        packet.code = OpCode::OPCODE_WHISPERMESSAGE;
        packet.size = 32 + lstrlen(packet.szMessage);
        packet.dwIP = 3;
        packet.dwObjectSerial = 0;
        (void)SummitNewsSend(player, &packet, packet.size);
    }
}

void News::msgNotice()
{
	try
	{
        std::vector<std::string> messages;
        if(!gameplay::community::NewsFeedService::LoadActiveMessages(
            gameplay::community::legacy::ServerLegacyNewsNoticeRepository(),
            OpenBossTimeMin, messages))
            return;

        for(const std::string& message : messages)
        {
            TRANS_CHATMESSAGE Login = {};
            if(!SummitNewsCopyToLegacyBuffer(Login.szMessage, sizeof(Login.szMessage), message))
                continue;

            Login.code = OpCode::OPCODE_WHISPERMESSAGE;
            Login.size = 32 + lstrlen(Login.szMessage);
            Login.dwIP = 13;
            Login.dwObjectSerial = 0;
            (void)SummitNewsSend(player, &Login, Login.size);
        }
	}
	catch (FileException)
	{
	}
}

void News::msgPlayerLogado(rsPLAYINFO *jogadores)
{
    if(!player || !player->lpsmSock || !jogadores || !jogadores->lpsmSock)
        return;

    TRANS_CHATMESSAGE packet = {};
    if(!SummitNewsFormatName(
        packet.szMessage,
        sizeof(packet.szMessage),
        "news.player_online",
        player->getName()))
        return;

    packet.code = OpCode::OPCODE_WHISPERMESSAGE;
    packet.size = 32 + lstrlen(packet.szMessage);
    packet.dwIP = 9;
    packet.dwObjectSerial = 0;
    (void)SummitNewsSend(jogadores, &packet, packet.size);
}



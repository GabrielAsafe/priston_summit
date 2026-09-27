#pragma once
#include <cstring>
#include <cstdio>
#include <windows.h>
#include <vector>
#include "content/v1/definition/content/DefinitionRuntime.h"
#include "content/v1/localization/content/LocalizationRuntime.h"
#include "formats/character_config/CharacterConfigReader.inl"
#include "formats/character_config/CharacterConfigLegacyAdapter.inl"
#include "formats/item_config/ItemDefinitionReader.inl"

namespace summit { namespace content { namespace definition {

inline UINT SummitLegacyLocalizationCodePage()
{
#ifdef _LANGUAGE_BRAZIL
 return 1252u; // Legacy PT-BR assets are byte-oriented Windows-1252.
#else
 return CP_ACP;
#endif
}

inline bool SummitEncodeLocalizedText(const std::string &utf8,std::string &legacyBytes)
{
 legacyBytes.clear();
 if(utf8.empty()) return true;
 const UINT codePage=SummitLegacyLocalizationCodePage();
 if(codePage==CP_UTF8){legacyBytes=utf8;return true;}

 const int wideCount=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.c_str(),static_cast<int>(utf8.size()),0,0);
 if(wideCount<=0)return false;
 std::vector<wchar_t> wide(static_cast<std::size_t>(wideCount));
 if(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.c_str(),static_cast<int>(utf8.size()),&wide[0],wideCount)!=wideCount)return false;

 BOOL usedDefault=FALSE;
 const int byteCount=WideCharToMultiByte(codePage,WC_NO_BEST_FIT_CHARS,&wide[0],wideCount,0,0,0,&usedDefault);
 if(byteCount<=0||usedDefault)return false;
 legacyBytes.resize(static_cast<std::size_t>(byteCount));
 usedDefault=FALSE;
 if(WideCharToMultiByte(codePage,WC_NO_BEST_FIT_CHARS,&wide[0],wideCount,&legacyBytes[0],byteCount,0,&usedDefault)!=byteCount||usedDefault){legacyBytes.clear();return false;}
 return true;
}

inline bool SummitCopyLegacyBytes(char *destination,std::size_t capacity,const std::string &value)
{
 if(!destination||capacity==0||value.size()>=capacity)return false;
 std::memcpy(destination,value.c_str(),value.size()+1);return true;
}

inline bool SummitCopyLocalizedText(char *destination,std::size_t capacity,const std::string &value)
{
 std::string legacyBytes;
 return SummitEncodeLocalizedText(value,legacyBytes)&&SummitCopyLegacyBytes(destination,capacity,legacyBytes);
}

inline bool ApplyCompiledCharacterLocalization(
 const std::string &contentId,smCHAR_INFO &character,smCHAR_MONSTER_INFO *monster,
 char *dialogMessage,std::size_t dialogCapacity,
 const summit::formats::character_config::CharacterConfigServices &services)
{
 using namespace summit::content::localization;
 if(!LocalizationOverrideEnabled())return true;
 if(!CompiledLocalizationEnabled())return LocalizationStrictMode()?false:true;
 if(!CompiledLocalizationReady())return LocalizationStrictMode()?false:true;
 std::string value;const std::string nameKey=contentId+".name";
 if(GetLocalizationValue(nameKey.c_str(),value)){
  const bool copied=monster?SummitCopyLocalizedText(monster->szName,sizeof(monster->szName),value):SummitCopyLocalizedText(character.szName,sizeof(character.szName),value);
  if(!copied&&LocalizationStrictMode())return false;
 }else if(LocalizationStrictMode())return false;

 if(monster){
  std::vector<std::string> chats;
  for(int i=1;i<=NPC_MESSAGE_MAX;++i){char suffix[32];std::sprintf(suffix,".chat.%d",i);std::string key=contentId+suffix;if(!GetLocalizationValue(key.c_str(),value))break;chats.push_back(value);}
  if(!chats.empty()){
   std::vector<std::string> legacyChats;legacyChats.reserve(chats.size());bool encodable=true;
   for(std::size_t i=0;i<chats.size();++i){std::string encoded;if(!SummitEncodeLocalizedText(chats[i],encoded)){encodable=false;break;}legacyChats.push_back(encoded);}
   if(!encodable){if(LocalizationStrictMode())return false;}
   else{
    monster->NpcMsgCount=0;
    for(std::size_t i=0;i<legacyChats.size()&&monster->NpcMsgCount<NPC_MESSAGE_MAX;++i){
     char *stored=services.internNpcMessage?services.internNpcMessage(legacyChats[i].c_str()):0;
     if(stored)monster->lpNpcMessage[monster->NpcMsgCount++]=stored;
    }
    if(dialogMessage&&dialogCapacity&&!SummitCopyLegacyBytes(dialogMessage,dialogCapacity,legacyChats.back())){if(LocalizationStrictMode())return false;}
   }
  }
 }
 return true;
}

inline bool ApplyCompiledCharacterDefinition(
 const char*path, smCHAR_INFO &character, smCHAR_MONSTER_INFO *monster,
 char *dialogMessage, std::size_t dialogCapacity,
 const summit::formats::character_config::CharacterConfigServices &services,
 std::string *contentId=0)
{
 CompiledDefinition def;
 if(!LoadCompiledDefinition(path,def) || (def.kind!=DefinitionKindMonster && def.kind!=DefinitionKindNpc)) return false;
 if(contentId) *contentId=def.contentId;
 using namespace summit::formats::character_config;
 smCHAR_INFO stagedCharacter=character;
 smCHAR_MONSTER_INFO stagedMonster;
 smCHAR_MONSTER_INFO *stagedMonsterPtr=monster?&stagedMonster:0;
 InitializeCharacterImport(stagedCharacter,stagedMonsterPtr);
 char stagedDialog[256]={0}; char *dialogTarget=0; std::size_t stagedDialogCapacity=0;
 if(dialogMessage&&dialogCapacity){dialogTarget=stagedDialog;stagedDialogCapacity=dialogCapacity<sizeof(stagedDialog)?dialogCapacity:sizeof(stagedDialog);if(!stagedDialogCapacity)return false;std::size_t n=0;while(n<stagedDialogCapacity&&dialogMessage[n])++n;if(n<stagedDialogCapacity)std::memcpy(stagedDialog,dialogMessage,n+1);}
 for(std::size_t i=0;i<def.directives.size();++i){const CompiledDirective&d=def.directives[i];
  summit::formats::legacy_config::LineCursor coreValues(d.values.c_str()); ParseResult r=HandleCharacterCore(d.command.c_str(),coreValues,stagedCharacter,stagedMonsterPtr,dialogTarget,stagedDialogCapacity,services);if(r==ParseResult::Error)return false;if(r==ParseResult::Handled)continue;
  if(stagedMonsterPtr){summit::formats::legacy_config::LineCursor monsterValues(d.values.c_str());r=HandleMonsterBehavior(d.command.c_str(),monsterValues,stagedCharacter,*stagedMonsterPtr,services);if(r==ParseResult::Error)return false;if(r==ParseResult::Handled)continue;summit::formats::legacy_config::LineCursor npcValues(d.values.c_str());r=HandleNpcServices(d.command.c_str(),npcValues,*stagedMonsterPtr,services);if(r==ParseResult::Error)return false;}
 }
 if(!ApplyCompiledCharacterLocalization(def.contentId,stagedCharacter,stagedMonsterPtr,stagedDialog,stagedDialogCapacity,services))return false;
 stagedCharacter.lpMonInfo=stagedMonsterPtr?monster:0;character=stagedCharacter;if(monster)*monster=stagedMonster;
 if(dialogMessage&&dialogCapacity){const std::size_t n=std::strlen(stagedDialog);if(n>=dialogCapacity)return false;std::memcpy(dialogMessage,stagedDialog,n+1);}return true;
}

inline bool ApplyCompiledItemDefinition(const char*path,sDEF_ITEMINFO &output,std::string *contentId=0)
{
 if(!CompiledItemDefinitionsEnabled())return false;
 CompiledDefinition def;if(!LoadCompiledDefinition(path,def)||def.kind!=DefinitionKindOpenItem)return false;if(contentId)*contentId=def.contentId;
 using namespace summit::formats::item_config;sDEF_ITEMINFO staged=sDEF_ITEMINFO();InitializeItemDefinitionImport(staged);
 for(std::size_t i=0;i<def.directives.size();++i){const CompiledDirective&d=def.directives[i];summit::formats::legacy_config::LineCursor values(d.values.c_str());if(HandleIdentityAndVisual(d.command.c_str(),values,staged))continue;summit::formats::legacy_config::LineCursor combat(d.values.c_str());if(HandleCombatStats(d.command.c_str(),combat,staged))continue;summit::formats::legacy_config::LineCursor req(d.values.c_str());if(HandleResourcesAndRequirements(d.command.c_str(),req,staged))continue;summit::formats::legacy_config::LineCursor job(d.values.c_str());if(HandleJobSpecialization(d.command.c_str(),job,staged))continue;if(TokenIs(d.command.c_str(),token::GenerationLimit)){summit::formats::legacy_config::LineCursor gen(d.values.c_str());staged.sGenDay[0]=ReadInt(gen);}}
 if(staged.Item.EffectBlink[0])staged.Item.EffectBlink[1]=staged.Item.EffectColor[0]+staged.Item.EffectColor[1]+staged.Item.EffectColor[2]+staged.Item.EffectColor[3]+static_cast<short>(staged.Item.DispEffect);
 // CODE=0 means the profile pack does not resolve against this executable's authoritative item catalog.
 // Reject the record instead of materializing a silent/invalid item entry.
 if(staged.Item.CODE==0)return false;
 using namespace summit::content::localization;
 if(LocalizationOverrideEnabled()){
  if(!CompiledLocalizationEnabled()){if(LocalizationStrictMode())return false;}
  else if(!CompiledLocalizationReady()){if(LocalizationStrictMode())return false;}
  else {std::string value;const std::string key=def.contentId+".name";if(GetLocalizationValue(key.c_str(),value)){if(!SummitCopyLocalizedText(staged.Item.ItemName,sizeof(staged.Item.ItemName),value)&&LocalizationStrictMode())return false;}else if(LocalizationStrictMode())return false;}
 }
 output=staged;return true;
}

}}}

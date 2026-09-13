#pragma once

#include <cstring>

#include "formats/character_config/CharacterCoreReader.h"
#include "formats/character_config/CharacterMonsterReader.h"
#include "formats/character_config/CharacterNpcReader.h"
#include "io/LegacyFile.h"
#include "io/LegacyPath.h"

namespace summit
{
namespace formats
{
namespace character_config
{

inline void InitializeCharacterImport(smCHAR_INFO &character, smCHAR_MONSTER_INFO *monster)
{
	character.szModelName2[0]=0;
	character.wPlayClass[1]=0;
	character.ArrowPosi[0]=0;
	character.ArrowPosi[1]=0;
	if(monster)
	{
		std::memset(monster,0,sizeof(*monster));
		monster->MoveRange=64*fONE;
		monster->DamageStunPers=100;
		monster->UseEventModel=0;
	}
}

inline bool ReadCharacterDefinition(
	const char *sourceFile,
	smCHAR_INFO &character,
	smCHAR_MONSTER_INFO *monster,
	char *dialogMessage,
	std::size_t dialogCapacity,
	const CharacterConfigServices &services)
{
	if(!sourceFile || !*sourceFile)
		return false;

	smCHAR_INFO stagedCharacter=character;
	smCHAR_MONSTER_INFO stagedMonster;
	smCHAR_MONSTER_INFO *stagedMonsterPtr=monster ? &stagedMonster : 0;
	InitializeCharacterImport(stagedCharacter,stagedMonsterPtr);

	char stagedDialog[256]={0};
	char *dialogTarget=0;
	std::size_t stagedDialogCapacity=0;
	if(dialogMessage && dialogCapacity)
	{
		dialogTarget=stagedDialog;
		stagedDialogCapacity=dialogCapacity < sizeof(stagedDialog) ? dialogCapacity : sizeof(stagedDialog);
		if(stagedDialogCapacity == 0) return false;
		std::size_t existingLength=0;
		while(existingLength < stagedDialogCapacity && dialogMessage[existingLength]) ++existingLength;
		if(existingLength < stagedDialogCapacity)
		{
			std::memcpy(stagedDialog,dialogMessage,existingLength+1);
		}
	}

	summit::io::LegacyFile file(sourceFile,"rb");
	if(!file) return false;

	char lineBuffer[256];
	char command[128];
	while(file.ReadLine(lineBuffer,sizeof(lineBuffer)))
	{
		legacy_config::LineCursor line(lineBuffer);
		if(!line.NextToken(command,sizeof(command))) continue;

		legacy_config::LineCursor coreValues(line.Position());
		ParseResult result=HandleCharacterCore(command,coreValues,stagedCharacter,stagedMonsterPtr,dialogTarget,stagedDialogCapacity,services);
		if(result == ParseResult::Error) return false;
		if(result == ParseResult::Handled) continue;

		if(stagedMonsterPtr)
		{
			legacy_config::LineCursor monsterValues(line.Position());
			result=HandleMonsterBehavior(command,monsterValues,stagedCharacter,*stagedMonsterPtr,services);
			if(result == ParseResult::Error) return false;
			if(result == ParseResult::Handled) continue;

			legacy_config::LineCursor npcValues(line.Position());
			result=HandleNpcServices(command,npcValues,*stagedMonsterPtr,services);
			if(result == ParseResult::Error) return false;
			if(result == ParseResult::Handled) continue;
		}

		if(IsToken(command,token::LinkedFile))
		{
			legacy_config::LineCursor linkValues(line.Position());
			char relative[128];
			char nextFile[260];
			if(!linkValues.NextValue(relative,sizeof(relative)) ||
				!summit::io::BuildPathFromFileDirectory(sourceFile,relative,nextFile,sizeof(nextFile)))
				return false;
			if(!file.Open(nextFile,"rb"))
				break; // historical behavior: linked-file open failure ends parsing successfully
		}
	}

	stagedCharacter.lpMonInfo=stagedMonsterPtr ? monster : 0;
	character=stagedCharacter;
	if(monster) *monster=stagedMonster;
	if(dialogMessage && dialogCapacity)
	{
		const std::size_t length=std::strlen(stagedDialog);
		if(length >= dialogCapacity) return false;
		std::memcpy(dialogMessage,stagedDialog,length+1);
	}
	return true;
}

} // namespace character_config
} // namespace formats
} // namespace summit

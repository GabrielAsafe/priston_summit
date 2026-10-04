#include "systems/admin/game_masters/GameMasters.h"
#include "systems/persistence/database/core/Database.h"
#include <iostream>
#include <memory>


using namespace std;


GameMasters gms;

GameMasters::GameMasters() : cursor(0)
{
	ZeroMemory(users, sizeof(users));
}

GameMasters::~GameMasters()
{
	clear();
}

bool GameMasters::readFromDatabase()
{
	clear();

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_AccountDB);

	if (db->Open())
	{
		if (db->Prepare("SELECT id,accountId,password,security,active FROM GameMasters WHERE active =?"))
		{
			int active = 1;
			db->BindInputParameter(&active, 1, PARAMTYPE_Integer);

			if (db->Execute())
			{
				while (true)
				{
					GameMaster *gamemaster = new GameMaster;

					db->GetData(&gamemaster->id, 0, 1, PARAMTYPE_Integer);
					db->GetData(gamemaster->accountId, 32, 2, PARAMTYPE_String);
					db->GetData(gamemaster->password, 30, 3, PARAMTYPE_String);
					db->GetData(&gamemaster->security, 0, 4, PARAMTYPE_Integer);
					db->GetData(&gamemaster->active, 0, 5, PARAMTYPE_Integer);


					users[cursor++] = gamemaster;

					if (!db->NextRow())
						break;
				}
			}
		}


		db->Close();
		rebuildAuthorization();
		return true;
	}

	return false;
}

AccountTypes GameMasters::getGameMaster(const std::string & accountId, const std::string & password) const
{
	return static_cast<AccountTypes>(authorization.Authenticate(
		accountId, password, static_cast<int>(AccountTypes::SEC_PLAYER)));
}

AccountTypes GameMasters::isGameMaster(const std::string & accountId) const
{
	return static_cast<AccountTypes>(authorization.ResolveActiveSecurity(
		accountId, static_cast<int>(AccountTypes::SEC_PLAYER)));
}

void GameMasters::print() const
{
	cout << endl;

	for(unsigned int i = 0; i < MAX_GMS; i++)
	{
		if(users[i])
		{
			cout << "id: [" << users[i]->id;
			cout << "] - AccountId: [" << users[i]->accountId;
			cout << "] - Password: [" << users[i]->password;
			cout << "] - security: [" << (int)users[i]->security << "]" << endl;
		}
	}

	cout << endl;
}



GameMasters* GameMasters::getInstance()
{
	return &gms;
}

void GameMasters::clear()
{
	for(unsigned int i = 0; i < MAX_GMS; i++)
	{
		delete users[i];
		users[i] = nullptr;
	}

	cursor = 0;
	authorization.ReplaceAll({});

	ZeroMemory(users, sizeof(users));
}

void GameMasters::rebuildAuthorization()
{
	std::vector<gameplay::admin::AdminIdentity> identities;
	identities.reserve(static_cast<std::size_t>(cursor));
	for (int i = 0; i < cursor && i < MAX_GMS; ++i)
	{
		if (!users[i])
			continue;
		identities.push_back({
			users[i]->accountId,
			users[i]->password,
			static_cast<int>(users[i]->security),
			users[i]->active});
	}
	authorization.ReplaceAll(std::move(identities));
}



#include "server_build_compat.h"
#include "CLI/CLI.h"
#include <iostream>
#include <windows.h>
#include "smwsock.h"
#include "Server/onserver.h"
#include "smPacket.h"
#include "GameMasters/GameMasters.h"
#include <chrono>
#include <ctime>
#include <ratio>

using namespace std;

extern int OpenBossTimeMin;
extern int rsRefreshConfig();

using std::chrono::system_clock;

CLI commandLine;


CLI::CLI()
{
	done = nullptr;

}

CLI::~CLI()
{

}
extern int ActivePlayCounter;

bool CLI::run(bool *done)
{
	this->done = done;

	std::string input;

	system_clock::time_point today = system_clock::now();

	std::time_t tt = system_clock::to_time_t(today);

	extern TRANS_SERVER_LIST TransServerList;

	cout << "Seja Bem Vindo ao Summit Tale. Os comandos terminam com ;" << endl;
	cout << "O Servidor esta Ativo no IP : " << TransServerList.ServerInfo[0].szServerIp1 << " : " << SERVER_TCP_PORT << " " << endl;
	cout << "Hoje: " << ctime(&tt) << endl;
	cout << "Versao: Oficial" << endl;

	cout << "GameServer: Mapas (Estavel) Verificando." << endl << endl;
	cout << "            Monstros (Estavel) Verificando.." << endl << endl;
	cout << "            NPCs (Estavel) Verificando..." << endl << endl;
	cout << "            Itens (Estavel) Verificando.." << endl << endl;

	cout << "Digite: 'help;' para abrir alguns comandos." << endl << endl;

	while (!(*this->done))
	{
		cout << "server> ";
		cin >> input;

		if (input.back() != ';')
		{
			cout << "Comandos precisam terminar com ;" << endl;
		}
		else
		{
			process(input);
		}
	}


	return true;
}

bool CLI::process(const std::string &cmd)
{
	
	if (cmd.compare("help;") == 0)
		handleHelpCommand();
	else if (cmd.compare("exit;") == 0)
		handleExitCommand();
	else if (cmd.compare("shutdown;") == 0)
		handleShutdownCommand();
	else if (cmd.compare("clear;") == 0)
		system("cls");
	else if (cmd.compare("connected;") == 0)
		handleConnectedCommand();
	else if (cmd.compare(0, 4, "kick") == 0)
		handleKickCommand(cmd);
	else if (cmd.compare("reloadhotuk;") == 0)
		handleReloadHotuk();
	else if (cmd.compare("reloadGMS;") == 0)
		handleReloadGMSCommand();
	else if (cmd.compare("showGMS;") == 0)
		handlePrintGMSCommand();
	else if (cmd.compare("boss;") == 0)
		handleBossTIME();
	else if (cmd.compare("bosschange;") == 0)
		handleBossTIMEChange();
	else
		cout << "Comando invalido. Digite help; para ajuda" << endl;




	return false;
}

void CLI::handleHelpCommand()
{
	cout << "Comandos terminam em ;" << endl;
	cout << "Comando: help; <Exibe mensagens para auxiliar o administrador>" << endl;
	cout << "Comando: exit; <Fecha o servidor>" << endl;
	cout << "Comando: shutdown; <Fecha o servidor em 3 minutos>" << endl;
	cout << "Comando: clear; <Limpa a tela de comandos>" << endl;
	cout << "Comando: connected; <Exibe todos os usuarios conectados ao servidor>" << endl;
	cout << "Comando: kick(player name); <Expulsa um jogador do servidor>" << endl;
	cout << "Comando: reloadGMS; <Recarrega os Game Masters do servidor>" << endl;
	cout << "Comando: showGMS; <Exibe todos os Game Masters ativados>" << endl;
}

void CLI::handleExitCommand()
{
	*this->done = true;
}

void CLI::handleShutdownCommand()
{
	rsShutDown();
	cout << "Servidor sera desligado em 3 minutos" << endl;
}

void CLI::handleConnectedCommand()
{
	int count = 0;

	for(int cnt = 0; cnt < CONNECTMAX; cnt++)
	{
		if(rsPlayInfo[cnt].lpsmSock)
		{
			rsPLAYINFO& player = rsPlayInfo[cnt];

			cout << "Id: " << player.getAccount() << " - Name: " << player.getName() << " - IP: " << player.lpsmSock->szIPAddr << " - Stage: " << player.Position.Area << endl;
			count++;
		}
	}

	cout << "Usuarios conectados: " << count << endl;
}

void CLI::handleKickCommand(const std::string & cmd)
{
	if(cmd.length() < 6)
		return;

	std::string name;
	bool flag = false;

	for(auto it = cmd.begin() + 5; it != cmd.end(); ++it)
	{
		if(*it == ')')
		{
			flag = true;
			break;
		}

		name += *it;
		
	}

	if(!flag) return;

	rsPLAYINFO* player = FindUserFromName(const_cast<char*>(name.c_str()));

	if(!player)
	{
		player = FindUserFromName2(const_cast<char*>(name.c_str()));
	}

	if(player)
	{
		DisconnectUser(player->lpsmSock);
	}
}

void CLI::handlePrintGMSCommand()
{
	GameMasters::getInstance()->print();
}

void CLI::handleReloadGMSCommand()
{
	GameMasters::getInstance()->readFromDatabase();
	cout << "GMS Recarregados" << endl;
}

CLI* CLI::getInstance()
{
	return &commandLine;
}

void CLI::handleBossTIME()
{
	printf("Boss Time: xx:%02d/n", OpenBossTimeMin);
	cout << "Boss time: " << OpenBossTimeMin << endl;
}

void CLI::handleBossTIMEChange()
{
	OpenBossTimeMin = 0;
	printf("Boss Time alterado para: xx:%02d/n", OpenBossTimeMin);
	/*cout << "BossTime alterado para " << OpenBossTimeMin << endl;*/
}


void CLI::handleReloadHotuk()
{
	rsRefreshConfig();
	cout << "Hotuk recarregado com sucesso!" << endl;
}



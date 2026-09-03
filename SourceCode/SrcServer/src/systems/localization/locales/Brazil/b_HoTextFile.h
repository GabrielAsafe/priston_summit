#include "systems/localization/legacy/ho_text/HoTextFile.h"

char *HoTextLoginMessage[18] = {
	"Estabelecendo a conex\343o...",
	"N\343o foi poss\355vel se conectar ao servidor",
	"     Digite sua ID novamente    ",
	" Digite sua senha novamente  ",
	"ID bloqueada, visite nosso site",
	"  Essa ID j\341 est\341 logada  ",
	"  Vers\343o n\343o compat\355vel   ",
	"      Servidor cheio       ",
	"Conex\343o perdida com o servidor",
	"   Tempo limite excedido  ",
	"   Tempo limite excedido   ",
	" Deletando a conta no servidor ",
	"Servidor sobrecarregado!!!, por favor, tente mais tarde.",
	" Por favor, tente mais tarde... ",
	"Senha incorreta por mais de 3 vezes.",
	"Voc\352 n\343o pode usar esses caracteres na senha.",
	"allow",
};

//char *HoTextLoginMessage[12] = {
//	"Estabelecendo conex�o...",
//	"N�o foi poss�vel conectar-se ao servidor.",
//	"     Usu�rio e/ou senha incorretos.    ",
//	"ID bloqueada, visite nosso site para mais informa��es.",
//	"IP bloqueado, visite nosso site para mais informa��es.",
//	"PC bloqueado, visite nosso site para mais informa��es.",
//	"  Esta ID j� est� conectada.  ",
//	"  Vers�o incompat�vel.   ",
//	"     O servidor est� cheio.      ",
//	"Conex�o perdida com o servidor.",
//	"Servidor em manuten��o, por favor, tente mais tarde.",
//};

char *HoTextSelectMessage[12] = { "Voc\352 deseja deletar?",
				  "Continuar esta aventura?",
				  "Limite de personagens.",
				  "Cria\347\343o completa. Confirmar?",
				  "Digite um nome",
				  "Esse nome j\341 existe",
				  "  Selecione um personagem   ",
				  "    Selecione uma tribo     ",
				  "    Selecione uma classe     ",
				  "     Selecione a face     ",
				  "Imposs\355vel deletar L\355der do Cl\343",
				  "Dados do Cl\343 n\343o recebidos"
};

char *HoTextTempscronInfo[5] = { " Os Tempskrons s\343o a civiliza\347\343o mais avan\347ada do continente de Priston, sendo capazes de controlar todos os tipos de equipamentos, at\351 mesmo os mais modernos.",

								" Conhecida pela sua tecnologia avan\347ada, a tribo dos Tempskrons \351 a \372nica capaz de usar m\341quinas modernas a favor do combate.",
								" Os Lutadores s\343o guerreiros natos, sua especialidade \351 a luta com Machados...",
								" Personagens eficientes, podem usar suas t\351cnicas a favor de uma luta de qualidade,.",
								" Mestres na arte da luta a dist\342ncia, s\343o as pioneiras com o Arco e Flecha." };



char *HoTextMoryon = "Morion";
char *HoTextMoryonInfo[5] = {
	"Os Morions residem na parte Norte do continente de Priston. S\343o ing\352nuos e d\363ceis. Sua cultura prospera. Valorizam ao extremo ambos os treinamentos - o espiritual e o f\355sico. S\343o os pioneiros na arte de usar as for\347as da magia e da ora\347\343o, a favor do combate.",
		"Com suas armaduras reluzentes e suas espadas poderosas, representam a classe Top dos Guerreiros dos Morions. Eles usam a for\347a sagrada para derrotar os inimigos do mal.",
	"Atalantas s\343o profissionais na arte das Lan\347as. Sua t\351cnica \351 perfeita quando o assunto \351 arremesso. Com seus poderes magn\355ficos, o estrago dos ataques \351 ainda maior.",
	"As Sacerdotisas s\343o as rainhas da magia, usando sua forca para combater o mal, seja atacando, seja curando. Mesmo com seu alto poder de cura, s\343o consideravelmente vulner\341veis ",
	"Magos convertem toda a for\347a da natureza em ataques brutais. Com conhecimento anormal acerca dos elementos, s\343o os melhores quando o assunto \351 Magia. Tamb\351m s\343o famosos por controlar a terra e as estrelas.",
};


char *HoText_PartyMessage = "enviou um pedido de grupo";

char *HoTextTempskron = "Tempskron";

char *HoTextFriendNear = "[Recentes]";
char *HoTextFriendList = "[Amigos] %d/%d";
char *HoTextFriendRefuse = "[Ignore] %d/%d";



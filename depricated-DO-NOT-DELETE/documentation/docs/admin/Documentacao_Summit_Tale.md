# Documentação Summit Tale

Conteúdo consolidado dos arquivos de referência.

## Índice

1. [Adicionar Atualização no launcher](#adicionar-atualização-no-launcher)
2. [adicionar GM](#adicionar-gm)
3. [Adicionar IP no Firewall](#adicionar-ip-no-firewall)
4. [adicionar iten no logado premiado](#adicionar-iten-no-logado-premiado)
5. [Codigos Shop](#codigos-shop)
6. [Comandos GM](#comandos-gm)
7. [@Divulgue SummitTale](#divulgue-summittale)
8. [@DV](#dv)
9. [Editar Drop de Mob e exp](#editar-drop-de-mob-e-exp)
10. [Editar iten](#editar-iten)
11. [Exemplo como adicionar iten no npc](#exemplo-como-adicionar-iten-no-npc)
12. [@FALTA FAZER](#falta-fazer)
13. [Guia-de-mapas](#guia-de-mapas)
14. [@INFO 1](#info-1)
15. [@INFO 2](#info-2)
16. [@ITENS](#itens)
17. [MAPAS](#mapas)
18. [@SKINS](#skins)
19. [@SPAWN BOSS E DROPS](#spawn-boss-e-drops)
20. [@SQL](#sql)
21. [@tutorial instalando iis](#tutorial-instalando-iis)

---

## Adicionar Atualização no launcher

Para adicionar atualização no launcher, é facil.


vamos abrir o IIS, (Disco c: Inetput, wwwroot)

Normalmente, lá o nome da pasta é Update

ou Launcher,

neste caso, está como launcherr,

Dentro dela,


tem 3 arquivos,

1 em .zip com a versão sendo ela no momento aqui 1000.zip

para atualizar,


Vamos muda o nome dela para 2000.zip e com oque você quer atualizar dentro,

entre na pasta onde tá o patch, chega até os arquivos, seleciona tudo

clicando com botão direito, dps em adicionar para o arquivo, ai vai ter a opção .zip la

só clica em ok ja vai ir pra .zip


depois de renomear como 2000 que é a versão que está aqui, vamos abrir o arquivo updates e muda a versão lá

estava 1000.zip aqui, mude as duas para 2000

   
<version>1000</version>
 
   
<file>1000.zip</file>
  

Mude para:

<version>2000</version>
 
   
<file>2000.zip</file>
 

Conforme a versão que você deixou no .zip

depois feito isso, salva,

agora vamos abrir o version.txt


e muda a versão lá tbm

aqui está:

1000


só coloca


2000 fecha e salva.

Depois disso, só abrir o launcher e testa, faça copia de tudo antes caso der errado, tente novamente, uma hora você consegue!

---

## adicionar GM

seguinte. para adicionar Game master.

basta abrir o Sql server managemente studio.

aperta em conectar.

expandi o banco de dados.

expandi accountdb

expandi tabelas

procura gamemaster.dbo

clica com botao direito encima e abre em 200 linhas superiores

Accountdb - id q voce quer add

password - senha para ativar a gm

security - você coloca 5 para Adm

ACtive - voce coloca True

Depois que adicionou a Id no sql.

vai até o cmd do servidor

e digita help;

depois reloadGMS;

depois so logar e /ativar (password - senha para ativar a gm)

---

## Adicionar IP no Firewall

Para adicionar ip no fire wall é simples.


Abre o firewall que esta na area de trabalho.

Depois clica em Firewall canto esquerdo encima.

Depois vai em More

Depois Clica em Policy

La vai ter 4 quadradinhos para adicionar ip.

adicionar la, não tem problema se for encima de outro, apenas o outro ip perde acesso a host.

Depois clica em freeips e aguarde 5 segundos.

Pronto, IP adicionado

---

## adicionar iten no logado premiado

abre o sql

expandi banco de dados

expandi LOGADO

expandi tabelias

e la vai tar a dbo.itemlist

clica com botao direito do mouse e abre essa dbo.item em 200 linhas superiores.

la vai ter 

code - numero do codigo do item

sem segredo, dps q add os itens que quer só fecha a db no x sem fecha o sql

---

## Codigos Shop

_______________ Shop Coins:


1 -> Armas de Ataque


2 -> Armaduras


3 -> Trajes


4 -> Acessórios


5 -> Sheltons


6 -> Itens Premium4


7 -> Serviços

_______________ Shop Time:


1 -> Ajuda no UP


2 -> Sheltons


3 -> Especiais


4 -> Trajes Festivos


5 -> Ajuda no Aging


6 -> Ajuda no MIX


7 -> Cristais

---

## Comandos GM

_______________________ COMANDOS: /@get
Comando pra criar iten: /drop
Comando pra estarta bc: /$gobc
pra finaliza bc: /endbc /$endbc /end_bc um dos 3 kkk

/A> --> falar de GM na Tela

/aging da303 22 4  --> criar iten com age.

/setcoinlogado --> quantidade de coins no logado premiado

/event_coin

/open_warehouse

/shutdownpk

/kill

/userid

/onlines

/rmonsters

/near

/username

/arena_monster

/agefreeOn

/sod_enter

/sod_view

/agefreeOff

/area_monster

/settimelogado_coin

/GlobalON

/modomanu

/GlobalOFF

/kick

/where

/area_users


/set_castle_tax

/reset_fountain_time

/get_fountain_info

/get_castle_info

/set_castle_master

/event_monster

/set_castle_time

/skills

/start_castle

/easy_castle

/settimelogado

/rates

/reload --> carrega config. novamente

/givemoney

/pegarpots

/setlevelpt

/avoid

/Bosstime

/setdelaychat

/hide /show

/criar

/@get

---

## @Divulgue SummitTale

-🎮Lançamento: 30/08/25 !!
-🔘https://summittale.com.br/
-🎖️Está a procura de um Servidor UP/prontinho para relembrar aquela nostalgia clássica 
pois bem, estamos trazendo um projeto diferenciado para vocês totalmente free!!!
-🎖️Traga seu clan para viver grandes batalhas e desafios!
-🎖️De fã para fã... INSPIRADO nos primeiros servidores private ( PT Pirata, FireUP PT, Endless PT). Rates Clássicas Hard Nostalgia, SEM ITEM SHOP!! NADA PAY TO WIN!! ABSOLUTAMENTE FULL DROPS!! UMA EQUIPE 24HORAS DANDO SUPORTE E APOIO AOS PLAYERS!! EVENTOS DIARIOS!! Vem ai o Summit TALE!

🎮Servidor Online
🎮Summit Tale - Uma nova aventura a sua espera🎮
🔘Level Inicial 1.
🔘Level Cap 120.
🔘Taxa de XP: 20x padrão bpt.
🔘Taxa de Drop: 3x padrão bpt.
🔘Linha de Itens até 118 "Mitológico".
🔘Mapa liberado até Mina de Gelo
🔘Up Via Mob Classico.
🔘Sistema PVP na letra K com bonus Exp e Drop.
🔘Resolução Full HD 1920x1080.
🔘Loja de coins in game.
🔘Loja de Minutos in-game.
🔘Classes Balanciadas PARA PVP.
🔘Alerta Boss ingame.
🔘Sod 100% Funcional e com 3 Coroas.
🔘Camera in-game com mais aproximação e distânciamento.
🔘Pedra da Perfeição, Deixando o item 100% perfeito.
🔘itens Premiuns não são contados na cidade, bc e deslogado.
🔘Coins liberada para passar por trade.
🔘Logado Premiado Sorteando tambem Coins.
🔘NPC para limpar seus itens mixado.
🔘Configuração ativa na tecla F11.
🔘Todos os itens são dropaveis.
🔘Sistema de Traje na tecla "T".
🔘Sistema de Chat global  /G>

-Venha Conhecer o Summit Tale
🔘Site: https://summittale.com.br/
🔘Disc: https://discord.gg/9JbQsDD74X
🔘Insta: https://www.instagram.com/summittalebr/
🔘Whats: https://chat.whatsapp.com/FPbAxvnr8bh3cWbvJbIsEc

---

## @DV

Lançamento: 01/08/25 !!

De fã para fã... INSPIRADO nos primeiros servidores private ( PT Pirata, FireUP PT, Endless PT). Rates Clássicas Hard Nostalgia, SEM ITEM SHOP!! NADA PAY TO WIN!! ABSOLUTAMENTE FULL DROPS!! UMA EQUIPE 24HORAS DANDO SUPORTE E APOIO AOS PLAYERS!! EVENTOS DIARIOS!! Vem ai o Summit TALE!
_____________ Summit Tale _____________

--> Level Inicial 1

--> Level Cap 120

--> Up por Mobs

--> Itens padrão BPT até 118 (Mitológico)

--> Taxa de XP: 20x padrão bpt

--> Taxa de Drop: 3x padrão bpt

--> Mapa liberado até Mina de Gelo

--> Sistema PVP na letra K com bonus Exp e Drop

--> Bless Castle 100%

--> Sod 100% Funcional e com Coroa

--> Camera in-game com mais aproximação e distânciamento

--> Pedra da Perfeição, Deixando o item 100% perfeito

--> itens Premiuns não são contados na cidade, bc e deslogado

--> Novas Poções Capilares

--> Diversos novos mobs/boss personalizados

--> NPC para limpar seus itens mixado

--> Loja de Minutos dentro do jogo

--> Loja de Creditos dentro do jogo

--> Nova Qualidade Gráfica

--> Nova Interface

--> Configuração ativa na tecla F11

--> Todos os itens são dropaveis

--> Camera com auto distanciamento (Um Angulo melhor)

--> Widescreen e SuperWidescreen com resolução até 21:9

--> Sistema de Traje na tecla "T"

--> Sistema de Chat global  /G>

--> Sistema de Rei do PVP com Efeito e bônus Exclusivo

--> Sistema de Alerta PVP

--> Sistema de Alerta Boss

--> Sistema de Visualizar o status perfeito de um item na tecla "P"

--> Sistema Booster FPS, Permitindo jogar sem lag algum

--> Sistema de Auto ajust, Maximize a janela e não perca a qualidade na resolução.

--> O Nosso Servidor Funciona em todos Windows

--> Para manter a camera parada, use o comando "/camera_wave_off".

--> Para Distribuir seus pontos de 10 em 10, Aperte a tecla "B" e Segure.

--> Para saber o horário do BOSS, use o comando "/get_bosstime" no chat.

--> se você estiver em modo janela e quiser deixar o jogo em modo full, não precisa relogar, basta aperta "F12" no seu teclado e o jogo muda automaticamente, Simples e rápido!

--> Mapa Oasis com PVP, drop de mana, celesto e goldbar 100k!

--> Ruinen, Iron 2 e Castelo Perdido com drop adicional de Mana Mistica!


Site: https://summittale.com.br/
Disc: https://discord.gg/9JbQsDD74X
Insta: https://www.instagram.com/summittalebr/
Whats: https://chat.whatsapp.com/FPbAxvnr8bh3cWbvJbIsEc

---

## Editar Drop de Mob e exp

Primeiramente, temos que saber qual mob é.

para sabemos qual mob é.

entra no jogo, vai no mapa, aperta f9

la em f9 vai mostrar o codigo do mapa encima no f9.

Depois, vai na server files

abre o pritator 1.8.857 que esta perto do server.exe na pasta da files

vai em options

depois server path e procura a pasta da server files que esta em disco c dois ponto servidor

seleciona  pasta servidor e clica em ok

depois vamos em maps

la esta a lista de mapas com mob

vamos pega um mapa aleatorio

vamos pegar jardim da liberdade.

fore-1.ase.spm

la vai ter o nome dos mob,

exemplo, 4.hopy - copy.inf

depois de pegamos o  nome do mob que agente quer edita,

vamos procurar ele.

vai na server files, gameserver, monster

procura o nome dele la, de exemplo vamos usa o 5.cuepy.inf.

depois de acha o mob, abre ele com o txt editor notepad==

para edita o drop, vamos descer tudo.

//???? ????? ??? ?? ??
*??????	1

*???	5000 ? 1800 2300
*???	1100 ps101 pl101 pl101 ps101 pl101
*???	400 da102 ws201 wa101 wc101 wh101 wp101 ws101 wt101 wd101 or103
*???	200 da103 wa102 wc102 wh102 wp102 ws102 ws202 wt102 wd102 gp101 ds101 db101 dg101 oa201
*???	100 os103 or103 or103 or101

*????	"name\Cuepy.zhoon"

la embaixo vai ter uma lista assim, 

*???	5000

5000 e a taxa de drop. os codigo que vem dps desse numero é os itens

*???	100 os103 or103 or103 or101

*??? comando de drop para adicionar no mapa

100 é a taxa

os103 or 103 or103 or101 é o codigo do iten.

só editar e salva. depois reinicia o server para consta.

---

## Editar iten

A Forma mais facil de edita iten

é ir na server files

e procura o programa que está com @ na frente

que tem nome de editor de itens, só abrir ele

e ser feliz,

---

## Exemplo como adicionar iten no npc

vamos até o pritator / carregar game server 

GameServer\npc\ricarden-store.NPC

ricarden-store

so adicionar e salva o arquivo

image/sinimage/ items  premiuns

acho que é ch101 alguma coisa assim dps se procura e add aqui

dai vc cria in-game e testa eles e add corretamente

pronto

---

## @FALTA FAZER

- AJUSTAR SPAWNS HS IRON 1 +
- DIMINUIR STATS DOS MOBS IRON 1+
- ATIVAR CHECKSUN
- MUDAR SENHA SQL
- DIMINUIR RATE EXP E TESTAR
- CONFIGURAR NOVO SITE E PAINEL

=====TESTAR=====
1 - FAZER NOVOS SPAWNS HS MAP SS+
2 - TESTAR ATK/DEF DOS MOBS IRON 1+
3 - TESTAR NOVAS QUESTS
5 - TESTAR DROPS IRON 1+


=====DBS A FAZER BKP DEPOIS DE PRONTO=====
ADM
BATTLEPASS
COINSHOP
JOBS
LOGADOPREMIADO
QUEST
SERVERDB

=====IDEIAS SOURCE FUTURAMENTE=====
ADD RARIDADE NOS ITENS
ADD AFFIX NOS ITENS
ADD SOCKET NOS ITENS
ADD TROCA DE NICK NO COINSHOP
ADD BOTAO PARA PROCURAR GRUPO
ADD DUNGEONS/CASTELOS ALGO DO TIPO

---

## Guia-de-mapas

_____________ BOSS KELVEZU

- Nascendo de 4 em 4h no covil de kelvezu


- dropando itens 115

_____________ Jardim da liberdade

- Nascendo 4_tobie


- dropando gold

_____________ Mobs Up 100-115 (	ok )


- 75_Hulk.inf


- 77_Dawlin.inf


- 85_Typhoon.inf

_____________ Mobs Up 115-130


- B_54_SkeletonKnight.inf


- B_59_HeadCutter 1.inf

_____________ mobs up 130-140


- 64_Mummy.inf

- 65_DarkKnight

_____________ mobs up 140-150

- 89_mountain.inf

- 74_stonegolem.inf

_____________ Arma Antiga


- AncientW.Ase.spm


- Lab_Flamo.inf


- Lab_LTechnician.inf


_____________ Ice Mine


- Mine-1.ase.spm


- minebat.inf


- minegolem.inf


- tarantulika.inf

---

## @INFO 1

_____________ Summit Tale _____________

Hardcore Rates Classico experiencia nostalgia, totalmente balanceado, anticheat poderoso e comunidade ativa

--> Level Inicial 1

--> Level Cap 120

--> Up por Mobs

--> Itens padrão BPT até 118 (Mitológico)

--> Taxa de XP: 20x padrão bpt

--> Taxa de Drop: 3x padrão bpt

--> Mapa liberado até Mina de Gelo

--> Sistema PVP na letra K com bonus Exp e Drop

--> Bless Castle 100%

--> Sod 100% Funcional e com Coroa

--> Camera in-game com mais aproximação e distânciamento

--> Pedra da Perfeição, Deixando o item 100% perfeito

--> itens Premiuns não são contados na cidade, bc e deslogado

--> Novas Poções Capilares

--> Diversos novos mobs/boss personalizados

--> NPC para limpar seus itens mixado

--> Loja de Minutos dentro do jogo

--> Loja de Creditos dentro do jogo

--> Nova Qualidade Gráfica

--> Nova Interface

--> Configuração ativa na tecla F11

--> Todos os itens são dropaveis

--> Camera com auto distanciamento (Um Angulo melhor)

--> Widescreen e SuperWidescreen com resolução até 21:9

--> Sistema de Traje na tecla "T"

--> Sistema de Chat global  /G>

--> Sistema de Rei do PVP com Efeito e bônus Exclusivo

--> Sistema de Alerta PVP

--> Sistema de Alerta Boss

--> Sistema de Visualizar o status perfeito de um item na tecla "P"

--> Sistema Booster FPS, Permitindo jogar sem lag algum

--> Sistema de Auto ajust, Maximize a janela e não perca a qualidade na resolução.

--> O Nosso Servidor Funciona em todos Windows

--> Para manter a camera parada, use o comando "/camera_wave_off".

--> Para Distribuir seus pontos de 10 em 10, Aperte a tecla "B" e Segure.

--> Para saber o horário do BOSS, use o comando "/get_bosstime" no chat.

--> se você estiver em modo janela e quiser deixar o jogo em modo full, não precisa relogar, basta aperta "F12" no seu teclado e o jogo muda automaticamente, Simples e rápido!

--> Mapa Oasis com PVP, drop de mana, celesto e goldbar 100k!

--> Ruinen, Iron 2 e Castelo Perdido com drop adicional de Mana Mistica!

---

## @INFO 2

SERVER ONLINE !! LANÇAMENTO !!
http://summittale.com.br/

De fã para fã... INSPIRADO nos primeiros servidores private ( PT Pirata, FireUP PT, Endless PT). Rates Clássicas Hard Nostalgia, SEM ITEM SHOP!! NADA PAY TO WIN!! ABSOLUTAMENTE FULL DROPS!! UMA EQUIPE 24HORAS DANDO SUPORTE E APOIO AOS PLAYERS!! EVENTOS DIARIOS!! Vem ai o Summit TALE!

-- > Level Inicial 1
-- > Level Maximo 120
-- > Itens BPT até 118 (Mitológico)
-- > Up Classico Nostalgia
-- > Modo PVP com buff exp/drop letra K
-- > Classico 8 Classes e Tier 4
-- > Sistema de Roleta em Boss
-- > Chat Global
-- > Sistema de Qeusts
-- > Traje Funcional Letra T
-- > Aging até +20 e 3 novos Mix
-- > Gold e Sheltons Valorizado
-- > Full drop sem pay2win
-- > Todos itens dropados ingame
-- > Coinshop e Timeshop ingame
-- > Mapa até Mina de Gelo
-- > Logado Premiado a cada 60 minutos dando Coins e itens Premiuns

Disc: https://discord.gg/9JbQsDD74X
Insta: https://www.instagram.com/summittalebr/
Whats: https://chat.whatsapp.com/FPbAxvnr8bh3cWbvJbIsEc

---

## @ITENS

CLIENTE\image\Sinimage\Items\
https://wiki.priston.com.br/index.php/Guia_dos_Itens_(Armas,_Defesas_e_Acess%C3%B3rios)

/quantidade pm104 12
/Levelup
/load nickdoplayer
/cquests
/event_monster Xetan 100 1000 1000
Codigo do evento - mob - tempo - quantidade de mob - quantidade de respaw
_______________________________

//Drop 80a
WA116 WC116 WH117 WM117 WP117 WS118 WS219 WT117 OA120 OA216 OM117 OR120 DA118 DA218 DB116 DG116 DS116

//Drop 80b
WA117 WC117 WH118 WM118 WP118 WS119 WS220 WT118 OA121 OA217 OM118 OR121 DA119 DA219 DB117 DG117 DS117

//Drop 80c
WA118 WC118 WH119 WM119 WP119 WS120 WS221 WT119 OA122 OA218 OM112 OR122 DA120 DA220 DB118 DG118 DS118

//Drop 80d
WA119 WC119 WH120 WM120 WP120 WS121 WS222 WT120 OA123 OA219 OM123 OR123 DA121 DA221 DB119 DG119 DS119

//Drop 90
OA220 DB120 DG120 DS120 WA120 WC120 WH121 WM121 WP121 WS122 WS223 WT121

//Drop 100
WA121 WC121 WH122 WM122 WP122 WS123 WS224 WT122 OA124 OA221 OM124 OR124 DA123 DA223 DB121 DG121 DS121

//Drop 102
WA122 WC122 WH123 WM123 WP123 WS124 WS225 WT123 OA124 OA222 OM125 OR124 DA124 DA224 DB122 DG122 DS122

//Drop 105
WA123 WC123 WH124 WM124 WP124 WS125 WS226 WT124 OA124 OA223 OM125 OR124 DA125 DA225 DB123 DG123 DS123

//Drop 108
DA127 DA227 DS124 DB124 DG124 OA224 OM125 WA124 WA125 WC124 WH125 WH126 WM125 WM126 WP125 WS126 WS227 WS228 WT125

//Drop 115
WA126 WC125 WH127 WM127 WP126 WS127 WS229 WT126 DA126 DA226

//Drop 118
DA128 DA228 DS125 DB125 DG125 OA225 OM125 WA127 WC126 WH128 WM128 WP127 WS128 WS230 WT127

//Drop Especiais
OR201 OS117 OS116 OS118 OR127 OA127 OA128 OE101 OE102 OE103

----------------DESATIVADO
CONJUNTO MORTAL			123 > 120	STATS BPT
DA129 DA229 DS126 DB126 DG126 OA226 OM125 OR127 OA127 OS115 WA128 WC127 WH129 WM129 WP128 WS129 WS231 WT128 OA128 OR201 OS117 OS116  oe101 oe102 oe103

CONJUNTO FLAMEJANTE/VERUS 	130 > 125	STATS BPT > FLAMEJANTE
DA302 DA402 DS127 DB127 DB128 DG127 DG128 OA227 OA228 OM125 OR201 OA128 OS116 WA129 WC128 WH130 WM130 WP129 WS130 WS232 WT129

CONJUNTO ABISSAL		 165 > 130	STATS BPT > VERUS
DA303 DA403 DS128 DB129 DG130 OA229 OM125 OR201 OA128 OS117 WA130 WC129 WH131 WM131 WP130 WS131 WS233 WT130
_______________________________
_________PREMIUNS /@get 
RESPEC - RS101 FS, RS102 MS,/@get RS103 3 PS,4 AS,5 KS,6 ATA,7 MG,8 SA
CX113 - Caixa 70 Aging 8
CX112 - CAIXA COM SHELTONS PARA AGE +20
CX102 - AGE +10, CX103,104,05,6,7,8,9,10,11
BI330 - PET 3 DIAS QUE ABRE BAU EM QUALQUER LUGAR
BI311-318 - HP/MP/RES BOOSTER
BI307/8/9 - COLETOR DE OURO 6/12/24H
BI302/3 - PACOTE UP 12/24H
BI301 - Caixa EndGame PACK OPEN BETA ITENS 100+ AGE
BI290-6 - ABRIR NPC
BI283 - RESSU PREMIUM 1H
/@get BI273 - AGE DIRETO +10
BI235/6/7 - BAU SUP 6/12/24H (SOMENTE EXP UP)
BI232/3/4 - BAU OURO 6/12/24H (SOMENTE EXP UP)
BI229/30/31 - BAU PRATA 6/12/24H (SOMENTE EXP UP)
BI226/7/8 - BAU BRONZE 6/12/24H (SOMENTE EXP UP)
BI223/4/5 - BAU SUPERIOR +DROP 6/12/24H
BI209/10/1/2 - BAU OURO +DROP 1/6/12/24H
BI205/6/7/8 - BAU PRATA +DROP 1/6/12/24H
BI201/2/3/4 - BAU BRONZE +DROP 1/6/12/24H
BI213-222 - CABELO D-M
BI193/4 - CORE ESTRANHO
/@get BI190 - PEDRA DA HONRA 100% PERF
BI189 - PEDRA DA LIMPEZA, REMOVE AGE MIX
BI186/7/8 - SUPER DRENA 1/3/24H
BI184 - PEDRA DA FORCA, AUMENTA 50 PESO, ATE 5x
BI180/150 - EXPUP 100% 1H/1D
BI179 - EXPUP 50% 6H
BI164 - AGING F MATURA BOTA
BI163 - AGING E MATURA LUVA
BI162 - AGING D MATURA BRACE
BI142 - ABRE FERREIRO
BI141 - MATURA 3 TIER SKILL
BI140 - MATURA 2 TIER SKILL
BI139 - MATURA 1 TIER SKILL
BI138 - AGING C ARMOR
BI137 - AGING B ESCUDO
BI136 - AGING A ARMA
BI110 - PEDRA AGE, DIMINUI A CHANCE DA FALHA
BI108 - CORE AZUL
BI104 - RESSU
gb101 - GOLD BAR 100K
gb104 - GOLD BAR 25KK
GP117 - CRISTAL ESQUELETO
GP124 - CRISTAL MIROTIC LV20
GP125 - CRISTAL WYVERN LV20

BI109 - CABECAO
BI178 - OLHO 50% 6H
BI112 - OLHO 50% 12H
BI113 - XP 30% 12H
BI114 - OLHO 50% 1D
BI115 - XP 30% 1D
BI119 - PRESA 12H
BI120 - PRESA 1D
BI121 - DRENA 12H
BI122 - DRENA 1D
BI124 - XP 50% 12H
BI125 - XP 50% 1D
BI132 - FENIX 12H
BI133 - FENIX 1D
BI134 - FENIX 6H
BI143 - XP 100% 6H
BI144 - XP 100% 12H
BI150 - XP 100% 1D
BI189 - PEDRA LIMPEZA
BI190 - PEDRA DA HONRA
BI302 - KIT UP 12H
BI303 - KIT UP 1D
BI307 - COLETOR DE GOLD 6H
BI308 - COLETOR DE GOLD 12H
BI309 - COLETOR DE GOLD 1D
BI310 - HP BOSTER 6H
BI311 - HP BOSTER 12H
BI312 - HP BOSTER 1D
BI313 - MP BOSTER 6H
BI314 - MP BOSTER 12H
BI315 - MP BOSTER 1D
BI316 - RES BOSTER 6H
BI317 - RES BOSTER 12H
BI318 - RES BOSTER 1D
BI330 - PET BAU 3D
BI331 - PET BAU 7D
GB101 - GOLD BAR 100K
GB104 - GOLD BAR 25KK
CX113 - KIT 70 +8
QW106 - ASA IMPERIAL
_______________________________
TRAJES
CA101 / CA714 - TRAJES
_______________________________
ARMOR
da122 - DRAGON (BUGANDO NAO FUNCIONA)
_______________________________
CRISTAL
GP101 - GP125
GP208 - CAOSCARA
GP205 - FURY
GP203 - KELVEZU
GP202 - VALENTO
_______________________________
ADM /@get /aging /quantidade pm104 12
wt133
ds142
oa242
db142
dg142
da150
da152
da184
fo137
os140
wm134
da284
da251
da249
oe101
oe102
oe103
or201
wc132
wc154
wh133
_______________________________

---

## MAPAS

field 50 arena de batalha			SOMENTE ADM POR ENQUANTO
field 48	abismo do mar - SEAA		entrar 195+	drop LVL190
field 47	arma secreta - ANCIENTW		entrar 180+	drop LVL175
field 46	labolatorio secreto - SLAB	entrar 165+	drop LVL160
field 45 TDP ARDOR (parece sod)			SOMENTE ADM POR ENQUANTO
field 44	mina de gelo - MINE1		entrar 150+	drop LVL145
field 43	et3 - DUN9			entrar 135+	drop LVL130
field 42 t.maldito 3andar - DUN6		entrar 175+	UP 175-200
field 41 	et2 - DUN8			entrar 120+	drop LVL115
field 40 	et1 - DUN7			entrar 110+	drop LVL105 +RATE
field 39 sei la q merda eh			SOMENTE ADM POR ENQUANTO
field 38 lost2 - LOSTTEMPLE			entrar 140+	UP 140-150
field 37 lost1 - LOSTISLAND			entrar 125+	UP 125-140
field 36 covil kelvezu - BOSS			DROP AMULETO BOSS KELVEZU
field 35 ice2			===========================================================
field 34 lago ganancia - GREEDY			DROP MANA
field 33 bc					BC
field 32 furya					QUEST
field 31 ice1			===========================================================
field 30 parece sod				SOD
field 29 eura			===========================================================
field 28 iron2 - IRON2				SHELTONS +RATE
field 27 iron1 - IRON1				SHELTONS
field 26 santuario sombrio	===========================================================
field 25 cav abelha		===========================================================
field 24 cav cogumelo		===========================================================
field 23 t.maldito 2andar - DUN5		entrar 160+	UP 160-175
field 22 t.maldito 1andar - DUN4		entrar 150+	UP 150-160
field 21 pilai					PILAI
field 20 estrada dos ventos	===========================================================
field 19 vale tranquilo		===========================================================
field 18 floresta das ilusoes	===========================================================
field 17 floresta dos espiritos	===========================================================
field 16 sala gm				SOMENTE ADM POR ENQUANTO
field 15 c.antigo 3andar	===========================================================
field 14 c.antigo 2andar	===========================================================
field 13 c.antigo 1andar	===========================================================
field 12 terra proibida		===========================================================
field 11 c. batalha dos ancioes	===========================================================
field 10 oasis - DE2				entrar 115+	UP 115-125
field 9 navisko			===========================================================
field 8 terra esquecida	- DE1			entrar 100+	UP 100-115 E DROP 105 HARD
field 7 terra maldita		===========================================================
field 6 ruinen			===========================================================
field 5 castelo perdido	- RUIN3			TULLA DROP BOSS
field 4 refugio anciao		===========================================================
field 3 ricartem				RICARTEM
field 2 jardim liberdade			DROP GOLD
field 1 floresta bamboo		===========================================================
field 0 mata das acacias	===========================================================

---

## @SKINS

skins

/@get SK
foices
SK242
SK245
SK248
SK243
SK244
SK246
SK247
SK249

machado 2mao
SK201
SK202
SK203
SK204
SK205

machado 1mao
SK101
SK102
SK103
SK106
SK117

espada 1mao
SK107
SK108
SK109
SK110

garra
SK111
SK112
SK113
SK114
SK115
SK116

lanca
SK172
SK173
SK174
SK175
SK176

escudo
SK181
SK182
SK183
SK184
SK185
SK186
SK187

martelo 2mao
SK221
SK222
SK223
SK224

cajado 2mao
SK231
SK232
SK233
SK234

arco
SK253
SK254
SK255
SK256
SK259

espada 2mao
SK265
SK266
SK267
SK268
SK269

---

## @SPAWN BOSS E DROPS

======= BOSS TIME SPAWN xx:32 MINUTOS =======

Castelo dos Perdidos / Drop 80-90 / MiniBosses e Boss Greedy = 22
Castelo dos Perdidos / Drop 80-90 / MiniBosses e Boss Draxos = 16
Estrada de Ferro do Caos / Drop Padrao BPT / Boss Babel = 1 4 7 10 13 16 19 22
Coração de Perum / Drop Padrao BPT / Boss Babel = 0 3 6 9 12 15 18 21
Vale Gallubia / Drop Padrao BPT / Boss Valento = 0 3 6 9 12 15 18 21
Vale Gallubia / Drop Padrao BPT / Boss Olho de Fogo = 1 5 9 13 17 21
Santuário Congelado / Drop Padrao BPT / Boss Tartaruga Dragao = 0 4 8 12 16 20
Covil do Kelvezu / Drop Padrao BPT / Boss Kelvezu = 0 4 8 12 16 20
Ilha Perdida / Drop Padrao BPT / Boss Cao Esqueleto = 2 6 10 14 18 22
Templo Perdido / Drop Padrao BPT / Boss Mokova = 2 7 12 17 22
Torre Sem Fim - 1º Andar / Drop 80-100-102 / Boss Mundo = 3 12 21
Torre Sem Fim - 2º Andar / Drop 80-100-102-105 / Boss Mundo = 8 16 23
Torre Sem Fim - 3º Andar / Drop 80-100-102-105-108 / Boss Devil Shy = 2 10 18
Mina de Gelo - 1º Andar / Drop 100-102-105-108-115-118 / Boss Tulla = 3 11 19
=============================================
Laboratório Secreto / Drop 120 / Boss Draxos = 4 12 20
Arma Antiga / Drop 125 / Boss Greedy = 2 10 18
Abismo do Mar / Drop 130-135 / Boss Yagditha = 3 11 19
=============================================

---

## @SQL

___________SQL
[Database]
Driver={SQL Server}
Host=PTL\SQLEXPRESS
User=sa
Password=#$pdl32xq

---

## @tutorial instalando iis

Hello,

 

OBS: Esse tutorial é apenas para instalar a extensão de PHP no IIS. Serve para o IIS 6.0 porém em alguns passos como o de autenticação o processo é diferente, porém a instalação do IIS é a mesma.

 

Vejo muitas pessoas usando Vertrigo ou Xampp na porta 8090, 90, 8080 para usar PHP e coloca o Clan na normal 80 no IIS ou vice vice-versa.


Esse tutorial você poderá juntar tudo no IIS e irá tornar tudo mais prático.

 

Instalando PHP

 

Após você instalar o IIS no seu Host, Local ou VM faça o Download do PHP!

 

- Download PHP 5.3.29 = https://windows.php.net/downloads/releases/archives/php-5.3.29-nts-win32-VC9-x86.msi

 

Passos
 

Instale o programa

Aceite os termos

Next

OBS: Escolha IIS FastCGI

Finish

 

Configurando
 

Arquivos de Programas e procure sua pasta PHP

Abra esse arquivo: PHP.ini

 

Agora preste atenção nas seguintes modificações que você terá que fazer! Mude para..

 

short_open_tag = On

allow_call_time_pass_reference = On

error_reporting = E_ERROR

 

Segurança para o PHP
 

No mesmo PHP.ini faça as as mudanças

 

allow_url_fopen = Off

display_errors = Off

magic_quotes_gpc = Off

allow_url_include = Off

expose_php = Off

register_globals = Off

 

Autentique seu IIS
 

Abra seu IIS

Sites

Default Web Site (Click com o Direito)

Editar Permissões

Conectar Como

Usuário Específico - Definir...

 

Usuário do Windows

Senha

Confirmar Senha

OK

OK

Testar Configurações (Vai aparecer os dois autenticado)

 

Abra o Gerenciador do IIS
Clique no site que você deseja editar
Clique duas vezes em Autenticação
Clique em Autenticação Anônima e clique em Editar ... em Ações
Altere para usar as credenciais de identidade do pool de aplicativos em vez do usuário específico (o meu era IUSR)

 

Reinicie o seu IIS!

#define SIN_SKILL_USE_RIGHT 0x01000000 // Available only for the right 
#define SIN_SKILL_USE_LEFT 0x02000000 // Available only for the left 
#define SIN_SKILL_USE_ALL 0x03000000 // Both available 
#define SIN_SKILL_USE_NOT 0x04000000 // Passive Skill

//PARA EDITAR DELAY DA SKILL DO CHAR É SÓ ALTERAR O QUARTO NºMERO EXEMPLO: 
//{"Escudo Extremo", "Aumenta o Bloqueio de Escudos, temporariamente",
//10, 34, 2, >>>>>>110<<<<<<<, 4, (110 será editado)
//{ 0,0,0 }, { sinDS1,0,0,0,0,0,0,0 },
//F_Extreme_Shield, SKILL_EXTREME_SHIELD, SIN_SKILL_USE_RIGHT, E_Shield_UseMana},

//Mechanician
{"Escudo Extremo", "Aumenta temporariamente o Bloqueio de Escudos.",
10, 34, 2, 110, 4,
{ 0,0,0 }, { sinDS1,0,0,0,0,0,0,0 },
F_Extreme_Shield, SKILL_EXTREME_SHIELD, SIN_SKILL_USE_RIGHT, E_Shield_UseMana},

{ "Bomba Mec\342nica",		"Joga uma granada para abater um grupo de inimigos.",
12, 28, 2, 73, 3,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Mechanic_Bomb,SKILL_MECHANIC_BOMB,SIN_SKILL_USE_ALL,Mechanic_Bomb_UseMana },

{ "Resist\352ncia a Veneno",		"Aumenta permanentemente a Resist\352ncia a Veneno.",
14, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Poison_Attribute,SKILL_POISON_ATTRIBUTE,SIN_SKILL_USE_NOT,0 },

{ "Absor\347\343o F\355sica",		"Aumenta temporariamente sua Absor\347\343o.",
17, 30, 2, 110, 5,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Physical_Absorption,SKILL_PHYSICAL_ABSORB,SIN_SKILL_USE_RIGHT,P_Absor_bUseMana },

{ "Grande Golpe",			"Carrega e desfere um golpe poderoso no inimigo.",
20, 38, 2, 73, 3,
{0,0,0}, {sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Great_Smash,SKILL_GREAT_SMASH,SIN_SKILL_USE_ALL,G_Smash_UseMana },

{ "Maximizar",			"Aumenta temporariamente o Poder de Ataque M\341ximo da arma equipada.",
23, 41, 2, 110, 4,
{0,0,0}, {sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Maximize,SKILL_MAXIMIZE,SIN_SKILL_USE_RIGHT,Maximize_UseMana },

{ "Automa\347\343o",			"Aperfei\347oa o uso de armas de longo alcance aumentando a Velocidade da Arma e o Poder de Ataque.",
26, 42, 2, 110, 4,
{0,0,0}, {sinWS1,sinWT1,0,0,0,0,0,0},
F_Automation,SKILL_AUTOMATION,SIN_SKILL_USE_RIGHT,Automation_UseMana },

{ "Spark",			"Ataca os inimigos utilizando esferas de energia concentrada.",
30, 40, 2, 82, 2,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Spark,SKILL_SPARK,SIN_SKILL_USE_ALL,Spark_UseMana },

{ "Armadura Met\341lica",		"Aumenta a Defesa da Armadura, se esta for Especializada para Mec\342nico.",
40, 48, 2, 110, 5,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Metal_Armor,SKILL_METAL_ARMOR,SIN_SKILL_USE_RIGHT,Metal_Armor_UseMana },

{ "Golpe Grandioso",	"Desfere dois poderosos golpes com Poder de Ataque e Taxa de Ataque adicionais.",
43, 44, 2, 82, 2,
{0,0,0}, {sinWA1,sinWC1,sinWP1,sinWS2, sinWH1,0,0,0},
F_Grand_Smash,SKILL_GRAND_SMASH,SIN_SKILL_USE_ALL,Grand_Smash_UseMana },

{ "Mestre dos Mec\342nicos",	"Aumenta o Poder de Ataque de todas as armas Especializadas para Mec\342nico.",
46, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_M_W_Mastery,SKILL_MECHANIC_WEAPON,SIN_SKILL_USE_NOT,0 },

{ "Escudo Spark",	    "Utiliza a habilidade Spark para encantar seu escudo. Todos os ataques bloqueados refletem para o atacante.",
50, 52, 2, 110, 2,
{0,0,0}, {sinDS1,0,0,0,0,0,0,0},
F_Spark_Shield,SKILL_SPARK_SHIELD,SIN_SKILL_USE_RIGHT,Spark_Shield_UseMana },


{ "Impulso","Ataque duplo que libera uma forte descarga el\351trica.",
60, 55, 2, 0, 0,
{1,0,0},{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Impulsion,SKILL_IMPULSION,SIN_SKILL_USE_ALL,Impulsion_UseMana },

{ "Implos\343o","Implos\343o que atrai inimigos e aumenta temporariamente a Absor\347\343o.",
63, 65, 2, 440, 0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Compulsion,SKILL_COMPULSION,SIN_SKILL_USE_RIGHT,Compulsion_UseMana },

{ "Esferas Magn\351ticas","Cria esferas magn\351ticas para atacar automaticamente os inimigos mais pr\363ximos.",
66, 71, 2, 600, 0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Magnetic_Sphere,SKILL_MAGNETIC_SPHERE,SIN_SKILL_USE_RIGHT,Magnetic_Sphere_UseMana },

{ "Golem de Metal","Invoca um Golem de Metal para auxiliar nas batalhas.",
70, 71, 2, 1000, 0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Metal_Golem,SKILL_METAL_GOLEM,SIN_SKILL_USE_RIGHT,Metal_Golem_UseMana },


//Fighter 
{ "Mestre das Armas",		"Aumenta permanentemente a efici\352ncia de Machados, Martelos e Espadas.",
10, 0,0, 0,0,
{0,0,0}, {sinWA1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Melee_Mastery,SKILL_MELEE_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Resist\352ncia a Fogo",		"Aumenta permanentemente a Resist\352ncia a Fogo.",
12, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Fire_Attribute,SKILL_FIRE_ATTRIBUTE,SIN_SKILL_USE_NOT,0 },

{ "Raiva",			"Fica nervoso e mais letal, mas perde Vida com o tempo.",
14, 35, 2, 93, 3,
{0,0,0}, {sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Raving,SKILL_RAVING,SIN_SKILL_USE_ALL,Raving_UseMana },

{ "Impacto",			"Desfere golpes com Poder de Ataque e Taxa de Ataque adicionais.",
17, 37, 2, 86, 4,
{0,0,0}, {sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Impact,SKILL_IMPACT,SIN_SKILL_USE_ALL,Impact_UseMana },

{ "Impacto Triplo",		"Vers\343o melhorada do Impacto.",
20, 39, 2, 80, 4,
{0,0,0}, {sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Triple_Impact,SKILL_TRIPLE_IMPACT,SIN_SKILL_USE_ALL,T_Impact_UseMana },

{ "Girada Brutal",		"Desfere um poderoso golpe com Cr\355tico adicional.",
23, 41, 2, 82, 2,
{0,0,0}, {sinWA1,sinWS2,0,0,0,0,0,0},
F_Brutal_Swing,SKILL_BRUTAL_SWING,SIN_SKILL_USE_ALL,B_Swing_UseMana },

{ "Rugido",			"Atordoa temporariamente os inimigos.",
26, 39, 2, 112, 4,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Roar,SKILL_ROAR,SIN_SKILL_USE_RIGHT,Roar_UseMana },

{ "F\372ria de Zecram",		"Ataque poderoso que acerta todos os inimigos ao redor do alvo.",
30, 45, 2, 84, 4,
{0,0,0}, {sinWA1,sinWH1,sinWS2,0,0,0,0,0},
F_RageOfZecram,SKILL_RAGE_OF_ZECRAM,SIN_SKILL_USE_ALL,R_Zecram_UseMana },

{ "Concentra\347\343o",	"Aumenta sua Taxa de Ataque temporariamente.",
40, 53, 2, 110, 3,
{0,0,0}, {sinWA1,sinWS2,0,0,0,0,0,0},
F_Concentration,SKILL_CONCENTRATION,SIN_SKILL_USE_RIGHT,Concentration_UseMana },

{ "Golpe da Vingan\347a",	"Ataque poderoso com adicional de Poder de Ataque e Taxa de Ataque.",
43, 48, 2, 73, 3,
{0,0,0}, {sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Avanging_Crash,SKILL_AVANGING_CRASH,SIN_SKILL_USE_ALL,A_Crash_UseMana },

{ "Machado Veloz",	"Aumenta a Velocidade de Machados temporariamente.",
46, 53, 2, 200, 3,
{0,0,0}, {sinWA1,0,0,0,0,0,0,0},
F_Swift_Axe,SKILL_SWIFT_AXE,SIN_SKILL_USE_RIGHT,Swift_Axe_UseMana },

{ "Quebra Ossos",		"Ataque poderoso com B\364nus contra Dem\364nios.",
50, 54, 2, 73, 3,
{0,0,0}, {sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Bone_Crash,SKILL_BONE_CRASH,SIN_SKILL_USE_ALL,B_Crash_UseMana },


{ "Destroyer",		"Ataque destrutivo que acerta tr\352s vezes o inimigo.",
60, 45, 2, 150, 0,
{1,0,0}, {sinWA1,sinWH1,0,0,0,0,0,0},
F_Destoryer,SKILL_DETORYER,SIN_SKILL_USE_ALL,Destoryer_UseMana },

{ "F\372ria","Sacrifica Absor\347\343o para aumentar o Poder de Ataque.",
63, 60, 2, 700, 0,
{1,0,0}, {0,0,0,0,0,0,0,0},
F_Berserker,SKILL_BERSERKER,SIN_SKILL_USE_RIGHT,Berserker_UseMana },

{ "Golpe Girat\363rio","Golpe m\372ltiplo e girat\363rio letal a grupos de inimigos.",
66, 55, 2, 0, 0,
{1,0,0}, {sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Cyclone_Strike,SKILL_CYCLONE_STRIKE,SIN_SKILL_USE_ALL,Cyclone_Strike_UseMana },

{ "B\364nus de Vitalidade","Aumenta permanentemente sua Vida.",
70, 0, 0, 0, 0,
{1,0,0}, {0,0,0,0,0,0,0,0},
F_Boost_Health,SKILL_BOOST_HEALTH,SIN_SKILL_USE_NOT, },

//Pikeman
{ "Sopro Afiado",			"Cria um vento circular para afastar os inimigos.",
10, 27, 2, 80, 4,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Pike_Wind,SKILL_PIKE_WIND,SIN_SKILL_USE_RIGHT,Pike_Wind_UseMana },

{ "Resist\352ncia a Gelo",		"Aumenta permanentemente a sua Resist\352ncia a Gelo.",
12, 0, 0, 0, 0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Ice_Attribute,SKILL_ICE_ATTRIBUTE,SIN_SKILL_USE_NOT,0 },

{ "Ataque Cr\355tico",		"Acerta os pontos fracos dos inimigos para aumentar a chance de Cr\355ticos.",
14, 31, 2, 84, 3,
{0,0,0}, {sinWP1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Critical_Hit,SKILL_CRITICAL_HIT,SIN_SKILL_USE_ALL,Critical_Hit_UseMana },

{ "Pulo Fatal",		"Ataque a\351reo com grandes chances de matar o oponente.",
17, 34, 2, 76, 4,
{0,0,0}, {sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Jumping_Crash,SKILL_JUMPING_CRASH,SIN_SKILL_USE_ALL,Jumping_Crash_UseMana },

{ "Espinhos de Gelo",		"Espinhos de gelo surgem e paralisam os inimigos ao seu redor.",
20, 36, 2, 1000, 4,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Ground_Pike,SKILL_GROUND_PIKE,SIN_SKILL_USE_RIGHT,G_Pike_UseMana },

{ "Tornado",			"Gira sua foice acertando todos os inimigos no caminho.",
23, 38, 2, 83, 4,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Tornado_Pike,SKILL_TORNADO,SIN_SKILL_USE_ALL,Tornado_UseMana },

{ "Mestre do Bloqueio",		"Aumenta a chance de Bloqueio com armas de duas m\343os.",
26, 0, 0, 0, 0,
{0,0,0}, {sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Weapone_Defence_Mastery,SKILL_WEAPONE_DEFENCE_MASTERY,SIN_SKILL_USE_NOT,0 },


{ "Expans\343o",			"Aumenta o tamanho da arma, elevando o Poder de Ataque dela.",
30, 42, 2, 82, 4,
{0,0,0}, {sinWA1,sinWH1,sinWP1,0,0,0,0,0},
F_Expansion,SKILL_EXPANSION,SIN_SKILL_USE_ALL,Expansion_UseMana },

{ "Foices Venenosas",	"Envenena os inimigos ao seu redor temporariamente.",
40, 50, 2, 110, 2,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Venom_Spear,SKILL_VENOM_SPEAR,SIN_SKILL_USE_RIGHT,VenomSpear_UseMana },

{ "Desaparecer",			"Fica invis\355vel temporariamente, n\343o sendo percebido pelos monstros. O primeiro ataque ap\363s o uso ser\341 mais forte.",
43, 53, 2, 140, 2,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Vanish,SKILL_VANISH,SIN_SKILL_USE_RIGHT,Vanish_UseMana },

{ "Mestre do Ataque Cr\355tico",		"Aumenta permanentemente a chance de Cr\355tico ao utilizar Foices, al\351m de aumentar o Poder de Ataque.",
46, 0, 0, 0, 0,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Critical_Mastery,SKILL_CRITICAL_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Foice de A\347o",	  		  "Ataque potente que desfere tr\352s golpes no inimigo.",
50, 51, 2, 88, 3,
{0,0,0}, {sinWC1,sinWP1,sinWS2,0,0,0,0,0},
F_Chain_Lance,SKILL_CHAIN_LANCE,SIN_SKILL_USE_ALL,Chain_Lance_UseMana },


{ "Olho Assassino",	 "Procura a fraqueza do inimigo para aumentar a chance de Cr\355tico.",
60, 61, 2, 520, 0,
{1,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Assassin_Eye,SKILL_ASSASSIN_EYE,SIN_SKILL_USE_ALL,Assassin_Eye_UseMana },

{ "Golpe Carregado",	 "Concentra-se para desferir golpes letais contra o inimigo.",
63, 75, 2, 150, 0,
{1,0,0}, {sinWP1,sinWS2,0,0,0,0,0,0},
F_Charging_Strike,SKILL_CHARGING_STRIKE,SIN_SKILL_USE_ALL,Charging_Strike_UseMana },

{ "Furtivo",	"Aumenta a Esquiva temporariamente.",
66, 71, 2, 650, 0,
{1,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Vague,SKILL_VAGUE,SIN_SKILL_USE_RIGHT,Vague_UseMana },

{ "Mestre das Sombras",	"Ataques m\372ltiplos de sombras misteriosas a fim de destruir o inimigo.",
70, 51, 2, 160, 0,
{1,0,0}, {sinWP1,sinWS2,0,0,0,0,0,0},
F_Shadow_Master,SKILL_SHADOW_MASTER,SIN_SKILL_USE_ALL,Shadow_Master_UseMana },


//Archer
{ "Falc\343o Vigia",			"Invoca um falc\343o para vigiar a \341rea a fim de aumentar sua Taxa de Ataque.",
10, 25, 2, 115, 5,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Scout_Hawk ,SKILL_SCOUT_HAWK,SIN_SKILL_USE_RIGHT,Scout_Hawk_UseMana },

{ "Mestra do Tiro",		"Aumenta permanentemente seu Poder de Ataque com Arcos.",
12, 0, 0, 0, 0,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Shooting_Mastery,SKILL_SHOOTING_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Flecha de Vento",			"Dispara uma flecha de vento com Velocidade adicional.",
14, 25, 2, 82, 3,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Wind_Arrow,SKILL_WIND_ARROW,SIN_SKILL_USE_ALL,Wind_Arrow_UseMana },

{ "Mira Perfeita",			"Um tiro perfeito. Tem aumento de Poder de Ataque e Taxa de Ataque.",
17, 29, 2, 81, 3,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Perfect_Aim,SKILL_PERFECT_AIM,SIN_SKILL_USE_ALL,Perfect_Aim_UseMana },

{ "Olho de Dion",			"Treina a pontaria, aumentando permanentemente sua Taxa de Ataque.",
20, 0, 0, 0, 0,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Dions_Eye,SKILL_DIONS_EYE,SIN_SKILL_USE_NOT,0 },

{ "Falc\343o",			"Invoca um falc\343o para atacar seus inimigos.",
23, 36, 2, 115, 4,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Falcon ,SKILL_FALCON,SIN_SKILL_USE_RIGHT,Falcon_UseMana },

{ "Flecha da F\372ria",		"Rajada fatal que explode e fere um grupo de inimigos.",
26, 38, 2, 82, 4,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Arrow_of_Rage,SKILL_ARROW_OF_RAGE,SIN_SKILL_USE_ALL,A_Rage_UseMana },

{ "Avalanche",			"Lan\347a uma sequencia de flechas perfuradoras para destruir os inimigos.",
30, 41, 2, 103, 2,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Avalanche,SKILL_AVALANCHE,SIN_SKILL_USE_ALL,Avalanche_UseMana },

{ "Tiro Elemental",		"Dispara flechas elementais contra o inimigo.",
40, 43, 2, 82, 2,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_E_Shot,SKILL_ELEMENTAL_SHOT,SIN_SKILL_USE_ALL,Elemental_Shot_UseMana },

{ "Falc\343o Dourado",		"Invoca um falc\343o dourado que ataca os inimigos e aumenta sua Regenera\347\343o de Vida.",
43, 52,2, 80,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_G_Falcon,SKILL_GOLDEN_FALCON,SIN_SKILL_USE_RIGHT,Golden_Falcon_UseMana },

{ "Tiro Explosivo",		    "Atira flechas explosivas para causar dano em \341rea.",
46, 46, 2, 87, 3,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_B_Shot,SKILL_BOMB_SHOT,SIN_SKILL_USE_ALL,BombShot_UseMana },

{ "Perfura\347\343o",		"Ataque preciso e cont\355nuo, que atravessa os inimigos.",
50, 48, 2, 82, 2,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Perforation,SKILL_PERFORATION,SIN_SKILL_USE_ALL,Perforation_UseMana },


{ "Invocar Wolverine",		"Invoca um Wolverine para auxiliar nas batalhas.",
60, 58, 2, 900, 0,
{1,0,0}, {0,0,0,0,0,0,0,0},
F_Recall_Wolverin,SKILL_RECALL_WOLVERIN,SIN_SKILL_USE_RIGHT,Recall_Wolverin_UseMana },

{ "Mestra da Esquiva",	"Aumenta permanentemente a Esquiva.",
63, 0, 0, 0, 0,
{1,0,0}, {0,0,0,0,0,0,0,0},
F_Evasion_Mastery,SKILL_EVASION_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Tiro F\352nix",	"Ataque carregado com a for\347a da F\352nix que atinge v\341rios inimigos ou concentra o poder em apenas um.",
66, 55, 2, 100, 0,
{1,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Phoenix_Shot,SKILL_PHOENIX_SHOT,SIN_SKILL_USE_ALL,Phoenix_Shot_UseMana },

{ "For\347a da Natureza","Aumenta seus poderes pela for\347a da natureza.",
70, 56, 2, 900, 0,
{1,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Force_Of_Nature,SKILL_FORCE_OF_NATURE,SIN_SKILL_USE_RIGHT,Force_Of_Nature_UseMana },


/*----------------------------------------------------------------------------*
*   Morion
*-----------------------------------------------------------------------------*/
// Knight
{ "Rajada Sagrada",		"Cria uma rajada de energia de sua espada.",
10, 22, 2, 82, 2,
{0,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_Sword_Blast,SKILL_SWORD_BLAST,SIN_SKILL_USE_ALL,Sword_Blast_UseMana },

{ "Corpo Sagrado",			"Recebe a b\352n\347\343o divina, aumentando a Absor\347\343o contra Mortos-Vivos.",
12, 28, 2, 110, 3,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_HolyBody ,SKILL_HOLY_BODY,SIN_SKILL_USE_RIGHT,HolyBody_UseMana },

{ "Treinamento F\355sico",		"Realiza treinamento f\355sico, aumentando permanentemente a Resist\352ncia do personagem.",
14, 0, 0, 0, 0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Physical_Traning,SKILL_PHYSICAL_TRANING,SIN_SKILL_USE_NOT,0 },

{ "Golpe Duplo",		"Ataque duplo, veloz e letal.",
17, 35, 2, 82, 2,
{0,0,0}, {sinWA1,sinWC1,sinWS2,0,0,0,0,0},
F_Double_Crash,SKILL_DOUBLE_CRASH,SIN_SKILL_USE_ALL,D_Crash_UseMana },

{ "Valor Sagrado",			"Aumenta temporariamente o seu Poder de Ataque e o do grupo contra Mortos-Vivos.",
20, 36, 2, 100, 3,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Holy_Valor,SKILL_HOLY_VALOR,SIN_SKILL_USE_RIGHT,Holy_Valor_Mana },

{ "Brandish",			"Ataque r\341pido e poderoso em \341rea.",
23, 38, 2, 82, 2,
{0,0,0}, {sinWP1,sinWS2,0,0,0,0,0,0},
F_Brandish,SKILL_BRANDISH,SIN_SKILL_USE_ALL,Brandish_Mana },

{ "Piercing",			"Carrega sua espada, desferindo um ataque que atravessa o inimigo.",
26, 41, 2, 82, 2,
{0,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_Piercing,SKILL_PIERCING,SIN_SKILL_USE_ALL,Piercing_Mana },

{ "Esp\355rito Dr\341stico",		"Aumenta a Defesa temporariamente.",
30, 44, 2, 100, 2,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Drastic_Spirit,SKILL_DRASTIC_SPIRIT,SIN_SKILL_USE_RIGHT,Drastic_Spirit_Mana },

{ "Mestre da Espada",	"Treina o uso de espadas aumentando permanentemente o Poder de Ataque.",
40, 0, 0, 0, 0,
{0,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_S_Mastery,SKILL_SWORD_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Escudo Divino",		"Bloqueia o ataque dos Mortos-Vivos, convertendo-os em Vida.",
43, 46, 2, 130, 1,
{0,0,0}, {sinDS1,0,0,0,0,0,0,0},
F_D_Inhalation,SKILL_DIVINE_INHALATION,SIN_SKILL_USE_RIGHT,D_Inhalation_UseMana },

{ "Encantamento Sagrado",	"Encanto sagrado que converte os Mrtos-Vivos em aliados.",
46, 50, 2, 120, 1,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Holy_Incantation,SKILL_HOLY_INCANTATION,SIN_SKILL_USE_ALL,H_Incantation_UseMana },

{ "Grande Cruzada",	"Ataque duplo, poderoso, veloz e cont\355nuo.",
50, 45, 2, 82, 2,
{0,0,0}, {sinWH1,sinWS2,0,0,0,0,0,0},
F_Grand_Cross,SKILL_GRAND_CROSS,SIN_SKILL_USE_ALL,Grand_Cross_UseMana },


{ "Piercing Divino",	"Utilizando a for\347a divina, desfere uma sequ\352ncia letal de golpes.",
70, 51, 2, 160, 0,
{1,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_Divine_Piercing,SKILL_DIVINE_PIERCING,SIN_SKILL_USE_ALL,Divine_Piercing_UseMana },

{ "Escudo dos Deuses",	"Cria um escudo divino que garante prote\347\343o adicional ao Cavaleiro.",
63, 50, 2, 400, 0,
{1,0,0}, {sinDS1,0,0,0,0,0,0,0},
F_Godly_Shield,SKILL_GODLY_SHIELD,SIN_SKILL_USE_RIGHT,Godly_Shield_UseMana },

{ "B\352n\347\343o Divina",	"Invoca a for\347a divina, aumentando temporariamente o Poder de Ataque.",
66, 51, 2, 330, 0,
{1,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_God_Bless,SKILL_GOD_BLESS,SIN_SKILL_USE_RIGHT,God_Bless_UseMana },

{ "Espada da Justi\347a","Poderoso ataque em \341rea.",
60, 53, 2, 250, 0,
{1,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_Sword_Of_Justice,SKILL_SWORD_OF_JUSTICE,SIN_SKILL_USE_ALL,Sword_Of_Justice_UseMana },


// Atalanta
{ "Golpe do Escudo",		"Atordoa temporariamente os inimigos.",
10, 23, 2, 82, 2,
{0,0,0}, {sinDS1,0,0,0,0,0,0,0},
F_S_Strike ,SKILL_SHIELD_STRIKE,SIN_SKILL_USE_ALL,S_Strike_UseMana },

{ "Farina",			"Desfere ataques com b\364nus na Taxa de Ataque.",
12, 26, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Farina,SKILL_FARINA,SIN_SKILL_USE_ALL,Farina_UseMana },

{ "Mestra do Arremesso",		"Aumenta seu Poder de Ataque com Lan\347as permanentemente.",
14, 0, 0, 0, 0,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_D_Mastery,SKILL_THROWING_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Lan\347a Gigante",		"Ataca os inimigos com a for\347a divina.",
17, 32, 2, 82, 2,
{0,0,0}, {sinWP1,sinWT1,0,0,0,0,0,0},
F_V_Spear,SKILL_VIGOR_SPEAR,SIN_SKILL_USE_ALL,V_Spear_UseMana },

{ "Windy",			"Adiciona a propriedade Vento na arma para arremessar mais longe e com maior precis\343o.",
20, 34, 2, 100, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Windy,SKILL_WINDY,SIN_SKILL_USE_RIGHT,Windy_Mana },

{ "Lan\347a Girat\363ria",		"Aumenta o Poder de Ataque e a Taxa de Ataque em cada acerto.",
23, 36, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Twist_Javelin,SKILL_TWIST_JAVELIN,SIN_SKILL_USE_ALL,Twist_Javelin_Mana },

{ "Ladr\343o de Almas",			"Absorve a Vida dos inimigos.",
26, 42, 2, 110, 2,
{0,0,0}, {sinDS1,0,0,0,0,0,0,0},
F_Soul_Sucker,SKILL_SOUL_SUCKER,SIN_SKILL_USE_RIGHT,Soul_Sucker_Mana },

{ "Lan\347a de Fogo",		"Adiciona a propriedade Fogo nos seus ataques.",
30, 40, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Fire_Javelin,SKILL_FIRE_JAVELIN,SIN_SKILL_USE_ALL,Fire_Javelin_Mana },

{ "Lan\347a Partida",	"Ataques cont\355nuos e r\341pidos.",
40, 43, 2, 115, 1,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Split_Javelin,SKILL_SPLIT_JAVELIN,SIN_SKILL_USE_ALL,Split_Javelin_UseMana },

{ "Triunfo de Valhalla",	"Aumenta o Poder de Ataque temporariamente.",
43, 50, 2, 120, 1,
{0,0,0}, {sinWT1,sinWS2,sinWS1,sinWP1,sinWH1,sinWA1,sinWC1,sinWM1},
F_T_Of_Valhalla,SKILL_TRIUMPH_OF_VALHALLA,SIN_SKILL_USE_RIGHT,T_Of_Valhalla_UseMana },

{ "Lan\347a de Raios",	"Adiciona dano elemental \340s Lan\347as.",
46, 46, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Lightning_Javelin,SKILL_LIGHTNING_JAVELIN,SIN_SKILL_USE_ALL,Lightning_Javelin_UseMana },

{ "Chuva de Lan\347as",	    "Arremessa lan\347as como devastadores espirais que destroem tudo em seu caminho.",
50, 48, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Storm_Javelin,SKILL_STORM_JAVELIN,SIN_SKILL_USE_ALL,Storm_Javelin_UseMana },


{ "Inferno de Valhalla",	"Aumenta temporariamente a Esquiva por meio do uso do poder m\341ximo de Valhalla.",
60, 52, 2, 750, 0,
{1,0,0}, {sinWA1,sinWC1,sinWH1,sinWP1,sinWS1,sinWT1,0,0},
F_Hall_Of_Valhalla,SKILL_HALL_OF_VALHALLA,SIN_SKILL_USE_RIGHT,Hall_Of_Valhalla_UseMana },

{ "F\372ria-X",	"Pula no ar e atira 3 lan\347as explosivas ferindo todos os inimigos na \341rea.",
63, 48, 2, 0, 0,
{1,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_X_Rage,SKILL_X_RAGE,SIN_SKILL_USE_ALL,X_Rage_UseMana },

{ "Lan\347a de Gelo",	"Adiciona a propriedade Gelo nos seus ataques.",
66, 50, 2, 720, 0,
{1,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Frost_Javelin,SKILL_FROST_JAVELIN,SIN_SKILL_USE_RIGHT,Frost_Javelin_UseMana },

{ "Vingan\347a",	"Ataque r\341pido, m\372ltiplo e letal.",
70, 55, 2, 100, 0,
{1,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Vengeance,SKILL_VENGEANCE,SIN_SKILL_USE_ALL,Vengeance_UseMana },

// Priestess
{ "Cura",			 "Com poder divino, recupera sua Vida ou a Vida de aliados.",
10, 15, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Healing,SKILL_HEALING,SIN_SKILL_USE_RIGHT,Healing_UseMana },

{ "Fagulha Sagrada",			"Fa\355sca m\341gica que fere os inimigos.",
12, 11, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_HolyBolt,SKILL_HOLY_BOLT,SIN_SKILL_USE_ALL,HolyBolt_UseMana },

{ "Espinhos M\372ltiplos",			"Dispara um ataque m\372ltiplo concentrado em um \372nico alvo.",
14, 14, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_M_Spark,SKILL_MULTISPARK,SIN_SKILL_USE_ALL,M_Spark_UseMana },

{ "Feiti\347o Divino",			"Encanta os inimigos, reduzindo temporariamente o Poder de Ataque.",
17, 19, 1, 100, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_HolyMind,SKILL_HOLY_MIND,SIN_SKILL_USE_ALL,HolyMind_UseMana },

{ "Medita\347\343o",			"Aumenta a Regenera\347\343o de Mana permanentemente, al\351m de aumentar o Poder de Ataque.",
20, 0, 0, 0, 0,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Meditation,SKILL_MEDITATION,SIN_SKILL_USE_NOT,0 },

{ "Raios Divinos",		"Invoca uma chuva de raios divinos, que caem das nuvens atingindo os inimigos ao seu redor.",
23, 21, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Divine_Lightning,SKILL_DIVINE_LIGHTNING,SIN_SKILL_USE_RIGHT,Divine_Lightning_Mana },

{ "Reflex\343o Divina",		"Cria um escudo m\341gico que reflete o dano sofrido por Mortos-Vivos.",
26, 23, 1, 120, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Holy_Reflection,SKILL_HOLY_REFLECTION,SIN_SKILL_USE_RIGHT,Holy_Reflection_Mana },

{ "Cura M\341xima",		"Recupera a Vida de todos no grupo.",
30, 27, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Grand_Healing,SKILL_GRAND_HEALING,SIN_SKILL_USE_RIGHT,Grand_Healing_Mana },

{ "Esferas de Vigor",			"Dispara duas esferas luminosas letais contra os inimigos.",
40, 24, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Vigor_Ball,SKILL_VIGOR_BALL,SIN_SKILL_USE_ALL,Vigor_Ball_UseMana },

{ "Ressurrei\347\343o",		"Permite trazer um membro do grupo de volta \340 vida.",
43, 31, 1, 130, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Resurrection,SKILL_RESURRECTION,SIN_SKILL_USE_RIGHT,Resurrection_UseMana },

{ "Extin\347\343o",		"Lan\347a um feiti\347o que elimina parte da Vida dos Mortos-Vivos.",
46, 28, 1, 120, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Extinction,SKILL_EXTINCTION,SIN_SKILL_USE_RIGHT,Extinction_UseMana },

{ "Vida Virtual",	"Aumenta a sua Vida ou a Vida do alvo temporariamente.",
50, 31, 1, 50, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Virtual_Life,SKILL_VIRTUAL_LIFE,SIN_SKILL_USE_RIGHT,Virtual_Life_UseMana },


{ "Espinhos Glaciais",	"L\342minas afiadas de gelo projetam do ch\343o congelando os inimigos.",
60, 35, 1, 120, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Glacial_Spike,SKILL_GLACIAL_SPIKE,SIN_SKILL_USE_ALL,Glacial_Spike_UseMana },

{ "Campo de Regenera\347\343o",	"Cria um campo m\341gico que acelera a Regenera\347\343o de Vida e Mana do grupo.",
63, 41, 1, 950, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Regeneration_Field,SKILL_REGENERATION_FIELD,SIN_SKILL_USE_RIGHT,Regeneration_Field_UseMana },

{ "Raios Cont\355nuos",	"Lan\347a um raio muito poderoso para abater o inimigo.",
66, 41, 1, 0, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Chain_Lightning,SKILL_CHAIN_LIGHTNING,SIN_SKILL_USE_ALL,Chain_Lightning_UseMana },

{ "Invocar Muspell",	"Anjo que batalha e protege a Celestial aumentando a Esquiva e a Absor\347\343o contra Mortos-Vivos.",
70, 47, 1, 800, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Summon_Muspell,SKILL_SUMMON_MUSPELL,SIN_SKILL_USE_RIGHT,Summon_Muspell_UseMana },

//Magician
{ "Agonia",			"Sacrifica a Vida restaurando a Mana.",
10, 12, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Agony,SKILL_AGONY,SIN_SKILL_USE_RIGHT,Agony_UseMana },

{ "Fagulha",			"Dispara pequenas fagulhas para ferir os inimigos.",
12, 11, 1, 84, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_FireBolt,SKILL_FIRE_BOLT,SIN_SKILL_USE_ALL,FireBolt_UseMana },

{ "Zenith",			"Aumenta temporariamente todas as Resist\352ncias.",
14, 15, 1, 100, 3,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Zenith,SKILL_ZENITH,SIN_SKILL_USE_RIGHT,Zenith_UseMana },

{ "Bola de Fogo",			"Dispara grandes bolas de fogo para abater os inimigos.",
17, 19, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_FireBall,SKILL_FIRE_BALL,SIN_SKILL_USE_ALL,FireBall_UseMana },

{ "Mestre da Mente",		"Aumenta sua Mana permanentemente.",
20, 0, 0, 0, 0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Mental_Mastery,SKILL_MENTAL_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Watornado",			"Um poderoso tornado d'\341gua \351 arremessado explodindo contra os inimigos.",
23, 21, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Watornado,SKILL_WATORNADO,SIN_SKILL_USE_ALL,Watornado_Mana },

{ "Encanto",		"Encanta sua arma ou do alvo proporcionando dano elemental.",
26, 28, 1, 104, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Enchant_Weapon,SKILL_ENCHANT_WEAPON,SIN_SKILL_USE_RIGHT,Enchant_Weapon_Mana },

{ "Raio da Morte",			"Um raio poderoso sai das m\343os do Feiticeiro desintegrando o alvo.",
30, 26, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Dead_Ray,SKILL_DEAD_RAY,SIN_SKILL_USE_ALL,Dead_Ray_Mana },

{ "Escudo Energ\351tico",		"Forma um escudo m\341gico absorvendo parte dos ataques sofridos ao custo de Mana.",
40, 40, 1, 150, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Energy_Shield,SKILL_ENERGY_SHIELD,SIN_SKILL_USE_RIGHT,Energy_Shield_UseMana },

{ "Terremoto",	"Cria um terremoto ao seu redor acertando todos os inimigos no alcance.",
43, 33, 1, 0, 1,
{ 0, 0, 0 }, { sinWM1, 0, 0, 0, 0, 0, 0, 0 },
F_Diastrophism,SKILL_DIASTROPHISM,SIN_SKILL_USE_RIGHT,Diastrophism_UseMana },

{ "Esp\355rito Elemental",	"Invoca um Esp\355rito dos elementos para aumentar o Poder de Ataque.",
46, 35, 1, 150, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Spirit_Elemental,SKILL_SPIRIT_ELEMENTAL,SIN_SKILL_USE_RIGHT,Spirit_Elemental_UseMana },

{ "Espada Dan\347ante",	"Uma espada fantasma aparece e ajuda a combater os inimigos com danos elementais.",
50, 30, 1, 150, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_D_Sword,SKILL_DANCING_SWORD,SIN_SKILL_USE_RIGHT,Dancing_Sword_UseMana },


{ "Chama Elemental",	"Invoca o Chama Elemental para acabar com os inimigos.",
60, 120, 2, 900, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Fire_Elemental,SKILL_FIRE_ELEMENTAL,SIN_SKILL_USE_RIGHT,Fire_Elemental_UseMana },

{ "Onda de Fogo",	"Ataque poderoso que queima todos os inimigos na frente do Mago Arcano.",
63, 32, 1, 124, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Flame_Wave,SKILL_FLAME_WAVE,SIN_SKILL_USE_ALL,Flame_Wave_UseMana },

{ "Distor\347\343o",	"Gera um pulso de energia que reduz temporariamente a Velocidade do inimigo.",
66, 43, 1, 460, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Distortion,SKILL_DISTORTION,SIN_SKILL_USE_RIGHT,Distortion_UseMana },

{ "Meteoro",	"Invoca uma poderosa chuva de meteoros para reduzir tudo a cinzas.",
70, 39, 1, 110, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Meteo,SKILL_M_METEO,SIN_SKILL_USE_ALL,Meteo_UseMana },


{ "For\347a Orb", "Aumenta o ATQ, de acordo com a Orb",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SKILL_FORCE_ORB,0,0 },


{ "Desafio", "Ca\347ar os monstros!",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CHANGE_JOB3_QUEST,0,0 },


{ "Desafio", "Ca\347ar os monstros!",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CHANGE_ELEMENTARY_QUEST,0,0 },


{ "Absor\347\343o Extrema","+10 de Absor\347\343o, por uma semana",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ABSORB,0,0 },

{ "For\347a Oculta","+10% de Poder de ATQ, por uma semana",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ATTACK,0,0 },

{ "Instinto","+10% de Esquiva, por uma semana",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CLANSKILL_EVASION,0,0 },


{ "Sacrif\355cio de Pritos","Invencibilidade tempor\341ria. Poder de ATQ reduzido pela metade",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_INVULNERABILITY,0,0 },

{ "Precis\343o de Skronipe","Aumento de 5% no Ataque Cr\355tico",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_CRITICAL,0,0 },

{ "Reflexo Selvagem","Aumento de 5% em Esquiva",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_EVASION,0,0 },

{ "Pedra da Revolta","Reduz o dano contra os Cristais de Fogo, em 50%",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_R_FIRECRYTAL,0,0 },

{ "Pedra da Revolta","Reduz o dano contra os Cristais de Gelo, em 50%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_R_ICECRYTAL,0,0 },

{ "Pedra da Revolta","Reduz o dano contra os Cristais de Raio, em 50%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_R_LINGHTINGCRYTAL,0,0 },

{ "Senten\347a de Morte ","Turbina o Poder de ATQ contra Lutadores, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_FIGHTER,0,0 },

{ "Senten\347a de Morte ","Turbina o Poder de ATQ contra Mec\342nicos, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_MECHANICIAN,0,0 },

{ "Senten\347a de Morte ","Turbina o Poder de ATQ contra Pikes, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_PIKEMAN,0,0 },

{ "Senten\347a de Morte ","Turbina o Poder de ATQ contra Arqueiras, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_ARCHER,0,0 },

{ "Senten\347a de Morte ","Turbina o Poder de ATQ contra Cavaleiros, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_KNIGHT,0,0 },

{ "Senten\347a de Morte ","Turbina o Poder de ATQ contra Atalantas, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_ATALANTA,0,0 },

{ "Senten\347a de Morte ","Turbina o Poder de ATQ contra Magos, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_MAGICIAN,0,0 },

{ "Senten\347a de Morte ","Turbina o Poder de ATQ contra Sacerdotisas, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_PRIESTESS,0,0 },

{ "Vida Eterna","Invencibilidade tempor\341ria. O ataque \351 reduzido pela 1/2",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_P_INVULNERABILITY,0,0 },

{ "Cr\355tico Adicional","5% a mais de ataque cr\355tico",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_P_CRITICAL,0,0 },

{ "Esquiva Adicional","5% a mais de esquiva",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_P_EVASION,0,0 },

{ "Chefe de Classe", "Adiciona 2% de ATK e 50 de HP",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, REIPVP, 0, 0 },

{ "Booster HP", "Regenera\347\343o HP 15%.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, BOOSTER_ITEM_LIFE, 0, 0 },

{ "Booster MP", "Regenera\347\343o MP 15%.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, BOOSTER_ITEM_MANA, 0, 0 },

{ "Booster STM", "Regenera\347\343o STM 15%.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, BOOSTER_ITEM_STAMINA, 0, 0 },


{ "Bellatra", "",
0,		0,0,		0,0,
{ 0,0,0 },{ 0,0,0,0,0,0,0,0 },
NULL,SOD_SKILL_GOLD,0,0 },
{ "Bellatra", "",
0,		0,0,		0,0,
{ 0,0,0 },{ 0,0,0,0,0,0,0,0 },
NULL,SOD_SKILL_SILVER,0,0 },
{ "Bellatra", "",
0,		0,0,		0,0,
{ 0,0,0 },{ 0,0,0,0,0,0,0,0 },
NULL,SOD_SKILL_BRONZE,0,0 },

{ "BlessCastle", "+5% de Poder de ATQ, por uma semana",
0,		0,0,		0,0,
{ 0,0,0 },{ 0,0,0,0,0,0,0,0 },
NULL,BC_SKILL_SILVER,0,0 },

{ "BlessCastle", "+3% de Poder de ATQ, por uma semana",
0,		0,0,		0,0,
{ 0,0,0 },{ 0,0,0,0,0,0,0,0 },
NULL,BC_SKILL_BRONZE,0,0 },

{ "Delay Chat Global", "Aguarde 30 segundos para a pr\363xima mensagem",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, CHATGLOBAL, 0, 0 },

//Crystals
{ "Cristal de Hopi", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 1, 0, 0 },

{ "Cristal de Duende Macabro", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 2, 0, 0 },

{ "Cristal de Decoy", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 3, 0, 0 },

{ "Cristal de Bargon", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 4, 0, 0 },

{ "Cristal de Decapitador", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 5, 0, 0 },

{ "Cristal de Figon", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 6, 0, 0 },

{ "Cristal de Rei Hopi", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 7, 0, 0 },

{ "Cristal de Hulk", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 8, 0, 0 },

{ "Cristal M\355stico", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 9, 0, 0 },

{ "Cristal de Guardi\343o Santo", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 10, 0, 0 },

{ "Cristal de Aranha", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 11, 0, 0 },

{ "Cristal do Caos", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 12, 0, 0 },

{ "Cristal de Ferro", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 13, 0, 0 },

{ "Cristal Mirotic", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 17, 0, 0 },

{ "Cristal G\352meo", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 18, 0, 0 },

{ "Cristal Imperial", "Cristal Sumonado. Tempo:240 seg.",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, 19, 0, 0 },

{ "VIP", "",
0, 0, 0, 0, 0,
{ 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0 },
NULL, PLAYERVIP, 0, 0 },




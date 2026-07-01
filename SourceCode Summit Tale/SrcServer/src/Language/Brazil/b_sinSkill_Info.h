#define SIN_SKILL_USE_RIGHT 0x01000000 // Available only for the right 
#define SIN_SKILL_USE_LEFT 0x02000000 // Available only for the left 
#define SIN_SKILL_USE_ALL 0x03000000 // Both available 
#define SIN_SKILL_USE_NOT 0x04000000 // Passive Skill

//Mechanician
{"Escudo Extremo", "Aumenta temporariamente o Bloqueio de Escudos.",
10, 34, 2, 110, 4,
{ 0,0,0 }, { sinDS1,0,0,0,0,0,0,0 },
F_Extreme_Shield, SKILL_EXTREME_SHIELD, SIN_SKILL_USE_RIGHT, E_Shield_UseMana},

{ "Bomba Mecânica",		"Joga uma granada para abater um grupo de inimigos.",
12, 28, 2, 73, 3,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Mechanic_Bomb,SKILL_MECHANIC_BOMB,SIN_SKILL_USE_ALL,Mechanic_Bomb_UseMana },

{ "Resistência a Veneno",		"Aumenta permanentemente a Resistência a Veneno.",
14, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Poison_Attribute,SKILL_POISON_ATTRIBUTE,SIN_SKILL_USE_NOT,0 },

{ "Absorção Física",		"Aumenta temporariamente sua Absorção.",
17, 30, 2, 110, 5,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Physical_Absorption,SKILL_PHYSICAL_ABSORB,SIN_SKILL_USE_RIGHT,P_Absor_bUseMana },

{ "Grande Golpe",			"Carrega e desfere um golpe poderoso no inimigo.",
20, 38, 2, 73, 3,
{0,0,0}, {sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Great_Smash,SKILL_GREAT_SMASH,SIN_SKILL_USE_ALL,G_Smash_UseMana },

{ "Maximizar",			"Aumenta temporariamente o Poder de Ataque Máximo da arma equipada.",
23, 41, 2, 110, 4,
{0,0,0}, {sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Maximize,SKILL_MAXIMIZE,SIN_SKILL_USE_RIGHT,Maximize_UseMana },

{ "Automação",			"Aperfeiçoa o uso de armas de longo alcance aumentando a Velocidade da Arma e o Poder de Ataque.",
26, 42, 2, 110, 4,
{0,0,0}, {sinWS1,sinWT1,0,0,0,0,0,0},
F_Automation,SKILL_AUTOMATION,SIN_SKILL_USE_RIGHT,Automation_UseMana },

{ "Spark",			"Ataca os inimigos utilizando esferas de energia concentrada.",
30, 40, 2, 82, 2,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Spark,SKILL_SPARK,SIN_SKILL_USE_ALL,Spark_UseMana },

{ "Armadura Metálica",		"Aumenta a Defesa da Armadura, se esta for Especializada para Mecânico.",
40, 48, 2, 110, 5,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Metal_Armor,SKILL_METAL_ARMOR,SIN_SKILL_USE_RIGHT,Metal_Armor_UseMana },

{ "Golpe Grandioso",	"Desfere dois poderosos golpes com Poder de Ataque e Taxa de Ataque adicionais.",
43, 44, 2, 82, 2,
{0,0,0}, {sinWA1,sinWC1,sinWP1,sinWS2, sinWH1,0,0,0},
F_Grand_Smash,SKILL_GRAND_SMASH,SIN_SKILL_USE_ALL,Grand_Smash_UseMana },

{ "Mestre dos Mecânicos",	"Aumenta o Poder de Ataque de todas as armas Especializadas para Mecânico.",
46, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_M_W_Mastery,SKILL_MECHANIC_WEAPON,SIN_SKILL_USE_NOT,0 },

{ "Escudo Spark",	    "Utiliza a habilidade Spark para encantar seu escudo. Todos os ataques bloqueados refletem para o atacante.",
50, 52, 2, 110, 2,
{0,0,0}, {sinDS1,0,0,0,0,0,0,0},
F_Spark_Shield,SKILL_SPARK_SHIELD,SIN_SKILL_USE_RIGHT,Spark_Shield_UseMana },


{ "Impulso","Ataque duplo que libera uma forte descarga elétrica.",
60, 55, 2, 0, 0,
{1,0,0},{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Impulsion,SKILL_IMPULSION,SIN_SKILL_USE_ALL,Impulsion_UseMana },

{ "Implosão","Implosão que atrai inimigos e aumenta temporariamente a Absorção.",
63, 65, 2, 440, 0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Compulsion,SKILL_COMPULSION,SIN_SKILL_USE_RIGHT,Compulsion_UseMana },

{ "Esferas Magnéticas","Cria esferas magnéticas para atacar automaticamente os inimigos mais próximos.",
66, 71, 2, 600, 0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Magnetic_Sphere,SKILL_MAGNETIC_SPHERE,SIN_SKILL_USE_RIGHT,Magnetic_Sphere_UseMana },

{ "Golem de Metal","Invoca um Golem de Metal para auxiliar nas batalhas.",
70, 71, 2, 1000, 0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Metal_Golem,SKILL_METAL_GOLEM,SIN_SKILL_USE_RIGHT,Metal_Golem_UseMana },


//Fighter 
{ "Mestre das Armas",		"Aumenta permanentemente a eficiência de Machados, Martelos e Espadas.",
10, 0,0, 0,0,
{0,0,0}, {sinWA1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Melee_Mastery,SKILL_MELEE_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Resistência a Fogo",		"Aumenta permanentemente a Resistência a Fogo.",
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

{ "Impacto Triplo",		"Versão melhorada do Impacto.",
20, 39, 2, 80, 4,
{0,0,0}, {sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Triple_Impact,SKILL_TRIPLE_IMPACT,SIN_SKILL_USE_ALL,T_Impact_UseMana },

{ "Girada Brutal",		"Desfere um poderoso golpe com Crítico adicional.",
23, 41, 2, 82, 2,
{0,0,0}, {sinWA1,sinWS2,0,0,0,0,0,0},
F_Brutal_Swing,SKILL_BRUTAL_SWING,SIN_SKILL_USE_ALL,B_Swing_UseMana },

{ "Rugido",			"Atordoa temporariamente os inimigos.",
26, 39, 2, 112, 4,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Roar,SKILL_ROAR,SIN_SKILL_USE_RIGHT,Roar_UseMana },

{ "Fúria de Zecram",		"Ataque poderoso que acerta todos os inimigos ao redor do alvo.",
30, 45, 2, 84, 4,
{0,0,0}, {sinWA1,sinWH1,sinWS2,0,0,0,0,0},
F_RageOfZecram,SKILL_RAGE_OF_ZECRAM,SIN_SKILL_USE_ALL,R_Zecram_UseMana },

{ "Concentração",	"Aumenta sua Taxa de Ataque temporariamente.",
40, 53, 2, 110, 3,
{0,0,0}, {sinWA1,sinWS2,0,0,0,0,0,0},
F_Concentration,SKILL_CONCENTRATION,SIN_SKILL_USE_RIGHT,Concentration_UseMana },

{ "Golpe da Vingança",	"Ataque poderoso com adicional de Poder de Ataque e Taxa de Ataque.",
43, 48, 2, 73, 3,
{0,0,0}, {sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Avanging_Crash,SKILL_AVANGING_CRASH,SIN_SKILL_USE_ALL,A_Crash_UseMana },

{ "Machado Veloz",	"Aumenta a Velocidade de Machados temporariamente.",
46, 53, 2, 200, 3,
{0,0,0}, {sinWA1,0,0,0,0,0,0,0},
F_Swift_Axe,SKILL_SWIFT_AXE,SIN_SKILL_USE_RIGHT,Swift_Axe_UseMana },

{ "Quebra Ossos",		"Ataque poderoso com Bônus contra Demônios.",
50, 54, 2, 73, 3,
{0,0,0}, {sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Bone_Crash,SKILL_BONE_CRASH,SIN_SKILL_USE_ALL,B_Crash_UseMana },


{ "Destroyer",		"Ataque destrutivo que acerta três vezes o inimigo.",
60, 45, 2, 150, 0,
{1,0,0}, {sinWA1,sinWH1,0,0,0,0,0,0},
F_Destoryer,SKILL_DETORYER,SIN_SKILL_USE_ALL,Destoryer_UseMana },

{ "Fúria","Sacrifica Absorção para aumentar o Poder de Ataque.",
63, 60, 2, 700, 0,
{1,0,0}, {0,0,0,0,0,0,0,0},
F_Berserker,SKILL_BERSERKER,SIN_SKILL_USE_RIGHT,Berserker_UseMana },

{ "Golpe Giratório","Golpe múltiplo e giratório letal a grupos de inimigos.",
66, 55, 2, 0, 0,
{1,0,0}, {sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Cyclone_Strike,SKILL_CYCLONE_STRIKE,SIN_SKILL_USE_ALL,Cyclone_Strike_UseMana },

{ "Bônus de Vitalidade","Aumenta permanentemente sua Vida.",
70, 0, 0, 0, 0,
{1,0,0}, {0,0,0,0,0,0,0,0},
F_Boost_Health,SKILL_BOOST_HEALTH,SIN_SKILL_USE_NOT, },

//Pikeman
{ "Sopro Afiado",			"Cria um vento circular para afastar os inimigos.",
10, 27, 2, 80, 4,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Pike_Wind,SKILL_PIKE_WIND,SIN_SKILL_USE_RIGHT,Pike_Wind_UseMana },

{ "Resistência a Gelo",		"Aumenta permanentemente a sua Resistência a Gelo.",
12, 0, 0, 0, 0,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Ice_Attribute,SKILL_ICE_ATTRIBUTE,SIN_SKILL_USE_NOT,0 },

{ "Ataque Crítico",		"Acerta os pontos fracos dos inimigos para aumentar a chance de Críticos.",
14, 31, 2, 84, 3,
{0,0,0}, {sinWP1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Critical_Hit,SKILL_CRITICAL_HIT,SIN_SKILL_USE_ALL,Critical_Hit_UseMana },

{ "Pulo Fatal",		"Ataque aéreo com grandes chances de matar o oponente.",
17, 34, 2, 76, 4,
{0,0,0}, {sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Jumping_Crash,SKILL_JUMPING_CRASH,SIN_SKILL_USE_ALL,Jumping_Crash_UseMana },

{ "Espinhos de Gelo",		"Espinhos de gelo surgem e paralisam os inimigos ao seu redor.",
20, 36, 2, 84, 4,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Ground_Pike,SKILL_GROUND_PIKE,SIN_SKILL_USE_RIGHT,G_Pike_UseMana },

{ "Tornado",			"Gira sua foice acertando todos os inimigos no caminho.",
23, 38, 2, 83, 4,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Tornado_Pike,SKILL_TORNADO,SIN_SKILL_USE_ALL,Tornado_UseMana },

{ "Mestre do Bloqueio",		"Aumenta a chance de Bloqueio com armas de duas mãos.",
26, 0, 0, 0, 0,
{0,0,0}, {sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Weapone_Defence_Mastery,SKILL_WEAPONE_DEFENCE_MASTERY,SIN_SKILL_USE_NOT,0 },


{ "Expansão",			"Aumenta o tamanho da arma, elevando o Poder de Ataque dela.",
30, 42, 2, 82, 4,
{0,0,0}, {sinWA1,sinWH1,sinWP1,0,0,0,0,0},
F_Expansion,SKILL_EXPANSION,SIN_SKILL_USE_ALL,Expansion_UseMana },

{ "Foices Venenosas",	"Envenena os inimigos ao seu redor temporariamente.",
40, 50, 2, 110, 2,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Venom_Spear,SKILL_VENOM_SPEAR,SIN_SKILL_USE_RIGHT,VenomSpear_UseMana },

{ "Desaparecer",			"Fica invisível temporariamente, não sendo percebido pelos monstros. O primeiro ataque após o uso será mais forte.",
43, 53, 2, 140, 2,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Vanish,SKILL_VANISH,SIN_SKILL_USE_RIGHT,Vanish_UseMana },

{ "Mestre do Ataque Crítico",		"Aumenta permanentemente a chance de Crítico ao utilizar Foices, além de aumentar o Poder de Ataque.",
46, 0, 0, 0, 0,
{0,0,0}, {sinWP1,0,0,0,0,0,0,0},
F_Critical_Mastery,SKILL_CRITICAL_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Foice de Aço",	  		  "Ataque potente que desfere três golpes no inimigo.",
50, 51, 2, 88, 3,
{0,0,0}, {sinWC1,sinWP1,sinWS2,0,0,0,0,0},
F_Chain_Lance,SKILL_CHAIN_LANCE,SIN_SKILL_USE_ALL,Chain_Lance_UseMana },


{ "Olho Assassino",	 "Procura a fraqueza do inimigo para aumentar a chance de Crítico.",
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

{ "Mestre das Sombras",	"Ataques múltiplos de sombras misteriosas a fim de destruir o inimigo.",
70, 51, 2, 160, 0,
{1,0,0}, {sinWP1,sinWS2,0,0,0,0,0,0},
F_Shadow_Master,SKILL_SHADOW_MASTER,SIN_SKILL_USE_ALL,Shadow_Master_UseMana },


//Archer
{ "Falcão Vigia",			"Invoca um falcão para vigiar a área a fim de aumentar sua Taxa de Ataque.",
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

{ "Falcão",			"Invoca um falcão para atacar seus inimigos.",
23, 36, 2, 115, 4,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_Falcon ,SKILL_FALCON,SIN_SKILL_USE_RIGHT,Falcon_UseMana },

{ "Flecha da Fúria",		"Rajada fatal que explode e fere um grupo de inimigos.",
26, 38, 2, 82, 4,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Arrow_of_Rage,SKILL_ARROW_OF_RAGE,SIN_SKILL_USE_ALL,A_Rage_UseMana },

{ "Avalanche",			"Lança uma sequencia de flechas perfuradoras para destruir os inimigos.",
30, 41, 2, 103, 2,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Avalanche,SKILL_AVALANCHE,SIN_SKILL_USE_ALL,Avalanche_UseMana },

{ "Tiro Elemental",		"Dispara flechas elementais contra o inimigo.",
40, 43, 2, 82, 2,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_E_Shot,SKILL_ELEMENTAL_SHOT,SIN_SKILL_USE_ALL,Elemental_Shot_UseMana },

{ "Falcão Dourado",		"Invoca um falcão dourado que ataca os inimigos e aumenta sua Regeneração de Vida.",
43, 52, 2, 120, 4,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_G_Falcon,SKILL_GOLDEN_FALCON,SIN_SKILL_USE_RIGHT,Golden_Falcon_UseMana },

{ "Tiro Explosivo",		    "Atira flechas explosivas para causar dano em área.",
46, 46, 2, 87, 3,
{0,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_B_Shot,SKILL_BOMB_SHOT,SIN_SKILL_USE_ALL,BombShot_UseMana },

{ "Perfuração",		"Ataque preciso e contínuo, que atravessa os inimigos.",
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

{ "Tiro Fênix",	"Ataque carregado com a força da Fênix que atinge vários inimigos ou concentra o poder em apenas um.",
66, 55, 2, 100, 0,
{1,0,0}, {sinWS1,0,0,0,0,0,0,0},
F_Phoenix_Shot,SKILL_PHOENIX_SHOT,SIN_SKILL_USE_ALL,Phoenix_Shot_UseMana },

{ "Força da Natureza","Aumenta seus poderes pela força da natureza.",
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

{ "Corpo Sagrado",			"Recebe a benção divina, aumentando a Absorção contra Mortos-Vivos.",
12, 28, 2, 110, 3,
{0,0,0}, {0,0,0,0,0,0,0,0},
F_HolyBody ,SKILL_HOLY_BODY,SIN_SKILL_USE_RIGHT,HolyBody_UseMana },

{ "Treinamento Físico",		"Realiza treinamento físico, aumentando permanentemente a Resistência do personagem.",
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

{ "Brandish",			"Ataque rápido e poderoso em área.",
23, 38, 2, 82, 2,
{0,0,0}, {sinWP1,sinWS2,0,0,0,0,0,0},
F_Brandish,SKILL_BRANDISH,SIN_SKILL_USE_ALL,Brandish_Mana },

{ "Piercing",			"Carrega sua espada, desferindo um ataque que atravessa o inimigo.",
26, 41, 2, 82, 2,
{0,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_Piercing,SKILL_PIERCING,SIN_SKILL_USE_ALL,Piercing_Mana },

{ "Espírito Drástico",		"Aumenta a Defesa temporariamente.",
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

{ "Grande Cruzada",	"Ataque duplo, poderoso, veloz e contínuo.",
50, 45, 2, 82, 2,
{0,0,0}, {sinWH1,sinWS2,0,0,0,0,0,0},
F_Grand_Cross,SKILL_GRAND_CROSS,SIN_SKILL_USE_ALL,Grand_Cross_UseMana },


{ "Piercing Divino",	"Utilizando a força divina, desfere uma sequência letal de golpes.",
70, 51, 2, 160, 0,
{1,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_Divine_Piercing,SKILL_DIVINE_PIERCING,SIN_SKILL_USE_ALL,Divine_Piercing_UseMana },

{ "Escudo dos Deuses",	"Cria um escudo divino que garante proteção adicional ao Cavaleiro.",
63, 50, 2, 400, 0,
{1,0,0}, {sinDS1,0,0,0,0,0,0,0},
F_Godly_Shield,SKILL_GODLY_SHIELD,SIN_SKILL_USE_RIGHT,Godly_Shield_UseMana },

{ "Bênção Divina",	"Invoca a força divina, aumentando temporariamente o Poder de Ataque.",
66, 51, 2, 330, 0,
{1,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_God_Bless,SKILL_GOD_BLESS,SIN_SKILL_USE_RIGHT,God_Bless_UseMana },

{ "Espada da Justiça","Poderoso ataque em área.",
60, 53, 2, 250, 0,
{1,0,0}, {sinWS2,0,0,0,0,0,0,0},
F_Sword_Of_Justice,SKILL_SWORD_OF_JUSTICE,SIN_SKILL_USE_ALL,Sword_Of_Justice_UseMana },


// Atalanta
{ "Golpe do Escudo",		"Atordoa temporariamente os inimigos.",
10, 23, 2, 82, 2,
{0,0,0}, {sinDS1,0,0,0,0,0,0,0},
F_S_Strike ,SKILL_SHIELD_STRIKE,SIN_SKILL_USE_ALL,S_Strike_UseMana },

{ "Farina",			"Desfere ataques com bônus na Taxa de Ataque.",
12, 26, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Farina,SKILL_FARINA,SIN_SKILL_USE_ALL,Farina_UseMana },

{ "Mestra do Arremesso",		"Aumenta seu Poder de Ataque com Lanças permanentemente.",
14, 0, 0, 0, 0,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_D_Mastery,SKILL_THROWING_MASTERY,SIN_SKILL_USE_NOT,0 },

{ "Lança Gigante",		"Ataca os inimigos com a força divina.",
17, 32, 2, 82, 2,
{0,0,0}, {sinWP1,sinWT1,0,0,0,0,0,0},
F_V_Spear,SKILL_VIGOR_SPEAR,SIN_SKILL_USE_ALL,V_Spear_UseMana },

{ "Windy",			"Adiciona a propriedade Vento na arma para arremessar mais longe e com maior precisão.",
20, 34, 2, 100, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Windy,SKILL_WINDY,SIN_SKILL_USE_RIGHT,Windy_Mana },

{ "Lança Giratória",		"Aumenta o Poder de Ataque e a Taxa de Ataque em cada acerto.",
23, 36, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Twist_Javelin,SKILL_TWIST_JAVELIN,SIN_SKILL_USE_ALL,Twist_Javelin_Mana },

{ "Ladrão de Almas",			"Absorve a Vida dos inimigos.",
26, 42, 2, 110, 2,
{0,0,0}, {sinDS1,0,0,0,0,0,0,0},
F_Soul_Sucker,SKILL_SOUL_SUCKER,SIN_SKILL_USE_RIGHT,Soul_Sucker_Mana },

{ "Lança de Fogo",		"Adiciona a propriedade Fogo nos seus ataques.",
30, 40, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Fire_Javelin,SKILL_FIRE_JAVELIN,SIN_SKILL_USE_ALL,Fire_Javelin_Mana },

{ "Lança Partida",	"Ataques contínuos e rápidos.",
40, 43, 2, 115, 1,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Split_Javelin,SKILL_SPLIT_JAVELIN,SIN_SKILL_USE_ALL,Split_Javelin_UseMana },

{ "Triunfo de Valhalla",	"Aumenta o Poder de Ataque temporariamente.",
43, 50, 2, 120, 1,
{0,0,0}, {sinWT1,sinWS2,sinWS1,sinWP1,sinWH1,sinWA1,sinWC1,sinWM1},
F_T_Of_Valhalla,SKILL_TRIUMPH_OF_VALHALLA,SIN_SKILL_USE_RIGHT,T_Of_Valhalla_UseMana },

{ "Lança de Raios",	"Adiciona dano elemental às Lanças.",
46, 46, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Lightning_Javelin,SKILL_LIGHTNING_JAVELIN,SIN_SKILL_USE_ALL,Lightning_Javelin_UseMana },

{ "Chuva de Lanças",	    "Arremessa lanças como devastadores espirais que destroem tudo em seu caminho.",
50, 48, 2, 82, 2,
{0,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Storm_Javelin,SKILL_STORM_JAVELIN,SIN_SKILL_USE_ALL,Storm_Javelin_UseMana },


{ "Inferno de Valhalla",	"Aumenta temporariamente a Esquiva por meio do uso do poder máximo de Valhalla.",
60, 52, 2, 750, 0,
{1,0,0}, {sinWA1,sinWC1,sinWH1,sinWP1,sinWS1,sinWT1,0,0},
F_Hall_Of_Valhalla,SKILL_HALL_OF_VALHALLA,SIN_SKILL_USE_RIGHT,Hall_Of_Valhalla_UseMana },

{ "Fúria-X",	"Pula no ar e atira 3 lanças explosivas ferindo todos os inimigos na área.",
63, 48, 2, 0, 0,
{1,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_X_Rage,SKILL_X_RAGE,SIN_SKILL_USE_ALL,X_Rage_UseMana },

{ "Lança de Gelo",	"Adiciona a propriedade Gelo nos seus ataques.",
66, 50, 2, 720, 0,
{1,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Frost_Javelin,SKILL_FROST_JAVELIN,SIN_SKILL_USE_RIGHT,Frost_Javelin_UseMana },

{ "Vingança",	"Ataque rápido, múltiplo e letal.",
70, 55, 2, 100, 0,
{1,0,0}, {sinWT1,0,0,0,0,0,0,0},
F_Vengeance,SKILL_VENGEANCE,SIN_SKILL_USE_ALL,Vengeance_UseMana },

// Priestess
{ "Cura",			 "Com poder divino, recupera sua Vida ou a Vida de aliados.",
10, 15, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Healing,SKILL_HEALING,SIN_SKILL_USE_RIGHT,Healing_UseMana },

{ "Fagulha Sagrada",			"Faísca mágica que fere os inimigos.",
12, 11, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_HolyBolt,SKILL_HOLY_BOLT,SIN_SKILL_USE_ALL,HolyBolt_UseMana },

{ "Espinhos Múltiplos",			"Dispara um ataque múltiplo concentrado em um único alvo.",
14, 14, 1, 1000, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_M_Spark,SKILL_MULTISPARK,SIN_SKILL_USE_ALL,M_Spark_UseMana },

{ "Feitiço Divino",			"Encanta os inimigos, reduzindo temporariamente o Poder de Ataque.",
17, 19, 1, 100, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_HolyMind,SKILL_HOLY_MIND,SIN_SKILL_USE_ALL,HolyMind_UseMana },

{ "Meditação",			"Aumenta a Regeneração de Mana permanentemente, além de aumentar o Poder de Ataque.",
20, 0, 0, 0, 0,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Meditation,SKILL_MEDITATION,SIN_SKILL_USE_NOT,0 },

{ "Raios Divinos",		"Invoca uma chuva de raios divinos, que caem das nuvens atingindo os inimigos ao seu redor.",
23, 21, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Divine_Lightning,SKILL_DIVINE_LIGHTNING,SIN_SKILL_USE_RIGHT,Divine_Lightning_Mana },

{ "Reflexão Divina",		"Cria um escudo mágico que reflete o dano sofrido por Mortos-Vivos.",
26, 23, 1, 120, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Holy_Reflection,SKILL_HOLY_REFLECTION,SIN_SKILL_USE_RIGHT,Holy_Reflection_Mana },

{ "Cura Máxima",		"Recupera a Vida de todos no grupo.",
30, 27, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Grand_Healing,SKILL_GRAND_HEALING,SIN_SKILL_USE_RIGHT,Grand_Healing_Mana },

{ "Esferas de Vigor",			"Dispara duas esferas luminosas letais contra os inimigos.",
40, 24, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Vigor_Ball,SKILL_VIGOR_BALL,SIN_SKILL_USE_ALL,Vigor_Ball_UseMana },

{ "Ressurreição",		"Permite trazer um membro do grupo de volta à vida.",
43, 31, 1, 130, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Resurrection,SKILL_RESURRECTION,SIN_SKILL_USE_RIGHT,Resurrection_UseMana },

{ "Extinção",		"Lança um feitiço que elimina parte da Vida dos Mortos-Vivos.",
46, 28, 1, 120, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Extinction,SKILL_EXTINCTION,SIN_SKILL_USE_RIGHT,Extinction_UseMana },

{ "Vida Virtual",	"Aumenta a sua Vida ou a Vida do alvo temporariamente.",
50, 31, 1, 50, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Virtual_Life,SKILL_VIRTUAL_LIFE,SIN_SKILL_USE_RIGHT,Virtual_Life_UseMana },


{ "Espinhos Glaciais",	"Lâminas afiadas de gelo projetam do chão congelando os inimigos.",
60, 35, 1, 120, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Glacial_Spike,SKILL_GLACIAL_SPIKE,SIN_SKILL_USE_ALL,Glacial_Spike_UseMana },

{ "Campo de Regeneração",	"Cria um campo mágico que acelera a Regeneração de Vida e Mana do grupo.",
63, 41, 1, 950, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Regeneration_Field,SKILL_REGENERATION_FIELD,SIN_SKILL_USE_RIGHT,Regeneration_Field_UseMana },

{ "Raios Contínuos",	"Lança um raio muito poderoso para abater o inimigo.",
66, 41, 1, 0, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Chain_Lightning,SKILL_CHAIN_LIGHTNING,SIN_SKILL_USE_ALL,Chain_Lightning_UseMana },

{ "Invocar Muspell",	"Anjo que batalha e protege a Celestial aumentando a Esquiva e a Absorção contra Mortos-Vivos.",
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

{ "Zenith",			"Aumenta temporariamente todas as Resistências.",
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

{ "Watornado",			"Um poderoso tornado d'água é arremessado explodindo contra os inimigos.",
23, 21, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Watornado,SKILL_WATORNADO,SIN_SKILL_USE_ALL,Watornado_Mana },

{ "Encanto",		"Encanta sua arma ou do alvo proporcionando dano elemental.",
26, 28, 1, 104, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Enchant_Weapon,SKILL_ENCHANT_WEAPON,SIN_SKILL_USE_RIGHT,Enchant_Weapon_Mana },

{ "Raio da Morte",			"Um raio poderoso sai das mãos do Feiticeiro desintegrando o alvo.",
30, 26, 1, 82, 2,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Dead_Ray,SKILL_DEAD_RAY,SIN_SKILL_USE_ALL,Dead_Ray_Mana },

{ "Escudo Energético",		"Forma um escudo mágico absorvendo parte dos ataques sofridos ao custo de Mana.",
40, 40, 1, 150, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Energy_Shield,SKILL_ENERGY_SHIELD,SIN_SKILL_USE_RIGHT,Energy_Shield_UseMana },

{ "Terremoto",	"Cria um terremoto ao seu redor acertando todos os inimigos no alcance.",
43, 33, 1, 106, 1,
{ 0, 0, 0 }, { sinWM1, 0, 0, 0, 0, 0, 0, 0 },
F_Diastrophism,SKILL_DIASTROPHISM,SIN_SKILL_USE_RIGHT,Diastrophism_UseMana },

{ "Espírito Elemental",	"Invoca um Espírito dos elementos para aumentar o Poder de Ataque.",
46, 35, 1, 150, 1,
{0,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Spirit_Elemental,SKILL_SPIRIT_ELEMENTAL,SIN_SKILL_USE_RIGHT,Spirit_Elemental_UseMana },

{ "Espada Dançante",	"Uma espada fantasma aparece e ajuda a combater os inimigos com danos elementais.",
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

{ "Distorção",	"Gera um pulso de energia que reduz temporariamente a Velocidade do inimigo.",
66, 43, 1, 460, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Distortion,SKILL_DISTORTION,SIN_SKILL_USE_RIGHT,Distortion_UseMana },

{ "Meteoro",	"Invoca uma poderosa chuva de meteoros para reduzir tudo a cinzas.",
70, 39, 1, 110, 0,
{1,0,0}, {sinWM1,0,0,0,0,0,0,0},
F_Meteo,SKILL_M_METEO,SIN_SKILL_USE_ALL,Meteo_UseMana },


{ "Força Orb", "Aumenta o ATQ, de acordo com a Orb",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SKILL_FORCE_ORB,0,0 },


{ "Desafio", "Caçar os monstros!",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CHANGE_JOB3_QUEST,0,0 },


{ "Desafio", "Caçar os monstros!",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CHANGE_ELEMENTARY_QUEST,0,0 },


{ "Absorção Extrema","+10 de Absorção, por uma semana",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ABSORB,0,0 },

{ "Força Oculta","+10% de Poder de ATQ, por uma semana",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ATTACK,0,0 },

{ "Instinto","+10% de Esquiva, por uma semana.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,CLANSKILL_EVASION,0,0 },


{ "Sacrifício de Pritos","Invencibilidade temporária. Poder de ATQ reduzido pela metade",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_INVULNERABILITY,0,0 },

{ "Precisão de Skronipe","Aumento de 5% no Ataque Crítico",
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

{ "Sentença de Morte ","Turbina o Poder de ATQ contra Lutadores, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_FIGHTER,0,0 },

{ "Sentença de Morte ","Turbina o Poder de ATQ contra Mecânicos, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_MECHANICIAN,0,0 },

{ "Sentença de Morte ","Turbina o Poder de ATQ contra Pikes, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_PIKEMAN,0,0 },

{ "Sentença de Morte ","Turbina o Poder de ATQ contra Arqueiras, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_ARCHER,0,0 },

{ "Sentença de Morte ","Turbina o Poder de ATQ contra Cavaleiros, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_KNIGHT,0,0 },

{ "Sentença de Morte ","Turbina o Poder de ATQ contra Atalantas, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_ATALANTA,0,0 },

{ "Sentença de Morte ","Turbina o Poder de ATQ contra Magos, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_MAGICIAN,0,0 },

{ "Sentença de Morte ","Turbina o Poder de ATQ contra Sacerdotisas, em 20%.",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,STONE_A_PRIESTESS,0,0 },

{ "Vida Eterna","Invencibilidade temporária. O ataque é reduzido pela 1/2",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_P_INVULNERABILITY,0,0 },

{ "Ponta Fatal","5% a mais de ataque crítico",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_P_CRITICAL,0,0 },

{ "Amuleto","5% a mais de esquiva",
0, 0,0, 0,0,
{0,0,0}, {0,0,0,0,0,0,0,0},
NULL,SCROLL_P_EVASION,0,0 },
{ "BOOSTER OF HEALTH","xxxx 15% xxxx",
0,		0,0,		0,0,
{ 0,0,0 },{ 0,0,0,0,0,0,0,0 },
NULL,BOOSTER_ITEM_LIFE,0,0 },

{ "BOOSTER OF MANA","xxx 15% xxxn",
0,		0,0,		0,0,
{ 0,0,0 },{ 0,0,0,0,0,0,0,0 },
NULL,BOOSTER_ITEM_MANA,0,0 },


{ "BOOSTER OF STAMINA","xx 15% xxxx",
0,		0,0,		0,0,
{ 0,0,0 },{ 0,0,0,0,0,0,0,0 },
NULL,BOOSTER_ITEM_STAMINA,0,0 },
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


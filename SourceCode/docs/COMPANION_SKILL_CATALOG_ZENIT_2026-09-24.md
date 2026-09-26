# Companion Skill Catalog - Zenit official mapping (2026-09-24)

Source classification: Zenit Games official Priston Tale site links its **GUIA DO JOGO / WIKI** to `wiki.priston.com.br`; the effect labels below were transcribed from that official guide and normalized into stable ASCII names for the legacy client.

Scope executed by current companion persistence: classic Tier 1-4, **16 persisted skill points per class**.

Tier 5 exists on the current official guide, but this source explicitly keeps Rank5 skill data/handlers disabled and the companion bridge currently loads only the 16 legacy T1-T4 points. Tier 5 is intentionally **not enabled** (`catalog-blocked`) until the Rank5 source contract is reconciled.

Catalog size: **128 skills** (8 classes x 16). Current authoritative adapters marked `READY`: **63**. `CATALOG_ONLY` means the skill is known/classified but is deliberately not selectable by the companion executor yet.

| Class | Slot | Code | Stable name | Role | Official effect classification | Companion adapter |
|---|---:|---|---|---|---|---|
| Mechanician | 1 | `SKILL_EXTREME_SHIELD` | Extreme Shield | `BUFF_SELF` | Buff/Defensivo | READY |
| Mechanician | 2 | `SKILL_MECHANIC_BOMB` | Mechanic Bomb | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Mechanician | 3 | `SKILL_PHYSICAL_ABSORB` | Physical Absorption | `BUFF_SELF` | Buff/Defensivo | READY |
| Mechanician | 4 | `SKILL_POISON_ATTRIBUTE` | Poison Resistance | `PASSIVE` | Passivo | CATALOG_ONLY |
| Mechanician | 5 | `SKILL_GREAT_SMASH` | Great Smash | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Mechanician | 6 | `SKILL_MAXIMIZE` | Maximize | `BUFF_SELF` | Buff/Ofensivo | READY |
| Mechanician | 7 | `SKILL_AUTOMATION` | Automation | `BUFF_SELF` | Buff/Ofensivo | READY |
| Mechanician | 8 | `SKILL_SPARK` | Spark | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Mechanician | 9 | `SKILL_METAL_ARMOR` | Metal Armor | `BUFF_SELF` | Buff/Defensivo | CATALOG_ONLY |
| Mechanician | 10 | `SKILL_GRAND_SMASH` | Grand Smash | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Mechanician | 11 | `SKILL_MECHANIC_WEAPON` | Mechanic Weapon Mastery | `PASSIVE` | Passivo | CATALOG_ONLY |
| Mechanician | 12 | `SKILL_SPARK_SHIELD` | Spark Shield | `REACTIVE` | Dano defensivo | CATALOG_ONLY |
| Mechanician | 13 | `SKILL_IMPULSION` | Impulsion | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Mechanician | 14 | `SKILL_COMPULSION` | Compulsion | `BUFF_GROUP` | Buff/Defensivo | CATALOG_ONLY |
| Mechanician | 15 | `SKILL_MAGNETIC_SPHERE` | Magnetic Sphere | `BUFF_SELF` | Buff/Ofensivo | CATALOG_ONLY |
| Mechanician | 16 | `SKILL_METAL_GOLEM` | Metal Golem | `SUMMON` | Pet/Ofensivo | CATALOG_ONLY |
| Fighter | 1 | `SKILL_MELEE_MASTERY` | Weapon Mastery | `PASSIVE` | Passivo | CATALOG_ONLY |
| Fighter | 2 | `SKILL_FIRE_ATTRIBUTE` | Fire Resistance | `PASSIVE` | Passivo | CATALOG_ONLY |
| Fighter | 3 | `SKILL_RAVING` | Raving | `DAMAGE_SINGLE` | Dano ofensivo X1 | CATALOG_ONLY |
| Fighter | 4 | `SKILL_IMPACT` | Impact | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Fighter | 5 | `SKILL_TRIPLE_IMPACT` | Triple Impact | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Fighter | 6 | `SKILL_BRUTAL_SWING` | Brutal Swing | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Fighter | 7 | `SKILL_ROAR` | Roar | `CONTROL` | Efeito em area | CATALOG_ONLY |
| Fighter | 8 | `SKILL_RAGE_OF_ZECRAM` | Rage of Zecram | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Fighter | 9 | `SKILL_CONCENTRATION` | Concentration | `BUFF_SELF` | Buff/Ofensivo | READY |
| Fighter | 10 | `SKILL_AVANGING_CRASH` | Avenging Crash | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Fighter | 11 | `SKILL_SWIFT_AXE` | Swift Axe | `BUFF_SELF` | Buff/Ofensivo | READY |
| Fighter | 12 | `SKILL_BONE_CRASH` | Bone Crash | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Fighter | 13 | `SKILL_DETORYER` | Destroyer | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Fighter | 14 | `SKILL_BERSERKER` | Berserker | `BUFF_SELF` | Buff/Ofensivo | READY |
| Fighter | 15 | `SKILL_CYCLONE_STRIKE` | Cyclone Strike | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Fighter | 16 | `SKILL_BOOST_HEALTH` | Boost Health | `PASSIVE` | Passivo | CATALOG_ONLY |
| Pikeman | 1 | `SKILL_PIKE_WIND` | Pike Wind | `CONTROL` | Ataque defensivo em area | CATALOG_ONLY |
| Pikeman | 2 | `SKILL_ICE_ATTRIBUTE` | Ice Resistance | `PASSIVE` | Passivo | CATALOG_ONLY |
| Pikeman | 3 | `SKILL_CRITICAL_HIT` | Critical Hit | `DAMAGE_SINGLE` | Dano ofensivo X1 | CATALOG_ONLY |
| Pikeman | 4 | `SKILL_JUMPING_CRASH` | Jumping Crash | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Pikeman | 5 | `SKILL_GROUND_PIKE` | Ground Pike | `DAMAGE_AOE` | Dano ofensivo em area | CATALOG_ONLY |
| Pikeman | 6 | `SKILL_TORNADO` | Tornado | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Pikeman | 7 | `SKILL_WEAPONE_DEFENCE_MASTERY` | Weapon Defence Mastery | `PASSIVE` | Passivo | CATALOG_ONLY |
| Pikeman | 8 | `SKILL_EXPANSION` | Expansion | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Pikeman | 9 | `SKILL_VENOM_SPEAR` | Venom Spear | `DAMAGE_AOE` | Dano ofensivo em area | CATALOG_ONLY |
| Pikeman | 10 | `SKILL_VANISH` | Vanish | `BUFF_SELF` | Buff/Ofensivo | CATALOG_ONLY |
| Pikeman | 11 | `SKILL_CRITICAL_MASTERY` | Critical Mastery | `PASSIVE` | Passivo | CATALOG_ONLY |
| Pikeman | 12 | `SKILL_CHAIN_LANCE` | Chain Lance | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Pikeman | 13 | `SKILL_ASSASSIN_EYE` | Assassin Eye | `DEBUFF_SINGLE` | Buff/Ofensivo X1 | CATALOG_ONLY |
| Pikeman | 14 | `SKILL_CHARGING_STRIKE` | Charging Strike | `DAMAGE_SINGLE` | Dano ofensivo X1 | CATALOG_ONLY |
| Pikeman | 15 | `SKILL_VAGUE` | Vague | `BUFF_SELF` | Buff/Defensivo | CATALOG_ONLY |
| Pikeman | 16 | `SKILL_SHADOW_MASTER` | Shadow Master | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Archer | 1 | `SKILL_SCOUT_HAWK` | Scout Hawk | `BUFF_SELF` | Buff/Ofensivo | READY |
| Archer | 2 | `SKILL_SHOOTING_MASTERY` | Shooting Mastery | `PASSIVE` | Passivo | CATALOG_ONLY |
| Archer | 3 | `SKILL_WIND_ARROW` | Wind Arrow | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Archer | 4 | `SKILL_PERFECT_AIM` | Perfect Aim | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Archer | 5 | `SKILL_DIONS_EYE` | Dions Eye | `PASSIVE` | Passivo | CATALOG_ONLY |
| Archer | 6 | `SKILL_FALCON` | Falcon | `SUMMON` | Buff/Ofensivo | CATALOG_ONLY |
| Archer | 7 | `SKILL_ARROW_OF_RAGE` | Arrow of Rage | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Archer | 8 | `SKILL_AVALANCHE` | Avalanche | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Archer | 9 | `SKILL_ELEMENTAL_SHOT` | Elemental Shot | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Archer | 10 | `SKILL_GOLDEN_FALCON` | Golden Falcon | `SUMMON` | Buff/Ofensivo + regen HP | CATALOG_ONLY |
| Archer | 11 | `SKILL_BOMB_SHOT` | Bomb Shot | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Archer | 12 | `SKILL_PERFORATION` | Perforation | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Archer | 13 | `SKILL_RECALL_WOLVERIN` | Recall Wolverine | `SUMMON` | Pet/Ofensivo | CATALOG_ONLY |
| Archer | 14 | `SKILL_EVASION_MASTERY` | Evasion Mastery | `PASSIVE` | Passivo | CATALOG_ONLY |
| Archer | 15 | `SKILL_PHOENIX_SHOT` | Phoenix Shot | `DAMAGE_AOE` | Dano ofensivo X1/area | READY |
| Archer | 16 | `SKILL_FORCE_OF_NATURE` | Force of Nature | `BUFF_GROUP` | Buff/Ofensivo | CATALOG_ONLY |
| Knight | 1 | `SKILL_SWORD_BLAST` | Sword Blast | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Knight | 2 | `SKILL_HOLY_BODY` | Holy Body | `BUFF_SELF` | Buff/Defensivo | CATALOG_ONLY |
| Knight | 3 | `SKILL_PHYSICAL_TRANING` | Physical Training | `PASSIVE` | Passivo | CATALOG_ONLY |
| Knight | 4 | `SKILL_DOUBLE_CRASH` | Double Crash | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Knight | 5 | `SKILL_HOLY_VALOR` | Holy Valor | `BUFF_GROUP` | Buff/Ofensivo em area | CATALOG_ONLY |
| Knight | 6 | `SKILL_BRANDISH` | Brandish | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Knight | 7 | `SKILL_PIERCING` | Piercing | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Knight | 8 | `SKILL_DRASTIC_SPIRIT` | Drastic Spirit | `BUFF_SELF` | Buff/Defensivo | READY |
| Knight | 9 | `SKILL_SWORD_MASTERY` | Sword Mastery | `PASSIVE` | Passivo | CATALOG_ONLY |
| Knight | 10 | `SKILL_DIVINE_INHALATION` | Divine Inhalation | `BUFF_SELF` | Buff/Defensivo | CATALOG_ONLY |
| Knight | 11 | `SKILL_HOLY_INCANTATION` | Holy Incantation | `DEBUFF_SINGLE` | Buff/Ofensivo | CATALOG_ONLY |
| Knight | 12 | `SKILL_GRAND_CROSS` | Grand Cross | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Knight | 13 | `SKILL_SWORD_OF_JUSTICE` | Sword of Justice | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Knight | 14 | `SKILL_GODLY_SHIELD` | Godly Shield | `BUFF_SELF` | Buff/Defensivo | CATALOG_ONLY |
| Knight | 15 | `SKILL_GOD_BLESS` | God Bless | `BUFF_GROUP` | Buff/Ofensivo | CATALOG_ONLY |
| Knight | 16 | `SKILL_DIVINE_PIERCING` | Divine Piercing | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Atalanta | 1 | `SKILL_SHIELD_STRIKE` | Shield Strike | `CONTROL` | Efeito em area | CATALOG_ONLY |
| Atalanta | 2 | `SKILL_FARINA` | Farina | `DAMAGE_SINGLE` | Dano ofensivo X1 | CATALOG_ONLY |
| Atalanta | 3 | `SKILL_THROWING_MASTERY` | Throwing Mastery | `PASSIVE` | Passivo | CATALOG_ONLY |
| Atalanta | 4 | `SKILL_VIGOR_SPEAR` | Vigor Spear | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Atalanta | 5 | `SKILL_WINDY` | Windy | `BUFF_SELF` | Buff/Ofensivo | READY |
| Atalanta | 6 | `SKILL_TWIST_JAVELIN` | Twist Javelin | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Atalanta | 7 | `SKILL_SOUL_SUCKER` | Soul Sucker | `DAMAGE_AOE` | Dano ofensivo em area + cura | CATALOG_ONLY |
| Atalanta | 8 | `SKILL_FIRE_JAVELIN` | Fire Javelin | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Atalanta | 9 | `SKILL_SPLIT_JAVELIN` | Split Javelin | `DAMAGE_SINGLE` | Dano ofensivo X1 | CATALOG_ONLY |
| Atalanta | 10 | `SKILL_TRIUMPH_OF_VALHALLA` | Triumph of Valhalla | `BUFF_GROUP` | Buff/Ofensivo | CATALOG_ONLY |
| Atalanta | 11 | `SKILL_LIGHTNING_JAVELIN` | Lightning Javelin | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Atalanta | 12 | `SKILL_STORM_JAVELIN` | Storm Javelin | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Atalanta | 13 | `SKILL_HALL_OF_VALHALLA` | Hall of Valhalla | `BUFF_GROUP` | Buff/Ofensivo | CATALOG_ONLY |
| Atalanta | 14 | `SKILL_X_RAGE` | X Rage | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Atalanta | 15 | `SKILL_FROST_JAVELIN` | Frost Javelin | `BUFF_SELF` | Buff/Ofensivo | CATALOG_ONLY |
| Atalanta | 16 | `SKILL_VENGEANCE` | Vengeance | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Priestess | 1 | `SKILL_HEALING` | Healing | `HEAL_SINGLE` | Defensivo X1 | READY |
| Priestess | 2 | `SKILL_HOLY_BOLT` | Holy Bolt | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Priestess | 3 | `SKILL_MULTISPARK` | Multi Spark | `DAMAGE_SINGLE` | Dano ofensivo X1 | CATALOG_ONLY |
| Priestess | 4 | `SKILL_HOLY_MIND` | Holy Mind | `DEBUFF_SINGLE` | Buff/Ofensivo X1 | CATALOG_ONLY |
| Priestess | 5 | `SKILL_MEDITATION` | Meditation | `PASSIVE` | Passivo (regen MP) | CATALOG_ONLY |
| Priestess | 6 | `SKILL_DIVINE_LIGHTNING` | Divine Lightning | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Priestess | 7 | `SKILL_HOLY_REFLECTION` | Holy Reflection | `BUFF_SELF` | Buff/Defensivo | CATALOG_ONLY |
| Priestess | 8 | `SKILL_GRAND_HEALING` | Grand Healing | `HEAL_AOE` | Defensivo em area | READY |
| Priestess | 9 | `SKILL_VIGOR_BALL` | Vigor Ball | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Priestess | 10 | `SKILL_RESURRECTION` | Resurrection | `UTILITY` | Defensivo X1 | CATALOG_ONLY |
| Priestess | 11 | `SKILL_EXTINCTION` | Extinction | `CONTROL` | Efeito em area | CATALOG_ONLY |
| Priestess | 12 | `SKILL_VIRTUAL_LIFE` | Virtual Life | `BUFF_GROUP` | Buff/Defensivo | READY |
| Priestess | 13 | `SKILL_GLACIAL_SPIKE` | Glacial Spike | `DAMAGE_AOE` | Dano ofensivo em area | CATALOG_ONLY |
| Priestess | 14 | `SKILL_REGENERATION_FIELD` | Regeneration Field | `BUFF_GROUP` | Buff/Defensivo em area (regen HP/MP) | CATALOG_ONLY |
| Priestess | 15 | `SKILL_CHAIN_LIGHTNING` | Chain Lightning | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Priestess | 16 | `SKILL_SUMMON_MUSPELL` | Summon Muspell | `SUMMON` | Buff/Defensivo | CATALOG_ONLY |
| Magician | 1 | `SKILL_AGONY` | Agony | `RESOURCE` | Ativo: HP para MP | READY |
| Magician | 2 | `SKILL_FIRE_BOLT` | Fire Bolt | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Magician | 3 | `SKILL_ZENITH` | Zenith | `BUFF_SELF` | Buff/Defensivo | READY |
| Magician | 4 | `SKILL_FIRE_BALL` | Fire Ball | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Magician | 5 | `SKILL_MENTAL_MASTERY` | Mental Mastery | `PASSIVE` | Passivo (MP) | CATALOG_ONLY |
| Magician | 6 | `SKILL_WATORNADO` | Watornado | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Magician | 7 | `SKILL_ENCHANT_WEAPON` | Enchant Weapon | `BUFF_TARGET` | Buff/Ofensivo | CATALOG_ONLY |
| Magician | 8 | `SKILL_DEAD_RAY` | Dead Ray | `DAMAGE_SINGLE` | Dano ofensivo X1 | READY |
| Magician | 9 | `SKILL_ENERGY_SHIELD` | Energy Shield | `BUFF_SELF` | Buff/Defensivo | CATALOG_ONLY |
| Magician | 10 | `SKILL_DIASTROPHISM` | Diastrophism | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Magician | 11 | `SKILL_SPIRIT_ELEMENTAL` | Spirit Elemental | `BUFF_SELF` | Buff/Ofensivo | READY |
| Magician | 12 | `SKILL_DANCING_SWORD` | Dancing Sword | `SUMMON` | Buff/Ofensivo | CATALOG_ONLY |
| Magician | 13 | `SKILL_FIRE_ELEMENTAL` | Fire Elemental | `SUMMON` | Pet/Ofensivo | CATALOG_ONLY |
| Magician | 14 | `SKILL_FLAME_WAVE` | Flame Wave | `DAMAGE_AOE` | Dano ofensivo em area | READY |
| Magician | 15 | `SKILL_DISTORTION` | Distortion | `DEBUFF_AOE` | Buff/Ofensivo em area | CATALOG_ONLY |
| Magician | 16 | `SKILL_M_METEO` | Meteor | `DAMAGE_AOE` | Dano ofensivo em area | READY |

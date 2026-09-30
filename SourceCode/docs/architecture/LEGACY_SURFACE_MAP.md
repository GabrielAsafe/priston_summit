# Summit Legacy Surface Map

Status baseline: RECOVERY_169 (`cbea4761b0250030b3b8b8989c2ea146bb88e6fc`)

## Purpose

This document records the boundary between modern Summit code and the frozen Priston Tale legacy kernel. It answers two questions:

1. Which RED/legacy entrypoints are still intentionally consumed by AMBER adapters?
2. Is any GREEN/modern consumer bypassing those adapters and reaching legacy code directly?

The target is **not** to wrap every legacy function in a one-to-one interface. The target is to keep modern feature development on contracts/ports/services while confining raw legacy types, globals, sockets, renderer handles and monolithic entrypoints to RED or AMBER.

## Current boundary proof

`python3 tools/architecture/validate_legacy_boundary.py --report-only` at RECOVERY_169 reports:

- GREEN files: **367**
- GREEN public headers: **203**
- AMBER files counted by the architecture guard: **149**
- GREEN legacy includes: **0**
- GREEN legacy symbols: **0**
- GREEN renderer includes: **0**
- GREEN renderer symbols: **0**
- GREEN direct socket sends: **0**
- allowlist entries: **0**
- unallowlisted violations: **0**

Therefore no currently-scanned GREEN path is allowed to bypass AMBER and directly consume the listed legacy categories.

The additional transport/ingress guards freeze direct legacy socket owners and feature-specific growth in `OnServer`/client `netplay`/server `netplay`. These guards intentionally allow historical RED internals to remain historical RED internals while preventing new modern dependencies on them.

## Layer model

```text
GREEN feature / service / UI
        |
        v
contract / port / repository / gateway
        |
        v
AMBER Legacy* adapter / runtime / protocol bridge
        |
        v
RED Legacy Kernel
(rsPLAYINFO, smCHAR, stage, inventory, quest handler, Winsock,
 DirectX/effects, record loaders, legacy globals and monoliths)
```

## Domain surface map

The table below lists the principal modern boundaries, their AMBER implementations and representative RED entrypoints/data they are allowed to touch. The RED column is deliberately a *surface*, not a request to refactor every named function.

| Domain | GREEN boundary / intent | AMBER implementation(s) | Intentional RED surface |
|---|---|---|---|
| Network send (server) | `IPacketSender`, `PacketView`, `SendResult` | `LegacyPacketSender` | `rsPLAYINFO::lpsmSock`, legacy `smWINSOCK::Send` |
| Network send (client gameplay) | `IClientCommandSender` | `LegacyClientCommandSender` | `smWsockServer->Send` |
| Network ingress | `PacketRouter` / protocol handlers | `LegacyGameplayPacketRouter` | legacy opcode packets, `OnServer` ingress, selected legacy handlers |
| Connection lookup | `IConnectionQuery` | `LegacyConnectionQuery` | `rsPLAYINFO`, socket/remote-address state |
| Audio | `IAudioService` | `LegacyAudioAdapter` | `sinPlaySound`, `CharPlaySound` |
| Animation | `IAnimationService` | `LegacyAnimationAdapter` | `smCHAR::SetMotionFromCode` |
| Visual effects | `IVisualEffectService` | `LegacyVisualEffectAdapter` | `StartEffect` and legacy effect runtime |
| Lighting | `ILightingService` | `LegacyLightingAdapter` | legacy dynamic-light/effect state |
| Texture | `ITextureService` | `LegacyTextureAdapter` | legacy texture/material loader/runtime |
| UI character stats | `ICharacterStatsGateway` | `LegacyCharacterStatsGateway` | `sinGetLife/Mana/Stamina`, `CheckCharForm`, inventory/equipment refresh |
| UI context | `IContextGateway` | `LegacyContextGateway` | legacy inventory, shop, trade, warehouse, coin-shop and message-window entrypoints |
| UI Companion | `ICompanionGateway` | `LegacyCompanionGateway` | legacy UI state + `IClientCommandSender` for wire send |
| UI Covenant | `ICovenantGateway` | `LegacyCovenantGateway` | legacy UI state + `IClientCommandSender` for wire send |
| UI Quest | `IQuestGateway` | `LegacyQuestGateway` | existing quest window/section operations |
| UI Skills | `ISkillGateway` | `LegacySkillGateway` | existing skill UI/runtime state |
| UI shell | UIV2 runtime/windows | `LegacyUIV2Bridge`, legacy window adapters | key polling, old HUD/window state, 2D drawing backend |
| Character persistence | `ICharacterRepository` | `LegacyCharacterRecordAdapter` | `rsLoadAccountCharacterRecord`, `TRANS_RECORD_DATA`, packed record decode |
| Online player resolution | `IPlayerQuery` / PlayerId boundary | `LegacyOnlinePlayerRegistry`, `LegacyPlayerAdapter` | `rsPLAYINFO*`, legacy online-player tables |
| Damage | `IDamageService` | `LegacyDamageAdapter` | `smCHAR` life/state/motion and legacy damage state |
| Healing | `IHealingService` | `LegacyHealingAdapter` | `srFindCharFromSerial`, `rsPlayHealing`, `smCHAR`/`rsPLAYINFO` life snapshots |
| Life commands | `ILifeCommands` | `LegacyLifeAdapter` | raw life/state/motion mutation on legacy entity |
| Status effects | `IStatusEffectService` | `LegacyStatusEffectAdapter` | legacy status arrays/timers/effect state |
| Targeting | `ITargetingService` | `LegacyTargetingAdapter` | legacy target pointers/coordinates, `smCHAR::SetNextTarget` |
| Threat | `IThreatService` | `LegacyThreatService` | legacy target/threat state |
| Combat buffs | `ICombatBuffService` | `LegacyCombatBuffAdapter` | legacy combat buff fields/timers |
| Skill runtime | `ISkillRuntime` | `LegacySkillRuntimeAdapter` | legacy skill/motion execution state |
| Monster query/commands | monster V2 contracts | `LegacyMonsterAdapter` | `srFindCharFromSerial`, `smCHAR` monster fields, follower conversion fields |
| Companion commands | `ICompanionCommands` | `LegacyCompanionCommandsAdapter` | `CompanionSystem::HandleCrystalUse` |
| Companion lifecycle | `ICompanionLifecycle` | `LegacyCompanionLifecycleAdapter` | `CompanionSystem::OnOwnerDisconnect` |
| Companion query | `ICompanionQuery` | `LegacyCompanionQueryAdapter` | legacy Companion runtime/state translated to snapshots |
| Companion UI ingress | `ICompanionUIIngress` | `LegacyCompanionUIIngressAdapter` | `CompanionSystem::HandleUIAction` |
| Covenant UI ingress | `ICovenantUIIngress` | `LegacyCovenantUIIngressAdapter` | Covenant legacy player/runtime state and packet response |
| Inventory core | `IInventoryQuery`, `IInventoryCommands` | `LegacyInventoryRuntime` | `rsFindInvenItem`, `rsAddInvenItem`, `rsDeleteInvenItem`, checked item grant |
| Item catalog | `IItemCatalog` | `LegacyItemCatalogAdapter` | legacy item definitions/catalog |
| Potions | `IPotionService`, potion stack contracts | `LegacyPotionAdapter`, `LegacyPotionStackAdapter` | `rsGetServerPotion`, `rsAddServerPotion`, offline potion persistence |
| Loot | `ILootService` | `LegacyLootAdapter` | `StageArea`, `STG_ITEMS`, `rsCompanionPickupStageItem` |
| Wallet | `IWalletQuery`, `IWalletCommands` | `LegacyWalletAdapter` | legacy gold/money fields and commit rules |
| Coin wallet | coin wallet contracts | `LegacyCoinWalletAdapter` | legacy coin persistence/account state |
| Trade | `ITradeService` | `LegacyTradeAdapter` | legacy trade session/item/gold transaction primitives |
| NPC shop | `IShopService` | `LegacyShopAdapter` | legacy shop buy/sell and inventory primitives |
| Personal shop | `IPersonalShopService` | `LegacyPersonalShopAdapter` | legacy personal-shop session, item/gold/coin/potion commit/logging |
| Crafting | `ICraftingService` | `LegacyCraftingAdapter` | `rsRecvCraftItem`, link core, wing, skin, smelting/manufacture handlers |
| Aging | `IAgingService` | `LegacyAgingAdapter` | `rsRecvAgingItem`, `rsRecvAgingUpgradeItem` |
| Mixing | `IMixingService` | `LegacyMixingAdapter` | `rsRecvMixtureResetItem` and legacy mixing packet data |
| Quest commands | `IQuestCommands` | `LegacyQuestCommands` | `cQuestHandler.StartQuest`, `CancelQuest`, `GetReward` |
| Quest progress | `IQuestProgressService` | `LegacyQuestProgressService` | `cQuestHandler.KillMonster`, Tier-5 kill handler |
| Quest view ingress | quest query/view contracts | `LegacyQuestQueryIngressAdapter`, `LegacyQuestViewQuery` | `cQuestHandler.GetSpoilerInfo`, `GetQuestInfo`, legacy DB/view data |
| Quest persistence | `IQuestRepository` | `LegacyQuestRepository` | legacy quest DB schema/rows and persistence format |
| Quest catalog/admin/lifecycle/objectives | V2 quest services | corresponding `LegacyQuest*` adapters | legacy definitions and mutation entrypoints, translated to V2 models |
| Progression | progression readers/gateways | `LegacyProgressionReaders`, `LegacyProgressionGateways` | legacy EXP/money/story state, `rsSendMoneyToUser`, old persistence readers |
| Harvest/Gathering | `IHarvestPlayerQuery`, `IGatheringService` | `LegacyHarvestPlayerQueryAdapter` | legacy player/world position/resource state |
| World query/stage | `IWorldQuery`, `IStageQuery` | `LegacyWorldAdapter`, `LegacyStageAdapter` | `STG_AREA`, stage globals and translated world state |
| Movement | `IMovementCommands` | `LegacyMovementAdapter` | `smCHAR::MoveAngle` and legacy position state |
| Teleport | `ITeleportService` | `LegacyTeleportAdapter` | legacy warp packet format, sent through `IPacketSender` |
| NPC interaction | `IInteractionService`, `INpcQuery` | `LegacyInteractionAdapter`, `LegacyNpcAdapter` | legacy NPC lookup/open entrypoints (for example Hell's Gate NPC open) |
| Spawn | `ISpawnService` | `LegacySpawnAdapter` | legacy spawn/monster creation primitives |
| World events | V2 admission/reward/event contracts | `LegacyWorldEventAdmissionAdapter`, `LegacyWorldEventRewardGateway`, `LegacyGameplayEventAdapter/Hooks` | old event globals, Bellatra/monster-defeated hooks, reward/entry rules |
| Pets | `IPetRuntime` | `LegacyPetRuntimeAdapter` | PC-bang entitlement, prime-item packet helpers and legacy socket delivery |
| Party | `IPartyQuery`, `IPartyCommands`, `IPartyRelayService` | `LegacyPartyQueryAdapter`, `LegacyPartyCommandsAdapter`, `LegacyPartyRelayRuntime` | `JoinParty`, `CreateParty`, `LeavePartyMaster`, `ReleaseParty`, `SecedeParty`, legacy membership links |
| Clan | `IClanQuery`, `IClanChatService`, `IClanUpdateNotifier` | `LegacyClanQueryAdapter`, `LegacyClanChatAdapter`, `LegacyClanUpdateNotifier` | legacy clan fields/user lookup; packet delivery goes through packet sender where migrated |
| Chat delivery | `IChatDeliveryService` | `LegacyChatDeliveryAdapter` | legacy player socket + historical chat packet format |
| Class content | class V2 registry/content source | `LegacyClassCatalog` / class adapters | legacy job/class constants and historical metadata |
| Skill content | skill V2 catalog/runtime | `LegacySkillCatalog`, `LegacySkillRuntimeAdapter` | historical skill tables/constants, legacy motion/skill runtime |

## Wide AMBER surfaces worth watching

These are legal AMBER, not GREEN violations, but they are broader than the small-domain ideal and should not grow casually:

- `SrcGame/src/systems/ui/v2/legacy/LegacyContextGateway.cpp` (~1.3k lines): intentionally bridges many old UI/commerce/inventory contexts.
- `SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp` (~1.1k lines): central compatibility bridge for old HUD/input/window state.
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestRepository.cpp` (~1k lines): owns substantial legacy DB translation/persistence logic.

The correct future pressure is to **prevent growth** or split by real domain authority when a consumer requires it, not to create one adapter per legacy function.

## What remains deliberately inside RED

The following categories remain legal inside the frozen kernel and are not required to be wrapped one-for-one:

- the large client/server `netplay` implementations and historical packet internals;
- `OnServer`/server legacy runtime internals;
- `smCHAR` implementation and legacy character state machine;
- old inventory/item/shop/trade implementations;
- renderer/effect engines and DirectX-era implementation details;
- legacy record loaders and compression formats;
- old HUD/message/help implementations;
- legacy clan/party internals where modern code only consumes the narrower V2 ports;
- other historical socket owners already captured by the direct-socket budget.

The RED rule is **freeze, do not proliferate**: internal RED-to-RED calls may remain, while new feature development must not make RED a public API again.

## Bypass conclusion

At RECOVERY_169, the architecture guard finds **zero** GREEN legacy symbols/includes, **zero** renderer leaks and **zero** direct socket sends. The allowlist is empty. That is the mechanical evidence that modern code is not currently bypassing the declared boundary classes covered by the guard.

This does not prove runtime/ABI behavior; native build and in-game certification are separate gates.

## Monolith audit

Run:

```bash
python3 tools/architecture/audit_legacy_monoliths.py --top-files 30 --top-functions 30
```

Important distinction: some source/include files exceed 15,000 lines, but the current heuristic audit found **no single function near 15,000 lines**. The largest probable function body in the active tree is approximately:

- `cITEM::ShowItemInfo` — **6,882 lines**
- `smCHAR::Main` — **3,511 lines**
- `cINVENTORY::RButtonDown` — **3,041 lines**
- server `RecvCommand` in shared OnServer legacy source — **2,989 lines**
- `HoEffectMgr::StartSkill` — **2,217 lines**

The largest files/includes include:

- `Shared/server/legacy_runtime/OnSever_legacy_server.inl` — **45,465 lines**
- `Shared/server/legacy_runtime/OnSever_legacy_client.inl` — **28,204 lines**
- `Shared/gameplay/character/character_legacy_client.inl` — **18,631 lines**
- `SrcServer/src/systems/gameplay/character/core/character.cpp` — **16,668 lines**
- `SrcGame/src/systems/network/client/netplay.cpp` — **15,427 lines**

## Safe annotation strategy for giant RED code

A comments-only readability pass is reasonable and materially safer than physically splitting these files/functions. The first pass should **not** move code, extract functions, rename symbols, reorder cases, alter scopes or change conditionals.

Recommended file/region marker:

```cpp
// =============================================================================
// LEGACY AREA: NETWORK / PACKET DISPATCH
// Responsibility: receive/route historical gameplay packets.
// Behavior: decodes legacy packet fields and dispatches to existing handlers.
// Side effects: player/session mutation, socket replies, logs as noted below.
// Modern boundary: LegacyGameplayPacketRouter / IPacketSender where applicable.
// Safety: preserve opcode values, ordering, fall-through and side effects.
// =============================================================================
```

Recommended marker inside a giant function:

```cpp
// -----------------------------------------------------------------------------
// AREA 04 — AUDIO / PRESENTATION SIDE EFFECTS
// Purpose: trigger historical sound/effect feedback for the action above.
// Inputs/state: existing legacy character/effect state.
// Side effects: audio/effect runtime only; no ownership transfer.
// -----------------------------------------------------------------------------
```

Good area names are responsibility-based rather than arbitrary line chunks, for example:

- INPUT / COMMAND DECODE
- NETWORK / PACKET DISPATCH
- SESSION / CONNECTION STATE
- CHARACTER STATE
- COMBAT / DAMAGE
- SKILLS / BUFFS
- INVENTORY / ITEMS
- TRADE / SHOP / ECONOMY
- QUEST / PROGRESSION
- PARTY / CLAN / SOCIAL
- WORLD / STAGE / TELEPORT
- AUDIO / PRESENTATION
- RENDER / EFFECTS
- PERSISTENCE / RECORDS
- LOGGING / SECURITY / ANTI-CHEAT
- CLEANUP / ERROR / ROLLBACK

### Safety gate for annotation recoveries

For every comments-only legacy annotation recovery:

1. compare the pre/post files after removing comments and whitespace;
2. require the resulting C/C++ token stream to be identical;
3. run the existing architecture validators;
4. keep each recovery limited to a small number of monoliths;
5. do **not** combine comments with gameplay/balance/network changes.

That makes the annotation pass reviewable and keeps the executable semantics unchanged by construction.

## Appendix — Legacy-named adapter implementation inventory

The following `Legacy*.cpp` implementation files are the explicit named adapter/content surface present at RECOVERY_169:

- `Shared/gameplay/classes/v2/content/LegacyClassCatalog.cpp`
- `Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp`
- `SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp`
- `SrcGame/src/systems/presentation/v2/legacy/LegacyAnimationAdapter.cpp`
- `SrcGame/src/systems/presentation/v2/legacy/LegacyAudioAdapter.cpp`
- `SrcGame/src/systems/presentation/v2/legacy/LegacyLightingAdapter.cpp`
- `SrcGame/src/systems/presentation/v2/legacy/LegacyTextureAdapter.cpp`
- `SrcGame/src/systems/presentation/v2/legacy/LegacyVisualEffectAdapter.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyCharacterStatsGateway.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyContextGateway.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyCovenantGateway.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyInventoryGateway.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyMessageWindow.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyNotificationHud.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyNpcDialogWindow.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyQuestGateway.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacySkillGateway.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacySkillNpcWindow.cpp`
- `SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp`
- `SrcServer/src/systems/commerce/v2/legacy/LegacyAgingAdapter.cpp`
- `SrcServer/src/systems/commerce/v2/legacy/LegacyCoinWalletAdapter.cpp`
- `SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingAdapter.cpp`
- `SrcServer/src/systems/commerce/v2/legacy/LegacyMixingAdapter.cpp`
- `SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp`
- `SrcServer/src/systems/commerce/v2/legacy/LegacyShopAdapter.cpp`
- `SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp`
- `SrcServer/src/systems/commerce/v2/legacy/LegacyWalletAdapter.cpp`
- `SrcServer/src/systems/community/chat/v2/legacy/LegacyChatDeliveryAdapter.cpp`
- `SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyCombatBuffAdapter.cpp`
- `SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyDamageAdapter.cpp`
- `SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyHealingAdapter.cpp`
- `SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyLifeAdapter.cpp`
- `SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.cpp`
- `SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyTargetingAdapter.cpp`
- `SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyThreatService.cpp`
- `SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionCommandsAdapter.cpp`
- `SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionLifecycleAdapter.cpp`
- `SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionQueryAdapter.cpp`
- `SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionUIIngressAdapter.cpp`
- `SrcServer/src/systems/gameplay/covenants/v2/legacy/LegacyCovenantUIIngressAdapter.cpp`
- `SrcServer/src/systems/gameplay/events/legacy/LegacyGameplayHooks.cpp`
- `SrcServer/src/systems/gameplay/events/v2/legacy/LegacyGameplayEventAdapter.cpp`
- `SrcServer/src/systems/gameplay/events/v2/legacy/LegacyWorldEventAdmissionAdapter.cpp`
- `SrcServer/src/systems/gameplay/events/v2/legacy/LegacyWorldEventRewardGateway.cpp`
- `SrcServer/src/systems/gameplay/gathering/v2/legacy/LegacyHarvestPlayerQueryAdapter.cpp`
- `SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyItemCatalogAdapter.cpp`
- `SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyPotionAdapter.cpp`
- `SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.cpp`
- `SrcServer/src/systems/gameplay/loot/v2/legacy/LegacyLootAdapter.cpp`
- `SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.cpp`
- `SrcServer/src/systems/gameplay/pets/v2/legacy/LegacyPetRuntimeAdapter.cpp`
- `SrcServer/src/systems/gameplay/player/v2/legacy/LegacyCharacterRecordAdapter.cpp`
- `SrcServer/src/systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.cpp`
- `SrcServer/src/systems/gameplay/player/v2/legacy/LegacyPlayerAdapter.cpp`
- `SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.cpp`
- `SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionReaders.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestAdminService.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestCatalog.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestCommands.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestLifecycleService.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestObjectiveService.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestProgressService.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestQueryIngressAdapter.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestRepository.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestStateQuery.cpp`
- `SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestViewQuery.cpp`
- `SrcServer/src/systems/gameplay/skills/v2/legacy/LegacySkillRuntimeAdapter.cpp`
- `SrcServer/src/systems/gameplay/world/v2/legacy/LegacyInteractionAdapter.cpp`
- `SrcServer/src/systems/gameplay/world/v2/legacy/LegacyMovementAdapter.cpp`
- `SrcServer/src/systems/gameplay/world/v2/legacy/LegacyNpcAdapter.cpp`
- `SrcServer/src/systems/gameplay/world/v2/legacy/LegacySpawnAdapter.cpp`
- `SrcServer/src/systems/gameplay/world/v2/legacy/LegacyStageAdapter.cpp`
- `SrcServer/src/systems/gameplay/world/v2/legacy/LegacyTeleportAdapter.cpp`
- `SrcServer/src/systems/gameplay/world/v2/legacy/LegacyWorldAdapter.cpp`
- `SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryRuntime.cpp`
- `SrcServer/src/systems/network/v2/legacy/LegacyConnectionQuery.cpp`
- `SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp`
- `SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.cpp`
- `SrcServer/src/systems/social/clan/v2/legacy/LegacyClanChatAdapter.cpp`
- `SrcServer/src/systems/social/clan/v2/legacy/LegacyClanQueryAdapter.cpp`
- `SrcServer/src/systems/social/clan/v2/legacy/LegacyClanUpdateNotifier.cpp`
- `SrcServer/src/systems/social/party/v2/legacy/LegacyPartyCommandsAdapter.cpp`
- `SrcServer/src/systems/social/party/v2/legacy/LegacyPartyQueryAdapter.cpp`
- `SrcServer/src/systems/social/party/v2/legacy/LegacyPartyRelayRuntime.cpp`

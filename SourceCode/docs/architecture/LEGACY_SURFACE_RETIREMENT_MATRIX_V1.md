# Legacy surface retirement matrix V1

`LEGACY_SURFACES_CLASSIFIED=11`
`CLASSIFIED_HIGH_AUTHORITY_RESIDUAL=ZERO_STATIC`
`VISUAL_RETIREMENT=BLOCKED_RUNTIME_PARITY`

This matrix separates **legacy shell** from **gameplay/business authority**. A
large function is not a bug by itself. Removal is only safe when the replacement
owns the same semantics and visual/runtime parity is proven.

| Legacy surface | Legitimate shell that may remain | Authority receiver / static proof | Retirement condition |
|---|---|---|---|
| `smCHAR::Main` | frame/timers, hydration, orchestration, presentation state | life/resource/status/target adapters; `validate_legacy_monolith_authority_residual_v1.py` | split lifecycle/presentation only after native + in-game parity |
| `cITEM::ShowItemInfo` | tooltip layout/text/legacy rendering | canonical item queries cross `IItemCatalog`; `validate_item_catalog_boundary_v1.py` | V2 tooltip reproduces item text, stats, colors and layout in-game |
| `cINVENTORY::RButtonDown` | pointer/input interpretation and legacy UI routing | inventory V2 boundary + server transaction authority; `validate_inventory_boundary_v1.py` | V2 inventory interaction parity, including shops/crafting/drop confirmation |
| `cSKILL::ShowSkillInfo` | skill tooltip rendering | UI V2 `SkillsPage` is the replacement presentation surface; combat authority is separately guarded | V2 skill tooltip/page visual + localization parity |
| `PlaySkillAttack` | client animation/request orchestration | attack-rating and animation authority guards reject direct owned-state writes | combat/skill runtime parity with V2 path for every skill family |
| `smCHAR::EventSkill` | event animation/orchestration | attack-rating and animation authority guards reject direct owned-state writes | event-skill runtime parity for all monster/player event paths |
| `RecvCommand` / `OnServer` | packet decode/routing, hydration and compatibility dispatch | monolith residual gate + packet/transport services | protocol handlers split only after native/network regression parity |
| `netplay.cpp` | protocol parsing/serialization compatibility | backend/socket transport-retired guards (`direct sends=0`) | protocol V2 coverage and live reconnect/world traffic parity |
| `sinTrade` | client trade window/input and legacy serialization shell | `validate_trade_exact_authority_v3.py` protects the coordinated server commit path | V2 trade UI + disconnect/replay/rollback runtime parity |
| `ClanChipLegacyCommon` | clan menu/layout/draw/input shell | clan query/mutation/chat/notifier/custom/packet boundaries | V2 clan UI visual/input parity; server authority is already outside the UI shell |
| `HoEffect` / `AssaParticle` | DX9-era effect graph/render execution | presentation renderer/animation/audio/texture/visual-effect boundaries for migrated consumers | effect-by-effect screenshot/runtime parity before retirement; never grep-delete the renderer shell |

## Static closure meaning

`CLASSIFIED_HIGH_AUTHORITY_RESIDUAL=ZERO_STATIC` means every business-authority
family listed above is either already delegated to a V2 service/adapter or has a
focused ownership guard preventing direct authority from returning. It does **not**
mean every legacy rendering/input function has been ported.

## Runtime-only remaining proof

Clan UI, inventory/tooltips/skill UI, HoEffect and AssaParticle require native
client execution and visual/input parity. These shells must remain until those
checks pass. The matrix intentionally blocks source-only retirement of those paths.

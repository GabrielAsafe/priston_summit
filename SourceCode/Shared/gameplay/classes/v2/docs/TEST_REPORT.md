# Playable Classes V2 — Test Report

## Scope

This report records tests available in the current Linux/host environment. It does **not** claim native MinGW i686 linkage or in-game runtime validation.

## Portable/domain tests

The V2 domain runner covers the existing gameplay foundations plus the playable-class migration:

- GameplayEventHub smoke;
- QuestObjective smoke;
- HuntingService smoke;
- ProfessionService smoke;
- GatheringService smoke;
- MiningService smoke;
- MiningInteractionService smoke;
- Condition/Reward/Modifier smoke;
- CovenantService smoke;
- StoryState smoke;
- PlayableClassRegistry parity/validation smoke;
- legacy class quest adapter smoke;
- legacy class commerce adapter smoke;
- legacy class onboarding adapter smoke;
- legacy class event-kit adapter smoke;
- Progression gateways smoke;
- Server progression condition-context smoke;
- class architecture guard.

The playable-class smoke compiles with the host compiler under C++14 with strict warnings (`-Wall -Wextra -Werror -pedantic`).

## Event-kit parity

The extraction of `CaixaItensRandom` type 11 and 14 class equipment bundles was compared against the original eight class switches before removal. The comparison covered **189 grants** and checked order, item code, specialization job, grant kind, and aging. The adapter smoke retains full-sequence signatures for all 16 `(kit, job)` combinations.

## Static architecture/build checks

- class architecture guard: PASS;
- every non-test `.cpp` under `Shared/gameplay/classes/v2` is required by the guard to appear in both client and server build manifests;
- client build-script shell syntax (`bash -n`): PASS;
- server build-script shell syntax (`bash -n`): PASS;
- modified tracked files preserve their baseline dominant EOL style;
- no real trailing whitespace was found in added lines/new source after CRLF-aware checking.

## Native validation still required

The current environment does not provide the project MinGW i686 toolchain. Before merge/release acceptance, run:

1. full client MinGW i686 compile + link;
2. full server MinGW i686 compile + link;
3. `Server.exe` startup and restart smoke;
4. create/select/login/relog for all eight existing classes;
5. model/head/hair/armor/equipment smoke;
6. OpenItem specialization + tooltip smoke;
7. motion `*APPLICABLE_JOB` smoke;
8. skill/rank/change-job regression;
9. party/ranking/kill/SOD/help presentation smoke;
10. optional legacy quest/event content smoke.

A portable PASS must not be interpreted as proof of DirectX-era ABI, packet/save compatibility, or native in-game behavior.

# Bugfix-source integration

This tree is the semantic integration of the Playable Classes V2 refactor into the current bugfix source (`srcAtual(6)`).

The integration deliberately ports the **class-system delta**, not the unrelated development-branch stack.

## Preserved bugfix behavior

Examples of overlapping bugfixes preserved during the three-way reconciliation:

- server login sends `PACKET_BUFFS`/new-player configuration before enabling `PLAYERNOVATO`;
- client start-field selection retains the `CharacterJob` / `NewCharInfo` fallback used when live `JOB_CODE` is temporarily zero;
- server-side account/password handling remains on the bugfix implementation;
- legacy UI V2-only message bridge code that is absent from this source was not imported.

## Deliberately not imported

The class port does not require and therefore does not import the development branch's:

- Covenant runtime;
- StoryState runtime;
- ProgressionRuntime composition root;
- ProgressionGateways;
- client UI V2 bridge.

Only the Playable Classes V2 implementation units are explicitly added to the client/server build manifests.

## Validation

Run:

```bash
bash Shared/gameplay/classes/v2/tests/run_class_system_tests.sh
bash -n build_client_mingw_i686_multicore.sh
bash -n build_server_mingw_i686_j8.sh
```

The host tests do not replace native MinGW i686 compile/link or in-game validation.

# Rebuild validation — R6

## Autoridade

- comportamento: `pre_refactor.zip`;
- estrutura: porte (`final_07`/histórico);
- exceção futura autorizada: SQLite server, ainda dormente.

## Cobertura funcional/estrutural

Ficheiros que tinham mudado no porte: **288**.

- `structured-safe`: **135**;
- `structured-exact`: **101**;
- `structured-macro-equivalent`: **10**;
- `structured-policy-equivalent`: **41**;
- `direct-exact-port-native`: **1** (`sinItem.cpp`, também monolítico no próprio porte);
- `oracle-wrapper`: **0**.

## Source graph

- baseline source manifest: **1204** ficheiros;
- baseline translation units: **420**;
- TUs removidas: **0**;
- TU nova autorizada: `SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp`.

Descoberta de fontes repo-local:

- client `SrcGame/src`: **201 baseline / 201 R6**;
- server `SrcServer/src`: **179 baseline / 180 R6**;
- diferença server: apenas SQLite autorizada.

## Preprocessing estrito

A auditoria R6 deixou de tratar `#pragma once` como detalhe cosmético. Foram encontrados e removidos **50 pragmas adicionais em wrappers target**, sem alterar bodies/helpers funcionais.

Resultados estritos:

```text
structured-safe strict preprocessor : 135/135 PASS
macro-equivalent strict             : 10/10 PASS
policy-equivalent strict            : 41/41 PASS
```

Assim, pragmas e include guards fazem parte da prova de equivalência.

## Grafo ativo de includes

- active Shared: **265**;
- ficheiros ativos novos vs baseline: **250**;
- novos fora de Shared: apenas `SQLiteConnection.cpp/.h`;
- quoted includes em `structured-exact`: **534 verificados, 0 mudanças de resolução**;
- quoted includes dentro de helpers Shared extraídos: **116 contextos, 0 mudanças de resolução**.

## Estrutura do porte

- ficheiros inventariados de `final_07`: **1517**;
- presentes em R6: **1517**;
- ausentes: **0**.

As abstrações/classes/scaffolding do porte permanecem presentes mesmo quando ficam dormentes para preservar o comportamento baseline.

## Validadores

```bash
python3 validate_oracle_parity.py
python3 validate_structural_shared_proof.py
python3 validate_structural_shared_include_resolution.py
python3 validate_structured_safe_parity.py
python3 validate_structured_safe_strict_preprocessor.py
python3 validate_structural_macro_equivalence.py
python3 validate_structural_macro_equivalence_strict.py
python3 validate_structural_policy_equivalence.py
python3 validate_structural_policy_equivalence_strict.py
python3 validate_active_shared_graph.py
python3 validate_structural_exact_includes.py
python3 validate_build_source_graph.py
python3 validate_active_nonbaseline_graph.py
python3 validate_port_structure_presence.py
python3 validate_build_scripts.py
python3 validate_full_source_parity.py

bash -n build_client_mingw_i686_multicore.sh
bash -n build_server_mingw_i686_j8.sh
bash -n link_client_mingw_i686.sh
bash -n link_server_mingw_i686.sh
```

## Auditoria adicional de preprocessing físico

- `#line` em helpers Shared ativos: **0**;
- BOM em helpers Shared ativos: **0**;
- ficheiros ativos com `\\` + newline: **7**; seis são headers Lua `baseline-shared-exact` e um é `OnSever_legacy_server.inl` `structured-exact`, todos congelados byte-a-byte;
- macros location-sensitive (`__FILE__`, `__BASE_FILE__`, `__INCLUDE_LEVEL__`) dentro de helpers novos/movidos ativos: **0**. O caso `smJpeg.cpp` mantém `__FILE__` no TU target original.

## Resultado

Todos os validadores acima: **PASS**.

## Limitação

Este ambiente não contém `i686-w64-mingw32-g++` nem as dependências externas completas do repositório real. R6 declara **paridade estática/estrutural**, não compile/link/runtime PASS. O primeiro build real deve ser tratado como teste de integração estrutural; qualquer erro deve ser investigado primeiro como include/declaration/linkage/source graph/preprocessing, sem corrigir comportamento legacy.

# R6.1 — primeiro build real / compiler adaptation

Primeiro build real do cliente R6:

```text
TOTAL : 203
OK    : 202
FAIL  : 1
```

Único erro reportado:

```text
cE_chip.cpp: too many arguments to SetFontTextColor(COLORREF)
```

O erro é reproduzível diretamente no `pre_refactor`: `cE_chip.cpp` chama `SetFontTextColor(HDC,COLORREF)`, enquanto a declaração disponível é somente `SetFontTextColor(COLORREF)`. Foi adicionada uma adaptação local, exclusiva de MinGW, que encaminha a forma de dois argumentos para `SetTextColor(HDC,COLORREF)` sem alterar o call site legacy ou o caminho MSVC.

Classificação atual dos 288 paths geridos:

```text
135 structured-safe
101 structured-exact
 10 macro-equivalent
 40 policy-equivalent
  1 compiler-adaptation (MinGW cE_chip)
  1 direct-exact-port-native
---
288
```

Validador novo:

```bash
python3 validate_compiler_adaptations.py
```

Ele verifica:

- hash exato do source adaptado;
- hash exato do oracle `pre_refactor`;
- presença única do bloco `__MINGW32__/__MINGW64__` autorizado;
- que a remoção desse bloco recupera byte-a-byte o source estrutural R6 anterior à adaptação;
- que o caminho legacy `SetFontTextColor(dc,color)` permanece presente;
- que o mapping MinGW é somente `SetTextColor(dc,color)`.

`validate_full_source_parity.py` passou a executar também esta prova.

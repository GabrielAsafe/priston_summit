# Auditoria de risco do port Visual Studio -> MinGW/Fedora

## Conclusão executiva

Há evidência concreta de diferenças relevantes entre o build histórico do `Game.exe` e o build MinGW atual. Nem toda diferença implica uma feature perdida, mas algumas merecem revisão prioritária porque podem alterar comportamento em runtime sem causar erro de compilação/link.

### Achados de maior risco

1. **Endereço-base/ASLR do executável mudou.** O PDB histórico contém a linha de link do Visual Studio com `/BASE:0x870000 /DYNAMICBASE:NO /FIXED /SAFESEH:NO`. O linker MinGW atual não replica `ImageBase=0x870000` nem desabilita relocação/ASLR explicitamente. Para um cliente com `Hook`, `memmap`, `cracker`, `AntiCheat` e código legado de baixo nível, isto é um risco alto se houver endereços absolutos, tabelas de função por endereço ou patches de memória.
2. **Cinco fontes `Server/*.cpp` que estavam no Game antigo são hoje excluídas explicitamente do client MinGW:** `ClientFuncPos.cpp`, `OnSever.cpp`, `PartySever.cpp`, `Svr_Damge.cpp`, `svrEventSpawn.cpp`.
3. **Cinco unidades `modules/Helper/Utils/*.cpp` estavam no Game antigo e não são selecionadas pelo build atual do client:** exception, filesystem, logging, string e testing. Os headers desses utilitários ainda aparecem na árvore atual.
4. **Quatro implementações matemáticas de NewEffect e `STRCLASS.CPP` existiam no build antigo, mas não aparecem na seleção atual:** `COLOR3.CPP`, `COLOR4.CPP`, `MATRIX4.CPP`, `POINT3.CPP`, `STRCLASS.CPP`.
5. **O port introduziu unidades que não existiam no PDB antigo:** `client_server_stubs.cpp`, `skill_missing_globals_mingw.cpp`, `d3dx_font_alias.cpp`, `mingw_debug_entry.cpp`, `UICombobox.cpp`, `UIItemFrame.cpp`, `stdafx.cpp`. Em especial, os nomes `client_server_stubs` e `skill_missing_globals_mingw` indicam áreas que precisam ser revisadas para saber se o port substituiu lógica real por no-ops/valores artificiais.

## Reconstrução do build histórico pelo Game.pdb

O `Game.pdb` contém 206 objetos do projeto:

- 199 fontes em `SrcGame/src`;
- 7 fontes em `modules`;
- total: 206 objetos de projeto.

Os sete módulos externos detectados no PDB são:

- `modules/Helper/Utils/utils_exception.cpp`
- `modules/Helper/Utils/utils_filesystem.cpp`
- `modules/Helper/Utils/utils_logging.cpp`
- `modules/Helper/Utils/utils_string.cpp`
- `modules/Helper/Utils/utils_testing.cpp`
- `modules/MD5/MD5.cpp`
- `modules/MD5/MD5Wrapper.cpp`

O build MinGW atual do client só adiciona fontes de `SrcGame/src` e `modules/MD5`, portanto os cinco `Helper/Utils` não entram mais no client por essa regra.

## Fontes antigas não selecionadas pelo client MinGW atual

### Ainda presentes, mas explicitamente excluídas

- `Server/ClientFuncPos.cpp`
- `Server/OnSever.cpp`
- `Server/PartySever.cpp`
- `Server/Svr_Damge.cpp`
- `Server/svrEventSpawn.cpp`

O PDB antigo não mostra apenas os objetos: também contém símbolos de lógica server-side no `Game`, como `rsTRANS_SERVER`, `rsAddClientFuncPos`, `rsGetClientPos`, `rsRecvMixtureResetItem` e outros. Isto prova que a arquitetura histórica era mais monolítica. Não prova sozinho que todas essas rotinas eram executadas em gameplay, mas elimina a hipótese de que eram simplesmente arquivos alheios ao projeto do Visual Studio.

### Não encontrados na árvore SrcGame auditada

- `HoBaram/NewEffect/math/COLOR3.CPP`
- `HoBaram/NewEffect/math/COLOR4.CPP`
- `HoBaram/NewEffect/math/MATRIX4.CPP`
- `HoBaram/NewEffect/math/POINT3.CPP`
- `STRCLASS.CPP`

Os headers `color4.h` e `point3.h` ainda aparecem no projeto atual. Deve-se confirmar se as implementações foram tornadas inline/movidas ou se funções foram descartadas.

### Módulos Helper não compilados pelo client atual

- `utils_exception.cpp`
- `utils_filesystem.cpp`
- `utils_logging.cpp`
- `utils_string.cpp`
- `utils_testing.cpp`

O server MinGW atual varre `modules` inteiro, portanto a omissão é especificamente do client.

## Unidades novas no port MinGW

Comparando por nome de translation unit, aparecem no client atual e não aparecem no PDB histórico:

- `client_server_stubs.cpp`
- `d3dx_font_alias.cpp`
- `mingw_debug_entry.cpp`
- `sinbaram/skill_missing_globals_mingw.cpp`
- `UICombobox.cpp`
- `UIItemFrame.cpp`
- `stdafx.cpp`

As duas primeiras categorias a revisar são os **stubs de server** e os **globals ausentes de skill**, porque podem mascarar perda funcional: o linker deixa de reclamar, mas o jogo pode receber valores padrão ou não executar a lógica histórica.

## Diferenças de link importantes

### Visual Studio histórico (extraído do Game.pdb)

A linha de link contém, entre outras opções:

- `/BASE:0x870000`
- `/DYNAMICBASE:NO`
- `/FIXED`
- `/SAFESEH:NO`
- `/SUBSYSTEM:WINDOWS`
- manifest `asInvoker`

### MinGW atual

O script atual usa:

- `-m32`
- `-mwindows`
- `-Wl,--gc-sections`
- `-Wl,--subsystem,windows`

Não há equivalente explícito a `--image-base 0x870000` nem a desabilitação de dynamic base/relocations.

### Risco

Se qualquer parte do cliente usa endereços hardcoded, patching, trampoline/hook ou assume offsets de imagem históricos, esta é provavelmente a diferença mais perigosa encontrada. O fato de o código conter módulos de hook, memmap, anti-cheat e segurança aumenta a prioridade da verificação, embora não prove por si só dependência em endereço absoluto.

## Bibliotecas: antigo x atual

O PDB histórico mostra ligação explícita com `Lua.lib` e `Lualib.lib`. O port atual usa somente `-llua`. Isto pode estar correto se a biblioteca MinGW `liblua.a` incorporar também as funções da biblioteca padrão; deve ser verificado pelos símbolos (`luaopen_*`, `luaL_openlibs`).

Também estavam presentes no link histórico e não aparecem literalmente no atual:

- `atls.lib`
- `winspool.lib`
- `wsock32.lib`

`ws2_32` normalmente cobre a API Winsock usada por aplicações modernas, portanto `wsock32` é risco baixo. `atls.lib` merece revisão se funcionalidades COM/ATL antigas tiverem sido removidas durante o port. O PDB contém símbolos de inicialização ATL, portanto ATL realmente estava presente no executável histórico.

`d3dx9.lib` histórico versus `d3dx9_43` atual provavelmente representa a mesma geração do runtime do DirectX SDK June 2010 e não é, isoladamente, evidência de perda.

## Diferenças de compilação

O PDB analisado é de uma configuração Debug e registra, entre outras flags:

- `-DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS`
- C++17
- RTTI desativado (`-GR-`)
- packing 8 em registros do toolchain (`-Zp8`)
- floating point preciso (`-fp:precise`)

O client MinGW usa:

- `-DWIN32 -D_WINDOWS -D_M_IX86 -Di386 -D__i386__ -DNDEBUG -D_CLIENT`
- `-std=gnu++17`
- `-fpermissive`

A diferença `_DEBUG`/`NDEBUG` é esperada porque o PDB é Debug. Porém `_MBCS` não está preservado. Em um jogo legado com strings multibyte/idiomas asiáticos, vale procurar condicionais `#ifdef _MBCS` e uso de CRT MBCS.

O Visual Studio histórico também tinha uma configuração chamada `Gandalf` além de Debug/Release. Os scripts MinGW atuais não possuem equivalente. Sem `src/priston.vcxproj` não é possível saber quais defines, libs ou exclusões o perfil Gandalf ativava; isto permanece uma lacuna real da auditoria.

## Server

O `Server.sln` confirma configurações Debug/Gandalf/Release e x86/x64, mas só o `.sln` foi fornecido. Não foi fornecido `Server.pdb` nem `src/priston.vcxproj`, então não é possível reconstruir a lista histórica de translation units do Server com o mesmo nível de certeza usado para o Game.

O linker MinGW atual do server também possui um problema de robustez: a chamada ao linker termina em `|| true`, descartando o status de saída e decidindo sucesso apenas pela existência de `Server.exe`. Isto deve ser corrigido para que uma falha parcial nunca seja tratada como build válido.

## Priorização

### P0 — verificar antes de considerar o port funcionalmente equivalente

1. ImageBase/ASLR/fixed image versus código de hooks/endereços absolutos.
2. Conteúdo de `client_server_stubs.cpp`.
3. Conteúdo de `sinbaram/skill_missing_globals_mingw.cpp`.
4. Motivo da exclusão dos cinco `Server/*.cpp` do client, comparando referências/símbolos.

### P1

5. `modules/Helper/Utils/*.cpp` fora do client.
6. `STRCLASS.CPP` e quatro `.CPP` de NewEffect/math removidos/movidos.
7. `_MBCS` e comportamento de encoding/locale.
8. Lua: confirmar que `-llua` contém o equivalente a `Lua.lib + Lualib.lib`.

### P2

9. ATL (`atls.lib`).
10. Configuração Visual Studio `Gandalf`.
11. `--gc-sections` versus registro indireto/hardcoded de funções.

## Veredito

**Sim: há sinais concretos de que o port pode ter perdido ou substituído comportamento importante.** Ainda não há evidência suficiente para afirmar que 15 arquivos correspondem a 15 features quebradas, porque parte pode ter sido movida, tornada inline ou deliberadamente removida. Mas há pelo menos três divergências que justificam investigação imediata: os cinco sources server-side retirados do Game, os stubs/globals introduzidos pelo port e a mudança do layout de imagem do executável.

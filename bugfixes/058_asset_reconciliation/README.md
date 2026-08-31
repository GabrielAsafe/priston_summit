# FIX 058 — Asset Reconciliation

Status: **CLOSED**

## Objetivo

Restaurar assets ausentes do cliente Summit Tale usando somente origens históricas confiáveis, preservando os assets já existentes e validando os casos críticos em runtime.

Raízes históricas autorizadas usadas durante a investigação:

* `/home/gabriel/armazem/priston/ExMachina`
* `/home/gabriel/armazem/priston/pristontale-1-master`
* `/home/gabriel/Área de Trabalho/priston-official`

  * `ptbr-antigo`
  * `ptbr-novo`

Também foi auditado posteriormente o cliente oficial instalado em:

* `C:\Zenit Games\Priston Tale Brasil`

## PROVADO

### Sincronização `ptbr-antigo`

Foi executada uma sincronização missing-only do cliente `ptbr-antigo` para o cliente Summit Tale.

Resultado:

* 3.754 arquivos ausentes restaurados.
* 766.832.664 bytes restaurados.
* 0 arquivos existentes sobrescritos.
* 0 blockers.
* 1.102 arquivos existentes em path exato mas com conteúdo diferente foram preservados.
* 28 arquivos existentes por variante de case mas com conteúdo diferente foram preservados.
* Após a sincronização: `MISSING_COPY=0`.

Entre os assets restaurados estavam:

* 208 arquivos de `Items/Defense`.
* 203 de `Weapon`.
* 96 de `Accessory`.
* 1.519 de outros itens.

A restauração ampla resolveu no runtime a ausência das armaduras e de outros conteúdos do cliente.

Manifest principal:

`evidence/ptbr_antigo_missing_sync/apply_manifest.tsv`

### Trajes de futebol

Foram restaurados e aprovados em runtime:

* `itDA147.bmp`
* `itDA153.bmp`
* `itDA247.bmp`
* `itDA253.bmp`

O formato físico usado foi o BMP protegido já conhecido pelo projeto.

Resultado do teste no jogo:

Os quatro trajes passaram a aparecer corretamente.

Evidência:

`evidence/football_costumes_runtime.sha256`

### Conversões DDS → TGA

Foram restaurados os seguintes assets:

* `image/Sinimage/Premium/Pet_terry-pose.tga`
* `image/Sinimage/Premium/Pet_nepsys-pose.tga`
* `image/Sinimage/Premium/Pet_io-pose.tga`
* `image/Sinimage/Premium/Pet-mute-pose.tga`
* `image/Sinimage/Premium/Vampiric_EX.tga`
* `image/Sinimage/skill/keep/Booster_01.tga`
* `image/Sinimage/skill/keep/Booster_02.tga`
* `image/Sinimage/skill/keep/Booster_03.tga`

As conversões foram validadas fisicamente quanto a dimensão, formato e integridade.

O smoke test individual de todos os oito assets não faz parte do critério de fechamento deste fix.

### BI283

Origem histórica:

`ExMachina/client/image/Sinimage/Items/Premium/itBI283.dds`

SHA-256 da origem:

`20c64fb21c28943223010a98ca4b12c1a70614f036b59d800cf0f22ea6fa8fd1`

O DDS 22x22 foi restaurado para o formato BMP protegido 32bpp correspondente aos vizinhos reais da família `itBI`.

SHA-256 final:

`bda291e7688d857c6c6f2b4affa0ba050dadefc1c7ecf680c71c2564cc5e3e49`

Runtime aprovado:

* `BI283` passou a gerar `MATCH`.
* `itBI283.bmp` deixou de gerar `IMAGE_FAIL`.

### BI330

Origem histórica:

`ExMachina/client/image/Sinimage/Items/Premium/itBI330.dds`

SHA-256 da origem:

`341ae500c6ed8cdd231cc7064b8472a5a9978a3ef6bcc597f61f2bdf54b2d25d`

Foi restaurado usando o mesmo pipeline de família `itBI` validado no runtime.

Runtime aprovado:

* `BI330` passou a gerar `MATCH`.
* `itBI330.bmp` deixou de gerar `IMAGE_FAIL`.

## Pesquisa dos itens de TimeShop / ItemShop

Após todas as restaurações aprovadas, o runtime ainda registrou 69 ícones ausentes.

Foram pesquisados por path exato, stem, extensão alternativa, aliases e referências semânticas em:

* Summit Tale atual.
* ExMachina.
* pristontale-1-master.
* priston-official.
* ptbr-antigo.
* ptbr-novo.
* cliente oficial instalado em `C:\Zenit Games\Priston Tale Brasil`.

O cliente oficial também retornou:

`UNRESOLVED=69`

Portanto esses assets não foram encontrados como arquivos físicos soltos em nenhuma das origens pesquisadas.

## PENDING — 69 ícones de TimeShop / ItemShop

Estes itens permanecem conhecidos e explicitamente fora do escopo de fechamento do FIX 058.

### Skins — 57

* SK101
* SK102
* SK103
* SK107
* SK108
* SK109
* SK110
* SK111
* SK112
* SK113
* SK114
* SK115
* SK116
* SK117
* SK172
* SK173
* SK174
* SK175
* SK176
* SK181
* SK182
* SK183
* SK184
* SK185
* SK186
* SK187
* SK201
* SK202
* SK203
* SK204
* SK205
* SK221
* SK222
* SK223
* SK224
* SK231
* SK232
* SK233
* SK234
* SK242
* SK243
* SK244
* SK245
* SK246
* SK247
* SK248
* SK249
* SK253
* SK254
* SK255
* SK256
* SK259
* SK265
* SK266
* SK267
* SK268
* SK269

### Premium — 10

* CX113
* RS101
* RS102
* RS103
* RS104
* RS105
* RS106
* RS107
* RS108
* BI142

### Accessory — 2

* OA150
* OR250

Total:

`57 + 10 + 2 = 69`

Status:

**PENDING / NÃO BLOQUEIA FIX 058**

Os itens são reais e existem no catálogo/OpenItem do servidor, mas nenhuma origem confiável para seus ícones físicos foi encontrada.

## REJEITADO

Foram explicitamente rejeitadas as seguintes abordagens:

* fabricar ícones para eliminar `IMAGE_FAIL`;
* gerar imagens arbitrárias;
* usar um ícone semelhante apenas pelo nome do item;
* copiar `SE106..SE112` para `RS102..RS108`;
* considerar os 69 itens como falha da sincronização `ptbr-antigo`;
* continuar indefinidamente a investigação desses 69 dentro deste fix.

Os pares `SE/RS` foram investigados e são semanticamente diferentes.

Exemplo:

* `RS102`: Pedra de Respec MS
* `SE106`: Mechanician Spec

Portanto os assets `SE` não são substitutos comprovados para os `RS`.

## Política adotada

Durante o FIX 058:

* arquivos existentes no Summit não foram sobrescritos pela sincronização ampla;
* restaurações automáticas exigiram origem histórica confiável;
* SHA-256 foi usado para evidência de conteúdo;
* conversão de formato somente ocorreu nos casos especificamente investigados;
* o runtime foi usado como árbitro final nos casos críticos;
* assets sem origem suficientemente provada não foram inventados.

## Resultado final

**FIX 058 = CLOSED**

Foram recuperados os assets cuja origem pôde ser suficientemente provada e as correções críticas passaram por validação real no jogo.

Conhecido para investigação futura:

**69 ícones de TimeShop/ItemShop permanecem PENDING e não bloqueiam o fechamento deste bug.**


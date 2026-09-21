# Asset Load System V2

## Objetivo

O Asset Load System V2 isola a política de residency do legado D3D9 sem alterar a ABI de `smTEXTUREHANDLE` nem o formato dos assets. Ele foi criado para permitir o uso do corpus enhanced sem transformar cada textura 4x em uma alocação permanente dentro de um processo PE32.

A fonte enhanced no disco continua intacta. O runtime escolhe uma representação residente compatível com o orçamento e com o device.

## Problemas que o sistema resolve

1. registry global antigo tratava `lpD3DTexture == NULL` como “tentar novamente para sempre”;
2. `ReadTextures()` global materializava recursos de subsistemas não relacionados durante stage load;
3. texturas enhanced podiam ser decodificadas/residentes em dimensões incompatíveis com o espaço de endereçamento de um cliente 32-bit;
4. falhas de criação não tinham estado explícito nem retry budget;
5. ownership de efeitos/AssaRes/stage objects tinha caminhos assimétricos;
6. binds legacy assumiam que a textura já estava residente e impediam eviction segura;
7. buffers de decode podiam lançar `std::bad_alloc`.

## Arquitetura

### Registry sidecar

`Shared/rendering/assets/v2/AssetLoadRegistry.h` mantém estado fora de `smTEXTUREHANDLE`:

- `UNLOADED`
- `LOADING`
- `LOADED`
- `FAILED_TRANSIENT`
- `FAILED_PERMANENT`
- `WAITING_DEVICE`
- `EVICTED`

Também guarda tentativas, dimensões source/resident, bytes estimados, owners, serial LRU e estado de prewarm.

### Compatibilidade legacy

Os materiais continuam guardando `smTEXTUREHANDLE*`. A diferença é o bind:

`AssetLoadV2GetResidentTexture(handle)` garante residency sob demanda. Se a textura tiver sido expulsa por LRU, ela é recriada antes do `SetTexture`/`DrawTexture`.

Por isso o cache pode liberar a representação D3D sem invalidar o handle/material legacy.

### Validação e retry policy

Assets com dimensão zero, dimensão source acima de 8192 ou pixel count acima de `8192*8192` são marcados `FAILED_PERMANENT` antes da criação D3D.

Falhas transitórias (open/read/create/OOM) têm no máximo três tentativas. Depois disso o slot torna-se `FAILED_PERMANENT` até o último owner liberar o handle e o slot ser registrado novamente.

Perda/reset do device não consome esse orçamento. O slot entra em `WAITING_DEVICE` e só volta a `UNLOADED` no restore do device, evitando transformar uma textura saudável em falha permanente durante `D3DERR_DEVICELOST`/`D3DERR_DEVICENOTRESET`.

Isso impede retry storms como o caso de `ArchivoTmp.tga` sem punir eventos globais do device.

### Runtime adaptation

Defaults:

- source máximo aceito: `8192x8192` / 8192² pixels;
- textura gerida pelo registry: máximo residente 4096 por eixo;
- textura raw/offscreen: máximo residente 2048 por eixo;
- budget do registry: 512 MiB;
- global prewarm: até 64 tentativas por chamada;
- scoped/material-group prewarm: até 128 tentativas por chamada;
- large texture threshold: 16 MiB RGBA estimado;
- transient retry limit: 3.

Texturas grandes usam DXT quando possível:

- alpha-capable: DXT5;
- opaque: DXT1.

Se o working set não comportar uma textura, o sistema expulsa LRU. Se a própria textura ainda não couber, reduz progressivamente a resolução residente sem alterar a imagem enhanced no disco.

### Raw/offscreen assets

`LoadDibSurfaceOffscreen()` entrega `LPDIRECT3DTEXTURE9` diretamente ao código legacy. Esses ponteiros não entram no LRU porque callers podem mantê-los sem um handle indireto. O caminho recebe:

- validação estrutural;
- `nothrow` decode buffer;
- cap runtime separado (2048 por padrão);
- DXT para recursos grandes quando seguro;
- fallback legacy se DXT falhar.

### Stage loading

Stage transitions não chamam mais o process-wide `ReadTextures()` para carregar tudo que qualquer sistema registrou. O fluxo é:

1. trim do working set para 75% do budget;
2. prewarm limitado do `smMaterialGroup` do stage;
3. demais assets carregam sob demanda no renderer.

O prewarm scoped também é limitado para evitar trocar um sweep process-wide por um sweep enorme dentro de um único stage/material group.

### Ownership/lifetime

- `HoEffectResTexture` libera o material global no destructor do client;
- `HoEffectResManager` não mantém entradas que falharam ao carregar;
- `AssaRes` libera materiais/patterns e zera o catálogo antes de rescan;
- `smSTAGE_OBJECT` inicializa e destrói `Pattern` e `BipPattern` simetricamente;
- material slots dinâmicos liberados podem ser reutilizados.

Não é feito `HoEffectResManager::Clear()` automático em map transition: effects vivos guardam pointers para esses recursos. O residency LRU já reduz o custo das texturas sem criar dangling pointers.

## Configuração opcional

Variáveis de ambiente:

- `SUMMIT_TEXTURE_BUDGET_MB` — 128..2048 MiB;
- `SUMMIT_TEXTURE_MAX_DIM` — 512..8192;
- `SUMMIT_EXTERNAL_TEXTURE_MAX_DIM` — 256..8192;
- `SUMMIT_TEXTURE_PREWARM_LIMIT` — 0..512 (`0` desativa prewarm global; demand loading continua funcionando);
- `SUMMIT_TEXTURE_SCOPED_PREWARM_LIMIT` — 0..1024 (`0` desativa prewarm de material groups; demand loading continua funcionando).

Os defaults são os valores suportados e devem ser usados antes de tuning baseado em telemetria real.

## Telemetria

- `LOGS/asset_load_v2_telemetry.txt` — registry/residency/budget;
- `LOGS/asset_load_v2_events.txt` — adaptations, evictions e external loads;
- `LOGS/texture_pipeline_rejects.txt` — assets permanentemente rejeitados;
- `LOGS/large_texture_memory_policy.txt` — decisões DXT;
- `LOGS/mingw_missing_texture_paths.txt` — paths ausentes.

## Invariantes

1. `smTEXTUREHANDLE` não recebe campos novos.
2. enhanced source nunca é regravado pelo runtime.
3. `FAILED_PERMANENT` não é tentado em cada frame.
4. eviction só ocorre em handles indiretos do registry.
5. raw/offscreen pointers nunca são expulsos por trás do owner.
6. server não depende do Asset Load V2 client-side.
7. LAA + runtime adaptation + DXT + bounded residency são camadas complementares, não alternativas.

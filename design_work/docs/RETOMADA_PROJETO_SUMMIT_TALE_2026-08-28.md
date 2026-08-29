# Retomada do Projeto — Summit Tale Asset Tool

**Atualizado em:** 28/08/2026  
**Backend base mais recente:** `pt_asset_tool` V0.4.7 (Multi-Object Object-Local Preview V2)  
**CLI:** `pt_asset.py`

## 1. Objetivo da 1.0

A 1.0 deve cobrir o funcionamento base real do cliente Summit Tale para personagens jogáveis, NPCs, mobs, armas/items, efeitos e mapas. O escopo inclui SMD Model 0.62 rigid/static, SMB, INX moderno/legacy/outliers, BMP/TGA proprietários, Stage 0.72, object-local animation, multilayer TexLink, LOD, materiais, texturas, rigging e animações existentes. Os 18 outliers raros conhecidos e os 2 TGA realmente anômalos continuam dentro da 1.0.

A unidade de engenharia é **TECHNICAL COMPATIBILITY FAMILY**, não um asset isolado. Também permanece a regra: **FORMAT COMPATIBILITY != ART ASSEMBLY COMPATIBILITY**.

## 2. Invariantes do projeto

- Não modificar SMD/INX/SMB originais durante gates de inspeção.
- `client_modified=false` nos gates Blender atuais.
- Certificações são append-only em `certifications/assets.jsonl`; FAIL histórico nunca é apagado.
- Resolvers e validators devem falhar fechados quando a evidência é ambígua.
- Não usar filename/diretório como binding quando existe ou é necessária evidência técnica real.
- Timing de preview Blender não equivale a timing do runtime: `PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED`.

## 3. Estado do backend

O backend unificado é `pt_asset_tool`, comandado por `pt_asset.py`. O levantamento canônico contém aproximadamente 21.865 registros e 18.537 assets. Os principais diretórios de evidência são:

- `logs/asset_similarity_v1_1/`
- `logs/asset_family_refine_v2/`
- `profiles/`
- `certifications/assets.jsonl`
- `pt_build/pt_asset_tool/`

## 4. Funcionalidades já comprovadas

### 4.1 Texturas e materiais proprietários

- TGA protegido: codec generalizado, com cobertura estrutural ~99,93%; dois TGA anômalos continuam outliers específicos.
- BMP protegido: decode/rebuild byte-identical em 214/214 amostras, incluindo 8/24/32 bpp, palette, dimensões variadas, prefixo e trailing bytes.
- Texture overlay para Blender sem alterar o cliente.
- Preparação de textura considera materiais realmente usados por faces; slots históricos serializados não derrubam o gate quando comprovadamente não utilizados.

### 4.2 INX / SMB

- INX `MODERN_172` e `LEGACY_120` reconhecidos.
- SMB aceita trailing payload legítimo depois do último object block quando o formato real exige.
- `asset motions` lista MotionInfo e motions ativas.

### 4.3 Rigid animation bridge — fechado visualmente

O bridge source → Blender foi corrigido usando `matrix_basis` derivado parent-first em vez de atribuir pose absoluta diretamente. A semântica de quaternion nulo é identidade e existe continuidade de sinal.

Representantes certificados:

- Monbagon: ATTACK, WALK, STAND, DAMAGE.
- Ahin: STAND, WALK.
- Acero: STAND, WALK, RUN, DAMAGE, ATTACK, SKILL, DEAD.

A exceção Monbagon STAND usa guard estreito Neck/Head; não generalizar esse guard.

Timing/runtime speed ainda não está certificado.

### 4.4 Config-only → root skeleton binding — fechado para família modular normal

A descoberta decisiva foi o `LinkFile` serializado no próprio INX e sua validação por `link_code/file_code`.

Exemplo real:

`CmfbB02.smd → Cmb002.inx → M2Bip.inx → m2.smb → tfbD01.smd`

Na família `char/tmABCD`, 1374/1375 rigid config-only convergem deterministicamente. `ar0nDrac.smd` permanece ambíguo por apontar para três roots diferentes e deve continuar fail-closed.

### 4.5 Player body + cabeças independentes + promotion tiers

As cabeças são peças SMD independentes, animadas pelo mesmo root/motion da classe. Para a Atalanta foram comprovadas as três cabeças-base B01/B02/B03 e os cinco estados normais de promoção `base/a/b/c/d`.

Visualmente comprovado com `CmfbB02 + M2Bip/m2.smb`:

- B01: STAND e ATTACK PASS.
- B02: ATTACK PASS.
- B03: ATTACK PASS.
- B01 PROMOTION_2 (multiobjeto): ATTACK PASS.
- B01 PROMOTION_4 (tier ornamentado): ATTACK PASS.

O sufixo `t` continua `SPECIAL_T`, fora da progressão normal até existir prova do significado.

### 4.6 Static Model 0.62 weapons/items roundtrip — fechado

O gate prova edição real:

`SMD → Blender → witness de vértices → rebuild SMD → reparse → reimport Blender`.

O writer de certificação altera somente os XYZ registrados pelo witness e o comparador exige que todo byte fora desses campos permaneça inalterado.

Representantes:

- `itPR101.smd`
- `itWA104.smd`
- `itWH118.smd` (caso multi-material)

Geometria, faces, UVs, materiais e roundtrip foram aprovados. O cliente permanece intocado.

### 4.7 Mixed-Physique — fechado por ausência certificada no corpus

Auditoria dos 3.148 SMD Model 0.62 canônicos:

- 1.952 rigid normais (+ 1 variante/damaged rigid)
- 1.195 static
- 0 mixed

Nenhum arquivo possui mistura `0 < physique_objects < object_count`. O importer deve continuar detectando e bloqueando um mixed futuro em vez de classificá-lo silenciosamente.

### 4.8 Object-local animation single-object — fechado semanticamente

Foram identificados 432 static Model0.62 com arrays object-local; 218 são single-object e 214 multi-object.

Semântica comprovada:

- quaternions `TmRot` são deltas locais incrementais;
- `TmPrevRot` contém rotação acumulada e é a verdade usada no preview;
- POS e SCALE são interpolados por frame;
- preview é normalizado pelo primeiro keyframe;
- timing continua não certificado.

Representantes:

- `Test.smd`: ROT visual PASS; material não usado como evidência porque houve FAIL de textura.
- `Web.smd`: POS+SCALE visual/material PASS.
- `crystal_ani01.smd`: ROT+POS visual/material PASS.
- `itds103.smd`: HOLD corretamente detectado.

O roundtrip de geometria preserva os arrays object-local byte a byte.

## 5. Investigação atual — V0.4.7 multi-object object-local

O pacote físico `PT_OBJECT_LOCAL_MULTI_REPS.zip` trouxe três representantes e SourceCode relevante.

### Semântica comprovada no SourceCode

- `smPAT3D::LinkObject()` resolve `NodeParent` e gera `TmSort` pai-primeiro.
- `smPAT3D::TmAnimation()` avalia objetos nessa ordem.
- `smOBJ3D::TmAnimation()` compõe filho como `qmat * pParent->TmResult`.
- filho sem curva própria usa bind local derivado de `Tm * pParent->TmInvert`.

### Representantes

- `bong.smd`: 3 objetos independentes, todos animados ROT+POS.
- `3ani-04.smd`: 2 objetos independentes, ambos ROT.
- `At3_SplitJavelin40.smd`: `wt108` animado + filho estático `bip SJ01` que herda o pai.

### V0.4.7 implementado localmente

- parser lê `Tm`, `TmInvert` e `NodeParent`;
- resolução de parent case-insensitive, validação de ciclo e grafo explícito;
- preview anima múltiplas roots independentes;
- recria parenting no Blender preservando world placement;
- filho estático/HOLD herda animação do pai;
- continua fail-closed para **filho animado que também possui curvas próprias**, pois falta exemplar físico;
- normaliza separadores repetidos em paths de textura; isso corrige `Test.smd`, cujo material aponta para `Effect\\ObjAnimationData\\Test\\TEST1.tga`; a textura física `Test1.tga` é RGBA 64×8 azul/ciano, não branca;
- `TmFrameCnt > 0` segmentado também permanece pendente.

Testes locais de parser, fake Blender orchestration e caso negativo passaram. Falta o gate visual no Blender real.

## 6. Roadmap restante para 1.0

1. **Fechar object-local multi-object:** validar V0.4.7 no Blender real; encontrar exemplar de filho animado com curvas próprias; tratar `TmFrameCnt > 0` se existir no corpus físico.
2. **Multilayer TexLink:** parser/representação Blender/writer e gates reais.
3. **Stage 0.72:** os 63 registros conhecidos; leitura, representação, edição/build e validação.
4. **Outliers raros:** 18 registros D_NEW_REVERSE_ENGINEERING + 2 TGA anômalos.
5. **Publish/restore transacional genérico:** `CLIENT_DRIFT`, backup verificado, rollback e recusa de overwrite desconhecido.
6. **Runtime/timing + regressão final:** velocidade real, full-client regression e gates finais de release 1.0.

## 7. Hipóteses antigas que NÃO devem ser reintroduzidas

- V0.2.8 STAND container anchor: errado; não reutilizar.
- Same-INX root inference V0.3.0: não resolveu config-only real.
- Structural-match global V0.3.1: permissivo demais.
- Full skeleton signature V0.3.2–0.3.4: abstração errada para peças modulares.
- Physique numeric-index subset V0.3.5: índices não são comparáveis globalmente entre skeletons.
- Literal config refs V0.3.6: `Cmb002.inx` não expunha o root dessa forma.
- Pairing assembly graph V0.3.7: componente de `Cmb002` isolado; não era a camada correta.
- A solução correta para config-only veio de `INX LinkFile + link_code/file_code`.

## 8. Como retomar se o chat deixar de funcionar

1. Trabalhar sempre a partir do patch/base mais novo, atualmente V0.4.7.
2. Manter `pt_asset_tool`, `pt_asset.py`, `logs`, `profiles`, `certifications` e `pt_build` intactos.
3. Ler este arquivo antes de criar qualquer patch.
4. Não apagar FAILs históricos de `certifications/assets.jsonl`.
5. Não modificar o cliente durante gates de inspeção.
6. Continuar pela família técnica atual, não por assets aleatórios.

### Prompt de retomada recomendado

> Estamos continuando o projeto Summit Tale Asset Tool. Leia `RETOMADA_PROJETO_SUMMIT_TALE_2026-08-28.md` inteiro antes de alterar código. A base mais recente é V0.4.7. O rigid animation bridge, config-only root binding, player heads/tiers, static weapons/items roundtrip, mixed-Physique corpus audit e object-local single-object já estão fechados nos limites descritos. A investigação atual é object-local multi-object. `bong.smd`, `3ani-04.smd` e `At3_SplitJavelin40.smd` são os representantes físicos; o SourceCode prova `NodeParent`, ordem pai-primeiro e composição `qmat * parent.TmResult`. V0.4.7 implementa roots independentes e pai animado + filho static/HOLD; filho animado com curvas próprias e `TmFrameCnt>0` continuam fail-closed. Sempre explique e crie o patch na mesma resposta quando houver evidência suficiente. Use a versão mais nova como base, não enfraqueça validators, preserve certificações append-only e não modifique o cliente.

## 9. Comandos imediatos após instalar V0.4.7

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"

python3 pt_asset.py validate object-local \
  Effect/ObjAnimationData/PikeWind/bong.smd

python3 pt_asset.py validate object-local \
  Effect/AssaEffect/Skill3Mesh/At3_SplitJavelin40.smd

python3 pt_asset.py validate object-local \
  Field/forest/3ani-04.smd
```

Esperado: movimentos de múltiplas peças sem deformação; no SplitJavelin, `bip SJ01` deve acompanhar `wt108` como filho.

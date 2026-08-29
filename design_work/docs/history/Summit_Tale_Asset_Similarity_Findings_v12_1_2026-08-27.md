# Summit Tale — Asset Similarity Findings
## Source of Truth v12.1 — levantamento canônico de 2026-08-27

## 1. Correção metodológica do Survey V1.1

O relatório bruto original escaneou simultaneamente:

- cliente canônico;
- `design_work`;
- backups/archives.

Por isso números como 2.095 SMD rigid incluíam cópias históricas.

Esta v12.1 usa somente arquivos cujo path pertence ao cliente canônico:

```text
Summit Tale Client Full/cliente
```

Arquivos deep-inspected no cliente: **21,865**.

---

## 2. Cobertura técnica real

| Tier | Arquivos | % |
|---|---:|---:|
| A_REUSE_CORE_NOW | 13,891 | 63.53% |
| B_REUSE_WITH_ADAPTER | 7,893 | 36.10% |
| C_SEPARATE_FAMILY | 63 | 0.29% |
| D_NEW_REVERSE_ENGINEERING | 18 | 0.08% |

Conclusão:

```text
A + B + C = 21,847 / 21,865
          = 99.92%

somente D = 18 arquivos
          = 0.08%
```

Portanto o problema do projeto **não é descobrir milhares de formatos**.
É automatizar profiles/adapters e certificar famílias.

---

## 3. SMD — resultado decisivo

SMD no cliente: **3,216**.

Reconhecidos como top-level já conhecido:

```text
SMD Model data Ver 0.62 : 3,148
SMD Stage data Ver 0.72 : 63
total reconhecido        : 3,211 / 3,216
                         : 99.84%
```

Apenas **5** SMD possuem header desconhecido.

Dentro de Model0.62:

```text
rigid Physique : 1,952
static         : 1,195
variant        : 1
```

### Rigid Physique

Distribuição:

```text
CHARACTER   1,393
MOB         392
NPC         102
MAP hint    38
EFFECT      26
```

**1,869/1,952 (95.75%)** dos SMD rigid são referenciados
diretamente por INX.

Todos os **392 mobs rigid** encontrados possuem referência INX.
Dos **102 NPC rigid**, 96 possuem referência INX.

Isso torna a família personagem/NPC/mob a candidata mais forte para
`Auto Profile Generator`.

### Static Model0.62

Total: **1,195**.

```text
WEAPON_ITEM 604
MAP_STAGE   355
EFFECT      223
```

Só armas/itens já representam **604** SMD.

A conclusão correta é criar um branch:

```text
SMD Model0.62 static / no Physique
```

e não uma nova pipeline de armas.

---

## 4. INX → SMD → SMB

INX canônicos: **2,046**.

Todos os 2,046 foram reconhecidos como:

```text
INX_MODELINFO_FIXED_GROUP_LAYOUT
```

Model reference:

```text
resolve para .smd físico:
2,045 / 2,046
= 99.95%
```

Existe somente um model ref sem SMD físico resolvido pelo padrão atual.

Motion:

```text
INX com motion_ref : 549
motion_ref → .smb  : 549
coverage           : 100.00%
```

Ou seja:

```text
TODOS os motion_ref encontrados
resolvem para um SMB de mesmo path/stem.
```

Isso é evidência forte de que:

```text
INX
→ model .ASE nominal → physical .SMD
→ motion .ASE nominal → physical .SMB
```

pode ser automatizado em escala.

---

## 5. SMB

SMB no cliente: **958**.

```text
Biped signal conhecido : 952
low-signal/outlier      : 6
coverage                 : 99.37%
```

A frente SMB deve passar para **skeleton-family clustering**, não
reverse engineering arquivo por arquivo.

---

## 6. BMP

BMP no cliente: **10,783**.

Reconhecidos como standard ou DIB-after-prefix14:

```text
10,771 / 10,783
= 99.89%
```

Protected prefix14:

```text
prefix exato já runtime-proven : 391
outros prefix14               : 3,675
```

Os 3.675 variantes usam **183 prefixos diferentes**.

Isso não deve ser tratado como 183 formatos.
A hipótese mais forte é:

```text
mesma família:
14 bytes proprietários por arquivo
+ DIB válido em offset 14
```

Nosso método de preservação do prefixo original por arquivo deve ser
testado em poucos representantes desses clusters.

---

## 7. TGA — maior oportunidade de adapter

TGA no cliente: **4,538**.

```text
standard                  : 1,816
nonstandard/protected     : 2,722
```

O TGA não-standard é o maior cluster ainda pouco caracterizado.

Próximo trabalho de textura deve ser:

```text
cluster header/footer de TGA
→ 3–5 representantes
→ generalized protected-TGA adapter
```

e não testar milhares de TGAs.

---

## 8. Stage/maps

Stage0.72 encontrados:

```text
63
```

Isso confirma que maps devem continuar em vertical slice separada.

Ao mesmo tempo existem centenas de `SMD Model0.62 static` dentro de diretórios
de Field. Esses provavelmente são objetos/props/animated meshes usados por maps,
não o stage container em si.

Portanto:

```text
MAP SYSTEM
├── Stage0.72 world/stage
└── Model0.62 static props/animations
```

precisa ser tratado como duas camadas.

---

## 9. Os únicos 18 outliers D

- `char/tmABCD/ar0nass.smd`
- `char/tmABCD/at_dragon.smd`
- `char/tmABCD/CartolaFS.smd`
- `char/tmABCD/CartolaMG.smd`
- `char/tmABCD/CartolaPS.smd`
- `char/tmABCD/CartolaSH.smd`
- `Effect/AssaEffect/force.bmp`
- `Effect/sky/s.bmp`
- `game/maps/chrselect/ship_bottom.bmp`
- `game/maps/chrselect/ship_crest.bmp`
- `game/maps/chrselect/ship_door.bmp`
- `game/maps/chrselect/ship_floor.bmp`
- `game/maps/chrselect/ship_frame.bmp`
- `game/maps/chrselect/ship_nest.bmp`
- `game/maps/chrselect/ship_nest_part.bmp`
- `game/maps/chrselect/ship_pillar.bmp`
- `game/maps/chrselect/ship_rope.bmp`
- `game/maps/chrselect/ship_sides.bmp`

Observação importante:

`at_dragon.smd` possui header Model0.62 e o único erro detectado foi:

```text
object table: MBHB19
object head:  MBHB18
```

Isso pode ser legado tolerado/inconsistência nominal, não necessariamente um
novo formato.

Os quatro `Cartola*.smd` e `ar0nass.smd` são candidatos a formato especial de
accessory/head item.

Os 12 BMP unknown concentram-se quase todos em character-select/ship e efeitos.

Esses 18 itens **não devem bloquear** a automação das famílias principais.

---

## 10. Roadmap atualizado

### S2 — Auto Profile Generator — PRIORIDADE 1

Entrada:

```text
INX + SMD + SMB
```

Saída:

```text
family id
model path
motion/skeleton path
object list
material count
texture refs
LOD groups
Physique mode
structural signature
runtime representative certification
```

Cobertura potencial imediata: milhares de configs/assets.

### S3 — Batch certification de SMD rigid

Escolher representantes por subfamília, não por filename:

```text
1 playable character diferente
1 NPC
2–3 mobs de estruturas distintas
```

Se passarem:

```text
SMD_MODEL_062_RIGID_PHYSIQUE
→ FAMILY CERTIFIED
```

### W1 — Static Model0.62 adapter — PRIORIDADE 2

Primeiro representative:

```text
weapon/drop-item
1 object
1 material
no Physique
```

Depois ampliar para:

```text
multi-object weapons
effects
Field static animation objects
```

### T2 — generalized protected texture families

```text
BMP prefix14 variants
TGA protected/nonstandard clusters
```

### MAP1 — Stage0.72

Somente depois de separar claramente:

```text
stage container
vs
Model0.62 map props
```

### OUT1 — 18 outliers

Baixa prioridade.

---

## 11. Nova política de cobertura

Não usar:

```text
runtime test para cada asset
```

Usar:

```text
FORMAT FAMILY
→ SEMANTIC SUBFAMILY
→ representative runtime test
→ family certification
→ structural regression for remaining members
```

Estados futuros:

```text
FORMAT_MATCH
STRUCTURAL_MATCH
SUBFAMILY_CERTIFIED
RUNTIME_REPRESENTATIVE
OUTLIER
```

---

## 12. Conclusão

O levantamento muda o tamanho real do problema.

Não temos:

```text
~20 mil problemas técnicos
```

Temos aproximadamente:

```text
poucas macro-famílias
+
algumas subfamílias semânticas
+
18 outliers reais no scan atual
```

O investimento feito em Pike/Priestess foi reutilizável:
ele atingiu exatamente a família binária dominante do cliente.

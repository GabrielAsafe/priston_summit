Retomada do Projeto — Summit Tale Asset Tool
Objetivo da 1.0

A definição de 1.0 ficou ampliada: ela deve cobrir o funcionamento base real do cliente, não apenas os casos mais comuns.

A 1.0 precisa contemplar personagens jogáveis, NPCs, mobs, armas/items, efeitos e mapas, incluindo SMD Model 0.62 rigid/static/mixed-Physique, SMB existente, INX moderno/legacy/outliers, BMP/TGA proprietários, Stage 0.72, object-local animation, multilayer TexLink, LOD, materiais, texturas, rigging e animações existentes.

Também fazem parte do escopo os 18 outliers raros conhecidos e os 2 TGA realmente anômalos.

Upgrade genérico de textura para resoluções maiores continua interessante, mas não é prioridade para fechar a 1.0.

Filosofia atual

A unidade de engenharia deixou de ser “um personagem específico”.

Agora trabalhamos por:

TECHNICAL COMPATIBILITY FAMILY

O objetivo é provar uma família com representantes suficientes e depois reaproveitar o core nos demais assets compatíveis.

Também mantemos a regra:

FORMAT COMPATIBILITY != ART ASSEMBLY COMPATIBILITY

O G7 provou isso quando a geometria continuava estruturalmente válida, mas os braços se separaram do tronco por transformação independente de peças conectadas.

Backend atual

Foi criado o primeiro backend unificado:

pt_asset_tool

O CLI principal é:

pt_asset.py

Versão atualmente instalada:

0.2.1

Ele já possui AssetRegistry, resolver físico case-sensitive/case-insensitive controlado, CertificationRegistry, BlenderRunner, structural validation, visual validation e animation validation.

O backend usa os levantamentos existentes em:

logs/asset_similarity_v1_1/

e:

logs/asset_family_refine_v2/

O último doctor executado no seu PC encontrou:

21.865 registros canônicos

18.537 assets registrados

e reconheceu corretamente os 10 pares do cliente que diferem apenas por casing.

O cliente nunca é modificado durante esses gates Blender:

client_modified=False

Sistema de certificação

Agora cada asset pode ter certificações independentes.

Exemplos:

STRUCTURAL

VISUAL_GEOMETRY

VISUAL_TEXTURE_MATERIAL

VISUAL_RIG

VISUAL_ANIMATION_*

VISUAL_ASSET

Isso evita considerar um asset completamente aprovado apenas porque uma parte dele funcionou.

O histórico é append-only em:

certifications/assets.jsonl

Um FAIL antigo não é apagado quando ocorre uma correção. O PASS posterior fica registrado como evidência mais recente.

Validação visual Blender

O fluxo atual já funciona assim:

asset → resolve → structural gate → Blender → inspeção humana → y/n/s → certificação

Para geometria, o programa cria automaticamente um witness visível.

Ele escolhe uma mesh, seleciona alguns vértices e desloca-os radialmente aproximadamente 12% da diagonal do bounding box.

Isso modifica somente o .blend de teste.

Não modifica SMD, INX, SMB ou cliente.

Para animação, ao contrário, a geometria fica pristine. Não existe witness de vértices durante validate animation.

Assets visualmente validados
Monbagon

Asset:

char/monster/Monbagon/Monbagon-C.smd

Família:

SMD_MODEL_062_RIGID_PHYSIQUE

Structural:

PASS

Geometry visual:

PASS

O teste inicial provou modelo/rig estático e pipeline visual.

Ahin

Asset:

char/npc/Ahin/Ahin.smd

No backend V0.1.0 abriu sem textura.

Isso revelou que o importer V3 tratava TGA como arquivo normal, enquanto os TGAs do Ahin eram proprietários/protegidos.

Foi criado o overlay temporário de TGA protegido.

Nenhum arquivo do cliente é modificado.

Após o patch V0.1.1:

Geometry:

PASS

Texture/material:

PASS

Visual asset:

PASS

O FAIL anterior ficou preservado no histórico.

Acero

Asset:

char/monster/Acero/Acero.smd

O primeiro teste falhou antes do Blender witness.

A causa foi identificada no INX: o Acero possui objeto apenas no grupo High, enquanto o backend inicialmente forçava Default.

Foi criado:

LOD=auto

com prioridade:

default → high → low

conforme os grupos realmente disponíveis.

Depois da correção:

Structural:

PASS

Geometry:

PASS

Texture/material:

PASS

Visual asset:

PASS

LOD escolhido:

high

TGA protegido

O codec generalizado encontrou o esquema aditivo de header.

Cobertura estrutural do levantamento:

2720 / 2722

ou:

99,926525%

Modos reconhecidos:

FULL18 = 2615

KEEP_00_16 = 105

Os quatro representantes físicos usados no desenvolvimento tiveram decode → reprotect byte-identical.

Ahin também provou que o adapter consegue alimentar corretamente o Blender com TGA protegido decodificado.

Os dois TGA anômalos continuam pendentes para tratamento individual na 1.0.

Um deles apresenta header essencialmente zerado.

O outro, quando submetido ao codec conhecido, revela conteúdo semelhante a HTML em vez de imagem TGA válida.

Eles não serão ignorados; precisam de adapters/outlier handling específicos.

BMP protegido

O adapter foi generalizado além das antigas limitações 256×256/24-bit.

Testes locais:

214 / 214 PASS

Cobre atualmente:

8 bpp com palette

24 bpp

32 bpp

dimensões variadas

prefixo proprietário preservado

palette preservada

trailing bytes preservados

O teste usado foi decode → rebuild sem alteração, exigindo resultado byte-identical.

INX

Agora temos duas famílias estruturais principais comprovadas.

MODERN_172

Arquivo típico:

95.268 bytes

MotionInfo:

172 bytes

LEGACY_120

Arquivo típico:

67.084 bytes

MotionInfo:

120 bytes

Os 14 INX legacy encontrados no cliente passaram a identificação/teste.

Também foram testados cinco controles modernos.

O caso especial GM-001f.inx não representa um terceiro layout; ele possui comportamento legacy particular nos dados estendidos.

SMB

O parser genérico foi corrigido para não exigir:

last skeleton object end == EOF

Alguns SMB reais possuem payload depois do último object block.

Exemplos encontrados:

Vampiric

SN-002

MN-005

MN-009

Esse trailing payload é legítimo.

A regra estrita de EOF continua válida onde apropriado para SMD Model0.62; não foi relaxada globalmente.

Auto Profile

O gerador foi evoluído para usar casing físico real do inventário e não paths normalizados de relatórios auxiliares.

Também passou a distinguir texturas realmente usadas por faces de slots históricos apenas serializados.

Em representantes reais:

11 / 11 PASS

O próximo estágio maior ainda é resolver automaticamente os modelos config-only contra seus skeleton roots sem adivinhação.

Universo descoberto

No cliente canônico foram inspecionados:

21.865 registros.

Distribuição inicial:

A_REUSE_CORE_NOW:

13.891

B_REUSE_WITH_ADAPTER:

7.893

C_SEPARATE_FAMILY:

63

D_NEW_REVERSE_ENGINEERING:

18

Portanto A+B+C cobre aproximadamente:

99,92%

Os 18 da categoria D continuam explicitamente dentro do escopo da 1.0 por decisão de projeto.

SMD Model0.62

Foram encontrados:

3.148 Model0.62

Destes:

1.952 rigid Physique

1.195 static

1 variante/damaged

Além disso:

63 Stage0.72

e 5 SMD com header diferente/não reconhecido inicialmente.

Rigid já possui forte evidência em character, NPC e mob.

Static possui adapters iniciais e probes funcionando especialmente em weapons.

Weapons/static

Representantes testados:

itPR101

itWA104

itWH118

Os três passaram o capability probe static.

O levantamento posterior também encontrou static em weapons, effects e maps.

Alguns maps/effects revelaram duas capacidades ainda pendentes:

OBJECT_LOCAL_ANIMATION

MULTILAYER_TEXLINK

Essas capacidades continuam obrigatórias para a 1.0.

Mixed-Physique

Ainda não está fechado.

É uma das famílias raras que precisa de identificação, parser/importer adequado, writer correspondente e gates estruturais/Blender/runtime.

Não será tratado como “unsupported” na 1.0.

Stage0.72

Ainda é uma vertical separada.

Header:

SMD Stage data Ver 0.72

Existem 63 registros Stage0.72 identificados no levantamento.

Precisamos implementar leitura, representação Blender, edição, build e validação dos dados de mapa.

Também precisamos fechar os objetos animados associados a mapas e multilayer texture links.

Stage0.72 permanece requisito da 1.0.

Estado da animação

O backend V0.2 adicionou:

asset motions

e:

validate animation

Ele resolve:

INX

SMB

MotionInfo

motion selecionada

frames source

skeleton

e cria Actions no Blender.

Para Monbagon foram encontradas:

WALK

STAND

ATTACK

DAMAGE

A Action ATTACK é reconhecível visualmente como um ataque.

Isso é uma evidência importante de que:

INX motion selection

e:

sequência temporal básica do SMB

estão no caminho correto.

Mas a animação ainda NÃO está certificada.

Blocker atual — animação explode

Primeiro teste ATTACK:

Rig:

FAIL

Animation:

SKIP

Texture/material:

não verificada inicialmente.

Foi encontrada uma primeira falha na semântica de quaternion nulo.

O evaluator estava tratando um operando nulo como:

(0,0,0,0)

quando o comportamento correto é identidade:

(0,0,0,1)

Isso foi corrigido na V0.2.1.

Também foram adicionados:

continuidade de sinal quaternion

interpolação LINEAR

proteções adicionais no baker

Depois disso, você repetiu o ATTACK.

Resultado visual:

a animação continua reconhecível como ataque;

as texturas estão corretas;

o corpo começa correto;

deforma/explode durante o movimento;

e volta ao estado correto no final.

Resultado registrado:

VISUAL_RIG = FAIL

VISUAL_ANIMATION = SKIP

VISUAL_TEXTURE_MATERIAL = PASS

VISUAL_ANIMATION_ASSET = FAIL

Portanto o erro restante não parece ser seleção da motion nem leitura grosseira dos frames.

O principal suspeito agora é:

SOURCE POSE SPACE → BLENDER POSE BONE SPACE

mais especificamente composição entre local/world/parent space e bind matrices.

O próximo trabalho NÃO deve alterar novamente o parser INX sem evidência.

Também não devemos assumir que o SMB está errado só porque Blender explode.

Precisamos comparar o transform aplicado pelo runtime source com o transform final aplicado aos pose bones no Blender.

Próxima investigação de animação

A investigação deve isolar um bone por vez.

Para alguns frames do ATTACK, especialmente início, meio onde explode e fim, precisamos calcular lado a lado:

runtime/source world matrix

parent runtime world matrix

derived local runtime matrix

SMD/SMB bind world matrix

Blender edit bone matrix

Blender pose matrix pretendida

Blender pose_basis

resultado final após hierarchy

O objetivo é encontrar exatamente onde a convenção diverge.

Como o personagem volta ao normal no final da motion, há forte indicação de erro de composição durante os frames intermediários, e não necessariamente corrupção de bind pose.

Não devemos avançar para validar WALK/STAND antes de corrigir esse problema, porque apenas produziríamos mais FAILs do mesmo adapter.

Estado dos materiais

Já foram comprovados anteriormente no runtime:

SelfIllum

TwoSide

BlendType

BlinkColor / UseState

Além disso, Shade é serializado e deve ser preservado.

Transparency/order/Z-write têm evidência estrutural/source, mas não devem ser promovidos além do nível efetivamente provado.

Segurança de publish/restore

Toda modificação futura do cliente deve continuar passando por backup transacional verificado.

Regra fundamental:

workspace == source AND client == source

→ UNCHANGED

workspace == source AND client == known managed build

→ restoration permitida

client SHA desconhecido

→ CLIENT_DRIFT

→ recusar overwrite

O backend visual atual não modifica o cliente.

Organização atual do design_work

A raiz foi limpa.

Ficaram principalmente:

cores ativos

assets-base

profiles

bridge/restore

logs

builds

Blender workspaces

Os experimentos antigos foram movidos para:

archives/

e:

history_py/

A intenção é não voltar a espalhar um novo script especializado na raiz para cada teste.

Novas capacidades devem entrar prioritariamente dentro de:

pt_asset_tool/

Direção de produto

O objetivo final é que o usuário não precise conhecer internamente SMD/SMB/INX ou scripts específicos.

Exemplo desejado:

python3 pt_asset.py assets list

python3 pt_asset.py asset inspect <asset>

python3 pt_asset.py validate visual <asset>

python3 pt_asset.py validate animation <asset>

e futuramente:

python3 pt_asset.py import <asset>

python3 pt_asset.py build <asset>

python3 pt_asset.py publish <asset>

python3 pt_asset.py restore <asset>

python3 pt_asset.py validate next

O AssetRegistry deve decidir internamente qual family adapter utilizar.

Ordem recomendada de continuação
Corrigir definitivamente SMB source pose → Blender pose bone space.
Repetir Monbagon ATTACK até obter RIG=PASS.
Testar WALK e STAND no mesmo skeleton para cobrir motion cíclica e idle.
Testar animação em NPC e outro mob com estruturas diferentes.
Generalizar config-only → root skeleton binding.
Fechar static Blender roundtrip de weapons/items.
Implementar mixed-Physique.
Implementar static object-local animation.
Implementar multilayer TexLink.
Implementar Stage0.72 completo.
Resolver os 18 outliers individualmente ou por novas famílias descobertas.
Resolver os 2 TGA anômalos.
Consolidar publish/restore genérico em todos os adapters.
Rodar full-client regression.
Fechar todos os gates estruturais, Blender e runtime exigidos.
Somente então gerar PT_ASSET_TOOL_V1_0_0.
Ponto exato para retomar

Não precisamos pedir novos assets agora.

O ponto técnico imediato é:

Monbagon ATTACK

com:

motion sequence = aparentemente correta

texture/material = PASS

rig deformation = FAIL

Precisamos corrigir:

runtime/source transform

→

Blender pose transform

sem alterar dados do SMB e sem enfraquecer validators.

O próximo patch deve ser focado exclusivamente nessa conversão de espaço/hierarquia.

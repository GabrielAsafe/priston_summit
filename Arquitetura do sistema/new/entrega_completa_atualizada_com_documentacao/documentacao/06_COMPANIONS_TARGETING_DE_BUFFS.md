# Companions: política de alvo dos buffs

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. Estado atual

O catálogo de companions já classifica skills como:

- `BUFF_SELF`;
- `BUFF_GROUP`;
- `BUFF_TARGET`;
- além de heal/support e outros comportamentos.

Exemplos no catálogo atual:

- Windy — `BUFF_SELF`;
- Triumph of Valhalla — `BUFF_GROUP`;
- Hall of Valhalla — `BUFF_GROUP`;
- Virtual Life — `BUFF_GROUP`;
- Regeneration Field — `BUFF_GROUP`;
- Enchant Weapon — `BUFF_TARGET`.

Porém o executor não implementa uma semântica única de alvo:

- `TryExecuteCompanionBuffLoadout` aplica o modificador genérico ao próprio `caster`;
- `TryExecuteCompanionSupportLoadout` contém regras específicas para owner e companions, dependendo da skill.

Ou seja: hoje a decisão “em mim ou nos outros companions” está espalhada em lógica específica.

## 2. Não escolher uma única regra global

A solução mais robusta é transformar alvo em dado explícito por skill.

Enum recomendado:

```cpp
enum class CompanionTargetPolicy {
    SelfCompanion,
    Owner,
    OwnerAndCompanions,
    OtherCompanions,
    ExplicitTarget
};
```

Pode existir também uma policy futura para outros jogadores da party, mas só deve ser adicionada se o design realmente quiser companions buffando terceiros.

## 3. Mapeamento inicial recomendado

| Papel | Policy inicial | Motivo |
|---|---|---|
| `BUFF_SELF` | `SelfCompanion` | preserva o comportamento atual do buff loadout |
| `BUFF_GROUP` | `OwnerAndCompanions` | torna “group” previsível no ecossistema do owner |
| `BUFF_TARGET` | `ExplicitTarget` com fallback configurado | evita assumir que todo target é owner |

Esse mapeamento é ponto de partida. Skills excepcionais podem declarar policy diferente.

## 4. Configuração por skill

Idealmente mover a policy para catálogo de conteúdo/skill definition:

```json
{
  "skill": "SKILL_TRIUMPH_OF_VALHALLA",
  "role": "BUFF_GROUP",
  "target_policy": "owner_and_companions",
  "range": 300,
  "include_caster": true,
  "include_dead": false
}
```

O source deve executar a policy, e não ter `if (skillCode == ...)` para cada buff novo.

## 5. Regras que precisam ser decididas explicitamente

- owner recebe buff se estiver fora do range?
- companion caster conta como membro do grupo?
- companions mortos recebem buff persistente?
- buff substitui ou acumula versão anterior?
- dois companions podem aplicar o mesmo buff?
- qual versão ganha: maior nível, maior duração ou última aplicação?
- buff do companion persiste ao teleport/relog?
- buffs aplicam a outros players da party? (recomendação inicial: **não**, até existir regra específica)

## 6. Refactor incremental

1. adicionar `CompanionTargetPolicy`;
2. criar resolver de alvos único;
3. adaptar `BUFF_SELF` sem mudar comportamento;
4. adaptar `BUFF_GROUP` e comparar com skills especiais atuais;
5. adaptar `BUFF_TARGET`;
6. reduzir branches por `skillCode` apenas ao efeito realmente especial;
7. mover metadados de targeting para conteúdo;
8. adicionar testes de range, morte, owner, caster e múltiplos companions.

## 7. Test matrix mínima

Para cada skill de buff:

- owner presente/ausente;
- owner dentro/fora do range;
- 1 e vários companions;
- caster incluído/excluído;
- companion morto;
- target explícito inválido;
- buff já ativo menor/maior;
- recast interval;
- teleport/relog;
- owner desconectando durante cast.

A implementação está pronta quando a mesma policy produz o mesmo conjunto de alvos independentemente da skill concreta, salvo exceções documentadas.

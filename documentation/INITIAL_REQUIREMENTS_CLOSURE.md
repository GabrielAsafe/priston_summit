# Fechamento da lista inicial

| Requisito | Estado no bundle | Próxima ação na checkout real |
|---|---|---|
| História coletiva por servidor | **Milestone M01** + POC de contribution/runtime | implementar repository SQLite/CAS real e ligar ao Story V2 |
| Mundo muda ao avançar história | **Milestone M02** + `WorldPhaseRuntime` | implementar gateway real de maps/spawn/NPC/gates/relocate |
| Irreversível | regra monotônica no runtime | reforçar no write SQL/repository |
| NPC placement | formato e conversão de coordenadas documentados | migrar editor canonical para world coords |
| Altar Covenant | definição canonical + placement plan + patch UI/server | aplicar patch e validar Y/placement em jogo |
| Novo personagem/classe | workflow V2 fechado | criar a definição concreta quando identidade/assets da classe forem escolhidos |
| Pastas/ícones/animações/modelos/efeitos/audio | checklist | PT Asset Tool deve validar manifest por classe |
| Novo mapa | bug report completo | implementar ticket map 53/StageArea |
| Warp gates | já data-driven no map catalog | após ticket de maps, criar por canonical sem C++ |
| Capacetes/máscaras | schema/contrato e implementação definida | auditar bit de slot e integrar save/network/render |
| Novos monstros gameplay | já canonical/SCFG | model config agora também catalogado em JSON; completar decode local dos INX |
| INI/INX de monstros -> JSON | catálogo/bridge/tooling criado | rodar sync com os bytes reais e depois promover JSON a autoria oficial |
| Buff set completo | canonical + serviço C++ testado | integrar equip-change -> modifier reconciliation server-side |
| Dinheiro/drops | fórmulas e planilha | usar planilha + PT Asset Tool para editar/simular |
| Balance EXP | canonical + materializer + overflow patch + party2..8 patch | aplicar patches e definir metas XP/h |
| `/Servidor/Configs` | audit/migração definida | separar gameplay/runtime mutable/secrets |
| `/Servidor/GameServer/Monster/*.inf` | runtime compilado já é caminho principal | manter só import/archive após parity final |
| `/Servidor/Field` | corretamente classificado como asset binário | não converter mesh para JSON; referenciar via map content |
| `/Servidor/char` | audit completo | não apagar; gerar manifest e migrar server para headless depois |
| PT Asset Tool | integração com parser INX reaproveitada | adicionar telas/workflows usando canonical/model_configs e validators |

## O que ainda depende de trabalho C++ real

Não há mais uma área grande da lista inicial sem solução técnica definida. Restam integrações de runtime:

1. ticket de maps;
2. gateway server de world phases;
3. set modifiers no fluxo real de equip/stats;
4. head cosmetic slot no protocolo/save/render;
5. terminar o model-config runtime para deixar INX como build artifact;
6. transformar configs de gameplay restantes em source canonical.

Esses itens estão documentados/estruturados neste bundle para serem implementados sem nova fase de descoberta ampla.
